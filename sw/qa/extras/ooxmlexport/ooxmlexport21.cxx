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

#include <comphelper/configuration.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>

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

DECLARE_OOXMLEXPORT_TEST(testTdf156105_percentSuffix, "tdf156105_percentSuffix.odt")
{
    // given a numbered list with a non-escaping percent symbol in the prefix and suffix
    CPPUNIT_ASSERT_EQUAL(OUString("(%)[%]"),
                         getProperty<OUString>(getParagraph(3), "ListLabelString"));

    // tdf#149258 - NONE number should not export separator since LO doesn't currently show it
    CPPUNIT_ASSERT_EQUAL_MESSAGE("showing levels 1, 2, and 4", OUString("(%)1.1.1[%]"),
                                 getProperty<OUString>(getParagraph(4), "ListLabelString"));
    if (isExported())
    {
        xmlDocUniquePtr pXmlNum = parseExport("word/numbering.xml");
        // The 3rd level is NONE. If we include the separator, MS Word will display it.
        assertXPath(pXmlNum, "/w:numbering/w:abstractNum[1]/w:lvl[4]/w:lvlText"_ostr, "val"_ostr,
                    "(%)%1.%2.%3%4[%]");
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

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMargin14, "tdf160049_anchorMargin14.docx")
{
    // given a DML compat14 (Word 2010) document with a LEFT "column/text" anchored image
    // followed by a RIGHT column/text anchored image (with a 2cm paragraph right margin)
    // followed by a CENTER column/text anchored image (with a large left margin)
    // followed by a LEFT FROM column/text anchored image (which ignores the left margin)
    // followed by a LEFT "margin" anchored image (which always ignores the left margin)

    // The shape takes into account the left margin, looking like it is in the middle of the doc,
    // which is "Paragraph text area"/PRINT_AREA/1, not "Entire paragraph area"/FRAME/0
    uno::Reference<drawing::XShape> xShape(getShapeByName(u"Picture 2"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    // The shape takes into account the right margin, looking like it is in the middle of the doc,
    xShape.set(getShapeByName(u"Picture 3"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    // The third shape takes "center" very seriously, and ignores the margins
    xShape.set(getShapeByName(u"Picture 4"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    // The fourth shape takes "left by 123", and ignores the margins, acting just like "margin"
    xShape.set(getShapeByName(u"Picture 5"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    // The fifth shape shows how "left" works with "margin", which apparently means page margin
    xShape.set(getShapeByName(u"Picture 6"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf160077_layoutInCellB, "tdf160077_layoutInCellB.docx")
{
    // given an in-table, group-shape vertically aligned -1.35 cm (above) the top page margin
    // (which is actually forced to layoutInCell, so that turns into 1.35cm above the paragraph)

    xmlDocUniquePtr pDump = parseLayoutDump();
    const sal_Int32 nShapeTop
        = getXPath(pDump,
                   "//body/tab[1]/row[1]/cell[1]/txt[1]/anchored/SwAnchoredDrawObject/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    // The shape is approximately 1 cm below the top of the page, and ~0.5cm above the cell
    // correct ShapeTop: 888 TWIPS, while incorrect value was -480. Cell top is 1148, PageTop is 284
    CPPUNIT_ASSERT_DOUBLES_EQUAL(888, nShapeTop, 50);
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

    // pre-emptive test: rectangle "margin" offset against cell, not outside-table-paragraph.
    // Since layoutInCell is true (as a non-defined default), the cell is the fly reference, thus
    // the rectangle should start at the paper's edge, 1.3cm to the left of the start of the table.
    sal_Int32 nRectLeft
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nTableLeft = getXPath(pDump, "//tab/row/infos/bounds"_ostr, "left"_ostr).toInt32();
    CPPUNIT_ASSERT(nTableLeft > nRectLeft);
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

CPPUNIT_TEST_FIXTURE(Test, testTdf160814_commentOrder)
{
    // given a document with a comment and 5 replies
    loadAndSave("tdf160814_commentOrder.docx");

    // make sure the order of the comments is imported and exported correctly
    xmlDocUniquePtr pXmlComments = parseExport("word/comments.xml");
    // This really should be "1. First comment", the 1. being list numbering...
    assertXPathContent(pXmlComments, "//w:comment[1]//w:t"_ostr, "First comment");
    assertXPathContent(pXmlComments, "//w:comment[2]//w:t"_ostr, "1.1 first reply.");
    assertXPathContent(pXmlComments, "//w:comment[4]//w:t"_ostr, "1.3");
    assertXPathContent(pXmlComments, "//w:comment[6]//w:t"_ostr, "1.5");
}

CPPUNIT_TEST_FIXTURE(Test, testPersonalMetaData)
{
    // 1. Remove all personal info
    auto pBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    xmlDocUniquePtr pAppDoc = parseExport("docProps/app.xml");
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template"_ostr, 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime"_ostr, 0);
    xmlDocUniquePtr pCoreDoc = parseExport("docProps/core.xml");
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision"_ostr, 0);

    // 2. Remove personal information, keep user information
    officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    pAppDoc = parseExport("docProps/app.xml");
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template"_ostr, 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime"_ostr, 0);
    pCoreDoc = parseExport("docProps/core.xml");
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision"_ostr, 0);
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

DECLARE_OOXMLEXPORT_TEST(testTdf131098_imageFill, "tdf131098_imageFill.docx")
{
    // given a document with an image background transparency (blue-white)
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(getShape(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), getProperty<Color>(getShape(1), "FillColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf154369, "tdf154369.docx")
{
    //Unit test for bug fix in tdf#154369
    // Docx file contains ordered list formatted with Heading 1 style, font color set as Accent 1 from theme
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the fix in place, this test would have failed with:
    // - Expected result: A & B bullets display same green color #527d55 as the paragraph
    // - Actual result: A & B bullets display black color, while the paragraph is green color #527d55
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                "expand"_ostr, "A.");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
                "color"_ostr, "00527d55");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                "expand"_ostr, "B.");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
                "color"_ostr, "00527d55");
}

CPPUNIT_TEST_FIXTURE(Test, testScreenTip)
{
    loadAndSave("tdf159897.docx");

    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");

    // Hyperlink with ScreenTip
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:hyperlink"_ostr, "tooltip"_ostr,
                "This is a hyperlink");
}

CPPUNIT_TEST_FIXTURE(Test, testEmptyObjectRange)
{
    // Before the fix, this failed an assertion like this:
    // Assertion failed: isBetween(n, (SAL_MIN_INT64 + d / 2) / m, (SAL_MAX_INT64 - d / 2) / m),
    // file C:\lo\core\include\o3tl/unit_conversion.hxx, line 75
    loadAndSave("cloud.docx");
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
