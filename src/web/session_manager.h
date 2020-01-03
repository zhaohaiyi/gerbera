/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    session_manager.h - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.cc>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>
    
    Copyright (C) 2006-2010 Gena Batyan <bgeradz@mediatomb.cc>,
                            Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>,
                            Leonhard Wimmer <leo@mediatomb.cc>
    
    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
    
    $Id$
*/

/// \file session_manager.h
/// \brief Definitions of the Session and SessionManager classes.
#ifndef __SESSION_MANAGER_H__
#define __SESSION_MANAGER_H__

#include <memory>
#include <unordered_set>
#include <vector>

#include "zmm/dictionary.h"
#include "util/timer.h"

// forward declaration
class ConfigManager;

namespace web {

/// \brief One UI session.
///
/// When the user logs in for the first time (via the web UI) a new Session will be
/// created. It then will be used to store various information to support
/// the UI - remember the position of browsing for each driver, remember the page
/// count for each driver and so on.
class Session : public Dictionary_r {
public:
    /// \brief Constructor, creates a session with a given timeout.
    /// \param timeout time in milliseconds after which the session will expire if not accessed.
    ///
    /// The session is created with a given timeout, each access to the session updates the
    /// last_access value, if last access lies further back than the timeout - the session will
    /// be deleted (will time out)
    Session(long timeout);

    /// \brief Returns the time of last access to the session.
    /// \return pointer to a timespec
    inline struct timespec* getLastAccessTime() { return &last_access; }

    inline long getTimeout() { return timeout; }

    /// \brief Returns the session identifier.
    inline std::string getID() { return sessionID; }

    /// \brief Sets the session identifier.
    inline void setID(std::string sessionID) { this->sessionID = sessionID; }

    inline bool isLoggedIn() { return loggedIn; }

    inline void logIn() { loggedIn = true; }

    inline void logOut() { loggedIn = false; }

    inline void access() { getTimespecNow(&last_access); }

    /// \brief Returns the updateIDs, collected for the sessions,
    /// and flushes the storage for the ids
    /// \return the container ids to be updated as String (comma separated)
    std::string getUIUpdateIDs();

    bool hasUIUpdateIDs();

    void clearUpdateIDs();

protected:
    /// \brief Is called by SessionManager if UI update is needed
    /// \param objectID the container that needs to be updated
    void containerChangedUI(int objectID);

    void containerChangedUI(const std::vector<int>& objectIDs);

    /// \brief True if the ui update id hash became to big and
    /// the UI shall update every container
    bool updateAll;

    std::shared_ptr<std::unordered_set<int>> uiUpdateIDs;

    /// \brief maximum time the session can be idle (starting from last_access)
    long timeout;

    /// \brief time of last access to the session, returned by getLastAccessTime()
    struct timespec last_access;

    /// \brief arbitrary but unique string representing the ID of the session (returned by getID())
    std::string sessionID;

    bool loggedIn;

    friend class SessionManager;
};

/// \brief This class offers ways to create new sessoins, stores all available sessions and provides access to them.
class SessionManager : public Timer::Subscriber {
protected:
    std::shared_ptr<Timer> timer;

    std::mutex mutex;
    using AutoLock = std::lock_guard<decltype(mutex)>;

    /// \brief This array is holding available sessions.
    zmm::Ref<zmm::Array<Session>> sessions;

    std::map<std::string,std::string> accounts;

    void checkTimer();
    bool timerAdded;

public:
    /// \brief Constructor, initializes the array.
    SessionManager(std::shared_ptr<ConfigManager> config, std::shared_ptr<Timer> timer);
    virtual ~SessionManager() override { log_debug("SessionManager destroyed\n"); }

    /// \brief Creates a Session with a given timeout.
    /// \param timeout Session timeout in milliseconds.
    zmm::Ref<Session> createSession(long timeout);

    /// \brief Returns the instance to a Session with a given sessionID
    /// \param ID of the Session.
    /// \return intance of the Session with a given ID or nullptr if no session with that ID was found.
    zmm::Ref<Session> getSession(std::string sessionID, bool doLock = true);

    /// \brief Removes a session
    void removeSession(std::string sessionID);

    std::string getUserPassword(std::string user);

    /// \brief Is called whenever a container changed in a way,
    /// so that it needs to be redrawn in the tree of the UI.
    /// notifies all active sessions.
    /// \param objectID
    void containerChangedUI(int objectID);

    void containerChangedUI(const std::vector<int>& objectIDs);

    virtual void timerNotify(std::shared_ptr<Timer::Parameter> parameter) override;
};

} // namespace

#endif //  __SESSION_MANAGER_H__
