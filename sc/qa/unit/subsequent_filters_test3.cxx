/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <sal/config.h>

#include <memory>

#include <sfx2/docfile.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdoole2.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <editeng/borderline.hxx>
#include <editeng/lineitem.hxx>
#include <dbdata.hxx>
#include <validat.hxx>
#include <userdat.hxx>
#include <postit.hxx>
#include <scitems.hxx>
#include <docsh.hxx>
#include <cellvalue.hxx>
#include <attrib.hxx>
#include <stlpool.hxx>
#include <hints.hxx>
#include <detfunc.hxx>
#include <scerrors.hxx>
#include <tabvwsh.hxx>
#include <scresid.hxx>
#include <globstr.hrc>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>
#include "helper/qahelper.hxx"
#include <officecfg/Office/Common.hxx>
#include <vcl/idletask.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScFiltersTest3 : public ScModelTestBase
{
public:
    ScFiltersTest3();
};

ScFiltersTest3::ScFiltersTest3()
    : ScModelTestBase("sc/qa/unit/data")
{
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testBorderColorsXLSXML)
{
    createScDoc("xml/border-colors.xml");
    ScDocument* pDoc = getScDoc();

    // B3 - red
    const ScPatternAttr* pPat = pDoc->GetPattern(ScAddress(1, 2, 0));
    CPPUNIT_ASSERT(pPat);
    const editeng::SvxBorderLine* pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, pLine->GetColor());

    // B4 - blue
    pPat = pDoc->GetPattern(ScAddress(1, 3, 0));
    CPPUNIT_ASSERT(pPat);
    pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 112, 192), pLine->GetColor());

    // B5 - green
    pPat = pDoc->GetPattern(ScAddress(1, 4, 0));
    CPPUNIT_ASSERT(pPat);
    pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 176, 80), pLine->GetColor());

    // B7 - yellow (left), purple (right), light blue (cross)
    pPat = pDoc->GetPattern(ScAddress(1, 6, 0));
    CPPUNIT_ASSERT(pPat);

    pLine = pPat->GetItem(ATTR_BORDER).GetLeft();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, pLine->GetColor());

    pLine = pPat->GetItem(ATTR_BORDER).GetRight();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(112, 48, 160), pLine->GetColor()); // purple

    pLine = pPat->GetItem(ATTR_BORDER_TLBR).GetLine();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 176, 240), pLine->GetColor()); // light blue

    pLine = pPat->GetItem(ATTR_BORDER_BLTR).GetLine();
    CPPUNIT_ASSERT(pLine);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pLine->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(Color(0, 176, 240), pLine->GetColor()); // light blue
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testHiddenRowsColumnsXLSXML)
{
    createScDoc("xml/hidden-rows-columns.xml");
    ScDocument* pDoc = getScDoc();

    struct Check
    {
        SCCOLROW nPos1;
        SCCOLROW nPos2;
        bool bVisible;
    };

    std::vector<Check> aRowChecks = {
        { 0, 0, true },
        { 1, 2, false },
        { 3, 3, true },
        { 4, 4, false },
        { 5, 7, true },
        { 8, 8, false },
        { 9, pDoc->MaxRow(), true },
    };

    for (const Check& c : aRowChecks)
    {
        SCROW nRow1 = -1, nRow2 = -1;
        bool bVisible = !pDoc->RowHidden(c.nPos1, 0, &nRow1, &nRow2);
        CPPUNIT_ASSERT_EQUAL(bVisible, c.bVisible);
        CPPUNIT_ASSERT_EQUAL(c.nPos1, nRow1);
        CPPUNIT_ASSERT_EQUAL(c.nPos2, nRow2);
    }

    std::vector<Check> aColChecks = {
        { 0, 1, true },
        { 2, 5, false },
        { 6, 9, true },
        { 10, 10, false },
        { 11, pDoc->MaxCol(), true },
    };

    for (const Check& c : aColChecks)
    {
        SCCOL nCol1 = -1, nCol2 = -1;
        bool bVisible = !pDoc->ColHidden(c.nPos1, 1, &nCol1, &nCol2);
        CPPUNIT_ASSERT_EQUAL(bVisible, c.bVisible);
        CPPUNIT_ASSERT_EQUAL(c.nPos1, SCCOLROW(nCol1));
        CPPUNIT_ASSERT_EQUAL(c.nPos2, SCCOLROW(nCol2));
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testColumnWidthRowHeightXLSXML)
{
    createScDoc("xml/column-width-row-height.xml");
    ScDocument* pDoc = getScDoc();

    struct RowHeight
    {
        SCROW nRow1;
        SCROW nRow2;
        sal_uInt16 nHeight; // in points (1 point == 20 twips)
    };

    std::vector<RowHeight> aRowChecks = {
        { 2, 2, 20 }, { 3, 3, 30 }, { 4, 4, 40 }, { 5, 5, 50 }, { 7, 9, 25 }, { 12, 13, 35 },
    };

    for (const RowHeight& rh : aRowChecks)
    {
        for (SCROW i = rh.nRow1; i <= rh.nRow2; ++i)
        {
            sal_uInt16 nHeight = pDoc->GetRowHeight(i, 0);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(rh.nHeight * 20), nHeight);
        }
    }

    struct ColWidth
    {
        SCCOL nCol1;
        SCCOL nCol2;
        sal_uInt16 nWidth; // in points (1 point == 20 twips
    };

    std::vector<ColWidth> aColChecks = {
        { 1, 1, 56 }, { 2, 2, 83 }, { 3, 3, 109 }, { 5, 7, 67 }, { 10, 11, 119 },
    };

    for (const ColWidth& cw : aColChecks)
    {
        for (SCCOL i = cw.nCol1; i <= cw.nCol2; ++i)
        {
            sal_uInt16 nWidth = pDoc->GetColWidth(i, 0);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(cw.nWidth * 20), nWidth);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testCharacterSetXLSXML)
{
    createScDoc("xml/character-set.xml");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pDoc->GetTableCount());

    OUString aName;
    pDoc->GetName(0, aName);

    // Check the sheet name.  The values are all Cyrillic letters.
    std::vector<sal_Unicode> aBuf = { 0x041b, 0x0438, 0x0441, 0x0442, 0x0031 };
    OUString aExpected(aBuf.data(), aBuf.size());
    CPPUNIT_ASSERT_EQUAL(aExpected, aName);

    // Check the value of I4
    OUString aVal = pDoc->GetString(ScAddress(8, 3, 0));
    aBuf = { 0x0421, 0x0443, 0x043c, 0x043c, 0x0430 };
    aExpected = OUString(aBuf.data(), aBuf.size());
    CPPUNIT_ASSERT_EQUAL(aExpected, aVal);

    // Check the value of J3
    aVal = pDoc->GetString(ScAddress(9, 2, 0));
    aBuf = { 0x041e, 0x0441, 0x0442, 0x0430, 0x0442, 0x043e, 0x043a };
    aExpected = OUString(aBuf.data(), aBuf.size());
    CPPUNIT_ASSERT_EQUAL(aExpected, aVal);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testWrapAndShrinkXLSXML)
{
    createScDoc("xml/wrap-and-shrink.xml");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pDoc->GetTableCount());

    struct Check
    {
        SCCOL nCol;
        SCROW nRow;

        bool bWrapText;
        bool bShrinkToFit;
    };

    constexpr Check aChecks[] = {
        { 1, 0, false, false },
        { 1, 1, true, false },
        { 1, 2, false, true },
    };

    for (const auto& rC : aChecks)
    {
        const ScLineBreakCell* pLB = pDoc->GetAttr(rC.nCol, rC.nRow, 0, ATTR_LINEBREAK);
        CPPUNIT_ASSERT_EQUAL(pLB->GetValue(), rC.bWrapText);

        const ScShrinkToFitCell* pSTF = pDoc->GetAttr(rC.nCol, rC.nRow, 0, ATTR_SHRINKTOFIT);
        CPPUNIT_ASSERT_EQUAL(pSTF->GetValue(), rC.bShrinkToFit);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testCondFormatXLSB)
{
    createScDoc("xlsb/cond_format.xlsb");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());
    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testPageScalingXLSX)
{
    createScDoc("xlsx/page_scale.xlsx");
    ScDocument* pDoc = getScDoc();

    OUString aStyleName = pDoc->GetPageStyle(0);
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find(aStyleName, SfxStyleFamily::Page);
    CPPUNIT_ASSERT(pStyleSheet);

    SfxItemSet& rSet = pStyleSheet->GetItemSet();
    sal_uInt16 nVal = rSet.Get(ATTR_PAGE_SCALE).GetValue();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(90), nVal);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testActiveXCheckboxXLSX)
{
    createScDoc("xlsx/activex_checkbox.xlsx");
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));

    // Check custom label
    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Caption"), sLabel);

    // Check background color (highlight system color)
    Color nColor;
    xPropertySet->getPropertyValue("BackgroundColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x316AC5), nColor);

    // Check Text color (active border system color)
    xPropertySet->getPropertyValue("TextColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xD4D0C8), nColor);

    // Check state of the checkbox
    sal_Int16 nState;
    xPropertySet->getPropertyValue("State") >>= nState;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nState);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf60673)
{
    createScDoc("xlsx/tdf60673.xlsx");
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);

    OUString sLabel;
    xPropertySet->getPropertyValue("Label") >>= sLabel;

    // Without the fix in place, this test would have failed with
    // - Expected: PL: ĄŚŻŹĆŃŁÓĘ
    // - Actual  : PL:
    CPPUNIT_ASSERT_EQUAL(u"PL: ĄŚŻŹĆŃŁÓĘ"_ustr, sLabel);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testtdf120301_xmlSpaceParsingXLSX)
{
    createScDoc("xlsx/tdf120301_xmlSpaceParsing.xlsx");
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);

    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> XPropSet(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    OUString sCaption;
    XPropSet->getPropertyValue("Label") >>= sCaption;
    CPPUNIT_ASSERT_EQUAL(OUString("Check Box 1"), sCaption);
}

namespace
{
struct PaintListener : public SfxListener
{
    bool mbCalled = false;
    virtual void Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint) override
    {
        const ScPaintHint* pPaintHint = dynamic_cast<const ScPaintHint*>(&rHint);
        if (pPaintHint)
        {
            if ((pPaintHint->GetStartCol() <= 0 && pPaintHint->GetEndCol() >= 0)
                && ((pPaintHint->GetStartRow() <= 9 && pPaintHint->GetEndRow() >= 9)
                    || (pPaintHint->GetStartRow() == 2 && pPaintHint->GetEndRow() == 3)
                    || (pPaintHint->GetStartRow() == 1 && pPaintHint->GetEndRow() == 1)))
            {
                mbCalled = true;
            }
        }
    }
};
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testCondFormatFormulaListenerXLSX)
{
    createScDoc("xlsx/cond_format_formula_listener.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    PaintListener aListener;
    aListener.StartListening(*pDocSh);
    ScDocument* pDoc = getScDoc();
    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());
    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);
    pDoc->SetDocVisible(true);
    pDoc->SetValue(0, 0, 0, 2.0);

    IdleTask::waitUntilIdleDispatched();

    CPPUNIT_ASSERT(aListener.mbCalled);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf131471)
{
    // Repaint range of conditional format in merged cell.
    createScDoc("ods/tdf131471.ods");
    ScDocShell* pDocSh = getScDocShell();
    PaintListener aListener;
    aListener.StartListening(*pDocSh);
    ScDocument* pDoc = getScDoc();
    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());
    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);
    pDoc->SetDocVisible(true);
    pDoc->SetValue(0, 0, 0, 1.0);

    IdleTask::waitUntilIdleDispatched();

    CPPUNIT_ASSERT(aListener.mbCalled);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf150815_RepaintSparkline)
{
    createScDoc("ods/tdf150815.ods");

    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();

    PaintListener aListener;
    aListener.StartListening(*pDocSh);

    auto pSparkline = pDoc->GetSparkline(ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(pSparkline);

    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->EnterData(0, 0, 0, "10");

    CPPUNIT_ASSERT(aListener.mbCalled);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf137091)
{
    // Set the system locale to Turkish
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("tr-TR");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    createScDoc("xlsx/tdf137091.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 28/4
    // - Actual  : Err:507
    CPPUNIT_ASSERT_EQUAL(OUString("28/4"), pDoc->GetString(ScAddress(2, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf141495)
{
    // Set the system locale to Turkish
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("tr-TR");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    createScDoc("xlsx/tdf141495.xlsx");
    ScDocument* pDoc = getScDoc();

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: 44926
    // - Actual  : #ADDIN?
    CPPUNIT_ASSERT_EQUAL(OUString("44926"), pDoc->GetString(ScAddress(11, 6, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf70455)
{
    createScDoc("xlsx/tdf70455.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: €780.00
    // - Actual  : Err:509
    CPPUNIT_ASSERT_EQUAL(u"€780.00"_ustr, pDoc->GetString(ScAddress(7, 7, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf62268)
{
    createScDoc("ods/tdf62268.ods");
    ScDocument* pDoc = getScDoc();
    int nHeight;

    SCTAB nTab = 0;
    nHeight = pDoc->GetRowHeight(0, nTab, false);
    CPPUNIT_ASSERT_LESSEQUAL(3, abs(256 - nHeight));
    nHeight = pDoc->GetRowHeight(1, nTab, false);
    CPPUNIT_ASSERT_LESSEQUAL(19, abs(1905 - nHeight));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf137453)
{
    createScDoc("ods/tdf137453.ods");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 3333333333/100
    // - Actual  : -961633963/100
    CPPUNIT_ASSERT_EQUAL(OUString("3333333333/100"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf112780)
{
    createScDoc("ods/tdf112780.ods");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : #VALUE!
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(3, 5, 0)));

    OUString aFormula = pDoc->GetFormula(3, 5, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=G6+J6+M6"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf72470)
{
    // Without the fix in place, this test would have hung

    createScDoc("xls/tdf72470.xls");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("name"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"أسمي walid"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf35636)
{
    createScDoc("ods/tdf35636.ods");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, SUMIF would have returned 0.0
    // with empty cells in the criteria
    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(1, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf98481)
{
    createScDoc("xlsx/tdf98481.xlsx");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(1, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(3, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf115022)
{
    createScDoc("xlsx/tdf115022.xlsx");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(6.0, pDoc->GetValue(ScAddress(1, 8, 0)));

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 21
    CPPUNIT_ASSERT_EQUAL(6.0, pDoc->GetValue(ScAddress(1, 8, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testVBAMacroFunctionODS)
{
    createScDoc("ods/vba_macro_functions.ods");
    ScDocument* pDoc = getScDoc();

    OUString aFunction = pDoc->GetFormula(2, 0, 0);
    std::cout << aFunction << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, pDoc->GetValue(2, 0, 0), 1e-6);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testAutoheight2Rows)
{
    createScDoc("ods/autoheight2rows.ods");
    ScDocument* pDoc = getScDoc();

    SCTAB nTab = 0;
    int nHeight1 = pDoc->GetRowHeight(0, nTab, false);
    int nHeight2 = pDoc->GetRowHeight(1, nTab, false);
    int nHeight4 = pDoc->GetRowHeight(3, nTab, false);
    int nHeight5 = pDoc->GetRowHeight(4, nTab, false);

    // We will do relative comparison, because calculated autoheight
    // can be different on different platforms
    CPPUNIT_ASSERT_MESSAGE("Row #1 and row #4 must have same height after load & auto-adjust",
                           abs(nHeight1 - nHeight4) < 10);
    CPPUNIT_ASSERT_MESSAGE("Row #2 and row #5 must have same height after load & auto-adjust",
                           abs(nHeight2 - nHeight5) < 10);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testXLSDefColWidth)
{
    // XLS has only 256 columns; but on import, we need to set default width to all above that limit
    createScDoc("xls/chartx.xls");
    ScDocument* pDoc = getScDoc();

    int nWidth = pDoc->GetColWidth(pDoc->MaxCol(), 0, false);
    // This was 1280
    CPPUNIT_ASSERT_EQUAL(1005, nWidth);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf148423)
{
    createScDoc("csv/tdf148423.csv");
    ScDocument* pDoc = getScDoc();

    int nWidth = pDoc->GetColWidth(0, 0, false);

    // Without the fix in place, this test would have failed with
    // - Expected: 32880
    // - Actual  : 112
    CPPUNIT_ASSERT_EQUAL(32880, nWidth);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf152053)
{
    createScDoc("csv/tdf152053.csv");
    ScDocument* pDoc = getScDoc();

    int nWidth = pDoc->GetColWidth(0, 0, false);

    // Without the fix in place, this test would have failed with
    // - Expected: 1162
    // - Actual  : 715
    // tolerance is for windows machines with non-default DPI
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1162, nWidth, 10);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testPreviewMissingObjLink)
{
    createScDoc("ods/keep-preview-missing-obj-link.ods");

    ScDocument* pDoc = getScDoc();

    // Retrieve the ole object
    const SdrOle2Obj* pOleObj = getSingleOleObject(*pDoc, 0);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve an ole object from the 2nd sheet.", pOleObj);

    const Graphic* pGraphic = pOleObj->GetGraphic();
    CPPUNIT_ASSERT_MESSAGE(
        "the ole object links to a missing file, but we should retain its preview", pGraphic);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testShapeRotationImport)
{
    if (!IsDefaultDPI())
        return;
    // tdf#83593 Incorrectly calculated bounding rectangles caused shapes to appear as if there
    // were extra or missing rotations. Hence, we check the sizes of these rectangles.
    createScDoc("xlsx/testShapeRotationImport.xlsx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);

    // The expected values are in the map below. Note that some of the angles are outside of the set which contains
    // the value of the wrong angles. This is to check the border cases and one value on both sides.
    std::map<sal_Int32, std::map<std::string, sal_Int32>> aExpectedValues{
        { 4400, { { "x", 6826 }, { "y", 36891 }, { "width", 8889 }, { "height", 1163 } } },
        { 4500, { { "x", 4485 }, { "y", 36397 }, { "width", 8889 }, { "height", 1164 } } },
        { 4600, { { "x", 1669 }, { "y", 36272 }, { "width", 8863 }, { "height", 1142 } } },
        { 13400, { { "x", 13752 }, { "y", 28403 }, { "width", 8863 }, { "height", 1194 } } },
        { 13500, { { "x", 10810 }, { "y", 27951 }, { "width", 8863 }, { "height", 1168 } } },
        { 13600, { { "x", 8442 }, { "y", 28334 }, { "width", 8889 }, { "height", 1163 } } },
        { 22400, { { "x", 14934 }, { "y", 12981 }, { "width", 8889 }, { "height", 1163 } } },
        { 22500, { { "x", 11754 }, { "y", 12837 }, { "width", 8889 }, { "height", 1163 } } },
        { 22600, { { "x", 8248 }, { "y", 12919 }, { "width", 8862 }, { "height", 1169 } } },
        { 31400, { { "x", 8089 }, { "y", 1160 }, { "width", 9815 }, { "height", 1171 } } },
        { 31500, { { "x", 4421 }, { "y", 1274 }, { "width", 10238 }, { "height", 1171 } } },
        { 31600, { { "x", 1963 }, { "y", 1882 }, { "width", 10297 }, { "height", 1163 } } },
    };

    for (sal_Int32 ind = 0; ind < 12; ++ind)
    {
        uno::Reference<drawing::XShape> xShape(xPage->getByIndex(ind), uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        uno::Any nRotProp = xShapeProperties->getPropertyValue("RotateAngle");
        sal_Int32 nRot = nRotProp.get<sal_Int32>();
        const OString sNote = "RotateAngle = " + OString::number(nRot);

        awt::Point aPosition = xShape->getPosition();
        awt::Size aSize = xShape->getSize();

        CPPUNIT_ASSERT_MESSAGE(sNote.getStr(), aExpectedValues.find(nRot) != aExpectedValues.end());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["x"], aPosition.X);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["y"], aPosition.Y);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["width"], aSize.Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sNote.getStr(), aExpectedValues[nRot]["height"], aSize.Height);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testShapeDisplacementOnRotationImport)
{
    // tdf#135918 shape is displaced on rotation if it is placed next to the sheets upper/left edges
    createScDoc("xlsx/testShapeDisplacementOnRotationImport.xlsx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);
    uno::Any aRectProp = xShapeProperties->getPropertyValue("FrameRect");
    awt::Rectangle aRectangle = aRectProp.get<awt::Rectangle>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRectangle.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRectangle.Y);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTextBoxBodyUpright)
{
    // tdf#106197 We should import the "upright" attribute of txBody.
    createScDoc("xlsx/tdf106197_import_upright.xlsx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);

    // Check that we imported "Upright".
    bool isUpright = false;
    if (xShapeProperties->getPropertySetInfo()->hasPropertyByName("InteropGrabBag"))
    {
        uno::Sequence<beans::PropertyValue> aGrabBag;
        xShapeProperties->getPropertyValue("InteropGrabBag") >>= aGrabBag;
        for (auto& aProp : aGrabBag)
        {
            if (aProp.Name == "Upright")
            {
                aProp.Value >>= isUpright;
                break;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(true, isUpright);

    // Check the TextPreRotateAngle has the compensation for the additional 90deg area rotation,
    // which is added in Shape::createAndInsert to get the same rendering as in MS Office.
    sal_Int32 nAngle;
    uno::Any aGeom = xShapeProperties->getPropertyValue("CustomShapeGeometry");
    auto aGeomSeq = aGeom.get<Sequence<beans::PropertyValue>>();
    for (const auto& aProp : aGeomSeq)
    {
        if (aProp.Name == "TextPreRotateAngle")
        {
            aProp.Value >>= nAngle;
            break;
        }
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(90), nAngle);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTextBoxBodyRotateAngle)
{
    createScDoc("xlsx/tdf141644.xlsx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);

    // Check the text direction.
    sal_Int16 eWritingMode = text::WritingMode2::LR_TB;
    if (xShapeProperties->getPropertySetInfo()->hasPropertyByName("WritingMode"))
        xShapeProperties->getPropertyValue("WritingMode") >>= eWritingMode;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(text::WritingMode2::BT_LR), eWritingMode);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTextLengthDataValidityXLSX)
{
    createScDoc("xlsx/textLengthDataValidity.xlsx");

    ScDocument* pDoc = getScDoc();

    const ScValidationData* pData = pDoc->GetValidationEntry(1);

    ScRefCellValue aCellA1; // A1 = 1234(numeric value)
    ScAddress aValBaseAddrA1(0, 0, 0);
    aCellA1.assign(*pDoc, aValBaseAddrA1);
    bool bValidA1 = pData->IsDataValid(aCellA1, aValBaseAddrA1);

    ScRefCellValue aCellA2; // A2 = 1234(numeric value format as text)
    ScAddress aValBaseAddrA2(0, 1, 0);
    aCellA2.assign(*pDoc, aValBaseAddrA2);
    bool bValidA2 = pData->IsDataValid(aCellA2, aValBaseAddrA2);

    ScRefCellValue aCellA3; // A3 = 1234.00(numeric value)
    ScAddress aValBaseAddrA3(0, 2, 0);
    aCellA3.assign(*pDoc, aValBaseAddrA3);
    bool bValidA3 = pData->IsDataValid(aCellA3, aValBaseAddrA3);

    ScRefCellValue aCellA4; // A4 = 12.3(numeric value)
    ScAddress aValBaseAddrA4(0, 3, 0);
    aCellA4.assign(*pDoc, aValBaseAddrA4);
    bool bValidA4 = pData->IsDataValid(aCellA4, aValBaseAddrA4);

    // True if text length = 4
    CPPUNIT_ASSERT_EQUAL(true, bValidA1);
    CPPUNIT_ASSERT_EQUAL(true, bValidA2);
    CPPUNIT_ASSERT_EQUAL(true, bValidA3);
    CPPUNIT_ASSERT_EQUAL(true, bValidA4);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testDeleteCircles)
{
    createScDoc("xlsx/testDeleteCircles.xlsx");

    ScDocument* pDoc = getScDoc();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    ScRefCellValue aCellA1; // A1 = "Hello"
    ScAddress aPosA1(0, 0, 0);
    aCellA1.assign(*pDoc, aPosA1);

    // Mark invalid value
    bool bOverflow;
    bool bMarkInvalid = ScDetectiveFunc(*pDoc, 0).MarkInvalid(bOverflow);
    CPPUNIT_ASSERT_EQUAL(true, bMarkInvalid);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    // The value of A1 change to Hello1.
    pDoc->SetString(0, 0, 0, "Hello1");

    // Check that the data is valid.(True if text length = 6)
    const ScValidationData* pData = pDoc->GetValidationEntry(1);
    bool bValidA1 = pData->IsDataValid(aCellA1, aPosA1);
    // if valid, delete circle.
    if (bValidA1)
        ScDetectiveFunc(*pDoc, 0).DeleteCirclesAt(aPosA1.Col(), aPosA1.Row());

    // There should not be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testDrawCircleInMergeCells)
{
    createScDoc("xlsx/testDrawCircleInMergeCells.xlsx");

    ScDocument* pDoc = getScDoc();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // A1:B2 is merged.
    ScRange aMergedRange(0, 0, 0);
    pDoc->ExtendTotalMerge(aMergedRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 1, 1, 0), aMergedRange);

    // Mark invalid value
    bool bOverflow;
    bool bMarkInvalid = ScDetectiveFunc(*pDoc, 0).MarkInvalid(bOverflow);
    CPPUNIT_ASSERT_EQUAL(true, bMarkInvalid);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    SdrObject* pObj = pPage->GetObj(0);
    tools::Rectangle aRect(pObj->GetLogicRect());
    Point aStartCircle = aRect.TopLeft();
    Point aEndCircle = aRect.BottomRight();

    tools::Rectangle aCellRect = pDoc->GetMMRect(0, 0, 1, 1, 0);
    aCellRect.AdjustLeft(-250);
    aCellRect.AdjustRight(250);
    aCellRect.AdjustTop(-70);
    aCellRect.AdjustBottom(70);
    Point aStartCell = aCellRect.TopLeft();
    Point aEndCell = aCellRect.BottomRight();

    CPPUNIT_ASSERT_EQUAL(aStartCell.X(), aStartCircle.X());
    CPPUNIT_ASSERT_EQUAL(aEndCell.X(), aEndCircle.X());
    CPPUNIT_ASSERT_EQUAL(aStartCell.Y(), aStartCircle.Y());
    CPPUNIT_ASSERT_EQUAL(aEndCell.Y(), aEndCircle.Y());

    // Change the height of the first row. (556 ~ 1cm)
    pDoc->SetRowHeight(0, 0, 556);
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    pDrawLayer->RecalcPos(pObj, *pData, false, false);

    tools::Rectangle aRecalcRect(pObj->GetLogicRect());
    Point aStartRecalcCircle = aRecalcRect.TopLeft();
    Point aEndRecalcCircle = aRecalcRect.BottomRight();

    tools::Rectangle aRecalcCellRect = pDoc->GetMMRect(0, 0, 1, 1, 0);
    aRecalcCellRect.AdjustLeft(-250);
    aRecalcCellRect.AdjustRight(250);
    aRecalcCellRect.AdjustTop(-70);
    aRecalcCellRect.AdjustBottom(70);
    Point aStartRecalcCell = aRecalcCellRect.TopLeft();
    Point aEndRecalcCell1 = aRecalcCellRect.BottomRight();

    CPPUNIT_ASSERT_EQUAL(aStartRecalcCell.X(), aStartRecalcCircle.X());
    CPPUNIT_ASSERT_EQUAL(aEndRecalcCell1.X(), aEndRecalcCircle.X());
    CPPUNIT_ASSERT_EQUAL(aStartRecalcCell.Y(), aStartRecalcCircle.Y());
    CPPUNIT_ASSERT_EQUAL(aEndRecalcCell1.Y(), aEndRecalcCircle.Y());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testDeleteCirclesInRowAndCol)
{
    createScDoc("ods/deleteCirclesInRowAndCol.ods");

    ScDocument* pDoc = getScDoc();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // There should be 6 circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), pPage->GetObjCount());

    // Delete first row.
    pDrawLayer->DeleteObjectsInArea(0, 0, 0, pDoc->MaxCol(), 0, true);

    // There should be 3 circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pPage->GetObjCount());

    // Delete first col.
    pDrawLayer->DeleteObjectsInArea(0, 0, 0, 0, pDoc->MaxRow(), true);

    // There should not be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf129940)
{
    // Test pure span elements inside text:ruby-base
    createScDoc("ods/tdf129940.ods");
    ScDocument* pDoc = getScDoc();
    // Pure text within text:ruby-base
    OUString aStr = pDoc->GetString(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"小笠原"_ustr, aStr);
    aStr = pDoc->GetString(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"徳彦"_ustr, aStr);

    // Multiple text:span within text:ruby-base
    aStr = pDoc->GetString(ScAddress(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"注音符號"_ustr, aStr);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf119190)
{
    createScDoc("xlsx/tdf119190.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(pDoc->HasNote(ScAddress(2, 0, 0)));

    ScPostIt* pNote = pDoc->GetNote(ScAddress(2, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(true, pNote->IsCaptionShown());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf139612)
{
    createScDoc("ods/tdf139612.ods");
    ScDocument* pDoc = getScDoc();

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1, 15, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf144740)
{
    createScDoc("ods/tdf144740.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf146722)
{
    createScDoc("ods/tdf146722.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 3
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(1, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf147014)
{
    createScDoc("xlsx/tdf147014.xlsx");
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 uno::UNO_QUERY_THROW);
    xIA.set(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW);
    // The sheet has a single shape, without the fix it was not imported, except in 32-bit builds
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape not imported", static_cast<sal_Int32>(1), xIA->getCount());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf139763ShapeAnchor)
{
    createScDoc("xlsx/tdf139763ShapeAnchor.xlsx");

    // There are two objects on the first sheet, anchored to page by element xdr:absoluteAnchor
    // and anchored to cell by element xdr:oneCellAnchor. Error was, that they were imported as
    // "anchor to cell (resize with cell".
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    // There should be 2 shapes
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pPage->GetObjCount());

    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get page anchored object.", pObj);
    CPPUNIT_ASSERT_MESSAGE("Shape must be page anchored", !ScDrawLayer::IsCellAnchored(*pObj));

    pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("Failed to get cell anchored object.", pObj);
    CPPUNIT_ASSERT_MESSAGE("Shape must be anchored to cell.", ScDrawLayer::IsCellAnchored(*pObj));
    CPPUNIT_ASSERT_MESSAGE("Shape must not resize with cell.",
                           !ScDrawLayer::IsResizeWithCell(*pObj));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testAutofilterNamedRangesXLSX)
{
    createScDoc("xlsx/autofilternamedrange.xlsx");

    ScDocument* pDoc = getScDoc();
    const ScDBData* pDBData = pDoc->GetDBCollection()->GetDBNearCursor(0, 0, 0);
    CPPUNIT_ASSERT(pDBData);
    ScRange aRange;
    pDBData->GetArea(aRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 0, 3, 0), aRange);
    OUString aPosStr;
    bool bSheetLocal = false;
    // test there is no '_xlnm._FilterDatabase' named range on the filter area
    const ScRangeData* pRData = pDoc->GetRangeAtBlock(aRange, aPosStr, &bSheetLocal);
    CPPUNIT_ASSERT(!pRData);
    CPPUNIT_ASSERT_EQUAL(OUString(), aPosStr);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testInvalidBareBiff5)
{
    createScDoc("xls/tdf144732.xls");
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    // Check that we import the contents from such file, as Excel does
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pDoc->GetTableCount());

    // Row 1
    ScAddress aPos(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));

    // Row 2
    aPos = ScAddress(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, pDoc->GetCellType(aPos));
    OUString sFormula = pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(OUString("=TRUE()"), sFormula);
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_FORMULA, pDoc->GetCellType(aPos));
    sFormula = pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab());
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), sFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"sheetjs"_ustr, pDoc->GetString(aPos));

    // Row 3
    aPos = ScAddress(0, 2, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"foo    bar"_ustr, pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(41689.4375, pDoc->GetValue(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"0.3"_ustr, pDoc->GetString(aPos));

    // Row 4
    aPos = ScAddress(0, 3, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"_"_ustr, pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_VALUE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(3.14159, pDoc->GetValue(aPos));

    // Row 5
    aPos = ScAddress(0, 4, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"hidden"_ustr, pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));

    // Row 6
    aPos = ScAddress(0, 5, 0);
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(u"visible"_ustr, pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));
    aPos.IncCol();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_NONE, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(aPos));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTooManyColsRows)
{
    // The intentionally doc has cells beyond our MAXROW/MAXCOL, so there
    // should be a warning on load.
    createScDoc("ods/too-many-cols-rows.ods", /*pPassword*/ nullptr, /*bCheckWarningError*/ false);

    ScDocShell* pDocSh = getScDocShell();
    SfxMedium* pMedium = pDocSh->GetMedium();

    CPPUNIT_ASSERT(pMedium->GetWarningError() == SCWARN_IMPORT_ROW_OVERFLOW
                   || pMedium->GetWarningError() == SCWARN_IMPORT_COLUMN_OVERFLOW);

    createScDoc("xlsx/too-many-cols-rows.xlsx", /*pPassword*/ nullptr,
                /*bCheckWarningError*/ false);

    pDocSh = getScDocShell();
    pMedium = pDocSh->GetMedium();

    CPPUNIT_ASSERT(pMedium->GetWarningError() == SCWARN_IMPORT_ROW_OVERFLOW
                   || pMedium->GetWarningError() == SCWARN_IMPORT_COLUMN_OVERFLOW);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf83671_SmartArt_import)
{
    // The example doc contains a diagram (SmartArt). Such should be imported as group object.
    // Error was, that the background shape had size zero and the diagram shapes where missing.

    // Make sure SmartArt is loaded as group shape
    bool bUseGroup = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pChange);
        pChange->commit();
    }

    // Get document and shape
    createScDoc("xlsx/tdf83671_SmartArt_import.xlsx");
    ScDocument* pDoc = getScDoc();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    SdrObject* pObj = pPage->GetObj(0);

    // Check that it is a group shape with 4 children
    CPPUNIT_ASSERT(pObj->IsGroupObject());
    SdrObjList* pChildren = pObj->getChildrenOfSdrObject();
    CPPUNIT_ASSERT_EQUAL(size_t(4), pChildren->GetObjCount());

    // The background shape should have about 60mm x 42mm size.
    // Without fix its size was zero.
    tools::Rectangle aBackground = pChildren->GetObj(0)->GetLogicRect();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(6000), aBackground.getOpenWidth(), 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4200), aBackground.getOpenHeight(), 10);

    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(false, pChange);
        pChange->commit();
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf83671_SmartArt_import2)
{
    // The example doc contains a diagram (SmartArt). Such should be imported as group object.
    // With conversion enabled, the group contains only a graphic. Error was, that the shape
    // had size 100x100 Hmm and position 0|0.

    // Make sure SmartArt is loaded with converting to metafile
    bool bUseGroup = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    if (bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(false, pChange);
        pChange->commit();
    }

    // Get document and shape
    createScDoc("xlsx/tdf83671_SmartArt_import.xlsx");
    ScDocument* pDoc = getScDoc();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    SdrObject* pObj = pPage->GetObj(0);

    // Check that it is a group shape with 1 child
    CPPUNIT_ASSERT(pObj->IsGroupObject());
    SdrObjList* pChildren = pObj->getChildrenOfSdrObject();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pChildren->GetObjCount());

    // The child shape should have about 60mm x 42mm size and position 1164|1270.
    // Without fix its size was 100x100 and position 0|0.
    tools::Rectangle aBackground = pChildren->GetObj(0)->GetLogicRect();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(6000), aBackground.getOpenWidth(), 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4200), aBackground.getOpenHeight(), 10);
    CPPUNIT_ASSERT_EQUAL(Point(1164, 1270), aBackground.GetPos());

    if (bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pChange);
        pChange->commit();
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf151818_SmartArtFontColor)
{
    // The document contains a SmartArt where the color for the texts in the shapes is given by
    // the theme.
    // Error was, that the theme was not considered and therefore the text was white.

    // Make sure it is not loaded as metafile but with single shapes.
    bool bUseGroup = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pChange);
        pChange->commit();
    }

    // Get document and shape in SmartArt object
    createScDoc("xlsx/tdf151818_SmartartThemeFontColor.xlsx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xSmartArt(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    // shape 0 is the background shape without text
    uno::Reference<text::XTextRange> xShape(xSmartArt->getByIndex(1), uno::UNO_QUERY);

    // text color
    uno::Reference<container::XEnumerationAccess> xText(xShape->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(xText->createEnumeration()->nextElement(),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);
    Color nActualColor{ 0 };
    xPortion->getPropertyValue("CharColor") >>= nActualColor;
    // Without fix the test would have failed with:
    // - Expected: rgba[44546aff]
    // - Actual  : rgba[ffffffff], that is text was white
    CPPUNIT_ASSERT_EQUAL(Color(0x44546A), nActualColor);

    // clrScheme. For map between name in xlsx and index from CharColorTheme see
    // oox::drawingml::Color::getSchemeColorIndex()
    // Without fix the color scheme was "lt1" (1) but should be "dk2" (2).
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2),
                         xPortion->getPropertyValue("CharColorTheme").get<sal_Int16>());

    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(false, pChange);
        pChange->commit();
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf137216_HideCol)
{
    // The document contains a shape anchored "To Cell (resize with cell)" with start in C3.
    // Error was, that hiding column C did not make the shape invisible.

    // Get document
    createScDoc("ods/tdf137216_HideCol.ods");
    ScDocument* pDoc = getScDoc();

    // Get shape
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object found", pObj);

    // Assert object is visible before and invisible after hiding column.
    CPPUNIT_ASSERT_MESSAGE("before column hide: Object should be visible", pObj->IsVisible());
    pDoc->SetColHidden(2, 2, 0, true); // col C in UI = col index 2 to 2.
    CPPUNIT_ASSERT_MESSAGE("after column hide: Object should be invisible", !pObj->IsVisible());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf64229)
{
    createScDoc("ods/fdo64229b.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    OUString aCSVFileName = createFilePath(u"contentCSV/fdo64229b.csv");
    testFile(aCSVFileName, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf36933)
{
    createScDoc("ods/fdo36933test.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    OUString aCSVFileName = createFilePath(u"contentCSV/fdo36933test.csv");
    testFile(aCSVFileName, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf43700)
{
    createScDoc("ods/fdo43700test.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    OUString aCSVFileName = createFilePath(u"contentCSV/fdo43700test.csv");
    testFile(aCSVFileName, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf43534)
{
    createScDoc("ods/fdo43534test.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    // createCSVPath("fdo43534test.", aCSVFileName);
    // testFile(aCSVFileName, rDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf91979)
{
    createScDoc();

    ScDocument* pDoc = getScDoc();
    ScTabViewShell* pViewShell = getViewShell();
    auto& aViewData = pViewShell->GetViewData();

    // Check coordinates of a distant cell
    Point aPos = aViewData.GetScrPos(pDoc->MaxCol() - 1, 10000, SC_SPLIT_TOPLEFT, true);
    int nColWidth = ScViewData::ToPixel(pDoc->GetColWidth(0, 0), aViewData.GetPPTX());
    int nRowHeight = ScViewData::ToPixel(pDoc->GetRowHeight(0, 0), aViewData.GetPPTY());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>((pDoc->MaxCol() - 1) * nColWidth), aPos.getX());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(10000 * nRowHeight), aPos.getY());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf98657)
{
    createScDoc("ods/tdf98657.ods");
    ScDocument* pDoc = getScDoc();

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    // this was a NaN before the fix
    CPPUNIT_ASSERT_EQUAL(285.0, pDoc->GetValue(ScAddress(1, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf88821)
{
    setImportFilterName("calc_HTML_WebQuery");
    createScDoc("html/tdf88821.html");
    ScDocument* pDoc = getScDoc();

    // B2 should be 'Périmètre', not 'PÃ©rimÃ¨tre'
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("P\xC3\xA9rim\xC3\xA8tre", RTL_TEXTENCODING_UTF8),
                         pDoc->GetString(1, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf88821_2)
{
    setImportFilterName("calc_HTML_WebQuery");
    createScDoc("html/tdf88821-2.html");
    ScDocument* pDoc = getScDoc();

    // A2 should be 'ABCabcČŠŽčšž', not 'ABCabcÄŒÅ Å½ÄﾍÅ¡Å¾'
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("ABCabc\xC4\x8C\xC5\xA0\xC5\xBD\xC4\x8D\xC5\xA1\xC5\xBE",
                                           RTL_TEXTENCODING_UTF8),
                         pDoc->GetString(0, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf103960)
{
    setImportFilterName("calc_HTML_WebQuery");
    createScDoc("html/tdf103960.html");
    ScDocument* pDoc = getScDoc();

    // A1 should be 'Data', not the entire content of the file
    CPPUNIT_ASSERT_EQUAL(OStringToOUString("Data", RTL_TEXTENCODING_UTF8),
                         pDoc->GetString(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testRhbz1390776)
{
    createScDoc("xml/rhbz1390776.xml");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong range", OUString("=SUM(A18:A23)"),
                                 pDoc->GetFormula(0, 27, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf104310)
{
    // 1. Test x14 extension
    {
        createScDoc("xlsx/tdf104310.xlsx");
        ScDocument* pDoc = getScDoc();

        const ScValidationData* pData = pDoc->GetValidationEntry(1);
        CPPUNIT_ASSERT(pData);

        // Make sure the list is correct.
        std::vector<ScTypedStrData> aList;
        pData->FillSelectionList(aList, ScAddress(0, 1, 0));
        CPPUNIT_ASSERT_EQUAL(size_t(5), aList.size());
        for (size_t i = 0; i < 5; ++i)
            CPPUNIT_ASSERT_DOUBLES_EQUAL(double(i + 1), aList[i].GetValue(), 1e-8);
    }

    // 2. Test x12ac extension
    {
        createScDoc("xlsx/tdf104310-2.xlsx");
        ScDocument* pDoc = getScDoc();

        const ScValidationData* pData = pDoc->GetValidationEntry(1);
        CPPUNIT_ASSERT(pData);

        // Make sure the list is correct.
        std::vector<ScTypedStrData> aList;
        pData->FillSelectionList(aList, ScAddress(0, 1, 0));
        CPPUNIT_ASSERT_EQUAL(size_t(3), aList.size());
        CPPUNIT_ASSERT_EQUAL(OUString("1"), aList[0].GetString());
        CPPUNIT_ASSERT_EQUAL(OUString("2,3"), aList[1].GetString());
        CPPUNIT_ASSERT_EQUAL(OUString("4"), aList[2].GetString());
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf31231)
{
    // We must open it read-write to allow setting modified flag
    createScDoc("ods/tdf31231.ods");
    ScDocShell* pDocSh = getScDocShell();

    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must be allowed to set modified state",
                           pDocSh->IsEnableSetModified());
    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must not be modified on open", !pDocSh->IsModified());

    pDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must be allowed to set modified state",
                           pDocSh->IsEnableSetModified());
    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must not be modified on open", pDocSh->IsModified());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf141914)
{
    // We must open it read-write to allow setting modified flag
    createScDoc("ods/tdf141914.ods");
    ScDocShell* pDocSh = getScDocShell();

    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must be allowed to set modified state",
                           pDocSh->IsEnableSetModified());
    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must not be modified on open", !pDocSh->IsModified());

    pDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must be allowed to set modified state",
                           pDocSh->IsEnableSetModified());
    CPPUNIT_ASSERT_MESSAGE("The spreadsheet must not be modified on open", pDocSh->IsModified());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf128951)
{
    createScDoc();

    // 2. Create a new sheet instance
    css::uno::Reference<css::lang::XMultiServiceFactory> xFac(mxComponent,
                                                              css::uno::UNO_QUERY_THROW);
    auto xSheet = xFac->createInstance("com.sun.star.sheet.Spreadsheet");

    // 3. Insert sheet into the spreadsheet (was throwing IllegalArgumentException)
    css::uno::Reference<css::sheet::XSpreadsheetDocument> xDoc(mxComponent,
                                                               css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_NO_THROW(xDoc->getSheets()->insertByName("mustNotThrow", css::uno::Any(xSheet)));
}

namespace
{
SdrCaptionObj* checkCaption(ScDocument& rDoc, const ScAddress& rAddress, bool bIsShown)
{
    ScPostIt* pNote = rDoc.GetNote(rAddress);

    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(pNote->IsCaptionShown(), bIsShown);

    if (!bIsShown)
        pNote->ShowCaption(rAddress, true);

    SdrCaptionObj* pCaption = pNote->GetCaption();
    CPPUNIT_ASSERT(pCaption);

    return pCaption;
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf129789)
{
    createScDoc("ods/tdf129789.ods");
    ScDocument* pDoc = getScDoc();

    {
        // Fill: None
        SdrCaptionObj* const pCaptionB2 = checkCaption(*pDoc, ScAddress(1, 1, 0), true);

        const XFillStyleItem& rStyleItemB2 = pCaptionB2->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rStyleItemB2.GetValue());

        SdrCaptionObj* const pCaptionB9 = checkCaption(*pDoc, ScAddress(1, 8, 0), false);

        const XFillStyleItem& rStyleItemB9 = pCaptionB9->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rStyleItemB9.GetValue());
    }

    {
        // Fill: Solid
        SdrCaptionObj* const pCaptionE2 = checkCaption(*pDoc, ScAddress(4, 1, 0), true);

        const XFillStyleItem& rStyleItemE2 = pCaptionE2->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItemE2.GetValue());

        const XFillColorItem& rColorItem = pCaptionE2->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0xffffc0), rColorItem.GetColorValue());

        SdrCaptionObj* const pCaptionE9 = checkCaption(*pDoc, ScAddress(4, 8, 0), false);

        const XFillStyleItem& rStyleItemE9 = pCaptionE9->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItemE9.GetValue());

        const XFillColorItem& rColorItem2 = pCaptionE9->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0xffffc0), rColorItem2.GetColorValue());
    }

    {
        // Fill: Gradient
        SdrCaptionObj* const pCaptionH2 = checkCaption(*pDoc, ScAddress(7, 1, 0), true);

        const XFillStyleItem& rStyleItemH2 = pCaptionH2->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, rStyleItemH2.GetValue());
        const XFillGradientItem& rGradientItem = pCaptionH2->GetMergedItem(XATTR_FILLGRADIENT);
        const basegfx::BColorStops& rColorStops(rGradientItem.GetGradientValue().GetColorStops());

        CPPUNIT_ASSERT_EQUAL(size_t(2), rColorStops.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(rColorStops[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xdde8cb), Color(rColorStops[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(rColorStops[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xffd7d7), Color(rColorStops[1].getStopColor()));

        SdrCaptionObj* const pCaptionH9 = checkCaption(*pDoc, ScAddress(7, 8, 0), false);

        const XFillStyleItem& rStyleItemH9 = pCaptionH9->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, rStyleItemH9.GetValue());
        const XFillGradientItem& rGradientItem2 = pCaptionH2->GetMergedItem(XATTR_FILLGRADIENT);
        const basegfx::BColorStops& rColorStops2(rGradientItem2.GetGradientValue().GetColorStops());

        CPPUNIT_ASSERT_EQUAL(size_t(2), rColorStops2.size());
        CPPUNIT_ASSERT(basegfx::fTools::equal(rColorStops2[0].getStopOffset(), 0.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xdde8cb), Color(rColorStops2[0].getStopColor()));
        CPPUNIT_ASSERT(basegfx::fTools::equal(rColorStops2[1].getStopOffset(), 1.0));
        CPPUNIT_ASSERT_EQUAL(Color(0xffd7d7), Color(rColorStops2[1].getStopColor()));
    }

    {
        // Fill: Hatch
        SdrCaptionObj* const pCaptionK2 = checkCaption(*pDoc, ScAddress(10, 1, 0), true);

        const XFillStyleItem& rStyleItemK2 = pCaptionK2->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, rStyleItemK2.GetValue());
        const XFillHatchItem& rHatchItem = pCaptionK2->GetMergedItem(XATTR_FILLHATCH);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, rHatchItem.GetHatchValue().GetColor());

        SdrCaptionObj* const pCaptionK9 = checkCaption(*pDoc, ScAddress(10, 8, 0), false);

        const XFillStyleItem& rStyleItemK9 = pCaptionK9->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_HATCH, rStyleItemK9.GetValue());
        const XFillHatchItem& rHatchItem2 = pCaptionK9->GetMergedItem(XATTR_FILLHATCH);
        CPPUNIT_ASSERT_EQUAL(COL_BLUE, rHatchItem2.GetHatchValue().GetColor());
    }

    {
        // Fill: Bitmap
        SdrCaptionObj* const pCaptionN2 = checkCaption(*pDoc, ScAddress(13, 1, 0), true);

        const XFillStyleItem& rStyleItemN2 = pCaptionN2->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, rStyleItemN2.GetValue());

        SdrCaptionObj* const pCaptionN9 = checkCaption(*pDoc, ScAddress(13, 8, 0), false);

        const XFillStyleItem& rStyleItemN9 = pCaptionN9->GetMergedItem(XATTR_FILLSTYLE);

        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, rStyleItemN9.GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf130725)
{
    createScDoc();

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    // 2. Insert 0.0042 into a cell as a formula, to force the conversion from string to double
    css::uno::Reference<css::sheet::XCellRangesAccess> xSheets(xDoc->getSheets(),
                                                               css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XCell> xCell = xSheets->getCellByPosition(0, 0, 0);
    xCell->setFormula("0.0042"); // this assumes en-US locale

    // 3. Check that the value is the nearest double-precision representation of the decimal 0.0042
    //    (it was 0.0042000000000000006 instead of 0.0041999999999999997).
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Value must be the nearest representation of decimal 0.0042",
                                 0.0042, xCell->getValue()); // strict equality
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf104502_hiddenColsCountedInPageCount)
{
    createScDoc("ods/tdf104502_hiddenColsCountedInPageCount.ods");

    ScDocument* pDoc = getScDoc();

    //Check that hidden columns are not calculated into Print Area
    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    CPPUNIT_ASSERT(pDoc->GetPrintArea(0, nEndCol, nEndRow, false));
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), nEndCol);
    CPPUNIT_ASSERT_EQUAL(SCROW(55), nEndRow);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf82984_zip64XLSXImport)
{
    // Without the fix in place, it would have crashed at import time
    createScDoc("xlsx/tdf82984_zip64XLSXImport.xlsx");
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest3, testTdf108188_pagestyle)
{
    createScDoc("ods/tdf108188_pagestyle.ods");

    // Check if the user defined page style is present
    constexpr OUString aTestPageStyle = u"TestPageStyle"_ustr;
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(aTestPageStyle, pDoc->GetPageStyle(0));

    // Without the accompanying fix in place, the page styles are always used
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    CPPUNIT_ASSERT(pStylePool->Find(aTestPageStyle, SfxStyleFamily::Page)->IsUsed());
    CPPUNIT_ASSERT(
        !pStylePool->Find(ScResId(STR_STYLENAME_STANDARD), SfxStyleFamily::Page)->IsUsed());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
