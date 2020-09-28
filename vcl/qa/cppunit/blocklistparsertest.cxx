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

#include <driverblocklist.hxx>

using namespace DriverBlocklist;

namespace
{

class BlocklistParserTest : public test::BootstrapFixtureBase
{
    void testParse();
    void testEvaluate();
    void testVulkan();

    CPPUNIT_TEST_SUITE(BlocklistParserTest);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testEvaluate);
    CPPUNIT_TEST(testVulkan);
    CPPUNIT_TEST_SUITE_END();
};

void BlocklistParserTest::testParse()
{
    std::vector<DriverInfo> aDriveInfos;

    Parser aBlocklistParser(m_directories.getURLFromSrc("vcl/qa/cppunit/") + "test_blocklist_parse.xml",
        aDriveInfos, VersionType::OpenGL);
    aBlocklistParser.parse();

    size_t const n = aDriveInfos.size();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), n);

    size_t i = 0;

    for (bool bIsAllowlisted : {true, false})
    {
        DriverInfo& aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(GetVendorId(VendorAll), aDriveInfo.maAdapterVendor); // "all"
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_LESS_THAN, aDriveInfo.meComparisonOp);
        CPPUNIT_ASSERT_EQUAL(OpenGLVersion(10,20,30,40), aDriveInfo.mnDriverVersion);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(GetVendorId(VendorNVIDIA), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_EQUAL, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(GetVendorId(VendorMicrosoft), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_NOT_EQUAL, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(OUString("0xcafe"), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_NOT_EQUAL, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(GetVendorId(VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_BETWEEN_EXCLUSIVE, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(GetVendorId(VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_BETWEEN_INCLUSIVE, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(GetVendorId(VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_BETWEEN_INCLUSIVE_START, aDriveInfo.meComparisonOp);

        aDriveInfo = aDriveInfos[i++];
        CPPUNIT_ASSERT_EQUAL(bIsAllowlisted, aDriveInfo.mbAllowlisted);
        CPPUNIT_ASSERT_EQUAL(GetVendorId(VendorAll), aDriveInfo.maAdapterVendor);
        CPPUNIT_ASSERT_EQUAL(VersionComparisonOp::DRIVER_COMPARISON_IGNORED, aDriveInfo.meComparisonOp);
    }
}

void BlocklistParserTest::testEvaluate()
{
    std::vector<DriverInfo> aDriveInfos;

    Parser aBlocklistParser(m_directories.getURLFromSrc("vcl/qa/cppunit/") + "test_blocklist_evaluate.xml",
        aDriveInfos, VersionType::OpenGL);
    aBlocklistParser.parse();

    OUString vendorAMD = GetVendorId(VendorAMD);
    OUString vendorNVIDIA = GetVendorId(VendorNVIDIA);
    OUString vendorIntel = GetVendorId(VendorIntel);
    OUString vendorMicrosoft = GetVendorId(VendorMicrosoft);

    // Check OS
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.40", vendorNVIDIA, "all", DRIVER_OS_WINDOWS_7));
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.40", vendorNVIDIA, "all", DRIVER_OS_WINDOWS_8));
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.40", vendorNVIDIA, "all", DRIVER_OS_WINDOWS_10));

    // Check generic OS
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.50", vendorMicrosoft, "all", DRIVER_OS_WINDOWS_10));
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.50", vendorMicrosoft, "all", DRIVER_OS_LINUX));
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.50", vendorMicrosoft, "all", DRIVER_OS_OSX_10_7));
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.50", vendorMicrosoft, "all", DRIVER_OS_OSX_10_8));

    // Check Vendors
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.40", vendorMicrosoft, "all", DRIVER_OS_WINDOWS_7));
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.40", vendorMicrosoft, "all", DRIVER_OS_WINDOWS_10));

    // Check Versions
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.39", vendorAMD, "all", DRIVER_OS_WINDOWS_7));
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.40", vendorAMD, "all", DRIVER_OS_WINDOWS_7));
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "10.20.30.41", vendorAMD, "all", DRIVER_OS_WINDOWS_7));

    // Check
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::OpenGL, "9.17.10.4229", vendorIntel, "all", DRIVER_OS_WINDOWS_7));


}

void BlocklistParserTest::testVulkan()
{
    std::vector<DriverInfo> aDriveInfos;

    Parser aBlocklistParser(m_directories.getURLFromSrc("vcl/qa/cppunit/") + "test_blocklist_vulkan.xml",
        aDriveInfos, VersionType::Vulkan);
    aBlocklistParser.parse();

    OUString vendorAMD = GetVendorId(VendorAMD);

    // Check Versions
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::Vulkan, "1.2.3", vendorAMD, "all", DRIVER_OS_ALL));
    CPPUNIT_ASSERT_EQUAL(true, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::Vulkan, "1.2.2", vendorAMD, "all", DRIVER_OS_ALL));
    CPPUNIT_ASSERT_EQUAL(false, FindBlocklistedDeviceInList(
                                    aDriveInfos, VersionType::Vulkan, "1.2.20", vendorAMD, "all", DRIVER_OS_ALL));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BlocklistParserTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
