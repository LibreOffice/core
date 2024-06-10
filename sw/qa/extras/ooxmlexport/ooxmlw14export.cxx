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

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:rPr/w14:glow"_ostr, "rad"_ostr).match("63500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:rPr/w14:glow/w14:srgbClr"_ostr, "val"_ostr).match("00B0F0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:rPr/w14:glow/w14:srgbClr/w14:alpha"_ostr, "val"_ostr).match("60000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow"_ostr, "rad"_ostr).match("228600"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow/w14:schemeClr"_ostr, "val"_ostr).match("accent6"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow/w14:schemeClr/w14:alpha"_ostr, "val"_ostr).match("60000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w14:glow/w14:schemeClr/w14:satMod"_ostr, "val"_ostr).match("175000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "blurRad"_ostr).match("63500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "dist"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "dir"_ostr).match("1800000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "sx"_ostr).match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "sy"_ostr).match("-30000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "kx"_ostr).match("-800400"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "ky"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow"_ostr, "algn"_ostr).match("bl"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow/w14:schemeClr"_ostr, "val"_ostr).match("accent3"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow/w14:schemeClr/w14:alpha"_ostr, "val"_ostr).match("38000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:rPr/w14:shadow/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr).match("75000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "blurRad"_ostr).match("190500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "dist"_ostr).match("190500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "dir"_ostr).match("3000000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "sx"_ostr).match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "sy"_ostr).match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "kx"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "ky"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow"_ostr, "algn"_ostr).match("ctr"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow/w14:srgbClr"_ostr, "val"_ostr).match("FF0000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:rPr/w14:shadow/w14:srgbClr/w14:alpha"_ostr, "val"_ostr).match("10000"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "blurRad"_ostr).match("6350"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "stA"_ostr).match("60000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "stPos"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "endA"_ostr).match("900"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "endPos"_ostr).match("60000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "dist"_ostr).match("60007"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "dir"_ostr).match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "fadeDir"_ostr).match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "sx"_ostr).match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "sy"_ostr).match("-100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "kx"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "ky"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:rPr/w14:reflection"_ostr, "algn"_ostr).match("bl"));

    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "blurRad"_ostr).match("6350"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "stA"_ostr).match("55000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "stPos"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "endA"_ostr).match("300"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "endPos"_ostr).match("45500"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "dist"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "dir"_ostr).match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "fadeDir"_ostr).match("5400000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "sx"_ostr).match("100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "sy"_ostr).match("-100000"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "kx"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "ky"_ostr).match("0"));
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[7]/w:rPr/w14:reflection"_ostr, "algn"_ostr).match("bl"));
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_TextOutline)
{
    loadAndReload("TextEffects_TextOutline.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Paragraph 1
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "w"_ostr, u"50800"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cap"_ostr, u"rnd"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cmpd"_ostr, u"dbl"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "algn"_ostr, u"ctr"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]"_ostr, "pos"_ostr, u"70000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr"_ostr, "val"_ostr, u"92D050"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:alpha"_ostr, "val"_ostr, u"30000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumMod"_ostr, "val"_ostr, u"75000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumOff"_ostr, "val"_ostr, u"25000"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]"_ostr, "pos"_ostr, u"30000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr"_ostr, "val"_ostr, u"accent1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, u"55000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, u"60000"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]"_ostr, "pos"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]/w14:srgbClr"_ostr, "val"_ostr, u"0070C0"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]"_ostr, "pos"_ostr, u"100000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]/w14:schemeClr"_ostr, "val"_ostr, u"accent4"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin"_ostr, "ang"_ostr, u"3600000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin"_ostr, "scaled"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash"_ostr, "val"_ostr, u"dash"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:miter"_ostr, "lim"_ostr, u"0"_ustr);

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "w"_ostr, u"9525"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cap"_ostr, u"rnd"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cmpd"_ostr, u"sng"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "algn"_ostr, u"ctr"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:solidFill/w14:srgbClr"_ostr, "val"_ostr, u"FF0000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash"_ostr, "val"_ostr, u"solid"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:bevel"_ostr, 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "w"_ostr, u"9525"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cap"_ostr, u"rnd"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cmpd"_ostr, u"sng"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "algn"_ostr, u"ctr"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:noFill"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash"_ostr, "val"_ostr, u"solid"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:bevel"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_TextFill)
{
    loadAndReload("TextEffects_TextFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Paragraph 1 has no textFill

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textFill/w14:noFill"_ostr, 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr"_ostr, "val"_ostr, u"accent1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, u"5000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, u"60000"_ustr);

    // Paragraph 4
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]"_ostr, "pos"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr"_ostr, "val"_ostr, u"accent1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, u"5000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, u"67000"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]"_ostr, "pos"_ostr, u"50000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr"_ostr, "val"_ostr, u"00B0F0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:alpha"_ostr, "val"_ostr, u"10000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:lumMod"_ostr, "val"_ostr, u"80000"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]"_ostr, "pos"_ostr, u"100000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr"_ostr, "val"_ostr, u"accent1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, u"15000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, u"60000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, u"40000"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin"_ostr, "ang"_ostr, u"16200000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin"_ostr, "scaled"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_Props3d_Ligatures_NumForm_NumSpacing)
{
    loadAndReload("TextEffects_Props3d_Ligatures_NumForm_NumSpacing.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Paragraph 1 - w14:props3d
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, "extrusionH"_ostr, u"63500"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, "contourW"_ostr, u"25400"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, "prstMaterial"_ostr, u"softEdge"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT"_ostr, "w"_ostr, u"38100"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT"_ostr, "h"_ostr, u"38100"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT"_ostr, "prst"_ostr, u"relaxedInset"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB"_ostr, "w"_ostr, u"69850"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB"_ostr, "h"_ostr, u"38100"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB"_ostr, "prst"_ostr, u"cross"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr"_ostr, "val"_ostr, u"accent2"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, u"20000"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, u"80000"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr/w14:srgbClr"_ostr, "val"_ostr, u"92D050"_ustr);

    // Paragraph 2 - w14:ligatures
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures"_ostr, "val"_ostr, u"standard"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures"_ostr, "val"_ostr, u"standardContextual"_ustr);

    // Paragraph 3 - w14:numForm and w14:numSpacing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm"_ostr, "val"_ostr, u"lining"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing"_ostr, "val"_ostr, u"tabular"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm"_ostr, "val"_ostr, u"oldStyle"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing"_ostr, "val"_ostr, u"proportional"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_StylisticSets_CntxtAlts)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Paragraph 1 - w14:stylisticSets
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, "id"_ostr, u"4"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, "id"_ostr, u"2"_ustr);

    // Paragraph 1 - w14:cntxtAlts
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:cntxtAlts"_ostr, 1);

}

CPPUNIT_TEST_FIXTURE(Test, Test_McIgnorable)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document"_ostr, "Ignorable"_ostr, u"w14 wp14 w15"_ustr);

    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);

    assertXPath(pXmlStyles, "/w:styles"_ostr, "Ignorable"_ostr, u"w14"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, Test_CompatSettingsForW14)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting"_ostr, 5);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]"_ostr, "name"_ostr, u"compatibilityMode"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]"_ostr, "uri"_ostr, u"http://schemas.microsoft.com/office/word"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]"_ostr, "val"_ostr, u"15"_ustr); // document was made with Word2013 -> 15

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]"_ostr, "name"_ostr, u"overrideTableStyleFontSizeAndJustification"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]"_ostr, "uri"_ostr, u"http://schemas.microsoft.com/office/word"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]"_ostr, "val"_ostr, u"1"_ustr);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]"_ostr, "name"_ostr, u"enableOpenTypeFeatures"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]"_ostr, "uri"_ostr, u"http://schemas.microsoft.com/office/word"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]"_ostr, "val"_ostr, u"1"_ustr);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]"_ostr, "name"_ostr, u"doNotFlipMirrorIndents"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]"_ostr, "uri"_ostr, u"http://schemas.microsoft.com/office/word"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]"_ostr, "val"_ostr, u"1"_ustr);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]"_ostr, "name"_ostr, u"differentiateMultirowTableHeaders"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]"_ostr, "uri"_ostr, u"http://schemas.microsoft.com/office/word"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]"_ostr, "val"_ostr, u"1"_ustr);
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
    assertXPath(pXmlDoc, sPathGlow, "rad"_ostr, u"127000"_ustr);
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr", "val"_ostr, u"00B050"_ustr);
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr/w14:alpha", "val"_ostr, u"60000"_ustr);

    // Shadow
    OString sPathShadow = sPathToShapeRunProperties + "/w14:shadow";
    assertXPath(pXmlDoc, sPathShadow, 1);
    assertXPath(pXmlDoc, sPathShadow, "blurRad"_ostr, u"127000"_ustr);
    assertXPath(pXmlDoc, sPathShadow, "dist"_ostr, u"787400"_ustr);
    assertXPath(pXmlDoc, sPathShadow, "dir"_ostr, u"12720000"_ustr);
    assertXPath(pXmlDoc, sPathShadow, "sx"_ostr, u"70000"_ustr);
    assertXPath(pXmlDoc, sPathShadow, "sy"_ostr, u"70000"_ustr);
    assertXPath(pXmlDoc, sPathShadow, "kx"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, sPathShadow, "ky"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, sPathShadow, "algn"_ostr, u"l"_ustr);
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val"_ostr, u"92D050"_ustr);
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val"_ostr, u"40000"_ustr);

    // Reflection
    OString sPathReflection = sPathToShapeRunProperties + "/w14:reflection";
    assertXPath(pXmlDoc, sPathReflection, 1);
    assertXPath(pXmlDoc, sPathReflection, "blurRad"_ostr, u"139700"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "stA"_ostr, u"47000"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "stPos"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "endA"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "endPos"_ostr, u"85000"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "dist"_ostr, u"63500"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "dir"_ostr, u"5400000"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "fadeDir"_ostr, u"5400000"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "sx"_ostr, u"100000"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "sy"_ostr, u"-100000"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "kx"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "ky"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, sPathReflection, "algn"_ostr, u"bl"_ustr);

    // TextOutline
    OString sPathTextOutline = sPathToShapeRunProperties + "/w14:textOutline";
    assertXPath(pXmlDoc, sPathTextOutline, 1);
    assertXPath(pXmlDoc, sPathTextOutline, "w"_ostr, u"25400"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline, "cap"_ostr, u"rnd"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline, "cmpd"_ostr, u"sng"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline, "algn"_ostr, u"ctr"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val"_ostr, u"accent2"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val"_ostr, u"75000"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val"_ostr, u"solid"_ustr);
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:round", 1);

    // TextFill
    OString sPathTextFill = sPathToShapeRunProperties + "/w14:textFill";
    assertXPath(pXmlDoc, sPathTextFill, 1);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val"_ostr, u"accent4"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos"_ostr, u"41000"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val"_ostr, u"accent6"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val"_ostr, u"60000"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos"_ostr, u"87000"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val"_ostr, u"accent5"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val"_ostr, u"60000"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff", "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "ang"_ostr, u"5400000"_ustr);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "scaled"_ostr, u"0"_ustr);

    // Props3D
    OString sPathProps3D = sPathToShapeRunProperties + "/w14:props3d";
    assertXPath(pXmlDoc, sPathProps3D, 1);
    assertXPath(pXmlDoc, sPathProps3D, "extrusionH"_ostr, u"63500"_ustr);
    assertXPath(pXmlDoc, sPathProps3D, "contourW"_ostr, u"12700"_ustr);
    assertXPath(pXmlDoc, sPathProps3D, "prstMaterial"_ostr, u"warmMatte"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w"_ostr, u"38100"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h"_ostr, u"38100"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst"_ostr, u"circle"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val"_ostr, u"accent2"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val"_ostr, u"60000"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val"_ostr, u"40000"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val"_ostr, u"accent4"_ustr);
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val"_ostr, u"75000"_ustr);

    // Ligatures
    OString sPathLigatures = sPathToShapeRunProperties + "/w14:ligatures";
    assertXPath(pXmlDoc, sPathLigatures, 1);
    assertXPath(pXmlDoc, sPathLigatures, "val"_ostr, u"standard"_ustr);

    // NumForm
    OString sPathNumForm = sPathToShapeRunProperties + "/w14:numForm";
    assertXPath(pXmlDoc, sPathNumForm, 1);
    assertXPath(pXmlDoc, sPathNumForm, "val"_ostr, u"oldStyle"_ustr);

    // NumSpacing
    OString sPathNumSpacing = sPathToShapeRunProperties + "/w14:numSpacing";
    assertXPath(pXmlDoc, sPathNumSpacing, 1);
    assertXPath(pXmlDoc, sPathNumSpacing, "val"_ostr, u"tabular"_ustr);

    // StylisticSets
    OString sPathStylisticSets = sPathToShapeRunProperties + "/w14:stylisticSets";
    assertXPath(pXmlDoc, sPathStylisticSets, 1);
    assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_InStyleXml)
{
    loadAndReload("TextEffects_InStyle.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);

    OString sPathToCharacterStyle = "/w:styles/w:style[3]"_ostr;

    assertXPath(pXmlDoc, sPathToCharacterStyle, "type"_ostr, u"character"_ustr);
    assertXPath(pXmlDoc, sPathToCharacterStyle, "styleId"_ostr, u"TextEffectsStyleChar"_ustr);
    assertXPath(pXmlDoc, sPathToCharacterStyle, "customStyle"_ostr, u"1"_ustr);

    {
        OString sPathToRun = sPathToCharacterStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad"_ostr, u"63500"_ustr);
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val"_ostr, u"accent2"_ustr);
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val"_ostr, u"60000"_ustr);
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val"_ostr, u"175000"_ustr);

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad"_ostr, u"50800"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "dist"_ostr, u"38100"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "dir"_ostr, u"16200000"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "sx"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "sy"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "kx"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "ky"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "algn"_ostr, u"b"_ustr);
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val"_ostr, u"000000"_ustr);
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val"_ostr, u"60000"_ustr);

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad"_ostr, u"6350"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "stA"_ostr, u"50000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "stPos"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "endA"_ostr, u"300"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "endPos"_ostr, u"50000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "dist"_ostr, u"29997"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "dir"_ostr, u"5400000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "fadeDir"_ostr, u"5400000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "sx"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "sy"_ostr, u"-100000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "kx"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "ky"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "algn"_ostr, u"bl"_ustr);

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w"_ostr, u"25400"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline, "cap"_ostr, u"rnd"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd"_ostr, u"sng"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline, "algn"_ostr, u"ctr"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val"_ostr, u"40000"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val"_ostr, u"75000"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val"_ostr, u"solid"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val"_ostr, u"10000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val"_ostr, u"40000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val"_ostr, u"60000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos"_ostr, u"46000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val"_ostr, u"95000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val"_ostr, u"5000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val"_ostr, u"60000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path"_ostr, u"circle"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b"_ostr, u"-30000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r"_ostr, u"50000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t"_ostr, u"130000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l"_ostr, u"50000"_ustr);

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH"_ostr, u"57150"_ustr);
        assertXPath(pXmlDoc, sPathProps3D, "contourW"_ostr, u"12700"_ustr);
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial"_ostr, u"metal"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w"_ostr, u"38100"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h"_ostr, u"38100"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst"_ostr, u"angle"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w"_ostr, u"69850"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h"_ostr, u"69850"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst"_ostr, u"divot"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val"_ostr, u"20000"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val"_ostr, u"80000"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val"_ostr, u"75000"_ustr);

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val"_ostr, u"standard"_ustr);

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val"_ostr, u"lining"_ustr);

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val"_ostr, u"proportional"_ustr);

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id"_ostr, u"1"_ustr);
    }

    OString sPathToParagraphStyle = "/w:styles/w:style[9]"_ostr;

    assertXPath(pXmlDoc, sPathToParagraphStyle, "type"_ostr, u"paragraph"_ustr);
    assertXPath(pXmlDoc, sPathToParagraphStyle, "styleId"_ostr, u"TextEffectsStyle"_ustr);
    assertXPath(pXmlDoc, sPathToParagraphStyle, "customStyle"_ostr, u"1"_ustr);

    {
        OString sPathToRun = sPathToParagraphStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad"_ostr, u"63500"_ustr);
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val"_ostr, u"accent2"_ustr);
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val"_ostr, u"60000"_ustr);
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val"_ostr, u"175000"_ustr);

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad"_ostr, u"50800"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "dist"_ostr, u"38100"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "dir"_ostr, u"16200000"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "sx"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "sy"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "kx"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "ky"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathShadow, "algn"_ostr, u"b"_ustr);
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val"_ostr, u"000000"_ustr);
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val"_ostr, u"60000"_ustr);

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad"_ostr, u"6350"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "stA"_ostr, u"50000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "stPos"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "endA"_ostr, u"300"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "endPos"_ostr, u"50000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "dist"_ostr, u"29997"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "dir"_ostr, u"5400000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "fadeDir"_ostr, u"5400000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "sx"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "sy"_ostr, u"-100000"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "kx"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "ky"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathReflection, "algn"_ostr, u"bl"_ustr);

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w"_ostr, u"25400"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline, "cap"_ostr, u"rnd"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd"_ostr, u"sng"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline, "algn"_ostr, u"ctr"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val"_ostr, u"40000"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val"_ostr, u"75000"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val"_ostr, u"solid"_ustr);
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos"_ostr, u"0"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val"_ostr, u"10000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val"_ostr, u"40000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val"_ostr, u"60000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos"_ostr, u"46000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val"_ostr, u"95000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val"_ostr, u"5000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos"_ostr, u"100000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val"_ostr, u"60000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path"_ostr, u"circle"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b"_ostr, u"-30000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r"_ostr, u"50000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t"_ostr, u"130000"_ustr);
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l"_ostr, u"50000"_ustr);

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH"_ostr, u"57150"_ustr);
        assertXPath(pXmlDoc, sPathProps3D, "contourW"_ostr, u"12700"_ustr);
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial"_ostr, u"metal"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w"_ostr, u"38100"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h"_ostr, u"38100"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst"_ostr, u"angle"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w"_ostr, u"69850"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h"_ostr, u"69850"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst"_ostr, u"divot"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val"_ostr, u"20000"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val"_ostr, u"80000"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val"_ostr, u"accent1"_ustr);
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val"_ostr, u"75000"_ustr);

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val"_ostr, u"standard"_ustr);

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val"_ostr, u"lining"_ustr);

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val"_ostr, u"proportional"_ustr);

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id"_ostr, u"1"_ustr);
    }
}

CPPUNIT_TEST_FIXTURE(Test, Test_no_tag_if_no_fill)
{
    loadAndReload("tdf112103_tablebgnofill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:shd"_ostr, 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
