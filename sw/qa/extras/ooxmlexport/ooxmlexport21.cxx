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

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMargin2, "tdf160049_anchorMargin2.docx")
{
    // given a DML compat14 (Word 2010) document with a LEFT "column/text" anchored shape

    // The shape takes into account the margin, so it looks like it is in the middle of the doc,
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

DECLARE_OOXMLEXPORT_TEST(testTdf160077_layoutInCell, "tdf160077_layoutInCell.docx")
{
    // given an in-table, slightly rotated image vertically aligned -1cm (above) the top page margin
    // (which is actually forced to layoutInCell, so that becomes 1cm above the paragraph instead)

    xmlDocUniquePtr pDump = parseLayoutDump();
    const sal_Int32 nCellTop
        = getXPath(pDump, "//row[1]/cell[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    const sal_Int32 nParaTop
        = getXPath(pDump, "//row[1]/cell[1]/txt/infos/bounds"_ostr, "top"_ostr).toInt32();
    const sal_Int32 nImageTop
        = getXPath(pDump, "//row[1]/cell[1]/txt/anchored/SwAnchoredDrawObject/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    // The image is approximately half-way between cell top and the start of the text
    // correct ImageTop: 3588, while incorrect value was 1117. Cell top is 3051, ParaTop is 4195
    const sal_Int32 nHalfway = nCellTop + (nParaTop - nCellTop) / 2;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nHalfway, nImageTop, 50); // +/- 4.4%

    // The effect is implemented by forcing "Entire paragraph area"/FRAME/0
    // instead of "Page text area"/PAGE_PRINT_AREA/8
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(getShape(1), "VertOrientRelation"));
    // since layoutInCell had been turned off. If the implementation changes, check the layout.
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "IsFollowingTextFlow"));
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

CPPUNIT_TEST_FIXTURE(Test, testTdf162370)
{
    // This must not crash on save; without the fix, it would fail with
    // "Assertion failed: vector subscript out of range"
    loadAndSave("too_many_styles.odt");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf162746)
{
    // Without the fix in place this hangs (and eventually OOMs) on opening
    loadAndSave("tdf162746.docx");
    // tdf#162781: test the page body table vertical offset and width
    xmlDocUniquePtr pDump = parseLayoutDump();
    // Without the fix, this would be 0 - i.e., the page body table didn't shift down
    // below the header's floating table
    assertXPath(pDump, "//page[1]/body/tab/infos/prtBounds"_ostr, "top"_ostr, u"35"_ustr);
    // Without the fix, this would be 100, because the page body table only used tiny space
    // to the left of the header's floating table
    assertXPath(pDump, "//page[1]/body/tab/infos/prtBounds"_ostr, "width"_ostr, u"9360"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testCommentWithChildrenTdf163092)
{
    loadAndSave("comment_with_children.odt");
    // commentsExtended should exist
    xmlDocUniquePtr pXmlCommExt = parseExport("word/commentsExtended.xml");
    CPPUNIT_ASSERT(pXmlCommExt);
    // And it should contain the same parent-child relations
    OUString sExChild1
        = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[1]"_ostr, "paraId"_ostr);
    OUString sExParent1
        = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[1]"_ostr, "paraIdParent"_ostr);
    OUString sExChild2
        = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[2]"_ostr, "paraId"_ostr);
    OUString sExParent2
        = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[2]"_ostr, "paraIdParent"_ostr);
    std::map<OUString, OUString> parents;
    parents[sExChild1] = sExParent1;
    parents[sExChild2] = sExParent2;
    xmlDocUniquePtr pXmlComments = parseExport("word/comments.xml");
    OUString sComment1Id
        = getXPath(pXmlComments, "/w:comments/w:comment[1]/w:p[1]"_ostr, "paraId"_ostr);
    OUString sComment2Id
        = getXPath(pXmlComments, "/w:comments/w:comment[2]/w:p[1]"_ostr, "paraId"_ostr);
    OUString sComment3Id
        = getXPath(pXmlComments, "/w:comments/w:comment[3]/w:p[1]"_ostr, "paraId"_ostr);
    CPPUNIT_ASSERT_EQUAL(parents[sComment2Id], sComment1Id);
    CPPUNIT_ASSERT_EQUAL(parents[sComment3Id], sComment2Id);
}

CPPUNIT_TEST_FIXTURE(Test, testMsWordUlTrailSpace)
{
    createSwDoc("UnderlineTrailingSpace.docx");
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true),
                             xSettings->getPropertyValue(u"MsWordUlTrailSpace"_ustr));
    }

    // Test also after save-and-reload:
    saveAndReload(u"Office Open XML Text"_ustr);
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true),
                             xSettings->getPropertyValue(u"MsWordUlTrailSpace"_ustr));
    }

    // Check that the compat setting is exported in OOXML
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:ulTrailSpace"_ostr);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
