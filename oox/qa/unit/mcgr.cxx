/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers tests for multi-color gradient (MCGR) feature, available since LO 7.6.0.
class TestMCGR : public UnoApiTest
{
public:
    TestMCGR()
        : UnoApiTest(u"/oox/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(TestMCGR, testFontworkColorGradient)
{
    // Given a document with three-color gradient on a Fontwork.
    loadFromFile(u"MCGR_FontworkColorGradient.fodp");
    // Save it to PPTX
    save(TestFilter::PPTX);
    // And make sure a multi-color gradient fill is exported.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // linear gradient with 30deg angle
    assertXPath(pXmlDoc, "//a:r/a:rPr/a:gradFill/a:lin", "ang", u"3600000");
    // three color stops, no transparency
    static constexpr OString sPath = "//a:r/a:rPr/a:gradFill/a:gsLst/"_ostr;
    assertXPath(pXmlDoc, sPath + "a:gs", 3);
    assertXPath(pXmlDoc, sPath + "a:gs[1]", "pos", u"0");
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[1]/a:srgbClr", "val", u"ff1493");
    assertXPath(pXmlDoc, sPath + "a:gs[2]", "pos", u"30000");
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[2]/a:srgbClr", "val", u"ffff00");
    assertXPath(pXmlDoc, sPath + "a:gs[3]", "pos", u"100000");
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[3]/a:srgbClr", "val", u"00ffff");
}

CPPUNIT_TEST_FIXTURE(TestMCGR, testFontworkColorGradientWord)
{
    // Fontwork is handled different in Word than in PowerPoint documents. So we need a separate
    // test for a text document.
    // Given a document with three-color gradient on a Fontwork.
    loadFromFile(u"MCGR_FontworkColorGradient.fodt");
    // Save it to DOCX
    save(TestFilter::DOCX);
    // And make sure a multi-color gradient fill is exported.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // linear gradient with 30deg angle
    assertXPath(pXmlDoc, "//w14:lin", "ang", u"3600000");
    // three color stops, no transparency
    static constexpr OString sPath = "//w14:gradFill/w14:gsLst/"_ostr;
    assertXPath(pXmlDoc, sPath + "w14:gs", 3);
    assertXPath(pXmlDoc, sPath + "w14:gs[1]", "pos", u"0");
    assertXPathInsensitive(pXmlDoc, sPath + "w14:gs[1]/w14:srgbClr", "val", u"ff1493");
    assertXPath(pXmlDoc, sPath + "w14:gs[2]", "pos", u"30000");
    assertXPathInsensitive(pXmlDoc, sPath + "w14:gs[2]/w14:srgbClr", "val", u"ffff00");
    assertXPath(pXmlDoc, sPath + "w14:gs[3]", "pos", u"100000");
    assertXPathInsensitive(pXmlDoc, sPath + "w14:gs[3]/w14:srgbClr", "val", u"00ffff");
}

CPPUNIT_TEST_FIXTURE(TestMCGR, testTdf155825_SourcOffsetRangeDifferent)
{
    // Color gradient has offset range [0,0.8] and transparency gradient has offset range [0,1].
    loadFromFile(u"tdf155825_MCGR_SourceOffsetRangeDifferent.fodp");
    // Save it to PPTX
    // Without fix, a debug-build would have crashed in oox/source/export/drawingml.cxx from
    // assert(false && "oox::WriteGradientFill: non-synchronized gradients (!)");
    save(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(TestMCGR, testStepCount)
{
    // Given a document with two-color gradient with StepCount 4.
    loadFromFile(u"tdf155852_MCGR_StepCount4.fodp");
    // Save it to PPTX
    save(TestFilter::PPTX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);

    // Without the fix the colors in the sections were wrong. And when opening a file with StepCount
    // and saving it immediately to pptx, a continuous gradient might be produced.

    static constexpr OString sPath = "//a:gradFill/a:gsLst/"_ostr;
    // The default way of load and save would have produced 2 stops, but we need start stop, end stop
    // and 3*2 inner stops.
    assertXPath(pXmlDoc, sPath + "a:gs", 8);
    // A sharp color changes needs a pair of two stops with same offset.
    assertXPath(pXmlDoc, sPath + "a:gs[@pos='25000']", 2);
    assertXPath(pXmlDoc, sPath + "a:gs[@pos='50000']", 2);
    assertXPath(pXmlDoc, sPath + "a:gs[@pos='75000']", 2);
    // Without fix the color was 808080.
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[@pos='75000'][1]/a:srgbClr", "val", u"55aaaa");
    // Without fix the color was 40bfbf, producing a gradient in the last segment.
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[@pos='75000'][2]/a:srgbClr", "val", u"00ffff");
}

CPPUNIT_TEST_FIXTURE(TestMCGR, testAxialColorLinearTrans)
{
    // Given a document with a shape with axial color gradient from inside red to outside cyan and
    // two-stop linear transparency gradient from start 80% to end 0%.
    loadFromFile(u"tdf155827_MCGR_AxialColorLinearTrans.fodp");
    // Save it to PPTX
    save(TestFilter::PPTX);
    // OOXML has transparency together with color. Transparency is stored as opacity.
    // Expected: pos 0 #00ffff 20000, pos 50000 #ff0000 60000, pos 100000 #00ffff 100000.
    // Because of conversion through gray color the opacity values are not exact. If rounding
    // method will be changed, the test needs to be adjusted.

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString sPath = "//a:gradFill/a:gsLst/"_ostr;
    assertXPath(pXmlDoc, sPath + "a:gs", 3);
    assertXPath(pXmlDoc, sPath + "a:gs[1]", "pos", u"0");
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[1]/a:srgbClr", "val", u"00ffff");
    assertXPath(pXmlDoc, sPath + "a:gs[1]/a:srgbClr/a:alpha", "val", u"20000");
    assertXPath(pXmlDoc, sPath + "a:gs[2]", "pos", u"50000");
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[2]/a:srgbClr", "val", u"ff0000");
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[2]/a:srgbClr/a:alpha", "val", u"60396");
    assertXPath(pXmlDoc, sPath + "a:gs[3]", "pos", u"100000");
    assertXPathInsensitive(pXmlDoc, sPath + "a:gs[3]/a:srgbClr", "val", u"00ffff");
    // no <a:alpha> element for default val="100000"
    assertXPath(pXmlDoc, sPath + "a:gs[3]/a:srgbClr/a:alpha", 0);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
