/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../helper/qahelper.hxx"
#include <editeng/brushitem.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <svx/svdpage.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/scheduler.hxx>
#include <stlsheet.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <dbdata.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <inputopt.hxx>
#include <postit.hxx>
#include <rangeutl.hxx>
#include <scitems.hxx>
#include <scmod.hxx>
#include <tabvwsh.hxx>
#include <undomanager.hxx>
#include <viewdata.hxx>

using namespace ::com::sun::star;

class ScUiCalcTest2 : public ScModelTestBase
{
public:
    ScUiCalcTest2();
};

ScUiCalcTest2::ScUiCalcTest2()
    : ScModelTestBase("sc/qa/unit/uicalc/data")
{
}

static void lcl_AssertCurrentCursorPosition(ScDocShell& rDocSh, std::u16string_view rStr)
{
    ScAddress aAddr;
    sal_Int32 nOffset = 0;
    ScRangeStringConverter::GetAddressFromString(aAddr, rStr, rDocSh.GetDocument(),
                                                 formula::FormulaGrammar::CONV_OOO, nOffset);
    ScTabViewShell* pViewShell = rDocSh.GetBestViewShell(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OUString(OUString::Concat("Incorrect Column in position ") + rStr).toUtf8().getStr(),
        aAddr.Col(), pViewShell->GetViewData().GetCurX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OUString(OUString::Concat("Incorrect Row in position ") + rStr).toUtf8().getStr(),
        aAddr.Row(), pViewShell->GetViewData().GetCurY());
}

static void lcl_SelectObjectByName(ScTabViewShell& rViewShell, std::u16string_view rObjName)
{
    bool bFound = rViewShell.SelectObject(rObjName);
    CPPUNIT_ASSERT_MESSAGE(
        OString(OUStringToOString(rObjName, RTL_TEXTENCODING_UTF8) + " not found.").getStr(),
        bFound);

    CPPUNIT_ASSERT(rViewShell.GetViewData().GetScDrawView()->AreObjectsMarked());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf150499)
{
    createScDoc("tdf150499.xls");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf158254)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell("A:G");
    dispatchCommand(mxComponent,
                    ".uno:StyleApply?Style:string=Accent%201&FamilyName:string=CellStyles", {});

    const ScPatternAttr* pPattern = pDoc->GetPattern(5, 0, 0);
    ScStyleSheet* pStyleSheet = const_cast<ScStyleSheet*>(pPattern->GetStyleSheet());

    // Without the fix in place, this test would have failed with
    // - Expected: Accent 1
    // - Actual  : Default
    CPPUNIT_ASSERT_EQUAL(OUString("Accent 1"), pStyleSheet->GetName());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    pPattern = pDoc->GetPattern(5, 0, 0);
    pStyleSheet = const_cast<ScStyleSheet*>(pPattern->GetStyleSheet());
    CPPUNIT_ASSERT_EQUAL(OUString("Default"), pStyleSheet->GetName());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf125030)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell("A1");
    dispatchCommand(mxComponent,
                    ".uno:StyleApply?Style:string=Accent%201&FamilyName:string=CellStyles", {});

    const ScPatternAttr* pPatternA1 = pDoc->GetPattern(0, 0, 0);
    ScStyleSheet* pStyleSheetA1 = const_cast<ScStyleSheet*>(pPatternA1->GetStyleSheet());

    CPPUNIT_ASSERT_EQUAL(OUString("Accent 1"), pStyleSheetA1->GetName());

    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:Repeat", {});

    const ScPatternAttr* pPatternA2 = pDoc->GetPattern(0, 1, 0);
    ScStyleSheet* pStyleSheetA2 = const_cast<ScStyleSheet*>(pPatternA2->GetStyleSheet());

    CPPUNIT_ASSERT_EQUAL(OUString("Accent 1"), pStyleSheetA2->GetName());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf133326)
{
    createScDoc("tdf133326.ods");
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    insertNewSheet(*pDoc);

    OUString aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("=RAND()*1000000"), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Redo", {});

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("=RAND()*1000000"), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf126685)
{
    createScDoc("tdf126685.ods");

    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:SelectAll", {}); // test should crash here without the fix

    CPPUNIT_ASSERT_EQUAL(
        OUString("Control Height will change from 0.65 to 0.61 cm with 120dpi ..."),
        pDoc->GetString(ScAddress(3, 1, 1)));
    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(3, 1, 1)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf119793)
{
    createScDoc("tdf119793.ods");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    uno::Reference<drawing::XDrawPage> xPage(pModelObj->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4984), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    // Move the shape to the right
    lcl_SelectObjectByName(*getViewShell(), u"Shape 1");
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    Scheduler::ProcessEventsToIdle();

    //position has changed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5084), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    // Type into the shape
    typeString(u"x");
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5084), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    // Without the fix in place, this test would have failed with
    // - Expected: 4984
    // - Actual  : 5084
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4984), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    // tdf#141816: Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : x
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_EQUAL(OUString(""), xText->getString());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf131455)
{
    // Note that tdf#131455 and tdf#126904 were actually incorrect,
    // but keep the test with a fixed version of the document.
    createScDoc("tdf131455-fixed.ods");
    ScDocShell* pDocSh = getScDocShell();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"E5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"F5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"I5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"J5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"M5");

    //Cursor can't move forward to the right
    for (size_t i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:GoRight", {});
        lcl_AssertCurrentCursorPosition(*pDocSh, u"N5");
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getViewShell()->GetViewData().GetTabNo());

    dispatchCommand(mxComponent, ".uno:JumpToNextTable", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getViewShell()->GetViewData().GetTabNo());
    lcl_AssertCurrentCursorPosition(*pDocSh, u"A4");

    // Go to row 9
    for (size_t i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, ".uno:GoDown", {});
    }

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A10");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    dispatchCommand(mxComponent, ".uno:JumpToPrevTable", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getViewShell()->GetViewData().GetTabNo());
    lcl_AssertCurrentCursorPosition(*pDocSh, u"N5");

    //Cursor can't move forward to the right
    for (size_t i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:GoRight", {});
        lcl_AssertCurrentCursorPosition(*pDocSh, u"N5");
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124818)
{
    createScDoc("tdf124818.xls");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), getViewShell()->GetViewData().GetTabNo());

    dispatchCommand(mxComponent, ".uno:JumpToPrevTable", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getViewShell()->GetViewData().GetTabNo());

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124816)
{
    createScDoc("tdf124816.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    ScDocument* pDoc = getScDoc();

    // The actual result is completely unrelated to this test and behaviour of
    // OFFSET() was changed as of tdf#85551 and here result of that test
    // document is now Err:502 instead of 0.
    constexpr OUString aExpectedResult(u"Err:502"_ustr);
    lcl_AssertCurrentCursorPosition(*pDocSh, u"D10");
    CPPUNIT_ASSERT_EQUAL(aExpectedResult, pDoc->GetString(ScAddress(3, 9, 0)));

    //Without the fix, it would crash
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(3, 9, 0)));
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(aExpectedResult, pDoc->GetString(ScAddress(3, 9, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124815)
{
    createScDoc("tdf124815.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(2, 0, 0)));

    //Without the fix, it would crash
    dispatchCommand(mxComponent, ".uno:InsertColumnsBefore", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(3, 0, 0)));
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(2, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf142010)
{
    createScDoc("tdf142010.xls");
    ScDocument* pDoc = getScDoc();

    goToCell("A1");

    OUString aFormula = pDoc->GetFormula(5, 71, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=MOD(F$71+$C72,9)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(5, 71, 0)));

    dispatchCommand(mxComponent, ".uno:InsertColumnsBefore", {});

    aFormula = pDoc->GetFormula(6, 71, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: =MOD(G$71+$D72,9)
    // - Actual  : =MOD(G$71+$K72,9)
    CPPUNIT_ASSERT_EQUAL(OUString("=MOD(G$71+$D72,9)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(6, 71, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    aFormula = pDoc->GetFormula(5, 71, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=MOD(F$71+$C72,9)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(5, 71, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf154061)
{
    createScDoc("simpleTable.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(OUString("Column2"), pDoc->GetString(ScAddress(1, 0, 0)));

    goToCell("B1");

    // Without the fix in place, it would crash here due to an out of bounds array access
    dispatchCommand(mxComponent, ".uno:InsertColumnsBefore", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Column2"), pDoc->GetString(ScAddress(2, 0, 0)));
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Column2"), pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf132431)
{
    createScDoc("tdf132431.ods");
    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(7, 219, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIFS($H$2:$H$198,B$2:B$198,G220)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(7, 219, 0)));

    // Without the fix in place, it would crash here with
    // uncaught exception of type std::exception (or derived).
    // - vector::_M_fill_insert
    insertStringToCell("H220", u"=SUMIFS($H$2:$DB$198,B$2:B$198,G220)");

    aFormula = pDoc->GetFormula(7, 219, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIFS($H$2:$DB$198,B$2:B$198,G220)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(7, 219, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Err:502"), pDoc->GetString(ScAddress(7, 219, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf131073)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    for (SCCOLROW nColRow = 0; nColRow < 3; nColRow++)
    {
        pDoc->SetString(ScAddress(0, nColRow, 0), "Hello World");
        pDoc->SetRowHeight(0, nColRow, 1000 * (nColRow + 1));
        pDoc->SetString(ScAddress(nColRow, 0, 0), "Hello World");
        pDoc->SetColWidth(nColRow, 0, 1000 * (nColRow + 1));
    }

    // Check rows
    pDoc->SetRowHidden(1, 1, 0, true);
    goToCell("A1:A3");
    dispatchCommand(
        mxComponent, ".uno:SetOptimalRowHeight",
        comphelper::InitPropertySequence({ { "aExtraHeight", uno::Any(sal_uInt16(0)) } }));

    CPPUNIT_ASSERT(!pDoc->RowHidden(0, 0));
    // tdf#131073: Without the fix in place, the second row would not be hidden
    CPPUNIT_ASSERT(pDoc->RowHidden(1, 0));
    CPPUNIT_ASSERT(!pDoc->RowHidden(2, 0));
    const sal_uInt16 nStdRowHeight = pDoc->GetRowHeight(4, 0);
    CPPUNIT_ASSERT_EQUAL(nStdRowHeight, pDoc->GetRowHeight(0, 0));
    CPPUNIT_ASSERT_EQUAL(nStdRowHeight, pDoc->GetRowHeight(1, SCTAB(0), false));
    CPPUNIT_ASSERT_EQUAL(nStdRowHeight, pDoc->GetRowHeight(2, 0));

    // Check columns
    pDoc->SetColHidden(1, 1, 0, true);
    goToCell("A1:C1");
    dispatchCommand(
        mxComponent, ".uno:SetOptimalColumnWidth",
        comphelper::InitPropertySequence({ { "aExtraWidth", uno::Any(sal_uInt16(0)) } }));

    CPPUNIT_ASSERT(!pDoc->ColHidden(0, 0));
    // tdf#131073: Without the fix in place, the second column would not be hidden
    CPPUNIT_ASSERT(pDoc->ColHidden(1, 0));
    CPPUNIT_ASSERT(!pDoc->ColHidden(2, 0));
    const sal_uInt16 nStdColWidth = pDoc->GetColWidth(0, 0);
    CPPUNIT_ASSERT_EQUAL(nStdColWidth, pDoc->GetColWidth(0, 0));
    CPPUNIT_ASSERT_EQUAL(nStdColWidth, pDoc->GetColWidth(1, SCTAB(0), false));
    CPPUNIT_ASSERT_EQUAL(nStdColWidth, pDoc->GetColWidth(2, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf159938)
{
    createScDoc("tdf159938.fods");
    ScDocument* pDoc = getScDoc();

    const sal_uInt16 nCol1Width = pDoc->GetColWidth(0, 0);
    const sal_uInt16 nCol2Width = pDoc->GetColWidth(1, 0);
    const sal_uInt16 nRow1Height = pDoc->GetRowHeight(0, 0);
    const sal_uInt16 nRow2Height = pDoc->GetRowHeight(1, 0);
    const sal_uInt16 nRow3Height = pDoc->GetRowHeight(2, 0);

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});

    ScDocShell* pDocSh = getScDocShell();
    lcl_AssertCurrentCursorPosition(*pDocSh, u"A4");
    CPPUNIT_ASSERT_EQUAL(nCol1Width, pDoc->GetColWidth(0, 0));
    CPPUNIT_ASSERT_EQUAL(nCol2Width, pDoc->GetColWidth(1, 0));
    CPPUNIT_ASSERT_EQUAL(nRow1Height, pDoc->GetRowHeight(0, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: 283
    // - Actual  : 1552
    CPPUNIT_ASSERT_EQUAL(nRow2Height, pDoc->GetRowHeight(1, 0));
    CPPUNIT_ASSERT_EQUAL(nRow3Height, pDoc->GetRowHeight(2, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf83901)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    insertStringToCell("A2", u"=ROW(A3)");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A3");
    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});

    //Without the fix, it would be 3.0
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124822)
{
    createScDoc("tdf124822.xls");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("X"), pDoc->GetString(ScAddress(0, 0, 2)));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 2)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("X"), pDoc->GetString(ScAddress(0, 0, 2)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf118189)
{
    createScDoc("tdf118189.xlsx");

    ScDocument* pDoc = getScDoc();

    // Select column A
    goToCell("A:A");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:Paste", {});

    OUString aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    dispatchCommand(mxComponent, ".uno:Cut", {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf118207)
{
    createScDoc("tdf118189.xlsx");

    ScDocument* pDoc = getScDoc();

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    // Select column A
    goToCell("A:A");

    OUString aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    dispatchCommand(mxComponent, ".uno:Cut", {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    // Select column B
    goToCell("B:B");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf148669)
{
    // Without the fix in place, this test would have failed with an assert
    createScDoc("tdf148669.xlsx");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be a note", pDoc->HasNote(ScAddress(701, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124778)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    // Add a new comment
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Comment")) } });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:ShowAnnotations", {});

    ScPostIt* pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(true, pNote->IsCaptionShown());

    dispatchCommand(mxComponent, ".uno:ShowAnnotations", {});

    CPPUNIT_ASSERT_EQUAL(false, pNote->IsCaptionShown());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf138428)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    // Add a new comment
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Comment")) } });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Copy", {});

    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B1");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on B1", pDoc->HasNote(ScAddress(1, 0, 0)));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on A1", !pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on B1", pDoc->HasNote(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf136113)
{
    createScDoc("tdf136113.xlsx");
    ScDocument* pDoc = getScDoc();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    SdrObject* pObj = pPage->GetObj(0);

    CPPUNIT_ASSERT_EQUAL(tools::Long(18142), pObj->GetSnapRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1709), pObj->GetSnapRect().Top());

    lcl_SelectObjectByName(*getViewShell(), u"Arrow");

    // Move the shape up
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::UP);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::UP);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(tools::Long(18142), pObj->GetSnapRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1609), pObj->GetSnapRect().Top());

    // Without the fix in place, this test would have failed here
    ScDocShell* pDocSh = getScDocShell();
    CPPUNIT_ASSERT(pDocSh->IsModified());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf130614)
{
    createScDoc("tdf130614.ods");
    ScDocument* pDoc = getScDoc();

    lcl_SelectObjectByName(*getViewShell(), u"Object 1");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf112735)
{
    createScDoc("tdf112735.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("(empty)"), pDoc->GetString(ScAddress(1, 0, 0)));

    goToCell("B3");

    dispatchCommand(mxComponent, ".uno:RecalcPivotTable", {});

    // Without the fix in place, this test would have failed with
    // - Expected: (empty)
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("(empty)"), pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf112884)
{
    createScDoc("tdf112884.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("0.5"), pDoc->GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.666666666666667"), pDoc->GetString(ScAddress(6, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.833333333333333"), pDoc->GetString(ScAddress(6, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#DIV/0!"), pDoc->GetString(ScAddress(6, 5, 0)));

    goToCell("G3");

    dispatchCommand(mxComponent, ".uno:RecalcPivotTable", {});

    // Without the fix in place, this test would have failed with
    // - Expected: #DIV/0!
    // - Actual  : 0.5
    CPPUNIT_ASSERT_EQUAL(OUString("#DIV/0!"), pDoc->GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.5"), pDoc->GetString(ScAddress(6, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.666666666666667"), pDoc->GetString(ScAddress(6, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.833333333333333"), pDoc->GetString(ScAddress(6, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf133342)
{
    createScDoc("tdf133342.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("12,35 %"), pDoc->GetString(ScAddress(0, 0, 0)));
    //Add decimals
    dispatchCommand(mxComponent, ".uno:NumberFormatIncDecimals", {});
    //Space should preserved before percent sign
    CPPUNIT_ASSERT_EQUAL(OUString("12,346 %"), pDoc->GetString(ScAddress(0, 0, 0)));

    //Delete decimals
    dispatchCommand(mxComponent, ".uno:NumberFormatDecDecimals", {});
    dispatchCommand(mxComponent, ".uno:NumberFormatDecDecimals", {});
    dispatchCommand(mxComponent, ".uno:NumberFormatDecDecimals", {});
    //Space should preserved before percent sign
    CPPUNIT_ASSERT_EQUAL(OUString("12 %"), pDoc->GetString(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, ".uno:NumberFormatDecDecimals", {});
    //Space should preserved before percent sign
    CPPUNIT_ASSERT_EQUAL(OUString("12 %"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf71339)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A2", u"1");
    insertStringToCell("A3", u"1");

    goToCell("A1:A3");

    executeAutoSum();

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    OUString aFormula = pDoc->GetFormula(0, 3, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: =SUM(A1:A3)
    // - Actual  : =SUM(A2:A3)
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:A3)"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf116421)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"1");
    insertStringToCell("A3", u"1");

    goToCell("A4");

    executeAutoSum();

    // Without the fix in place, this test would have failed with
    // - Expected: 3
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    OUString aFormula = pDoc->GetFormula(0, 3, 0);

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:A3)"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf86305)
{
    createScDoc("tdf86305.ods");
    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(1, 6, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("{=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(1, 6, 0)));

    insertStringToCell("B3", u"50");
    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(1, 2, 0)));

    aFormula = pDoc->GetFormula(1, 6, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: {=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}
    // - Actual  : {=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0.175)}
    CPPUNIT_ASSERT_EQUAL(OUString("{=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.175, pDoc->GetValue(ScAddress(1, 6, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf81351)
{
    createScDoc("tdf81351.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 4, 0)));

    goToCell("A1:F5");

    dispatchCommand(mxComponent, ".uno:SortAscending", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});

    // Without the fix in place, this test would have crashed
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 4, 0)));

    goToCell("A1:F5");

    dispatchCommand(mxComponent, ".uno:SortDescending", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});

    // Without the fix in place, this test would have crashed
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Undo"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Save"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Paste"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(".uno:Bold"), pDoc->GetString(ScAddress(0, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf123202)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell(u"A1"_ustr, u"1");
    insertStringToCell(u"A2"_ustr, u"2");
    insertStringToCell(u"A3"_ustr, u"3");
    insertStringToCell(u"A4"_ustr, u"4");

    goToCell("A3");

    dispatchCommand(mxComponent, ".uno:HideRow", {});

    goToCell("A1:A4");

    dispatchCommand(mxComponent, ".uno:SortDescending", {});

    CPPUNIT_ASSERT_EQUAL(OUString("4"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(0, 3, 0)));

    // This failed, if the "3" is visible.
    CPPUNIT_ASSERT(pDoc->RowHidden(1, 0));
    CPPUNIT_ASSERT(!pDoc->RowHidden(2, 0));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("4"), pDoc->GetString(ScAddress(0, 3, 0)));

    CPPUNIT_ASSERT(!pDoc->RowHidden(1, 0));
    CPPUNIT_ASSERT(pDoc->RowHidden(2, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf134675)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    insertStringToCell("A1", u"A");

    // Select column A
    goToCell("A:A");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Select column B to Z
    goToCell("B:Z");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    for (size_t i = 1; i < 24; ++i)
    {
        // Without the fix in place, this test would have failed here with
        // - Expected: A
        // - Actual  :
        CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(i, 0, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf116215)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"1");
    insertStringToCell("B1", u"1");
    insertStringToCell("B2", u"1");
    goToCell("A1:C3");

    executeAutoSum();

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 2, 0)));
    OUString aFormula = pDoc->GetFormula(0, 2, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:A2)"), aFormula);

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 4
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 2, 0)));
    aFormula = pDoc->GetFormula(1, 2, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B1:B2)"), aFormula);

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(2, 0, 0)));
    aFormula = pDoc->GetFormula(2, 0, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:B1)"), aFormula);

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(2, 1, 0)));
    aFormula = pDoc->GetFormula(2, 1, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A2:B2)"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf99913)
{
    createScDoc("tdf99913.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT(pDoc->RowFiltered(2, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf126540_GridToggleModifiesTheDocument)
{
    createScDoc("tdf99913.xlsx");

    // Toggling the grid of a sheet, must set the document modified state
    ScDocShell* pDocSh = getScDocShell();
    CPPUNIT_ASSERT(!pDocSh->IsModified());
    dispatchCommand(mxComponent, ".uno:ToggleSheetGrid", {});
    CPPUNIT_ASSERT(pDocSh->IsModified());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf118983)
{
    createScDoc("tdf118983.ods");
    ScDocument* pDoc = getScDoc();

    css::uno::Reference<css::sheet::XGlobalSheetSettings> xGlobalSheetSettings
        = css::sheet::GlobalSheetSettings::create(::comphelper::getProcessComponentContext());
    bool bOldValue = xGlobalSheetSettings->getExpandReferences();

    xGlobalSheetSettings->setExpandReferences(true);

    const ScRangeData* pRD = pDoc->GetRangeName()->findByUpperName("TEST");
    CPPUNIT_ASSERT(pRD);
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$A$3:$D$7"), pRD->GetSymbol());

    //Select columns E to G
    goToCell("E:G");

    dispatchCommand(mxComponent, ".uno:InsertColumnsBefore", {});

    // Without the fix in place, this test would have failed with
    // - Expected: $Test.$A$3:$D$7
    // - Actual  : $Test.$A$3:$G$7
    CPPUNIT_ASSERT_EQUAL(OUString("$Test.$A$3:$D$7"), pRD->GetSymbol());

    xGlobalSheetSettings->setExpandReferences(bOldValue);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf107952)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    insertStringToCell("B1", u"=SUM(A1:A2)");

    goToCell("D10");

    dispatchCommand(mxComponent, ".uno:Undo", {});

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 3
    // - Incorrect Column in position B1
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B1");

    goToCell("D10");

    dispatchCommand(mxComponent, ".uno:Redo", {});

    lcl_AssertCurrentCursorPosition(*pDocSh, u"B1");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf150766)
{
    createScDoc("tdf150766.ods");
    ScDocument* pDoc = getScDoc();

    goToCell("A3:C6");

    dispatchCommand(mxComponent, ".uno:SortDescending", {});

    insertStringToCell("B3", u"10");

    CPPUNIT_ASSERT_EQUAL(12.0, pDoc->GetValue(ScAddress(2, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(13.0, pDoc->GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(14.0, pDoc->GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(15.0, pDoc->GetValue(ScAddress(2, 5, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: 16
    // - Actual  : 7
    CPPUNIT_ASSERT_EQUAL(16.0, pDoc->GetValue(ScAddress(2, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(17.0, pDoc->GetValue(ScAddress(2, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(18.0, pDoc->GetValue(ScAddress(2, 8, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf144022)
{
    createScDoc("tdf144022.ods");
    ScDocument* pDoc = getScDoc();

    goToCell("A5:B79");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("D5");

    //Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:PasteTransposed", {});

    for (size_t i = 3; i < 76; ++i)
    {
        OUString sExpected = "A" + OUString::number(i + 2);
        CPPUNIT_ASSERT_EQUAL(sExpected, pDoc->GetString(ScAddress(i, 4, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf99386)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("B1", u"This");
    insertStringToCell("B2", u"=B1");

    goToCell("A1:B1");

    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});

    CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(1, 1, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("This"), pDoc->GetString(ScAddress(1, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: This
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(OUString("This"), pDoc->GetString(ScAddress(1, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf149378)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"=MINVERSE(A1:C3)");

    // Without the fix in place, this test would have failed with
    // - Expected: {=MINVERSE(A1:C3)}
    // - Actual  : =MINVERSE(A1:C3)
    CPPUNIT_ASSERT_EQUAL(OUString("{=MINVERSE(A1:C3)}"), pDoc->GetFormula(0, 0, 0));

    insertStringToCell("B1", u"={1;2}");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}}"), pDoc->GetFormula(1, 0, 0));

    insertStringToCell("C1", u"={1;2}+3");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}+3}"), pDoc->GetFormula(2, 0, 0));

    insertStringToCell("D1", u"={1;2}+{3;4}");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}+{3;4}}"), pDoc->GetFormula(3, 0, 0));

    insertStringToCell("E1", u"={1;2}+A1");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}+A1}"), pDoc->GetFormula(4, 0, 0));

    insertStringToCell("F1", u"={1;2}+A1:A2");
    CPPUNIT_ASSERT_EQUAL(OUString("={1;2}+A1:A2"), pDoc->GetFormula(5, 0, 0));

    insertStringToCell("G1", u"=SUM(MUNIT(3))");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(MUNIT(3))"), pDoc->GetFormula(6, 0, 0));

    insertStringToCell("H1", u"=SUM({1;2})");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM({1;2})"), pDoc->GetFormula(7, 0, 0));

    insertStringToCell("I1", u"=ABS({-1;-2})");
    CPPUNIT_ASSERT_EQUAL(OUString("{=ABS({-1;-2})}"), pDoc->GetFormula(8, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf152014)
{
    createScDoc();

    insertStringToCell("A1", u"=MATCH(1,A2,0)");
    insertStringToCell("A2", u"1");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(0, 0, 0)));

    goToCell("A1");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Create a second document
    mxComponent2 = loadFromDesktop("private:factory/scalc");

    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xFrames->getCount());

    dispatchCommand(mxComponent2, ".uno:Paste", {});

    ScModelObj* pModelObj2 = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent2);
    CPPUNIT_ASSERT(pModelObj2);
    ScDocument* pDoc2 = pModelObj2->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: #N/A
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), pDoc2->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf156286)
{
    createScDoc("tdf156286.ods");
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    // Without the fix in place, this test would have crash here
    dispatchCommand(mxComponent, ".uno:DeleteColumns", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(12, 2, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("xxxxxxxxxxxx"), pDoc->GetString(ScAddress(12, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf126926)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"2");
    insertStringToCell("B1", u"3");
    insertStringToCell("B2", u"4");

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 1, 1);
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    goToCell("A1:B1");

    dispatchCommand(mxComponent, ".uno:DeleteColumns", {});

    ScDBCollection* pDBs = pDoc->GetDBCollection();
    CPPUNIT_ASSERT(pDBs->empty());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testUnallocatedColumnsAttributes)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(INITIALCOLCOUNT, pDoc->GetAllocatedColumnsCount(0));

    // Except for first 1 cell make the entire first row bold.
    assert(INITIALCOLCOUNT >= 1);
    goToCell("B1:" + pDoc->MaxColAsString() + "1");
    dispatchCommand(mxComponent, ".uno:Bold", {});

    // That shouldn't need allocating more columns, just changing the default attribute.
    CPPUNIT_ASSERT_EQUAL(INITIALCOLCOUNT, pDoc->GetAllocatedColumnsCount(0));
    vcl::Font aFont;
    pDoc->GetPattern(pDoc->MaxCol(), 0, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());

    goToCell("A2:CV2"); // first 100 cells in row 2
    dispatchCommand(mxComponent, ".uno:Bold", {});
    // These need to be explicitly allocated.
    CPPUNIT_ASSERT_EQUAL(SCCOL(100), pDoc->GetAllocatedColumnsCount(0));
    pDoc->GetPattern(99, 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
    pDoc->GetPattern(100, 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should not be bold", WEIGHT_NORMAL, aFont.GetWeight());

    goToCell("CW3:" + pDoc->MaxColAsString() + "3"); // All but first 100 cells in row 3.
    dispatchCommand(mxComponent, ".uno:Bold", {});
    // First 100 columns need to be allocated to not be bold, the rest should be handled
    // by the default attribute.
    CPPUNIT_ASSERT_EQUAL(SCCOL(100), pDoc->GetAllocatedColumnsCount(0));
    pDoc->GetPattern(99, 2, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should not be bold", WEIGHT_NORMAL, aFont.GetWeight());
    pDoc->GetPattern(100, 2, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testAutoSum)
{
    createScDoc("autosum.ods");
    ScDocument* pDoc = getScDoc();

    //Sum on range and Sum on Sum's
    goToCell("B10");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 9, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B8:B9)"), pDoc->GetFormula(1, 9, 0));

    goToCell("B13");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 12, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B11:B12)"), pDoc->GetFormula(1, 12, 0));

    goToCell("B14");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(1, 13, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B13:B13,B10:B10)"), pDoc->GetFormula(1, 13, 0));

    goToCell("F8:F14");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(5, 13, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(F13:F13,F10:F10)"), pDoc->GetFormula(5, 13, 0));

    //Sum on Row and Column
    goToCell("E25");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 24, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(E22:E24)"), pDoc->GetFormula(4, 24, 0));

    goToCell("E26");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 25, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B26:D26)"), pDoc->GetFormula(4, 25, 0));

    goToCell("E27");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 26, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B27:D27)"), pDoc->GetFormula(4, 26, 0));

    goToCell("E28");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 27, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B28:D28)"), pDoc->GetFormula(4, 27, 0));

    goToCell("E29");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(9.0, pDoc->GetValue(ScAddress(4, 28, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(E26:E28)"), pDoc->GetFormula(4, 28, 0));

    goToCell("E30");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(12.0, pDoc->GetValue(ScAddress(4, 29, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(E29:E29,E25:E25)"), pDoc->GetFormula(4, 29, 0));

    //Subtotals on Autosum
    goToCell("C49");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(20.0, pDoc->GetValue(ScAddress(2, 48, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUBTOTAL(9,C38:C48)"), pDoc->GetFormula(2, 48, 0));

    //Autosum on column with selected empty cell for result
    goToCell("B59:B64");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 63, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B59:B63)"), pDoc->GetFormula(1, 63, 0));

    //Autosum on rows with selected empty cell for result
    goToCell("B76:E80");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(4, 75, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B76:D76)"), pDoc->GetFormula(4, 75, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, pDoc->GetValue(ScAddress(4, 76, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B77:D77)"), pDoc->GetFormula(4, 76, 0));
    CPPUNIT_ASSERT_EQUAL(90.0, pDoc->GetValue(ScAddress(4, 77, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B78:D78)"), pDoc->GetFormula(4, 77, 0));
    CPPUNIT_ASSERT_EQUAL(120.0, pDoc->GetValue(ScAddress(4, 78, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B79:D79)"), pDoc->GetFormula(4, 78, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(4, 79, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B80:D80)"), pDoc->GetFormula(4, 79, 0));

    //Subtotal on column with selected empty cell for result
    goToCell("C92:C101");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(19.0, pDoc->GetValue(ScAddress(2, 100, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUBTOTAL(9,C92:C100)"), pDoc->GetFormula(2, 100, 0));

    //Autosum on column without selected empty cell for result
    goToCell("B109:B113");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 113, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B109:B113)"), pDoc->GetFormula(1, 113, 0));

    //Subtotal on column without selected empty cell for result
    goToCell("C142:C149");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(19.0, pDoc->GetValue(ScAddress(2, 150, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUBTOTAL(9,C142:C149)"), pDoc->GetFormula(2, 150, 0));

    //Autosum on multiselected columns without selected empty cell for result
    goToCell("B160:D164");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 164, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B160:B164)"), pDoc->GetFormula(1, 164, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(2, 164, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C160:C164)"), pDoc->GetFormula(2, 164, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(3, 164, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(D160:D164)"), pDoc->GetFormula(3, 164, 0));

    //Autosum on columns with formula results without selected empty cell for result
    goToCell("B173:D177");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(2.54, std::floor(pDoc->GetValue(ScAddress(1, 177, 0)) * 100.0) / 100.0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B173:B177)"), pDoc->GetFormula(1, 177, 0));
    CPPUNIT_ASSERT_EQUAL(-4.91, std::floor(pDoc->GetValue(ScAddress(2, 177, 0)) * 100.0) / 100.0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C173:C177)"), pDoc->GetFormula(2, 177, 0));
    CPPUNIT_ASSERT_EQUAL(5500.0, pDoc->GetValue(ScAddress(3, 177, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(D173:D177)"), pDoc->GetFormula(3, 177, 0));

    //Autosum on column with filled cell under selected area
    goToCell("B186:D190");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 191, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B186:B190)"), pDoc->GetFormula(1, 191, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(2, 191, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C186:C190)"), pDoc->GetFormula(2, 191, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(3, 191, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(D186:D190)"), pDoc->GetFormula(3, 191, 0));

    //Autosum on column and rows with empty cells selected for row and column
    goToCell("B203:E208");
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B203:B207)"), pDoc->GetFormula(1, 207, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(2, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C203:C207)"), pDoc->GetFormula(2, 207, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(3, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(D203:D207)"), pDoc->GetFormula(3, 207, 0));
    CPPUNIT_ASSERT_EQUAL(450.0, pDoc->GetValue(ScAddress(4, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B208:D208)"), pDoc->GetFormula(4, 207, 0));
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(4, 202, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B203:D203)"), pDoc->GetFormula(4, 202, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, pDoc->GetValue(ScAddress(4, 203, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B204:D204)"), pDoc->GetFormula(4, 203, 0));
    CPPUNIT_ASSERT_EQUAL(90.0, pDoc->GetValue(ScAddress(4, 204, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B205:D205)"), pDoc->GetFormula(4, 204, 0));
    CPPUNIT_ASSERT_EQUAL(120.0, pDoc->GetValue(ScAddress(4, 205, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B206:D206)"), pDoc->GetFormula(4, 205, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(4, 206, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B207:D207)"), pDoc->GetFormula(4, 206, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf152577)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"2");
    insertStringToCell("B1", u"3");
    insertStringToCell("B2", u"4");

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 1, 1);
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    insertNewSheet(*pDoc);
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(2)) } }));
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    ScDBCollection* pDBs = pDoc->GetDBCollection();
    CPPUNIT_ASSERT(!pDBs->empty());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf155796)
{
    createScDoc();

    goToCell("A1:A3");
    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});
    goToCell("A4:A6");
    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});

    goToCell("A1:A6");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_UP);
    Scheduler::ProcessEventsToIdle();

    ScRangeList aMarkedArea = getViewShell()->GetViewData().GetMarkData().GetMarkedRanges();
    ScDocument* pDoc = getScDoc();
    OUString aMarkedAreaString;
    ScRangeStringConverter::GetStringFromRangeList(aMarkedAreaString, &aMarkedArea, pDoc,
                                                   formula::FormulaGrammar::CONV_OOO);

    // Without the fix in place, this test would have failed with
    // - Expected: Sheet1.A1:Sheet1.A3
    // - Actual  : Sheet1.A1:Sheet1.A5
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1.A1:Sheet1.A3"), aMarkedAreaString);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf156174)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertNewSheet(*pDoc);

    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"2");
    insertStringToCell("B1", u"3");
    insertStringToCell("B2", u"4");

    ScDBData* pDBData = new ScDBData("testDB", 1, 0, 0, 1, 1);
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    insertNewSheet(*pDoc);
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(3)) } }));
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    ScDBCollection* pDBs = pDoc->GetDBCollection();
    CPPUNIT_ASSERT(!pDBs->empty());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf154044)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    auto getBackColor = [pDoc](SCCOL c) {
        const ScPatternAttr* pattern = pDoc->GetPattern(c, 0, 0);
        const SvxBrushItem& brush = pattern->GetItemSet().Get(ATTR_BACKGROUND);
        return brush.GetColor();
    };

    CPPUNIT_ASSERT_EQUAL(INITIALCOLCOUNT, pDoc->GetAllocatedColumnsCount(0));
    for (SCCOL i = 0; i <= pDoc->MaxCol(); ++i)
    {
        OString msg = "i=" + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), COL_AUTO, getBackColor(i));
    }

    // Set the background color of A1:CV1
    auto aColorArg(
        comphelper::InitPropertySequence({ { "BackgroundColor", uno::Any(COL_LIGHTBLUE) } }));
    goToCell("A1:CV1");
    dispatchCommand(mxComponent, ".uno:BackgroundColor", aColorArg);

    // Partial row range allocates necessary columns
    CPPUNIT_ASSERT_EQUAL(SCCOL(100), pDoc->GetAllocatedColumnsCount(0));

    // Check that settings are applied
    for (SCCOL i = 0; i < 100; ++i)
    {
        OString msg = "i=" + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), COL_LIGHTBLUE, getBackColor(i));
    }

    // Undo
    SfxUndoManager* pUndoMgr = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    // Check that all the cells have restored the setting
    for (SCCOL i = 0; i < 100; ++i)
    {
        OString msg = "i=" + OString::number(i);
        // Without the fix in place, this would fail with
        // - Expected: rgba[ffffff00]
        // - Actual  : rgba[0000ffff]
        // - i=1
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), COL_AUTO, getBackColor(i));
    }

    // Also check the whole row selection case - it is handled specially: columns are not allocated.
    // See commit 3db91487e57277f75d64d95d06d4ddcc29f1c4e0 (set properly attributes for cells in
    // unallocated Calc columns, 2022-03-04).
    goToCell("A1:" + pDoc->MaxColAsString() + "1");
    dispatchCommand(mxComponent, ".uno:BackgroundColor", aColorArg);

    // Check that settings are applied
    for (SCCOL i = 0; i <= pDoc->MaxCol(); ++i)
    {
        OString msg = "i=" + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), COL_LIGHTBLUE, getBackColor(i));
    }

    // Undo
    pUndoMgr->Undo();

    // No additional columns have been allocated for whole-row range
    CPPUNIT_ASSERT_EQUAL(SCCOL(100), pDoc->GetAllocatedColumnsCount(0));

    // Check that all the cells have restored the setting
    for (SCCOL i = 0; i <= pDoc->MaxCol(); ++i)
    {
        OString msg = "i=" + OString::number(i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), COL_AUTO, getBackColor(i));
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
