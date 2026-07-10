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

#include <config.h>

#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/McpRequests.hpp>
#include <wsd/McpTools.hpp>
#include <wsd/McpUploads.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <test/lokassert.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

/// Whitebox tests for the short-lived store of documents uploaded to the MCP endpoint. The store
/// takes the current time as an argument, so these tests drive its expiry without waiting.
class McpUploadsTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(McpUploadsTests);
    CPPUNIT_TEST(testStoredDocumentIsFoundByItsId);
    CPPUNIT_TEST(testUnknownIdIsNotFound);
    CPPUNIT_TEST(testRoomIsOnlyForWhatTheStoreHolds);
    CPPUNIT_TEST(testReservedRoomIsHeldUntilGivenBack);
    CPPUNIT_TEST(testDocumentIsGoneAfterItsTimeToLive);
    CPPUNIT_TEST(testExpiredDocumentIsRemovedFromDisk);
    CPPUNIT_TEST(testTakenDocumentIsHandedOutOnlyOnce);
    CPPUNIT_TEST(testDiscardedFileIsRemovedFromDisk);
    CPPUNIT_TEST(testAnIdentifierServesOnlyItsOwnKind);
    CPPUNIT_TEST(testStreamedDocumentIsStoredWhole);
    CPPUNIT_TEST(testShortStreamIsRefusedAndLeavesNothing);
    CPPUNIT_TEST(testLongNameKeepsItsExtension);
    CPPUNIT_TEST(testStoreHoldsABoundedNumberOfDocuments);
    CPPUNIT_TEST(testStoredFileIsKeptWholeAndSourceIsLeft);
    CPPUNIT_TEST(testUploadIsGivenTimeForItsSize);
    CPPUNIT_TEST(testUploadSinkReportsAFullStore);
    CPPUNIT_TEST_SUITE_END();

    void testStoredDocumentIsFoundByItsId();
    void testUnknownIdIsNotFound();
    void testRoomIsOnlyForWhatTheStoreHolds();
    void testReservedRoomIsHeldUntilGivenBack();
    void testDocumentIsGoneAfterItsTimeToLive();
    void testExpiredDocumentIsRemovedFromDisk();
    void testTakenDocumentIsHandedOutOnlyOnce();
    void testDiscardedFileIsRemovedFromDisk();
    void testAnIdentifierServesOnlyItsOwnKind();
    void testStreamedDocumentIsStoredWhole();
    void testShortStreamIsRefusedAndLeavesNothing();
    void testLongNameKeepsItsExtension();
    void testStoreHoldsABoundedNumberOfDocuments();
    void testStoredFileIsKeptWholeAndSourceIsLeft();
    void testUploadIsGivenTimeForItsSize();
    void testUploadSinkReportsAFullStore();

    std::string _savedChildRoot;

public:
    void setUp() override
    {
        // Every test starts from a store that holds no bytes, and leaves it that way.
        constexpr std::string_view testname = "McpUploadsTests";
        LOK_ASSERT(McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes));

        // Point the incoming directory at a fresh temporary tree so the store writes its files
        // there and leaves the real jail root alone.
        _savedChildRoot = COOLWSD::ChildRoot;
        COOLWSD::ChildRoot = FileUtil::createRandomTmpDir();
    }

    void tearDown() override
    {
        // Every document of this test has expired an hour from now, so the store is empty for the
        // next test, and the temporary tree goes with it.
        McpUploads::instance().purgeExpiredEntries(McpUploads::Clock::now() +
                                                   std::chrono::hours(1));
        FileUtil::removeFile(COOLWSD::ChildRoot, /*recursive=*/true);
        COOLWSD::ChildRoot = _savedChildRoot;
    }

    static bool fileExists(const std::string& path)
    {
        std::ifstream stream(path);
        return stream.good();
    }

    /// The number of entries in the directory that holds incoming documents.
    static std::size_t countIncomingEntries()
    {
        const std::filesystem::path incoming =
            COOLWSD::ChildRoot + JailUtil::CHILDROOT_TMP_INCOMING_PATH;
        std::error_code error;
        std::size_t count = 0;
        for (std::filesystem::directory_iterator it(incoming, error), end; !error && it != end;
             it.increment(error))
            ++count;
        return count;
    }

    /// Room counted against the store for documents about to be written. The room still counted is
    /// given back when this goes, so a test that stops at a failed assertion leaves the store as it
    /// found it.
    class HeldRoom
    {
    public:
        HeldRoom() = default;
        ~HeldRoom()
        {
            while (!_sizes.empty())
                release();
        }
        HeldRoom(const HeldRoom&) = delete;
        HeldRoom& operator=(const HeldRoom&) = delete;

        /// Count size bytes and one document against the store. Returns false when the store has no
        /// room, and then nothing is counted.
        bool reserve(std::size_t size,
                     McpUploads::Clock::time_point now = McpUploads::Clock::now())
        {
            if (!McpUploads::instance().reserve(size, now))
                return false;
            _sizes.push_back(size);
            return true;
        }

        /// Give back the room counted last.
        void release()
        {
            McpUploads::instance().release(_sizes.back());
            _sizes.pop_back();
        }

    private:
        std::vector<std::size_t> _sizes;
    };
};

void McpUploadsTests::testStoredDocumentIsFoundByItsId()
{
    constexpr std::string_view testname = __func__;

    const auto now = McpUploads::Clock::now();
    const std::string id = McpUploads::instance().store(McpUploads::Kind::Upload, "hello.odt",
                                                       "some document bytes", now);
    LOK_ASSERT(!id.empty());

    const std::string path = McpUploads::instance().lookup(id, now);
    LOK_ASSERT(!path.empty());
    LOK_ASSERT(fileExists(path));
}

void McpUploadsTests::testUnknownIdIsNotFound()
{
    constexpr std::string_view testname = __func__;

    // An identifier that was never handed out finds nothing.
    LOK_ASSERT(McpUploads::instance().lookup("0123456789abcdef").empty());
}

void McpUploadsTests::testRoomIsOnlyForWhatTheStoreHolds()
{
    constexpr std::string_view testname = __func__;

    // A small document fits, and one larger than the whole store does not.
    LOK_ASSERT(McpUploads::instance().hasRoomFor(1024));
    LOK_ASSERT(!McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes + 1));
}

void McpUploadsTests::testReservedRoomIsHeldUntilGivenBack()
{
    constexpr std::string_view testname = __func__;

    // Two documents that each fit, but not together, cannot both be reserved.
    HeldRoom room;
    const std::size_t half = McpUploads::MaxStoredBytes / 2 + 1;
    LOK_ASSERT(room.reserve(half));
    LOK_ASSERT(!room.reserve(half));
    LOK_ASSERT(!McpUploads::instance().hasRoomFor(half));

    // Once the first is given back, the second fits.
    room.release();
    LOK_ASSERT(room.reserve(half));

    // Once that is given back too, the whole store is free again.
    room.release();
    LOK_ASSERT(McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes));
}

void McpUploadsTests::testDocumentIsGoneAfterItsTimeToLive()
{
    constexpr std::string_view testname = __func__;

    const auto now = McpUploads::Clock::now();
    const std::string id =
        McpUploads::instance().store(McpUploads::Kind::Upload, "hello.odt", "bytes", now);

    // Just before its time is up the document is still found.
    LOK_ASSERT(!McpUploads::instance().lookup(id, now + McpUploads::TimeToLive -
                                                      std::chrono::seconds(1))
                    .empty());

    // Just after its time is up it is gone.
    LOK_ASSERT(McpUploads::instance()
                   .lookup(id, now + McpUploads::TimeToLive + std::chrono::seconds(1))
                   .empty());
}

void McpUploadsTests::testExpiredDocumentIsRemovedFromDisk()
{
    constexpr std::string_view testname = __func__;

    const auto now = McpUploads::Clock::now();
    const std::string id =
        McpUploads::instance().store(McpUploads::Kind::Upload, "hello.odt", "bytes", now);
    const std::string path = McpUploads::instance().lookup(id, now);
    LOK_ASSERT(fileExists(path));

    // A later access after the document's time is up purges it, and its file no longer exists.
    LOK_ASSERT(McpUploads::instance()
                   .lookup("anything", now + McpUploads::TimeToLive + std::chrono::seconds(1))
                   .empty());
    LOK_ASSERT(!fileExists(path));
}

void McpUploadsTests::testTakenDocumentIsHandedOutOnlyOnce()
{
    constexpr std::string_view testname = __func__;

    const auto now = McpUploads::Clock::now();
    const std::string id =
        McpUploads::instance().store(McpUploads::Kind::Download, "result.pdf", "bytes", now);

    // Taking the document returns its path and leaves the file on disk to be served.
    bool alreadyTaken = true;
    const std::string path = McpUploads::instance().take(id, alreadyTaken, now);
    LOK_ASSERT(!path.empty());
    LOK_ASSERT(!alreadyTaken);
    LOK_ASSERT(fileExists(path));

    // A second take, and a lookup, find nothing: the document is handed out only once, and the
    // second take says it was taken before.
    LOK_ASSERT(McpUploads::instance().take(id, alreadyTaken, now).empty());
    LOK_ASSERT(alreadyTaken);
    LOK_ASSERT(McpUploads::instance().lookup(id, now).empty());

    // An identifier that was never issued is not one taken before.
    LOK_ASSERT(McpUploads::instance().take("0123456789abcdef", alreadyTaken, now).empty());
    LOK_ASSERT(!alreadyTaken);
}

void McpUploadsTests::testDiscardedFileIsRemovedFromDisk()
{
    constexpr std::string_view testname = __func__;

    const std::size_t before = countIncomingEntries();
    const auto now = McpUploads::Clock::now();
    const std::string id =
        McpUploads::instance().store(McpUploads::Kind::Download, "result.pdf", "bytes", now);
    bool alreadyTaken = false;
    const std::string path = McpUploads::instance().take(id, alreadyTaken, now);
    LOK_ASSERT(fileExists(path));

    // Discarding the served file removes it from disk, and the directory that held it with it.
    McpUploads::discardFile(path);
    LOK_ASSERT(!fileExists(path));
    LOK_ASSERT(!std::filesystem::exists(std::filesystem::path(path).parent_path()));
    LOK_ASSERT_EQUAL(before, countIncomingEntries());
}

void McpUploadsTests::testAnIdentifierServesOnlyItsOwnKind()
{
    constexpr std::string_view testname = __func__;

    const auto now = McpUploads::Clock::now();

    // An uploaded document is not a produced one, so it cannot be taken and stays available to be
    // looked up afterwards.
    const std::string uploadId =
        McpUploads::instance().store(McpUploads::Kind::Upload, "hello.odt", "bytes", now);
    bool alreadyTaken = false;
    LOK_ASSERT(McpUploads::instance().take(uploadId, alreadyTaken, now).empty());
    LOK_ASSERT(!McpUploads::instance().lookup(uploadId, now).empty());

    // A produced document is not an uploaded one, so it cannot be looked up and stays available to
    // be taken afterwards.
    const std::string downloadId =
        McpUploads::instance().store(McpUploads::Kind::Download, "result.pdf", "bytes", now);
    LOK_ASSERT(McpUploads::instance().lookup(downloadId, now).empty());
    LOK_ASSERT(!McpUploads::instance().take(downloadId, alreadyTaken, now).empty());
}

void McpUploadsTests::testStreamedDocumentIsStoredWhole()
{
    constexpr std::string_view testname = __func__;

    // The stored document is the stated number of bytes, and what follows in the stream is left.
    std::istringstream source("document bytes and what follows");
    const auto now = McpUploads::Clock::now();
    const std::string id =
        McpUploads::instance().store(McpUploads::Kind::Upload, "hello.odt", source, 14, now);
    LOK_ASSERT(!id.empty());

    std::string stored;
    FileUtil::readFile(McpUploads::instance().lookup(id, now), stored);
    LOK_ASSERT_EQUAL_STR("document bytes", stored);
}

void McpUploadsTests::testShortStreamIsRefusedAndLeavesNothing()
{
    constexpr std::string_view testname = __func__;

    // A stream that ends before the stated length stores nothing, and leaves no file or directory
    // behind.
    const std::size_t before = countIncomingEntries();
    std::istringstream source("short");
    LOK_ASSERT(
        McpUploads::instance().store(McpUploads::Kind::Upload, "hello.odt", source, 100).empty());
    LOK_ASSERT_EQUAL(before, countIncomingEntries());
}

void McpUploadsTests::testLongNameKeepsItsExtension()
{
    constexpr std::string_view testname = __func__;

    // A name over the longest allowed is cut before its extension, which it keeps.
    const std::string longName = McpTools::filenameOrDefault(std::string(300, 'a') + ".docx");
    LOK_ASSERT_EQUAL(McpTools::MaxFilenameBytes, longName.size());
    LOK_ASSERT(longName.ends_with("a.docx"));

    // A cut that would land inside a UTF-8 character moves back to the start of it.
    const std::string twoByteCharacter = "\xc3\xa9";
    const std::string accented = McpTools::filenameOrDefault(
        std::string(194, 'a') + twoByteCharacter + std::string(10, 'b') + ".docx");
    LOK_ASSERT_EQUAL(std::string(194, 'a') + ".docx", accented);

    // A name within the limit is kept as it is.
    LOK_ASSERT_EQUAL_STR("report.odt", McpTools::filenameOrDefault("report.odt"));
}

void McpUploadsTests::testStoreHoldsABoundedNumberOfDocuments()
{
    constexpr std::string_view testname = __func__;

    // A stored document and the documents being written together fill the store up to its number of
    // documents, and one more is refused, however small.
    const auto now = McpUploads::Clock::now();
    LOK_ASSERT(
        !McpUploads::instance().store(McpUploads::Kind::Upload, "one.odt", "bytes", now).empty());
    {
        HeldRoom room;
        for (std::size_t i = 1; i < McpUploads::MaxStoredEntries; ++i)
            LOK_ASSERT(room.reserve(1, now));
        LOK_ASSERT(!McpUploads::instance().hasRoomFor(1, now));
        LOK_ASSERT(!room.reserve(1, now));
        LOK_ASSERT(McpUploads::instance()
                       .store(McpUploads::Kind::Upload, "two.odt", "bytes", now)
                       .empty());

        // A document given back makes room for one more.
        room.release();
        LOK_ASSERT(McpUploads::instance().hasRoomFor(1, now));
        LOK_ASSERT(room.reserve(1, now));
    }

    // Once the documents being written are given back, the store has room again.
    LOK_ASSERT(McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes - 5, now));
}

void McpUploadsTests::testStoredFileIsKeptWholeAndSourceIsLeft()
{
    constexpr std::string_view testname = __func__;

    const std::string source = COOLWSD::ChildRoot + "/produced.pdf";
    {
        std::ofstream stream(source, std::ios::binary);
        stream << "produced bytes";
    }

    // The stored document holds the bytes of the file, and the file itself stays where it was.
    const auto now = McpUploads::Clock::now();
    const std::string id =
        McpUploads::instance().storeFile(McpUploads::Kind::Download, "result.pdf", source, 14, now);
    LOK_ASSERT(!id.empty());
    LOK_ASSERT(fileExists(source));

    // The size given is counted against the store until the document is taken.
    LOK_ASSERT(!McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes - 13, now));

    // Removing the file leaves the stored document whole.
    FileUtil::removeFile(source);
    bool alreadyTaken = false;
    const std::string path = McpUploads::instance().take(id, alreadyTaken, now);
    std::string stored;
    FileUtil::readFile(path, stored);
    LOK_ASSERT_EQUAL_STR("produced bytes", stored);
    LOK_ASSERT(McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes, now));
    McpUploads::discardFile(path);

    // A file that does not exist stores nothing, and leaves no directory behind.
    const std::size_t before = countIncomingEntries();
    LOK_ASSERT(McpUploads::instance()
                   .storeFile(McpUploads::Kind::Download, "result.pdf", source, 14, now)
                   .empty());
    LOK_ASSERT_EQUAL(before, countIncomingEntries());
}

void McpUploadsTests::testUploadIsGivenTimeForItsSize()
{
    constexpr std::string_view testname = __func__;

    // A small upload is given the least time, and a large one a second for each block of the
    // slowest rate on top: 100 MB is given 1660 seconds.
    LOK_ASSERT_EQUAL(McpUploads::MinimumUploadTime.count(),
                     McpUploads::uploadTimeLimit(1024).count());
    LOK_ASSERT_EQUAL(static_cast<long>(1660),
                     static_cast<long>(McpUploads::uploadTimeLimit(100 * 1024 * 1024).count()));

    // An upload under way is past its deadline only once that time has gone by.
    constexpr std::size_t size = 1024 * 1024;
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST,
                                   "/cool/mcp/upload?filename=big.odt",
                                   Poco::Net::HTTPMessage::HTTP_1_1);
    request.setContentLength(size);

    const std::size_t before = countIncomingEntries();
    const auto now = McpUploads::Clock::now();
    bool storeFull = true;
    std::shared_ptr<McpUploadSink> sink = McpUploadSink::start(request, storeFull, now);
    LOK_ASSERT(sink);
    LOK_ASSERT(!storeFull);

    const auto limit = McpUploads::uploadTimeLimit(size);
    LOK_ASSERT(!sink->isPastDeadline(now + limit));
    LOK_ASSERT(sink->isPastDeadline(now + limit + std::chrono::seconds(1)));

    // The upload holds its room until it is dropped, and then its file is gone and its room is
    // given back.
    LOK_ASSERT(!McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes - size + 1, now));
    sink.reset();
    LOK_ASSERT_EQUAL(before, countIncomingEntries());
    LOK_ASSERT(McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes, now));
}

void McpUploadsTests::testUploadSinkReportsAFullStore()
{
    constexpr std::string_view testname = __func__;

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST,
                                   "/cool/mcp/upload?filename=big.odt",
                                   Poco::Net::HTTPMessage::HTTP_1_1);
    request.setContentLength(1024);
    const std::size_t before = countIncomingEntries();

    // An upload the store has no room for is refused, and the refusal says the store is full.
    const auto now = McpUploads::Clock::now();
    bool storeFull = false;
    {
        HeldRoom room;
        LOK_ASSERT(room.reserve(McpUploads::MaxStoredBytes, now));
        LOK_ASSERT(!McpUploadSink::start(request, storeFull, now));
        LOK_ASSERT(storeFull);
    }

    // Once there is room, the same upload starts and holds its room.
    {
        const std::shared_ptr<McpUploadSink> sink = McpUploadSink::start(request, storeFull, now);
        LOK_ASSERT(sink);
        LOK_ASSERT(!storeFull);
        LOK_ASSERT(!McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes, now));
    }

    // An upload dropped before its body arrives gives its room back and leaves no file.
    LOK_ASSERT(McpUploads::instance().hasRoomFor(McpUploads::MaxStoredBytes, now));
    LOK_ASSERT_EQUAL(before, countIncomingEntries());
}

CPPUNIT_TEST_SUITE_REGISTRATION(McpUploadsTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
