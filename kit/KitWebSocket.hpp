/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

/*
 * The main entry point for the COKit process serving
 * a document editing session.
 */

#include <net/WebSocketHandler.hpp>

#include <memory>
#include <string>

class ChildSession;
class Document;
class KitQueue;
class KitSocketPoll;

namespace kit
{
class Office;
}

class KitWebSocketHandler final : public WebSocketHandler
{
    std::string _socketName;
    std::shared_ptr<kit::Office> _loKit;
    std::string _jailId;
    std::string _docKey; ///< When we get it while creating a new view.
    std::shared_ptr<Document> _document;
    std::shared_ptr<KitSocketPoll> _ksPoll;
    const unsigned _mobileAppDocId;
    bool _backgroundSaver;

public:
    KitWebSocketHandler(const std::string& socketName, const std::shared_ptr<kit::Office>& loKit,
                        const std::string& jailId, std::shared_ptr<KitSocketPoll> ksPoll,
                        unsigned mobileAppDocId)
        : WebSocketHandler(/* isClient = */ true, /* isMasking */ false)
        , _socketName(socketName)
        , _loKit(loKit)
        , _jailId(jailId)
        , _ksPoll(std::move(ksPoll))
        , _mobileAppDocId(mobileAppDocId)
        , _backgroundSaver(false)
    {
    }

    ~KitWebSocketHandler()
    {
        // Just to make it easier to set a breakpoint
    }

    void shutdownForBackgroundSave();

protected:
    virtual void handleMessage(const std::vector<char>& data) override;
    virtual void enableProcessInput(bool enable = true) override;
    virtual void onDisconnect() override;
};

/// WebSocket for a background save child process to talk to its parent Kit
class BgSaveChildWebSocketHandler final : public WebSocketHandler
{
    std::string _socketName;

public:
    BgSaveChildWebSocketHandler(const std::string& socketName)
        : WebSocketHandler(/* isClient = */ true, /* isMasking */ false)
        , _socketName(socketName)
    {
    }

    ~BgSaveChildWebSocketHandler();

protected:
    virtual void handleMessage(const std::vector<char>& data) override;
    virtual void onDisconnect() override;
};

/// WebSocket for a Kit process to talk to its background save child
class BgSaveParentWebSocketHandler final : public WebSocketHandler
{
    pid_t _childPid;
    bool _saveCompleted;
    std::string _socketName;
    std::shared_ptr<Document> _document;
    std::shared_ptr<ChildSession> _session;

public:
    BgSaveParentWebSocketHandler(const std::string& socketName, const pid_t childPid,
                                 std::shared_ptr<Document> document,
                                 const std::shared_ptr<ChildSession>& session);

    ~BgSaveParentWebSocketHandler();

    // Decide if it is not an interactive prompt that the user must answer.
    static bool isBenignBgSaveJSDialog(const std::string& jsdialogPayload);

protected:
    virtual void handleMessage(const std::vector<char>& data) override;
    virtual void onDisconnect() override;

    // something weird happened, cleanup & report save failure
    void terminateSave(const std::string &reason);

    // let WSD know something went wrong during the save
    void reportFailedSave(const std::string &reason);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
