/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <string_view>

#include <test/xmltesttools.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/wmf.hxx>
#include <mtftools.hxx>

using namespace css;

class WmfTest : public test::BootstrapFixture, public XmlTestTools
{
    OUString maDataUrl;

    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

public:
    WmfTest()
        : BootstrapFixture(true, false)
        , maDataUrl(u"/emfio/qa/cppunit/wmf/data/"_ustr)
    {
    }

    void testEOFWmf();
    void testNonPlaceableWmf();
    void testTdf88163NonPlaceableWmf();
    void testTdf88163PlaceableWmf();
    void testSetTextAlignWmf();
    void testSine();
    void testEmfProblem();
    void testEmfLineStyles();
    void testWorldTransformFontSize();
    void testBigPPI();
    void testTdf93750();
    void testTdf99402();
    void testTdf39894Wmf();
    void testTdf39894Emf();
    void testETO_PDYWmf();
    void testETO_PDYEmf();
    void testStockObject();

    CPPUNIT_TEST_SUITE(WmfTest);
    CPPUNIT_TEST(testEOFWmf);
    CPPUNIT_TEST(testNonPlaceableWmf);
    CPPUNIT_TEST(testTdf88163NonPlaceableWmf);
    CPPUNIT_TEST(testTdf88163PlaceableWmf);
    CPPUNIT_TEST(testSetTextAlignWmf);
    CPPUNIT_TEST(testSine);
    CPPUNIT_TEST(testEmfProblem);
    CPPUNIT_TEST(testEmfLineStyles);
    CPPUNIT_TEST(testWorldTransformFontSize);
    CPPUNIT_TEST(testBigPPI);
    CPPUNIT_TEST(testTdf93750);
    CPPUNIT_TEST(testTdf99402);
    CPPUNIT_TEST(testTdf39894Wmf);
    CPPUNIT_TEST(testTdf39894Emf);
    CPPUNIT_TEST(testETO_PDYWmf);
    CPPUNIT_TEST(testETO_PDYEmf);
    CPPUNIT_TEST(testStockObject);
    CPPUNIT_TEST_SUITE_END();
};

void WmfTest::testEOFWmf()
{
    // tdf#155887 Test META_EOF with size different than 3
    // It should be properly displayed as MS Office do
    SvFileStream aFileStream(getFullUrl(u"EOF.wmf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/metafile/push"_ostr, 2);
    assertXPath(pDoc, "/metafile/push[2]"_ostr, "flags"_ostr, u"PushClipRegion"_ustr);
    assertXPath(pDoc, "/metafile/push[2]/fillcolor"_ostr, 2);
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[1]"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[2]"_ostr, "color"_ostr, u"#d0d0d0"_ustr);
    assertXPath(pDoc, "/metafile/push[2]/linecolor"_ostr, 60);
    assertXPath(pDoc, "/metafile/push[2]/polyline"_ostr, 209);
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point"_ostr, 5);
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point[3]"_ostr, "x"_ostr, u"16906"_ustr);
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point[3]"_ostr, "y"_ostr, u"12673"_ustr);
    assertXPath(pDoc, "/metafile/push[2]/textarray"_ostr, 307);
}

void WmfTest::testNonPlaceableWmf()
{
    SvFileStream aFileStream(getFullUrl(u"visio_import_source.wmf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::POLYLINE, false);

    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // These values come from changes done in tdf#88163
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]"_ostr, "x"_ostr, u"16813"_ustr);
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]"_ostr, "y"_ostr, u"1004"_ustr);

    assertXPath(pDoc, "/metafile/polyline[1]/point[2]"_ostr, "x"_ostr, u"16813"_ustr);
    assertXPath(pDoc, "/metafile/polyline[1]/point[2]"_ostr, "y"_ostr, u"7514"_ustr);

    assertXPath(pDoc, "/metafile/polyline[1]/point[3]"_ostr, "x"_ostr, u"26112"_ustr);
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]"_ostr, "y"_ostr, u"7514"_ustr);

    assertXPath(pDoc, "/metafile/polyline[1]/point[4]"_ostr, "x"_ostr, u"26112"_ustr);
    assertXPath(pDoc, "/metafile/polyline[1]/point[4]"_ostr, "y"_ostr, u"1004"_ustr);

    assertXPath(pDoc, "/metafile/polyline[1]/point[5]"_ostr, "x"_ostr, u"16813"_ustr);
    assertXPath(pDoc, "/metafile/polyline[1]/point[5]"_ostr, "y"_ostr, u"1004"_ustr);
}

void WmfTest::testTdf88163NonPlaceableWmf()
{
    OUString fileName(u"tdf88163-non-placeable.wmf"_ustr);
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // These values come from the fix for tdf#88163

    // Fails without the fix
    // With fix: 3272, without fix: ~ 8000
    auto x = getXPath(pDoc, "/metafile/push[2]/font[1]"_ostr, "height"_ostr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3272), x.toInt32());

    // Fails without the fix: Expected: 7359, Actual: 7336
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "x"_ostr, u"7359"_ustr);
    // Fails without the fix: Expected: 4118, Actual: 4104
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "y"_ostr, u"4118"_ustr);

    // Fails without the fix: Expected: 5989, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr, u"5989"_ustr);
    // Fails without the fix: Expected: 16264, Actual: 16208
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr, u"16264"_ustr);

    // Fails without the fix: Expected: 20769, Actual: 20705
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "x"_ostr, u"20769"_ustr);
    // Fails without the fix: Expected: 4077, Actual: 4062
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr, u"4077"_ustr);
}

void WmfTest::testTdf88163PlaceableWmf()
{
    OUString fileName(u"tdf88163-wrong-font-size.wmf"_ustr);
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;

    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // These values come from the fix for tdf#88163

    // The fix does not affect the font size
    auto x = getXPath(pDoc, "/metafile/push[2]/font[1]"_ostr, "height"_ostr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(313), x.toInt32());

    // Fails without the fix: Expected: 1900, Actual: 19818
    assertXPath(pDoc, "/metafile"_ostr, "height"_ostr, u"1900"_ustr);

    // Fails without the fix: Expected: 704, Actual: 7336
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "x"_ostr, u"704"_ustr);
    // Fails without the fix: Expected: 394, Actual: 4110
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "y"_ostr, u"394"_ustr);

    // Fails without the fix: Expected: 573, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr, u"573"_ustr);
    // Fails without the fix: Expected: 1556, Actual: 16230
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr, u"1556"_ustr);

    // Fails without the fix: Expected: 1987, Actual: 20706
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "x"_ostr, u"1987"_ustr);
    // Fails without the fix: Expected: 390, Actual: 4068
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr, u"390"_ustr);
}

void WmfTest::testSetTextAlignWmf()
{
    OUString fileName(u"TestSetTextAlign.wmf"_ustr);
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;

    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile"_ostr, "height"_ostr, u"20999"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[2]"_ostr, "align"_ostr, u"top"_ustr);
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr, u"11642"_ustr);
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr, u"212"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[2]/text"_ostr,
                       u"textalignment:default"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[3]"_ostr, "align"_ostr, u"top"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr, u"212"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[3]/text"_ostr,
                       u"textalignment:0x00"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[4]"_ostr, "align"_ostr, u"top"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[4]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[4]"_ostr, "y"_ostr, u"423"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[4]/text"_ostr,
                       u"textalignment:0x02"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[5]"_ostr, "align"_ostr, u"top"_ustr);
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[5]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[5]"_ostr, "y"_ostr, u"635"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[5]/text"_ostr,
                       u"textalignment:0x04"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[6]"_ostr, "align"_ostr, u"top"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[6]"_ostr, "x"_ostr, 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[6]"_ostr, "y"_ostr, u"847"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[6]/text"_ostr,
                       u"textalignment:0x06"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[7]"_ostr, "align"_ostr, u"top"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[7]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[7]"_ostr, "y"_ostr, u"1058"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[7]/text"_ostr,
                       u"textalignment:0x08"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[8]"_ostr, "align"_ostr, u"top"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[8]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[8]"_ostr, "y"_ostr, u"1270"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[8]/text"_ostr,
                       u"textalignment:0x0A"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[9]"_ostr, "align"_ostr, u"bottom"_ustr);
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[9]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[9]"_ostr, "y"_ostr, u"1482"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[9]/text"_ostr,
                       u"textalignment:0x0C"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[10]"_ostr, "align"_ostr, u"bottom"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[10]"_ostr, "x"_ostr, 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[10]"_ostr, "y"_ostr, u"1693"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[10]/text"_ostr,
                       u"textalignment:0x0E"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[11]"_ostr, "align"_ostr, u"bottom"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[11]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[11]"_ostr, "y"_ostr, u"1905"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[11]/text"_ostr,
                       u"textalignment:0x10"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[12]"_ostr, "align"_ostr, u"bottom"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[12]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[12]"_ostr, "y"_ostr, u"2117"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[12]/text"_ostr,
                       u"textalignment:0x12"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[13]"_ostr, "align"_ostr, u"bottom"_ustr);
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[13]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[13]"_ostr, "y"_ostr, u"2328"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[13]/text"_ostr,
                       u"textalignment:0x14"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[14]"_ostr, "align"_ostr, u"bottom"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[14]"_ostr, "x"_ostr, 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[14]"_ostr, "y"_ostr, u"2540"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[14]/text"_ostr,
                       u"textalignment:0x16"_ustr);

    assertXPath(pDoc, "/metafile/push[2]/textalign[15]"_ostr, "align"_ostr, u"bottom"_ustr);
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[15]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[15]"_ostr, "y"_ostr, u"2752"_ustr);
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[15]/text"_ostr,
                       u"textalignment:0x18"_ustr);
}

void WmfTest::testSine()
{
    SvFileStream aFileStream(getFullUrl(u"sine_wave.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::ISECTRECTCLIPREGION, false);
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/sectrectclipregion"_ostr, 0);
}

void WmfTest::testEmfProblem()
{
    SvFileStream aFileStream(getFullUrl(u"computer_mail.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::ISECTRECTCLIPREGION, false);
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/sectrectclipregion"_ostr, 2);
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "top"_ostr, u"2125"_ustr);
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "left"_ostr, u"1084"_ustr);
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "bottom"_ostr, u"2927"_ustr);
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "right"_ostr, u"2376"_ustr);
}

void WmfTest::testEmfLineStyles()
{
    SvFileStream aFileStream(getFullUrl(u"line_styles.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::LINE, false);
    dumper.filterActionType(MetaActionType::LINECOLOR, false);
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/line"_ostr, 4);
    assertXPath(pDoc, "/metafile/linecolor"_ostr, 5);

    assertXPath(pDoc, "/metafile/linecolor[1]"_ostr, "color"_ostr, u"#ffffff"_ustr);
    assertXPath(pDoc, "/metafile/linecolor[2]"_ostr, "color"_ostr, u"#00ff00"_ustr);
    assertXPath(pDoc, "/metafile/linecolor[3]"_ostr, "color"_ostr, u"#408080"_ustr);
    assertXPath(pDoc, "/metafile/linecolor[4]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDoc, "/metafile/linecolor[5]"_ostr, "color"_ostr, u"#0000ff"_ustr);

    assertXPath(pDoc, "/metafile/line[1]"_ostr, "style"_ostr, u"dash"_ustr);
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dashlen"_ostr, u"528"_ustr);
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dashcount"_ostr, u"1"_ustr);
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dotlen"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dotcount"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "distance"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "join"_ostr, u"miter"_ustr);
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "cap"_ostr, u"butt"_ustr);

    assertXPath(pDoc, "/metafile/line[2]"_ostr, "style"_ostr, u"dash"_ustr);
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dashlen"_ostr, u"528"_ustr);
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dashcount"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dotlen"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dotcount"_ostr, u"1"_ustr);
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "distance"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "join"_ostr, u"miter"_ustr);
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "cap"_ostr, u"butt"_ustr);

    assertXPath(pDoc, "/metafile/line[3]"_ostr, "style"_ostr, u"dash"_ustr);
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dashlen"_ostr, u"528"_ustr);
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dashcount"_ostr, u"1"_ustr);
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dotlen"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dotcount"_ostr, u"1"_ustr);
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "distance"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "join"_ostr, u"miter"_ustr);
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "cap"_ostr, u"butt"_ustr);

    assertXPath(pDoc, "/metafile/line[4]"_ostr, "style"_ostr, u"dash"_ustr);
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dashlen"_ostr, u"528"_ustr);
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dashcount"_ostr, u"1"_ustr);
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dotlen"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dotcount"_ostr, u"2"_ustr);
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "distance"_ostr, u"176"_ustr);
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "join"_ostr, u"miter"_ustr);
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "cap"_ostr, u"butt"_ustr);
};

void WmfTest::testWorldTransformFontSize()
{
    SvFileStream aFileStream(getFullUrl(u"image1.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::FONT, false);
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/font"_ostr, 9);

    assertXPath(pDoc, "/metafile/font[1]"_ostr, "color"_ostr, u"#595959"_ustr);
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "width"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "height"_ostr, u"389"_ustr);
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "orientation"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "weight"_ostr, u"bold"_ustr);

    assertXPath(pDoc, "/metafile/font[3]"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "width"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "height"_ostr, u"389"_ustr);
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "orientation"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "weight"_ostr, u"bold"_ustr);

    // World transform should not affect font size. Rotating text for 90 degrees
    // should not exchange font width and height.
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "color"_ostr, u"#000000"_ustr);
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "width"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "height"_ostr, u"530"_ustr);
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "orientation"_ostr, u"900"_ustr);
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "weight"_ostr, u"normal"_ustr);
}

void WmfTest::testBigPPI()
{
    // Test that PPI is reduced from 2540 to 96 (width from META_SETWINDOWEXT) to make the graphic
    // bigger
    SvFileStream aFileStream(getFullUrl(u"TestBigPPI.wmf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::FONT, false);
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // If the PPI was not reduced the width and height would be <100 which is too small
    // Related: tdf#150888
    assertXPath(pDoc, "/metafile"_ostr, "width"_ostr, u"2540"_ustr);
    assertXPath(pDoc, "/metafile"_ostr, "height"_ostr, u"2143"_ustr);
}

void WmfTest::testTdf93750()
{
    SvFileStream aFileStream(getFullUrl(u"tdf93750.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/push[1]/comment[2]"_ostr, "datasize"_ostr, u"28"_ustr);
    assertXPath(pDoc, "/metafile/push[1]/comment[3]"_ostr, "datasize"_ostr, u"72"_ustr);
}

void WmfTest::testTdf99402()
{
    // Symbol font should arrive with RTL_TEXTENCODING_SYMBOL encoding,
    // even if charset is OEM_CHARSET/DEFAULT_CHARSET in WMF
    emfio::LOGFONTW logfontw;
    logfontw.lfHeight = 0;
    logfontw.lfWidth = 0;
    logfontw.lfEscapement = 0;
    logfontw.lfWeight = 0;
    logfontw.lfItalic = 0;
    logfontw.lfUnderline = 0;
    logfontw.lfStrikeOut = 0;
    logfontw.lfCharSet = emfio::CharacterSet::OEM_CHARSET;
    logfontw.lfPitchAndFamily = emfio::FamilyFont::FF_ROMAN << 4 | emfio::PitchFont::DEFAULT_PITCH;
    logfontw.alfFaceName = "Symbol";

    emfio::WinMtfFontStyle fontStyle(logfontw);

    CPPUNIT_ASSERT_EQUAL(RTL_TEXTENCODING_SYMBOL, fontStyle.aFont.GetCharSet());
}

void WmfTest::testTdf39894Wmf()
{
    SvFileStream aFileStream(getFullUrl(u"tdf39894.wmf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The x position of the second text must take into account
    // the previous text's last Dx (previously was ~300)
    auto x = getXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr);
    CPPUNIT_ASSERT_GREATER(sal_Int32(2700), x.toInt32());
}

void WmfTest::testTdf39894Emf()
{
    SvFileStream aFileStream(getFullUrl(u"tdf39894.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The x position of the second text must take into account
    // the previous text's last Dx (previously was ~300)
    auto x = getXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr);
    CPPUNIT_ASSERT_GREATER(sal_Int32(2700), x.toInt32());
}

void WmfTest::testETO_PDYWmf()
{
    SvFileStream aFileStream(getFullUrl(u"ETO_PDY.wmf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The y position of following text
    // must be smaller than that of previous
    auto y1 = getXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "y"_ostr);
    auto y2 = getXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr);
    auto y3 = getXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr);
    CPPUNIT_ASSERT(y2.toInt32() < y1.toInt32());
    CPPUNIT_ASSERT(y3.toInt32() < y2.toInt32());
}

void WmfTest::testETO_PDYEmf()
{
    SvFileStream aFileStream(getFullUrl(u"ETO_PDY.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The y position of following text
    // must be smaller than that of previous
    auto y1 = getXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "y"_ostr);
    auto y2 = getXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr);
    auto y3 = getXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr);
    CPPUNIT_ASSERT(y2.toInt32() < y1.toInt32());
    CPPUNIT_ASSERT(y3.toInt32() < y2.toInt32());
}

void WmfTest::testStockObject()
{
    SvFileStream aFileStream(getFullUrl(u"stockobject.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    //   Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/metafile/push[2]/fillcolor[2]' number of nodes is incorrect
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[2]"_ostr, "color"_ostr, u"#000000"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(WmfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
