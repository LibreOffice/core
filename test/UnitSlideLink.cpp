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
 * Integration test for the slidelink command family: insert slides as links to
 * the source document they came from, read the links the document holds,
 * refresh them from a file staged in the jail, the way wsd hands the kit the
 * content of a source, and take the source off one page. Also covers the error
 * kinds and what a client may ask for itself.
 */

#include <config.h>

#include <HttpRequest.hpp>
#include <Unit.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

/// The source document as the user knows it. The characters that a reference
/// escapes are in the name, so that the escaping is exercised end to end.
static constexpr auto SourceName = "Q3 #1 100%.odp";
/// SourceName as it travels on the wire.
static constexpr auto EncodedSourceName = "Q3%20%231%20100%25.odp";

class UnitSlideLink : public UnitWSD
{
    TestResult testSlideLinkList();
    TestResult testSlideLinkRefresh();
    TestResult testSlideLinkBreak();
    TestResult testSlideLinkErrors();
    TestResult testReadOnly();

    /// The session that last loaded a document, from which wsd's own messages
    /// to the kit are sent.
    std::mutex _sessionMutex;
    std::weak_ptr<ClientSession> _session;

    /// Loads a presentation of one slide and returns the session for it.
    std::shared_ptr<http::WebSocketSession> load(const std::shared_ptr<SocketPoll>& socketPoll,
                                                 const std::string& documentURL)
    {
        return helpers::loadDocAndGetSession(socketPoll, Poco::URI(helpers::getTestServerURI()),
                                             documentURL, testname);
    }

    /// Stages a ten-slide presentation in the jail under the given name. An insert takes the
    /// file it names, so every insert is staged for.
    void stageSource(const std::shared_ptr<http::WebSocketSession>& socket,
                     const std::string& documentURL, const std::string& stagedName)
    {
        const std::string childId = helpers::getChildId(socket, testname);
        LOK_ASSERT_EQUAL(http::StatusCode::OK,
                         helpers::postToInsertFile(documentURL, childId, stagedName,
                                                   helpers::readFileAsString("setclientpart.odp")));
    }

    /// The insert command for the staged file, naming the source document the pages record.
    std::string insertCommand(const std::string& options, bool named = true)
    {
        std::string command = "slideimport insert file=source.odp ";
        if (named)
            command += std::string("source=") + EncodedSourceName + ' ';

        return command + options;
    }

    /// Send a message to the kit the way wsd sends one, so that a command a
    /// client cannot reach is still exercised end to end.
    void sendAsWsd(const std::string& message)
    {
        std::shared_ptr<ClientSession> session;
        {
            std::lock_guard<std::mutex> lock(_sessionMutex);
            session = _session.lock();
        }
        LOK_ASSERT_MESSAGE("no session to send [" + message + "] through", session);

        const std::shared_ptr<DocumentBroker> docBroker = session->getDocumentBroker();
        LOK_ASSERT_MESSAGE("the session has no document broker", docBroker);

        TST_LOG("Sending as wsd: " << message);
        docBroker->addCallback([docBroker, session, message]()
                               { docBroker->forwardToChild(session, message); });
    }

    /// The link list of the document, from a slidelink list request.
    Poco::JSON::Array::Ptr getLinks(const std::shared_ptr<http::WebSocketSession>& socket)
    {
        helpers::sendTextFrame(socket, "slidelink list", testname);
        const std::string reply = helpers::getResponseString(socket, "slidelinks:", testname);
        Poco::JSON::Object::Ptr links = helpers::parseJsonReply(reply, "slidelinks:");
        Poco::JSON::Array::Ptr array = links->getArray("links");
        LOK_ASSERT_MESSAGE("missing link list", array);
        return array;
    }

    /// The parts of the slides one link list entry holds, in the order it
    /// reports them, checking the source and the slide names on the way.
    std::vector<std::string> getParts(const Poco::JSON::Object::Ptr& source,
                                      std::size_t expectedSlides)
    {
        LOK_ASSERT_MESSAGE("missing source entry", source);
        LOK_ASSERT_EQUAL(std::string(SourceName), source->getValue<std::string>("source"));
        Poco::JSON::Array::Ptr slides = source->getArray("slides");
        LOK_ASSERT_MESSAGE("missing slide list", slides);
        LOK_ASSERT_EQUAL(expectedSlides, slides->size());

        std::vector<std::string> parts;
        for (std::size_t i = 0; i < slides->size(); ++i)
        {
            Poco::JSON::Object::Ptr slide = slides->getObject(i);
            LOK_ASSERT_MESSAGE("missing slide entry", slide);
            // The exact name is locale-dependent ("Slide 1" in English), so
            // only check that the source slide is named at all.
            LOK_ASSERT_MESSAGE("missing source slide name",
                               !slide->getValue<std::string>("name").empty());
            const std::string part = slide->getValue<std::string>("part");
            LOK_ASSERT_MESSAGE("a linked page must report a part", !part.empty());
            parts.push_back(part);
        }

        return parts;
    }

    /// Send a command the way wsd does and assert the error reply it provokes.
    void assertErrorAsWsd(const std::shared_ptr<http::WebSocketSession>& socket,
                          const std::string& command, const std::string& expectedKind,
                          const std::string& named = std::string())
    {
        sendAsWsd(command);
        const std::string error = helpers::getResponseString(socket, "error:", testname);
        LOK_ASSERT_EQUAL("error: cmd=slidelink kind=" + expectedKind + named, error);
    }

public:
    UnitSlideLink()
        : UnitWSD("UnitSlideLink")
    {
    }

    void onDocBrokerAddSession(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        std::lock_guard<std::mutex> lock(_sessionMutex);
        _session = session;
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitSlideLink::testSlideLinkList()
{
    try
    {
        // The target presentation has a single slide and links to nothing.
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("insert-delete.odp", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = load(socketPoll, documentURL);
        stageSource(socket, documentURL, "source.odp");

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), getLinks(socket)->size());

        // An insert names the source document, and the slides it takes from the staged file
        // are linked to it.
        helpers::sendTextFrame(socket, insertCommand("slides=0,1 at=0 keepdesign=0 link=1"),
                               testname);
        const std::string insertReply =
            helpers::getResponseString(socket, "slideimport:", testname);
        Poco::JSON::Object::Ptr insertObject = helpers::parseJsonReply(insertReply, "slideimport:");
        LOK_ASSERT_EQUAL(std::string("inserted"), insertObject->getValue<std::string>("status"));

        Poco::JSON::Array::Ptr links = getLinks(socket);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), links->size());
        const std::vector<std::string> parts = getParts(links->getObject(0), 2);
        LOK_ASSERT_MESSAGE("two linked pages must hold two parts", parts[0] != parts[1]);

        // A plain copy of the same slides is no link, so the list still holds
        // the two linked pages alone.
        stageSource(socket, documentURL, "source.odp");
        helpers::sendTextFrame(socket, insertCommand("slides=0,1 keepdesign=0 link=0"), testname);
        helpers::getResponseString(socket, "slideimport:", testname);
        getParts(getLinks(socket)->getObject(0), 2);

        socketPoll->joinThread();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSlideLink::testSlideLinkRefresh()
{
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("insert-delete.odp", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = load(socketPoll, documentURL);
        stageSource(socket, documentURL, "source.odp");

        helpers::sendTextFrame(socket, insertCommand("slides=0,1 at=0 keepdesign=0 link=1"),
                               testname);
        helpers::getResponseString(socket, "slideimport:", testname);

        const std::vector<std::string> parts = getParts(getLinks(socket)->getObject(0), 2);

        // wsd stages the slides of the source as a file of the jail and has the
        // kit read the pages of that source from it. The staged name holds a
        // space, so the encoding of the name is exercised as well.
        stageSource(socket, documentURL, "Q3 staged.odp");

        sendAsWsd(std::string("slidelink update source=") + EncodedSourceName +
                  " file=Q3%20staged.odp");

        // The refreshed pages are the pages read for them, so the reply carries
        // the list they hold now before it reports how many were refreshed.
        const std::string listReply = helpers::getResponseString(socket, "slidelinks:", testname);
        Poco::JSON::Object::Ptr listObject = helpers::parseJsonReply(listReply, "slidelinks:");
        const std::vector<std::string> refreshedParts =
            getParts(listObject->getArray("links")->getObject(0), 2);
        LOK_ASSERT_MESSAGE("a refreshed page holds a new part", refreshedParts[0] != parts[0]);
        LOK_ASSERT_MESSAGE("a refreshed page holds a new part", refreshedParts[1] != parts[1]);

        const std::string reply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr replyObject = helpers::parseJsonReply(reply, "slidelink:");
        LOK_ASSERT_EQUAL(std::string("updated"), replyObject->getValue<std::string>("status"));
        LOK_ASSERT_EQUAL(std::string(SourceName), replyObject->getValue<std::string>("source"));
        LOK_ASSERT_EQUAL(2, replyObject->getValue<int>("count"));

        // Every page was read in place of the one it refreshed, so the deck is
        // the size it was.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), helpers::getPartCount(socket, testname));

        // The staged file was read for that one refresh and is gone, so asking
        // for it again fails instead of reading it a second time.
        assertErrorAsWsd(socket, std::string("slidelink update source=") + EncodedSourceName +
                                     " file=Q3%20staged.odp",
                         "failed", std::string(" source=") + EncodedSourceName);

        socketPoll->joinThread();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSlideLink::testSlideLinkBreak()
{
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("insert-delete.odp", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = load(socketPoll, documentURL);
        stageSource(socket, documentURL, "source.odp");

        helpers::sendTextFrame(socket, insertCommand("slides=0,1 at=0 keepdesign=0 link=1"),
                               testname);
        helpers::getResponseString(socket, "slideimport:", testname);

        const std::vector<std::string> parts = getParts(getLinks(socket)->getObject(0), 2);

        // Taking the source off the first of the two linked pages is a command a client sends.
        helpers::sendTextFrame(socket, "slidelink break part=" + parts[0], testname);
        const std::string reply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr replyObject = helpers::parseJsonReply(reply, "slidelink:");
        LOK_ASSERT_EQUAL(std::string("broken"), replyObject->getValue<std::string>("status"));
        LOK_ASSERT_EQUAL(parts[0], replyObject->getValue<std::string>("part"));

        // The pages linked to a source are not what they were, so the kit sends the fresh list
        // without being asked: the page that kept its source is the only one left.
        const std::string listReply = helpers::getResponseString(socket, "slidelinks:", testname);
        Poco::JSON::Object::Ptr listObject = helpers::parseJsonReply(listReply, "slidelinks:");
        const std::vector<std::string> leftParts =
            getParts(listObject->getArray("links")->getObject(0), 1);
        LOK_ASSERT_EQUAL(parts[1], leftParts[0]);

        // The page kept the content it holds, so the deck is the size it was.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), helpers::getPartCount(socket, testname));

        // A page that belongs to no source has no source to take off, and a part that names no
        // page of the document is the same matter.
        helpers::assertErrorReply(socket, "slidelink break part=" + parts[0], "slidelink",
                                  "notlinked", testname, " part=" + parts[0]);
        helpers::assertErrorReply(socket, "slidelink break part=999999", "slidelink", "notlinked",
                                  testname, " part=999999");

        // A break names one page, and it names it with a part identifier.
        helpers::assertErrorReply(socket, "slidelink break", "slidelink", "syntax", testname);
        helpers::assertErrorReply(socket, "slidelink break part=zero", "slidelink", "syntax",
                                  testname);
        helpers::assertErrorReply(socket, "slidelink break part={1BE1A269}", "slidelink", "syntax",
                                  testname);

        socketPoll->joinThread();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSlideLink::testSlideLinkErrors()
{
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("insert-delete.odp", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = load(socketPoll, documentURL);
        stageSource(socket, documentURL, "source.odp");

        // A source is the name of a document, so a name holding a path and a
        // name holding a control character are both refused.
        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0", /*named=*/false) +
                                      " source=deck%2Fsales.odp",
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0", /*named=*/false) +
                                      " source=deck%01.odp",
                                  "slideimport", "syntax", testname);

        // An option is given once, and an insert carries no option of its own.
        helpers::assertErrorReply(socket, insertCommand("slides=0") + " file=other.odp",
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket, insertCommand("slides=0 bogus=1"),
                                  "slideimport", "syntax", testname);

        // Without a source there is nothing to link the inserted slides to.
        helpers::assertErrorReply(socket, insertCommand("slides=0 link=1", /*named=*/false),
                                  "slideimport", "nosource", testname);

        // An update names a file the server staged, so one naming a file that was never
        // staged reaches nothing.
        helpers::assertErrorReply(socket,
                                  std::string("slidelink update source=") + EncodedSourceName +
                                      " file=staged.odp",
                                  "slidelink", "failed", testname,
                                  std::string(" source=") + EncodedSourceName);
        helpers::assertErrorReply(socket, "slidelink bogus", "slidelink", "syntax", testname);
        helpers::assertErrorReply(socket, "slidelink", "slidelink", "syntax", testname);

        // A source no page of the document is linked to is a different matter
        // from a file the kit could not read, so this update gets a staged
        // file of its own.
        stageSource(socket, documentURL, "staged.odp");
        assertErrorAsWsd(socket, "slidelink update source=Other%20deck.odp file=staged.odp",
                         "notlinked", " source=Other%20deck.odp");

        // That file left the staging area with the refresh it was staged for,
        // refused as it was, so the same name reaches nothing now.
        assertErrorAsWsd(socket, "slidelink update source=Other%20deck.odp file=staged.odp",
                         "failed", " source=Other%20deck.odp");
        assertErrorAsWsd(socket, std::string("slidelink update source=") + EncodedSourceName +
                                     " file=nosuchfile.odp",
                         "failed", std::string(" source=") + EncodedSourceName);
        assertErrorAsWsd(socket, "slidelink update source=deck.odp", "syntax");

        // None of the failures changed the document, which still holds one
        // slide and no link at all.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), getLinks(socket)->size());

        socketPoll->joinThread();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSlideLink::testReadOnly()
{
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("insert-delete.odp", documentPath, documentURL, testname);

        // Request a readonly session by adding the permission to the document
        // URI part of the URL ("%3F" and "%3D" are the encoded "?" and "=").
        std::string readOnlyURL = documentURL;
        readOnlyURL.insert(readOnlyURL.rfind("/ws"), "%3Fpermission%3Dreadonly");

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = load(socketPoll, readOnlyURL);
        stageSource(socket, readOnlyURL, "staged.odp");

        // A readonly view reads the links its document holds...
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), getLinks(socket)->size());

        // ...but refreshing them is an edit, so the staged file is left unread
        // rather than reported as a source no page is linked to.
        assertErrorAsWsd(socket, std::string("slidelink update source=") + EncodedSourceName +
                                     " file=staged.odp",
                         "failed", std::string(" source=") + EncodedSourceName);

        // Taking the source off a page is an edit as well.
        helpers::assertErrorReply(socket, "slidelink break part=1", "slidelink", "failed",
                                  testname, " part=1");

        socketPoll->joinThread();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

void UnitSlideLink::invokeWSDTest()
{
    UnitBase::TestResult result = testSlideLinkList();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testSlideLinkRefresh();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testSlideLinkBreak();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testSlideLinkErrors();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testReadOnly();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitSlideLink(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
