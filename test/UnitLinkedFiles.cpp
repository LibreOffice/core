/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Integration test for the files a master document links to. The document links its sections to
 * two files by their path relative to it, and the jail holds neither. The server asks the storage
 * for both by those paths, downloads the one the storage has into the jail and gives it to the
 * kit, which reads it for the links to that file from then on. The client sees none of this
 * traffic, and later reads the list of the linked files itself. Then the user inserts a file in
 * the Navigator of the master document: the kit asks the client for the file to link to, the
 * client answers with the path the integration gave for the file it picked, and the server asks
 * the storage for that file and downloads it for the kit, which links a new section to it. Last
 * the user updates the links, and the server asks the storage again for every file and downloads
 * fresh copies.
 */

#include <config.h>

#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <common/JsonUtil.hpp>
#include <helpers.hpp>
#include <test/testlog.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include <chrono>
#include <iterator>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
/// The paths the document links its sections to, relative to it.
constexpr std::string_view PresentPath = "master-chapter.odt";
constexpr std::string_view MissingPath = "master-missing-chapter.odt";
/// The file of PresentPath on the WOPI host, and the token the storage gives for it.
constexpr std::string_view ChapterWopiPath = "/wopi/files/chapter";
constexpr std::string_view ChapterToken = "chaptertoken";
}

class UnitLinkedFiles : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLinkAccess, WaitChapter, WaitRegistered, WaitList, WaitNavigator,
               WaitLinkPicker, WaitPickLinkAccess, WaitPickChapter, WaitPickInserted, WaitPickList,
               WaitRefreshLinkAccess, WaitRefreshChapter, Done)
    _phase;

    std::chrono::steady_clock::time_point _chapterServed;
    int _listsAsked = 0;
    bool _loadSent = false;
    /// The user allowed the links of the document to be updated.
    bool _allowSent = false;
    /// The messages the client sends next, one per call of invokeWSDTest().
    std::vector<std::string> _commandsDue;
    /// Guards _commandsDue, which more than one thread uses.
    std::mutex _commandsDueMutex;

    /// Queue a message for the client to send.
    void queueCommand(const std::string& command)
    {
        std::lock_guard<std::mutex> lock(_commandsDueMutex);
        _commandsDue.push_back(command);
    }

    /// Take the next message for the client to send, or return false if there is none.
    bool takeCommand(std::string& command)
    {
        std::lock_guard<std::mutex> lock(_commandsDueMutex);
        if (_commandsDue.empty())
            return false;

        command = _commandsDue.front();
        _commandsDue.erase(_commandsDue.begin());
        return true;
    }

public:
    UnitLinkedFiles()
        : WopiTestServer("UnitLinkedFiles", "master-document.odm")
        , _phase(Phase::Load)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsRelativeLinkAccess", true);
    }

    bool handleHttpRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                           const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        const std::string path = uriReq.getPath();

        if (path.ends_with("/linkaccess") && _phase == Phase::WaitPickLinkAccess)
        {
            TST_LOG("The storage is asked for the picked file: " << request.getURI());
            TRANSITION_STATE(_phase, Phase::WaitPickChapter);

            // The picked file is asked for by the path the integration gave, alone.
            const std::string body(std::istreambuf_iterator<char>(message), {});
            Poco::JSON::Object::Ptr object;
            LOK_ASSERT_MESSAGE("The question is JSON", JsonUtil::parseJSON(body, object));
            LOK_ASSERT_EQUAL(std::string("relative"),
                             JsonUtil::getJSONValue<std::string>(object, "Kind"));
            const Poco::JSON::Array::Ptr paths = object->getArray("RelativePaths");
            LOK_ASSERT_MESSAGE("The question names the path", paths);
            LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), paths->size());
            LOK_ASSERT_EQUAL(std::string(PresentPath), paths->getElement<std::string>(0));

            sendLinkAccessAnswer(socket, /*withMissing=*/false);
            return true;
        }

        if (path.ends_with("/linkaccess") && _phase == Phase::WaitRefreshLinkAccess)
        {
            TST_LOG("The storage is asked again: " << request.getURI());
            TRANSITION_STATE(_phase, Phase::WaitRefreshChapter);

            // Every file is asked for again, also the one the kit reads a copy of.
            const std::string body(std::istreambuf_iterator<char>(message), {});
            Poco::JSON::Object::Ptr object;
            LOK_ASSERT_MESSAGE("The question is JSON", JsonUtil::parseJSON(body, object));
            const Poco::JSON::Array::Ptr paths = object->getArray("RelativePaths");
            LOK_ASSERT_MESSAGE("The question names the paths", paths);
            LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), paths->size());
            LOK_ASSERT_EQUAL(std::string(PresentPath), paths->getElement<std::string>(0));

            sendLinkAccessAnswer(socket, /*withMissing=*/true);
            return true;
        }

        if (path.ends_with("/linkaccess"))
        {
            TST_LOG("The storage is asked: " << request.getURI());
            LOK_ASSERT_STATE(_phase, Phase::WaitLinkAccess);
            LOK_ASSERT_MESSAGE("The storage is asked once the user allowed the links", _allowSent);
            TRANSITION_STATE(_phase, Phase::WaitChapter);

            // Both files are asked for by their path relative to the document.
            const std::string body(std::istreambuf_iterator<char>(message), {});
            Poco::JSON::Object::Ptr object;
            LOK_ASSERT_MESSAGE("The question is JSON", JsonUtil::parseJSON(body, object));
            LOK_ASSERT_EQUAL(std::string("relative"),
                             JsonUtil::getJSONValue<std::string>(object, "Kind"));
            const Poco::JSON::Array::Ptr paths = object->getArray("RelativePaths");
            LOK_ASSERT_MESSAGE("The question names the paths", paths);
            LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), paths->size());
            LOK_ASSERT_EQUAL(std::string(PresentPath), paths->getElement<std::string>(0));
            LOK_ASSERT_EQUAL(std::string(MissingPath), paths->getElement<std::string>(1));

            // The storage has the first file and not the second one.
            sendLinkAccessAnswer(socket, /*withMissing=*/true);
            return true;
        }

        if (path == std::string(ChapterWopiPath) + "/contents")
        {
            TST_LOG("The linked file is downloaded: " << request.getURI());
            if (_phase == Phase::WaitPickChapter)
            {
                // The kit inserts a section that links to the picked file.
                _chapterServed = std::chrono::steady_clock::now();
                _listsAsked = 0;
                TRANSITION_STATE(_phase, Phase::WaitPickInserted);
            }
            else if (_phase == Phase::WaitRefreshChapter)
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("fresh-copy-fetched");
            }
            else
            {
                LOK_ASSERT_STATE(_phase, Phase::WaitChapter);
                _chapterServed = std::chrono::steady_clock::now();
                TRANSITION_STATE(_phase, Phase::WaitRegistered);
            }

            // The file is downloaded with the token the storage gave for it.
            std::string token;
            for (const auto& param : uriReq.getQueryParameters())
            {
                if (param.first == "access_token")
                    token = param.second;
            }
            LOK_ASSERT_EQUAL(std::string(ChapterToken), token);

            const std::vector<char> chapter = helpers::readDataFromFile(std::string(PresentPath));
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody(std::string(chapter.begin(), chapter.end()),
                                 "application/octet-stream");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        return WopiTestServer::handleHttpRequest(request, message, socket);
    }

    /// Answers a question for linked files: the storage has the chapter, and not the missing
    /// file when it is asked for that too.
    void sendLinkAccessAnswer(const std::shared_ptr<StreamSocket>& socket, bool withMissing)
    {
        Poco::JSON::Object::Ptr present = new Poco::JSON::Object();
        present->set("RelativePath", std::string(PresentPath));
        present->set("Status", 200);
        present->set("WOPISrc", getWopiHostURI() + std::string(ChapterWopiPath));
        present->set("AccessToken", std::string(ChapterToken));
        Poco::JSON::Array::Ptr links = new Poco::JSON::Array();
        links->add(present);
        if (withMissing)
        {
            Poco::JSON::Object::Ptr missing = new Poco::JSON::Object();
            missing->set("RelativePath", std::string(MissingPath));
            missing->set("Status", 404);
            links->add(missing);
        }
        Poco::JSON::Object::Ptr answer = new Poco::JSON::Object();
        answer->set("Links", links);
        std::ostringstream answerStream;
        answer->stringify(answerStream);

        http::Response httpResponse(http::StatusCode::OK);
        httpResponse.setBody(answerStream.str(), "application/json; charset=utf-8");
        socket->sendAndShutdown(httpResponse);
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        // What the server asks the kit about the linked files on its own, and the answers, are
        // not the client's.
        const bool listAsked = _phase == Phase::WaitList || _phase == Phase::WaitPickList;
        if (message.starts_with("linkedfile:") || message.starts_with("linkedfilesrefresh:") ||
            (message.starts_with("linkedfiles:") && !listAsked))
        {
            TST_LOG("The client was sent: " << message);
            failTest("server-traffic-reached-client");
            return false;
        }

        // The Navigator of the master document: the user picks Insert > File in it, as the
        // client sends the choices to the window an update of the Navigator names.
        if (_phase == Phase::WaitNavigator && message.starts_with("jsdialog:") &&
            message.find("\"jsontype\": \"navigator\"") != std::string_view::npos)
        {
            Poco::JSON::Object::Ptr dialog;
            LOK_ASSERT_MESSAGE("The Navigator is JSON",
                               JsonUtil::parseJSON(std::string(message.substr(message.find('{'))),
                                                   dialog));
            const std::string window = dialog->get("id").toString();
            TST_LOG("The Navigator is window [" << window << ']');
            TRANSITION_STATE(_phase, Phase::WaitLinkPicker);
            const std::string event = "dialogevent " + window + ' ';
            queueCommand(event +
                         R"({"id":"globaltree","cmd":"select","data":"1","type":"treeview"})");
            queueCommand(event +
                         R"({"id":"global","cmd":"openmenu","data":"insert","type":"toolbox"})");
            queueCommand(event +
                         R"({"id":"__MENU__","cmd":"select","data":"insertfile","type":"menu"})");
            return false;
        }

        // The kit asks for the file to link to, and the client answers with the path the
        // integration gave for the file the user picked.
        if (_phase == Phase::WaitLinkPicker && message.starts_with("linkpicker:"))
        {
            Poco::JSON::Object::Ptr request;
            LOK_ASSERT_MESSAGE("The request is JSON",
                               JsonUtil::parseJSON(std::string(message.substr(message.find('{'))),
                                                   request));
            const std::string id = request->get("id").toString();
            TST_LOG("The kit asks for a file to link to, pick [" << id << ']');
            TRANSITION_STATE(_phase, Phase::WaitPickLinkAccess);
            queueCommand("linkpickerresult id=" + id + " path=" + std::string(PresentPath));
            return false;
        }

        if (!listAsked || !message.starts_with("linkedfiles:"))
            return false;

        Poco::JSON::Object::Ptr object;
        LOK_ASSERT_MESSAGE("The list is JSON",
                           JsonUtil::parseJSON(std::string(message.substr(message.find('{'))),
                                               object));
        const Poco::JSON::Array::Ptr links = object->getArray("links");
        LOK_ASSERT_MESSAGE("The list has links", links);

        if (_phase == Phase::WaitPickList)
        {
            // A new section links to the picked file, next to the one that linked to it before,
            // and both read the copy.
            std::size_t readableChapters = 0;
            for (std::size_t i = 0; i < links->size(); ++i)
            {
                const Poco::JSON::Object::Ptr link = links->getObject(i);
                if (link->getValue<std::string>("id") == PresentPath &&
                    link->getValue<bool>("readable"))
                    ++readableChapters;
            }
            TST_LOG("The client reads the list: " << links->size() << " links, "
                                                  << readableChapters << " of the chapter");
            if (links->size() == 3 && readableChapters == 2)
            {
                // The user updates the links of the master document.
                TRANSITION_STATE(_phase, Phase::WaitRefreshLinkAccess);
                queueCommand("uno .uno:UpdateAllLinks");
            }
            else
                TRANSITION_STATE(_phase, Phase::WaitPickInserted);
            return false;
        }

        LOK_ASSERT_MESSAGE("The list has two links", links->size() == 2);
        const bool presentReadable = links->getObject(0)->getValue<bool>("readable");
        const bool missingReadable = links->getObject(1)->getValue<bool>("readable");
        LOK_ASSERT_MESSAGE("The client is not told where the files are in the jail",
                           !links->getObject(0)->has("url"));
        TST_LOG("The client reads the list: present " << presentReadable << ", missing "
                                                      << missingReadable);
        LOK_ASSERT_MESSAGE("The file the storage does not have is not readable",
                           !missingReadable);

        if (presentReadable)
        {
            // The user opens the Navigator, to insert a file into the master document.
            TRANSITION_STATE(_phase, Phase::WaitNavigator);
            queueCommand("uno .uno:Navigator?Navigator:bool=true");
        }
        else
        {
            // The kit may not have taken the copy yet when the client asks.
            TRANSITION_STATE(_phase, Phase::WaitRegistered);
        }

        return false;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::Load);
        TRANSITION_STATE(_phase, Phase::WaitLinkAccess);
        // The user allows the links to be updated, which the storage is asked for no file before.
        queueCommand("allowlinkupdate");
        return true;
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load && !_loadSent)
        {
            _loadSent = true;
            initWebsocket("/wopi/files/0?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
            return;
        }

        std::string command;
        if (takeCommand(command))
        {
            if (command == "allowlinkupdate")
                _allowSent = true;
            WSD_CMD(command);
            return;
        }

        // Some time after a file was downloaded, the client reads the list, and again until the
        // copy is there.
        const bool afterLoad = _phase == Phase::WaitRegistered;
        if ((afterLoad || _phase == Phase::WaitPickInserted) &&
            std::chrono::steady_clock::now() - _chapterServed > std::chrono::milliseconds(500))
        {
            if (++_listsAsked > 20)
            {
                failTest("copy-never-readable");
                return;
            }

            TRANSITION_STATE(_phase, afterLoad ? Phase::WaitList : Phase::WaitPickList);
            _chapterServed = std::chrono::steady_clock::now();
            WSD_CMD("linkedfile list");
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitLinkedFiles(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
