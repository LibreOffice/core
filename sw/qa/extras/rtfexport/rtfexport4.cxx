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

#include <svx/swframetypes.hxx>

#include <doc.hxx>
#include <unotxdoc.hxx>
#include <pam.hxx>
#include <fmtanchr.hxx>

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
