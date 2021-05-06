/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "sdmodeltestbase.hxx"

class SdLayoutTest : public SdModelTestBaseXML
{
public:
    void testTdf104722();
    void testTdf136949();
    void testTdf128212();
    void testColumnsLayout();

    CPPUNIT_TEST_SUITE(SdLayoutTest);

    CPPUNIT_TEST(testTdf104722);
    CPPUNIT_TEST(testTdf136949);
    CPPUNIT_TEST(testTdf128212);
    CPPUNIT_TEST(testColumnsLayout);

    CPPUNIT_TEST_SUITE_END();
};

void SdLayoutTest::testTdf104722()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf104722.pptx"), PPTX);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this would have failed with
    // - Expected: 2093
    // - Actual  : -10276
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]", "x", "2093");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]", "y", "9273");

    xDocShRef->DoClose();
}

void SdLayoutTest::testTdf136949()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf136949.odp"), ODP);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: 13687
    // - Actual  : 2832
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[7]/polyline/point[1]", "x", "13687");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[7]/polyline/point[2]", "x", "24759");

    xDocShRef->DoClose();
}

void SdLayoutTest::testTdf128212()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf128212.pptx"), PPTX);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: 7798
    // - Actual  : 12068
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray", "x", "4525");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray", "y", "7798");

    xDocShRef->DoClose();
}

void SdLayoutTest::testColumnsLayout()
{
    // This tests a 2-column text box's layout

    const OUString sText[] = {
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum consequat mi quis "
        "pretium semper. Proin luctus orci ac neque venenatis, quis commodo dolor posuere. "
        "Curabitur dignissim sapien quis cursus egestas. Donec blandit auctor arcu, nec "
        "pellentesque eros molestie eget. In consectetur aliquam hendrerit. Sed cursus mauris "
        "vitae ligula pellentesque, non pellentesque urna aliquet. Fusce placerat mauris enim, "
        "nec rutrum purus semper vel. Praesent tincidunt neque eu pellentesque pharetra. Fusce "
        "pellentesque est orci.",
        "Integer sodales tincidunt tristique. Sed a metus posuere, adipiscing nunc et, viverra "
        "odio. Donec auctor molestie sem, sit amet tristique lectus hendrerit sed. Cras sodales "
        "nisl sed orci mattis iaculis. Nunc eget dolor accumsan, pharetra risus a, vestibulum "
        "mauris. Nunc vulputate lobortis mollis. Vivamus nec tellus faucibus, tempor magna nec, "
        "facilisis felis. Donec commodo enim a vehicula pellentesque. Nullam vehicula vestibulum "
        "est vel ultricies.",
        "Aliquam velit massa, laoreet vel leo nec, volutpat facilisis eros. Donec consequat arcu "
        "ut diam tempor luctus. Cum sociis natoque penatibus et magnis dis parturient montes, "
        "nascetur ridiculus mus. Praesent vitae lacus vel leo sodales pharetra a a nibh. "
        "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; "
        "Nam luctus tempus nibh, fringilla dictum augue consectetur eget. Curabitur at ante sit "
        "amet tortor pharetra molestie eu nec ante. Mauris tincidunt, nibh eu sollicitudin "
        "molestie, dolor sapien congue tortor, a pulvinar sapien turpis sed ante. Donec nec est "
        "elementum, euismod nulla in, mollis nunc."
    };

    // sentence#, index, length, x, y
    const std::tuple<int, int, int, int, int> strings[] = {
        // Column 1
        { 0, 0, 40, 3750, 3193 },
        { 0, 40, 41, 3750, 3587 },
        { 0, 81, 39, 3750, 3981 },
        { 0, 120, 33, 3750, 4375 },
        { 0, 153, 35, 3750, 4769 },
        { 0, 188, 34, 3750, 5163 },
        { 0, 222, 38, 3750, 5557 },
        { 0, 260, 35, 3750, 5951 },
        { 0, 295, 37, 3750, 6345 },
        { 0, 332, 31, 3750, 6739 },
        { 0, 363, 33, 3750, 7133 },
        { 0, 396, 39, 3750, 7527 },
        { 0, 435, 37, 3750, 7921 },
        { 0, 472, 32, 3750, 8315 },
        { 0, 504, 22, 3750, 8709 },
        { 1, 0, 43, 3750, 9103 },
        { 1, 43, 35, 3750, 9497 },
        { 1, 78, 36, 3750, 9891 },
        { 1, 114, 41, 3750, 10285 },
        { 1, 155, 39, 3750, 10679 },
        { 1, 194, 35, 3750, 11073 },
        { 1, 229, 37, 3750, 11467 },
        { 1, 266, 40, 3750, 11861 },
        { 1, 306, 39, 3750, 12255 },
        { 1, 345, 38, 3750, 12649 },
        { 1, 383, 39, 3750, 13043 },
        { 1, 422, 29, 3750, 13437 },
        { 2, 0, 42, 3750, 13831 },
        { 2, 42, 41, 3750, 14225 },
        { 2, 83, 39, 3750, 14619 },
        { 2, 122, 32, 3750, 15013 },
        { 2, 154, 38, 3750, 15407 },
        { 2, 192, 34, 3750, 15801 },
        { 2, 226, 38, 3750, 16195 },
        // Column 2
        { 2, 264, 42, 10725, 3193 },
        { 2, 306, 39, 10725, 3587 },
        { 2, 345, 37, 10725, 3981 },
        { 2, 382, 37, 10725, 4375 },
        { 2, 419, 42, 10725, 4769 },
        { 2, 461, 36, 10725, 5163 },
        { 2, 497, 36, 10725, 5557 },
        { 2, 533, 40, 10725, 5951 },
        { 2, 573, 35, 10725, 6345 },
        { 2, 608, 30, 10725, 6739 },
    };

    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odg/two_columns.odg"), ODG);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    for (size_t i = 0; i < SAL_N_ELEMENTS(strings); ++i)
    {
        const auto & [ sentence, index, length, x, y ] = strings[i];
        OString sXPath = "/metafile/push[1]/push[1]/textarray[" + OString::number(i + 1) + "]";
        assertXPathContent(pXmlDoc, sXPath + "/text", sText[sentence]);
        assertXPath(pXmlDoc, sXPath, "index", OUString::number(index));
        assertXPath(pXmlDoc, sXPath, "length", OUString::number(length));
        assertXPath(pXmlDoc, sXPath, "x", OUString::number(x));
        assertXPath(pXmlDoc, sXPath, "y", OUString::number(y));
    }

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdLayoutTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
