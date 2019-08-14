/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

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

DECLARE_RTFEXPORT_TEST(testCjklist34, "cjklist34.rtf")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_RTFIMPORT_TEST(testTabStopFillChars, "tab-stop-fill-chars.rtf")
{
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
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xShape, "WritingMode");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 0
    // i.e. custom writing mode was lost.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, nActual);
}

DECLARE_RTFIMPORT_TEST(testPageBorder, "page-border.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testTdf126309, "tdf126309.rtf")
{
    // Without the accompanying fix in place, this test would have failed, as
    // the paragraph was aligned to left, not right.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_RIGHT,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
