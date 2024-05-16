/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <test/unoapixml_test.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>

class SdLayoutTest : public UnoApiXmlTest
{
public:
    SdLayoutTest()
        : UnoApiXmlTest("/sd/qa/unit/data/")
    {
    }

    xmlDocUniquePtr load(const char* pName)
    {
        loadFromFile(OUString::createFromAscii(pName));
        SfxBaseModel* pModel = dynamic_cast<SfxBaseModel*>(mxComponent.get());
        CPPUNIT_ASSERT(pModel);
        SfxObjectShell* pShell = pModel->GetObjectShell();
        std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
        MetafileXmlDump dumper;

        xmlDocUniquePtr pXmlDoc = XmlTestTools::dumpAndParse(dumper, *xMetaFile);
        CPPUNIT_ASSERT(pXmlDoc);

        return pXmlDoc;
    }
};

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf104722)
{
    xmlDocUniquePtr pXmlDoc = load("pptx/tdf104722.pptx");

    // Without the fix in place, this would have failed with
    // - Expected: 2093
    // - Actual  : -10276
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]"_ostr, "x"_ostr, "2093");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]"_ostr, "y"_ostr, "9273");
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf135843)
{
    xmlDocUniquePtr pXmlDoc = load("pptx/tdf135843.pptx");

    // Without the fix, the test fails with:
    // - Expected: 21165
    // - Actual  : 4218
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[1]"_ostr, "x"_ostr,
                "21165");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[1]"_ostr, "y"_ostr,
                "3866");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[2]"_ostr, "x"_ostr,
                "21165");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]/point[2]"_ostr, "y"_ostr,
                "5956");
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf146876)
{
    xmlDocUniquePtr pXmlDoc = load("odp/tdf146876.odp");

    // Check the shape is inside the (5000,8500) - (11500,12500) area
    for (size_t i = 2; i < 4; ++i)
    {
        for (size_t j = 1; j < 6; ++j)
        {
            const OString xPath = "/metafile/push[1]/push[1]/push[" + OString::number(i)
                                  + "]/polyline/point[" + OString::number(j) + "]";
            const sal_Int32 nX = getXPath(pXmlDoc, xPath, "x"_ostr).toInt32();
            const sal_Int32 nY = getXPath(pXmlDoc, xPath, "y"_ostr).toInt32();

            // Without the fix in place, this test would have failed with
            // - Expected greater or equal than: 5000
            // - Actual  : 0
            CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(5000), nX);
            CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(11500), nX);

            CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(8500), nY);
            CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(12500), nY);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf136949)
{
    xmlDocUniquePtr pXmlDoc = load("odp/tdf136949.odp");

    // Without the fix in place, this test would have failed with
    // - Expected: 13687
    // - Actual  : 2832
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[7]/polyline/point[1]"_ostr, "x"_ostr,
                "13687");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[7]/polyline/point[2]"_ostr, "x"_ostr,
                "24759");
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf128212)
{
    xmlDocUniquePtr pXmlDoc = load("pptx/tdf128212.pptx");

    // Without the fix in place, this test would have failed with
    // - Expected: 7795
    // - Actual  : 12068
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray"_ostr, "x"_ostr, "4523");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray"_ostr, "y"_ostr, "7795");
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

    xmlDocUniquePtr pXmlDoc = load("odg/two_columns.odg");

    for (size_t i = 0; i < SAL_N_ELEMENTS(strings); ++i)
    {
        const auto & [ sentence, index, length, x, y ] = strings[i];
        OString sXPath = "/metafile/push[1]/push[1]/textarray[" + OString::number(i + 1) + "]";
        assertXPathContent(pXmlDoc, sXPath + "/text", sText[sentence]);
        assertXPath(pXmlDoc, sXPath, "index"_ostr, OUString::number(index));
        assertXPath(pXmlDoc, sXPath, "length"_ostr, OUString::number(length));
        assertXPath(pXmlDoc, sXPath, "x"_ostr, OUString::number(x));
        assertXPath(pXmlDoc, sXPath, "y"_ostr, OUString::number(y));
    }
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
        { 1, 11, 7, 3924, 7250 },
        { 1, 18, 8, 3213, 7250 },
        // Box 2 column 2
        { 1, 26, 2, 5346, 9600 },
        { 1, 28, 7, 4635, 9600 },
        { 1, 35, 5, 3924, 9600 },
        { 1, 40, 3, 3213, 9600 },
    };

    xmlDocUniquePtr pXmlDoc = load("odg/tb-rl-textbox.odg");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/font"_ostr, SAL_N_ELEMENTS(strings));
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray"_ostr, SAL_N_ELEMENTS(strings));
    for (size_t i = 0; i < SAL_N_ELEMENTS(strings); ++i)
    {
        const auto & [ sentence, index, length, x, y ] = strings[i];
        OString sXPath = "/metafile/push[1]/push[1]/font[" + OString::number(i + 1) + "]";
        assertXPath(pXmlDoc, sXPath, "orientation"_ostr, "-900");
        assertXPath(pXmlDoc, sXPath, "vertical"_ostr, "true");
        sXPath = "/metafile/push[1]/push[1]/textarray[" + OString::number(i + 1) + "]";
        assertXPathContent(pXmlDoc, sXPath + "/text", sText[sentence]);
        assertXPath(pXmlDoc, sXPath, "index"_ostr, OUString::number(index));
        assertXPath(pXmlDoc, sXPath, "length"_ostr, OUString::number(length));

        // Without the fix in place, this would have failed with
        // - Expected: 5346
        // - Actual  : 503924
        // - In <>, attribute 'x' of '/metafile/push[1]/push[1]/textarray[1]' incorrect value.
        assertXPath(pXmlDoc, sXPath, "x"_ostr, OUString::number(x));
        assertXPath(pXmlDoc, sXPath, "y"_ostr, OUString::number(y));
    }
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf146731)
{
    xmlDocUniquePtr pXmlDoc = load("pptx/tdf146731.pptx");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[3]/polyline[1]"_ostr, "width"_ostr, "187");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[4]/polyline[1]"_ostr, "width"_ostr, "187");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[5]/polyline[1]"_ostr, "width"_ostr, "187");

    // Without the fix in place, this test would have failed with
    // - Expected: 30
    // - Actual  : 187
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[6]/polyline[1]"_ostr, "width"_ostr, "30");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[7]/polyline[1]"_ostr, "width"_ostr, "187");
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf135843_InsideHBorders)
{
    xmlDocUniquePtr pXmlDoc = load("pptx/tdf135843_insideH.pptx");

    // Without the fix, the test fails with:
    //- Expected: 34
    //- Actual  : 36
    // We shouldn't see two vertical borders inside the table on ui.

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push"_ostr, 34);
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testBnc480256)
{
    xmlDocUniquePtr pXmlDoc = load("pptx/bnc480256-2.pptx");

    // Without the fix, the test fails with:
    //- Expected: #ff0000
    //- Actual  : #ffffff
    // We should see the red vertical border inside the table.

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[8]/linecolor[1]"_ostr, "color"_ostr,
                "#ff0000");
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testFitToFrameTextFitting)
{
    // This test checks that the text fitting is working correctly when
    // the textbox is set to "fit to frame" by stretching the text to or
    // near the textbox boundary. The problem is especially complicated
    // when the font size is set to a higher number (like 999)
    //
    // The text fitting behaviour when "fit by frame" is enabled is to
    // always fit the text into the text box (without forcing the text
    // into new line) by shrinking or expanding the text horizontally
    // and vertically.

    xmlDocUniquePtr pXmlDoc = load("odg/FitToFrameText.odg");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]"_ostr, "x"_ostr, "0");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]"_ostr, "y"_ostr, "406");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]/dxarray"_ostr, "first"_ostr,
                "114");
#ifndef _WIN32 // Windows seems to differ in text layouting, so ignore for now
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[1]/dxarray"_ostr, "last"_ostr,
                "6984");
#endif
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf156955)
{
    xmlDocUniquePtr pXmlDoc = load("odp/tdf156955.odp");

    // Make sure text box has the right size - without the fix it was 2759.
    assertXPath(pXmlDoc, "/metafile/push/push/textarray[5]"_ostr, "y"_ostr, "3183");
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf148966)
{
    // Test related to IgnoreBreakAfterMultilineField compatibility flag.
    {
        xmlDocUniquePtr pXmlDoc = load("pptx/tdf148966.pptx");
        // Without the accompanying fix, would fail with:
        // - Expected: 5952
        // - Actual  : 7814
        // i.e. Line break after multiline field should have been ignored.
        assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[3]"_ostr, "y"_ostr, "5952");
    }
    {
        xmlDocUniquePtr pXmlDoc = load("odp/tdf148966-withflag.odp");
        // Without the accompanying fix, would fail with:
        // - Expected: 5952
        // - Actual  : 7814
        // i.e. When IgnoreBreakAfterMultilineField flag is set, line break
        // after multiline field should have been ignored.
        assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[3]"_ostr, "y"_ostr, "5952");
    }
    {
        xmlDocUniquePtr pXmlDoc = load("odp/tdf148966-withoutflag.odp");
        assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[3]"_ostr, "y"_ostr, "7814");
    }
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTableVerticalText)
{
    xmlDocUniquePtr pXmlDoc = load("pptx/tcPr-vert-roundtrip.pptx");

    // Without the accompanying fix, would fail with:
    // - Expected: -900
    // - Actual  : 0
    // - In <>, attribute 'orientation' of '//font[1]' incorrect value.
    // i.e. table cell text that was supposed to be vertical (rotated 90
    // degrees) was not vertical.
    assertXPath(pXmlDoc, "//font[1]"_ostr, "orientation"_ostr, "-900");
    assertXPath(pXmlDoc, "//font[2]"_ostr, "orientation"_ostr, "900");
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf112594)
{
    xmlDocUniquePtr pXmlDoc = load("odp/Tdf112594.fodp");

    // Test that a NNBSP is grouped with the Mongolian characters after it, so
    // we have one text array covering the whole string.
    //
    // Without the fix, it fails with:
    // - Expected: 4
    // - Actual  : 3
    // - In <>, attribute 'length' of '/metafile/push[1]/push[1]/textarray[3]' incorrect value.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[3]"_ostr, "index"_ostr, "0");
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray[3]"_ostr, "length"_ostr, "4");
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/textarray[3]/text"_ostr,
                       u"11\u202f\u1824"_ustr);
}

CPPUNIT_TEST_FIXTURE(SdLayoutTest, testTdf152906_AdjustToContour)
{
    // Test that the text adjusts to contour properly

    constexpr OUString sText
        = u"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum consequat mi quis "
          "pretium semper. Proin luctus orci ac neque venenatis, quis commodo dolor posuere. "
          "Curabitur dignissim sapien quis cursus egestas. Donec blandit auctor arcu, nec "
          "pellentesque eros molestie eget. In consectetur aliquam hendrerit. Sed cursus mauris "
          "vitae ligula pellentesque, non pellentesque urna aliquet. Fusce placerat mauris enim, "
          "nec rutrum purus semper vel. Praesent tincidunt neque eu pellentesque pharetra. Fusce "
          "pellentesque est orci."_ustr;

    // index, length, x, y
    const std::tuple<int, int, int, int> strings[] = {
        { 0, 6, 9599, 8647 }, //                        Lorem
        { 6, 22, 7570, 9358 }, //               ipsum dolor sit amet,
        { 28, 29, 6775, 10069 }, //         consectetur adipiscing elit.
        { 57, 29, 6299, 10780 }, //         Vestibulum consequat mi quis
        { 86, 37, 5453, 11491 }, //     pretium semper. Proin luctus orci ac
        { 123, 36, 5134, 12202 }, //     neque venenatis, quis commodo dolor
        { 159, 41, 4764, 12913 }, //  posuere. Curabitur dignissim sapien quis
        { 200, 43, 4481, 13624 }, // cursus egestas. Donec blandit auctor arcu,
        { 243, 40, 4975, 14335 }, //   nec pellentesque eros molestie eget. In
        { 283, 42, 4552, 15046 }, //  consectetur aliquam hendrerit. Sed cursus
        { 325, 38, 5363, 15757 }, //    mauris vitae ligula pellentesque, non
        { 363, 42, 4692, 16468 }, //  pellentesque urna aliquet. Fusce placerat
        { 405, 37, 5047, 17179 }, //    mauris enim, nec rutrum purus semper
        { 442, 33, 5963, 17890 }, //      vel. Praesent tincidunt neque eu
        { 475, 29, 6387, 18601 }, //        pellentesque pharetra. Fusce
        { 504, 22, 7499, 19312 }, //           pellentesque est orci.
    };

    xmlDocUniquePtr pXmlDoc = load("odg/adjust-to-contour.fodg");

    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/textarray"_ostr, std::size(strings));
    for (size_t i = 0; i < std::size(strings); ++i)
    {
        const auto & [ index, length, x, y ] = strings[i];
        OString sXPath = "/metafile/push[1]/push[1]/textarray[" + OString::number(i + 1) + "]";
        assertXPathContent(pXmlDoc, sXPath + "/text", sText);
        assertXPath(pXmlDoc, sXPath, "index"_ostr, OUString::number(index));
        assertXPath(pXmlDoc, sXPath, "length"_ostr, OUString::number(length));
        assertXPath(pXmlDoc, sXPath, "x"_ostr, OUString::number(x));
        assertXPath(pXmlDoc, sXPath, "y"_ostr, OUString::number(y));
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
