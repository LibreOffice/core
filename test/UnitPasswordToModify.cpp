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
 * Unit test for a document that has a password to modify. Such a document
 * loads read-only, and each view stays read-only until that view sends the
 * password with an editwithpassword message.
 */

#include <config.h>

#include <Unit.hpp>
#include <WebSocketSession.hpp>
#include <common/JsonUtil.hpp>
#include <helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/URI.h>

#include <chrono>
#include <cstring>
#include <memory>
#include <string>

using namespace std::literals;

namespace
{
/// The value of the readonly flag in a status: message.
bool isStatusReadOnly(const std::string& statusResponse)
{
    Poco::JSON::Object::Ptr object;
    if (!JsonUtil::parseJSON(statusResponse.substr(strlen("status: ")), object))
        return false;
    return JsonUtil::getJSONValue<bool>(object, "readonly");
}

/// Receives the "haspasswordtomodify" and "status" messages that a view gets after loading a
/// document with a password to edit. The two arrive in no particular order. Asserts the
/// password flag and returns the "status" message.
std::string assertLockedViewLoad(const std::shared_ptr<http::WebSocketSession>& socket,
                                 const std::string_view testname)
{
    std::string statusResponse;
    std::string passwordResponse;
    for (int i = 0; i < 2; ++i)
    {
        const std::string response = helpers::getResponseStringAny(
            socket, { "haspasswordtomodify:", "status:" }, testname);
        if (response.rfind("status:", 0) == 0)
            statusResponse = response;
        else
            passwordResponse = response;
    }
    LOK_ASSERT_EQUAL_STR("haspasswordtomodify: true", passwordResponse);
    return statusResponse;
}
}

class UnitPasswordToModify : public UnitWSD
{
    TestResult testUnlockDocument(const std::string& docFilename, const std::string& openPassword,
                                  const std::string& modifyPassword);

public:
    UnitPasswordToModify()
        : UnitWSD("UnitPasswordToModify")
    {
        setTimeout(120s);
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitPasswordToModify::testUnlockDocument(const std::string& docFilename,
                                                              const std::string& openPassword,
                                                              const std::string& modifyPassword)
{
    TST_LOG("Testing unlock of [" << docFilename << ']');

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL(docFilename, documentPath, documentURL, testname);

    std::string loadMessage = "load url=" + documentURL;
    if (!openPassword.empty())
        loadMessage += " password=" + openPassword;

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname + "Poll");
    socketPoll->startThread();

    Poco::URI uri(helpers::getTestServerURI());
    std::shared_ptr<http::WebSocketSession> socket =
        helpers::connectLOKit(socketPoll, uri, documentURL, testname);

    // The document loads read-only, and the status says so.
    helpers::sendTextFrame(socket, loadMessage, testname);
    std::string response = assertLockedViewLoad(socket, testname);
    LOK_ASSERT(isStatusReadOnly(response));

    // A message without a password is a syntax error.
    helpers::sendTextFrame(socket, "editwithpassword", testname);
    response = helpers::assertResponseString(socket, "error: cmd=editwithpassword", testname);
    LOK_ASSERT_EQUAL_STR("error: cmd=editwithpassword kind=syntax", response);

    // A wrong password keeps the view read-only
    helpers::sendTextFrame(socket, "editwithpassword password=wrong", testname);
    response = helpers::assertResponseString(socket, "error: cmd=editwithpassword", testname);
    LOK_ASSERT_EQUAL_STR("error: cmd=editwithpassword kind=wrongpassword", response);

    // The right password on a second attempt unlocks this view. The first
    // unlock also switches the document itself to editable.
    helpers::sendTextFrame(socket, "editwithpassword password=" + modifyPassword, testname);
    response = helpers::assertResponseString(socket, "editwithpassword:", testname);
    LOK_ASSERT_EQUAL_STR("editwithpassword: success", response);
    helpers::assertResponseString(socket, "statechanged: .uno:EditDoc=true", testname);

    // Typing works after the unlock.
    helpers::sendTextFrame(socket, "textinput id=0 text=x", testname);
    helpers::assertResponseString(socket, "statechanged: .uno:ModifiedStatus=true", testname);

    // The modified document saves back without errors.
    helpers::sendTextFrame(socket, "save dontTerminateEdit=0 dontSaveIfUnmodified=0", testname);
    while (true)
    {
        response = helpers::assertResponseString(socket, "unocommandresult:", testname);
        Poco::JSON::Object::Ptr object;
        LOK_ASSERT(JsonUtil::parseJSON(response.substr(strlen("unocommandresult: ")), object));
        if (JsonUtil::getJSONValue<std::string>(object, "commandName") == ".uno:Save")
        {
            LOK_ASSERT_EQUAL(true, JsonUtil::getJSONValue<bool>(object, "success"));
            break;
        }
    }

    // A view that joins after the unlock is asked for the password as well.
    std::shared_ptr<http::WebSocketSession> socket2 =
        helpers::connectLOKit(socketPoll, uri, documentURL, testname);
    helpers::sendTextFrame(socket2, loadMessage, testname);
    // The earlier unlock made the document itself editable, but only for the view that sent the
    // password. The second view's status is still read-only.
    response = assertLockedViewLoad(socket2, testname);
    LOK_ASSERT(isStatusReadOnly(response));

    helpers::sendTextFrame(socket2, "textinput id=0 text=y", testname);
    helpers::assertNotInResponse(socket2, "statechanged: .uno:ModifiedStatus=true", testname);

    helpers::sendTextFrame(socket2, "editwithpassword password=wrong", testname);
    response = helpers::assertResponseString(socket2, "error: cmd=editwithpassword", testname);
    LOK_ASSERT_EQUAL_STR("error: cmd=editwithpassword kind=wrongpassword", response);

    helpers::sendTextFrame(socket2, "editwithpassword password=" + modifyPassword, testname);
    response = helpers::assertResponseString(socket2, "editwithpassword:", testname);
    LOK_ASSERT_EQUAL_STR("editwithpassword: success", response);

    // Typing works in the second view after its own unlock.
    helpers::sendTextFrame(socket2, "textinput id=0 text=z", testname);
    helpers::assertResponseString(socket2, "statechanged: .uno:ModifiedStatus=true", testname);

    return TestResult::Ok;
}

void UnitPasswordToModify::invokeWSDTest()
{
    // A document with both passwords, saved with the read-only recommendation.
    TestResult result = testUnlockDocument("password-to-open-and-modify.odt", "hello", "world");

    // A document whose only protection is the password to modify, with no read-only
    // recommendation and no password to open.
    if (result == TestResult::Ok)
        result = testUnlockDocument("password-to-modify-no-recommend.docx", "", "world");

    exitTest(result);
}

UnitBase* unit_create_wsd(void) { return new UnitPasswordToModify(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
