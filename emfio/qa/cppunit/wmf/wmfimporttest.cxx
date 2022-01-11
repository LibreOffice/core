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

    void testNonPlaceableWmf();
    void testTdf88163NonPlaceableWmf();
    void testTdf88163PlaceableWmf();
    void testSine();
    void testEmfProblem();
    void testEmfLineStyles();
    void testWorldTransformFontSize();
    void testTdf93750();
    void testTdf99402();
    void testTdf39894();
    void testETO_PDY();

    CPPUNIT_TEST_SUITE(WmfTest);
    CPPUNIT_TEST(testNonPlaceableWmf);
    CPPUNIT_TEST(testTdf88163NonPlaceableWmf);
    CPPUNIT_TEST(testTdf88163PlaceableWmf);
    CPPUNIT_TEST(testSine);
    CPPUNIT_TEST(testEmfProblem);
    CPPUNIT_TEST(testEmfLineStyles);
    CPPUNIT_TEST(testWorldTransformFontSize);
    CPPUNIT_TEST(testTdf93750);
    CPPUNIT_TEST(testTdf99402);
    CPPUNIT_TEST(testTdf39894);
    CPPUNIT_TEST(testETO_PDY);
    CPPUNIT_TEST_SUITE_END();
};

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
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", "16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", "1004");

    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "x", "16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "y", "7514");

    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "x", "26112");
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "y", "7514");

    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "x", "26112");
    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "y", "1004");

    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "x", "16813");
    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "y", "1004");
}

void WmfTest::testTdf88163NonPlaceableWmf()
{
    OUString fileName(u"tdf88163-non-placeable.wmf");
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // These values come from the fix for tdf#88163

    // Font 'Roman' and its height can vary according to the platform
    // Fails without the fix
    // Linux:   With fix: 3136, without fix: ~ 8000
    // Mac:     With fix: 3230, without fix: ~ 8000
    // Windows: With fix: 3303, without fix: ~ 8000
    auto x = getXPath(pDoc, "/metafile/push[2]/font[1]", "height");
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(3000), x.toInt32());
    CPPUNIT_ASSERT_LESS(sal_Int32(3500), x.toInt32());

    // Fails without the fix: Expected: 7359, Actual: 7336
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "x", "7359");
    // Fails without the fix: Expected: 4118, Actual: 4104
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "y", "4118");

    // Fails without the fix: Expected: 5989, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "x", "5989");
    // Fails without the fix: Expected: 16264, Actual: 16208
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "y", "16264");

    // Fails without the fix: Expected: 20769, Actual: 20705
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "x", "20769");
    // Fails without the fix: Expected: 4077, Actual: 4062
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "y", "4077");
}

void WmfTest::testTdf88163PlaceableWmf()
{
    OUString fileName(u"tdf88163-wrong-font-size.wmf");
    SvFileStream aFileStream(getFullUrl(fileName), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;

    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    // These values come from the fix for tdf#88163

    // Font 'Roman' and its height can vary according to the platform
    // The fix does not affect the font size
    // Linux: 300 - Mac: 309 - Windows: 316
    auto x = getXPath(pDoc, "/metafile/push[2]/font[1]", "height");
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(287), x.toInt32());
    CPPUNIT_ASSERT_LESS(sal_Int32(320), x.toInt32());

    // Fails without the fix: Expected: 1900, Actual: 19818
    assertXPath(pDoc, "/metafile", "height", "1900");

    // Fails without the fix: Expected: 704, Actual: 7336
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "x", "704");
    // Fails without the fix: Expected: 394, Actual: 4110
    assertXPath(pDoc, "/metafile/push[2]/textarray[1]", "y", "394");

    // Fails without the fix: Expected: 573, Actual: 5971
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "x", "573");
    // Fails without the fix: Expected: 1556, Actual: 16230
    assertXPath(pDoc, "/metafile/push[2]/textarray[2]", "y", "1556");

    // Fails without the fix: Expected: 1987, Actual: 20706
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "x", "1987");
    // Fails without the fix: Expected: 390, Actual: 4068
    assertXPath(pDoc, "/metafile/push[2]/textarray[3]", "y", "390");
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
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "top", "2125");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "left", "1084");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "bottom", "2927");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "right", "2376");
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

    assertXPath(pDoc, "/metafile/linecolor[1]", "color", "#ffffff");
    assertXPath(pDoc, "/metafile/linecolor[2]", "color", "#00ff00");
    assertXPath(pDoc, "/metafile/linecolor[3]", "color", "#408080");
    assertXPath(pDoc, "/metafile/linecolor[4]", "color", "#ff0000");
    assertXPath(pDoc, "/metafile/linecolor[5]", "color", "#0000ff");

    assertXPath(pDoc, "/metafile/line[1]", "style", "dash");
    assertXPath(pDoc, "/metafile/line[1]", "dashlen", "528");
    assertXPath(pDoc, "/metafile/line[1]", "dashcount", "1");
    assertXPath(pDoc, "/metafile/line[1]", "dotlen", "176");
    assertXPath(pDoc, "/metafile/line[1]", "dotcount", "0");
    assertXPath(pDoc, "/metafile/line[1]", "distance", "176");
    assertXPath(pDoc, "/metafile/line[1]", "join", "miter");
    assertXPath(pDoc, "/metafile/line[1]", "cap", "butt");

    assertXPath(pDoc, "/metafile/line[2]", "style", "dash");
    assertXPath(pDoc, "/metafile/line[2]", "dashlen", "528");
    assertXPath(pDoc, "/metafile/line[2]", "dashcount", "0");
    assertXPath(pDoc, "/metafile/line[2]", "dotlen", "176");
    assertXPath(pDoc, "/metafile/line[2]", "dotcount", "1");
    assertXPath(pDoc, "/metafile/line[2]", "distance", "176");
    assertXPath(pDoc, "/metafile/line[2]", "join", "miter");
    assertXPath(pDoc, "/metafile/line[2]", "cap", "butt");

    assertXPath(pDoc, "/metafile/line[3]", "style", "dash");
    assertXPath(pDoc, "/metafile/line[3]", "dashlen", "528");
    assertXPath(pDoc, "/metafile/line[3]", "dashcount", "1");
    assertXPath(pDoc, "/metafile/line[3]", "dotlen", "176");
    assertXPath(pDoc, "/metafile/line[3]", "dotcount", "1");
    assertXPath(pDoc, "/metafile/line[3]", "distance", "176");
    assertXPath(pDoc, "/metafile/line[3]", "join", "miter");
    assertXPath(pDoc, "/metafile/line[3]", "cap", "butt");

    assertXPath(pDoc, "/metafile/line[4]", "style", "dash");
    assertXPath(pDoc, "/metafile/line[4]", "dashlen", "528");
    assertXPath(pDoc, "/metafile/line[4]", "dashcount", "1");
    assertXPath(pDoc, "/metafile/line[4]", "dotlen", "176");
    assertXPath(pDoc, "/metafile/line[4]", "dotcount", "2");
    assertXPath(pDoc, "/metafile/line[4]", "distance", "176");
    assertXPath(pDoc, "/metafile/line[4]", "join", "miter");
    assertXPath(pDoc, "/metafile/line[4]", "cap", "butt");
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

    assertXPath(pDoc, "/metafile/font[1]", "color", "#595959");
    assertXPath(pDoc, "/metafile/font[1]", "width", "0");
    assertXPath(pDoc, "/metafile/font[1]", "height", "389");
    assertXPath(pDoc, "/metafile/font[1]", "orientation", "0");
    assertXPath(pDoc, "/metafile/font[1]", "weight", "bold");

    assertXPath(pDoc, "/metafile/font[3]", "color", "#000000");
    assertXPath(pDoc, "/metafile/font[3]", "width", "0");
    assertXPath(pDoc, "/metafile/font[3]", "height", "389");
    assertXPath(pDoc, "/metafile/font[3]", "orientation", "0");
    assertXPath(pDoc, "/metafile/font[3]", "weight", "bold");

    // World transform should not affect font size. Rotating text for 90 degrees
    // should not exchange font width and height.
    assertXPath(pDoc, "/metafile/font[4]", "color", "#000000");
    assertXPath(pDoc, "/metafile/font[4]", "width", "0");
    assertXPath(pDoc, "/metafile/font[4]", "height", "530");
    assertXPath(pDoc, "/metafile/font[4]", "orientation", "900");
    assertXPath(pDoc, "/metafile/font[4]", "weight", "normal");
}

void WmfTest::testTdf93750()
{
    SvFileStream aFileStream(getFullUrl(u"tdf93750.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/metafile/push[1]/comment[2]", "datasize", "28");
    assertXPath(pDoc, "/metafile/push[1]/comment[3]", "datasize", "72");
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
    logfontw.lfCharSet = OEM_CHARSET;
    logfontw.lfPitchAndFamily = FF_ROMAN | DEFAULT_PITCH;
    logfontw.alfFaceName = "Symbol";

    emfio::WinMtfFontStyle fontStyle(logfontw);

    CPPUNIT_ASSERT_EQUAL(RTL_TEXTENCODING_SYMBOL, fontStyle.aFont.GetCharSet());
}

void WmfTest::testTdf39894()
{
    OUString files[] = { "tdf39894.wmf", "tdf39894.emf" };
    for (const auto& file : files)
    {
        SvFileStream aFileStream(getFullUrl(file), StreamMode::READ);
        GDIMetaFile aGDIMetaFile;
        ReadWindowMetafile(aFileStream, aGDIMetaFile);

        MetafileXmlDump dumper;
        xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

        CPPUNIT_ASSERT(pDoc);

        // The x position of the second text must take into account
        // the previous text's last Dx (previously was ~300)
        auto x = getXPath(pDoc, "/metafile/push[2]/textarray[2]", "x");
        CPPUNIT_ASSERT_GREATER(sal_Int32(2700), x.toInt32());
    }
}

void WmfTest::testETO_PDY()
{
    OUString files[] = { "ETO_PDY.wmf", "ETO_PDY.emf" };
    for (const auto& file : files)
    {
        SvFileStream aFileStream(getFullUrl(file), StreamMode::READ);
        GDIMetaFile aGDIMetaFile;
        ReadWindowMetafile(aFileStream, aGDIMetaFile);

        MetafileXmlDump dumper;
        xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

        CPPUNIT_ASSERT(pDoc);

        // The y position of following text
        // must be smaller than that of previous
        auto y1 = getXPath(pDoc, "/metafile/push[2]/textarray[1]", "y");
        auto y2 = getXPath(pDoc, "/metafile/push[2]/textarray[2]", "y");
        auto y3 = getXPath(pDoc, "/metafile/push[2]/textarray[3]", "y");
        CPPUNIT_ASSERT_MESSAGE(file.toUtf8().getStr(), y2.toInt32() < y1.toInt32());
        CPPUNIT_ASSERT_MESSAGE(file.toUtf8().getStr(), y3.toInt32() < y2.toInt32());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(WmfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
