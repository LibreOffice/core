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
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}
};

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_GlowShadowReflection)
{
    loadAndReload("TextEffects_Glow_Shadow_Reflection.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

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
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Paragraph 1
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "w"_ostr, "50800");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cap"_ostr, "rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cmpd"_ostr, "dbl");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline"_ostr, "algn"_ostr, "ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]"_ostr, "pos"_ostr, "70000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr"_ostr, "val"_ostr, "92D050");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:alpha"_ostr, "val"_ostr, "30000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumMod"_ostr, "val"_ostr, "75000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[1]/w14:srgbClr/w14:lumOff"_ostr, "val"_ostr, "25000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]"_ostr, "pos"_ostr, "30000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr"_ostr, "val"_ostr, "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, "55000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, "60000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]"_ostr, "pos"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[3]/w14:srgbClr"_ostr, "val"_ostr, "0070C0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]"_ostr, "pos"_ostr, "100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:gsLst/w14:gs[4]/w14:schemeClr"_ostr, "val"_ostr, "accent4");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin"_ostr, "ang"_ostr, "3600000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:gradFill/w14:lin"_ostr, "scaled"_ostr, "0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash"_ostr, "val"_ostr, "dash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:textOutline/w14:miter"_ostr, "lim"_ostr, "0");

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "w"_ostr, "9525");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cap"_ostr, "rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cmpd"_ostr, "sng");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline"_ostr, "algn"_ostr, "ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:solidFill/w14:srgbClr"_ostr, "val"_ostr, "FF0000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash"_ostr, "val"_ostr, "solid");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textOutline/w14:bevel"_ostr, 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "w"_ostr, "9525");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cap"_ostr, "rnd");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "cmpd"_ostr, "sng");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline"_ostr, "algn"_ostr, "ctr");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:noFill"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:prstDash"_ostr, "val"_ostr, "solid");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textOutline/w14:bevel"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_TextFill)
{
    loadAndReload("TextEffects_TextFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Paragraph 1 has no textFill

    // Paragraph 2
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:textFill/w14:noFill"_ostr, 1);

    // Paragraph 3
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr"_ostr, "val"_ostr, "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, "5000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, "40000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:rPr/w14:textFill/w14:solidFill/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, "60000");

    // Paragraph 4
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]"_ostr, "pos"_ostr, "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr"_ostr, "val"_ostr, "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, "5000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, "67000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]"_ostr, "pos"_ostr, "50000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr"_ostr, "val"_ostr, "00B0F0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:alpha"_ostr, "val"_ostr, "10000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[2]/w14:srgbClr/w14:lumMod"_ostr, "val"_ostr, "80000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]"_ostr, "pos"_ostr, "100000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr"_ostr, "val"_ostr, "accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:alpha"_ostr, "val"_ostr, "15000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, "60000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, "40000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin"_ostr, "ang"_ostr, "16200000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w14:textFill/w14:gradFill/w14:lin"_ostr, "scaled"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_Props3d_Ligatures_NumForm_NumSpacing)
{
    loadAndReload("TextEffects_Props3d_Ligatures_NumForm_NumSpacing.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Paragraph 1 - w14:props3d
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, "extrusionH"_ostr, "63500");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, "contourW"_ostr, "25400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d"_ostr, "prstMaterial"_ostr, "softEdge");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT"_ostr, "w"_ostr, "38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT"_ostr, "h"_ostr, "38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelT"_ostr, "prst"_ostr, "relaxedInset");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB"_ostr, "w"_ostr, "69850");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB"_ostr, "h"_ostr, "38100");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:bevelB"_ostr, "prst"_ostr, "cross");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr"_ostr, "val"_ostr, "accent2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumMod"_ostr, "val"_ostr, "20000");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:extrusionClr/w14:schemeClr/w14:lumOff"_ostr, "val"_ostr, "80000");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:props3d/w14:contourClr/w14:srgbClr"_ostr, "val"_ostr, "92D050");

    // Paragraph 2 - w14:ligatures
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:ligatures"_ostr, "val"_ostr, "standard");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:rPr/w14:ligatures"_ostr, "val"_ostr, "standardContextual");

    // Paragraph 3 - w14:numForm and w14:numSpacing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numForm"_ostr, "val"_ostr, "lining");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[2]/w:rPr/w14:numSpacing"_ostr, "val"_ostr, "tabular");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numForm"_ostr, "val"_ostr, "oldStyle");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr/w14:numSpacing"_ostr, "val"_ostr, "proportional");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_StylisticSets_CntxtAlts)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Paragraph 1 - w14:stylisticSets
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, "id"_ostr, "4");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:rPr/w14:stylisticSets/w14:styleSet"_ostr, "id"_ostr, "2");

    // Paragraph 1 - w14:cntxtAlts
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:rPr/w14:cntxtAlts"_ostr, 1);

}

CPPUNIT_TEST_FIXTURE(Test, Test_McIgnorable)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    assertXPath(pXmlDocument, "/w:document"_ostr, "Ignorable"_ostr, "w14 wp14 w15");

    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");

    assertXPath(pXmlStyles, "/w:styles"_ostr, "Ignorable"_ostr, "w14");
}

CPPUNIT_TEST_FIXTURE(Test, Test_CompatSettingsForW14)
{
    loadAndReload("TextEffects_StylisticSets_CntxtAlts.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/settings.xml");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting"_ostr, 5);

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]"_ostr, "name"_ostr, "compatibilityMode");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]"_ostr, "uri"_ostr, "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]"_ostr, "val"_ostr, "15"); // document was made with Word2013 -> 15

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]"_ostr, "name"_ostr, "overrideTableStyleFontSizeAndJustification");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]"_ostr, "uri"_ostr, "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[2]"_ostr, "val"_ostr, "1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]"_ostr, "name"_ostr, "enableOpenTypeFeatures");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]"_ostr, "uri"_ostr, "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[3]"_ostr, "val"_ostr, "1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]"_ostr, "name"_ostr, "doNotFlipMirrorIndents");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]"_ostr, "uri"_ostr, "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[4]"_ostr, "val"_ostr, "1");

    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]"_ostr, "name"_ostr, "differentiateMultirowTableHeaders");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]"_ostr, "uri"_ostr, "http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[5]"_ostr, "val"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_Groupshapes)
{
    loadAndReload("TextEffects_Groupshapes.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    OString sPathToWGP = "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wpg:wgp"_ostr;

    assertXPath(pXmlDoc, sPathToWGP + "/wps:wsp", 2);

    OString sPathToShapeRunProperties = sPathToWGP + "/wps:wsp[2]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr";

    // Glow
    OString sPathGlow = sPathToShapeRunProperties + "/w14:glow";
    assertXPath(pXmlDoc, sPathGlow, 1);
    assertXPath(pXmlDoc, sPathGlow, "rad"_ostr, "127000");
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr", "val"_ostr, "00B050");
    assertXPath(pXmlDoc, sPathGlow+"/w14:srgbClr/w14:alpha", "val"_ostr, "60000");

    // Shadow
    OString sPathShadow = sPathToShapeRunProperties + "/w14:shadow";
    assertXPath(pXmlDoc, sPathShadow, 1);
    assertXPath(pXmlDoc, sPathShadow, "blurRad"_ostr, "127000");
    assertXPath(pXmlDoc, sPathShadow, "dist"_ostr, "787400");
    assertXPath(pXmlDoc, sPathShadow, "dir"_ostr, "12720000");
    assertXPath(pXmlDoc, sPathShadow, "sx"_ostr, "70000");
    assertXPath(pXmlDoc, sPathShadow, "sy"_ostr, "70000");
    assertXPath(pXmlDoc, sPathShadow, "kx"_ostr, "0");
    assertXPath(pXmlDoc, sPathShadow, "ky"_ostr, "0");
    assertXPath(pXmlDoc, sPathShadow, "algn"_ostr, "l");
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val"_ostr, "92D050");
    assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val"_ostr, "40000");

    // Reflection
    OString sPathReflection = sPathToShapeRunProperties + "/w14:reflection";
    assertXPath(pXmlDoc, sPathReflection, 1);
    assertXPath(pXmlDoc, sPathReflection, "blurRad"_ostr, "139700");
    assertXPath(pXmlDoc, sPathReflection, "stA"_ostr, "47000");
    assertXPath(pXmlDoc, sPathReflection, "stPos"_ostr, "0");
    assertXPath(pXmlDoc, sPathReflection, "endA"_ostr, "0");
    assertXPath(pXmlDoc, sPathReflection, "endPos"_ostr, "85000");
    assertXPath(pXmlDoc, sPathReflection, "dist"_ostr, "63500");
    assertXPath(pXmlDoc, sPathReflection, "dir"_ostr, "5400000");
    assertXPath(pXmlDoc, sPathReflection, "fadeDir"_ostr, "5400000");
    assertXPath(pXmlDoc, sPathReflection, "sx"_ostr, "100000");
    assertXPath(pXmlDoc, sPathReflection, "sy"_ostr, "-100000");
    assertXPath(pXmlDoc, sPathReflection, "kx"_ostr, "0");
    assertXPath(pXmlDoc, sPathReflection, "ky"_ostr, "0");
    assertXPath(pXmlDoc, sPathReflection, "algn"_ostr, "bl");

    // TextOutline
    OString sPathTextOutline = sPathToShapeRunProperties + "/w14:textOutline";
    assertXPath(pXmlDoc, sPathTextOutline, 1);
    assertXPath(pXmlDoc, sPathTextOutline, "w"_ostr, "25400");
    assertXPath(pXmlDoc, sPathTextOutline, "cap"_ostr, "rnd");
    assertXPath(pXmlDoc, sPathTextOutline, "cmpd"_ostr, "sng");
    assertXPath(pXmlDoc, sPathTextOutline, "algn"_ostr, "ctr");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val"_ostr, "accent2");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val"_ostr, "40000");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val"_ostr, "75000");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val"_ostr, "solid");
    assertXPath(pXmlDoc, sPathTextOutline+"/w14:round", 1);

    // TextFill
    OString sPathTextFill = sPathToShapeRunProperties + "/w14:textFill";
    assertXPath(pXmlDoc, sPathTextFill, 1);
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos"_ostr, "0");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val"_ostr, "accent4");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos"_ostr, "41000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val"_ostr, "accent6");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val"_ostr, "60000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val"_ostr, "40000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos"_ostr, "87000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val"_ostr, "accent5");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val"_ostr, "60000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumOff", "val"_ostr, "40000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "ang"_ostr, "5400000");
    assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:lin", "scaled"_ostr, "0");

    // Props3D
    OString sPathProps3D = sPathToShapeRunProperties + "/w14:props3d";
    assertXPath(pXmlDoc, sPathProps3D, 1);
    assertXPath(pXmlDoc, sPathProps3D, "extrusionH"_ostr, "63500");
    assertXPath(pXmlDoc, sPathProps3D, "contourW"_ostr, "12700");
    assertXPath(pXmlDoc, sPathProps3D, "prstMaterial"_ostr, "warmMatte");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w"_ostr, "38100");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h"_ostr, "38100");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst"_ostr, "circle");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val"_ostr, "accent2");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val"_ostr, "60000");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val"_ostr, "40000");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val"_ostr, "accent4");
    assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val"_ostr, "75000");

    // Ligatures
    OString sPathLigatures = sPathToShapeRunProperties + "/w14:ligatures";
    assertXPath(pXmlDoc, sPathLigatures, 1);
    assertXPath(pXmlDoc, sPathLigatures, "val"_ostr, "standard");

    // NumForm
    OString sPathNumForm = sPathToShapeRunProperties + "/w14:numForm";
    assertXPath(pXmlDoc, sPathNumForm, 1);
    assertXPath(pXmlDoc, sPathNumForm, "val"_ostr, "oldStyle");

    // NumSpacing
    OString sPathNumSpacing = sPathToShapeRunProperties + "/w14:numSpacing";
    assertXPath(pXmlDoc, sPathNumSpacing, 1);
    assertXPath(pXmlDoc, sPathNumSpacing, "val"_ostr, "tabular");

    // StylisticSets
    OString sPathStylisticSets = sPathToShapeRunProperties + "/w14:stylisticSets";
    assertXPath(pXmlDoc, sPathStylisticSets, 1);
    assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Test, Test_TextEffects_InStyleXml)
{
    loadAndReload("TextEffects_InStyle.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");

    OString sPathToCharacterStyle = "/w:styles/w:style[3]"_ostr;

    assertXPath(pXmlDoc, sPathToCharacterStyle, "type"_ostr, "character");
    assertXPath(pXmlDoc, sPathToCharacterStyle, "styleId"_ostr, "TextEffectsStyleChar");
    assertXPath(pXmlDoc, sPathToCharacterStyle, "customStyle"_ostr, "1");

    {
        OString sPathToRun = sPathToCharacterStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad"_ostr, "63500");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val"_ostr, "accent2");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val"_ostr, "60000");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val"_ostr, "175000");

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad"_ostr, "50800");
        assertXPath(pXmlDoc, sPathShadow, "dist"_ostr, "38100");
        assertXPath(pXmlDoc, sPathShadow, "dir"_ostr, "16200000");
        assertXPath(pXmlDoc, sPathShadow, "sx"_ostr, "100000");
        assertXPath(pXmlDoc, sPathShadow, "sy"_ostr, "100000");
        assertXPath(pXmlDoc, sPathShadow, "kx"_ostr, "0");
        assertXPath(pXmlDoc, sPathShadow, "ky"_ostr, "0");
        assertXPath(pXmlDoc, sPathShadow, "algn"_ostr, "b");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val"_ostr, "000000");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val"_ostr, "60000");

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad"_ostr, "6350");
        assertXPath(pXmlDoc, sPathReflection, "stA"_ostr, "50000");
        assertXPath(pXmlDoc, sPathReflection, "stPos"_ostr, "0");
        assertXPath(pXmlDoc, sPathReflection, "endA"_ostr, "300");
        assertXPath(pXmlDoc, sPathReflection, "endPos"_ostr, "50000");
        assertXPath(pXmlDoc, sPathReflection, "dist"_ostr, "29997");
        assertXPath(pXmlDoc, sPathReflection, "dir"_ostr, "5400000");
        assertXPath(pXmlDoc, sPathReflection, "fadeDir"_ostr, "5400000");
        assertXPath(pXmlDoc, sPathReflection, "sx"_ostr, "100000");
        assertXPath(pXmlDoc, sPathReflection, "sy"_ostr, "-100000");
        assertXPath(pXmlDoc, sPathReflection, "kx"_ostr, "0");
        assertXPath(pXmlDoc, sPathReflection, "ky"_ostr, "0");
        assertXPath(pXmlDoc, sPathReflection, "algn"_ostr, "bl");

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w"_ostr, "25400");
        assertXPath(pXmlDoc, sPathTextOutline, "cap"_ostr, "rnd");
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd"_ostr, "sng");
        assertXPath(pXmlDoc, sPathTextOutline, "algn"_ostr, "ctr");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val"_ostr, "40000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val"_ostr, "75000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val"_ostr, "solid");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos"_ostr, "0");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val"_ostr, "10000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val"_ostr, "40000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val"_ostr, "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos"_ostr, "46000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val"_ostr, "95000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val"_ostr, "5000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos"_ostr, "100000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val"_ostr, "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path"_ostr, "circle");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b"_ostr, "-30000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r"_ostr, "50000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t"_ostr, "130000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l"_ostr, "50000");

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH"_ostr, "57150");
        assertXPath(pXmlDoc, sPathProps3D, "contourW"_ostr, "12700");
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial"_ostr, "metal");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w"_ostr, "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h"_ostr, "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst"_ostr, "angle");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w"_ostr, "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h"_ostr, "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst"_ostr, "divot");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val"_ostr, "20000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val"_ostr, "80000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val"_ostr, "75000");

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val"_ostr, "standard");

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val"_ostr, "lining");

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val"_ostr, "proportional");

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id"_ostr, "1");
    }

    OString sPathToParagraphStyle = "/w:styles/w:style[9]"_ostr;

    assertXPath(pXmlDoc, sPathToParagraphStyle, "type"_ostr, "paragraph");
    assertXPath(pXmlDoc, sPathToParagraphStyle, "styleId"_ostr, "TextEffectsStyle");
    assertXPath(pXmlDoc, sPathToParagraphStyle, "customStyle"_ostr, "1");

    {
        OString sPathToRun = sPathToParagraphStyle + "/w:rPr";

        // Glow
        OString sPathGlow = sPathToRun + "/w14:glow";
        assertXPath(pXmlDoc, sPathGlow, "rad"_ostr, "63500");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr", "val"_ostr, "accent2");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:alpha", "val"_ostr, "60000");
        assertXPath(pXmlDoc, sPathGlow+"/w14:schemeClr/w14:satMod", "val"_ostr, "175000");

        // Shadow
        OString sPathShadow = sPathToRun + "/w14:shadow";
        assertXPath(pXmlDoc, sPathShadow, "blurRad"_ostr, "50800");
        assertXPath(pXmlDoc, sPathShadow, "dist"_ostr, "38100");
        assertXPath(pXmlDoc, sPathShadow, "dir"_ostr, "16200000");
        assertXPath(pXmlDoc, sPathShadow, "sx"_ostr, "100000");
        assertXPath(pXmlDoc, sPathShadow, "sy"_ostr, "100000");
        assertXPath(pXmlDoc, sPathShadow, "kx"_ostr, "0");
        assertXPath(pXmlDoc, sPathShadow, "ky"_ostr, "0");
        assertXPath(pXmlDoc, sPathShadow, "algn"_ostr, "b");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr", "val"_ostr, "000000");
        assertXPath(pXmlDoc, sPathShadow+"/w14:srgbClr/w14:alpha", "val"_ostr, "60000");

        // Reflection
        OString sPathReflection = sPathToRun + "/w14:reflection";
        assertXPath(pXmlDoc, sPathReflection, "blurRad"_ostr, "6350");
        assertXPath(pXmlDoc, sPathReflection, "stA"_ostr, "50000");
        assertXPath(pXmlDoc, sPathReflection, "stPos"_ostr, "0");
        assertXPath(pXmlDoc, sPathReflection, "endA"_ostr, "300");
        assertXPath(pXmlDoc, sPathReflection, "endPos"_ostr, "50000");
        assertXPath(pXmlDoc, sPathReflection, "dist"_ostr, "29997");
        assertXPath(pXmlDoc, sPathReflection, "dir"_ostr, "5400000");
        assertXPath(pXmlDoc, sPathReflection, "fadeDir"_ostr, "5400000");
        assertXPath(pXmlDoc, sPathReflection, "sx"_ostr, "100000");
        assertXPath(pXmlDoc, sPathReflection, "sy"_ostr, "-100000");
        assertXPath(pXmlDoc, sPathReflection, "kx"_ostr, "0");
        assertXPath(pXmlDoc, sPathReflection, "ky"_ostr, "0");
        assertXPath(pXmlDoc, sPathReflection, "algn"_ostr, "bl");

        // TextOutline
        OString sPathTextOutline = sPathToRun + "/w14:textOutline";
        assertXPath(pXmlDoc, sPathTextOutline, "w"_ostr, "25400");
        assertXPath(pXmlDoc, sPathTextOutline, "cap"_ostr, "rnd");
        assertXPath(pXmlDoc, sPathTextOutline, "cmpd"_ostr, "sng");
        assertXPath(pXmlDoc, sPathTextOutline, "algn"_ostr, "ctr");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:alpha", "val"_ostr, "40000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:solidFill/w14:schemeClr/w14:lumMod", "val"_ostr, "75000");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:prstDash", "val"_ostr, "solid");
        assertXPath(pXmlDoc, sPathTextOutline+"/w14:bevel", 1);

        // TextFill
        OString sPathTextFill = sPathToRun + "/w14:textFill";
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]", "pos"_ostr, "0");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:alpha", "val"_ostr, "10000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumMod", "val"_ostr, "40000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[1]/w14:schemeClr/w14:lumOff", "val"_ostr, "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]", "pos"_ostr, "46000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumMod", "val"_ostr, "95000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[2]/w14:schemeClr/w14:lumOff", "val"_ostr, "5000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]", "pos"_ostr, "100000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:gsLst/w14:gs[3]/w14:schemeClr/w14:lumMod", "val"_ostr, "60000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path", "path"_ostr, "circle");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "b"_ostr, "-30000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "r"_ostr, "50000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "t"_ostr, "130000");
        assertXPath(pXmlDoc, sPathTextFill+"/w14:gradFill/w14:path/w14:fillToRect", "l"_ostr, "50000");

        // Props3D
        OString sPathProps3D = sPathToRun + "/w14:props3d";
        assertXPath(pXmlDoc, sPathProps3D, "extrusionH"_ostr, "57150");
        assertXPath(pXmlDoc, sPathProps3D, "contourW"_ostr, "12700");
        assertXPath(pXmlDoc, sPathProps3D, "prstMaterial"_ostr, "metal");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "w"_ostr, "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "h"_ostr, "38100");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelT", "prst"_ostr, "angle");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "w"_ostr, "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "h"_ostr, "69850");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:bevelB", "prst"_ostr, "divot");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumMod", "val"_ostr, "20000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:extrusionClr/w14:schemeClr/w14:lumOff", "val"_ostr, "80000");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr", "val"_ostr, "accent1");
        assertXPath(pXmlDoc, sPathProps3D+"/w14:contourClr/w14:schemeClr/w14:lumMod", "val"_ostr, "75000");

        // Ligatures
        OString sPathLigatures = sPathToRun + "/w14:ligatures";
        assertXPath(pXmlDoc, sPathLigatures, "val"_ostr, "standard");

        // NumForm
        OString sPathNumForm = sPathToRun + "/w14:numForm";
        assertXPath(pXmlDoc, sPathNumForm, "val"_ostr, "lining");

        // NumSpacing
        OString sPathNumSpacing = sPathToRun + "/w14:numSpacing";
        assertXPath(pXmlDoc, sPathNumSpacing, "val"_ostr, "proportional");

        // StylisticSets
        OString sPathStylisticSets = sPathToRun + "/w14:stylisticSets";
        assertXPath(pXmlDoc, sPathStylisticSets+"/w14:styleSet", "id"_ostr, "1");
    }
}

CPPUNIT_TEST_FIXTURE(Test, Test_no_tag_if_no_fill)
{
    loadAndReload("tdf112103_tablebgnofill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:shd"_ostr, 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
