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

    CPPUNIT_TEST_SUITE(SdLayoutTest);

    CPPUNIT_TEST(testTdf104722);
    CPPUNIT_TEST(testTdf136949);
    CPPUNIT_TEST(testTdf128212);

    CPPUNIT_TEST_SUITE_END();
};

void SdLayoutTest::testTdf104722()
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

void SdLayoutTest::testTdf136949()
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

void SdLayoutTest::testTdf128212()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf128212.pptx"), PPTX);

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

CPPUNIT_TEST_SUITE_REGISTRATION(SdLayoutTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
