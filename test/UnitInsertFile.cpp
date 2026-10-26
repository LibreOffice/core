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

/*
 * Integration test for uploading a file to insert: only a live view that can edit stages a file,
 * found by the access_token or access_header the upload carries, each upload is staged under a
 * fresh name of the server's own, and the staged file holds what was uploaded.
 */

#include <config.h>

#include <HttpRequest.hpp>
#include <Unit.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>

#include <chrono>
#include <fstream>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

class UnitInsertFile : public UnitWSD
{
    /// The view that loaded first.
    std::mutex _sessionMutex;
    std::weak_ptr<ClientSession> _editSession;

    /// What the file staged in the document of the first view under the given name holds.
    std::string readStaged(const std::string& stagedName)
    {
        std::shared_ptr<ClientSession> session;
        {
            std::lock_guard<std::mutex> lock(_sessionMutex);
            session = _editSession.lock();
        }
        LOK_ASSERT_MESSAGE("no view to read [" + stagedName + "] for", session);

        const std::shared_ptr<DocumentBroker> docBroker = session->getDocumentBroker();
        LOK_ASSERT_MESSAGE("the view has no document broker", docBroker);

        auto staged = std::make_shared<std::promise<std::string>>();
        docBroker->addCallback(
            [docBroker, stagedName, staged]()
            {
                std::ifstream file(docBroker->jailStagingPath(stagedName), std::ios::binary);
                std::ostringstream content;
                content << file.rdbuf();
                staged->set_value(content.str());
            });

        std::future<std::string> result = staged->get_future();
        LOK_ASSERT_MESSAGE("reading [" + stagedName + "] timed out",
                           result.wait_for(std::chrono::seconds(10)) ==
                               std::future_status::ready);
        return result.get();
    }

    /// The URL of a view of the document of documentURL opened with the given query, which is
    /// added to the document URI part of the URL encoded ("%3F" is the encoded "?").
    static std::string viewURL(std::string documentURL, const std::string& encodedQuery)
    {
        documentURL.insert(documentURL.rfind("/ws"), "%3F" + encodedQuery);
        return documentURL;
    }

public:
    UnitInsertFile()
        : UnitWSD("UnitInsertFile")
    {
    }

    void onDocBrokerAddSession(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        if (!_editSession.lock())
            _editSession = session;
    }

    void invokeWSDTest() override
    {
        try
        {
            std::string documentPath, documentURL;
            helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

            std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
            socketPoll->startThread();

            // Three views of the one document: one that can edit with an access_token, one
            // that can edit with an access_header, and one that cannot edit.
            const Poco::URI uri(helpers::getTestServerURI());
            std::shared_ptr<http::WebSocketSession> editSocket = helpers::loadDocAndGetSession(
                socketPoll, uri, viewURL(documentURL, "access_token%3Dedit-token"), testname);
            std::shared_ptr<http::WebSocketSession> headerSocket =
                helpers::loadDocAndGetSession(
                    socketPoll, uri, viewURL(documentURL, "access_header%3Dedit-header"),
                    testname);
            std::shared_ptr<http::WebSocketSession> readOnlySocket =
                helpers::loadDocAndGetSession(
                    socketPoll, uri,
                    viewURL(documentURL, "permission%3Dreadonly%26access_token%3Dview-token"),
                    testname);

            // A view that can edit stages its file, and the reply names the file.
            std::shared_ptr<const http::Response> first = helpers::postToInsertFile(
                documentURL, "access_token", "edit-token", "first upload");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, first->statusCode());
            const std::string firstName(first->getBody());
            LOK_ASSERT_MESSAGE("the staged name keeps the uploaded file's name",
                               firstName.ends_with("-upload"));
            LOK_ASSERT_EQUAL(std::string("first upload"), readStaged(firstName));

            // A second upload gets a name of its own, so it leaves the first file as it was.
            std::shared_ptr<const http::Response> second = helpers::postToInsertFile(
                documentURL, "access_token", "edit-token", "second upload");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, second->statusCode());
            const std::string secondName(second->getBody());
            LOK_ASSERT_MESSAGE("each upload gets its own name", firstName != secondName);
            LOK_ASSERT_EQUAL(std::string("first upload"), readStaged(firstName));
            LOK_ASSERT_EQUAL(std::string("second upload"), readStaged(secondName));

            // A view opened with an access_header uploads with it.
            std::shared_ptr<const http::Response> third = helpers::postToInsertFile(
                documentURL, "access_header", "edit-header", "third upload");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, third->statusCode());
            LOK_ASSERT_EQUAL(std::string("third upload"), readStaged(third->getBody()));

            // A view that cannot edit stages nothing, and neither does a credential no view holds.
            LOK_ASSERT_EQUAL(http::StatusCode::Forbidden,
                             helpers::postToInsertFile(documentURL, "access_token", "view-token",
                                                       "from a viewer")
                                 ->statusCode());
            LOK_ASSERT_EQUAL(http::StatusCode::Forbidden,
                             helpers::postToInsertFile(documentURL, "access_token",
                                                       "unknown-token", "from nobody")
                                 ->statusCode());
            LOK_ASSERT_EQUAL(http::StatusCode::BadRequest,
                             helpers::postToInsertFile(documentURL, "access_token", std::string(),
                                                       "with no credential")
                                 ->statusCode());

            socketPoll->joinThread();
        }
        catch (const Poco::Exception& exc)
        {
            LOK_ASSERT_FAIL(exc.displayText());
        }

        exitTest(TestResult::Ok);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitInsertFile(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
