/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <o3tl/cppunittraitshelper.hxx>
#include <svx/swframetypes.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <pam.hxx>
#include <fmtanchr.hxx>
#include <frameformats.hxx>

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
    CPPUNIT_ASSERT_EQUAL(sal_uLong(12), rFlys[0]->GetAnchor().GetContentAnchor()->nNode.GetIndex());
    CPPUNIT_ASSERT_EQUAL(nIndex, rFlys[0]->GetAnchor().GetContentAnchor()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_CHAR, rFlys[1]->GetAnchor().GetAnchorId());
    CPPUNIT_ASSERT_EQUAL(sal_uLong(12), rFlys[1]->GetAnchor().GetContentAnchor()->nNode.GetIndex());
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

DECLARE_RTFEXPORT_TEST(testTdf129631_lostBorders, "tdf129631_lostBorders.rtf")
{
    uno::Reference<container::XNameAccess> paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName("Border"),
                                                    uno::UNO_QUERY_THROW);
    table::BorderLine2 aBorderLine = getProperty<table::BorderLine2>(xStyleProps, "RightBorder");
    CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "The border style has normal black borders", COL_BLACK,
        Color(getProperty<table::BorderLine>(xStyleProps, "RightBorder").Color));

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
    uno::Reference<container::XNameAccess> paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName("Style15"),
                                                    uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                         getProperty<sal_Int16>(xStyleProps, "ParaAdjust"));

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
