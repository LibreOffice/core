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

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/rtfexport/data/"_ustr, u"Rich Text Format"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testCjklist12)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::AIU_HALFWIDTH_JA, numFormat);
    };
    createSwDoc("cjklist12.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist13)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::IROHA_HALFWIDTH_JA, numFormat);
    };
    createSwDoc("cjklist13.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist16)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_TRADITIONAL_JA, numFormat);
    };
    createSwDoc("cjklist16.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist20)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::AIU_FULLWIDTH_JA, numFormat);
    };
    createSwDoc("cjklist20.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist21)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::IROHA_FULLWIDTH_JA, numFormat);
    };
    createSwDoc("cjklist21.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist24)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::HANGUL_SYLLABLE_KO, numFormat);
    };
    createSwDoc("cjklist24.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist25)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::HANGUL_JAMO_KO, numFormat);
    };
    createSwDoc("cjklist25.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist30)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
    };
    createSwDoc("cjklist30.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist31)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
    };
    createSwDoc("cjklist31.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, test148518)
{
    auto verify = [this]() {
        SwDoc* const pDoc(getSwDoc());

        CPPUNIT_ASSERT(pDoc->getIDocumentMarkAccess()->getFieldmarksBegin()
                       != pDoc->getIDocumentMarkAccess()->getFieldmarksEnd());
        ::sw::mark::Fieldmark* pFieldmark = *pDoc->getIDocumentMarkAccess()->getFieldmarksBegin();
        uno::Sequence<OUString> entries;
        (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_LISTENTRY] >>= entries;
        uno::Sequence<OUString> const expected{ u"x"_ustr, u"v"_ustr, u"d"_ustr };
        CPPUNIT_ASSERT_EQUAL(expected, entries);
        sal_Int32 result(-1);
        (*pFieldmark->GetParameters())[ODF_FORMDROPDOWN_RESULT] >>= result;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), result);
    };
    createSwDoc("FORMDROPDOWN.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, test150269)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1, u"\n\n\n"_ustr);
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xRun, u"CharHidden"_ustr));
    };
    createSwDoc("hidden-linebreaks.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, test129758)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> xStyles(getStyles(u"ParagraphStyles"_ustr));
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(u"Border"_ustr),
                                                   uno::UNO_QUERY);
        // style has borders
        table::BorderLine2 border;
        border = getProperty<table::BorderLine2>(xStyle, u"RightBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
        border = getProperty<table::BorderLine2>(xStyle, u"LeftBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
        border = getProperty<table::BorderLine2>(xStyle, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
        border = getProperty<table::BorderLine2>(xStyle, u"BottomBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
        // style applied
        uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Border"_ustr, getProperty<OUString>(xPara2, u"ParaStyleName"_ustr));
        // but no borders
        border = getProperty<table::BorderLine2>(xPara2, u"RightBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara2, u"LeftBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara2, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara2, u"BottomBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
        // last paragraph: style applied, no override
        uno::Reference<beans::XPropertySet> xPara4(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Border"_ustr, getProperty<OUString>(xPara4, u"ParaStyleName"_ustr));
        border = getProperty<table::BorderLine2>(xPara4, u"RightBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara4, u"LeftBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara4, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara4, u"BottomBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88), border.LineWidth);
    };
    createSwDoc("tdf129631_lostBorders3.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, test150382)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> xStyles(getStyles(u"ParagraphStyles"_ustr));
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(u"Normal,Bordered"_ustr),
                                                   uno::UNO_QUERY);
        // style has borders
        table::BorderLine2 border;
        border = getProperty<table::BorderLine2>(xStyle, u"RightBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        border = getProperty<table::BorderLine2>(xStyle, u"LeftBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        border = getProperty<table::BorderLine2>(xStyle, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        border = getProperty<table::BorderLine2>(xStyle, u"BottomBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        // first paragraph: style applied, no override
        uno::Reference<beans::XPropertySet> xPara1(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Normal,Bordered"_ustr,
                             getProperty<OUString>(xPara1, u"ParaStyleName"_ustr));
        border = getProperty<table::BorderLine2>(xPara1, u"RightBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        border = getProperty<table::BorderLine2>(xPara1, u"LeftBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        border = getProperty<table::BorderLine2>(xPara1, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        border = getProperty<table::BorderLine2>(xPara1, u"BottomBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::SOLID, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(35), border.LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7384391), border.Color);
        // second paragraph: style applied
        uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Normal,Bordered"_ustr,
                             getProperty<OUString>(xPara2, u"ParaStyleName"_ustr));
        // but no borders
        border = getProperty<table::BorderLine2>(xPara2, u"RightBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara2, u"LeftBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara2, u"TopBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
        border = getProperty<table::BorderLine2>(xPara2, u"BottomBorder"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, border.LineStyle);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), border.LineWidth);
    };
    createSwDoc("para-border.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testAnchoredAtSamePosition)
{
    auto verify = [this](bool bIsExport = false) {
        SwDoc* const pDoc = getSwDoc();

        CPPUNIT_ASSERT_EQUAL(u"foobar"_ustr, getParagraph(1)->getString());

        auto& rFlys = *pDoc->GetSpzFrameFormats();
        if (bIsExport)
        { // 2, not 3: the form control becomes a field on export...
            CPPUNIT_ASSERT_EQUAL(size_t(2), rFlys.size());
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(size_t(3), rFlys.size());
        }

        sal_Int32 const nIndex(bIsExport ? 4 : 3);
        CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_CHAR, rFlys[0]->GetAnchor().GetAnchorId());
        CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), rFlys[0]->GetAnchor().GetAnchorNode()->GetIndex());
        CPPUNIT_ASSERT_EQUAL(nIndex, rFlys[0]->GetAnchor().GetAnchorContentOffset());
        CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_CHAR, rFlys[1]->GetAnchor().GetAnchorId());
        CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), rFlys[1]->GetAnchor().GetAnchorNode()->GetIndex());
        CPPUNIT_ASSERT_EQUAL(nIndex, rFlys[1]->GetAnchor().GetAnchorContentOffset());
    };
    createSwDoc("anchor.fodt");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineInsdel)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xParagraph = getParagraph(1);

        uno::Reference<text::XTextRange> xRun = getRun(xParagraph, 1);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"aaa"_ustr, xRun->getString());

        xRun = getRun(xParagraph, 2);
        CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Delete"_ustr, getProperty<OUString>(xRun, u"RedlineType"_ustr));
        xRun = getRun(xParagraph, 3);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, xRun->getString());
        xRun = getRun(xParagraph, 4);
        CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Delete"_ustr, getProperty<OUString>(xRun, u"RedlineType"_ustr));

        xRun = getRun(xParagraph, 5);
        CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr, getProperty<OUString>(xRun, u"RedlineType"_ustr));
        xRun = getRun(xParagraph, 6);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"BBB"_ustr, xRun->getString());
        xRun = getRun(xParagraph, 7);
        CPPUNIT_ASSERT_EQUAL(u"Redline"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr, getProperty<OUString>(xRun, u"RedlineType"_ustr));

        xRun = getRun(xParagraph, 8);
        CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, xRun->getString());
    };
    createSwDoc("redline-insdel.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testParaAdjustDistribute)
{
    auto verify = [this]() {
        // Without the accompanying fix in place, this test would have failed with
        // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was
        // left, not block.
        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                             static_cast<style::ParagraphAdjust>(
                                 getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)));
        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                             static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(
                                 getParagraph(1), u"ParaLastLineAdjust"_ustr)));

        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                             static_cast<style::ParagraphAdjust>(
                                 getProperty<sal_Int16>(getParagraph(2), u"ParaAdjust"_ustr)));
        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                             static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(
                                 getParagraph(2), u"ParaLastLineAdjust"_ustr)));
    };
    createSwDoc("para-adjust-distribute.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129574)
{
    auto verify = [this]() {
        uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexes(xIndexSupplier->getDocumentIndexes());
        uno::Reference<text::XDocumentIndex> xTOC(xIndexes->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xTOC.is());
        uno::Reference<text::XTextRange> xTextRange(xTOC->getAnchor());
        // table of contents contains 4 paragraphs
        CPPUNIT_ASSERT_EQUAL(u"1.Koffice 1" SAL_NEWLINE_STRING "2.Kword 1" SAL_NEWLINE_STRING
                             "3.Kspread 1" SAL_NEWLINE_STRING "4.Kpresenter 1"_ustr,
                             xTextRange->getString());
    };
    createSwDoc("mw00_table_of_contents_templates.doc");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist34)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
    };
    createSwDoc("cjklist34.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTabStopFillChars)
{
    createSwDoc("tab-stop-fill-chars.rtf");
    // tlmdot
    auto aTabstops
        = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), u"ParaTabStops"_ustr);
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u'·', aTabstops[0].FillChar);

    // tlhyph
    aTabstops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(2), u"ParaTabStops"_ustr);
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u'-', aTabstops[0].FillChar);

    // tlth
    aTabstops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(3), u"ParaTabStops"_ustr);
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u'_', aTabstops[0].FillChar);

    // tleq
    aTabstops = getProperty<uno::Sequence<style::TabStop>>(getParagraph(4), u"ParaTabStops"_ustr);
    CPPUNIT_ASSERT(aTabstops.hasElements());
    CPPUNIT_ASSERT_EQUAL(u' ', aTabstops[0].FillChar);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146489)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xPara = getParagraph(2, u"unnumbered"_ustr);
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    };
    createSwDoc("tdf146489.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCjklist38)
{
    auto verify = [this]() {
        sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
        CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH, numFormat);
    };
    createSwDoc("cjklist38.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testBtlrCell)
{
    auto verify = [this]() {
        // Without the accompanying fix in place, this test would have failed, as
        // the btlr text direction in the A1 cell was lost.
        uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
        uno::Reference<text::XTextTable> xTable(xTables->getByName(u"Table1"_ustr), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xA1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR,
                             getProperty<sal_Int16>(xA1, u"WritingMode"_ustr));

        uno::Reference<beans::XPropertySet> xB1(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        auto nActual = getProperty<sal_Int16>(xB1, u"WritingMode"_ustr);
        CPPUNIT_ASSERT(nActual == text::WritingMode2::LR_TB
                       || nActual == text::WritingMode2::CONTEXT);

        uno::Reference<beans::XPropertySet> xC1(xTable->getCellByName(u"C1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL,
                             getProperty<sal_Int16>(xC1, u"WritingMode"_ustr));
    };
    createSwDoc("btlr-cell.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf114303)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE,
                             getProperty<sal_Int16>(getShape(1), u"HoriOrient"_ustr));

        // Without the fix in place, this test would have failed with
        // - Expected: 0
        // - Actual  : 1
        CPPUNIT_ASSERT_EQUAL(text::VertOrientation::NONE,
                             getProperty<sal_Int16>(getShape(1), u"VertOrient"_ustr));
    };
    createSwDoc("tdf114303.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTbrlFrame)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        auto nActual = getProperty<sal_Int16>(xShape, u"WritingMode"_ustr);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2
        // - Actual  : 0
        // i.e. custom writing mode was lost.
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, nActual);
    };
    createSwDoc("tbrl-frame.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testBtlrFrame)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        auto nActual = getProperty<sal_Int16>(xShape, u"WritingMode"_ustr);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 5
        // - Actual  : 0
        // i.e. custom writing mode was lost.
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, nActual);
    };
    createSwDoc("btlr-frame.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129578_lostBI)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(
            150.f, getProperty<float>(getRun(getParagraph(1), 1), u"CharWeightAsian"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            awt::FontSlant_ITALIC,
            getProperty<awt::FontSlant>(getRun(getParagraph(2), 1), u"CharPostureAsian"_ustr));
    };
    createSwDoc("tdf129578_lostBI.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129631_lostBorders)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> paragraphStyles = getStyles(u"ParagraphStyles"_ustr);
        uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName(u"Border"_ustr),
                                                        uno::UNO_QUERY_THROW);
        table::BorderLine2 aBorderLine
            = getProperty<table::BorderLine2>(xStyleProps, u"RightBorder"_ustr);
        CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "The border style has normal black borders", COL_BLACK,
            Color(ColorTransparency,
                  getProperty<table::BorderLine>(xStyleProps, u"RightBorder"_ustr).Color));

        aBorderLine = getProperty<table::BorderLine2>(getParagraph(2), u"RightBorder"_ustr);
        CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "The second paragraph should have dark green borders", sal_Int32(432686),
            getProperty<table::BorderLine>(getParagraph(2), u"RightBorder"_ustr).Color);
    };
    createSwDoc("tdf129631_lostBorders.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129522_removeShadowStyle)
{
    auto verify = [this](bool bIsExport = false) {
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        uno::Reference<container::XNameAccess> paragraphStyles = getStyles(u"ParagraphStyles"_ustr);
        uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName(u"Shadow"_ustr),
                                                        uno::UNO_QUERY_THROW);
        table::ShadowFormat aShadow
            = getProperty<table::ShadowFormat>(xStyleProps, u"ParaShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

        // Shadows were inherited regardless of whether the style disabled them.
        xStyleProps.set(paragraphStyles->getByName(u"Shadow-removed"_ustr), uno::UNO_QUERY_THROW);
        aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"ParaShadowFormat"_ustr);
        //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

        uno::Reference<container::XNameAccess> characterStyles = getStyles(u"CharacterStyles"_ustr);
        xStyleProps.set(characterStyles->getByName(u"CharShadow"_ustr), uno::UNO_QUERY_THROW);
        aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

        xStyleProps.set(characterStyles->getByName(u"CharShadow-removed"_ustr),
                        uno::UNO_QUERY_THROW);
        aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"CharShadowFormat"_ustr);
        //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

        uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 2, u"style"_ustr);
        aShadow = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
        //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

        xRun.set(getRun(getParagraph(1), 4, u"shadow"_ustr));
        aShadow = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        table::BorderLine2 aBorderLine
            = getProperty<table::BorderLine2>(xRun, u"CharRightBorder"_ustr);
        // MS formats can't have a shadow without a border.
        // Char borders are all or none, so have to decide to add borders, or throw away shadow...
        if (bIsExport)
            CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);

        xRun.set(getRun(getParagraph(4), 2, u"shadow"_ustr));
        aShadow = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
        //CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

        xRun.set(getRun(getParagraph(9), 2, u"End of test"_ustr));
        aShadow = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        aBorderLine = getProperty<table::BorderLine2>(xRun, u"CharRightBorder"_ustr);
        CPPUNIT_ASSERT(sal_uInt32(0) != aBorderLine.LineWidth);
    };
    createSwDoc("tdf129522_removeShadowStyle.odt");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136587_noStyleName)
{
    auto verify = [this]() {
        // An exception stopped all style definitions.
        uno::Reference<container::XNameAccess> paragraphStyles = getStyles(u"ParagraphStyles"_ustr);
        uno::Reference<beans::XPropertySet> xStyleProps(paragraphStyles->getByName(u"Style15"_ustr),
                                                        uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(style::ParagraphAdjust_CENTER),
                             getProperty<sal_Int16>(xStyleProps, u"ParaAdjust"_ustr));

        // The problem was that the default style wasn't imported at all, so the fontsize was only 12.
        xStyleProps.set(paragraphStyles->getByName(u"Standard"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(32.0f, getProperty<float>(xStyleProps, u"CharHeight"_ustr));
    };
    createSwDoc("tdf136587_noStyleName.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testPageBorder)
{
    createSwDoc("page-border.rtf");
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    auto aTopBorder = getProperty<table::BorderLine2>(xPageStyle, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(18), aTopBorder.LineWidth);

    auto aLeftBorder = getProperty<table::BorderLine2>(xPageStyle, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(35), aLeftBorder.LineWidth);

    auto aBottomBorder = getProperty<table::BorderLine2>(xPageStyle, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(53), aBottomBorder.LineWidth);

    auto aRightBorder = getProperty<table::BorderLine2>(xPageStyle, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(71), aRightBorder.LineWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTbrlPage)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        auto nActual = getProperty<sal_Int16>(xPageStyle, u"WritingMode"_ustr);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2
        // - Actual  : 0
        // i.e. custom writing mode was lost.
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, nActual);
    };
    createSwDoc("tbrl-page.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126309)
{
    createSwDoc("tdf126309.rtf");
    // Without the accompanying fix in place, this test would have failed, as
    // the paragraph was aligned to left, not right.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_RIGHT,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116358)
{
    auto verify = [this]() {
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        CPPUNIT_ASSERT(pWrtShell);

        // Insert a paragraph break
        pWrtShell->SplitNode(false);

        // Entire table should go to page 2, no remains on first page
        xmlDocUniquePtr pDump = parseLayoutDump();
        assertXPath(pDump, "/root/page[1]/body/tab", 0);
        assertXPath(pDump, "/root/page[2]/body/tab", 1);
    };
    createSwDoc("tdf116358.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testGutterLeft)
{
    createSwDoc("gutter-left.rtf");
    saveAndReload(u"Rich Text Format"_ustr);
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr) >>= xPageStyle;
    sal_Int32 nGutterMargin{};
    xPageStyle->getPropertyValue(u"GutterMargin"_ustr) >>= nGutterMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1270
    // - Actual  : 0
    // i.e. gutter margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nGutterMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testGutterTop)
{
    createSwDoc("gutter-top.rtf");
    saveAndReload(u"Rich Text Format"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    bool bGutterAtTop{};
    xSettings->getPropertyValue(u"GutterAtTop"_ustr) >>= bGutterAtTop;
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
        xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"LineBreak"_ustr, aPortionType);
        uno::Reference<text::XTextContent> xLineBreak;
        xPortion->getPropertyValue(u"LineBreak"_ustr) >>= xLineBreak;
        sal_Int16 eClear{};
        uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
        xLineBreakProps->getPropertyValue(u"Clear"_ustr) >>= eClear;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
    };

    // Given a document with a clearing break:
    // When loading that file:
    createSwDoc("clearing-break.rtf");
    // Then make sure that the clear property of the break is not ignored:
    verify();
    saveAndReload(u"Rich Text Format"_ustr);
    // Make sure that the clear property of the break is not ignored during export:
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95706)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xRun2
            = getRun(getParagraph(2), 1, u"\u0104\u012e\u0100\u0106\u00c4"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun2, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun3
            = getRun(getParagraph(3), 1, u"\u0154\u00c1\u00c2\u0102\u00c4"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun3, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun4
            = getRun(getParagraph(4), 1, u"\u0410\u0411\u0412\u0413\u0414"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun4, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun5
            = getRun(getParagraph(5), 1, u"\u0390\u0391\u0392\u0393\u0394"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun5, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun6
            = getRun(getParagraph(6), 1, u"\u00c0\u00c1\u00c2\u00c3\u00c4"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun6, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun7
            = getRun(getParagraph(7), 1, u"\u05b0\u05b1\u05b2\u05b3\u05b4"_ustr);
        // Do not check font for Hebrew: it can be substituted by smth able to handle these chars
        //CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun7, "CharFontName"));

        uno::Reference<text::XTextRange> xRun8
            = getRun(getParagraph(8), 1, u"\u06c1\u0621\u0622\u0623\u0624"_ustr);
        // Do not check font for Arabic: it can be substituted by smth able to handle these chars
        //CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun8, "CharFontName"));

        uno::Reference<text::XTextRange> xRun9
            = getRun(getParagraph(9), 1, u"\u00c0\u00c1\u00c2\u0102\u00c4"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun9, u"CharFontName"_ustr));

        // Ensure strange font remains strange. No reason to check content: in this case it can vary on locale
        uno::Reference<text::XTextRange> xRun10 = getRun(getParagraph(10), 1);
        CPPUNIT_ASSERT_EQUAL(u"Arial BlaBlaBla"_ustr,
                             getProperty<OUString>(xRun10, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun12
            = getRun(getParagraph(12), 1, u"\u0390\u0391\u0392\u0393\u0394"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun12, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun13
            = getRun(getParagraph(13), 1, u"\u0390\u0391\u0392\u0393\u0394"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun13, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun14 = getRun(getParagraph(14), 1);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun14, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun15
            = getRun(getParagraph(15), 1, u"\u0104\u012e\u0100\u0106\u00c4"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun15, u"CharFontName"_ustr));

        uno::Reference<text::XTextRange> xRun16
            = getRun(getParagraph(16), 1, u"\u0104\u012e\u0100\u0106\u00c4"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun16, u"CharFontName"_ustr));
    };
    createSwDoc("tdf95706.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95706_2)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(
            u"Arial"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Impact"_ustr, getProperty<OUString>(getRun(getParagraph(2), 1),
                                                                   u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            u"Times New Roman"_ustr,
            getProperty<OUString>(getRun(getParagraph(3), 1), u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Tahoma"_ustr, getProperty<OUString>(getRun(getParagraph(4), 1),
                                                                   u"CharFontName"_ustr));
    };
    createSwDoc("tdf95706_2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf111851)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);

        // No shading
        uno::Reference<text::XTextRange> xCell1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, xCell1->getString());
        CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell1, u"BackColor"_ustr));

        uno::Reference<text::XTextRange> xCell2(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"b"_ustr, xCell2->getString());
        CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell2, u"BackColor"_ustr));

        // Check some random not standard shading values and ensure some non-white background color
        uno::Reference<text::XTextRange> xCell3(xTable->getCellByName(u"C1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"c"_ustr, xCell3->getString());
        Color nShadingColor3 = getProperty<Color>(xCell3, u"BackColor"_ustr);
        CPPUNIT_ASSERT(COL_WHITE > nShadingColor3);
        CPPUNIT_ASSERT(COL_BLACK < nShadingColor3);

        uno::Reference<text::XTextRange> xCell4(xTable->getCellByName(u"D1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"d"_ustr, xCell4->getString());
        Color nShadingColor4 = getProperty<Color>(xCell4, u"BackColor"_ustr);
        CPPUNIT_ASSERT(COL_WHITE > nShadingColor4);
        CPPUNIT_ASSERT(COL_BLACK < nShadingColor4);

        // Values 10000 and more - black
        uno::Reference<text::XTextRange> xCell5(xTable->getCellByName(u"E1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"e"_ustr, xCell5->getString());
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xCell5, u"BackColor"_ustr));

        uno::Reference<text::XTextRange> xCell6(xTable->getCellByName(u"F1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"f"_ustr, xCell6->getString());
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xCell6, u"BackColor"_ustr));
    };
    createSwDoc("tdf111851.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf139948)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(0),
            getProperty<table::BorderLine2>(getParagraph(1, u"No border"_ustr), u"TopBorder"_ustr)
                .LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
                             getProperty<table::BorderLine2>(getParagraph(2, u"Border below"_ustr),
                                                             u"TopBorder"_ustr)
                                 .LineWidth);
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(88), getProperty<table::BorderLine2>(
                                getParagraph(3, u"Borders below and above"_ustr), u"TopBorder"_ustr)
                                .LineWidth);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(88),
                             getProperty<table::BorderLine2>(getParagraph(4, u"Border above"_ustr),
                                                             u"TopBorder"_ustr)
                                 .LineWidth);
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(0),
            getProperty<table::BorderLine2>(getParagraph(5, u"No border"_ustr), u"TopBorder"_ustr)
                .LineWidth);

        // And let's ensure that there are no other horizontal borders
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(0),
            getProperty<table::BorderLine2>(getParagraph(1), u"BottomBorder"_ustr).LineWidth);
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(0),
            getProperty<table::BorderLine2>(getParagraph(2), u"BottomBorder"_ustr).LineWidth);
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(0),
            getProperty<table::BorderLine2>(getParagraph(3), u"BottomBorder"_ustr).LineWidth);
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(0),
            getProperty<table::BorderLine2>(getParagraph(4), u"BottomBorder"_ustr).LineWidth);
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(0),
            getProperty<table::BorderLine2>(getParagraph(5), u"BottomBorder"_ustr).LineWidth);
    };
    createSwDoc("tdf139948.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103956)
{
    auto verify = [this]() {
        // Ensure that RTF tables without column width (\cellx or \cellx0) are displayed with some
        // suitable width. Currently there is no good support for autofit for RTF docs, so ensure
        // that table cells are somehow visible. Width for it is not clear, so here we will check
        // table & cell widths are more than default minimal value of 41.
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        CPPUNIT_ASSERT_MESSAGE(
            "Table #1 is too narrow!",
            82 < getXPath(pXmlDoc, "/root/page/body/tab[1]/row/infos/bounds", "width").toInt32());
        CPPUNIT_ASSERT_MESSAGE(
            "Table #1 cell#1 is too narrow!",
            41 < getXPath(pXmlDoc, "/root/page/body/tab[1]/row/cell[1]/infos/bounds", "width")
                     .toInt32());
        CPPUNIT_ASSERT_MESSAGE(
            "Table #1 cell#2 is too narrow!",
            41 < getXPath(pXmlDoc, "/root/page/body/tab[1]/row/cell[2]/infos/bounds", "width")
                     .toInt32());

        CPPUNIT_ASSERT_MESSAGE(
            "Table #2 is too narrow!",
            82 < getXPath(pXmlDoc, "/root/page/body/tab[2]/row/infos/bounds", "width").toInt32());
        CPPUNIT_ASSERT_MESSAGE(
            "Table #2 cell#1 is too narrow!",
            41 < getXPath(pXmlDoc, "/root/page/body/tab[2]/row/cell[1]/infos/bounds", "width")
                     .toInt32());
        CPPUNIT_ASSERT_MESSAGE(
            "Table #2 cell#2 is too narrow!",
            41 < getXPath(pXmlDoc, "/root/page/body/tab[2]/row/cell[2]/infos/bounds", "width")
                     .toInt32());

        CPPUNIT_ASSERT_MESSAGE(
            "Table #3 is too narrow!",
            82 < getXPath(pXmlDoc, "/root/page/body/tab[3]/row/infos/bounds", "width").toInt32());
        CPPUNIT_ASSERT_MESSAGE(
            "Table #3 cell#1 is too narrow!",
            41 < getXPath(pXmlDoc, "/root/page/body/tab[3]/row/cell[1]/infos/bounds", "width")
                     .toInt32());
        CPPUNIT_ASSERT_MESSAGE(
            "Table #3 cell#2 is too narrow!",
            41 < getXPath(pXmlDoc, "/root/page/body/tab[3]/row/cell[2]/infos/bounds", "width")
                     .toInt32());
    };
    createSwDoc("tdf103956.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148515)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);

        uno::Reference<text::XTextRange> xCell1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"XXXXXX"_ustr, xCell1->getString());
        CPPUNIT_ASSERT_EQUAL(5.0f,
                             getProperty<float>(getRun(getParagraphOfText(1, xCell1->getText()), 1),
                                                u"CharHeight"_ustr));

        uno::Reference<text::XTextRange> xCell2(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u""_ustr, xCell2->getString());
        CPPUNIT_ASSERT_EQUAL(5.0f,
                             getProperty<float>(getRun(getParagraphOfText(1, xCell2->getText()), 1),
                                                u"CharHeight"_ustr));

        CPPUNIT_ASSERT_EQUAL(10.f,
                             getProperty<float>(getRun(getParagraph(2), 1), u"CharHeight"_ustr));
    };
    createSwDoc("tdf148515.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
