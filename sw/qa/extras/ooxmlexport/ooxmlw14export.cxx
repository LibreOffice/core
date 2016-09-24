/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <string>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx"));
    }
};

DECLARE_OOXMLEXPORT_TEST(Test_TextEffects_GlowShadowReflection, "TextEffects_Glow_Shadow_Reflection.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

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

DECLARE_OOXMLEXPORT_TEST(Test_TextEffects_TextOutline, "TextEffects_TextOutline.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

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

DECLARE_OOXMLEXPORT_TEST(Test_TextEffects_TextFill, "TextEffects_TextFill.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
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

DECLARE_OOXMLEXPORT_TEST(Test_TextEffects_Props3d_Ligatures_NumForm_NumSpacing, "TextEffects_Props3d_Ligatures_NumForm_NumSpacing.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

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

    // Paragraph 3 - w14:numFurm and w14:numSpacing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm", "val", "lining");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing", "val", "tabular");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm", "val", "oldStyle");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing", "val", "proportional");
}

DECLARE_OOXMLEXPORT_TEST(Test_TextEffects_StylisticSets_CntxtAlts, "TextEffects_StylisticSets_CntxtAlts.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // Paragraph 1 - w14:stylisticSets
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet", "id", "4");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet", "id", "2");

    // Paragraph 1 - w14:cntxtAlts
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:cntxtAlts", 1);

}

DECLARE_OOXMLEXPORT_TEST(Test_McIgnorable, "TextEffects_StylisticSets_CntxtAlts.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    assertXPath(pXmlDocument, "/w:document", "Ignorable", "w14 wp14");

    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    assertXPath(pXmlStyles, "/w:styles", "Ignorable", "w14");
}

DECLARE_OOXMLEXPORT_TEST(Test_CompatSettingsForW14, "TextEffects_StylisticSets_CntxtAlts.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/settings.xml");
    if (!pXmlDoc)
        return;

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

DECLARE_OOXMLEXPORT_TEST(Test_TextEffects_Groupshapes, "TextEffects_Groupshapes.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    std::string sPathToWGP = "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp";

    assertXPath(pXmlDoc, (sPathToWGP + "/wps:wsp").c_str(), 2);

    std::string sPathToShapeRunProperties = sPathToWGP + "/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr";

    // Glow
    std::string sPathGlow = sPathToShapeRunProperties + "/w14:glow";
    assertXPath(pXmlDoc, (sPathGlow).c_str(), 1);
    assertXPath(pXmlDoc, (sPathGlow).c_str(), "rad", "127000");
    assertXPath(pXmlDoc, (sPathGlow+"/w14:srgbClr").c_str(), "val", "00B050");
    assertXPath(pXmlDoc, (sPathGlow+"/w14:srgbClr/w14:alpha").c_str(), "val", "60000");

    // Shadow
    std::string sPathShadow = sPathToShapeRunProperties + "/w14:shadow";
    assertXPath(pXmlDoc, (sPathShadow).c_str(), 1);
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "blurRad", "127000");
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "dist", "787400");
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "dir", "12720000");
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "sx", "70000");
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "sy", "70000");
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "kx", "0");
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "ky", "0");
    assertXPath(pXmlDoc, (sPathShadow).c_str(), "algn", "l");
    assertXPath(pXmlDoc, (sPathShadow+"/w14:srgbClr").c_str(), "val", "92D050");
    assertXPath(pXmlDoc, (sPathShadow+"/w14:srgbClr/w14:alpha").c_str(), "val", "40000");

    // Reflection
    std::string sPathReflection = sPathToShapeRunProperties + "/w14:reflection";
    assertXPath(pXmlDoc, (sPathReflection).c_str(), 1);
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "blurRad", "139700");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "stA", "47000");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "stPos", "0");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "endA", "0");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "endPos", "85000");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "dist", "63500");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "dir", "5400000");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "fadeDir", "5400000");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "sx", "100000");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "sy", "-100000");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "kx", "0");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "ky", "0");
    assertXPath(pXmlDoc, (sPathReflection).c_str(), "algn", "bl");

    // TextOutline
    std::string sPathTextOutline = sPathToShapeRunProperties + "/w14:textOutline";
    assertXPath(pXmlDoc, (sPathTextOutline).c_str(), 1);
    assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "w", "25400");
    assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "cap", "rnd");
    assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "cmpd", "sng");
    assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "algn", "ctr");
    assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr").c_str(), "val", "accent2");
    assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha").c_str(), "val", "40000");
    assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod").c_str(), "val", "75000");
    assertXPath(pXmlDoc, (sPathTextOutline+"/w14:prstDash").c_str(), "val", "solid");
    assertXPath(pXmlDoc, (sPathTextOutline+"/w14:round").c_str(), 1);

    // TextFill
    std::string sPathTextFill = sPathToShapeRunProperties + "/w14:textFill";
    assertXPath(pXmlDoc, (sPathTextFill).c_str(), 1);
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]").c_str(), "pos", "0");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr").c_str(), "val", "accent4");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]").c_str(), "pos", "41000");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr").c_str(), "val", "accent6");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod").c_str(), "val", "60000");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff").c_str(), "val", "40000");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]").c_str(), "pos", "87000");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr").c_str(), "val", "accent5");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod").c_str(), "val", "60000");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff").c_str(), "val", "40000");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:lin").c_str(), "ang", "5400000");
    assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:lin").c_str(), "scaled", "0");

    // Props3D
    std::string sPathProps3D = sPathToShapeRunProperties + "/w14:props3d";
    assertXPath(pXmlDoc, (sPathProps3D).c_str(), 1);
    assertXPath(pXmlDoc, (sPathProps3D).c_str(), "extrusionH", "63500");
    assertXPath(pXmlDoc, (sPathProps3D).c_str(), "contourW", "12700");
    assertXPath(pXmlDoc, (sPathProps3D).c_str(), "prstMaterial", "warmMatte");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "w", "38100");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "h", "38100");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "prst", "circle");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr").c_str(), "val", "accent2");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod").c_str(), "val", "60000");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff").c_str(), "val", "40000");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:contourClr/w14:schemeClr").c_str(), "val", "accent4");
    assertXPath(pXmlDoc, (sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod").c_str(), "val", "75000");

    // Ligatures
    std::string sPathLigatures = sPathToShapeRunProperties + "/w14:ligatures";
    assertXPath(pXmlDoc, (sPathLigatures).c_str(), 1);
    assertXPath(pXmlDoc, (sPathLigatures).c_str(), "val", "standard");

    // NumForm
    std::string sPathNumForm = sPathToShapeRunProperties + "/w14:numForm";
    assertXPath(pXmlDoc, (sPathNumForm).c_str(), 1);
    assertXPath(pXmlDoc, (sPathNumForm).c_str(), "val", "oldStyle");

    // NumSpacing
    std::string sPathNumSpacing = sPathToShapeRunProperties + "/w14:numSpacing";
    assertXPath(pXmlDoc, (sPathNumSpacing).c_str(), 1);
    assertXPath(pXmlDoc, (sPathNumSpacing).c_str(), "val", "tabular");

    // StylisticSets
    std::string sPathStylisticSets = sPathToShapeRunProperties + "/w14:stylisticSets";
    assertXPath(pXmlDoc, (sPathStylisticSets).c_str(), 1);
    assertXPath(pXmlDoc, (sPathStylisticSets+"/w14:styleSet").c_str(), "id", "1");
}

DECLARE_OOXMLEXPORT_TEST(Test_TextEffects_InStyleXml, "TextEffects_InStyle.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");
    if (!pXmlDoc)
        return;

    std::string sPathToCharacterStyle = "/w:styles/w:style[3]";

    assertXPath(pXmlDoc, (sPathToCharacterStyle).c_str(), "type", "character");
    assertXPath(pXmlDoc, (sPathToCharacterStyle).c_str(), "styleId", "TextEffectsStyleChar");
    assertXPath(pXmlDoc, (sPathToCharacterStyle).c_str(), "customStyle", "1");

    {
        std::string sPathToRun = sPathToCharacterStyle + "/w:rPr";

        // Glow
        std::string sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, (sPathGlow).c_str(), "rad", "63500");
        assertXPath(pXmlDoc, (sPathGlow+"/w14:schemeClr").c_str(), "val", "accent2");
        assertXPath(pXmlDoc, (sPathGlow+"/w14:schemeClr/w14:alpha").c_str(), "val", "60000");
        assertXPath(pXmlDoc, (sPathGlow+"/w14:schemeClr/w14:satMod").c_str(), "val", "175000");

        // Shadow
        std::string sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "blurRad", "50800");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "dist", "38100");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "dir", "16200000");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "sx", "100000");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "sy", "100000");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "kx", "0");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "ky", "0");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "algn", "b");
        assertXPath(pXmlDoc, (sPathShadow+"/w14:srgbClr").c_str(), "val", "000000");
        assertXPath(pXmlDoc, (sPathShadow+"/w14:srgbClr/w14:alpha").c_str(), "val", "60000");

        // Reflection
        std::string sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "blurRad", "6350");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "stA", "50000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "stPos", "0");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "endA", "300");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "endPos", "50000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "dist", "29997");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "dir", "5400000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "fadeDir", "5400000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "sx", "100000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "sy", "-100000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "kx", "0");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "ky", "0");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "algn", "bl");

        // TextOutline
        std::string sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "w", "25400");
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "cap", "rnd");
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "cmpd", "sng");
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "algn", "ctr");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha").c_str(), "val", "40000");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod").c_str(), "val", "75000");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:prstDash").c_str(), "val", "solid");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:bevel").c_str(), 1);

        // TextFill
        std::string sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]").c_str(), "pos", "0");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha").c_str(), "val", "10000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod").c_str(), "val", "40000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff").c_str(), "val", "60000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]").c_str(), "pos", "46000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod").c_str(), "val", "95000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff").c_str(), "val", "5000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]").c_str(), "pos", "100000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod").c_str(), "val", "60000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path").c_str(), "path", "circle");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "b", "-30000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "r", "50000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "t", "130000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "l", "50000");

        // Props3D
        std::string sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, (sPathProps3D).c_str(), "extrusionH", "57150");
        assertXPath(pXmlDoc, (sPathProps3D).c_str(), "contourW", "12700");
        assertXPath(pXmlDoc, (sPathProps3D).c_str(), "prstMaterial", "metal");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "w", "38100");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "h", "38100");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "prst", "angle");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelB").c_str(), "w", "69850");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelB").c_str(), "h", "69850");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelB").c_str(), "prst", "divot");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod").c_str(), "val", "20000");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff").c_str(), "val", "80000");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:contourClr/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod").c_str(), "val", "75000");

        // Ligatures
        std::string sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, (sPathLigatures).c_str(), "val", "standard");

        // NumForm
        std::string sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, (sPathNumForm).c_str(), "val", "lining");

        // NumSpacing
        std::string sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, (sPathNumSpacing).c_str(), "val", "proportional");

        // StylisticSets
        std::string sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, (sPathStylisticSets+"/w14:styleSet").c_str(), "id", "1");
    }

    std::string sPathToParagraphStyle = "/w:styles/w:style[9]";

    assertXPath(pXmlDoc, (sPathToParagraphStyle).c_str(), "type", "paragraph");
    assertXPath(pXmlDoc, (sPathToParagraphStyle).c_str(), "styleId", "TextEffectsStyle");
    assertXPath(pXmlDoc, (sPathToParagraphStyle).c_str(), "customStyle", "1");

    {
        std::string sPathToRun = sPathToParagraphStyle + "/w:rPr";

        // Glow
        std::string sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, (sPathGlow).c_str(), "rad", "63500");
        assertXPath(pXmlDoc, (sPathGlow+"/w14:schemeClr").c_str(), "val", "accent2");
        assertXPath(pXmlDoc, (sPathGlow+"/w14:schemeClr/w14:alpha").c_str(), "val", "60000");
        assertXPath(pXmlDoc, (sPathGlow+"/w14:schemeClr/w14:satMod").c_str(), "val", "175000");

        // Shadow
        std::string sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "blurRad", "50800");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "dist", "38100");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "dir", "16200000");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "sx", "100000");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "sy", "100000");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "kx", "0");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "ky", "0");
        assertXPath(pXmlDoc, (sPathShadow).c_str(), "algn", "b");
        assertXPath(pXmlDoc, (sPathShadow+"/w14:srgbClr").c_str(), "val", "000000");
        assertXPath(pXmlDoc, (sPathShadow+"/w14:srgbClr/w14:alpha").c_str(), "val", "60000");

        // Reflection
        std::string sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "blurRad", "6350");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "stA", "50000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "stPos", "0");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "endA", "300");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "endPos", "50000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "dist", "29997");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "dir", "5400000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "fadeDir", "5400000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "sx", "100000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "sy", "-100000");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "kx", "0");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "ky", "0");
        assertXPath(pXmlDoc, (sPathReflection).c_str(), "algn", "bl");

        // TextOutline
        std::string sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "w", "25400");
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "cap", "rnd");
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "cmpd", "sng");
        assertXPath(pXmlDoc, (sPathTextOutline).c_str(), "algn", "ctr");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha").c_str(), "val", "40000");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod").c_str(), "val", "75000");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:prstDash").c_str(), "val", "solid");
        assertXPath(pXmlDoc, (sPathTextOutline+"/w14:bevel").c_str(), 1);

        // TextFill
        std::string sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]").c_str(), "pos", "0");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha").c_str(), "val", "10000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod").c_str(), "val", "40000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff").c_str(), "val", "60000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]").c_str(), "pos", "46000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod").c_str(), "val", "95000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff").c_str(), "val", "5000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]").c_str(), "pos", "100000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod").c_str(), "val", "60000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path").c_str(), "path", "circle");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "b", "-30000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "r", "50000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "t", "130000");
        assertXPath(pXmlDoc, (sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect").c_str(), "l", "50000");

        // Props3D
        std::string sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, (sPathProps3D).c_str(), "extrusionH", "57150");
        assertXPath(pXmlDoc, (sPathProps3D).c_str(), "contourW", "12700");
        assertXPath(pXmlDoc, (sPathProps3D).c_str(), "prstMaterial", "metal");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "w", "38100");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "h", "38100");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelT").c_str(), "prst", "angle");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelB").c_str(), "w", "69850");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelB").c_str(), "h", "69850");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:bevelB").c_str(), "prst", "divot");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod").c_str(), "val", "20000");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff").c_str(), "val", "80000");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:contourClr/w14:schemeClr").c_str(), "val", "accent1");
        assertXPath(pXmlDoc, (sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod").c_str(), "val", "75000");

        // Ligatures
        std::string sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, (sPathLigatures).c_str(), "val", "standard");

        // NumForm
        std::string sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, (sPathNumForm).c_str(), "val", "lining");

        // NumSpacing
        std::string sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, (sPathNumSpacing).c_str(), "val", "proportional");

        // StylisticSets
        std::string sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, (sPathStylisticSets+"/w14:styleSet").c_str(), "id", "1");
    }
}


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
