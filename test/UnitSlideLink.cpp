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
 * Integration test for the slidelink command family: insert slides and a section as links to
 * the source document they came from, read the links the document holds, refresh them from a
 * file staged in the jail, the way wsd hands the kit the content of a source, and take a link
 * off. Also covers the error kinds and what a client may ask for itself.
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
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <chrono>
#include <fstream>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/// The source document as the user knows it. The characters that a reference
/// escapes are in the name, so that the escaping is exercised end to end.
static constexpr auto SourceName = "Q3 #1 100%.odp";
/// SourceName as it travels on the wire.
static constexpr auto EncodedSourceName = "Q3%20%231%20100%25.odp";
/// An identifier that no slide and no section of the documents of the test holds, as it travels
/// on the wire.
static constexpr auto EncodedUnknownId = "%7B00000000-0000-4000-8000-000000000000%7D";

class UnitSlideLink : public UnitWSD
{
    TestResult testSlideLinkList();
    TestResult testSlideLinkSection();
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

    /// Stages a ten-slide presentation in the jail of the loaded document under the given name.
    /// An insert takes the file it names, so every insert is staged for.
    void stageSource(const std::shared_ptr<http::WebSocketSession>& socket,
                     const std::string& stagedName)
    {
        // The kit answers getchildid in turn, so the answer comes after the kit is done with
        // the file staged before, and after every message sent earlier on the socket.
        helpers::sendTextFrame(socket, "getchildid", testname);
        helpers::getResponseString(socket, "getchildid:", testname);

        std::shared_ptr<ClientSession> session;
        {
            std::lock_guard<std::mutex> lock(_sessionMutex);
            session = _session.lock();
        }
        LOK_ASSERT_MESSAGE("no session to stage [" + stagedName + "] for", session);

        const std::shared_ptr<DocumentBroker> docBroker = session->getDocumentBroker();
        LOK_ASSERT_MESSAGE("the session has no document broker", docBroker);

        const std::string content = helpers::readFileAsString("slide-link-source.odp");
        auto staged = std::make_shared<std::promise<bool>>();
        docBroker->addCallback(
            [docBroker, stagedName, content, staged]()
            {
                const std::string stagedPath = docBroker->jailStagingPath(stagedName);
                if (stagedPath.empty())
                {
                    staged->set_value(false);
                    return;
                }

                Poco::File(Poco::Path(stagedPath).parent()).createDirectories();
                std::ofstream file(stagedPath, std::ios::binary);
                file << content;
                file.close();
                staged->set_value(static_cast<bool>(file));
            });

        std::future<bool> result = staged->get_future();
        LOK_ASSERT_MESSAGE("staging [" + stagedName + "] timed out",
                           result.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
        LOK_ASSERT_MESSAGE("failed to stage [" + stagedName + ']', result.get());
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

    /// The parts of the linked slides a link list holds, in the order it reports them, checking
    /// the source and the slide names on the way.
    std::vector<std::string> getParts(const Poco::JSON::Array::Ptr& links,
                                      std::size_t expectedSlides)
    {
        LOK_ASSERT_MESSAGE("missing link list", links);

        std::vector<std::string> parts;
        for (std::size_t i = 0; i < links->size(); ++i)
        {
            Poco::JSON::Object::Ptr link = links->getObject(i);
            LOK_ASSERT_MESSAGE("missing link entry", link);
            if (link->getValue<std::string>("unit") != "slide")
                continue;

            LOK_ASSERT_EQUAL(std::string(SourceName), link->getValue<std::string>("source"));
            // The slides of the source carry no name of their own, so a page of
            // one of them names a slide only when the insert asked to keep the
            // name that slide holds by its position.
            LOK_ASSERT_MESSAGE("a linked page must report a slide name entry", link->has("name"));
            const std::string part = link->getValue<std::string>("id");
            LOK_ASSERT_MESSAGE("a linked page must report a part", !part.empty());
            parts.push_back(part);
        }
        LOK_ASSERT_EQUAL(expectedSlides, parts.size());

        return parts;
    }

    /// The entries of a link list of the given unit, "slide" or "section".
    std::vector<Poco::JSON::Object::Ptr> getLinksOfUnit(const Poco::JSON::Array::Ptr& links,
                                                        const std::string& unit)
    {
        LOK_ASSERT_MESSAGE("missing link list", links);
        std::vector<Poco::JSON::Object::Ptr> entries;
        for (std::size_t i = 0; i < links->size(); ++i)
        {
            Poco::JSON::Object::Ptr link = links->getObject(i);
            LOK_ASSERT_MESSAGE("missing link entry", link);
            if (link->getValue<std::string>("unit") == unit)
                entries.push_back(link);
        }
        return entries;
    }

    /// A braced identifier, as the link list reports one, the way it travels on the wire.
    static std::string encodeId(const std::string& id)
    {
        return "%7B" + id.substr(1, id.size() - 2) + "%7D";
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
        stageSource(socket, "source.odp");

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), getLinks(socket)->size());

        // An insert names the source document, and the slides it takes from the staged file
        // are linked to it. It also names the time the source was last modified.
        helpers::sendTextFrame(
            socket, insertCommand("slides=0,1 at=0 keepdesign=0 link=1 time=2021-05-05T10:00:00Z"),
            testname);
        const std::string insertReply =
            helpers::getResponseString(socket, "slideimport:", testname);
        Poco::JSON::Object::Ptr insertObject = helpers::parseJsonReply(insertReply, "slideimport:");
        LOK_ASSERT_EQUAL(std::string("inserted"), insertObject->getValue<std::string>("status"));
        LOK_ASSERT_EQUAL(2, insertObject->getValue<int>("count"));

        Poco::JSON::Array::Ptr links = getLinks(socket);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), links->size());
        const std::vector<std::string> parts = getParts(links, 2);
        LOK_ASSERT_MESSAGE("two linked pages must hold two parts", parts[0] != parts[1]);

        // Each linked page reports the source time the insert named.
        for (std::size_t i = 0; i < links->size(); ++i)
        {
            LOK_ASSERT_EQUAL(std::string("2021-05-05T10:00:00Z"),
                             links->getObject(i)->getValue<std::string>("lastModifiedTime"));
            LOK_ASSERT_EQUAL(std::string(), links->getObject(i)->getValue<std::string>("name"));
        }

        // A plain copy of the same slides is no link, so the list still holds
        // the two linked pages alone.
        stageSource(socket, "source.odp");
        helpers::sendTextFrame(socket, insertCommand("slides=0,1 keepdesign=0 link=0"), testname);
        helpers::getResponseString(socket, "slideimport:", testname);
        getParts(getLinks(socket), 2);

        // An insert that names the position a slide holds in the source links the page to that
        // position, which is what a source keeping no slide identifiers can be matched by later.
        stageSource(socket, "source.odp");
        helpers::sendTextFrame(
            socket, insertCommand("slides=2 at=0 keepdesign=0 link=1 sourcepositions=3"), testname);
        helpers::getResponseString(socket, "slideimport:", testname);
        Poco::JSON::Array::Ptr named = getLinks(socket);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), named->size());
        LOK_ASSERT_MESSAGE("a page linked to a position must report the name of that position",
                           !named->getObject(0)->getValue<std::string>("name").empty());
        LOK_ASSERT_MESSAGE("a page linked to a position must record the position and no slide "
                           "identifier",
                           named->getObject(0)->getValue<std::string>("sourceId") ==
                               named->getObject(0)->getValue<std::string>("name"));

        // An insert that names no slides takes every page of the staged file, and reports how
        // many pages it added.
        stageSource(socket, "source.odp");
        helpers::sendTextFrame(socket, insertCommand("at=0 keepdesign=0 link=0"), testname);
        const std::string wholeReply = helpers::getResponseString(socket, "slideimport:", testname);
        Poco::JSON::Object::Ptr wholeObject = helpers::parseJsonReply(wholeReply, "slideimport:");
        LOK_ASSERT_EQUAL(std::string("inserted"), wholeObject->getValue<std::string>("status"));
        LOK_ASSERT_EQUAL(10, wholeObject->getValue<int>("count"));

        // A source is a persistent link the storage resolves.
        stageSource(socket, "source.odp");
        helpers::sendTextFrame(socket,
                               insertCommand("slides=0 at=0 keepdesign=0 link=1", /*named=*/false) +
                                   " source=https%3A%2F%2Fstorage.example%2Ffiles%2F42",
                               testname);
        helpers::getResponseString(socket, "slideimport:", testname);
        Poco::JSON::Array::Ptr linked = getLinks(socket);
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(4), linked->size());
        bool foundAddress = false;
        for (std::size_t i = 0; i < linked->size(); ++i)
        {
            if (linked->getObject(i)->getValue<std::string>("source") ==
                "https://storage.example/files/42")
                foundAddress = true;
        }
        LOK_ASSERT_MESSAGE("the link list must report the persistent link as recorded",
                           foundAddress);

        socketPoll->joinThread();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSlideLink::testSlideLinkSection()
{
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("insert-delete.odp", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = load(socketPoll, documentURL);
        stageSource(socket, "source.odp");

        // Slides inserted with the section of the source they make up go in as a linked section.
        helpers::sendTextFrame(
            socket,
            insertCommand("slides=0,1 at=0 keepdesign=0 link=1 time=2021-05-05T10:00:00Z "
                          "as=section id=%7BD2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21%7D "
                          "name=Opening%20slides"),
            testname);
        const std::string insertReply =
            helpers::getResponseString(socket, "slideimport:", testname);
        Poco::JSON::Object::Ptr insertObject = helpers::parseJsonReply(insertReply, "slideimport:");
        LOK_ASSERT_EQUAL(2, insertObject->getValue<int>("count"));

        // The section comes with the two slides it spans, each linked under it.
        Poco::JSON::Array::Ptr links = getLinks(socket);
        std::vector<Poco::JSON::Object::Ptr> sections = getLinksOfUnit(links, "section");
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), sections.size());
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), links->size());

        Poco::JSON::Object::Ptr section = sections[0];
        LOK_ASSERT_EQUAL(std::string(SourceName), section->getValue<std::string>("source"));
        LOK_ASSERT_EQUAL(std::string("{D2A46A4F-3C82-4E7B-A6B3-9F1E5C0D7A21}"),
                         section->getValue<std::string>("sourceId"));
        LOK_ASSERT_EQUAL(std::string("Opening slides"), section->getValue<std::string>("name"));
        LOK_ASSERT_EQUAL(std::string("2021-05-05T10:00:00Z"),
                         section->getValue<std::string>("lastModifiedTime"));
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), section->getArray("slides")->size());
        const std::string sectionId = section->getValue<std::string>("id");
        for (const Poco::JSON::Object::Ptr& slide : getLinksOfUnit(links, "slide"))
            LOK_ASSERT_EQUAL(sectionId, slide->getValue<std::string>("parent"));

        // An update of the section gives it every slide of the staged file, which holds the
        // slides its source section holds now, and the new name of that source section.
        const std::string encodedSectionId = encodeId(sectionId);
        stageSource(socket, "staged.odp");
        sendAsWsd(std::string("slidelink update source=") + EncodedSourceName +
                  " file=staged.odp time=2021-06-06T10:00:00Z unit=section id=" +
                  encodedSectionId + " name=Opening%20slides%2C%20revised");
        const std::string updateListReply =
            helpers::getResponseString(socket, "slidelinks:", testname);
        sections = getLinksOfUnit(
            helpers::parseJsonReply(updateListReply, "slidelinks:")->getArray("links"), "section");
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), sections.size());
        section = sections[0];
        LOK_ASSERT_EQUAL(sectionId, section->getValue<std::string>("id"));
        LOK_ASSERT_EQUAL(std::string("Opening slides, revised"),
                         section->getValue<std::string>("name"));
        LOK_ASSERT_EQUAL(std::string("2021-06-06T10:00:00Z"),
                         section->getValue<std::string>("lastModifiedTime"));
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(10), section->getArray("slides")->size());

        const std::string updateReply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr updateObject = helpers::parseJsonReply(updateReply, "slidelink:");
        LOK_ASSERT_EQUAL(std::string("section"), updateObject->getValue<std::string>("unit"));
        LOK_ASSERT_EQUAL(sectionId, updateObject->getValue<std::string>("id"));
        LOK_ASSERT_EQUAL(10, updateObject->getValue<int>("count"));
        // The file listed the section, so no slide of it was left as it was.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0),
                         updateObject->getArray("notUpdated")->size());

        // An update of the whole source names no unit and no name. The section follows the name
        // the file lists for its source section, which is the name it had before the rename, and
        // the reply names no unit.
        stageSource(socket, "staged.odp");
        sendAsWsd(std::string("slidelink update source=") + EncodedSourceName +
                  " file=staged.odp time=2021-07-07T10:00:00Z");
        const std::string sourceListReply =
            helpers::getResponseString(socket, "slidelinks:", testname);
        sections = getLinksOfUnit(
            helpers::parseJsonReply(sourceListReply, "slidelinks:")->getArray("links"), "section");
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), sections.size());
        section = sections[0];
        LOK_ASSERT_EQUAL(sectionId, section->getValue<std::string>("id"));
        LOK_ASSERT_EQUAL(std::string("Opening slides"), section->getValue<std::string>("name"));
        LOK_ASSERT_EQUAL(std::string("2021-07-07T10:00:00Z"),
                         section->getValue<std::string>("lastModifiedTime"));
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(10), section->getArray("slides")->size());
        const std::string sourceReply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr sourceObject = helpers::parseJsonReply(sourceReply, "slidelink:");
        LOK_ASSERT_EQUAL(10, sourceObject->getValue<int>("count"));
        LOK_ASSERT_MESSAGE("an update of the whole source names no unit",
                           !sourceObject->has("unit"));
        LOK_ASSERT_MESSAGE("an update of the whole source names no id", !sourceObject->has("id"));

        // A section that is not linked has no source section to follow.
        stageSource(socket, "staged.odp");
        assertErrorAsWsd(socket,
                         std::string("slidelink update source=") + EncodedSourceName +
                             " file=staged.odp unit=section id=" + EncodedUnknownId,
                         "notlinked",
                         std::string(" source=") + EncodedSourceName +
                             " unit=section id=" + EncodedUnknownId);

        // Taking the link off the section, and off each of its slides, leaves no link at all.
        // deep= may come before the unit.
        helpers::sendTextFrame(socket, "slidelink break deep=1 unit=section id=" + encodedSectionId,
                               testname);
        const std::string breakReply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr breakObject = helpers::parseJsonReply(breakReply, "slidelink:");
        LOK_ASSERT_EQUAL(std::string("broken"), breakObject->getValue<std::string>("status"));
        LOK_ASSERT_EQUAL(std::string("section"), breakObject->getValue<std::string>("unit"));
        LOK_ASSERT_EQUAL(sectionId, breakObject->getValue<std::string>("id"));

        const std::string breakListReply =
            helpers::getResponseString(socket, "slidelinks:", testname);
        Poco::JSON::Object::Ptr breakList =
            helpers::parseJsonReply(breakListReply, "slidelinks:");
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), breakList->getArray("links")->size());

        // The section is linked to nothing now, a break names a section by a GUID, and deep is
        // 0 or 1.
        helpers::assertErrorReply(socket, "slidelink break unit=section id=" + encodedSectionId,
                                  "slidelink", "notlinked", testname,
                                  " unit=section id=" + encodedSectionId);
        helpers::assertErrorReply(socket, "slidelink break unit=section id=opening", "slidelink",
                                  "syntax", testname, " unit=section id=opening");
        helpers::assertErrorReply(
            socket, "slidelink break unit=section id=" + encodedSectionId + " deep=all",
            "slidelink", "syntax", testname, " unit=section id=" + encodedSectionId);

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
        stageSource(socket, "source.odp");

        helpers::sendTextFrame(socket, insertCommand("slides=0,1 at=0 keepdesign=0 link=1"),
                               testname);
        helpers::getResponseString(socket, "slideimport:", testname);

        const std::vector<std::string> insertedParts = getParts(getLinks(socket), 2);

        // An update that names one slide reads that page alone: it holds a new
        // part afterwards and the other page of the source keeps the one it has.
        stageSource(socket, "one.odp");
        sendAsWsd(std::string("slidelink update source=") + EncodedSourceName +
                  " file=one.odp unit=slide id=" + encodeId(insertedParts[1]));
        const std::string oneListReply =
            helpers::getResponseString(socket, "slidelinks:", testname);
        const std::vector<std::string> parts =
            getParts(helpers::parseJsonReply(oneListReply, "slidelinks:")->getArray("links"), 2);
        LOK_ASSERT_EQUAL(insertedParts[0], parts[0]);
        LOK_ASSERT_MESSAGE("the refreshed page holds a new part", parts[1] != insertedParts[1]);
        const std::string oneReply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr oneObject = helpers::parseJsonReply(oneReply, "slidelink:");
        LOK_ASSERT_EQUAL(1, oneObject->getValue<int>("count"));
        // The reply names the unit as the command did, by the part the page held then.
        LOK_ASSERT_EQUAL(std::string("slide"), oneObject->getValue<std::string>("unit"));
        LOK_ASSERT_EQUAL(insertedParts[1], oneObject->getValue<std::string>("id"));

        // A slide that is not linked to the source is refreshed from nothing, and
        // the staged file goes with the answer.
        stageSource(socket, "none.odp");
        assertErrorAsWsd(socket,
                         std::string("slidelink update source=") + EncodedSourceName +
                             " file=none.odp unit=slide id=" + EncodedUnknownId,
                         "notlinked",
                         std::string(" source=") + EncodedSourceName +
                             " unit=slide id=" + EncodedUnknownId);

        // wsd stages the slides of the source as a file of the jail and has the
        // kit read the pages of that source from it. The staged name holds a
        // space, so the encoding of the name is exercised as well.
        stageSource(socket, "Q3 staged.odp");

        sendAsWsd(std::string("slidelink update source=") + EncodedSourceName +
                  " file=Q3%20staged.odp");

        // The refreshed pages are the pages read for them, so the reply carries
        // the list they hold now before it reports how many were refreshed.
        const std::string listReply = helpers::getResponseString(socket, "slidelinks:", testname);
        Poco::JSON::Object::Ptr listObject = helpers::parseJsonReply(listReply, "slidelinks:");
        const std::vector<std::string> refreshedParts = getParts(listObject->getArray("links"), 2);
        LOK_ASSERT_MESSAGE("a refreshed page holds a new part", refreshedParts[0] != parts[0]);
        LOK_ASSERT_MESSAGE("a refreshed page holds a new part", refreshedParts[1] != parts[1]);

        const std::string reply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr replyObject = helpers::parseJsonReply(reply, "slidelink:");
        LOK_ASSERT_EQUAL(std::string("updated"), replyObject->getValue<std::string>("status"));
        LOK_ASSERT_EQUAL(std::string(SourceName), replyObject->getValue<std::string>("source"));
        LOK_ASSERT_EQUAL(2, replyObject->getValue<int>("count"));
        // Both pages were read, so the reply names no slide the source held no slide for.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0),
                         replyObject->getArray("notUpdated")->size());

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
        stageSource(socket, "source.odp");

        helpers::sendTextFrame(socket, insertCommand("slides=0,1 at=0 keepdesign=0 link=1"),
                               testname);
        helpers::getResponseString(socket, "slideimport:", testname);

        const std::vector<std::string> parts = getParts(getLinks(socket), 2);

        // Taking the link off the first of the two linked pages is a command a client sends.
        helpers::sendTextFrame(socket, "slidelink break unit=slide id=" + encodeId(parts[0]),
                               testname);
        const std::string reply = helpers::getResponseString(socket, "slidelink:", testname);
        Poco::JSON::Object::Ptr replyObject = helpers::parseJsonReply(reply, "slidelink:");
        LOK_ASSERT_EQUAL(std::string("broken"), replyObject->getValue<std::string>("status"));
        LOK_ASSERT_EQUAL(std::string("slide"), replyObject->getValue<std::string>("unit"));
        LOK_ASSERT_EQUAL(parts[0], replyObject->getValue<std::string>("id"));

        // The pages linked to a source are not what they were, so the kit sends the fresh list
        // without being asked: the page that kept its source is the only one left.
        const std::string listReply = helpers::getResponseString(socket, "slidelinks:", testname);
        Poco::JSON::Object::Ptr listObject = helpers::parseJsonReply(listReply, "slidelinks:");
        const std::vector<std::string> leftParts = getParts(listObject->getArray("links"), 1);
        LOK_ASSERT_EQUAL(parts[1], leftParts[0]);

        // The page kept the content it holds, so the deck is the size it was.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), helpers::getPartCount(socket, testname));

        // A page that is linked to nothing has no link to take off, and an identifier that names
        // no page of the document is the same matter.
        helpers::assertErrorReply(socket, "slidelink break unit=slide id=" + encodeId(parts[0]),
                                  "slidelink", "notlinked", testname,
                                  " unit=slide id=" + encodeId(parts[0]));
        helpers::assertErrorReply(socket, std::string("slidelink break unit=slide id=") +
                                              EncodedUnknownId,
                                  "slidelink", "notlinked", testname,
                                  std::string(" unit=slide id=") + EncodedUnknownId);

        // A break names one unit, by a unit of a known kind and a GUID together, and the answer
        // names them as the command spelled them once it carries both.
        helpers::assertErrorReply(socket, "slidelink break", "slidelink", "syntax", testname);
        helpers::assertErrorReply(socket, "slidelink break unit=slide id=zero", "slidelink",
                                  "syntax", testname, " unit=slide id=zero");
        helpers::assertErrorReply(socket, "slidelink break unit=slide id=%7B1BE1A269%7D",
                                  "slidelink", "syntax", testname,
                                  " unit=slide id=%7B1BE1A269%7D");
        helpers::assertErrorReply(socket, std::string("slidelink break unit=page id=") +
                                              EncodedUnknownId,
                                  "slidelink", "syntax", testname,
                                  std::string(" unit=page id=") + EncodedUnknownId);
        helpers::assertErrorReply(socket, std::string("slidelink break id=") + EncodedUnknownId,
                                  "slidelink", "syntax", testname);
        helpers::assertErrorReply(socket, "slidelink break unit=slide", "slidelink", "syntax",
                                  testname);
        helpers::assertErrorReply(socket, std::string("slidelink break unit=slide id=") +
                                              EncodedUnknownId + " time=now",
                                  "slidelink", "syntax", testname,
                                  std::string(" unit=slide id=") + EncodedUnknownId);

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
        stageSource(socket, "source.odp");

        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0", /*named=*/false) +
                                      " source=deck%01.odp",
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0", /*named=*/false) + " source=" +
                                      std::string(2000, 'x'),
                                  "slideimport", "syntax", testname);

        // The time is recorded on the pages the insert links and is written out with them, so a
        // time holding a control character, and one no storage reports, are refused as well.
        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0 time=2021-05-05T10%3A00%3A00Z%01"),
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0 time=" + std::string(200, 'x')),
                                  "slideimport", "syntax", testname);

        // An option is given once, and an insert carries no option of its own.
        helpers::assertErrorReply(socket, insertCommand("slides=0") + " file=other.odp",
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket, insertCommand("slides=0 bogus=1"),
                                  "slideimport", "syntax", testname);

        // Without a source there is nothing to link the inserted slides to.
        helpers::assertErrorReply(socket, insertCommand("slides=0 link=1", /*named=*/false),
                                  "slideimport", "nosource", testname);

        // The pages go in as slides or as a section. A section is named by the identifier of its
        // source section, a GUID, and by its name, which is recorded like the source time,
        // linked or not, and a slides insert names no unit. A linked section reads each slide by
        // the identifier of its source slide, so a section takes no positions.
        const std::string section = std::string("as=section id=") + EncodedUnknownId;
        helpers::assertErrorReply(socket, insertCommand("slides=0 as=deck"), "slideimport",
                                  "syntax", testname);
        helpers::assertErrorReply(socket, insertCommand("slides=0 " + section + " name=a%01"),
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0 as=section id=opening name=Opening"),
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket, insertCommand("slides=0 " + section), "slideimport",
                                  "syntax", testname);
        helpers::assertErrorReply(socket, insertCommand("slides=0 as=section name=Opening"),
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(
            socket, insertCommand("slides=0 link=1 sourcepositions=1 " + section + " name=Opening"),
            "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket,
                                  insertCommand("slides=0 link=1 " + section + " name=a%01"),
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket, insertCommand("slides=0 link=1 name=Opening"),
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket,
                                  insertCommand(std::string("slides=0 link=1 id=") +
                                                EncodedUnknownId),
                                  "slideimport", "syntax", testname);

        // A file is staged for one insert, and an insert the document cannot make sense of
        // takes it all the same, so the insert that follows reaches nothing.
        stageSource(socket, "source.odp");
        helpers::assertErrorReply(socket, insertCommand("slides=0 bogus=1"),
                                  "slideimport", "syntax", testname);
        helpers::assertErrorReply(socket, insertCommand("slides=0"),
                                  "slideimport", "cantload", testname);

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
        stageSource(socket, "staged.odp");
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

        // A source holding a control character is refused, and the file staged for that
        // refresh goes with it: the refresh that follows reaches nothing.
        stageSource(socket, "staged.odp");
        assertErrorAsWsd(socket, "slidelink update source=deck%01.odp file=staged.odp",
                         "syntax", " source=deck%01.odp");

        // A refresh records the time it names on the pages it reads, so a time of that shape is
        // refused the same way, and the file staged for it goes with the refresh it was staged
        // for.
        stageSource(socket, "staged.odp");
        assertErrorAsWsd(socket, std::string("slidelink update source=") + EncodedSourceName +
                                     " file=staged.odp time=2021-05-05T10%3A00%3A00Z%01",
                         "syntax", std::string(" source=") + EncodedSourceName);
        assertErrorAsWsd(socket, std::string("slidelink update source=") + EncodedSourceName +
                                     " file=staged.odp",
                         "failed", std::string(" source=") + EncodedSourceName);

        // An update names a unit by unit= and id= together, a unit of a known kind and a GUID, a
        // name goes with a unit, and deep is a matter of a break. The answers name the unit as
        // well once the command carries both tokens.
        const std::string update =
            std::string("slidelink update source=") + EncodedSourceName + " file=staged.odp";
        const std::string namedSource = std::string(" source=") + EncodedSourceName;
        assertErrorAsWsd(socket, update + " deep=1 unit=section id=" + EncodedUnknownId, "syntax",
                         namedSource + " unit=section id=" + EncodedUnknownId);
        assertErrorAsWsd(socket, update + " unit=section id=", "syntax",
                         namedSource + " unit=section id=");
        assertErrorAsWsd(socket, update + " name=Opening", "syntax", namedSource);
        assertErrorAsWsd(socket, update + " unit=section id=Opening", "syntax",
                         namedSource + " unit=section id=Opening");
        assertErrorAsWsd(socket, update + " unit=section", "syntax", namedSource);
        assertErrorAsWsd(socket, update + " id=" + EncodedUnknownId, "syntax", namedSource);
        assertErrorAsWsd(socket, update + " unit=deck id=" + EncodedUnknownId, "syntax",
                         namedSource + " unit=deck id=" + EncodedUnknownId);
        // A token that comes twice is refused, and the answer names the value that came first.
        assertErrorAsWsd(socket,
                         update + " unit=slide id=" + EncodedUnknownId
                             + " id=%7B2F7A9C41-0000-4000-8000-000000000001%7D",
                         "syntax", namedSource + " unit=slide id=" + EncodedUnknownId);

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
        stageSource(socket, "staged.odp");

        // A readonly view reads the links its document holds...
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), getLinks(socket)->size());

        // ...but refreshing them is an edit, so the staged file is left unread
        // rather than reported as a source no page is linked to.
        assertErrorAsWsd(socket, std::string("slidelink update source=") + EncodedSourceName +
                                     " file=staged.odp",
                         "failed", std::string(" source=") + EncodedSourceName);

        // Taking the link off a slide is an edit as well.
        helpers::assertErrorReply(socket, std::string("slidelink break unit=slide id=") +
                                              EncodedUnknownId,
                                  "slidelink", "failed", testname,
                                  std::string(" unit=slide id=") + EncodedUnknownId);

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

    result = testSlideLinkSection();
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
