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
 * Unit tests for the local document version store and its retention
 * policy (common/VersionStore.cpp).
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <common/FileUtil.hpp>
#include <common/Prefs.hpp>
#include <common/VersionSettings.hpp>
#include <common/VersionStore.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <chrono>
#include <fstream>
#include <string>
#include <vector>

#include <unistd.h>
#include <utime.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif

using versioning::SnapshotResult;
using versioning::VersionInfo;

class VersionStoreTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(VersionStoreTests);

    CPPUNIT_TEST(testFilenameRoundTrip);
    CPPUNIT_TEST(testThinKeepsRecent);
    CPPUNIT_TEST(testThinHourlyBuckets);
    CPPUNIT_TEST(testThinTierBoundaries);
    CPPUNIT_TEST(testThinExpiry);
    CPPUNIT_TEST(testThinProtectsNewest);
    CPPUNIT_TEST(testThinFutureTimestamps);
    CPPUNIT_TEST(testThinCountCapCollapsesBursts);
    CPPUNIT_TEST(testSnapshotLifecycle);
    CPPUNIT_TEST(testSnapshotMergeWindow);
    CPPUNIT_TEST(testSnapshotTooLarge);
    CPPUNIT_TEST(testSnapshotLargerThanStore);
    CPPUNIT_TEST(testClearHistoryDuringCopy);
    CPPUNIT_TEST(testSnapshotDuringCopy);
    CPPUNIT_TEST(testSnapshotExcluded);
    CPPUNIT_TEST(testUncreatableDirectory);
    CPPUNIT_TEST(testExportVersion);
    CPPUNIT_TEST(testClearHistory);
    CPPUNIT_TEST(testScanRebuildAndOrphanSweep);
    CPPUNIT_TEST(testGlobalCapEviction);
    CPPUNIT_TEST(testLimitsFromPrefs);
    CPPUNIT_TEST(testDocKeyFromPortalHostPath);
    CPPUNIT_TEST(testDocKeyPlainAndSymlinkedPath);
    CPPUNIT_TEST(testPortalHostPath);

    CPPUNIT_TEST_SUITE_END();

    void testFilenameRoundTrip();
    void testThinKeepsRecent();
    void testThinHourlyBuckets();
    void testThinTierBoundaries();
    void testThinExpiry();
    void testThinProtectsNewest();
    void testThinFutureTimestamps();
    void testThinCountCapCollapsesBursts();
    void testSnapshotLifecycle();
    void testSnapshotMergeWindow();
    void testSnapshotTooLarge();
    void testSnapshotLargerThanStore();
    void testClearHistoryDuringCopy();
    void testSnapshotDuringCopy();
    void testSnapshotExcluded();
    void testUncreatableDirectory();
    void testExportVersion();
    void testClearHistory();
    void testScanRebuildAndOrphanSweep();
    void testGlobalCapEviction();
    void testLimitsFromPrefs();
    void testDocKeyFromPortalHostPath();
    void testDocKeyPlainAndSymlinkedPath();
    void testPortalHostPath();

    std::string _root;

public:
    void setUp() override { _root = FileUtil::createRandomTmpDir(); }

    void tearDown() override
    {
        if (!_root.empty())
            FileUtil::removeFile(_root, /*recursive=*/true);
    }

private:
    std::string versionsRoot() const { return _root + "/versions"; }
    std::string exportsDir() const { return _root + "/exports"; }
    std::string prefsFile() const { return _root + "/coda-prefs.json"; }

    void initStore(versioning::Limits limits,
                   std::vector<std::string> excluded = std::vector<std::string>())
    {
        versioning::initialize(versionsRoot(), limits, std::move(excluded), exportsDir());
    }

    std::string writeDoc(const std::string& name, const std::string& content) const
    {
        const std::string path = _root + '/' + name;
        std::ofstream stream(path, std::ios::trunc | std::ios::binary);
        stream << content;
        return path;
    }

    static bool setPortalHostPath(const std::string& path, const std::string& hostPath)
    {
#ifdef __linux__
        return ::setxattr(path.c_str(), "user.document-portal.host-path", hostPath.data(),
                          hostPath.size(), 0) == 0;
#else
        (void)path;
        (void)hostPath;
        return false;
#endif
    }

    /// A version at @age before @now; policy input only.
    static VersionInfo versionAged(std::chrono::system_clock::time_point now,
                                   std::chrono::seconds age, std::uint64_t size = 100)
    {
        VersionInfo version;
        version.timestamp = now - age;
        version.size = size;
        version.fileName = versioning::formatTimestamp(version.timestamp) + ".odt";
        return version;
    }
};

void VersionStoreTests::testFilenameRoundTrip()
{
    constexpr std::string_view testname = __func__;

    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));
    const std::string stamp = versioning::formatTimestamp(now);

    std::chrono::system_clock::time_point parsed;
    LOK_ASSERT(versioning::parseVersionFilename(stamp + ".odt", parsed));
    LOK_ASSERT(parsed == now);
    LOK_ASSERT(versioning::parseVersionFilename(stamp + "-1.odt", parsed));
    LOK_ASSERT(parsed == now);
    LOK_ASSERT(versioning::parseVersionFilename(stamp + "-12.docx", parsed));

    // The strict grammar: anything else is an orphan.
    LOK_ASSERT(!versioning::parseVersionFilename("", parsed));
    LOK_ASSERT(!versioning::parseVersionFilename("meta.json", parsed));
    LOK_ASSERT(!versioning::parseVersionFilename(stamp, parsed)); // No extension.
    LOK_ASSERT(!versioning::parseVersionFilename(stamp + ".", parsed));
    LOK_ASSERT(!versioning::parseVersionFilename(stamp + "-.odt", parsed));
    // A copyAtomic temporary (final name + 12 random characters) must
    // not parse, or the startup sweep would index partial copies.
    LOK_ASSERT(!versioning::parseVersionFilename(stamp + ".odtAbCdEfGhIjKl", parsed));
    LOK_ASSERT(!versioning::parseVersionFilename("20261340-000000.odt", parsed)); // Month 13.
    LOK_ASSERT(!versioning::parseVersionFilename("garbage-000000.odt", parsed));
}

void VersionStoreTests::testThinKeepsRecent()
{
    constexpr std::string_view testname = __func__;

    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));
    std::vector<VersionInfo> versions;
    for (int i = 0; i < 6; ++i)
        versions.push_back(versionAged(now, std::chrono::seconds(60 * i)));

    LOK_ASSERT(versioning::policy::thin(versions, now, 120).empty());
}

void VersionStoreTests::testThinHourlyBuckets()
{
    constexpr std::string_view testname = __func__;

    // now is on an hour boundary, so bucket edges are predictable.
    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));
    std::vector<VersionInfo> versions;
    // Three protected recents.
    versions.push_back(versionAged(now, std::chrono::seconds(10)));
    versions.push_back(versionAged(now, std::chrono::seconds(20)));
    versions.push_back(versionAged(now, std::chrono::seconds(30)));
    // Two versions in the same clock hour, ~1h old.
    versions.push_back(versionAged(now, std::chrono::seconds(3700)));
    versions.push_back(versionAged(now, std::chrono::seconds(3800)));
    // Two more in the previous clock hour.
    versions.push_back(versionAged(now, std::chrono::seconds(7300)));
    versions.push_back(versionAged(now, std::chrono::seconds(7400)));

    // The newest of each hour bucket survives.
    const std::vector<std::size_t> expected{ 4, 6 };
    LOK_ASSERT(versioning::policy::thin(versions, now, 120) == expected);
}

void VersionStoreTests::testThinTierBoundaries()
{
    constexpr std::string_view testname = __func__;

    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));
    const auto day = std::chrono::hours(24);
    std::vector<VersionInfo> versions;
    versions.push_back(versionAged(now, std::chrono::seconds(10)));
    versions.push_back(versionAged(now, std::chrono::seconds(20)));
    versions.push_back(versionAged(now, std::chrono::seconds(30)));
    // Daily tier: two versions on the same day, five days old.
    versions.push_back(versionAged(now, 5 * day + std::chrono::hours(1)));
    versions.push_back(versionAged(now, 5 * day + std::chrono::hours(2)));
    // Weekly tier: two versions in the same week, ~six weeks old.
    versions.push_back(versionAged(now, 42 * day + std::chrono::hours(1)));
    versions.push_back(versionAged(now, 42 * day + std::chrono::hours(4)));
    // Monthly tier: two versions in the same 30-day bucket, ~200 days old.
    versions.push_back(versionAged(now, 200 * day + std::chrono::hours(1)));
    versions.push_back(versionAged(now, 200 * day + std::chrono::hours(4)));

    const std::vector<std::size_t> expected{ 4, 6, 8 };
    LOK_ASSERT(versioning::policy::thin(versions, now, 120) == expected);
}

void VersionStoreTests::testThinExpiry()
{
    constexpr std::string_view testname = __func__;

    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));
    const auto day = std::chrono::hours(24);
    std::vector<VersionInfo> versions;
    versions.push_back(versionAged(now, std::chrono::seconds(10)));
    versions.push_back(versionAged(now, std::chrono::seconds(20)));
    versions.push_back(versionAged(now, std::chrono::seconds(30)));
    versions.push_back(versionAged(now, 366 * day));
    versions.push_back(versionAged(now, 400 * day));

    // Older than a year and not protected: gone.
    const std::vector<std::size_t> expected{ 3, 4 };
    LOK_ASSERT(versioning::policy::thin(versions, now, 120) == expected);
}

void VersionStoreTests::testThinProtectsNewest()
{
    constexpr std::string_view testname = __func__;

    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));
    const auto day = std::chrono::hours(24);

    // Every version is years old; the newest three still survive.
    std::vector<VersionInfo> versions;
    for (int i = 0; i < 5; ++i)
        versions.push_back(versionAged(now, 400 * day + std::chrono::hours(i)));

    const std::vector<std::size_t> expected{ 3, 4 };
    LOK_ASSERT(versioning::policy::thin(versions, now, 120) == expected);
}

void VersionStoreTests::testThinFutureTimestamps()
{
    constexpr std::string_view testname = __func__;

    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));

    // A clock jump stamped versions in the future; clamped age keeps them.
    std::vector<VersionInfo> versions;
    for (int i = 0; i < 5; ++i)
        versions.push_back(versionAged(now, std::chrono::seconds(-3600 * (5 - i))));

    LOK_ASSERT(versioning::policy::thin(versions, now, 120).empty());
}

void VersionStoreTests::testThinCountCapCollapsesBursts()
{
    constexpr std::string_view testname = __func__;

    const auto now = std::chrono::system_clock::time_point(std::chrono::seconds(2000001600));
    const auto day = std::chrono::hours(24);

    std::vector<VersionInfo> versions;
    // A dense recent burst (one minute apart) ...
    for (int i = 0; i < 5; ++i)
        versions.push_back(versionAged(now, std::chrono::seconds(60 * (i + 1))));
    // ... and a sparse old tail (days apart, distinct daily buckets).
    for (int i = 0; i < 3; ++i)
        versions.push_back(versionAged(now, (i + 2) * day));

    const auto deletions = versioning::policy::thin(versions, now, 6);
    // Two must go; the dense burst collapses, the sparse tail survives.
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), deletions.size());
    for (const std::size_t index : deletions)
    {
        LOK_ASSERT(index >= 3); // Not protected.
        LOK_ASSERT(index < 5); // From the burst, not the tail.
    }
}

void VersionStoreTests::testSnapshotLifecycle()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    limits.mergeWindow = std::chrono::seconds::zero();
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "first content");

    LOK_ASSERT(versioning::isEnabled());
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, true));

    // Unchanged content: deduplicated regardless of bypass.
    LOK_ASSERT(SnapshotResult::Deduplicated == versioning::snapshotSync(doc, true));
    LOK_ASSERT(SnapshotResult::Deduplicated == versioning::snapshotSync(doc, false));

    writeDoc("report.odt", "second content");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, false));

    const std::vector<VersionInfo> versions = versioning::listVersions(doc);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), versions.size());
    // Newest first, and every name obeys the grammar.
    LOK_ASSERT(versions[0].timestamp >= versions[1].timestamp);
    std::chrono::system_clock::time_point parsed;
    for (const VersionInfo& version : versions)
    {
        LOK_ASSERT(versioning::parseVersionFilename(version.fileName, parsed));
        LOK_ASSERT(FileUtil::Stat(versioning::docDirectory(doc) + '/' + version.fileName)
                       .isFile());
    }

    LOK_ASSERT(versioning::storeSizeBytes() > 0);
}

void VersionStoreTests::testSnapshotMergeWindow()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    limits.mergeWindow = std::chrono::seconds(600);
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "first content");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, true));

    // Within the window and changed: coalesced without bypass ...
    writeDoc("report.odt", "second content");
    LOK_ASSERT(SnapshotResult::Coalesced == versioning::snapshotSync(doc, false));
    // ... but a manual save bypasses.
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, true));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), versioning::listVersions(doc).size());
}

void VersionStoreTests::testSnapshotTooLarge()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    limits.maxFileSizeBytes = 8;
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "this is more than eight bytes");
    LOK_ASSERT(SnapshotResult::TooLarge == versioning::snapshotSync(doc, true));
    LOK_ASSERT(versioning::listVersions(doc).empty());
}

void VersionStoreTests::testSnapshotLargerThanStore()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    limits.maxStoreSizeBytes = 16;
    initStore(limits);

    const std::string small = writeDoc("small.odt", "ten bytes!");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(small, true));

    // The document cannot fit, so the store keeps what it has.
    const std::string doc = writeDoc("report.odt", "this is more than sixteen bytes");
    LOK_ASSERT(SnapshotResult::TooLarge == versioning::snapshotSync(doc, true));
    LOK_ASSERT(versioning::listVersions(doc).empty());
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), versioning::listVersions(small).size());
}

void VersionStoreTests::testClearHistoryDuringCopy()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "some content");
    const std::string other = writeDoc("other.odt", "other content");

    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, true));
    writeDoc("report.odt", "newer content");

    // The hook runs with the store unlocked, or the calls in it would not
    // return. The copy under way is no version yet, so the delete leaves it.
    std::size_t listedInHook = 0;
    versioning::setCopyHookForTest(
        [&]
        {
            listedInHook = versioning::listVersions(doc).size();
            versioning::clearHistory(doc);
        });
    const SnapshotResult result = versioning::snapshotSync(doc, true);
    versioning::setCopyHookForTest(nullptr);

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), listedInHook);
    LOK_ASSERT(SnapshotResult::Created == result);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), versioning::listVersions(doc).size());
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(13), versioning::storeSizeBytes());

    // The store still takes snapshots afterwards.
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(other, true));
}

void VersionStoreTests::testSnapshotDuringCopy()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "some content");

    // A second snapshot of the same content ends first; the first one finds
    // its version and adds none.
    SnapshotResult inner = SnapshotResult::Failed;
    bool nested = false;
    versioning::setCopyHookForTest(
        [&]
        {
            if (nested)
                return;
            nested = true;
            inner = versioning::snapshotSync(doc, true);
        });
    const SnapshotResult outer = versioning::snapshotSync(doc, true);
    versioning::setCopyHookForTest(nullptr);

    LOK_ASSERT(SnapshotResult::Created == inner);
    LOK_ASSERT(SnapshotResult::Deduplicated == outer);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), versioning::listVersions(doc).size());
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), FileUtil::getDirEntries(versionsRoot()).size());
}

void VersionStoreTests::testSnapshotExcluded()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    initStore(limits, { _root + "/drafts" });

    FileUtil::createDirectories(_root + "/drafts");
    const std::string doc = writeDoc("drafts/Untitled.odt", "draft content");
    LOK_ASSERT(SnapshotResult::Excluded == versioning::snapshotSync(doc, true));
}

void VersionStoreTests::testUncreatableDirectory()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    initStore(limits);
    LOK_ASSERT(versioning::isEnabled());

    FileUtil::removeFile(versionsRoot(), /*recursive=*/true);
    writeDoc("versions", "not a directory");
    const std::string doc = writeDoc("report.odt", "content");
    LOK_ASSERT(SnapshotResult::Failed == versioning::snapshotSync(doc, true));
    LOK_ASSERT(versioning::listVersions(doc).empty());

    initStore(limits);
    LOK_ASSERT(!versioning::isEnabled());
    LOK_ASSERT(SnapshotResult::Disabled == versioning::snapshotSync(doc, true));
}

void VersionStoreTests::testExportVersion()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    limits.mergeWindow = std::chrono::seconds::zero();
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "the exported content");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, true));

    const std::vector<VersionInfo> versions = versioning::listVersions(doc);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), versions.size());

    const std::string exported = versioning::exportVersion(doc, versions[0].fileName);
    LOK_ASSERT(!exported.empty());
    LOK_ASSERT(exported.find(exportsDir()) == 0);
    LOK_ASSERT(exported.find("report (") != std::string::npos);
    LOK_ASSERT(FileUtil::Stat(exported).isFile());
    LOK_ASSERT(FileUtil::compareFileContents(doc, exported));

    // A second export of the same version does not clobber the first.
    const std::string exportedAgain = versioning::exportVersion(doc, versions[0].fileName);
    LOK_ASSERT(!exportedAgain.empty());
    LOK_ASSERT(exported != exportedAgain);

    // Unknown names and path escapes are refused.
    LOK_ASSERT(versioning::exportVersion(doc, "nosuch.odt").empty());
    LOK_ASSERT(versioning::exportVersion(doc, "../" + versions[0].fileName).empty());
}

void VersionStoreTests::testClearHistory()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "some content");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, true));
    LOK_ASSERT(!versioning::listVersions(doc).empty());

    versioning::clearHistory(doc);
    LOK_ASSERT(versioning::listVersions(doc).empty());
    LOK_ASSERT(!FileUtil::Stat(versioning::docDirectory(doc)).exists());
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(0), versioning::storeSizeBytes());
}

void VersionStoreTests::testScanRebuildAndOrphanSweep()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    limits.mergeWindow = std::chrono::seconds::zero();
    initStore(limits);

    const std::string doc = writeDoc("report.odt", "some content");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(doc, true));
    const std::string docDir = versioning::docDirectory(doc);

    // Plant a stale orphan (a forced exit's copy temp), a fresh orphan,
    // and a stray file in the root.
    const std::string staleOrphan = docDir + "/20260101-000000.odtAbCdEfGhIjKl";
    std::ofstream(staleOrphan) << "partial copy";
    const struct utimbuf twoDaysAgo = { time(nullptr) - 2 * 86400, time(nullptr) - 2 * 86400 };
    LOK_ASSERT_EQUAL(0, ::utime(staleOrphan.c_str(), &twoDaysAgo));
    const std::string freshOrphan = docDir + "/20260101-000000.odtMnOpQrStUvWx";
    std::ofstream(freshOrphan) << "in-flight copy";
    std::ofstream(versionsRoot() + "/stray.txt") << "does not belong";

    // Re-initialize: the index rebuilds from the scan.
    initStore(limits);

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), versioning::listVersions(doc).size());
    LOK_ASSERT(!FileUtil::Stat(staleOrphan).exists()); // Swept.
    LOK_ASSERT(FileUtil::Stat(freshOrphan).exists()); // Grace period.
    LOK_ASSERT(!FileUtil::Stat(versionsRoot() + "/stray.txt").exists());

    // The display name survives via meta.json.
    const std::vector<VersionInfo> versions = versioning::listVersions(doc);
    const std::string exported = versioning::exportVersion(doc, versions[0].fileName);
    LOK_ASSERT(exported.find("report (") != std::string::npos);
}

void VersionStoreTests::testGlobalCapEviction()
{
    constexpr std::string_view testname = __func__;

    versioning::Limits limits;
    limits.mergeWindow = std::chrono::seconds::zero();
    initStore(limits);

    const std::string content(100, 'x');
    const std::string docA = writeDoc("a.odt", content + "a1");
    const std::string docB = writeDoc("b.odt", content + "b1");

    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(docA, true));
    writeDoc("a.odt", content + "a2");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(docA, true));
    writeDoc("a.odt", content + "a3");
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(docA, true));
    LOK_ASSERT(SnapshotResult::Created == versioning::snapshotSync(docB, true));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), versioning::listVersions(docA).size());

    // Shrink the cap and prune: docA (most versions) loses its oldest
    // versions; single-version documents are exempt until last.
    limits.maxStoreSizeBytes = 2 * 102 + 50;
    initStore(limits);

    LOK_ASSERT(versioning::storeSizeBytes() <= limits.maxStoreSizeBytes);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), versioning::listVersions(docA).size());
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), versioning::listVersions(docB).size());

    // The survivor of docA is the newest version.
    std::ifstream stream(versioning::docDirectory(docA) + '/' +
                         versioning::listVersions(docA)[0].fileName);
    std::string survivorContent((std::istreambuf_iterator<char>(stream)),
                                std::istreambuf_iterator<char>());
    LOK_ASSERT_EQUAL(content + "a3", survivorContent);
}

void VersionStoreTests::testLimitsFromPrefs()
{
    constexpr std::string_view testname = __func__;

    Prefs prefs(prefsFile());
    versioning::Limits limits = versioning::limitsFromPrefs(prefs);
    LOK_ASSERT(limits.enabled);
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(600),
                     static_cast<std::int64_t>(limits.mergeWindow.count()));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(120), limits.maxVersionsPerDoc);
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(128) << 20, limits.maxFileSizeBytes);
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(4096) << 20, limits.maxStoreSizeBytes);

    prefs.set("versions.enabled", "false");
    prefs.set("versions.mergeWindowSecs", "0");
    prefs.set("versions.maxPerDoc", "0");
    prefs.set("versions.maxFileSizeMB", "garbage");
    prefs.set("versions.maxStoreSizeMB", "7");
    limits = versioning::limitsFromPrefs(prefs);
    LOK_ASSERT(!limits.enabled);
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(0),
                     static_cast<std::int64_t>(limits.mergeWindow.count()));
    // A zero cap would delete every version kept so far.
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), limits.maxVersionsPerDoc);
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(128) << 20, limits.maxFileSizeBytes);
    // The storage limit is at least the file size limit.
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(128) << 20, limits.maxStoreSizeBytes);

    prefs.set("versions.maxFileSizeMB", "5");
    limits = versioning::limitsFromPrefs(prefs);
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(5) << 20, limits.maxFileSizeBytes);
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(7) << 20, limits.maxStoreSizeBytes);

    // The clamp is not written back.
    LOK_ASSERT_EQUAL(std::string("0"), Prefs(prefsFile()).get("versions.maxPerDoc"));
}

void VersionStoreTests::testDocKeyFromPortalHostPath()
{
    constexpr std::string_view testname = __func__;

    initStore(versioning::Limits());

    const std::string doc = writeDoc("doc-portal-id.odt", "content");
    if (!setPortalHostPath(doc, "/home/user/Documents/report.odt"))
    {
        TST_LOG("No user xattr support here, skipping");
        return;
    }

    // Existing histories live in this directory: the key must never change.
    LOK_ASSERT_EQUAL(versionsRoot() + "/e81b949293e047c8", versioning::docDirectory(doc));
}

void VersionStoreTests::testDocKeyPlainAndSymlinkedPath()
{
    constexpr std::string_view testname = __func__;

    initStore(versioning::Limits());

    const std::string doc = writeDoc("report.odt", "content");
    const std::string link = _root + "/link.odt";
    LOK_ASSERT_EQUAL(0, ::symlink(doc.c_str(), link.c_str()));
    FileUtil::createDirectories(_root + "/sub");
    const std::string dotted = _root + "/sub/../report.odt";

    const std::string key = versioning::docDirectory(doc);
    LOK_ASSERT_EQUAL(key, versioning::docDirectory(link));
    LOK_ASSERT_EQUAL(key, versioning::docDirectory(dotted));
    LOK_ASSERT(key != versioning::docDirectory(writeDoc("other.odt", "content")));

    // The key of a plain file is the bytes of its resolved path, the same key a
    // portal file carrying that path as its host path gets.
    const std::string portalDoc = writeDoc("doc-portal-id.odt", "content");
    if (!setPortalHostPath(portalDoc, FileUtil::realpath(doc)))
    {
        TST_LOG("No user xattr support here, skipping the portal comparison");
        return;
    }
    LOK_ASSERT_EQUAL(key, versioning::docDirectory(portalDoc));
}

void VersionStoreTests::testPortalHostPath()
{
    constexpr std::string_view testname = __func__;

    const std::string doc = writeDoc("report.odt", "content");
    LOK_ASSERT(!FileUtil::portalHostPath(doc));
    LOK_ASSERT(!FileUtil::portalHostPath(_root + "/missing.odt"));

    if (!setPortalHostPath(doc, "/home/user/report.odt"))
    {
        TST_LOG("No user xattr support here, skipping");
        return;
    }
    LOK_ASSERT_EQUAL(std::string("/home/user/report.odt"),
                     FileUtil::portalHostPath(doc).value_or(std::string()));

    // Longer than PATH_MAX; ext4 refuses a value this size, tmpfs takes it.
    const std::string shm = FileUtil::createRandomTmpDir("/dev/shm");
    const std::string longPath = '/' + std::string(5000, 'x') + ".odt";
    const std::string shmDoc = shm + "/report.odt";
    std::ofstream(shmDoc) << "content";
    const bool longSet = setPortalHostPath(shmDoc, longPath);
    const std::optional<std::string> longHostPath = FileUtil::portalHostPath(shmDoc);
    FileUtil::removeFile(shm, /*recursive=*/true);
    if (!longSet)
    {
        TST_LOG("No long user xattr support here, skipping the long host path");
        return;
    }
    LOK_ASSERT_EQUAL(longPath, longHostPath.value_or(std::string()));
}

CPPUNIT_TEST_SUITE_REGISTRATION(VersionStoreTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
