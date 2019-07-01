/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unotest/bootstrapfixturebase.hxx>

#include <opengl/win/blocklist_parser.hxx>

namespace
{

class BlocklistParserTest : public test::BootstrapFixtureBase
{
    void testParse();
    void testEvaluate();

    CPPUNIT_TEST_SUITE(BlocklistParserTest);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testEvaluate);
    CPPUNIT_TEST_SUITE_END();
};

void BlocklistParserTest::testParse()
{
    std::vector<wgl::DriverInfo> aDriveInfos;

    WinBlocklistParser aBlocklistParser(m_directories.getURLFromSrc("vcl/qa/cppunit/") + "test_blocklist_parse.xml", aDriveInfos);
    aBlocklistParser.parse();

    size_t const n = aDriveInfos.size();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), n);

    size_t i = 0;

    for (bool bIsWhitelisted : {true, false})
    {
        wgl::DriverInfo& aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorAll), aDriveInfo.maAdapterVendor); // "all"
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_LESS_THAN, aDriveInfo.meComparisonOp);
        CPPUNIT_ASSERT_EQUAL(wgl::V(10,20,30,40), aDriveInfo.mnDriverVersion);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorNVIDIA), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_EQUAL, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorMicrosoft), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_NOT_EQUAL, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(OUString("0xcafe"), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_NOT_EQUAL, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_BETWEEN_EXCLUSIVE, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_BETWEEN_INCLUSIVE, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_BETWEEN_INCLUSIVE_START, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsWhitelisted, aDriveInfo.mbWhitelisted);
        CPPUNIT_ASSERT_EQUAL(WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(wgl::VersionComparisonOp::DRIVER_COMPARISON_IGNORED, aDriveInfo.meComparisonOp);
    }
}

void BlocklistParserTest::testEvaluate()
{
    std::vector<wgl::DriverInfo> aDriveInfos;

    WinBlocklistParser aBlocklistParser(m_directories.getURLFromSrc("vcl/qa/cppunit/") + "test_blocklist_evaluate.xml", aDriveInfos);
    aBlocklistParser.parse();

    OUString vendorAMD = WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorAMD);
    OUString vendorNVIDIA = WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorNVIDIA);
    OUString vendorIntel = WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorIntel);
    OUString vendorMicrosoft = WinOpenGLDeviceInfo::GetDeviceVendor(wgl::VendorMicrosoft);

    uint32_t const osWindows7 = 0x00060001;
    uint32_t const osWindows8 = 0x00060002;
    uint32_t const osWindows10 = 0x000A0000;

    // Check OS
    CPPUNIT_ASSERT_EQUAL(false, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.40", vendorNVIDIA, "all", osWindows7));
    CPPUNIT_ASSERT_EQUAL(false, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.40", vendorNVIDIA, "all", osWindows8));
    CPPUNIT_ASSERT_EQUAL(false, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.40", vendorNVIDIA, "all", osWindows10));


    // Check Vendors
    CPPUNIT_ASSERT_EQUAL(true, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.40", vendorMicrosoft, "all", osWindows7));
    CPPUNIT_ASSERT_EQUAL(true, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.40", vendorMicrosoft, "all", osWindows10));

    // Check Versions
    CPPUNIT_ASSERT_EQUAL(true, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.39", vendorAMD, "all", osWindows7));
    CPPUNIT_ASSERT_EQUAL(false, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.40", vendorAMD, "all", osWindows7));
    CPPUNIT_ASSERT_EQUAL(false, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "10.20.30.41", vendorAMD, "all", osWindows7));

    // Check
    CPPUNIT_ASSERT_EQUAL(true, WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(
                                    aDriveInfos, "9.17.10.4229", vendorIntel, "all", osWindows7));


}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BlocklistParserTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
