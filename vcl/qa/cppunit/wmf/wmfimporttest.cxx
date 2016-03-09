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

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <test/mtfxmldump.hxx>
#include <vcl/wmf.hxx>
#include <vcl/metaact.hxx>

using namespace css;

class WmfTest : public test::BootstrapFixture, public XmlTestTools
{
    OUString maDataUrl;

    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

public:
    WmfTest() :
        BootstrapFixture(true, false),
        maDataUrl("/vcl/qa/cppunit/wmf/data/")
    {}

    void testNonPlaceableWmf();
    void testSine();
    void testEmfProblem();
    void testEmfLineStyles();
    void testWorldTransformFontSize();
    void testTdf93750();

    CPPUNIT_TEST_SUITE(WmfTest);
    CPPUNIT_TEST(testNonPlaceableWmf);
    CPPUNIT_TEST(testSine);
    CPPUNIT_TEST(testEmfProblem);
    CPPUNIT_TEST(testEmfLineStyles);
    CPPUNIT_TEST(testWorldTransformFontSize);
    CPPUNIT_TEST(testTdf93750);

    CPPUNIT_TEST_SUITE_END();
};

void WmfTest::testNonPlaceableWmf()
{
    SvFileStream aFileStream(getFullUrl("visio_import_source.wmf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::POLYLINE, false);
    xmlDocPtr pDoc = dumper.dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", "16798");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", "1003");

    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "x", "16798");
    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "y", "7507");

    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "x", "26090");
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "y", "7507");

    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "x", "26090");
    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "y", "1003");

    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "x", "16798");
    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "y", "1003");
}

void WmfTest::testSine()
{
    SvFileStream aFileStream(getFullUrl("sine_wave.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::ISECTRECTCLIPREGION, false);
    xmlDocPtr pDoc = dumper.dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/sectrectclipregion", 0);
}

void WmfTest::testEmfProblem()
{
    SvFileStream aFileStream(getFullUrl("computer_mail.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::ISECTRECTCLIPREGION, false);
    xmlDocPtr pDoc = dumper.dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "top", "427");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "left", "740");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "bottom", "2823");
    assertXPath(pDoc, "/metafile/sectrectclipregion[1]", "right", "1876");
}

void WmfTest::testEmfLineStyles()
{
    SvFileStream aFileStream(getFullUrl("line_styles.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::LINE, false);
    dumper.filterActionType(MetaActionType::LINECOLOR, false);
    xmlDocPtr pDoc = dumper.dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

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
    SvFileStream aFileStream(getFullUrl("image1.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::FONT, false);
    xmlDocPtr pDoc = dumper.dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/font", 8);

    assertXPath(pDoc, "/metafile/font[1]", "color", "#595959");
    assertXPath(pDoc, "/metafile/font[1]", "width", "0");
    assertXPath(pDoc, "/metafile/font[1]", "height", "389");
    assertXPath(pDoc, "/metafile/font[1]", "orientation", "0");
    assertXPath(pDoc, "/metafile/font[1]", "weight", "bold");

    // World transform should not affect font size. Rotating text for 90 degrees
    // should not exchange font width and height.
    assertXPath(pDoc, "/metafile/font[3]", "color", "#000000");
    assertXPath(pDoc, "/metafile/font[3]", "width", "0");
    assertXPath(pDoc, "/metafile/font[3]", "height", "530");
    assertXPath(pDoc, "/metafile/font[3]", "orientation", "900");
    assertXPath(pDoc, "/metafile/font[3]", "weight", "normal");
}

void WmfTest::testTdf93750()
{
    SvFileStream aFileStream(getFullUrl("tdf93750.emf"), StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    xmlDocPtr pDoc = dumper.dumpAndParse(aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/push[1]/comment[2]", "datasize", "28");
    assertXPath(pDoc, "/metafile/push[1]/comment[3]", "datasize", "72");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WmfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
