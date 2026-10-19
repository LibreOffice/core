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

#include <common/Message.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Unit.hpp>
#include <net/HttpRequest.hpp>
#include <test/WopiTestServer.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

/// A document that several users open, each with an access token of their own. The first of
/// them may change its security label: each upload the host takes stores the label of the latest
/// change, and from then on the host answers each CheckFileInfo as the scenario says. A scenario
/// ends once what it waits for has happened and nothing more happens for a while.
class SecurityLabelTest : public WopiTestServer
{
public:
    /// How the host answers a CheckFileInfo.
    enum class Answer
    {
        Default, ///< 200 with the file info, as configFileInfo makes it.
        Respond, ///< The response the scenario gives.
        Drop, ///< The connection closes with no answer.
        Hold, ///< 200 with the file info as it is now, sent on releaseHeldCheckFileInfo.
    };

protected:
    /// A CheckFileInfo answer that waits to be sent, with the connection to send it on.
    using HeldAnswer = std::pair<std::shared_ptr<StreamSocket>, std::unique_ptr<http::Response>>;

    SecurityLabelTest(const std::string& name, std::vector<std::string> tokens)
        : WopiTestServer(name)
        , _tokens(std::move(tokens))
        , _running(false)
        , _labelChanged(false)
        , _uploads(0)
        , _uploaded(0)
        , _uploadResults(0)
    {
        useOwnWopiHost();
        setTimeout(std::chrono::seconds(90));
    }

    /// The host's answer to a CheckFileInfo of the view with the given token, the count-th one
    /// since the label first changed, with the label it stores then (empty when none).
    virtual Answer answerCheckFileInfo(const std::string& /*token*/, const std::string& /*label*/,
                                       std::size_t /*count*/,
                                       std::unique_ptr<http::Response>& /*response*/)
    {
        return Answer::Default;
    }

    /// The permissions the host gives the view with the given token, with the label it stores
    /// (empty when none). Everyone may edit by default.
    virtual void configFileInfo(const std::string& /*token*/, const std::string& /*label*/,
                                Poco::JSON::Object::Ptr& /*fileInfo*/)
    {
    }

    /// The host's answer to the count-th upload, or null to store it.
    virtual std::unique_ptr<http::Response> answerUpload(std::size_t /*count*/) { return nullptr; }

    /// Every view is loaded. The label is applied by default.
    virtual void onViewsLoaded() { changeLabel("Secret"); }

    /// The result of the count-th upload reached the document broker. Runs on its thread.
    virtual void onUploaded(std::size_t /*count*/, bool /*success*/) {}

    /// The host lets the user only read the document.
    static void makeReader(Poco::JSON::Object::Ptr& fileInfo)
    {
        fileInfo->set("UserCanWrite", "false");
    }

    /// The host lets the user only comment on the document.
    static void makeCommenter(Poco::JSON::Object::Ptr& fileInfo)
    {
        fileInfo->set("UserCanWrite", "false");
        fileInfo->set("UserCanOnlyComment", "true");
    }

    /// The host's 428 refusal of an upload, with its message for the user.
    static std::unique_ptr<http::Response> makePreconditionResponse(const std::string& message)
    {
        auto response = std::make_unique<http::Response>(http::StatusCode::PreconditionRequired);
        // The key's case is the host's choice.
        response->setBody("{\"message\": \"" + message + "\"}", "application/json; charset=utf-8");
        return response;
    }

    /// Runs on each pass of the test loop, until the scenario settles.
    virtual void onTick() {}

    /// Whether everything the scenario waits for has happened.
    virtual bool isSettled() = 0;

    /// Checks the outcome, on the document broker's thread.
    virtual void verify() = 0;

    /// Gives the view of the first user the label, with the payload the kit sends. An empty
    /// marking removes the label.
    void changeLabel(const std::string& marking)
    {
        std::string action;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            action = marking.empty() ? "removed" : _pendingLabel.empty() ? "applied" : "changed";
            _pendingLabel = marking;
            _labelChanged = true;
        }

        const std::string message =
            "statechanged: {\"commandName\":\".uno:SecurityLabel\",\"state\":{\"action\":\"" +
            action + "\",\"marking\":\"" + marking + "\"}}";
        TST_LOG("Changing the label: " << message);
        const std::shared_ptr<ClientSession> session = getSession(_tokens[0]);
        const std::shared_ptr<DocumentBroker> docBroker = session->getDocumentBroker();
        LOK_ASSERT(docBroker);
        docBroker->addCallback(
            [session, message]()
            {
                session->handleKitToClientMessage(
                    std::make_shared<Message>(message, Message::Dir::Out));
            });
    }

    /// Sends the CheckFileInfo answers held so far.
    void releaseHeldCheckFileInfo()
    {
        runOnWopiHost(
            [this]()
            {
                std::vector<HeldAnswer> held;
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    held.swap(_held);
                }

                TST_LOG("Releasing " << held.size() << " held CheckFileInfo answer(s)");
                for (const auto& it : held)
                    it.first->sendAndShutdown(*it.second);
            });
    }

    /// Modifies the document in the first user's view and saves it.
    void modifyAndSave()
    {
        TST_LOG("Modifying and saving");
        WSD_CMD_BY_CONNECTION_INDEX(0, "key type=input char=97 key=0");
        WSD_CMD_BY_CONNECTION_INDEX(0, "key type=up char=0 key=512");
        WSD_CMD_BY_CONNECTION_INDEX(0, "save dontTerminateEdit=0 dontSaveIfUnmodified=0");
    }

    std::shared_ptr<ClientSession> getSession(const std::string& token)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _sessions[token];
    }

    /// The labels the host stored at each CheckFileInfo of the view with the given token since
    /// the label first changed, comma-separated.
    std::string getChecks(const std::string& token)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return join(_checks[token]);
    }

    /// The number of CheckFileInfo requests of the view with the given token since the label
    /// first changed, or of all views for an empty token.
    std::size_t getCheckCount(const std::string& token = std::string())
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!token.empty())
            return _checks[token].size();

        std::size_t count = 0;
        for (const auto& it : _checks)
            count += it.second.size();
        return count;
    }

    std::size_t getHeldCount()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _held.size();
    }

    /// How many times views were sent the error of the given kind.
    std::size_t getErrorCount(const std::string& kind)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _errors[kind].size();
    }

    /// The last error of the given kind that a view was sent, or empty if none was.
    std::string getLastError(const std::string& kind)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _errors[kind].empty() ? std::string() : _errors[kind].back();
    }

    /// The tokens of the views removed since the label first changed, sorted and comma-separated.
    std::string getRemoved()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return join(_removed);
    }

    /// Whether each upload was an autosave, in order: "autosave" or "save", comma-separated.
    std::string getAutosaves()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return join(_autosaves);
    }

    /// The number of uploads the host got, and of those that succeeded.
    std::size_t getUploadCount() const { return _uploads; }
    std::size_t getUploadedCount() const { return _uploaded; }

private:
    template <typename T> static std::string join(const T& items)
    {
        std::string result;
        for (const std::string& item : items)
            result += (result.empty() ? "" : ",") + item;
        return result;
    }

    /// How long to wait, once everything expected happened, for anything unexpected.
    static constexpr std::chrono::seconds SettleDuration = std::chrono::seconds(3);

    static std::string getToken(const Poco::URI& uri)
    {
        for (const auto& param : uri.getQueryParameters())
        {
            if (param.first == "access_token")
                return param.second;
        }

        return std::string();
    }

    bool handleCheckFileInfoRequest(const Poco::Net::HTTPRequest& request,
                                    const std::shared_ptr<StreamSocket>& socket) override
    {
        const std::string token = getToken(Poco::URI(request.getURI()));
        std::string label;
        std::size_t count = 0;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            label = _storedLabel;
            if (_labelChanged)
            {
                _checks[token].push_back(label);
                count = _checks[token].size();
            }
        }

        if (count == 0)
            return WopiTestServer::handleCheckFileInfoRequest(request, socket);

        std::unique_ptr<http::Response> response;
        const Answer answer = answerCheckFileInfo(token, label, count, response);
        TST_LOG("CheckFileInfo #" << count << " of [" << token << "] with label [" << label
                                  << "]: " << static_cast<int>(answer));
        switch (answer)
        {
            case Answer::Default:
                socket->sendAndShutdown(*makeCheckFileInfoResponse(request));
                break;
            case Answer::Respond:
                socket->sendAndShutdown(*response);
                break;
            case Answer::Drop:
                socket->asyncShutdown();
                break;
            case Answer::Hold:
            {
                response = makeCheckFileInfoResponse(request);
                std::lock_guard<std::mutex> lock(_mutex);
                _held.emplace_back(socket, std::move(response));
                break;
            }
        }

        return true;
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        std::string label;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            label = _storedLabel;
        }

        configFileInfo(getToken(Poco::URI(request.getURI())), label, fileInfo);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        // The upload carries the label of the latest change made before it arrived.
        std::string label;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            label = _pendingLabel;
        }

        // Counted once answered, so that a scenario sees the count with the answer's effects.
        const std::size_t count = _uploads + 1;
        std::unique_ptr<http::Response> response = answerUpload(count);
        _uploads = count;
        const bool stored = !response || response->statusLine().statusCategory() ==
                                             http::StatusLine::StatusCodeClass::Successful;
        TST_LOG("PutFile #" << count << (stored ? " stored" : " refused"));

        std::lock_guard<std::mutex> lock(_mutex);
        _autosaves.push_back(
            request.get("X-COOL-WOPI-IsAutosave", std::string()) == "true" ? "autosave" : "save");
        if (stored)
            _storedLabel = label;

        return response;
    }

    void onDocumentUploaded(bool success) override
    {
        if (success)
            ++_uploaded;

        onUploaded(++_uploadResults, success);
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        const std::string token = getToken(session->getPublicUri());
        TST_LOG("View [" << session->getName() << "] of [" << token << "] loaded");

        bool loaded;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _sessions[token] = session;
            loaded = _sessions.size() == _tokens.size();
        }

        if (loaded)
        {
            _running = true;
            onViewsLoaded();
        }
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');
        const StringVector tokens = StringVector::tokenize(message);
        std::string kind;
        if (tokens.size() >= 3 && COOLProtocol::getTokenString(tokens[2], "kind", kind))
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _errors[kind].push_back(message);
        }

        return false;
    }

    void onDocBrokerRemovingSession(const std::string&,
                                    const std::shared_ptr<ClientSession>& session) override
    {
        if (!_running || !_labelChanged)
            return;

        std::lock_guard<std::mutex> lock(_mutex);
        for (const auto& it : _sessions)
        {
            if (it.second == session)
            {
                TST_LOG("The view of [" << it.first << "] is removed");
                _removed.insert(it.first);
            }
        }
    }

    void invokeWSDTest() override
    {
        if (!_started)
        {
            _started = true;

            // Each view has a token of its own, so the host can tell them apart. Each is
            // inserted at the front, so the first user ends up first.
            std::map<std::string, std::string> wopiSrcs;
            for (auto it = _tokens.rbegin(); it != _tokens.rend(); ++it)
                wopiSrcs[*it] = initWebsocket("/wopi/files/0?access_token=" + *it);

            for (std::size_t index = 0; index < _tokens.size(); ++index)
                WSD_CMD_BY_CONNECTION_INDEX(index, "load url=" + wopiSrcs[_tokens[index]]);
            return;
        }

        if (!_running || _done)
            return;

        onTick();
        if (!isSettled())
        {
            _settleStart = std::chrono::steady_clock::time_point();
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        if (_settleStart == std::chrono::steady_clock::time_point())
            _settleStart = now;

        if (now - _settleStart < SettleDuration || !isSettled())
            return;

        _done = true;

        // Session state is the broker's, so check there. A broker that has closed holds none.
        const std::shared_ptr<DocumentBroker> docBroker =
            getSession(_tokens[0])->getDocumentBroker();
        if (!docBroker)
        {
            verify();
            passTest("Everything the scenario expects happened");
            return;
        }

        docBroker->addCallback(
            [this]()
            {
                verify();
                passTest("Everything the scenario expects happened");
            });
    }

    /// The views' tokens, the first user's first.
    const std::vector<std::string> _tokens;

    /// Guards the members below that are not atomic. The test, the document broker and the
    /// WOPI host each reach them from a thread of their own.
    std::mutex _mutex;
    std::map<std::string, std::shared_ptr<ClientSession>> _sessions;
    std::map<std::string, std::vector<std::string>> _checks;
    std::map<std::string, std::vector<std::string>> _errors;
    std::set<std::string> _removed;
    std::vector<std::string> _autosaves;
    std::vector<HeldAnswer> _held;

    /// The label of the latest change, and the one the host stores. Empty when there is none.
    std::string _pendingLabel;
    std::string _storedLabel;

    std::atomic_bool _running;
    std::atomic_bool _labelChanged;
    std::atomic<std::size_t> _uploads;
    std::atomic<std::size_t> _uploaded;
    std::atomic<std::size_t> _uploadResults;
    bool _started = false;
    bool _done = false;
    std::chrono::steady_clock::time_point _settleStart;
};
