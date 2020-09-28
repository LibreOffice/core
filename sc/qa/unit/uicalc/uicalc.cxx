/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>

#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <document.hxx>
#include <docuno.hxx>
#include <tabvwsh.hxx>

using namespace ::com::sun::star;

class ScUiCalcTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp() override;

    virtual void tearDown() override;

    ScModelObj* createDoc(const char* pName);
    void checkCurrentCell(SCCOL nCol, SCROW nRow);

protected:
    uno::Reference<lang::XComponent> mxComponent;
};

void ScUiCalcTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void ScUiCalcTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void ScUiCalcTest::checkCurrentCell(SCCOL nCol, SCROW nRow)
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(nCol), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nRow), ScDocShell::GetViewData()->GetCurY());
}

char const DATA_DIRECTORY[] = "/sc/qa/unit/uicalc/data/";

ScModelObj* ScUiCalcTest::createDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY)
                                      + OUString::createFromAscii(pName),
                                  "com.sun.star.sheet.SpreadsheetDocument");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    return pModelObj;
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf122232)
{
    ScModelObj* pModelObj = createDoc("tdf122232.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    //Start with from C6. Press tabulator to reach G6.
    checkCurrentCell(2, 5);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    checkCurrentCell(6, 5);

    //without the fix, cursor would jump to C29 instead of C7.
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    checkCurrentCell(2, 6);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf133326)
{
    ScModelObj* pModelObj = createDoc("tdf133326.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    // .uno:Copy without touching shared clipboard
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    ScDocShell::GetViewData()->GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Name", uno::Any(OUString("")) }, { "Index", uno::Any(sal_Int32(2)) } }));
    dispatchCommand(mxComponent, ".uno:Insert", aArgs);

    OUString aFormula;
    pDoc->GetFormula(0, 0, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    // .uno:Paste without touching shared clipboard
    ScDocShell::GetViewData()->GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);
    Scheduler::ProcessEventsToIdle();

    pDoc->GetFormula(0, 0, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=RAND()*1000000"), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    pDoc->GetFormula(0, 0, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
    pDoc->GetFormula(0, 0, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    pDoc->GetFormula(0, 0, 1, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=RAND()*1000000"), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf126904)
{
    ScModelObj* pModelObj = createDoc("tdf126904.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    checkCurrentCell(0, 4);
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    checkCurrentCell(1, 4);
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    checkCurrentCell(4, 4);
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    checkCurrentCell(5, 4);
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    checkCurrentCell(8, 4);
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    checkCurrentCell(9, 4);
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    checkCurrentCell(12, 4);

    //Cursor can't move forward to the right
    for (size_t i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:GoRight", {});
        checkCurrentCell(13, 4);
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf124816)
{
    ScModelObj* pModelObj = createDoc("tdf124816.xlsx");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // The actual result is completely unrelated to this test and behaviour of
    // OFFSET() was changed as of tdf#85551 and here result of that test
    // document is now Err:502 instead of 0.
    const OUString aExpectedResult("Err:502");
    checkCurrentCell(3, 9);
    CPPUNIT_ASSERT_EQUAL(aExpectedResult, pDoc->GetString(ScAddress(3, 9, 0)));

    //Without the fix, it would crash
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(3, 9, 0)));
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(aExpectedResult, pDoc->GetString(ScAddress(3, 9, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf124815)
{
    ScModelObj* pModelObj = createDoc("tdf124815.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    checkCurrentCell(0, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(2, 0, 0)));

    //Without the fix, it would crash
    dispatchCommand(mxComponent, ".uno:InsertColumnsBefore", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(3, 0, 0)));
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(2, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf132431)
{
    ScModelObj* pModelObj = createDoc("tdf132431.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    OUString aFormula;
    pDoc->GetFormula(7, 219, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIFS($H$2:$H$198,B$2:B$198,G220)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(7, 219, 0)));

    // Without the fix in place, it would crash here with
    // uncaught exception of type std::exception (or derived).
    // - vector::_M_fill_insert
    pDoc->SetString(ScAddress(7, 219, 0), "=SUMIFS($H$2:$DB$198,B$2:B$198,G220)");

    pDoc->GetFormula(7, 219, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIFS($H$2:$DB$198,B$2:B$198,G220)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(7, 219, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Err:502"), pDoc->GetString(ScAddress(7, 219, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf83901)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    checkCurrentCell(0, 0);
    pDoc->SetString(ScAddress(0, 1, 0), "=ROW(A3)");
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    checkCurrentCell(0, 2);
    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});

    //Without the fix, it would be 3.0
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf133342)
{
    ScModelObj* pModelObj = createDoc("tdf133342.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    //Select cell A1
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
