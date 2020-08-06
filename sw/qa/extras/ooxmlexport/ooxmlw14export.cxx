/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_TextEffects_GlowShadowReflection, "TextEffects_Glow_Shadow_Reflection.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:rPr/w14:glow", "rad").match("63500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:rPr/w14:glow/w14:srgbClr", "val").match("00B0F0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:rPr/w14:glow/w14:srgbClr/w14:alpha", "val").match("60000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow", "rad").match("228600"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow/w14:schemeClr", "val").match("accent6"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow/w14:schemeClr/w14:alpha", "val").match("60000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow/w14:schemeClr/w14:satMod", "val").match("175000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "blurRad").match("63500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "dist").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "dir").match("1800000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "sx").match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "sy").match("-30000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "kx").match("-800400"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "ky").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow", "algn").match("bl"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow/w14:schemeClr", "val").match("accent3"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow/w14:schemeClr/w14:alpha", "val").match("38000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow/w14:schemeClr/w14:lumMod", "val").match("75000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "blurRad").match("190500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "dist").match("190500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "dir").match("3000000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "sx").match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "sy").match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "kx").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "ky").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow", "algn").match("ctr"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow/w14:srgbClr", "val").match("FF0000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow/w14:srgbClr/w14:alpha", "val").match("10000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "blurRad").match("6350"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "stA").match("60000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "stPos").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "endA").match("900"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "endPos").match("60000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "dist").match("60007"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "dir").match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "fadeDir").match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "sx").match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "sy").match("-100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "kx").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "ky").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection", "algn").match("bl"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "blurRad").match("6350"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "stA").match("55000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "stPos").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "endA").match("300"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "endPos").match("45500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "dist").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "dir").match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "fadeDir").match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "sx").match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "sy").match("-100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "kx").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "ky").match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection", "algn").match("bl"));
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_TextEffects_TextOutline, "TextEffects_TextOutline.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Paragraph 1
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "w", "50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "cap", "rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "cmpd", "dbl");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "algn", "ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", "70000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr", "val", "92D050");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:alpha", "val", "30000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumMod", "val", "75000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumOff", "val", "25000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", "30000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:alpha", "val", "55000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", "60000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]/w14:srgbClr", "val", "0070C0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]", "pos", "100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]/w14:schemeClr", "val", "accent4");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin", "ang", "3600000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin", "scaled", "0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash", "val", "dash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:miter", "lim", "0");

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "w", "9525");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "cap", "rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "cmpd", "sng");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "algn", "ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:solidFill/w14:srgbClr", "val", "FF0000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash", "val", "solid");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:bevel", 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "w", "9525");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "cap", "rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "cmpd", "sng");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "algn", "ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:noFill", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash", "val", "solid");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:bevel", 1);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_TextEffects_TextFill, "TextEffects_TextFill.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Paragraph 1 has no textFill

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textFill/w14:noFill", 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr", "val", "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:alpha", "val", "5000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod", "val", "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumOff", "val", "60000");

    // Paragraph 4
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val", "5000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val", "67000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", "50000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr", "val", "00B0F0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:alpha", "val", "10000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:lumMod", "val", "80000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", "100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:alpha", "val", "15000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", "60000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff", "val", "40000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin", "ang", "16200000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin", "scaled", "0");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_TextEffects_Props3d_Ligatures_NumForm_NumSpacing, "TextEffects_Props3d_Ligatures_NumForm_NumSpacing.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Paragraph 1 - w14:props3d
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", "extrusionH", "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", "contourW", "25400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", "prstMaterial", "softEdge");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT", "w", "38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT", "h", "38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT", "prst", "relaxedInset");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB", "w", "69850");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB", "h", "38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB", "prst", "cross");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr", "val", "accent2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", "20000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", "80000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr/w14:srgbClr", "val", "92D050");

    // Paragraph 2 - w14:ligatures
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures", "val", "standard");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures", "val", "standardContextual");

    // Paragraph 3 - w14:numForm and w14:numSpacing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm", "val", "lining");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing", "val", "tabular");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm", "val", "oldStyle");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing", "val", "proportional");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_TextEffects_StylisticSets_CntxtAlts, "TextEffects_StylisticSets_CntxtAlts.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Paragraph 1 - w14:stylisticSets
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet", "id", "4");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet", "id", "2");

    // Paragraph 1 - w14:cntxtAlts
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:cntxtAlts", 1);

}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_McIgnorable, "TextEffects_StylisticSets_CntxtAlts.docx")
{
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPath(pXmlDocument, "/w:document", "Ignorable", "w14 wp14");

    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");

    assertXPath(pXmlStyles, "/w:styles", "Ignorable", "w14");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_CompatSettingsForW14, "TextEffects_StylisticSets_CntxtAlts.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/settings.xml");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting", 5);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "name", "compatibilityMode");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "uri", "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "val", "15"); // document was made with Word2013 -> 15

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]", "name", "overrideTableStyleFontSizeAndJustification");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]", "uri", "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]", "val", "1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]", "name", "enableOpenTypeFeatures");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]", "uri", "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]", "val", "1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]", "name", "doNotFlipMirrorIndents");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]", "uri", "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]", "val", "1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]", "name", "differentiateMultirowTableHeaders");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]", "uri", "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]", "val", "1");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_TextEffects_Groupshapes, "TextEffects_Groupshapes.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    OString sPathToWGP = "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp";

    assertXPath(pXmlDoc, sPathToWGP + "/wps:wsp", 2);

    OString sPathToShapeRunProperties = sPathToWGP + "/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr";

    // Glow
    OString sPathGlow = sPathToShapeRunProperties + "/w14:glow";
    assertXPath(pXmlDoc, sPathGlow, 1);
    assertXPath(pXmlDoc, sPathGlow, "rad", "127000");
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr", "val", "00B050");
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr/w14:alpha", "val", "60000");

    // Shadow
    OString sPathShadow = sPathToShapeRunProperties + "/w14:shadow";
    assertXPath(pXmlDoc, sPathShadow, 1);
    assertXPath(pXmlDoc, sPathShadow, "blurRad", "127000");
    assertXPath(pXmlDoc, sPathShadow, "dist", "787400");
    assertXPath(pXmlDoc, sPathShadow, "dir", "12720000");
    assertXPath(pXmlDoc, sPathShadow, "sx", "70000");
    assertXPath(pXmlDoc, sPathShadow, "sy", "70000");
    assertXPath(pXmlDoc, sPathShadow, "kx", "0");
    assertXPath(pXmlDoc, sPathShadow, "ky", "0");
    assertXPath(pXmlDoc, sPathShadow, "algn", "l");
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val", "92D050");
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val", "40000");

    // Reflection
    OString sPathReflection = sPathToShapeRunProperties + "/w14:reflection";
    assertXPath(pXmlDoc, sPathReflection, 1);
    assertXPath(pXmlDoc, sPathReflection, "blurRad", "139700");
    assertXPath(pXmlDoc, sPathReflection, "stA", "47000");
    assertXPath(pXmlDoc, sPathReflection, "stPos", "0");
    assertXPath(pXmlDoc, sPathReflection, "endA", "0");
    assertXPath(pXmlDoc, sPathReflection, "endPos", "85000");
    assertXPath(pXmlDoc, sPathReflection, "dist", "63500");
    assertXPath(pXmlDoc, sPathReflection, "dir", "5400000");
    assertXPath(pXmlDoc, sPathReflection, "fadeDir", "5400000");
    assertXPath(pXmlDoc, sPathReflection, "sx", "100000");
    assertXPath(pXmlDoc, sPathReflection, "sy", "-100000");
    assertXPath(pXmlDoc, sPathReflection, "kx", "0");
    assertXPath(pXmlDoc, sPathReflection, "ky", "0");
    assertXPath(pXmlDoc, sPathReflection, "algn", "bl");

    // TextOutline
    OString sPathTextOutline = sPathToShapeRunProperties + "/w14:textOutline";
    assertXPath(pXmlDoc, sPathTextOutline, 1);
    assertXPath(pXmlDoc, sPathTextOutline, "w", "25400");
    assertXPath(pXmlDoc, sPathTextOutline, "cap", "rnd");
    assertXPath(pXmlDoc, sPathTextOutline, "cmpd", "sng");
    assertXPath(pXmlDoc, sPathTextOutline, "algn", "ctr");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val", "accent2");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val", "40000");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val", "75000");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val", "solid");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:round", 1);

    // TextFill
    OString sPathTextFill = sPathToShapeRunProperties + "/w14:textFill";
    assertXPath(pXmlDoc, sPathTextFill, 1);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", "0");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", "accent4");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", "41000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", "accent6");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", "60000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", "40000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", "87000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", "accent5");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", "60000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff", "val", "40000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "ang", "5400000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "scaled", "0");

    // Props3D
    OString sPathProps3D = sPathToShapeRunProperties + "/w14:props3d";
    assertXPath(pXmlDoc, sPathProps3D, 1);
    assertXPath(pXmlDoc, sPathProps3D, "extrusionH", "63500");
    assertXPath(pXmlDoc, sPathProps3D, "contourW", "12700");
    assertXPath(pXmlDoc, sPathProps3D, "prstMaterial", "warmMatte");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w", "38100");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h", "38100");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst", "circle");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val", "accent2");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", "60000");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", "40000");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val", "accent4");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val", "75000");

    // Ligatures
    OString sPathLigatures = sPathToShapeRunProperties + "/w14:ligatures";
    assertXPath(pXmlDoc, sPathLigatures, 1);
    assertXPath(pXmlDoc, sPathLigatures, "val", "standard");

    // NumForm
    OString sPathNumForm = sPathToShapeRunProperties + "/w14:numForm";
    assertXPath(pXmlDoc, sPathNumForm, 1);
    assertXPath(pXmlDoc, sPathNumForm, "val", "oldStyle");

    // NumSpacing
    OString sPathNumSpacing = sPathToShapeRunProperties + "/w14:numSpacing";
    assertXPath(pXmlDoc, sPathNumSpacing, 1);
    assertXPath(pXmlDoc, sPathNumSpacing, "val", "tabular");

    // StylisticSets
    OString sPathStylisticSets = sPathToShapeRunProperties + "/w14:stylisticSets";
    assertXPath(pXmlDoc, sPathStylisticSets, 1);
    assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id", "1");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_TextEffects_InStyleXml, "TextEffects_InStyle.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");

    OString sPathToCharacterStyle = "/w:styles/w:style[3]";

    assertXPath(pXmlDoc, sPathToCharacterStyle, "type", "character");
    assertXPath(pXmlDoc, sPathToCharacterStyle, "styleId", "TextEffectsStyleChar");
    assertXPath(pXmlDoc, sPathToCharacterStyle, "customStyle", "1");

    {
        OString sPathToRun = sPathToCharacterStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad", "63500");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val", "accent2");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val", "60000");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val", "175000");

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad", "50800");
        assertXPath(pXmlDoc, sPathShadow, "dist", "38100");
        assertXPath(pXmlDoc, sPathShadow, "dir", "16200000");
        assertXPath(pXmlDoc, sPathShadow, "sx", "100000");
        assertXPath(pXmlDoc, sPathShadow, "sy", "100000");
        assertXPath(pXmlDoc, sPathShadow, "kx", "0");
        assertXPath(pXmlDoc, sPathShadow, "ky", "0");
        assertXPath(pXmlDoc, sPathShadow, "algn", "b");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val", "000000");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val", "60000");

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad", "6350");
        assertXPath(pXmlDoc, sPathReflection, "stA", "50000");
        assertXPath(pXmlDoc, sPathReflection, "stPos", "0");
        assertXPath(pXmlDoc, sPathReflection, "endA", "300");
        assertXPath(pXmlDoc, sPathReflection, "endPos", "50000");
        assertXPath(pXmlDoc, sPathReflection, "dist", "29997");
        assertXPath(pXmlDoc, sPathReflection, "dir", "5400000");
        assertXPath(pXmlDoc, sPathReflection, "fadeDir", "5400000");
        assertXPath(pXmlDoc, sPathReflection, "sx", "100000");
        assertXPath(pXmlDoc, sPathReflection, "sy", "-100000");
        assertXPath(pXmlDoc, sPathReflection, "kx", "0");
        assertXPath(pXmlDoc, sPathReflection, "ky", "0");
        assertXPath(pXmlDoc, sPathReflection, "algn", "bl");

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w", "25400");
        assertXPath(pXmlDoc, sPathTextOutline, "cap", "rnd");
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd", "sng");
        assertXPath(pXmlDoc, sPathTextOutline, "algn", "ctr");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val", "40000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val", "75000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val", "solid");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", "0");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val", "10000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val", "40000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val", "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", "46000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", "95000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", "5000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", "100000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path", "circle");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b", "-30000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r", "50000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t", "130000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l", "50000");

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH", "57150");
        assertXPath(pXmlDoc, sPathProps3D, "contourW", "12700");
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial", "metal");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w", "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h", "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst", "angle");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w", "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h", "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst", "divot");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", "20000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", "80000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val", "75000");

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val", "standard");

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val", "lining");

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val", "proportional");

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id", "1");
    }

    OString sPathToParagraphStyle = "/w:styles/w:style[9]";

    assertXPath(pXmlDoc, sPathToParagraphStyle, "type", "paragraph");
    assertXPath(pXmlDoc, sPathToParagraphStyle, "styleId", "TextEffectsStyle");
    assertXPath(pXmlDoc, sPathToParagraphStyle, "customStyle", "1");

    {
        OString sPathToRun = sPathToParagraphStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad", "63500");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val", "accent2");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val", "60000");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val", "175000");

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad", "50800");
        assertXPath(pXmlDoc, sPathShadow, "dist", "38100");
        assertXPath(pXmlDoc, sPathShadow, "dir", "16200000");
        assertXPath(pXmlDoc, sPathShadow, "sx", "100000");
        assertXPath(pXmlDoc, sPathShadow, "sy", "100000");
        assertXPath(pXmlDoc, sPathShadow, "kx", "0");
        assertXPath(pXmlDoc, sPathShadow, "ky", "0");
        assertXPath(pXmlDoc, sPathShadow, "algn", "b");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val", "000000");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val", "60000");

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad", "6350");
        assertXPath(pXmlDoc, sPathReflection, "stA", "50000");
        assertXPath(pXmlDoc, sPathReflection, "stPos", "0");
        assertXPath(pXmlDoc, sPathReflection, "endA", "300");
        assertXPath(pXmlDoc, sPathReflection, "endPos", "50000");
        assertXPath(pXmlDoc, sPathReflection, "dist", "29997");
        assertXPath(pXmlDoc, sPathReflection, "dir", "5400000");
        assertXPath(pXmlDoc, sPathReflection, "fadeDir", "5400000");
        assertXPath(pXmlDoc, sPathReflection, "sx", "100000");
        assertXPath(pXmlDoc, sPathReflection, "sy", "-100000");
        assertXPath(pXmlDoc, sPathReflection, "kx", "0");
        assertXPath(pXmlDoc, sPathReflection, "ky", "0");
        assertXPath(pXmlDoc, sPathReflection, "algn", "bl");

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w", "25400");
        assertXPath(pXmlDoc, sPathTextOutline, "cap", "rnd");
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd", "sng");
        assertXPath(pXmlDoc, sPathTextOutline, "algn", "ctr");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val", "40000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val", "75000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val", "solid");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", "0");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val", "10000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val", "40000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val", "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", "46000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", "95000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", "5000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", "100000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path", "circle");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b", "-30000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r", "50000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t", "130000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l", "50000");

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH", "57150");
        assertXPath(pXmlDoc, sPathProps3D, "contourW", "12700");
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial", "metal");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w", "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h", "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst", "angle");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w", "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h", "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst", "divot");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", "20000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", "80000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val", "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val", "75000");

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val", "standard");

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val", "lining");

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val", "proportional");

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id", "1");
    }
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_no_tag_if_no_fill, "tdf112103_tablebgnofill.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:shd", 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
