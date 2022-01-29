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
};

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf104722)
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf104722.pptx"), PPTX);

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

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf135843)
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf135843.pptx"), PPTX);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix, the test fails with:
    // - Expected: 21165
    // - Actual  : 4218
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[1]", "x", "21165");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[1]", "y", "3866");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[2]", "x", "21165");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[2]", "y", "5956");

    xDocShRef->DoClose();
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf146876)
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf146876.odp"), ODP);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the shape is inside the (5000,8500) - (11500,12500) area
    for (size_t i = 2; i < 4; ++i)
    {
        for (size_t j = 1; j < 6; ++j)
        {
            const OString xPath = "/metafile/push[1]/push[1]/push[" + OString::number(i)
                                  + "]/polyline/point[" + OString::number(j) + "]";
            const sal_Int32 nX = getXPath(pXmlDoc, xPath.getStr(), "x").toInt32();
            const sal_Int32 nY = getXPath(pXmlDoc, xPath.getStr(), "y").toInt32();

            // Without the fix in place, this test would have failed with
            // - Expected greater or equal than: 5000
            // - Actual  : 0
            CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(5000), nX);
            CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(11500), nX);

            CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(8500), nY);
            CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(12500), nY);
        }
    }

    xDocShRef->DoClose();
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf136949)
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf136949.odp"), ODP);

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

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf128212)
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf128212.pptx"), PPTX);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: 7797
    // - Actual  : 12068
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray", "x", "4525");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray", "y", "7797");

    xDocShRef->DoClose();
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testColumnsLayout)
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

CPPUNIT_TEST_FIXTURE(SdLayoutTest, tdf143258_testTbRlLayout)
{
    // This tests a 1-column and a 2-column text boxes' layout

    const OUString sText[] = {
        "tb-rl text within a small text box", // Box 1
        "tb-rl text within a small 2-column text box", // Box 2
    };

    // sentence#, index, length, x, y
    const std::tuple<int, int, int, int, int> strings[] = {
        // Box 1
        { 0, 0, 11, 5346, 3250 },
        { 0, 11, 9, 4635, 3250 },
        { 0, 20, 6, 3924, 3250 },
        { 0, 26, 8, 3213, 3250 },
        // Box 2 column 1
        { 1, 0, 6, 5346, 7250 },
        { 1, 6, 5, 4635, 7250 },
        { 1, 11, 9, 3924, 7250 },
        { 1, 20, 6, 3213, 7250 },
        // Box 2 column 2
        { 1, 26, 2, 5346, 9600 },
        { 1, 28, 7, 4635, 9600 },
        { 1, 35, 5, 3924, 9600 },
        { 1, 40, 3, 3213, 9600 },
    };

    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odg/tb-rl-textbox.odg"), ODG);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/font", SAL_N_ELEMENTS(strings));
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray", SAL_N_ELEMENTS(strings));
    for (size_t i = 0; i < SAL_N_ELEMENTS(strings); ++i)
    {
        const auto & [ sentence, index, length, x, y ] = strings[i];
        OString sXPath = "/metafile/push[1]/push[1]/font[" + OString::number(i + 1) + "]";
        assertXPath(pXmlDoc, sXPath, "orientation", "-900");
        assertXPath(pXmlDoc, sXPath, "vertical", "true");
        sXPath = "/metafile/push[1]/push[1]/textarray[" + OString::number(i + 1) + "]";
        assertXPathContent(pXmlDoc, sXPath + "/text", sText[sentence]);
        assertXPath(pXmlDoc, sXPath, "index", OUString::number(index));
        assertXPath(pXmlDoc, sXPath, "length", OUString::number(length));

        // Without the fix in place, this would have failed with
        // - Expected: 5346
        // - Actual  : 503924
        // - In <>, attribute 'x' of '/metafile/push[1]/push[1]/textarray[1]' incorrect value.
        assertXPath(pXmlDoc, sXPath, "x", OUString::number(x));
        assertXPath(pXmlDoc, sXPath, "y", OUString::number(y));
    }

    xDocShRef->DoClose();
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf146731)
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf146731.pptx"), PPTX);

    std::shared_ptr<GDIMetaFile> xMetaFile = xDocShRef->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[3]/polyline[1]", "width", "187");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[4]/polyline[1]", "width", "187");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]", "width", "187");

    // Without the fix in place, this test would have failed with
    // - Expected: 30
    // - Actual  : 187
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[6]/polyline[1]", "width", "30");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[7]/polyline[1]", "width", "187");

    xDocShRef->DoClose();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
