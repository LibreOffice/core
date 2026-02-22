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
#include <tools/stream.hxx>
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

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/metafile/push", 2);
    assertXPath(pDoc, "/metafile/push[2]", "flags", u"PushClipRegion");
    assertXPath(pDoc, "/metafile/push[2]/fillcolor", 2);
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[1]", "color", u"#000000");
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[2]", "color", u"#d0d0d0");
    assertXPath(pDoc, "/metafile/push[2]/linecolor", 60);
    assertXPath(pDoc, "/metafile/push[2]/polyline", 209);
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point", 5);
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point[3]", "x", u"16906");
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point[3]", "y", u"12673");
    assertXPath(pDoc, "/metafile/push[2]/textarray", 307);
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
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", u"16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", u"1004");

    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "x", u"16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "y", u"7514");

    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "x", u"26112");
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "y", u"7514");

    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "x", u"26112");
    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "y", u"1004");

    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "x", u"16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "y", u"1004");
}

void WmfTest::testTdf88163NonPlaceableWmf()
{
    OUString fileName(u"tdf88163-non-placeable.wmf"_ustr);
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // These values come from the fix for tdf#88163

    // Fails without the fix
    // With fix: 3272, without fix: ~ 8000
    auto x = getXPath(pDoc, "/metafile/push[2]/font[1]", "height");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3272), x.toInt32());

    // Fails without the fix: Expected: 7359, Actual: 7336
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "x", u"7359");
    // Fails without the fix: Expected: 4118, Actual: 4104
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "y", u"4118");

    // Fails without the fix: Expected: 5989, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "x", u"5989");
    // Fails without the fix: Expected: 16264, Actual: 16208
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "y", u"16264");

    // Fails without the fix: Expected: 20769, Actual: 20705
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "x", u"20769");
    // Fails without the fix: Expected: 4077, Actual: 4062
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "y", u"4077");
}

void WmfTest::testTdf88163PlaceableWmf()
{
    OUString fileName(u"tdf88163-wrong-font-size.wmf"_ustr);
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // These values come from the fix for tdf#88163

    // The fix does not affect the font size
    auto x = getXPath(pDoc, "/metafile/push[2]/font[1]", "height");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(313), x.toInt32());

    // Fails without the fix: Expected: 1900, Actual: 19818
    assertXPath(pDoc, "/metafile", "height", u"1900");

    // Fails without the fix: Expected: 704, Actual: 7336
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "x", u"704");
    // Fails without the fix: Expected: 394, Actual: 4110
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "y", u"394");

    // Fails without the fix: Expected: 573, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "x", u"573");
    // Fails without the fix: Expected: 1556, Actual: 16230
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "y", u"1556");

    // Fails without the fix: Expected: 1987, Actual: 20706
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "x", u"1987");
    // Fails without the fix: Expected: 390, Actual: 4068
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "y", u"390");
}

void WmfTest::testSetTextAlignWmf()
{
    OUString fileName(u"TestSetTextAlign.wmf"_ustr);
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile", "height", u"20999");

    assertXPath(pDoc, "/metafile/push[2]/textalign[2]", "align", u"top");
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "x", u"11642");
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "y", u"212");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[2]/text", u"textalignment:default");

    assertXPath(pDoc, "/metafile/push[2]/textalign[3]", "align", u"top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[3]", "x", 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "y", u"212");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[3]/text", u"textalignment:0x00");

    assertXPath(pDoc, "/metafile/push[2]/textalign[4]", "align", u"top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[4]", "x", 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[4]", "y", u"423");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[4]/text", u"textalignment:0x02");

    assertXPath(pDoc, "/metafile/push[2]/textalign[5]", "align", u"top");
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[5]", "x", 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[5]", "y", u"635");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[5]/text", u"textalignment:0x04");

    assertXPath(pDoc, "/metafile/push[2]/textalign[6]", "align", u"top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[6]", "x", 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[6]", "y", u"847");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[6]/text", u"textalignment:0x06");

    assertXPath(pDoc, "/metafile/push[2]/textalign[7]", "align", u"top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[7]", "x", 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[7]", "y", u"1058");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[7]/text", u"textalignment:0x08");

    assertXPath(pDoc, "/metafile/push[2]/textalign[8]", "align", u"top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[8]", "x", 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[8]", "y", u"1270");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[8]/text", u"textalignment:0x0A");

    assertXPath(pDoc, "/metafile/push[2]/textalign[9]", "align", u"bottom");
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[9]", "x", 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[9]", "y", u"1482");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[9]/text", u"textalignment:0x0C");

    assertXPath(pDoc, "/metafile/push[2]/textalign[10]", "align", u"bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[10]", "x", 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[10]", "y", u"1693");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[10]/text", u"textalignment:0x0E");

    assertXPath(pDoc, "/metafile/push[2]/textalign[11]", "align", u"bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[11]", "x", 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[11]", "y", u"1905");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[11]/text", u"textalignment:0x10");

    assertXPath(pDoc, "/metafile/push[2]/textalign[12]", "align", u"bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[12]", "x", 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[12]", "y", u"2117");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[12]/text", u"textalignment:0x12");

    assertXPath(pDoc, "/metafile/push[2]/textalign[13]", "align", u"bottom");
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[13]", "x", 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[13]", "y", u"2328");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[13]/text", u"textalignment:0x14");

    assertXPath(pDoc, "/metafile/push[2]/textalign[14]", "align", u"bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[14]", "x", 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[14]", "y", u"2540");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[14]/text", u"textalignment:0x16");

    assertXPath(pDoc, "/metafile/push[2]/textalign[15]", "align", u"bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[15]", "x", 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[15]", "y", u"2752");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[15]/text", u"textalignment:0x18");
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

    assertXPath(pDoc, "/metafile/sectrectclipregion", 0);
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

    assertXPath(pDoc, "/metafile/sectrectclipregion", 2);
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "top", u"2125");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "left", u"1084");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "bottom", u"2927");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "right", u"2376");
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

    assertXPath(pDoc, "/metafile/line", 4);
    assertXPath(pDoc, "/metafile/linecolor", 5);

    assertXPath(pDoc, "/metafile/linecolor[1]", "color", u"#ffffff");
    assertXPath(pDoc, "/metafile/linecolor[2]", "color", u"#00ff00");
    assertXPath(pDoc, "/metafile/linecolor[3]", "color", u"#408080");
    assertXPath(pDoc, "/metafile/linecolor[4]", "color", u"#ff0000");
    assertXPath(pDoc, "/metafile/linecolor[5]", "color", u"#0000ff");

    assertXPath(pDoc, "/metafile/line[1]", "style", u"dash");
    assertXPath(pDoc, "/metafile/line[1]", "dashlen", u"528");
    assertXPath(pDoc, "/metafile/line[1]", "dashcount", u"1");
    assertXPath(pDoc, "/metafile/line[1]", "dotlen", u"176");
    assertXPath(pDoc, "/metafile/line[1]", "dotcount", u"0");
    assertXPath(pDoc, "/metafile/line[1]", "distance", u"176");
    assertXPath(pDoc, "/metafile/line[1]", "join", u"miter");
    assertXPath(pDoc, "/metafile/line[1]", "cap", u"butt");

    assertXPath(pDoc, "/metafile/line[2]", "style", u"dash");
    assertXPath(pDoc, "/metafile/line[2]", "dashlen", u"528");
    assertXPath(pDoc, "/metafile/line[2]", "dashcount", u"0");
    assertXPath(pDoc, "/metafile/line[2]", "dotlen", u"176");
    assertXPath(pDoc, "/metafile/line[2]", "dotcount", u"1");
    assertXPath(pDoc, "/metafile/line[2]", "distance", u"176");
    assertXPath(pDoc, "/metafile/line[2]", "join", u"miter");
    assertXPath(pDoc, "/metafile/line[2]", "cap", u"butt");

    assertXPath(pDoc, "/metafile/line[3]", "style", u"dash");
    assertXPath(pDoc, "/metafile/line[3]", "dashlen", u"528");
    assertXPath(pDoc, "/metafile/line[3]", "dashcount", u"1");
    assertXPath(pDoc, "/metafile/line[3]", "dotlen", u"176");
    assertXPath(pDoc, "/metafile/line[3]", "dotcount", u"1");
    assertXPath(pDoc, "/metafile/line[3]", "distance", u"176");
    assertXPath(pDoc, "/metafile/line[3]", "join", u"miter");
    assertXPath(pDoc, "/metafile/line[3]", "cap", u"butt");

    assertXPath(pDoc, "/metafile/line[4]", "style", u"dash");
    assertXPath(pDoc, "/metafile/line[4]", "dashlen", u"528");
    assertXPath(pDoc, "/metafile/line[4]", "dashcount", u"1");
    assertXPath(pDoc, "/metafile/line[4]", "dotlen", u"176");
    assertXPath(pDoc, "/metafile/line[4]", "dotcount", u"2");
    assertXPath(pDoc, "/metafile/line[4]", "distance", u"176");
    assertXPath(pDoc, "/metafile/line[4]", "join", u"miter");
    assertXPath(pDoc, "/metafile/line[4]", "cap", u"butt");
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

    assertXPath(pDoc, "/metafile/font", 9);

    assertXPath(pDoc, "/metafile/font[1]", "color", u"#595959");
    assertXPath(pDoc, "/metafile/font[1]", "width", u"0");
    assertXPath(pDoc, "/metafile/font[1]", "height", u"389");
    assertXPath(pDoc, "/metafile/font[1]", "orientation", u"0");
    assertXPath(pDoc, "/metafile/font[1]", "weight", u"bold");

    assertXPath(pDoc, "/metafile/font[3]", "color", u"#000000");
    assertXPath(pDoc, "/metafile/font[3]", "width", u"0");
    assertXPath(pDoc, "/metafile/font[3]", "height", u"389");
    assertXPath(pDoc, "/metafile/font[3]", "orientation", u"0");
    assertXPath(pDoc, "/metafile/font[3]", "weight", u"bold");

    // World transform should not affect font size. Rotating text for 90 degrees
    // should not exchange font width and height.
    assertXPath(pDoc, "/metafile/font[4]", "color", u"#000000");
    assertXPath(pDoc, "/metafile/font[4]", "width", u"0");
    assertXPath(pDoc, "/metafile/font[4]", "height", u"530");
    assertXPath(pDoc, "/metafile/font[4]", "orientation", u"900");
    assertXPath(pDoc, "/metafile/font[4]", "weight", u"normal");
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
    assertXPath(pDoc, "/metafile", "width", u"2540");
    assertXPath(pDoc, "/metafile", "height", u"2143");
}

void WmfTest::testTdf93750()
{
    SvFileStream aFileStream(getFullUrl(u"tdf93750.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/push[1]/comment[2]", "datasize", u"28");
    assertXPath(pDoc, "/metafile/push[1]/comment[3]", "datasize", u"72");
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

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The x position of the second text must take into account
    // the previous text's last Dx (previously was ~300)
    auto x = getXPath(pDoc, "/metafile/push[2]/textarray[2]", "x");
    CPPUNIT_ASSERT_GREATER(sal_Int32(2700), x.toInt32());
}

void WmfTest::testTdf39894Emf()
{
    SvFileStream aFileStream(getFullUrl(u"tdf39894.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The x position of the second text must take into account
    // the previous text's last Dx (previously was ~300)
    auto x = getXPath(pDoc, "/metafile/push[2]/textarray[2]", "x");
    CPPUNIT_ASSERT_GREATER(sal_Int32(2700), x.toInt32());
}

void WmfTest::testETO_PDYWmf()
{
    SvFileStream aFileStream(getFullUrl(u"ETO_PDY.wmf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The y position of following text
    // must be smaller than that of previous
    auto y1 = getXPath(pDoc, "/metafile/push[2]/textarray[1]", "y");
    auto y2 = getXPath(pDoc, "/metafile/push[2]/textarray[2]", "y");
    auto y3 = getXPath(pDoc, "/metafile/push[2]/textarray[3]", "y");
    CPPUNIT_ASSERT(y2.toInt32() < y1.toInt32());
    CPPUNIT_ASSERT(y3.toInt32() < y2.toInt32());
}

void WmfTest::testETO_PDYEmf()
{
    SvFileStream aFileStream(getFullUrl(u"ETO_PDY.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // The y position of following text
    // must be smaller than that of previous
    auto y1 = getXPath(pDoc, "/metafile/push[2]/textarray[1]", "y");
    auto y2 = getXPath(pDoc, "/metafile/push[2]/textarray[2]", "y");
    auto y3 = getXPath(pDoc, "/metafile/push[2]/textarray[3]", "y");
    CPPUNIT_ASSERT(y2.toInt32() < y1.toInt32());
    CPPUNIT_ASSERT(y3.toInt32() < y2.toInt32());
}

void WmfTest::testStockObject()
{
    SvFileStream aFileStream(getFullUrl(u"stockobject.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    xmlDocUniquePtr pDoc = dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    //   Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/metafile/push[2]/fillcolor[2]' number of nodes is incorrect
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[2]", "color", u"#000000");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WmfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
