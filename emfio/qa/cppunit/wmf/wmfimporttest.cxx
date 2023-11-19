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
        , maDataUrl("/emfio/qa/cppunit/wmf/data/")
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
    assertXPath(pDoc, "/metafile/push[2]"_ostr, "flags"_ostr, "PushClipRegion");
    assertXPath(pDoc, "/metafile/push[2]/fillcolor"_ostr, 2);
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[1]"_ostr, "color"_ostr, "#000000");
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[2]"_ostr, "color"_ostr, "#d0d0d0");
    assertXPath(pDoc, "/metafile/push[2]/linecolor"_ostr, 60);
    assertXPath(pDoc, "/metafile/push[2]/polyline"_ostr, 209);
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point"_ostr, 5);
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point[3]"_ostr, "x"_ostr, "16906");
    assertXPath(pDoc, "/metafile/push[2]/polyline[1]/point[3]"_ostr, "y"_ostr, "12673");
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
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]"_ostr, "x"_ostr, "16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]"_ostr, "y"_ostr, "1004");

    assertXPath(pDoc, "/metafile/polyline[1]/point[2]"_ostr, "x"_ostr, "16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[2]"_ostr, "y"_ostr, "7514");

    assertXPath(pDoc, "/metafile/polyline[1]/point[3]"_ostr, "x"_ostr, "26112");
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]"_ostr, "y"_ostr, "7514");

    assertXPath(pDoc, "/metafile/polyline[1]/point[4]"_ostr, "x"_ostr, "26112");
    assertXPath(pDoc, "/metafile/polyline[1]/point[4]"_ostr, "y"_ostr, "1004");

    assertXPath(pDoc, "/metafile/polyline[1]/point[5]"_ostr, "x"_ostr, "16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[5]"_ostr, "y"_ostr, "1004");
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
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "x"_ostr, "7359");
    // Fails without the fix: Expected: 4118, Actual: 4104
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "y"_ostr, "4118");

    // Fails without the fix: Expected: 5989, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr, "5989");
    // Fails without the fix: Expected: 16264, Actual: 16208
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr, "16264");

    // Fails without the fix: Expected: 20769, Actual: 20705
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "x"_ostr, "20769");
    // Fails without the fix: Expected: 4077, Actual: 4062
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr, "4077");
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
    assertXPath(pDoc, "/metafile"_ostr, "height"_ostr, "1900");

    // Fails without the fix: Expected: 704, Actual: 7336
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "x"_ostr, "704");
    // Fails without the fix: Expected: 394, Actual: 4110
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]"_ostr, "y"_ostr, "394");

    // Fails without the fix: Expected: 573, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr, "573");
    // Fails without the fix: Expected: 1556, Actual: 16230
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr, "1556");

    // Fails without the fix: Expected: 1987, Actual: 20706
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "x"_ostr, "1987");
    // Fails without the fix: Expected: 390, Actual: 4068
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr, "390");
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

    assertXPath(pDoc, "/metafile"_ostr, "height"_ostr, "20999");

    assertXPath(pDoc, "/metafile/push[2]/textalign[2]"_ostr, "align"_ostr, "top");
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "x"_ostr, "11642");
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]"_ostr, "y"_ostr, "212");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[2]/text"_ostr, "textalignment:default");

    assertXPath(pDoc, "/metafile/push[2]/textalign[3]"_ostr, "align"_ostr, "top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]"_ostr, "y"_ostr, "212");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[3]/text"_ostr, "textalignment:0x00");

    assertXPath(pDoc, "/metafile/push[2]/textalign[4]"_ostr, "align"_ostr, "top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[4]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[4]"_ostr, "y"_ostr, "423");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[4]/text"_ostr, "textalignment:0x02");

    assertXPath(pDoc, "/metafile/push[2]/textalign[5]"_ostr, "align"_ostr, "top");
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[5]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[5]"_ostr, "y"_ostr, "635");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[5]/text"_ostr, "textalignment:0x04");

    assertXPath(pDoc, "/metafile/push[2]/textalign[6]"_ostr, "align"_ostr, "top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[6]"_ostr, "x"_ostr, 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[6]"_ostr, "y"_ostr, "847");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[6]/text"_ostr, "textalignment:0x06");

    assertXPath(pDoc, "/metafile/push[2]/textalign[7]"_ostr, "align"_ostr, "top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[7]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[7]"_ostr, "y"_ostr, "1058");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[7]/text"_ostr, "textalignment:0x08");

    assertXPath(pDoc, "/metafile/push[2]/textalign[8]"_ostr, "align"_ostr, "top");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[8]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[8]"_ostr, "y"_ostr, "1270");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[8]/text"_ostr, "textalignment:0x0A");

    assertXPath(pDoc, "/metafile/push[2]/textalign[9]"_ostr, "align"_ostr, "bottom");
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[9]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[9]"_ostr, "y"_ostr, "1482");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[9]/text"_ostr, "textalignment:0x0C");

    assertXPath(pDoc, "/metafile/push[2]/textalign[10]"_ostr, "align"_ostr, "bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[10]"_ostr, "x"_ostr, 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[10]"_ostr, "y"_ostr, "1693");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[10]/text"_ostr, "textalignment:0x0E");

    assertXPath(pDoc, "/metafile/push[2]/textalign[11]"_ostr, "align"_ostr, "bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[11]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[11]"_ostr, "y"_ostr, "1905");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[11]/text"_ostr, "textalignment:0x10");

    assertXPath(pDoc, "/metafile/push[2]/textalign[12]"_ostr, "align"_ostr, "bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[12]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[12]"_ostr, "y"_ostr, "2117");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[12]/text"_ostr, "textalignment:0x12");

    assertXPath(pDoc, "/metafile/push[2]/textalign[13]"_ostr, "align"_ostr, "bottom");
    // Fails without the fix: Expected: 12026, Actual: 12350
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[13]"_ostr, "x"_ostr, 12026, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[13]"_ostr, "y"_ostr, "2328");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[13]/text"_ostr, "textalignment:0x14");

    assertXPath(pDoc, "/metafile/push[2]/textalign[14]"_ostr, "align"_ostr, "bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[14]"_ostr, "x"_ostr, 12363, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[14]"_ostr, "y"_ostr, "2540");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[14]/text"_ostr, "textalignment:0x16");

    assertXPath(pDoc, "/metafile/push[2]/textalign[15]"_ostr, "align"_ostr, "bottom");
    assertXPathDoubleValue(pDoc, "/metafile/push[2]/textarray[15]"_ostr, "x"_ostr, 12700, 30);
    assertXPath(pDoc, "/metafile/push[2]/textarray[15]"_ostr, "y"_ostr, "2752");
    assertXPathContent(pDoc, "/metafile/push[2]/textarray[15]/text"_ostr, "textalignment:0x18");
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
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "top"_ostr, "2125");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "left"_ostr, "1084");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "bottom"_ostr, "2927");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]"_ostr, "right"_ostr, "2376");
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

    assertXPath(pDoc, "/metafile/linecolor[1]"_ostr, "color"_ostr, "#ffffff");
    assertXPath(pDoc, "/metafile/linecolor[2]"_ostr, "color"_ostr, "#00ff00");
    assertXPath(pDoc, "/metafile/linecolor[3]"_ostr, "color"_ostr, "#408080");
    assertXPath(pDoc, "/metafile/linecolor[4]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDoc, "/metafile/linecolor[5]"_ostr, "color"_ostr, "#0000ff");

    assertXPath(pDoc, "/metafile/line[1]"_ostr, "style"_ostr, "dash");
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dashlen"_ostr, "528");
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dashcount"_ostr, "1");
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dotlen"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "dotcount"_ostr, "0");
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "distance"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "join"_ostr, "miter");
    assertXPath(pDoc, "/metafile/line[1]"_ostr, "cap"_ostr, "butt");

    assertXPath(pDoc, "/metafile/line[2]"_ostr, "style"_ostr, "dash");
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dashlen"_ostr, "528");
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dashcount"_ostr, "0");
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dotlen"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "dotcount"_ostr, "1");
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "distance"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "join"_ostr, "miter");
    assertXPath(pDoc, "/metafile/line[2]"_ostr, "cap"_ostr, "butt");

    assertXPath(pDoc, "/metafile/line[3]"_ostr, "style"_ostr, "dash");
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dashlen"_ostr, "528");
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dashcount"_ostr, "1");
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dotlen"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "dotcount"_ostr, "1");
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "distance"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "join"_ostr, "miter");
    assertXPath(pDoc, "/metafile/line[3]"_ostr, "cap"_ostr, "butt");

    assertXPath(pDoc, "/metafile/line[4]"_ostr, "style"_ostr, "dash");
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dashlen"_ostr, "528");
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dashcount"_ostr, "1");
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dotlen"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "dotcount"_ostr, "2");
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "distance"_ostr, "176");
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "join"_ostr, "miter");
    assertXPath(pDoc, "/metafile/line[4]"_ostr, "cap"_ostr, "butt");
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

    assertXPath(pDoc, "/metafile/font[1]"_ostr, "color"_ostr, "#595959");
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "width"_ostr, "0");
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "height"_ostr, "389");
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "orientation"_ostr, "0");
    assertXPath(pDoc, "/metafile/font[1]"_ostr, "weight"_ostr, "bold");

    assertXPath(pDoc, "/metafile/font[3]"_ostr, "color"_ostr, "#000000");
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "width"_ostr, "0");
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "height"_ostr, "389");
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "orientation"_ostr, "0");
    assertXPath(pDoc, "/metafile/font[3]"_ostr, "weight"_ostr, "bold");

    // World transform should not affect font size. Rotating text for 90 degrees
    // should not exchange font width and height.
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "color"_ostr, "#000000");
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "width"_ostr, "0");
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "height"_ostr, "530");
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "orientation"_ostr, "900");
    assertXPath(pDoc, "/metafile/font[4]"_ostr, "weight"_ostr, "normal");
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
    assertXPath(pDoc, "/metafile"_ostr, "width"_ostr, "2540");
    assertXPath(pDoc, "/metafile"_ostr, "height"_ostr, "2143");
}

void WmfTest::testTdf93750()
{
    SvFileStream aFileStream(getFullUrl(u"tdf93750.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/push[1]/comment[2]"_ostr, "datasize"_ostr, "28");
    assertXPath(pDoc, "/metafile/push[1]/comment[3]"_ostr, "datasize"_ostr, "72");
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
    assertXPath(pDoc, "/metafile/push[2]/fillcolor[2]"_ostr, "color"_ostr, "#000000");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WmfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
