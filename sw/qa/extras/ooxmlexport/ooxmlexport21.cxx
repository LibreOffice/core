/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

#include <comphelper/sequenceashashmap.hxx>

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

DECLARE_OOXMLEXPORT_TEST(testTdf158597, "tdf158597.docx")
{
    // test with 2 properties: font size, italic (toggle)
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
        CPPUNIT_ASSERT_EQUAL(OUString("No style"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(2));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        CPPUNIT_ASSERT(!listAutoFormat["CharPosture"].hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(3));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style mark and text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        CPPUNIT_ASSERT(!listAutoFormat["CharPosture"].hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(4));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(5));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(6));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        // bug was that here the toggle property was not overwritten
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(7));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style mark and text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat["CharHeight"].hasValue());
        // bug was that here the toggle property was not overwritten
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(8));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(9));
        CPPUNIT_ASSERT_EQUAL(OUString("No style + direct mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT(!listAutoFormat["CharStyleName"].hasValue());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(10));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style + direct mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(11));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style + direct mark and text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(12));
        CPPUNIT_ASSERT_EQUAL(OUString("Char style + direct text"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(13));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + direct mark"), xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT(!listAutoFormat["CharStyleName"].hasValue());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(14));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style + direct mark"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(15));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style + direct mark and text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        comphelper::SequenceAsHashMap listAutoFormat(xProps->getPropertyValue("ListAutoFormat"));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), listAutoFormat["CharStyleName"].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat["CharHeight"].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat["CharPosture"].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(16));
        CPPUNIT_ASSERT_EQUAL(OUString("Para style + Char style + direct text"),
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("List Paragraph E"),
                             getProperty<OUString>(xProps, "ParaStyleName"));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"), getProperty<OUString>(xRun, "CharStyleName"));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, "CharPosture"));
        CPPUNIT_ASSERT(!xProps->getPropertyValue("ListAutoFormat").hasValue());
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMarginVML, "tdf160049_anchorMarginVML.docx")
{
    // given a VML (Word 2003) document with a LEFT "column/text" anchored image
    // (which will import as DML compat12 on the round-trip)

    // The image takes into account the margin, so it looks like it is in the middle of the doc,
    // which is "Paragraph text area"/PRINT_AREA/1, not "Entire paragraph area"/FRAME/0
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMargin15, "tdf160049_anchorMargin15.docx")
{
    // given a DML compat15 (Word 2013) document with a LEFT "column/text" anchored image

    // The image ignores the margin, so it is at the left for compat15 (but middle-ish for compat14)
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153909_followTextFlow, "tdf153909_followTextFlow.docx")
{
    // Although MSO's UI reports "layoutInCell" for the rectangle, it isn't specified or honored
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "IsFollowingTextFlow"));

    // Given a table with a rectangle anchored in it (wrap-through) that appears above the table...
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nRectBottom
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "bottom"_ostr).toInt32();
    sal_Int32 nTableTop = getXPath(pDump, "//tab/row/infos/bounds"_ostr, "top"_ostr).toInt32();
    // The entire table must be below the rectangle
    CPPUNIT_ASSERT(nTableTop > nRectBottom);
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

DECLARE_OOXMLEXPORT_TEST(testTdf126533_noPageBitmap, "tdf126533_noPageBitmap.docx")
{
    // given a document with a v:background bitmap, but no w:background fillcolor
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // the image (or any fill for that matter) should be ignored.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(xPageStyle, "FillStyle"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf126533_pageGradient, "fill.docx")
{
    // given a document with a gradient page background
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xPageStyle, "FillStyle"));

    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xPageStyle, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RECT, aGradient.Style);
}

DECLARE_OOXMLEXPORT_TEST(testTdf126533_pageBitmap, "tdf126533_pageBitmap.docx")
{
    // given a document with a page background image
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP,
                         getProperty<drawing::FillStyle>(xPageStyle, "FillStyle"));

    if (!isExported())
        return;

    xmlDocUniquePtr pXmlDocRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDocRels,
                "/rels:Relationships/rels:Relationship[@Target='media/image1.jpeg']"_ostr, 1);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
