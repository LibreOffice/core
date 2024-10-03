/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_GlowShadowReflection)
{
    loadAndReload("TextEffects_Glow_Shadow_Reflection.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

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

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_TextOutline)
{
    loadAndReload("TextEffects_TextOutline.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Paragraph 1
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "w", u"50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "cap", u"rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "cmpd", u"dbl");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline", "algn", u"ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", u"70000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr", "val", u"92D050");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:alpha", "val", u"30000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumMod", "val", u"75000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumOff", "val", u"25000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", u"30000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", u"accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:alpha", "val", u"55000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", u"40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", u"60000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]/w14:srgbClr", "val", u"0070C0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]", "pos", u"100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]/w14:schemeClr", "val", u"accent4");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin", "ang", u"3600000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin", "scaled", u"0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash", "val", u"dash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:miter", "lim", u"0");

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "w", u"9525");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "cap", u"rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "cmpd", u"sng");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline", "algn", u"ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:solidFill/w14:srgbClr", "val", u"FF0000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash", "val", u"solid");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:bevel", 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "w", u"9525");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "cap", u"rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "cmpd", u"sng");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline", "algn", u"ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:noFill", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash", "val", u"solid");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:bevel", 1);
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_TextFill)
{
    loadAndReload("TextEffects_TextFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Paragraph 1 has no textFill

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textFill/w14:noFill", 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr", "val", u"accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:alpha", "val", u"5000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod", "val", u"40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumOff", "val", u"60000");

    // Paragraph 4
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", u"accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val", u"5000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val", u"67000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", u"50000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr", "val", u"00B0F0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:alpha", "val", u"10000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:lumMod", "val", u"80000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", u"100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", u"accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:alpha", "val", u"15000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", u"60000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff", "val", u"40000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin", "ang", u"16200000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin", "scaled", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_Props3d_Ligatures_NumForm_NumSpacing)
{
    loadAndReload("TextEffects_Props3d_Ligatures_NumForm_NumSpacing.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Paragraph 1 - w14:props3d
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", "extrusionH", u"63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", "contourW", u"25400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d", "prstMaterial", u"softEdge");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT", "w", u"38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT", "h", u"38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT", "prst", u"relaxedInset");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB", "w", u"69850");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB", "h", u"38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB", "prst", u"cross");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr", "val", u"accent2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", u"20000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", u"80000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr/w14:srgbClr", "val", u"92D050");

    // Paragraph 2 - w14:ligatures
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures", "val", u"standard");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures", "val", u"standardContextual");

    // Paragraph 3 - w14:numForm and w14:numSpacing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm", "val", u"lining");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing", "val", u"tabular");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm", "val", u"oldStyle");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing", "val", u"proportional");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_StylisticSets_CntxtAlts)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Paragraph 1 - w14:stylisticSets
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet", "id", u"4");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet", "id", u"2");

    // Paragraph 1 - w14:cntxtAlts
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:cntxtAlts", 1);

}

CPPUNIT_TEST_FIXTURE(Test, Test_McIgnorable)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document", "Ignorable", u"w14 wp14 w15");

    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);

    assertXPath(pXmlStyles, "/w:styles", "Ignorable", u"w14");
}

CPPUNIT_TEST_FIXTURE(Test, Test_CompatSettingsForW14)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting", 5);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "name", u"compatibilityMode");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "uri", u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "val", u"15"); // document was made with Word2013 -> 15

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]", "name", u"overrideTableStyleFontSizeAndJustification");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]", "uri", u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]", "val", u"1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]", "name", u"enableOpenTypeFeatures");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]", "uri", u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]", "val", u"1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]", "name", u"doNotFlipMirrorIndents");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]", "uri", u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]", "val", u"1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]", "name", u"differentiateMultirowTableHeaders");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]", "uri", u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]", "val", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_Groupshapes)
{
    loadAndReload("TextEffects_Groupshapes.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    OString sPathToWGP = "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp"_ostr;

    assertXPath(pXmlDoc, sPathToWGP + "/wps:wsp", 2);

    OString sPathToShapeRunProperties = sPathToWGP + "/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr";

    // Glow
    OString sPathGlow = sPathToShapeRunProperties + "/w14:glow";
    assertXPath(pXmlDoc, sPathGlow, 1);
    assertXPath(pXmlDoc, sPathGlow, "rad", u"127000");
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr", "val", u"00B050");
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr/w14:alpha", "val", u"60000");

    // Shadow
    OString sPathShadow = sPathToShapeRunProperties + "/w14:shadow";
    assertXPath(pXmlDoc, sPathShadow, 1);
    assertXPath(pXmlDoc, sPathShadow, "blurRad", u"127000");
    assertXPath(pXmlDoc, sPathShadow, "dist", u"787400");
    assertXPath(pXmlDoc, sPathShadow, "dir", u"12720000");
    assertXPath(pXmlDoc, sPathShadow, "sx", u"70000");
    assertXPath(pXmlDoc, sPathShadow, "sy", u"70000");
    assertXPath(pXmlDoc, sPathShadow, "kx", u"0");
    assertXPath(pXmlDoc, sPathShadow, "ky", u"0");
    assertXPath(pXmlDoc, sPathShadow, "algn", u"l");
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val", u"92D050");
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val", u"40000");

    // Reflection
    OString sPathReflection = sPathToShapeRunProperties + "/w14:reflection";
    assertXPath(pXmlDoc, sPathReflection, 1);
    assertXPath(pXmlDoc, sPathReflection, "blurRad", u"139700");
    assertXPath(pXmlDoc, sPathReflection, "stA", u"47000");
    assertXPath(pXmlDoc, sPathReflection, "stPos", u"0");
    assertXPath(pXmlDoc, sPathReflection, "endA", u"0");
    assertXPath(pXmlDoc, sPathReflection, "endPos", u"85000");
    assertXPath(pXmlDoc, sPathReflection, "dist", u"63500");
    assertXPath(pXmlDoc, sPathReflection, "dir", u"5400000");
    assertXPath(pXmlDoc, sPathReflection, "fadeDir", u"5400000");
    assertXPath(pXmlDoc, sPathReflection, "sx", u"100000");
    assertXPath(pXmlDoc, sPathReflection, "sy", u"-100000");
    assertXPath(pXmlDoc, sPathReflection, "kx", u"0");
    assertXPath(pXmlDoc, sPathReflection, "ky", u"0");
    assertXPath(pXmlDoc, sPathReflection, "algn", u"bl");

    // TextOutline
    OString sPathTextOutline = sPathToShapeRunProperties + "/w14:textOutline";
    assertXPath(pXmlDoc, sPathTextOutline, 1);
    assertXPath(pXmlDoc, sPathTextOutline, "w", u"25400");
    assertXPath(pXmlDoc, sPathTextOutline, "cap", u"rnd");
    assertXPath(pXmlDoc, sPathTextOutline, "cmpd", u"sng");
    assertXPath(pXmlDoc, sPathTextOutline, "algn", u"ctr");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val", u"accent2");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val", u"40000");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val", u"75000");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val", u"solid");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:round", 1);

    // TextFill
    OString sPathTextFill = sPathToShapeRunProperties + "/w14:textFill";
    assertXPath(pXmlDoc, sPathTextFill, 1);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", u"accent4");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", u"41000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", u"accent6");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", u"60000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", u"40000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", u"87000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", u"accent5");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", u"60000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff", "val", u"40000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "ang", u"5400000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "scaled", u"0");

    // Props3D
    OString sPathProps3D = sPathToShapeRunProperties + "/w14:props3d";
    assertXPath(pXmlDoc, sPathProps3D, 1);
    assertXPath(pXmlDoc, sPathProps3D, "extrusionH", u"63500");
    assertXPath(pXmlDoc, sPathProps3D, "contourW", u"12700");
    assertXPath(pXmlDoc, sPathProps3D, "prstMaterial", u"warmMatte");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w", u"38100");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h", u"38100");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst", u"circle");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val", u"accent2");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", u"60000");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", u"40000");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val", u"accent4");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val", u"75000");

    // Ligatures
    OString sPathLigatures = sPathToShapeRunProperties + "/w14:ligatures";
    assertXPath(pXmlDoc, sPathLigatures, 1);
    assertXPath(pXmlDoc, sPathLigatures, "val", u"standard");

    // NumForm
    OString sPathNumForm = sPathToShapeRunProperties + "/w14:numForm";
    assertXPath(pXmlDoc, sPathNumForm, 1);
    assertXPath(pXmlDoc, sPathNumForm, "val", u"oldStyle");

    // NumSpacing
    OString sPathNumSpacing = sPathToShapeRunProperties + "/w14:numSpacing";
    assertXPath(pXmlDoc, sPathNumSpacing, 1);
    assertXPath(pXmlDoc, sPathNumSpacing, "val", u"tabular");

    // StylisticSets
    OString sPathStylisticSets = sPathToShapeRunProperties + "/w14:stylisticSets";
    assertXPath(pXmlDoc, sPathStylisticSets, 1);
    assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id", u"1");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_InStyleXml)
{
    loadAndReload("TextEffects_InStyle.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);

    OString sPathToCharacterStyle = "/w:styles/w:style[3]"_ostr;

    assertXPath(pXmlDoc, sPathToCharacterStyle, "type", u"character");
    assertXPath(pXmlDoc, sPathToCharacterStyle, "styleId", u"TextEffectsStyleChar");
    assertXPath(pXmlDoc, sPathToCharacterStyle, "customStyle", u"1");

    {
        OString sPathToRun = sPathToCharacterStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad", u"63500");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val", u"accent2");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val", u"60000");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val", u"175000");

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad", u"50800");
        assertXPath(pXmlDoc, sPathShadow, "dist", u"38100");
        assertXPath(pXmlDoc, sPathShadow, "dir", u"16200000");
        assertXPath(pXmlDoc, sPathShadow, "sx", u"100000");
        assertXPath(pXmlDoc, sPathShadow, "sy", u"100000");
        assertXPath(pXmlDoc, sPathShadow, "kx", u"0");
        assertXPath(pXmlDoc, sPathShadow, "ky", u"0");
        assertXPath(pXmlDoc, sPathShadow, "algn", u"b");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val", u"000000");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val", u"60000");

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad", u"6350");
        assertXPath(pXmlDoc, sPathReflection, "stA", u"50000");
        assertXPath(pXmlDoc, sPathReflection, "stPos", u"0");
        assertXPath(pXmlDoc, sPathReflection, "endA", u"300");
        assertXPath(pXmlDoc, sPathReflection, "endPos", u"50000");
        assertXPath(pXmlDoc, sPathReflection, "dist", u"29997");
        assertXPath(pXmlDoc, sPathReflection, "dir", u"5400000");
        assertXPath(pXmlDoc, sPathReflection, "fadeDir", u"5400000");
        assertXPath(pXmlDoc, sPathReflection, "sx", u"100000");
        assertXPath(pXmlDoc, sPathReflection, "sy", u"-100000");
        assertXPath(pXmlDoc, sPathReflection, "kx", u"0");
        assertXPath(pXmlDoc, sPathReflection, "ky", u"0");
        assertXPath(pXmlDoc, sPathReflection, "algn", u"bl");

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w", u"25400");
        assertXPath(pXmlDoc, sPathTextOutline, "cap", u"rnd");
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd", u"sng");
        assertXPath(pXmlDoc, sPathTextOutline, "algn", u"ctr");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val", u"40000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val", u"75000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val", u"solid");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", u"0");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val", u"10000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val", u"40000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val", u"60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", u"46000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", u"95000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", u"5000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", u"100000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", u"60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path", u"circle");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b", u"-30000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r", u"50000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t", u"130000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l", u"50000");

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH", u"57150");
        assertXPath(pXmlDoc, sPathProps3D, "contourW", u"12700");
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial", u"metal");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w", u"38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h", u"38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst", u"angle");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w", u"69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h", u"69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst", u"divot");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", u"20000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", u"80000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val", u"75000");

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val", u"standard");

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val", u"lining");

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val", u"proportional");

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id", u"1");
    }

    OString sPathToParagraphStyle = "/w:styles/w:style[9]"_ostr;

    assertXPath(pXmlDoc, sPathToParagraphStyle, "type", u"paragraph");
    assertXPath(pXmlDoc, sPathToParagraphStyle, "styleId", u"TextEffectsStyle");
    assertXPath(pXmlDoc, sPathToParagraphStyle, "customStyle", u"1");

    {
        OString sPathToRun = sPathToParagraphStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad", u"63500");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val", u"accent2");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val", u"60000");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val", u"175000");

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad", u"50800");
        assertXPath(pXmlDoc, sPathShadow, "dist", u"38100");
        assertXPath(pXmlDoc, sPathShadow, "dir", u"16200000");
        assertXPath(pXmlDoc, sPathShadow, "sx", u"100000");
        assertXPath(pXmlDoc, sPathShadow, "sy", u"100000");
        assertXPath(pXmlDoc, sPathShadow, "kx", u"0");
        assertXPath(pXmlDoc, sPathShadow, "ky", u"0");
        assertXPath(pXmlDoc, sPathShadow, "algn", u"b");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val", u"000000");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val", u"60000");

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad", u"6350");
        assertXPath(pXmlDoc, sPathReflection, "stA", u"50000");
        assertXPath(pXmlDoc, sPathReflection, "stPos", u"0");
        assertXPath(pXmlDoc, sPathReflection, "endA", u"300");
        assertXPath(pXmlDoc, sPathReflection, "endPos", u"50000");
        assertXPath(pXmlDoc, sPathReflection, "dist", u"29997");
        assertXPath(pXmlDoc, sPathReflection, "dir", u"5400000");
        assertXPath(pXmlDoc, sPathReflection, "fadeDir", u"5400000");
        assertXPath(pXmlDoc, sPathReflection, "sx", u"100000");
        assertXPath(pXmlDoc, sPathReflection, "sy", u"-100000");
        assertXPath(pXmlDoc, sPathReflection, "kx", u"0");
        assertXPath(pXmlDoc, sPathReflection, "ky", u"0");
        assertXPath(pXmlDoc, sPathReflection, "algn", u"bl");

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w", u"25400");
        assertXPath(pXmlDoc, sPathTextOutline, "cap", u"rnd");
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd", u"sng");
        assertXPath(pXmlDoc, sPathTextOutline, "algn", u"ctr");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val", u"40000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val", u"75000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val", u"solid");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos", u"0");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val", u"10000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val", u"40000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val", u"60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos", u"46000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val", u"95000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val", u"5000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos", u"100000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val", u"60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path", u"circle");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b", u"-30000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r", u"50000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t", u"130000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l", u"50000");

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH", u"57150");
        assertXPath(pXmlDoc, sPathProps3D, "contourW", u"12700");
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial", u"metal");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w", u"38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h", u"38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst", u"angle");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w", u"69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h", u"69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst", u"divot");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val", u"20000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val", u"80000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val", u"accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val", u"75000");

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val", u"standard");

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val", u"lining");

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val", u"proportional");

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id", u"1");
    }
}

CPPUNIT_TEST_FIXTURE(Test, Test_no_tag_if_no_fill)
{
    loadAndReload("tdf112103_tablebgnofill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:shd", 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
