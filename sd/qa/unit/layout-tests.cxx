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

    CPPUNIT_TEST_SUITE(SdLayoutTest);

    CPPUNIT_TEST(testTdf104722);

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

CPPUNIT_TEST_SUITE_REGISTRATION(SdLayoutTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
