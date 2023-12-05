/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf153909_followTextFlow, "tdf153909_followTextFlow.docx")
{
    // Although MSO's UI reports "layoutInCell" for the rectangle, it isn't specified or honored
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "IsFollowingTextFlow"));

    // Given a table with a rectangle anchored in it (wrap-through) that appears above the table...
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nRectBottom
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds", "bottom").toInt32();
    sal_Int32 nTableTop = getXPath(pDump, "//tab/row/infos/bounds", "top").toInt32();
    // The entire table must be below the rectangle
    CPPUNIT_ASSERT(nTableTop > nRectBottom);
}

CPPUNIT_TEST_FIXTURE(Test, testtdf158044)
{
    loadAndSave("tdf158044.odt");
    // write hard attributes to prevent multiple toggle attributes from vanishing
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:rStyle[1]", "val",
                "BoldItalicCapsEmbossedStrike");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:b[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:bCs[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:i[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:iCs[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:strike[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:rPr[1]/w:emboss[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr[1]/w:rStyle[1]", "val",
                "SmallcapsImprint");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr[1]/w:imprint[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[4]/w:rPr[1]/w:smallCaps[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[2]/w:rPr[1]/w:rStyle[1]", "val", "AllCaps");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[2]/w:rPr[1]/w:caps[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r[2]/w:rPr[1]/w:rStyle[1]", "val", "Hidden");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:r[2]/w:rPr[1]/w:vanish[1]");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r[4]/w:rPr[1]/w:rStyle[1]", "val",
                "OutlineShadow");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r[4]/w:rPr[1]/w:outline[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:r[4]/w:rPr[1]/w:shadow[1]");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159207_footerFramePrBorder)
{
    loadFromFile(u"tdf159207_footerFramePrBorder.docx"); // re-imports as editeng Frame/Shape

    // given a doc with footer paragraphs frame (with a top border, but no left border)
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame0(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xFrame0, "LeftBorder");
    sal_uInt32 nBorderWidth
        = aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance;
    // Without patch it failed with Expected 0, Actual 26
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left border:", static_cast<sal_uInt32>(0), nBorderWidth);

    // TODO: there SHOULD BE a top border, and even if loaded, it would be lost on re-import...
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
