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
    : ScModelTestBase(u"sc/qa/unit/uicalc/data"_ustr)
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

    CPPUNIT_ASSERT(rViewShell.GetViewData().GetScDrawView()->GetMarkedObjectList().GetMarkCount()
                   != 0);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf150499)
{
    createScDoc("tdf150499.xls");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Remove"_ustr, aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf158254)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell(u"A:G"_ustr);
    dispatchCommand(mxComponent,
                    u".uno:StyleApply?Style:string=Accent%201&FamilyName:string=CellStyles"_ustr,
                    {});

    const ScPatternAttr* pPattern = pDoc->GetPattern(5, 0, 0);
    ScStyleSheet* pStyleSheet = const_cast<ScStyleSheet*>(pPattern->GetStyleSheet());

    // Without the fix in place, this test would have failed with
    // - Expected: Accent 1
    // - Actual  : Default
    CPPUNIT_ASSERT_EQUAL(u"Accent 1"_ustr, pStyleSheet->GetName());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    pPattern = pDoc->GetPattern(5, 0, 0);
    pStyleSheet = const_cast<ScStyleSheet*>(pPattern->GetStyleSheet());
    CPPUNIT_ASSERT_EQUAL(u"Default"_ustr, pStyleSheet->GetName());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf125030)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell(u"A1"_ustr);
    dispatchCommand(mxComponent,
                    u".uno:StyleApply?Style:string=Accent%201&FamilyName:string=CellStyles"_ustr,
                    {});

    const ScPatternAttr* pPatternA1 = pDoc->GetPattern(0, 0, 0);
    ScStyleSheet* pStyleSheetA1 = const_cast<ScStyleSheet*>(pPatternA1->GetStyleSheet());

    CPPUNIT_ASSERT_EQUAL(u"Accent 1"_ustr, pStyleSheetA1->GetName());

    goToCell(u"A2"_ustr);
    dispatchCommand(mxComponent, u".uno:Repeat"_ustr, {});

    const ScPatternAttr* pPatternA2 = pDoc->GetPattern(0, 1, 0);
    ScStyleSheet* pStyleSheetA2 = const_cast<ScStyleSheet*>(pPatternA2->GetStyleSheet());

    CPPUNIT_ASSERT_EQUAL(u"Accent 1"_ustr, pStyleSheetA2->GetName());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf133326)
{
    createScDoc("tdf133326.ods");
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    insertNewSheet(*pDoc);

    OUString aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aFormula);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(u"=RAND()*1000000"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aFormula);

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(u"=RAND()*1000000"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf126685)
{
    createScDoc("tdf126685.ods");

    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr,
                    {}); // test should crash here without the fix

    CPPUNIT_ASSERT_EQUAL(u"Control Height will change from 0.65 to 0.61 cm with 120dpi ..."_ustr,
                         pDoc->GetString(ScAddress(3, 1, 1)));
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(3, 1, 1)));
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

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5084), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xText->getString());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf131455)
{
    // Note that tdf#131455 and tdf#126904 were actually incorrect,
    // but keep the test with a fixed version of the document.
    createScDoc("tdf131455-fixed.ods");
    ScDocShell* pDocSh = getScDocShell();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A5");
    dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B5");
    dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"E5");
    dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"F5");
    dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"I5");
    dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"J5");
    dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"M5");

    //Cursor can't move forward to the right
    for (size_t i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
        lcl_AssertCurrentCursorPosition(*pDocSh, u"N5");
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getViewShell()->GetViewData().GetTabNo());

    dispatchCommand(mxComponent, u".uno:JumpToNextTable"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getViewShell()->GetViewData().GetTabNo());
    lcl_AssertCurrentCursorPosition(*pDocSh, u"A4");

    // Go to row 9
    for (size_t i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    }

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A10");

    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, {});
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    dispatchCommand(mxComponent, u".uno:JumpToPrevTable"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getViewShell()->GetViewData().GetTabNo());
    lcl_AssertCurrentCursorPosition(*pDocSh, u"N5");

    //Cursor can't move forward to the right
    for (size_t i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
        lcl_AssertCurrentCursorPosition(*pDocSh, u"N5");
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124818)
{
    createScDoc("tdf124818.xls");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), getViewShell()->GetViewData().GetTabNo());

    dispatchCommand(mxComponent, u".uno:JumpToPrevTable"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getViewShell()->GetViewData().GetTabNo());

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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
    dispatchCommand(mxComponent, u".uno:InsertRowsBefore"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(3, 9, 0)));
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(aExpectedResult, pDoc->GetString(ScAddress(3, 9, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124815)
{
    createScDoc("tdf124815.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(u"Rakennukset"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));

    //Without the fix, it would crash
    dispatchCommand(mxComponent, u".uno:InsertColumnsBefore"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Rakennukset"_ustr, pDoc->GetString(ScAddress(3, 0, 0)));
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Rakennukset"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf142010)
{
    createScDoc("tdf142010.xls");
    ScDocument* pDoc = getScDoc();

    goToCell(u"A1"_ustr);

    OUString aFormula = pDoc->GetFormula(5, 71, 0);
    CPPUNIT_ASSERT_EQUAL(u"=MOD(F$71+$C72,9)"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(5, 71, 0)));

    dispatchCommand(mxComponent, u".uno:InsertColumnsBefore"_ustr, {});

    aFormula = pDoc->GetFormula(6, 71, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: =MOD(G$71+$D72,9)
    // - Actual  : =MOD(G$71+$K72,9)
    CPPUNIT_ASSERT_EQUAL(u"=MOD(G$71+$D72,9)"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(6, 71, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    aFormula = pDoc->GetFormula(5, 71, 0);
    CPPUNIT_ASSERT_EQUAL(u"=MOD(F$71+$C72,9)"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(5.0, pDoc->GetValue(ScAddress(5, 71, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf154061)
{
    createScDoc("simpleTable.xlsx");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));

    goToCell(u"B1"_ustr);

    // Without the fix in place, it would crash here due to an out of bounds array access
    dispatchCommand(mxComponent, u".uno:InsertColumnsBefore"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Column2"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf132431)
{
    createScDoc("tdf132431.ods");
    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(7, 219, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUMIFS($H$2:$H$198,B$2:B$198,G220)"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(7, 219, 0)));

    // Without the fix in place, it would crash here with
    // uncaught exception of type std::exception (or derived).
    // - vector::_M_fill_insert
    insertStringToCell(u"H220"_ustr, u"=SUMIFS($H$2:$DB$198,B$2:B$198,G220)");

    aFormula = pDoc->GetFormula(7, 219, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUMIFS($H$2:$DB$198,B$2:B$198,G220)"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(7, 219, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Err:502"_ustr, pDoc->GetString(ScAddress(7, 219, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf131073)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    for (SCCOLROW nColRow = 0; nColRow < 3; nColRow++)
    {
        pDoc->SetString(ScAddress(0, nColRow, 0), u"Hello World"_ustr);
        pDoc->SetRowHeight(0, nColRow, 1000 * (nColRow + 1));
        pDoc->SetString(ScAddress(nColRow, 0, 0), u"Hello World"_ustr);
        pDoc->SetColWidth(nColRow, 0, 1000 * (nColRow + 1));
    }

    // Check rows
    pDoc->SetRowHidden(1, 1, 0, true);
    goToCell(u"A1:A3"_ustr);
    dispatchCommand(
        mxComponent, u".uno:SetOptimalRowHeight"_ustr,
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
    goToCell(u"A1:C1"_ustr);
    dispatchCommand(
        mxComponent, u".uno:SetOptimalColumnWidth"_ustr,
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

    goToCell(u"A1"_ustr);
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

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

    insertStringToCell(u"A2"_ustr, u"=ROW(A3)");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A3");
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertRowsBefore"_ustr, {});

    //Without the fix, it would be 3.0
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf124822)
{
    createScDoc("tdf124822.xls");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"X"_ustr, pDoc->GetString(ScAddress(0, 0, 2)));

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(0, 0, 2)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"X"_ustr, pDoc->GetString(ScAddress(0, 0, 2)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf118189)
{
    createScDoc("tdf118189.xlsx");

    ScDocument* pDoc = getScDoc();

    // Select column A
    goToCell(u"A:A"_ustr);

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    OUString aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u"=FALSE()"_ustr, aFormula);

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aFormula);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u"=FALSE()"_ustr, aFormula);
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
    goToCell(u"A:A"_ustr);

    OUString aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u"=FALSE()"_ustr, aFormula);

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aFormula);

    // Select column B
    goToCell(u"B:B"_ustr);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u"=FALSE()"_ustr, aFormula);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u"=FALSE()"_ustr, aFormula);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u"=FALSE()"_ustr, aFormula);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aFormula);
    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, aFormula);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(u"=FALSE()"_ustr, aFormula);

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
        = comphelper::InitPropertySequence({ { "Text", uno::Any(u"Comment"_ustr) } });
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aArgs);

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, u".uno:ShowAnnotations"_ustr, {});

    ScPostIt* pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(true, pNote->IsCaptionShown());

    dispatchCommand(mxComponent, u".uno:ShowAnnotations"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(false, pNote->IsCaptionShown());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf138428)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    // Add a new comment
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(u"Comment"_ustr) } });
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aArgs);

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:GoRight"_ustr, {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B1");

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on B1", pDoc->HasNote(ScAddress(1, 0, 0)));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on A1", !pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

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

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf112735)
{
    createScDoc("tdf112735.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"(empty)"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));

    goToCell(u"B3"_ustr);

    dispatchCommand(mxComponent, u".uno:RecalcPivotTable"_ustr, {});

    // Without the fix in place, this test would have failed with
    // - Expected: (empty)
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"(empty)"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf112884)
{
    createScDoc("tdf112884.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"0.5"_ustr, pDoc->GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"0.666666666666667"_ustr, pDoc->GetString(ScAddress(6, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u"0.833333333333333"_ustr, pDoc->GetString(ScAddress(6, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#DIV/0!"_ustr, pDoc->GetString(ScAddress(6, 5, 0)));

    goToCell(u"G3"_ustr);

    dispatchCommand(mxComponent, u".uno:RecalcPivotTable"_ustr, {});

    // Without the fix in place, this test would have failed with
    // - Expected: #DIV/0!
    // - Actual  : 0.5
    CPPUNIT_ASSERT_EQUAL(u"#DIV/0!"_ustr, pDoc->GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"0.5"_ustr, pDoc->GetString(ScAddress(6, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u"0.666666666666667"_ustr, pDoc->GetString(ScAddress(6, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(u"0.833333333333333"_ustr, pDoc->GetString(ScAddress(6, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf133342)
{
    createScDoc("tdf133342.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"12,35 %"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    //Add decimals
    dispatchCommand(mxComponent, u".uno:NumberFormatIncDecimals"_ustr, {});
    //Space should preserved before percent sign
    CPPUNIT_ASSERT_EQUAL(u"12,346 %"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));

    //Delete decimals
    dispatchCommand(mxComponent, u".uno:NumberFormatDecDecimals"_ustr, {});
    dispatchCommand(mxComponent, u".uno:NumberFormatDecDecimals"_ustr, {});
    dispatchCommand(mxComponent, u".uno:NumberFormatDecDecimals"_ustr, {});
    //Space should preserved before percent sign
    CPPUNIT_ASSERT_EQUAL(u"12 %"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, u".uno:NumberFormatDecDecimals"_ustr, {});
    //Space should preserved before percent sign
    CPPUNIT_ASSERT_EQUAL(u"12 %"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf71339)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell(u"A2"_ustr, u"1");
    insertStringToCell(u"A3"_ustr, u"1");

    goToCell(u"A1:A3"_ustr);

    executeAutoSum();

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    OUString aFormula = pDoc->GetFormula(0, 3, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: =SUM(A1:A3)
    // - Actual  : =SUM(A2:A3)
    CPPUNIT_ASSERT_EQUAL(u"=SUM(A1:A3)"_ustr, aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf116421)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell(u"A1"_ustr, u"1");
    insertStringToCell(u"A2"_ustr, u"1");
    insertStringToCell(u"A3"_ustr, u"1");

    goToCell(u"A4"_ustr);

    executeAutoSum();

    // Without the fix in place, this test would have failed with
    // - Expected: 3
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    OUString aFormula = pDoc->GetFormula(0, 3, 0);

    CPPUNIT_ASSERT_EQUAL(u"=SUM(A1:A3)"_ustr, aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf86305)
{
    createScDoc("tdf86305.ods");
    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(1, 6, 0);
    CPPUNIT_ASSERT_EQUAL(u"{=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(1, 6, 0)));

    insertStringToCell(u"B3"_ustr, u"50");
    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(1, 2, 0)));

    aFormula = pDoc->GetFormula(1, 6, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: {=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}
    // - Actual  : {=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0.175)}
    CPPUNIT_ASSERT_EQUAL(u"{=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}"_ustr, aFormula);
    CPPUNIT_ASSERT_EQUAL(0.175, pDoc->GetValue(ScAddress(1, 6, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf81351)
{
    createScDoc("tdf81351.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));

    goToCell(u"A1:F5"_ustr);

    dispatchCommand(mxComponent, u".uno:SortAscending"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

    // Without the fix in place, this test would have crashed
    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));

    goToCell(u"A1:F5"_ustr);

    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

    // Without the fix in place, this test would have crashed
    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u".uno:Undo"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Save"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Paste"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, pDoc->GetString(ScAddress(0, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf123202)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell(u"A1"_ustr, u"1");
    insertStringToCell(u"A2"_ustr, u"2");
    insertStringToCell(u"A3"_ustr, u"3");
    insertStringToCell(u"A4"_ustr, u"4");

    goToCell(u"A3"_ustr);

    dispatchCommand(mxComponent, u".uno:HideRow"_ustr, {});

    goToCell(u"A1:A4"_ustr);

    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));

    // This failed, if the "3" is visible.
    CPPUNIT_ASSERT(pDoc->RowHidden(1, 0));
    CPPUNIT_ASSERT(!pDoc->RowHidden(2, 0));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));

    CPPUNIT_ASSERT(!pDoc->RowHidden(1, 0));
    CPPUNIT_ASSERT(pDoc->RowHidden(2, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf134675)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    insertStringToCell(u"A1"_ustr, u"A");

    // Select column A
    goToCell(u"A:A"_ustr);

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Select column B to Z
    goToCell(u"B:Z"_ustr);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    for (size_t i = 1; i < 24; ++i)
    {
        // Without the fix in place, this test would have failed here with
        // - Expected: A
        // - Actual  :
        CPPUNIT_ASSERT_EQUAL(u"A"_ustr, pDoc->GetString(ScAddress(i, 0, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf116215)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();
    insertStringToCell(u"A1"_ustr, u"1");
    insertStringToCell(u"A2"_ustr, u"1");
    insertStringToCell(u"B1"_ustr, u"1");
    insertStringToCell(u"B2"_ustr, u"1");
    goToCell(u"A1:C3"_ustr);

    executeAutoSum();

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 2, 0)));
    OUString aFormula = pDoc->GetFormula(0, 2, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(A1:A2)"_ustr, aFormula);

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 4
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 2, 0)));
    aFormula = pDoc->GetFormula(1, 2, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B1:B2)"_ustr, aFormula);

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(2, 0, 0)));
    aFormula = pDoc->GetFormula(2, 0, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(A1:B1)"_ustr, aFormula);

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(2, 1, 0)));
    aFormula = pDoc->GetFormula(2, 1, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(A2:B2)"_ustr, aFormula);
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
    dispatchCommand(mxComponent, u".uno:ToggleSheetGrid"_ustr, {});
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

    const ScRangeData* pRD = pDoc->GetRangeName()->findByUpperName(u"TEST"_ustr);
    CPPUNIT_ASSERT(pRD);
    CPPUNIT_ASSERT_EQUAL(u"$Test.$A$3:$D$7"_ustr, pRD->GetSymbol());

    //Select columns E to G
    goToCell(u"E:G"_ustr);

    dispatchCommand(mxComponent, u".uno:InsertColumnsBefore"_ustr, {});

    // Without the fix in place, this test would have failed with
    // - Expected: $Test.$A$3:$D$7
    // - Actual  : $Test.$A$3:$G$7
    CPPUNIT_ASSERT_EQUAL(u"$Test.$A$3:$D$7"_ustr, pRD->GetSymbol());

    xGlobalSheetSettings->setExpandReferences(bOldValue);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf107952)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    insertStringToCell(u"B1"_ustr, u"=SUM(A1:A2)");

    goToCell(u"D10"_ustr);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 3
    // - Incorrect Column in position B1
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B1");

    goToCell(u"D10"_ustr);

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    lcl_AssertCurrentCursorPosition(*pDocSh, u"B1");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf150766)
{
    createScDoc("tdf150766.ods");
    ScDocument* pDoc = getScDoc();

    goToCell(u"A3:C6"_ustr);

    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});

    insertStringToCell(u"B3"_ustr, u"10");

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

    goToCell(u"A5:B79"_ustr);

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    goToCell(u"D5"_ustr);

    //Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, u".uno:PasteTransposed"_ustr, {});

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

    insertStringToCell(u"B1"_ustr, u"This");
    insertStringToCell(u"B2"_ustr, u"=B1");

    goToCell(u"A1:B1"_ustr);

    dispatchCommand(mxComponent, u".uno:ToggleMergeCells"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, pDoc->GetString(ScAddress(1, 1, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"This"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: This
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(u"This"_ustr, pDoc->GetString(ScAddress(1, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf149378)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell(u"A1"_ustr, u"=MINVERSE(A1:C3)");

    // Without the fix in place, this test would have failed with
    // - Expected: {=MINVERSE(A1:C3)}
    // - Actual  : =MINVERSE(A1:C3)
    CPPUNIT_ASSERT_EQUAL(u"{=MINVERSE(A1:C3)}"_ustr, pDoc->GetFormula(0, 0, 0));

    insertStringToCell(u"B1"_ustr, u"={1;2}");
    CPPUNIT_ASSERT_EQUAL(u"{={1;2}}"_ustr, pDoc->GetFormula(1, 0, 0));

    insertStringToCell(u"C1"_ustr, u"={1;2}+3");
    CPPUNIT_ASSERT_EQUAL(u"{={1;2}+3}"_ustr, pDoc->GetFormula(2, 0, 0));

    insertStringToCell(u"D1"_ustr, u"={1;2}+{3;4}");
    CPPUNIT_ASSERT_EQUAL(u"{={1;2}+{3;4}}"_ustr, pDoc->GetFormula(3, 0, 0));

    insertStringToCell(u"E1"_ustr, u"={1;2}+A1");
    CPPUNIT_ASSERT_EQUAL(u"{={1;2}+A1}"_ustr, pDoc->GetFormula(4, 0, 0));

    insertStringToCell(u"F1"_ustr, u"={1;2}+A1:A2");
    CPPUNIT_ASSERT_EQUAL(u"={1;2}+A1:A2"_ustr, pDoc->GetFormula(5, 0, 0));

    insertStringToCell(u"G1"_ustr, u"=SUM(MUNIT(3))");
    CPPUNIT_ASSERT_EQUAL(u"=SUM(MUNIT(3))"_ustr, pDoc->GetFormula(6, 0, 0));

    insertStringToCell(u"H1"_ustr, u"=SUM({1;2})");
    CPPUNIT_ASSERT_EQUAL(u"=SUM({1;2})"_ustr, pDoc->GetFormula(7, 0, 0));

    insertStringToCell(u"I1"_ustr, u"=ABS({-1;-2})");
    CPPUNIT_ASSERT_EQUAL(u"{=ABS({-1;-2})}"_ustr, pDoc->GetFormula(8, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf152014)
{
    createScDoc();

    insertStringToCell(u"A1"_ustr, u"=MATCH(1,A2,0)");
    insertStringToCell(u"A2"_ustr, u"1");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));

    goToCell(u"A1"_ustr);

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Create a second document
    mxComponent2 = loadFromDesktop(u"private:factory/scalc"_ustr);

    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xFrames->getCount());

    dispatchCommand(mxComponent2, u".uno:Paste"_ustr, {});

    ScModelObj* pModelObj2 = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent2);
    CPPUNIT_ASSERT(pModelObj2);
    ScDocument* pDoc2 = pModelObj2->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: #N/A
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(u"#N/A"_ustr, pDoc2->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf156286)
{
    createScDoc("tdf156286.ods");
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, this test would have crash here
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u""_ustr, pDoc->GetString(ScAddress(12, 2, 0)));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"xxxxxxxxxxxx"_ustr, pDoc->GetString(ScAddress(12, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf126926)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell(u"A1"_ustr, u"1");
    insertStringToCell(u"A2"_ustr, u"2");
    insertStringToCell(u"B1"_ustr, u"3");
    insertStringToCell(u"B2"_ustr, u"4");

    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 0, 0, 0, 1, 1);
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    goToCell(u"A1:B1"_ustr);

    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

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
    dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});

    // That shouldn't need allocating more columns, just changing the default attribute.
    CPPUNIT_ASSERT_EQUAL(INITIALCOLCOUNT, pDoc->GetAllocatedColumnsCount(0));
    vcl::Font aFont;
    pDoc->GetPattern(pDoc->MaxCol(), 0, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());

    goToCell(u"A2:CV2"_ustr); // first 100 cells in row 2
    dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});
    // These need to be explicitly allocated.
    CPPUNIT_ASSERT_EQUAL(SCCOL(100), pDoc->GetAllocatedColumnsCount(0));
    pDoc->GetPattern(99, 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
    pDoc->GetPattern(100, 1, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should not be bold", WEIGHT_NORMAL, aFont.GetWeight());

    goToCell("CW3:" + pDoc->MaxColAsString() + "3"); // All but first 100 cells in row 3.
    dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});
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
    goToCell(u"B10"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 9, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B8:B9)"_ustr, pDoc->GetFormula(1, 9, 0));

    goToCell(u"B13"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 12, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B11:B12)"_ustr, pDoc->GetFormula(1, 12, 0));

    goToCell(u"B14"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(1, 13, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B13:B13,B10:B10)"_ustr, pDoc->GetFormula(1, 13, 0));

    goToCell(u"F8:F14"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(5, 13, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(F13:F13,F10:F10)"_ustr, pDoc->GetFormula(5, 13, 0));

    //Sum on Row and Column
    goToCell(u"E25"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 24, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(E22:E24)"_ustr, pDoc->GetFormula(4, 24, 0));

    goToCell(u"E26"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 25, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B26:D26)"_ustr, pDoc->GetFormula(4, 25, 0));

    goToCell(u"E27"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 26, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B27:D27)"_ustr, pDoc->GetFormula(4, 26, 0));

    goToCell(u"E28"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(4, 27, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B28:D28)"_ustr, pDoc->GetFormula(4, 27, 0));

    goToCell(u"E29"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(9.0, pDoc->GetValue(ScAddress(4, 28, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(E26:E28)"_ustr, pDoc->GetFormula(4, 28, 0));

    goToCell(u"E30"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(12.0, pDoc->GetValue(ScAddress(4, 29, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(E29:E29,E25:E25)"_ustr, pDoc->GetFormula(4, 29, 0));

    //Subtotals on Autosum
    goToCell(u"C49"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(20.0, pDoc->GetValue(ScAddress(2, 48, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUBTOTAL(9,C38:C48)"_ustr, pDoc->GetFormula(2, 48, 0));

    //Autosum on column with selected empty cell for result
    goToCell(u"B59:B64"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 63, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B59:B63)"_ustr, pDoc->GetFormula(1, 63, 0));

    //Autosum on rows with selected empty cell for result
    goToCell(u"B76:E80"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(4, 75, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B76:D76)"_ustr, pDoc->GetFormula(4, 75, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, pDoc->GetValue(ScAddress(4, 76, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B77:D77)"_ustr, pDoc->GetFormula(4, 76, 0));
    CPPUNIT_ASSERT_EQUAL(90.0, pDoc->GetValue(ScAddress(4, 77, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B78:D78)"_ustr, pDoc->GetFormula(4, 77, 0));
    CPPUNIT_ASSERT_EQUAL(120.0, pDoc->GetValue(ScAddress(4, 78, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B79:D79)"_ustr, pDoc->GetFormula(4, 78, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(4, 79, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B80:D80)"_ustr, pDoc->GetFormula(4, 79, 0));

    //Subtotal on column with selected empty cell for result
    goToCell(u"C92:C101"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(19.0, pDoc->GetValue(ScAddress(2, 100, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUBTOTAL(9,C92:C100)"_ustr, pDoc->GetFormula(2, 100, 0));

    //Autosum on column without selected empty cell for result
    goToCell(u"B109:B113"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 113, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B109:B113)"_ustr, pDoc->GetFormula(1, 113, 0));

    //Subtotal on column without selected empty cell for result
    goToCell(u"C142:C149"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(19.0, pDoc->GetValue(ScAddress(2, 150, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUBTOTAL(9,C142:C149)"_ustr, pDoc->GetFormula(2, 150, 0));

    //Autosum on multiselected columns without selected empty cell for result
    goToCell(u"B160:D164"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 164, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B160:B164)"_ustr, pDoc->GetFormula(1, 164, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(2, 164, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(C160:C164)"_ustr, pDoc->GetFormula(2, 164, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(3, 164, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(D160:D164)"_ustr, pDoc->GetFormula(3, 164, 0));

    //Autosum on columns with formula results without selected empty cell for result
    goToCell(u"B173:D177"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(2.54, std::floor(pDoc->GetValue(ScAddress(1, 177, 0)) * 100.0) / 100.0);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B173:B177)"_ustr, pDoc->GetFormula(1, 177, 0));
    CPPUNIT_ASSERT_EQUAL(-4.91, std::floor(pDoc->GetValue(ScAddress(2, 177, 0)) * 100.0) / 100.0);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(C173:C177)"_ustr, pDoc->GetFormula(2, 177, 0));
    CPPUNIT_ASSERT_EQUAL(5500.0, pDoc->GetValue(ScAddress(3, 177, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(D173:D177)"_ustr, pDoc->GetFormula(3, 177, 0));

    //Autosum on column with filled cell under selected area
    goToCell(u"B186:D190"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 191, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B186:B190)"_ustr, pDoc->GetFormula(1, 191, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(2, 191, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(C186:C190)"_ustr, pDoc->GetFormula(2, 191, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(3, 191, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(D186:D190)"_ustr, pDoc->GetFormula(3, 191, 0));

    //Autosum on column and rows with empty cells selected for row and column
    goToCell(u"B203:E208"_ustr);
    executeAutoSum();
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(1, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B203:B207)"_ustr, pDoc->GetFormula(1, 207, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(2, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(C203:C207)"_ustr, pDoc->GetFormula(2, 207, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(3, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(D203:D207)"_ustr, pDoc->GetFormula(3, 207, 0));
    CPPUNIT_ASSERT_EQUAL(450.0, pDoc->GetValue(ScAddress(4, 207, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B208:D208)"_ustr, pDoc->GetFormula(4, 207, 0));
    CPPUNIT_ASSERT_EQUAL(30.0, pDoc->GetValue(ScAddress(4, 202, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B203:D203)"_ustr, pDoc->GetFormula(4, 202, 0));
    CPPUNIT_ASSERT_EQUAL(60.0, pDoc->GetValue(ScAddress(4, 203, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B204:D204)"_ustr, pDoc->GetFormula(4, 203, 0));
    CPPUNIT_ASSERT_EQUAL(90.0, pDoc->GetValue(ScAddress(4, 204, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B205:D205)"_ustr, pDoc->GetFormula(4, 204, 0));
    CPPUNIT_ASSERT_EQUAL(120.0, pDoc->GetValue(ScAddress(4, 205, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B206:D206)"_ustr, pDoc->GetFormula(4, 205, 0));
    CPPUNIT_ASSERT_EQUAL(150.0, pDoc->GetValue(ScAddress(4, 206, 0)));
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B207:D207)"_ustr, pDoc->GetFormula(4, 206, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf152577)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell(u"A1"_ustr, u"1");
    insertStringToCell(u"A2"_ustr, u"2");
    insertStringToCell(u"B1"_ustr, u"3");
    insertStringToCell(u"B2"_ustr, u"4");

    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 0, 0, 0, 1, 1);
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    insertNewSheet(*pDoc);
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(2)) } }));
    dispatchCommand(mxComponent, u".uno:Remove"_ustr, aArgs);

    ScDBCollection* pDBs = pDoc->GetDBCollection();
    CPPUNIT_ASSERT(!pDBs->empty());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf155796)
{
    createScDoc();

    goToCell(u"A1:A3"_ustr);
    dispatchCommand(mxComponent, u".uno:ToggleMergeCells"_ustr, {});
    goToCell(u"A4:A6"_ustr);
    dispatchCommand(mxComponent, u".uno:ToggleMergeCells"_ustr, {});

    goToCell(u"A1:A6"_ustr);

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
    CPPUNIT_ASSERT_EQUAL(u"Sheet1.A1:Sheet1.A3"_ustr, aMarkedAreaString);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest2, testTdf156174)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertNewSheet(*pDoc);

    insertStringToCell(u"A1"_ustr, u"1");
    insertStringToCell(u"A2"_ustr, u"2");
    insertStringToCell(u"B1"_ustr, u"3");
    insertStringToCell(u"B2"_ustr, u"4");

    ScDBData* pDBData = new ScDBData(u"testDB"_ustr, 1, 0, 0, 1, 1);
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    insertNewSheet(*pDoc);
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(3)) } }));
    dispatchCommand(mxComponent, u".uno:Remove"_ustr, aArgs);

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
    goToCell(u"A1:CV1"_ustr);
    dispatchCommand(mxComponent, u".uno:BackgroundColor"_ustr, aColorArg);

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
    dispatchCommand(mxComponent, u".uno:BackgroundColor"_ustr, aColorArg);

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
