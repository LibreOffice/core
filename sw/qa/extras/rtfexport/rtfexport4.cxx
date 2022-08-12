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
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <o3tl/cppunittraitshelper.hxx>

#include <xmloff/odffields.hxx>

#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <pam.hxx>
#include <fmtanchr.hxx>
#include <frameformats.hxx>
#include <formatlinebreak.hxx>
#include <wrtsh.hxx>

using namespace css;

/**
  Split these tests into their own file because they are really really slow
*/

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testCjklist12, "cjklist12.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::AIU_HALFWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist13, "cjklist13.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::IROHA_HALFWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist16, "cjklist16.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_TRADITIONAL_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist20, "cjklist20.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::AIU_FULLWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist21, "cjklist21.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::IROHA_FULLWIDTH_JA, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist24, "cjklist24.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::HANGUL_SYLLABLE_KO, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist25, "cjklist25.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::HANGUL_JAMO_KO, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist30, "cjklist30.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_RTFEXPORT_TEST(testCjklist31, "cjklist31.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_RTFEXPORT_TEST(test148518, "FORMDROPDOWN.rtf")
{
    SwXTextDocument* const pTextDoc(dynamic_cast<SwXTextDocument*>(mxComponent.get()));
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* const pDoc(pTextDoc->GetDocShell()->GetDoc());

    CPPUNIT_ASSERT(pDoc->getIDocumentMarkAccess()->getFieldmarksBegin()
                   != pDoc->getIDocumentMarkAccess()->getFieldmarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(
        *pDoc->getIDocumentMarkAccess()->getFieldmarksBegin());
    uno::Sequence<OUString> entries;
    (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] >>= entries;
    uno::Sequence<OUString> const expected{ OUString("x"), OUString("v"), OUString("d") };
    CPPUNIT_ASSERT_EQUAL(expected, entries);
    sal_Int32 result(-1);
    (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_RESULT] >>= result;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), result);
}

DECLARE_RTFEXPORT_TEST(test150269, "hidden-linebreaks.rtf")
{
    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1, u"\n\n\n");
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xRun, "CharHidden"));
}

DECLARE_RTFEXPORT_TEST(test129758, "tdf129631_lostBorders3.rtf")
{
    uno::Reference<container::XNameAccess> xStyles(getStyles("ParagraphStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName("Border"), uno::UNO_QUERY);
    // style has borders
    table::BorderLine2 border;
    border = getProperty<table::BorderLine2>(xStyle, "RightBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    border = getProperty<table::BorderLine2>(xStyle, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    border = getProperty<table::BorderLine2>(xStyle, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    border = getProperty<table::BorderLine2>(xStyle, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    // style applied
    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Border"), getProperty<OUString>(xPara2, "ParaStyleName"));
    // but no borders
    border = getProperty<table::BorderLine2>(xPara2, "RightBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara2, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara2, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara2, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    // last paragraph: style applied, no override
    uno::Reference<beans::XPropertySet> xPara4(getParagraph(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Border"), getProperty<OUString>(xPara4, "ParaStyleName"));
    border = getProperty<table::BorderLine2>(xPara4, "RightBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara4, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara4, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara4, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
}

DECLARE_RTFEXPORT_TEST(test150382, "para-border.rtf")
{
    uno::Reference<container::XNameAccess> xStyles(getStyles("ParagraphStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName("Normal,Bordered"),
                                               uno::UNO_QUERY);
    // style has borders
    table::BorderLine2 border;
    border = getProperty<table::BorderLine2>(xStyle, "RightBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    border = getProperty<table::BorderLine2>(xStyle, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    border = getProperty<table::BorderLine2>(xStyle, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    border = getProperty<table::BorderLine2>(xStyle, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    // first paragraph: style applied, no override
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Normal,Bordered"),
                         getProperty<OUString>(xPara1, "ParaStyleName"));
    border = getProperty<table::BorderLine2>(xPara1, "RightBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    border = getProperty<table::BorderLine2>(xPara1, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    border = getProperty<table::BorderLine2>(xPara1, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    border = getProperty<table::BorderLine2>(xPara1, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
    // second paragraph: style applied
    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Normal,Bordered"),
                         getProperty<OUString>(xPara2, "ParaStyleName"));
    // but no borders
    border = getProperty<table::BorderLine2>(xPara2, "RightBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara2, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara2, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    border = getProperty<table::BorderLine2>(xPara2, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
}

DECLARE_RTFEXPORT_TEST(testAnchoredAtSamePosition, "anchor.fodt")
{
    SwXTextDocument* const pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* const pDoc = pTextDoc->GetDocShell()->GetDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), getParagraph(1)->getString());

    SwFrameFormats& rFlys(*pDoc->GetSpzFrameFormats());
    if (mbExported)
    { // 2, not 3: the form control becomes a field on export...
        CPPUNIT_ASSERT_EQUAL(size_t(2), rFlys.size());
    }
    else
    {
        CPPUNIT_ASSERT_EQUAL(size_t(3), rFlys.size());
    }

    sal_Int32 const nIndex(mbExported ? 4 : 3);
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_CHAR, rFlys[0]->GetAnchor().GetAnchorId());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12),
                         rFlys[0]->GetAnchor().GetContentAnchor()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL(nIndex, rFlys[0]->GetAnchor().GetContentAnchor()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_CHAR, rFlys[1]->GetAnchor().GetAnchorId());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12),
                         rFlys[1]->GetAnchor().GetContentAnchor()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL(nIndex, rFlys[1]->GetAnchor().GetContentAnchor()->nContent.GetIndex());
}

DECLARE_RTFEXPORT_TEST(testRedlineInsdel, "redline-insdel.rtf")
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);

    uno::Reference<text::XTextRange> xRun = getRun(xParagraph, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), xRun->getString());

    xRun = getRun(xParagraph, 2);
    CPPUNIT_ASSERT_EQUAL(OUString("Redline"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(xRun, "RedlineType"));
    xRun = getRun(xParagraph, 3);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), xRun->getString());
    xRun = getRun(xParagraph, 4);
    CPPUNIT_ASSERT_EQUAL(OUString("Redline"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"), getProperty<OUString>(xRun, "RedlineType"));

    xRun = getRun(xParagraph, 5);
    CPPUNIT_ASSERT_EQUAL(OUString("Redline"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"), getProperty<OUString>(xRun, "RedlineType"));
    xRun = getRun(xParagraph, 6);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("BBB"), xRun->getString());
    xRun = getRun(xParagraph, 7);
    CPPUNIT_ASSERT_EQUAL(OUString("Redline"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"), getProperty<OUString>(xRun, "RedlineType"));

    xRun = getRun(xParagraph, 8);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), getProperty<OUString>(xRun, "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), xRun->getString());
}

DECLARE_RTFEXPORT_TEST(testParaAdjustDistribute, "para-adjust-distribute.rtf")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was
    // left, not block.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(1), "ParaLastLineAdjust")));

    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(2), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(2), "ParaLastLineAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf129574, "mw00_table_of_contents_templates.doc")
{
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes(xIndexSupplier->getDocumentIndexes());
    uno::Reference<text::XDocumentIndex> xTOC(xIndexes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTOC.is());
    uno::Reference<text::XTextRange> xTextRange(xTOC->getAnchor());
    // table of contents contains 4 paragraphs
    CPPUNIT_ASSERT_EQUAL(OUString("1.Koffice 1" SAL_NEWLINE_STRING "2.Kword 1" SAL_NEWLINE_STRING
                                  "3.Kspread 1" SAL_NEWLINE_STRING "4.Kpresenter 1"),
                         xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testCjklist34, "cjklist34.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

CPPUNIT_TEST_FIXTURE(Test, testTabStopFillChars)
{
    load(mpTestDocumentPath, "tab-stop-fill-chars.rtf");
    // tlmdot
    auto aTabstops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops");
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u'·', aTabstops[0].FillChar);

    // tlhyph
    aTabstops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(2), "ParaTabStops");
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u'-', aTabstops[0].FillChar);

    // tlth
    aTabstops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(3), "ParaTabStops");
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u'_', aTabstops[0].FillChar);

    // tleq
    aTabstops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(4), "ParaTabStops");
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u' ', aTabstops[0].FillChar);
}

DECLARE_RTFEXPORT_TEST(testTdf146489, "tdf146489.rtf")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(2, "unnumbered");
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, "ListLabelString"));
}

DECLARE_RTFEXPORT_TEST(testCjklist38, "cjklist38.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH, numFormat);
}

DECLARE_RTFEXPORT_TEST(testBtlrCell, "btlr-cell.rtf")
{
    // Without the accompanying fix in place, this test would have failed, as
    // the btlr text direction in the A1 cell was lost.
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName("Table1"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xA1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, getProperty<sal_Int16>(xA1, "WritingMode"));

    uno::Reference<beans::XPropertySet> xB1(xTable->getCellByName("B1"), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xB1, "WritingMode");
    CPPUNIT_ASSERT(nActual == text::WritingMode2::LR_TB || nActual == text::WritingMode2::CONTEXT);

    uno::Reference<beans::XPropertySet> xC1(xTable->getCellByName("C1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, getProperty<sal_Int16>(xC1, "WritingMode"));
}

DECLARE_RTFEXPORT_TEST(testTdf114303, "tdf114303.rtf")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE,
                         getProperty<sal_Int16>(getShape(1), "HoriOrient"));

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::NONE,
                         getProperty<sal_Int16>(getShape(1), "VertOrient"));
}

DECLARE_RTFEXPORT_TEST(testTbrlFrame, "tbrl-frame.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xShape, "WritingMode");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. custom writing mode was lost.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, nActual);
}

DECLARE_RTFEXPORT_TEST(testBtlrFrame, "btlr-frame.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xShape, "WritingMode");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 0
    // i.e. custom writing mode was lost.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, nActual);
}

DECLARE_RTFEXPORT_TEST(testTdf129578_lostBI, "tdf129578_lostBI.rtf")
{
    CPPUNIT_ASSERT_EQUAL(150.f, getProperty<float>(getRun(getParagraph(1), 1), "CharWeightAsian"));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontSlant_ITALIC,
        getProperty<awt::FontSlant>(getRun(getParagraph(2), 1), "CharPostureAsian"));
}

DECLARE_RTFEXPORT_TEST(testTdf129631_lostBorders, "tdf129631_lostBorders.rtf")
{
    uno::Reference<container::XNameAccess> paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName("Border"),
                                                    uno::UNO_QUERY_THROW);
    table::BorderLine2 aBorderLine = getProperty<table::BorderLine2>(xStyleProps, "RightBorder");
    CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "The border style has normal black borders", COL_BLACK,
        Color(ColorTransparency, getProperty<table::BorderLine>(xStyleProps, "RightBorder").Color));

    aBorderLine = getProperty<table::BorderLine2>(getParagraph(2), "RightBorder");
    CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "The second paragraph should have dark green borders", sal_Int32(432686),
        getProperty<table::BorderLine>(getParagraph(2), "RightBorder").Color);
}

DECLARE_RTFEXPORT_TEST(testTdf129522_removeShadowStyle, "tdf129522_removeShadowStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XNameAccess> paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName("Shadow"),
                                                    uno::UNO_QUERY_THROW);
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    // Shadows were inherited regardless of whether the style disabled them.
    xStyleProps.set(paragraphStyles->getByName("Shadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    uno::Reference<container::XNameAccess> characterStyles = getStyles("CharacterStyles");
    xStyleProps.set(characterStyles->getByName("CharShadow"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    xStyleProps.set(characterStyles->getByName("CharShadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
    //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 2, "style");
    aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
    //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    xRun.set(getRun(getParagraph(1), 4, "shadow"));
    aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    table::BorderLine2 aBorderLine = getProperty<table::BorderLine2>(xRun, "CharRightBorder");
    // MS formats can't have a shadow without a border.
    // Char borders are all or none, so have to decide to add borders, or throw away shadow...
    if (mbExported)
        CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);

    xRun.set(getRun(getParagraph(4), 2, "shadow"));
    aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
    //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    xRun.set(getRun(getParagraph(9), 2, "End of test"));
    aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
    aBorderLine = getProperty<table::BorderLine2>(xRun, "CharRightBorder");
    CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);
}

DECLARE_RTFEXPORT_TEST(testTdf136587_noStyleName, "tdf136587_noStyleName.rtf")
{
    // An exception stopped all style definitions.
    uno::Reference<container::XNameAccess> paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName("Style15"),
                                                    uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                         getProperty<sal_Int16>(xStyleProps, "ParaAdjust"));

    // The problem was that the default style wasn't imported at all, so the fontsize was only 12.
    xStyleProps.set(paragraphStyles->getByName("Default Paragraph Style"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(32.0f, getProperty<float>(xStyleProps, "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testPageBorder)
{
    load(mpTestDocumentPath, "page-border.rtf");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    auto aTopBorder = getProperty<table::BorderLine2>(xPageStyle, "TopBorder");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(18), aTopBorder.LineWidth);

    auto aLeftBorder = getProperty<table::BorderLine2>(xPageStyle, "LeftBorder");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(35), aLeftBorder.LineWidth);

    auto aBottomBorder = getProperty<table::BorderLine2>(xPageStyle, "BottomBorder");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(53), aBottomBorder.LineWidth);

    auto aRightBorder = getProperty<table::BorderLine2>(xPageStyle, "RightBorder");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(71), aRightBorder.LineWidth);
}

DECLARE_RTFEXPORT_TEST(testTbrlPage, "tbrl-page.rtf")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xPageStyle, "WritingMode");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. custom writing mode was lost.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, nActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126309)
{
    load(mpTestDocumentPath, "tdf126309.rtf");
    // Without the accompanying fix in place, this test would have failed, as
    // the paragraph was aligned to left, not right.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_RIGHT,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf116358, "tdf116358.rtf")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Insert a paragraph break
    pWrtShell->SplitNode(false);

    // Entire table should go to page 2, no remains on first page
    xmlDocUniquePtr pDump = parseLayoutDump();
    {
        xmlXPathObjectPtr pXmlObj = getXPathNode(pDump, "/root/page[1]/body/tab");
        xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
        sal_Int32 tablesOnPage = xmlXPathNodeSetGetLength(pXmlNodes);
        xmlXPathFreeObject(pXmlObj);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), tablesOnPage);
    }

    {
        xmlXPathObjectPtr pXmlObj = getXPathNode(pDump, "/root/page[2]/body/tab");
        xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
        sal_Int32 tablesOnPage = xmlXPathNodeSetGetLength(pXmlNodes);
        xmlXPathFreeObject(pXmlObj);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), tablesOnPage);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testGutterLeft)
{
    load(mpTestDocumentPath, "gutter-left.rtf");
    reload(mpFilter, "gutter-left.rtf");
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles("PageStyles")->getByName("Standard") >>= xPageStyle;
    sal_Int32 nGutterMargin{};
    xPageStyle->getPropertyValue("GutterMargin") >>= nGutterMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1270
    // - Actual  : 0
    // i.e. gutter margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nGutterMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testGutterTop)
{
    load(mpTestDocumentPath, "gutter-top.rtf");
    reload(mpFilter, "gutter-left.rtf");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    bool bGutterAtTop{};
    xSettings->getPropertyValue("GutterAtTop") >>= bGutterAtTop;
    // Without the accompanying fix in place, this test would have failed, because the gutter was
    // at the left.
    CPPUNIT_ASSERT(bGutterAtTop);
}

CPPUNIT_TEST_FIXTURE(Test, testClearingBreak)
{
    auto verify = [this]() {
        uno::Reference<container::XEnumerationAccess> xParagraph(getParagraph(1), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        xPortions->nextElement();
        xPortions->nextElement();
        // Without the accompanying fix in place, this test would have failed with:
        // An uncaught exception of type com.sun.star.container.NoSuchElementException
        // i.e. the first para was just a fly + text portion, the clearing break was lost.
        uno::Reference<beans::XPropertySet> xPortion(xPortions->nextElement(), uno::UNO_QUERY);
        OUString aPortionType;
        xPortion->getPropertyValue("TextPortionType") >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(OUString("LineBreak"), aPortionType);
        uno::Reference<text::XTextContent> xLineBreak;
        xPortion->getPropertyValue("LineBreak") >>= xLineBreak;
        sal_Int16 eClear{};
        uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
        xLineBreakProps->getPropertyValue("Clear") >>= eClear;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
    };

    // Given a document with a clearing break:
    // When loading that file:
    load(mpTestDocumentPath, "clearing-break.rtf");
    // Then make sure that the clear property of the break is not ignored:
    verify();
    reload(mpFilter, "clearing-break.rtf");
    // Make sure that the clear property of the break is not ignored during export:
    verify();
}

DECLARE_RTFEXPORT_TEST(testTdf95706, "tdf95706.rtf")
{
    uno::Reference<text::XTextRange> xRun2
        = getRun(getParagraph(2), 1, u"\u0104\u012e\u0100\u0106\u00c4");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun2, "CharFontName"));

    uno::Reference<text::XTextRange> xRun3
        = getRun(getParagraph(3), 1, u"\u0154\u00c1\u00c2\u0102\u00c4");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun3, "CharFontName"));

    uno::Reference<text::XTextRange> xRun4
        = getRun(getParagraph(4), 1, u"\u0410\u0411\u0412\u0413\u0414");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun4, "CharFontName"));

    uno::Reference<text::XTextRange> xRun5
        = getRun(getParagraph(5), 1, u"\u0390\u0391\u0392\u0393\u0394");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun5, "CharFontName"));

    uno::Reference<text::XTextRange> xRun6
        = getRun(getParagraph(6), 1, u"\u00c0\u00c1\u00c2\u00c3\u00c4");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun6, "CharFontName"));

    uno::Reference<text::XTextRange> xRun7
        = getRun(getParagraph(7), 1, u"\u05b0\u05b1\u05b2\u05b3\u05b4");
    // Do not check font for Hebrew: it can be substituted by smth able to handle these chars
    //CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun7, "CharFontName"));

    uno::Reference<text::XTextRange> xRun8
        = getRun(getParagraph(8), 1, u"\u06c1\u0621\u0622\u0623\u0624");
    // Do not check font for Arabic: it can be substituted by smth able to handle these chars
    //CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun8, "CharFontName"));

    uno::Reference<text::XTextRange> xRun9
        = getRun(getParagraph(9), 1, u"\u00c0\u00c1\u00c2\u0102\u00c4");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun9, "CharFontName"));

    // Ensure strange font remains strange. No reason to check content: in this case it can vary on locale
    uno::Reference<text::XTextRange> xRun10 = getRun(getParagraph(10), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial BlaBlaBla"),
                         getProperty<OUString>(xRun10, "CharFontName"));

    uno::Reference<text::XTextRange> xRun12
        = getRun(getParagraph(12), 1, u"\u0390\u0391\u0392\u0393\u0394");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun12, "CharFontName"));

    uno::Reference<text::XTextRange> xRun13
        = getRun(getParagraph(13), 1, u"\u0390\u0391\u0392\u0393\u0394");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun13, "CharFontName"));

    uno::Reference<text::XTextRange> xRun14 = getRun(getParagraph(14), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun14, "CharFontName"));

    uno::Reference<text::XTextRange> xRun15
        = getRun(getParagraph(15), 1, u"\u0104\u012e\u0100\u0106\u00c4");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun15, "CharFontName"));

    uno::Reference<text::XTextRange> xRun16
        = getRun(getParagraph(16), 1, u"\u0104\u012e\u0100\u0106\u00c4");
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun16, "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testTdf95706_2, "tdf95706_2.rtf")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Impact"),
                         getProperty<OUString>(getRun(getParagraph(2), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"),
                         getProperty<OUString>(getRun(getParagraph(3), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Tahoma"),
                         getProperty<OUString>(getRun(getParagraph(4), 1), "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testTdf111851, "tdf111851.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);

    // No shading
    uno::Reference<text::XTextRange> xCell1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xCell1->getString());
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell1, "BackColor"));

    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName("B1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xCell2->getString());
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell2, "BackColor"));

    // Check some random not standard shading values and ensure some non-white background color
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName("C1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("c"), xCell3->getString());
    Color nShadingColor3 = getProperty<Color>(xCell3, "BackColor");
    CPPUNIT_ASSERT(COL_WHITE > nShadingColor3);
    CPPUNIT_ASSERT(COL_BLACK < nShadingColor3);

    uno::Reference<text::XTextRange> xCell4(xTable->getCellByName("D1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("d"), xCell4->getString());
    Color nShadingColor4 = getProperty<Color>(xCell4, "BackColor");
    CPPUNIT_ASSERT(COL_WHITE > nShadingColor4);
    CPPUNIT_ASSERT(COL_BLACK < nShadingColor4);

    // Values 10000 and more - black
    uno::Reference<text::XTextRange> xCell5(xTable->getCellByName("E1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("e"), xCell5->getString());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xCell5, "BackColor"));

    uno::Reference<text::XTextRange> xCell6(xTable->getCellByName("F1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("f"), xCell6->getString());
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xCell6, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf139948, "tdf139948.rtf")
{
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(1, "No border"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(2, "Border below"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(88),
        getProperty<table::BorderLine2>(getParagraph(3, "Borders below and above"), "TopBorder")
            .LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(88),
        getProperty<table::BorderLine2>(getParagraph(4, "Border above"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(5, "No border"), "TopBorder").LineWidth);

    // And let's ensure that there are no other horizontal borders
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0), getProperty<table::BorderLine2>(getParagraph(1), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0), getProperty<table::BorderLine2>(getParagraph(2), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0), getProperty<table::BorderLine2>(getParagraph(3), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0), getProperty<table::BorderLine2>(getParagraph(4), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(0), getProperty<table::BorderLine2>(getParagraph(5), "BottomBorder").LineWidth);
}

DECLARE_RTFEXPORT_TEST(testTdf148515, "tdf148515.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xCell1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("XXXXXX"), xCell1->getString());
    CPPUNIT_ASSERT_EQUAL(
        5.0f,
        getProperty<float>(getRun(getParagraphOfText(1, xCell1->getText()), 1), "CharHeight"));

    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName("B1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), xCell2->getString());
    CPPUNIT_ASSERT_EQUAL(
        5.0f,
        getProperty<float>(getRun(getParagraphOfText(1, xCell2->getText()), 1), "CharHeight"));

    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(getRun(getParagraph(2), 1), "CharHeight"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
