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
#include <wsd/DocumentBroker.hpp>

#include <test/lokassert.hpp>

#include <Poco/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Node.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <fstream>
#include <string>
#include <string_view>
#include <vector>

/// Tests the xcu preset upload filter: which oor:path roots are allowed, and
/// that scrubbing a registrymodifications.xcu keeps only those.
class XcuScrubWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(XcuScrubWhiteBoxTests);
    CPPUNIT_TEST(testIsAllowedXcuPath);
    CPPUNIT_TEST(testScrubKeepsAllowedDropsTheRest);
    CPPUNIT_TEST(testScrubDropsEverythingReturnsEmpty);
    CPPUNIT_TEST_SUITE_END();

    void testIsAllowedXcuPath();
    void testScrubKeepsAllowedDropsTheRest();
    void testScrubDropsEverythingReturnsEmpty();
};

namespace
{
std::string writeTempXcu(const std::string& dir, const std::string& body)
{
    const std::string path = dir + "/registrymodifications.xcu";
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           "<oor:items xmlns:oor=\"http://openoffice.org/2001/registry\">"
        << body << "</oor:items>";
    return path;
}

std::string xcuItem(const std::string& oorPath)
{
    return "<item oor:path=\"" + oorPath + "\"><prop oor:name=\"n\"><value>1</value></prop></item>";
}

// The oor:path of every top-level <item> in the file, in document order.
std::vector<std::string> itemPaths(const std::string& path)
{
    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> doc = parser.parse(path);
    std::vector<std::string> paths;
    for (Poco::XML::Node* node = doc->documentElement()->firstChild(); node;
         node = node->nextSibling())
    {
        if (node->nodeType() == Poco::XML::Node::ELEMENT_NODE)
            paths.push_back(static_cast<Poco::XML::Element*>(node)->getAttribute("oor:path"));
    }
    return paths;
}
}

void XcuScrubWhiteBoxTests::testIsAllowedXcuPath()
{
    constexpr std::string_view testname = __func__;

    // An allowed root matches itself and anything below it.
    LOK_ASSERT(isAllowedXcuPath("/org.openoffice.Office.Common/BulletsNumbering"));
    LOK_ASSERT(isAllowedXcuPath("/org.openoffice.Office.Common/BulletsNumbering/Format"));
    LOK_ASSERT(isAllowedXcuPath("/org.openoffice.Office.Writer/Grid"));

    // The match stops at a path-segment boundary, so a longer sibling name is
    // not let through.
    LOK_ASSERT(!isAllowedXcuPath("/org.openoffice.Office.Common/BulletsNumberingFoo"));

    // An ancestor of an allowed root is not itself allowed.
    LOK_ASSERT(!isAllowedXcuPath("/org.openoffice.Office.Common"));

    // Unrelated roots and the empty string are rejected.
    LOK_ASSERT(!isAllowedXcuPath("/org.openoffice.Office.Recovery/RecoveryList"));
    LOK_ASSERT(!isAllowedXcuPath(""));
}

void XcuScrubWhiteBoxTests::testScrubKeepsAllowedDropsTheRest()
{
    constexpr std::string_view testname = __func__;

    const std::string dir = FileUtil::createRandomTmpDir();
    FileUtil::OwnedFile dirCleanup(dir, /*recursive=*/true);

    const std::string raw = writeTempXcu(
        dir, xcuItem("/org.openoffice.Office.Common/BulletsNumbering") +
                 xcuItem("/org.openoffice.Office.Recovery/RecoveryList") +
                 xcuItem("/org.openoffice.Office.Writer/Grid"));

    const std::string scrubbed = scrubXcuForUpload(raw);
    LOK_ASSERT(!scrubbed.empty());
    FileUtil::OwnedFile scrubbedCleanup(scrubbed);

    // Only the two allowed items survive, in their original order. The
    // BulletsNumbering item surviving also proves the oor:path attribute is
    // read back through Poco's namespace-aware DOM, not dropped.
    const std::vector<std::string> kept = itemPaths(scrubbed);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), kept.size());
    LOK_ASSERT_EQUAL(std::string("/org.openoffice.Office.Common/BulletsNumbering"), kept[0]);
    LOK_ASSERT_EQUAL(std::string("/org.openoffice.Office.Writer/Grid"), kept[1]);
}

void XcuScrubWhiteBoxTests::testScrubDropsEverythingReturnsEmpty()
{
    constexpr std::string_view testname = __func__;

    const std::string dir = FileUtil::createRandomTmpDir();
    FileUtil::OwnedFile dirCleanup(dir, /*recursive=*/true);

    const std::string raw =
        writeTempXcu(dir, xcuItem("/org.openoffice.Office.Recovery/RecoveryList"));

    // Nothing allowed remains, so the caller is told to skip the upload and no
    // file is written.
    LOK_ASSERT_EQUAL(std::string(), scrubXcuForUpload(raw));
}

CPPUNIT_TEST_SUITE_REGISTRATION(XcuScrubWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
