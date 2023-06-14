/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers tests for multi-color gradient (MCGR) feature, available since LO 7.6.0.
class TestMCGR : public UnoApiXmlTest
{
public:
    TestMCGR()
        : UnoApiXmlTest("/oox/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(TestMCGR, testFontworkColorGradient)
{
    // Given a document with three-color gradient on a Fontwork.
    loadFromURL(u"MCGR_FontworkColorGradient.fodp");
    // Save it to PPTX
    save("Impress Office Open XML");
    // And make sure a multi-color gradient fill is exported.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/slides/slide1.xml");
    // linear gradient with 30deg angle
    assertXPath(pXmlDoc, "//a:r/a:rPr/a:gradFill/a:lin", "ang", "3600000");
    // three color stops, no transparency
    const OString sPath = "//a:r/a:rPr/a:gradFill/a:gsLst/";
    assertXPath(pXmlDoc, sPath + "a:gs", 3);
    assertXPath(pXmlDoc, sPath + "a:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sPath + "a:gs[1]/a:srgbClr", "val", "ff1493");
    assertXPath(pXmlDoc, sPath + "a:gs[2]", "pos", "30000");
    assertXPath(pXmlDoc, sPath + "a:gs[2]/a:srgbClr", "val", "ffff00");
    assertXPath(pXmlDoc, sPath + "a:gs[3]", "pos", "100000");
    assertXPath(pXmlDoc, sPath + "a:gs[3]/a:srgbClr", "val", "00ffff");
}

CPPUNIT_TEST_FIXTURE(TestMCGR, testFontworkColorGradientWord)
{
    // Fontwork is handled different in Word than in PowerPoint documents. So we need a separate
    // test for a text document.
    // Given a document with three-color gradient on a Fontwork.
    loadFromURL(u"MCGR_FontworkColorGradient.fodt");
    // Save it to DOCX
    save("Office Open XML Text");
    // And make sure a multi-color gradient fill is exported.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // linear gradient with 30deg angle
    assertXPath(pXmlDoc, "//w14:lin", "ang", "3600000");
    // three color stops, no transparency
    const OString sPath = "//w14:gradFill/w14:gsLst/";
    assertXPath(pXmlDoc, sPath + "w14:gs", 3);
    assertXPath(pXmlDoc, sPath + "w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sPath + "w14:gs[1]/w14:srgbClr", "val", "ff1493");
    assertXPath(pXmlDoc, sPath + "w14:gs[2]", "pos", "30000");
    assertXPath(pXmlDoc, sPath + "w14:gs[2]/w14:srgbClr", "val", "ffff00");
    assertXPath(pXmlDoc, sPath + "w14:gs[3]", "pos", "100000");
    assertXPath(pXmlDoc, sPath + "w14:gs[3]/w14:srgbClr", "val", "00ffff");
}

CPPUNIT_TEST_FIXTURE(TestMCGR, testTdf155825_SourcOffsetRangeDifferent)
{
    // Color gradient has offset range [0,0.8] and transparency gradient has offset range [0,1].
    loadFromURL(u"tdf155825_MCGR_SourceOffsetRangeDifferent.fodp");
    // Save it to PPTX
    // Without fix, a debug-build would have crashed in oox/source/export/drawingml.cxx from
    // assert(false && "oox::WriteGradientFill: non-synchronized gradients (!)");
    save("Impress Office Open XML");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */