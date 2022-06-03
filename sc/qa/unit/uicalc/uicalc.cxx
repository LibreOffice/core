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
#include <svx/svdpage.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/scheduler.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <conditio.hxx>
#include <dbdata.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <inputopt.hxx>
#include <postit.hxx>
#include <rangeutl.hxx>
#include <scmod.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>

using namespace ::com::sun::star;

class ScUiCalcTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp() override;

    virtual void tearDown() override;

    ScModelObj* createDoc(const char* pName);
    utl::TempFile save(css::uno::Reference<css::lang::XComponent>& xComponent,
                       const OUString& rFilter);
    ScModelObj* saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                              const OUString& rFilter);
    void goToCell(const OUString& rCell);
    void typeString(ScModelObj& rModelObj, const std::string& rStr);
    void insertStringToCell(ScModelObj& rModelObj, const OUString& rCell, const std::string& rStr);
    void insertArrayToCell(ScModelObj& rModelObj, const OUString& rCell, const std::string& rStr);
    void insertNewSheet(ScDocument& rDoc);

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

static void lcl_AssertCurrentCursorPosition(const ScDocument& rDoc, const OUString& rStr)
{
    ScAddress aAddr;
    sal_Int32 nOffset = 0;
    ScRangeStringConverter::GetAddressFromString(aAddr, rStr, rDoc,
                                                 formula::FormulaGrammar::CONV_OOO, nOffset);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OUString("Incorrect Column in position " + rStr).toUtf8().getStr(),
                                 aAddr.Col(), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OUString("Incorrect Row in position " + rStr).toUtf8().getStr(),
                                 aAddr.Row(), ScDocShell::GetViewData()->GetCurY());
}

static void lcl_SelectObjectByName(std::u16string_view rObjName)
{
    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);

    bool bFound = pViewShell->SelectObject(rObjName);
    CPPUNIT_ASSERT_MESSAGE(
        OString(OUStringToOString(rObjName, RTL_TEXTENCODING_UTF8) + " not found.").getStr(),
        bFound);

    CPPUNIT_ASSERT(ScDocShell::GetViewData()->GetScDrawView()->AreObjectsMarked());
}

void ScUiCalcTest::goToCell(const OUString& rCell)
{
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "ToPoint", uno::Any(rCell) } });
    dispatchCommand(mxComponent, ".uno:GoToCell", aArgs);
}

void ScUiCalcTest::typeString(ScModelObj& rModelObj, const std::string& rStr)
{
    for (const char c : rStr)
    {
        rModelObj.postKeyEvent(LOK_KEYEVENT_KEYINPUT, c, 0);
        rModelObj.postKeyEvent(LOK_KEYEVENT_KEYUP, c, 0);
        Scheduler::ProcessEventsToIdle();
    }
}

void ScUiCalcTest::insertStringToCell(ScModelObj& rModelObj, const OUString& rCell,
                                      const std::string& rStr)
{
    goToCell(rCell);

    typeString(rModelObj, rStr);

    rModelObj.postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    rModelObj.postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
}

void ScUiCalcTest::insertArrayToCell(ScModelObj& rModelObj, const OUString& rCell,
                                     const std::string& rStr)
{
    goToCell(rCell);

    typeString(rModelObj, rStr);

    rModelObj.postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | KEY_SHIFT | awt::Key::RETURN);
    rModelObj.postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_MOD1 | KEY_SHIFT | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
}

void ScUiCalcTest::insertNewSheet(ScDocument& rDoc)
{
    sal_Int32 nTabs = static_cast<sal_Int32>(rDoc.GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Name", uno::Any(OUString("NewTab")) }, { "Index", uno::Any(nTabs + 1) } }));
    dispatchCommand(mxComponent, ".uno:Insert", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(nTabs + 1), rDoc.GetTableCount());
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/sc/qa/unit/uicalc/data/";

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

utl::TempFile ScUiCalcTest::save(css::uno::Reference<css::lang::XComponent>& xComponent,
                                 const OUString& rFilter)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    css::uno::Sequence aArgs{ comphelper::makePropertyValue("FilterName", rFilter) };
    css::uno::Reference<css::frame::XStorable> xStorable(xComponent, css::uno::UNO_QUERY_THROW);
    xStorable->storeAsURL(aTempFile.GetURL(), aArgs);
    css::uno::Reference<css::util::XCloseable> xCloseable(xComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(true);

    return aTempFile;
}

ScModelObj* ScUiCalcTest::saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                                        const OUString& rFilter)
{
    utl::TempFile aTempFile = save(xComponent, rFilter);

    mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.sheet.SpreadsheetDocument");

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    return pModelObj;
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf100847)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Save the document
    utl::TempFile aTempFile = save(mxComponent, "calc8");

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testExternalReferences)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "2015");
    insertStringToCell(*pModelObj, "A2", "2015");
    insertStringToCell(*pModelObj, "A3", "2015");

    insertStringToCell(*pModelObj, "B1", "1");
    insertStringToCell(*pModelObj, "B2", "1");
    insertStringToCell(*pModelObj, "B3", "2");

    insertStringToCell(*pModelObj, "C1", "10");
    insertStringToCell(*pModelObj, "C2", "20");
    insertStringToCell(*pModelObj, "C3", "5");

    insertStringToCell(*pModelObj, "D1", "BIG FISH");
    insertStringToCell(*pModelObj, "D2", "FISHFISH");
    insertStringToCell(*pModelObj, "D3", "FISHY");

    // Save the document
    utl::TempFile aTempFile = save(mxComponent, "calc8");

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Insert the references to the external document
    {
        // tdf#115162
        OUString aFormula = "=SUMIFS('" + aTempFile.GetURL() + "'#$Sheet1.C1:C3,'"
                            + aTempFile.GetURL() + "'#$Sheet1.B1:B3,1,'" + aTempFile.GetURL()
                            + "'#$Sheet1.A1:A3,2015)";
        insertStringToCell(*pModelObj, "A1", aFormula.toUtf8().getStr());

        // tdf#115162: Without the fix in place, this test would have failed with
        // - Expected: 30
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("30"), pDoc->GetString(ScAddress(0, 0, 0)));
    }

    {
        // tdf#114820
        OUString aFormula = "=VLOOKUP('" + aTempFile.GetURL() + "'#$Sheet1.A1;'"
                            + aTempFile.GetURL() + "'#$Sheet1.A1:B3,2,0)";
        insertStringToCell(*pModelObj, "A1", aFormula.toUtf8().getStr());

        // Without the fix in place, this test would have failed with
        // - Expected: 1
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(0, 0, 0)));
    }

    {
        // tdf#116149
        OUString aFormula = "=VAR('" + aTempFile.GetURL() + "'#$Sheet1.C1;'" + aTempFile.GetURL()
                            + "'#$Sheet1.C2)";
        insertStringToCell(*pModelObj, "A1", aFormula.toUtf8().getStr());

        // Without the fix in place, this test would have failed with
        // - Expected: 50
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("50"), pDoc->GetString(ScAddress(0, 0, 0)));
    }

    {
        // tdf#100847
        // Use an empty cell
        OUString aFormula = "=+'" + aTempFile.GetURL() + "'#$Sheet1.A1000";
        insertStringToCell(*pModelObj, "A1", aFormula.toUtf8().getStr());

        aFormula = "=+'" + aTempFile.GetURL() + "'#$Sheet1.A1000*1";
        insertStringToCell(*pModelObj, "B1", aFormula.toUtf8().getStr());

        aFormula = "=+N('" + aTempFile.GetURL() + "'#$Sheet1.A1000)*1";
        insertStringToCell(*pModelObj, "C1", aFormula.toUtf8().getStr());

        CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(0, 0, 0)));

        // Without the fix in place, this test would have failed with
        // - Expected: 0
        // - Actual  : #VALUE!
        CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(1, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(2, 0, 0)));
    }

    {
        //tdf#36387
        OUString aAndFormula = "=AND('" + aTempFile.GetURL() + "'#$Sheet1.A1:C1)";
        insertStringToCell(*pModelObj, "A1", aAndFormula.toUtf8().getStr());

        OUString aOrFormula = "=OR('" + aTempFile.GetURL() + "'#$Sheet1.A1:C1)";
        insertStringToCell(*pModelObj, "B1", aOrFormula.toUtf8().getStr());

        // Without the fix in place, this test would have failed with
        // - Expected: TRUE
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), pDoc->GetString(ScAddress(1, 0, 0)));
    }

    {
        //tdf#113898
        OUString aAndFormula = "=SUMPRODUCT(NOT(ISERROR(FIND(\"FISH\";'" + aTempFile.GetURL()
                               + "'#$Sheet1.D1:D3))))";
        insertStringToCell(*pModelObj, "A1", aAndFormula.toUtf8().getStr());

        // Without the fix in place, this test would have failed with
        // - Expected: 3
        // - Actual  : 1
        CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 0, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf103994)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "1");
    insertStringToCell(*pModelObj, "B1", "2");

    // Save the document
    utl::TempFile aTempFile = save(mxComponent, "calc8");

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Insert the reference to the external document
    OUString aFormula = "='" + aTempFile.GetURL() + "'#$Sheet1.A1";
    insertStringToCell(*pModelObj, "A1", aFormula.toUtf8().getStr());

    CPPUNIT_ASSERT_EQUAL(aFormula, pDoc->GetFormula(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    goToCell("A1");

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    goToCell("B1");

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: ='file:///tmp/lu124171irlmb.tmp'#$Sheet1.A1
    // - Actual  : ='file:///tmp/lu124171irlmb.tmp'#$Sheet1.B1
    CPPUNIT_ASSERT_EQUAL(aFormula, pDoc->GetFormula(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf113541)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "50");

    // Save the document
    utl::TempFile aTempFile = save(mxComponent, "calc8");

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Change grammar to Excel A1
    pDoc->SetGrammar(formula::FormulaGrammar::GRAM_ENGLISH_XL_A1);

    // Insert the reference to the external document
    OUString aFormula = "=['" + aTempFile.GetURL() + "']Sheet1!A1";
    insertStringToCell(*pModelObj, "A1", aFormula.toUtf8().getStr());

    // Without the fix in place, this test would have failed with
    // - Expected: 50
    // - Actual  : Err:507
    CPPUNIT_ASSERT_EQUAL(OUString("50"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Change grammar to default
    pDoc->SetGrammar(formula::FormulaGrammar::GRAM_ENGLISH);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf126577)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("A1:A20");

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "FillDir", uno::Any(OUString("B")) },
                                           { "FillCmd", uno::Any(OUString("L")) },
                                           { "FillStep", uno::Any(OUString("1")) },
                                           { "FillDateCmd", uno::Any(OUString("D")) },
                                           { "FillStart", uno::Any(OUString("1")) },
                                           { "FillMax", uno::Any(OUString("10")) } }));
    dispatchCommand(mxComponent, ".uno:FillSeries", aArgs);

    for (size_t i = 0; i < 10; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString(OUString::number(i + 1)),
                             pDoc->GetString(ScAddress(0, i, 0)));
    }

    for (size_t i = 10; i < 20; ++i)
    {
        // Without the fix in place, this test would have failed with
        // - Expected:
        // - Actual  : #NUM!
        CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, i, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf107869)
{
    // Without the fix in place, this test would have crashed
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "A");
    insertStringToCell(*pModelObj, "A2", "B");

    // Add a new comment to A1 and A2
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Comment")) } });

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A2", pDoc->HasNote(ScAddress(0, 1, 0)));

    goToCell("A1");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    Scheduler::ProcessEventsToIdle();

    for (size_t i = 0; i < 10; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("B"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be no note on A2", !pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("B"), pDoc->GetString(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A2", pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Redo", {});
        Scheduler::ProcessEventsToIdle();
    }

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    goToCell("A1:A2");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    Scheduler::ProcessEventsToIdle();

    for (size_t i = 0; i < 10; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be no note on A1", !pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be no note on A2", !pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("B"), pDoc->GetString(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A2", pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Redo", {});
        Scheduler::ProcessEventsToIdle();
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf63805)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "2012-10-31");

    goToCell("A1:A20");

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "FillDir", uno::Any(OUString("B")) },
                                           { "FillCmd", uno::Any(OUString("D")) },
                                           { "FillStep", uno::Any(OUString("1")) },
                                           { "FillDateCmd", uno::Any(OUString("M")) },
                                           { "FillStart", uno::Any(OUString("41213")) } }));
    dispatchCommand(mxComponent, ".uno:FillSeries", aArgs);

    CPPUNIT_ASSERT_EQUAL(OUString("2012-10-31"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: 2012-11-30
    // - Actual  : 2012-12-01
    CPPUNIT_ASSERT_EQUAL(OUString("2012-11-30"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2012-12-31"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-01-31"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-02-28"), pDoc->GetString(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-03-31"), pDoc->GetString(ScAddress(0, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-04-30"), pDoc->GetString(ScAddress(0, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-05-31"), pDoc->GetString(ScAddress(0, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-06-30"), pDoc->GetString(ScAddress(0, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-07-31"), pDoc->GetString(ScAddress(0, 9, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-08-31"), pDoc->GetString(ScAddress(0, 10, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-09-30"), pDoc->GetString(ScAddress(0, 11, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-10-31"), pDoc->GetString(ScAddress(0, 12, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-11-30"), pDoc->GetString(ScAddress(0, 13, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2013-12-31"), pDoc->GetString(ScAddress(0, 14, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-01-31"), pDoc->GetString(ScAddress(0, 15, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-02-28"), pDoc->GetString(ScAddress(0, 16, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-03-31"), pDoc->GetString(ScAddress(0, 17, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-04-30"), pDoc->GetString(ScAddress(0, 18, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2014-05-31"), pDoc->GetString(ScAddress(0, 19, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("2012-10-31"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf147894)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    //Select the first row
    goToCell("1:1");

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "FillDir", uno::Any(OUString("R")) },
                                           { "FillCmd", uno::Any(OUString("L")) },
                                           { "FillStep", uno::Any(OUString("1")) },
                                           { "FillDateCmd", uno::Any(OUString("D")) },
                                           { "FillStart", uno::Any(OUString("1")) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:FillSeries", aArgs);

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(16384.0, pDoc->GetValue(ScAddress(16383, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf94208)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "=COUNTA(B:B)");
    insertStringToCell(*pModelObj, "A3", "Range");
    insertStringToCell(*pModelObj, "A4", "Range");
    insertStringToCell(*pModelObj, "A5", "Range");
    insertStringToCell(*pModelObj, "A6", "Range");
    insertStringToCell(*pModelObj, "A7", "Range");
    insertStringToCell(*pModelObj, "A8", "Range");
    insertStringToCell(*pModelObj, "B6", "Test");

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    goToCell("A3:A8");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    Scheduler::ProcessEventsToIdle();

    //type Control-D/Fill Down
    dispatchCommand(mxComponent, ".uno:FillDown", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    for (SCROW i = 2; i < 8; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("Range"), pDoc->GetString(ScAddress(0, i, 0)));
    }

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), pDoc->GetString(ScAddress(1, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf37623)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("A3:A4");

    dispatchCommand(mxComponent, ".uno:HideRow", {});

    insertStringToCell(*pModelObj, "A2", "1");

    goToCell("A2:A6");

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "FillDir", uno::Any(OUString("B")) },
                                           { "FillCmd", uno::Any(OUString("A")) },
                                           { "FillStep", uno::Any(OUString("1")) },
                                           { "FillDateCmd", uno::Any(OUString("M")) } }));
    dispatchCommand(mxComponent, ".uno:FillSeries", aArgs);

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 5, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf144308)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    css::uno::Reference<css::sheet::XGlobalSheetSettings> xGlobalSheetSettings
        = css::sheet::GlobalSheetSettings::create(::comphelper::getProcessComponentContext());
    bool bOldValue = xGlobalSheetSettings->getDoAutoComplete();

    xGlobalSheetSettings->setDoAutoComplete(true);

    pDoc->SetString(ScAddress(0, 0, 0), "ABC");

    insertStringToCell(*pModelObj, "A2", "A");

    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), pDoc->GetString(ScAddress(0, 1, 0)));

    xGlobalSheetSettings->setDoAutoComplete(false);

    insertStringToCell(*pModelObj, "A3", "A");

    // Without the fix in place, this test would have failed with
    // - Expected: A
    // - Actual  : ABC
    CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(0, 2, 0)));

    // Restore the previous value
    xGlobalSheetSettings->setDoAutoComplete(bOldValue);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf56036)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("A1");

    typeString(*pModelObj, "=SUM( 1 + 2 ");

    // Insert Newline
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_MOD1 | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    typeString(*pModelObj, "+ 3)");

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : Err:501
    CPPUNIT_ASSERT_EQUAL(OUString("6"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf119162)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("A1");

    typeString(*pModelObj, "Test");

    // Insert Newline
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_MOD1 | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"Test" + OUStringChar(u'\xA')),
                         pDoc->GetString(ScAddress(0, 0, 0)));

    goToCell("A1");

    // Without the fix in place, this test would have hung here
    dispatchCommand(mxComponent, ".uno:ChangeCaseRotateCase", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"Test" + OUStringChar(u'\xA')),
                         pDoc->GetString(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, ".uno:ChangeCaseToLower", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"test" + OUStringChar(u'\xA')),
                         pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf90579)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "2300");
    insertStringToCell(*pModelObj, "A2", "Libre");
    insertStringToCell(*pModelObj, "B1", "10");
    insertStringToCell(*pModelObj, "B2", "Office");
    insertStringToCell(*pModelObj, "C1", "=SUM(A1:B1)");
    insertStringToCell(*pModelObj, "C2", "=A2&B2");

    CPPUNIT_ASSERT_EQUAL(OUString("2310"), pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice"), pDoc->GetString(ScAddress(2, 1, 0)));

    goToCell("C1:C2");

    dispatchCommand(mxComponent, ".uno:ConvertFormulaToValue", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("2310"), pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice"), pDoc->GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetFormula(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetFormula(2, 1, 0));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("2310"), pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice"), pDoc->GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:B1)"), pDoc->GetFormula(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("=A2&B2"), pDoc->GetFormula(2, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf124820)
{
    ScModelObj* pModelObj = createDoc("tdf124820.xlsx");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("B2");

    dispatchCommand(mxComponent, ".uno:Strikeout", {});
    Scheduler::ProcessEventsToIdle();

    pModelObj = saveAndReload(mxComponent, "Calc Office Open XML");
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    vcl::Font aFont;
    const ScPatternAttr* pPattern = pDoc->GetPattern(1, 1, 0);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be striked out", STRIKEOUT_SINGLE,
                                 aFont.GetStrikeout());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf119155)
{
    ScModelObj* pModelObj = createDoc("tdf119155.xlsx");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("C2:C14");

    // Without the fix in place, this test would have hung here
    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("Collagene Expert Targeted Wrinkle Corrector Unboxed 10 Ml"),
                         pDoc->GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Vitamina Suractivee Hand Cream 400 Ml"),
                         pDoc->GetString(ScAddress(2, 13, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf146795)
{
    ScModelObj* pModelObj = createDoc("tdf146795.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    goToCell("B2");

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    // Move to B3
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Select the three following cells
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(ScAddress(1, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("44"), pDoc->GetString(ScAddress(1, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("44"), pDoc->GetString(ScAddress(1, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(ScAddress(1, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("44"), pDoc->GetString(ScAddress(1, 8, 0)));

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf147744)
{
    ScModelObj* pModelObj = createDoc("tdf147744.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    goToCell("A2");

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    // Move to A3
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Select the following cell
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(0, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(0, 6, 0)));

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf138432)
{
    ScModelObj* pModelObj = createDoc("tdf138432.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Set the system locale to Hungarian
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("hu-HU");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    OUString sExpectedA1 = "12" + OUStringChar(u'\xa0') + "345,67";
    CPPUNIT_ASSERT_EQUAL(sExpectedA1, pDoc->GetString(ScAddress(0, 0, 0)));

    goToCell("A1");

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    goToCell("A2");

    typeString(*pModelObj, "=");

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: 12345,67
    // - Actual  : Err:509
    CPPUNIT_ASSERT_EQUAL(OUString("12345,67"), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf143896)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A2000", "Test");

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), pDoc->GetString(ScAddress(0, 1999, 0)));

    pModelObj = saveAndReload(mxComponent, "Calc Office Open XML");
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: Test
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("Test"), pDoc->GetString(ScAddress(0, 1999, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf145085)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertArrayToCell(*pModelObj, "A1", "=HYPERLINK(\"a\";\"b\")");

    CPPUNIT_ASSERT_EQUAL(OUString("b"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : a
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf148863)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertArrayToCell(*pModelObj, "A1", "=TRANSPOSE(IF({0|0|0}=0;RANDBETWEEN.NV(1;1000000)))");

    double nA1 = pDoc->GetValue(ScAddress(0, 0, 0));
    double nB1 = pDoc->GetValue(ScAddress(1, 0, 0));
    double nC1 = pDoc->GetValue(ScAddress(2, 0, 0));

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(nA1 != nB1);
    CPPUNIT_ASSERT(nA1 != nC1);
    CPPUNIT_ASSERT(nB1 != nC1);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf144244)
{
    ScModelObj* pModelObj = createDoc("tdf144244.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    uno::Reference<drawing::XDrawPage> xPage(pModelObj->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xPage->getCount());

    // Select column A and B
    goToCell("A:B");

    dispatchCommand(mxComponent, ".uno:DeleteColumns", {});

    CPPUNIT_ASSERT_EQUAL(OUString("x"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have crashed
    pModelObj = saveAndReload(mxComponent, "calc8");
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("x"), pDoc->GetString(ScAddress(0, 0, 0)));

    xPage.set(pModelObj->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xPage->getCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf100582)
{
    ScModelObj* pModelObj = createDoc("tdf100582.xls");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    goToCell("C10");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("C10:H14");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    pModelObj = saveAndReload(mxComponent, "MS Excel 97");
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    OUString aFormula = pDoc->GetFormula(3, 10, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: {=SUM(($B$3:$B$7=$B11)*(D$3:D$7))}
    //- Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("{=SUM(($B$3:$B$7=$B11)*(D$3:D$7))}"), aFormula);
    aFormula = pDoc->GetFormula(4, 10, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("{=SUM(($B$3:$B$7=$B11)*(E$3:E$7))}"), aFormula);
    aFormula = pDoc->GetFormula(5, 10, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("{=SUM(($B$3:$B$7=$B11)*(F$3:F$7))}"), aFormula);
    aFormula = pDoc->GetFormula(6, 10, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("{=SUM(($B$3:$B$7=$B11)*(G$3:G$7))}"), aFormula);
    aFormula = pDoc->GetFormula(7, 10, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("{=SUM(($B$3:$B$7=$B11)*(H$3:H$7))}"), aFormula);

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf145640)
{
    ScModelObj* pModelObj = createDoc("tdf145640.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Enable sorting with update reference
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetSortRefUpdate();
    aInputOption.SetSortRefUpdate(true);
    pMod->SetInputOptions(aInputOption);

    goToCell("A2:F17");

    dispatchCommand(mxComponent, ".uno:SortDescending", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A15:B15:C15:D15:E15:F15)"), pDoc->GetFormula(6, 3, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: 10
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(6, 3, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A4:B4:C4:D4:E4:F4)"), pDoc->GetFormula(6, 3, 0));
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(6, 3, 0)));

    // Restore previous status
    aInputOption.SetSortRefUpdate(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf97215)
{
    ScModelObj* pModelObj = createDoc("tdf97215.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Enable sorting with update reference
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetSortRefUpdate();
    aInputOption.SetSortRefUpdate(true);
    pMod->SetInputOptions(aInputOption);

    CPPUNIT_ASSERT_EQUAL(OUString("Inserted at bottom"), pDoc->GetString(ScAddress(0, 23, 0)));

    goToCell("A1:M24");

    // Without the fix in place, this test would have hung here
    dispatchCommand(mxComponent, ".uno:SortAscending", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("Inserted at bottom"), pDoc->GetString(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("Inserted at bottom"), pDoc->GetString(ScAddress(0, 23, 0)));

    // Restore previous status
    aInputOption.SetSortRefUpdate(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf92963)
{
    ScModelObj* pModelObj = createDoc("tdf92963.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT_EQUAL(size_t(3), pList->size());

    goToCell("A3:C4");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("A1:C1");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(size_t(2), pList->size());

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf140151)
{
#if !defined(MACOSX) && !defined(_WIN32) //FIXME
    ScModelObj* pModelObj = createDoc("tdf140151.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Focus is already on the button
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the current cursor position wouldn't have changed
    lcl_AssertCurrentCursorPosition(*pDoc, "B111");
#endif
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf68290)
{
    ScModelObj* pModelObj = createDoc("tdf68290.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    const std::vector<OUString> aExpectedAddresses{ "L3", "L6", "L9", "L10", "L11", "L13", "L15" };

    for (const auto& rAddress : aExpectedAddresses)
    {
        lcl_AssertCurrentCursorPosition(*pDoc, rAddress);

        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    }

    lcl_AssertCurrentCursorPosition(*pDoc, "M3");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf132057)
{
    ScModelObj* pModelObj = createDoc("tdf132057.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    lcl_AssertCurrentCursorPosition(*pDoc, "AU43");

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the cursor would have jumped to cell BM1
    lcl_AssertCurrentCursorPosition(*pDoc, "G39");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf122232)
{
    ScModelObj* pModelObj = createDoc("tdf122232.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    //Start with from C6. Press tabulator to reach G6.
    lcl_AssertCurrentCursorPosition(*pDoc, "C6");

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    lcl_AssertCurrentCursorPosition(*pDoc, "G6");

    //without the fix, cursor would jump to C29 instead of C7.
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    lcl_AssertCurrentCursorPosition(*pDoc, "C7");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf123052)
{
    ScModelObj* pModelObj = createDoc("tdf123052.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    std::vector<OUString> aExpectedAddresses{ "F3", "D5", "E5", "F6", "A8", "E9" };

    for (const auto& rAddress : aExpectedAddresses)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
        Scheduler::ProcessEventsToIdle();

        lcl_AssertCurrentCursorPosition(*pDoc, rAddress);
    }

    aExpectedAddresses.pop_back();

    for (std::vector<OUString>::reverse_iterator it = aExpectedAddresses.rbegin();
         it != aExpectedAddresses.rend(); ++it)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | awt::Key::TAB);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_SHIFT | awt::Key::TAB);
        Scheduler::ProcessEventsToIdle();

        // Without the fix in place, this test would have failed here
        lcl_AssertCurrentCursorPosition(*pDoc, *it);
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf120660)
{
    ScModelObj* pModelObj = createDoc("tdf120660.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    goToCell("A8:E8");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("A4:E4");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(1200.0, pDoc->GetValue(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(-100.0, pDoc->GetValue(ScAddress(4, 7, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(2200.0, pDoc->GetValue(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(900.0, pDoc->GetValue(ScAddress(4, 7, 0)));

    goToCell("A8:D8");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("A4:D4");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(1200.0, pDoc->GetValue(ScAddress(4, 3, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: -100
    // - Actual  : 900
    CPPUNIT_ASSERT_EQUAL(-100.0, pDoc->GetValue(ScAddress(4, 7, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(2200.0, pDoc->GetValue(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(900.0, pDoc->GetValue(ScAddress(4, 7, 0)));

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf146994)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("B3");
    lcl_AssertCurrentCursorPosition(*pDoc, "B3");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDoc, "D3");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    ScRangeList aMarkedArea = ScDocShell::GetViewData()->GetMarkData().GetMarkedRanges();
    OUString aMarkedAreaString;
    ScRangeStringConverter::GetStringFromRangeList(aMarkedAreaString, &aMarkedArea, pDoc,
                                                   formula::FormulaGrammar::CONV_OOO);

    // Without the fix in place, this test would have failed with
    // - Expected: Sheet1.D3:Sheet1.D4
    // - Actual  : Sheet1.A2:Sheet1.D3
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1.D3:Sheet1.D4"), aMarkedAreaString);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf45020)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("A2:A3");

    dispatchCommand(mxComponent, ".uno:HideRow", {});

    goToCell("A1");

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    ScRangeList aMarkedArea = ScDocShell::GetViewData()->GetMarkData().GetMarkedRanges();
    OUString aMarkedAreaString;
    ScRangeStringConverter::GetStringFromRangeList(aMarkedAreaString, &aMarkedArea, pDoc,
                                                   formula::FormulaGrammar::CONV_OOO);

    // Without the fix in place, this test would have failed with
    // - Expected: Sheet1.A1:Sheet1.A4
    // - Actual  : Sheet1.A1:Sheet1.A2
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1.A1:Sheet1.A4"), aMarkedAreaString);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf117706)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "A1");
    insertStringToCell(*pModelObj, "A3", "A3");

    // Use Adding Selection
    dispatchCommand(mxComponent, ".uno:StatusSelectionModeExp", {});
    Scheduler::ProcessEventsToIdle();

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "A3");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Copy", {});

    mxComponent->dispose();

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without the fix in place, this test would have failed with
    // - Expected: A1
    // - Actual  : A3
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A3"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf86166)
{
    ScModelObj* pModelObj = createDoc("tdf86166.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf108292)
{
    ScModelObj* pModelObj = createDoc("tdf108292.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    mxComponent->dispose();

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("2D"), pDoc->GetString(ScAddress(26, 0, 0)));
}

// Inspired from testTdf117706, test columns instead of rows
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testMultiRangeCol)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "A1");
    insertStringToCell(*pModelObj, "C1", "C1");

    // Use Adding Selection
    dispatchCommand(mxComponent, ".uno:StatusSelectionModeExp", {});
    Scheduler::ProcessEventsToIdle();

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:SelectColumn", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:GoRight", {});
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "C1");

    dispatchCommand(mxComponent, ".uno:SelectColumn", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Copy", {});

    mxComponent->dispose();

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("A1"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("C1"), pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(2, 0, 0)));
}

// Test the call of .uno:PasteTransposed (tdf#102255)
// Note: the transpose functionality is tested in ucalc
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testPasteTransposed)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "1");
    insertStringToCell(*pModelObj, "A2", "a");
    insertStringToCell(*pModelObj, "A3", "=A1");

    // Add a note to A1
    goToCell("A1");
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Note in A1")) } });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    // Set A2 bold
    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:Bold", {});

    // Check preconditions
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    const ScPatternAttr* pPattern = pDoc->GetPattern(0, 1, 0);
    vcl::Font aFont;
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());

    goToCell("A1:A3");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    mxComponent->dispose();

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:PasteTransposed", {});

    OUString aFormula;
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(0, 0, 0)); // A1
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 0, 0)); // A1
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pDoc->GetString(1, 0, 0)); // B1
    aFormula = pDoc->GetFormula(2, 0, 0); // C1
    CPPUNIT_ASSERT_EQUAL(OUString("=A1"), aFormula); // C1
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(2, 0, 0)); // C1
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(2, 0, 0)); // C1

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Note in A1"), pDoc->GetNote(ScAddress(0, 0, 0))->GetText());

    pPattern = pDoc->GetPattern(1, 0, 0);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
}

// Test the call of .uno:PasteAsLink (tdf#90101)
// Note: the paste as link functionaly is tested in ucalc
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testPasteAsLink)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "1");
    insertStringToCell(*pModelObj, "A2", "a");
    insertStringToCell(*pModelObj, "A3", "=A1");

    // Add a note to A1
    goToCell("A1");
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Note in A1")) } });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    // Set A2 bold
    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:Bold", {});

    // Check preconditions
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    const ScPatternAttr* pPattern = pDoc->GetPattern(0, 1, 0);
    vcl::Font aFont;
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());

    goToCell("A1:A3");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("C1");

    dispatchCommand(mxComponent, ".uno:PasteAsLink", {});

    OUString aFormula = pDoc->GetFormula(2, 0, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=$Sheet1.$A$1"), aFormula); // C1
    aFormula = pDoc->GetFormula(2, 1, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=$Sheet1.$A$2"), aFormula); // C2
    aFormula = pDoc->GetFormula(2, 2, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=$Sheet1.$A$3"), aFormula); // C3
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(2, 0, 0)); // C1
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(2, 2, 0)); // C3

    CPPUNIT_ASSERT_MESSAGE("There should be no note on C1", !pDoc->HasNote(ScAddress(2, 0, 0)));

    pPattern = pDoc->GetPattern(2, 1, 0);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be normal (cell attributes should not be copied)",
                                 WEIGHT_NORMAL, aFont.GetWeight());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf131442)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertArrayToCell(*pModelObj, "A1:A5", "={6;4;2;5;3}");

    CPPUNIT_ASSERT_EQUAL(OUString("6"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("4"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, ".uno:SortAscending", {});

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : #REF!
    CPPUNIT_ASSERT_EQUAL(OUString("6"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("4"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), pDoc->GetString(ScAddress(0, 4, 0)));

    dispatchCommand(mxComponent, ".uno:SortDescending", {});

    CPPUNIT_ASSERT_EQUAL(OUString("6"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("4"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetString(ScAddress(0, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), pDoc->GetString(ScAddress(0, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf117458)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    sal_uInt16 bOldStatus = aInputOption.GetMoveDir();

    lcl_AssertCurrentCursorPosition(*pDoc, "A1");

    aInputOption.SetMoveDir(DIR_BOTTOM);
    pMod->SetInputOptions(aInputOption);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDoc, "A2");

    aInputOption.SetMoveDir(DIR_TOP);
    pMod->SetInputOptions(aInputOption);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDoc, "A1");

    aInputOption.SetMoveDir(DIR_RIGHT);
    pMod->SetInputOptions(aInputOption);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDoc, "B1");

    aInputOption.SetMoveDir(DIR_LEFT);
    pMod->SetInputOptions(aInputOption);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDoc, "A1");

    // Restore previous status
    aInputOption.SetMoveDir(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf90694)
{
    ScModelObj* pModelObj = createDoc("tdf90694.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Select row 30 to 60
    goToCell("30:60");

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 29, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 59, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("#REF!"), pDoc->GetString(ScAddress(0, 29, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#REF!"), pDoc->GetString(ScAddress(0, 59, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf138710)
{
    ScModelObj* pModelObj = createDoc("tdf138710.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 1)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Total"), pDoc->GetString(ScAddress(0, 0, 1)));

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 1)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Total"), pDoc->GetString(ScAddress(0, 0, 1)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf128914)
{
    ScModelObj* pModelObj = createDoc("tdf128914.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(6.0, pDoc->GetValue(ScAddress(3, 1, 0)));

    goToCell("D2");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("D3:D6");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without the fix in place, this test would have failed with
    // - Expected: 24
    // - Actual  : 6
    CPPUNIT_ASSERT_EQUAL(24.0, pDoc->GetValue(ScAddress(3, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(60.0, pDoc->GetValue(ScAddress(3, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(120.0, pDoc->GetValue(ScAddress(3, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(210.0, pDoc->GetValue(ScAddress(3, 5, 0)));

    CPPUNIT_ASSERT_EQUAL(6.0, pDoc->GetValue(ScAddress(1, 14, 0)));

    goToCell("B15");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("C15:F15");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(24.0, pDoc->GetValue(ScAddress(2, 14, 0)));
    CPPUNIT_ASSERT_EQUAL(60.0, pDoc->GetValue(ScAddress(3, 14, 0)));
    CPPUNIT_ASSERT_EQUAL(120.0, pDoc->GetValue(ScAddress(4, 14, 0)));
    CPPUNIT_ASSERT_EQUAL(210.0, pDoc->GetValue(ScAddress(5, 14, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf108654)
{
    ScModelObj* pModelObj = createDoc("tdf108654.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    insertNewSheet(*pDoc);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    OUString aFormula = pDoc->GetFormula(3, 126, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("=VLOOKUP(C127,#REF!,D$1,0)"), aFormula);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    aFormula = pDoc->GetFormula(3, 126, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf133326)
{
    ScModelObj* pModelObj = createDoc("tdf133326.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

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
    Scheduler::ProcessEventsToIdle();

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    aFormula = pDoc->GetFormula(0, 0, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("=RAND()*1000000"), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf126685)
{
    ScModelObj* pModelObj = createDoc("tdf126685.ods");

    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:SelectAll", {}); // test should crash here without the fix
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(
        OUString("Control Height will change from 0.65 to 0.61 cm with 120dpi ..."),
        pDoc->GetString(ScAddress(3, 1, 1)));
    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(3, 1, 1)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf119793)
{
    ScModelObj* pModelObj = createDoc("tdf119793.ods");

    uno::Reference<drawing::XDrawPage> xPage(pModelObj->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4984), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    // Move the shape to the right
    lcl_SelectObjectByName(u"Shape 1");
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    Scheduler::ProcessEventsToIdle();

    //position has changed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5084), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    // Type into the shape
    typeString(*pModelObj, "x");
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5084), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1381), xShape->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf131455)
{
    // Note that tdf#131455 and tdf#126904 were actually incorrect,
    // but keep the test with a fixed version of the document.
    ScModelObj* pModelObj = createDoc("tdf131455-fixed.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    lcl_AssertCurrentCursorPosition(*pDoc, "A5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "B5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "E5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "F5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "I5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "J5");
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "M5");

    //Cursor can't move forward to the right
    for (size_t i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:GoRight", {});
        lcl_AssertCurrentCursorPosition(*pDoc, "N5");
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), ScDocShell::GetViewData()->GetTabNo());

    dispatchCommand(mxComponent, ".uno:JumpToNextTable", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), ScDocShell::GetViewData()->GetTabNo());
    lcl_AssertCurrentCursorPosition(*pDoc, "A4");

    // Go to row 9
    for (size_t i = 0; i < 6; ++i)
    {
        dispatchCommand(mxComponent, ".uno:GoDown", {});
    }

    lcl_AssertCurrentCursorPosition(*pDoc, "A10");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    dispatchCommand(mxComponent, ".uno:JumpToPrevTable", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), ScDocShell::GetViewData()->GetTabNo());
    lcl_AssertCurrentCursorPosition(*pDoc, "N5");

    //Cursor can't move forward to the right
    for (size_t i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:GoRight", {});
        lcl_AssertCurrentCursorPosition(*pDoc, "N5");
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf124818)
{
    ScModelObj* pModelObj = createDoc("tdf124818.xls");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), ScDocShell::GetViewData()->GetTabNo());

    dispatchCommand(mxComponent, ".uno:JumpToPrevTable", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), ScDocShell::GetViewData()->GetTabNo());

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
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
    lcl_AssertCurrentCursorPosition(*pDoc, "D10");
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

    lcl_AssertCurrentCursorPosition(*pDoc, "A1");
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(2, 0, 0)));

    //Without the fix, it would crash
    dispatchCommand(mxComponent, ".uno:InsertColumnsBefore", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(3, 0, 0)));
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Rakennukset"), pDoc->GetString(ScAddress(2, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf142010)
{
    ScModelObj* pModelObj = createDoc("tdf142010.xls");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf132431)
{
    ScModelObj* pModelObj = createDoc("tdf132431.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    OUString aFormula = pDoc->GetFormula(7, 219, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMIFS($H$2:$H$198,B$2:B$198,G220)"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(7, 219, 0)));

    // Without the fix in place, it would crash here with
    // uncaught exception of type std::exception (or derived).
    // - vector::_M_fill_insert
    insertStringToCell(*pModelObj, "H220", "=SUMIFS($H$2:$DB$198,B$2:B$198,G220)");

    aFormula = pDoc->GetFormula(7, 219, 0);
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

    lcl_AssertCurrentCursorPosition(*pDoc, "A1");
    insertStringToCell(*pModelObj, "A2", "=ROW(A3)");
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    lcl_AssertCurrentCursorPosition(*pDoc, "A3");
    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});

    //Without the fix, it would be 3.0
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(ScAddress(0, 1, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf124822)
{
    ScModelObj* pModelObj = createDoc("tdf124822.xls");

    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("X"), pDoc->GetString(ScAddress(0, 0, 2)));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 2)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("X"), pDoc->GetString(ScAddress(0, 0, 2)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf118189)
{
    ScModelObj* pModelObj = createDoc("tdf118189.xlsx");

    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Select column A
    goToCell("A:A");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    mxComponent->dispose();

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    OUString aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    dispatchCommand(mxComponent, ".uno:Cut", {});

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf118207)
{
    ScModelObj* pModelObj = createDoc("tdf118189.xlsx");

    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

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
    Scheduler::ProcessEventsToIdle();

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    aFormula = pDoc->GetFormula(1, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    aFormula = pDoc->GetFormula(0, 77, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=FALSE()"), aFormula);

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf148669)
{
    // Without the fix in place, this test would have failed with an assert
    ScModelObj* pModelObj = createDoc("tdf148669.xlsx");

    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_MESSAGE("There should be a note", pDoc->HasNote(ScAddress(701, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf124778)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Add a new comment
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Comment")) } });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:ShowAnnotations", {});

    ScPostIt* pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(true, pNote->IsCaptionShown());

    dispatchCommand(mxComponent, ".uno:ShowAnnotations", {});

    CPPUNIT_ASSERT_EQUAL(false, pNote->IsCaptionShown());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf138428)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    lcl_AssertCurrentCursorPosition(*pDoc, "A1");

    // Add a new comment
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Comment")) } });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Copy", {});

    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDoc, "B1");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on B1", pDoc->HasNote(ScAddress(1, 0, 0)));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on A1", !pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be a note on B1", !pDoc->HasNote(ScAddress(1, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on B1", pDoc->HasNote(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf136113)
{
    ScModelObj* pModelObj = createDoc("tdf136113.xlsx");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    SdrObject* pObj = pPage->GetObj(0);

    CPPUNIT_ASSERT_EQUAL(tools::Long(18142), pObj->GetSnapRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1709), pObj->GetSnapRect().Top());

    lcl_SelectObjectByName(u"Arrow");

    // Move the shape up
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::UP);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::UP);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(tools::Long(18142), pObj->GetSnapRect().Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1609), pObj->GetSnapRect().Top());

    // Without the fix in place, this test would have failed here
    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    CPPUNIT_ASSERT(pDocSh->IsModified());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf130614)
{
    ScModelObj* pModelObj = createDoc("tdf130614.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    lcl_SelectObjectByName(u"Object 1");

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    mxComponent->dispose();

    // Open a new document
    mxComponent = loadFromDesktop("private:factory/scalc");
    pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf112735)
{
    ScModelObj* pModelObj = createDoc("tdf112735.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("(empty)"), pDoc->GetString(ScAddress(1, 0, 0)));

    goToCell("B3");

    dispatchCommand(mxComponent, ".uno:RecalcPivotTable", {});

    // Without the fix in place, this test would have failed with
    // - Expected: (empty)
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("(empty)"), pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf112884)
{
    ScModelObj* pModelObj = createDoc("tdf112884.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf133342)
{
    ScModelObj* pModelObj = createDoc("tdf133342.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf71339)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A2", "1");
    insertStringToCell(*pModelObj, "A3", "1");

    goToCell("A1:A3");

    dispatchCommand(mxComponent, ".uno:AutoSum", {});

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    OUString aFormula = pDoc->GetFormula(0, 3, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: =SUM(A1:A3)
    // - Actual  : =SUM(A2:A3)
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:A3)"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf116421)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "1");
    insertStringToCell(*pModelObj, "A2", "1");
    insertStringToCell(*pModelObj, "A3", "1");

    goToCell("A4");

    dispatchCommand(mxComponent, ".uno:AutoSum", {});

    // Use RETURN key to exit autosum edit view
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: 3
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 3, 0)));

    OUString aFormula = pDoc->GetFormula(0, 3, 0);

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:A3)"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf86305)
{
    ScModelObj* pModelObj = createDoc("tdf86305.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    OUString aFormula = pDoc->GetFormula(1, 6, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("{=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(1, 6, 0)));

    insertStringToCell(*pModelObj, "B3", "50");
    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(1, 2, 0)));

    aFormula = pDoc->GetFormula(1, 6, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: {=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}
    // - Actual  : {=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0.175)}
    CPPUNIT_ASSERT_EQUAL(OUString("{=IF(SUM(B2:B4) > 0, SUM(B2:B4*D2:D4/C2:C4), 0)}"), aFormula);
    CPPUNIT_ASSERT_EQUAL(0.175, pDoc->GetValue(ScAddress(1, 6, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf81351)
{
    ScModelObj* pModelObj = createDoc("tdf81351.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf123202)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "1");
    insertStringToCell(*pModelObj, "A2", "2");
    insertStringToCell(*pModelObj, "A3", "3");
    insertStringToCell(*pModelObj, "A4", "4");

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf134675)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    insertStringToCell(*pModelObj, "A1", "A");

    // Select column A
    goToCell("A:A");

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    // Select column B to Z
    goToCell("B:Z");

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    for (size_t i = 1; i < 24; ++i)
    {
        // Without the fix in place, this test would have failed here with
        // - Expected: A
        // - Actual  :
        CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(i, 0, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf116215)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    insertStringToCell(*pModelObj, "A1", "1");
    insertStringToCell(*pModelObj, "A2", "1");
    insertStringToCell(*pModelObj, "B1", "1");
    insertStringToCell(*pModelObj, "B2", "1");
    goToCell("A1:C3");
    dispatchCommand(mxComponent, ".uno:AutoSum", {});

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf99913)
{
    ScModelObj* pModelObj = createDoc("tdf99913.xlsx");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT(pDoc->RowFiltered(2, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf126540_GridToggleModifiesTheDocument)
{
    ScModelObj* pModelObj = createDoc("tdf99913.xlsx");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Toggling the grid of a sheet, must set the document modified state
    ScDocShell* pDocSh = ScDocShell::GetViewData()->GetDocShell();
    CPPUNIT_ASSERT(!pDocSh->IsModified());
    dispatchCommand(mxComponent, ".uno:ToggleSheetGrid", {});
    CPPUNIT_ASSERT(pDocSh->IsModified());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf118983)
{
    ScModelObj* pModelObj = createDoc("tdf118983.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf107952)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "B1", "=SUM(A1:A2)");

    goToCell("D10");

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 3
    // - Incorrect Column in position B1
    lcl_AssertCurrentCursorPosition(*pDoc, "B1");

    goToCell("D10");

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDoc, "B1");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf144022)
{
    ScModelObj* pModelObj = createDoc("tdf144022.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    goToCell("A5:B79");

    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    goToCell("D5");

    //Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:PasteTransposed", {});
    Scheduler::ProcessEventsToIdle();

    for (size_t i = 3; i < 76; ++i)
    {
        OUString sExpected = "A" + OUString::number(i + 2);
        CPPUNIT_ASSERT_EQUAL(sExpected, pDoc->GetString(ScAddress(i, 4, 0)));
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf99386)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "B1", "This");
    insertStringToCell(*pModelObj, "B2", "=B1");

    goToCell("A1:B1");

    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(1, 1, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("This"), pDoc->GetString(ScAddress(1, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: This
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(OUString("This"), pDoc->GetString(ScAddress(1, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf149378)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "=MINVERSE(A1:C3)");

    // Without the fix in place, this test would have failed with
    // - Expected: {=MINVERSE(A1:C3)}
    // - Actual  : =MINVERSE(A1:C3)
    CPPUNIT_ASSERT_EQUAL(OUString("{=MINVERSE(A1:C3)}"), pDoc->GetFormula(0, 0, 0));

    insertStringToCell(*pModelObj, "B1", "={1;2}");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}}"), pDoc->GetFormula(1, 0, 0));

    insertStringToCell(*pModelObj, "C1", "={1;2}+3");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}+3}"), pDoc->GetFormula(2, 0, 0));

    insertStringToCell(*pModelObj, "D1", "={1;2}+{3;4}");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}+{3;4}}"), pDoc->GetFormula(3, 0, 0));

    insertStringToCell(*pModelObj, "E1", "={1;2}+A1");
    CPPUNIT_ASSERT_EQUAL(OUString("{={1;2}+A1}"), pDoc->GetFormula(4, 0, 0));

    insertStringToCell(*pModelObj, "F1", "={1;2}+A1:A2");
    CPPUNIT_ASSERT_EQUAL(OUString("={1;2}+A1:A2"), pDoc->GetFormula(5, 0, 0));

    insertStringToCell(*pModelObj, "G1", "=SUM(MUNIT(3))");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(MUNIT(3))"), pDoc->GetFormula(6, 0, 0));

    insertStringToCell(*pModelObj, "H1", "=SUM({1;2})");
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM({1;2})"), pDoc->GetFormula(7, 0, 0));

    insertStringToCell(*pModelObj, "I1", "=ABS({-1;-2})");
    CPPUNIT_ASSERT_EQUAL(OUString("{=ABS({-1;-2})}"), pDoc->GetFormula(8, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf126926)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    insertStringToCell(*pModelObj, "A1", "1");
    insertStringToCell(*pModelObj, "A2", "2");
    insertStringToCell(*pModelObj, "B1", "3");
    insertStringToCell(*pModelObj, "B2", "4");

    ScDBData* pDBData = new ScDBData("testDB", 0, 0, 0, 1, 1);
    bool bInserted
        = pDoc->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));
    CPPUNIT_ASSERT(bInserted);

    goToCell("A1:B1");

    dispatchCommand(mxComponent, ".uno:DeleteColumns", {});

    ScDBCollection* pDBs = pDoc->GetDBCollection();
    CPPUNIT_ASSERT(pDBs->empty());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testUnallocatedColumnsAttributes)
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    CPPUNIT_ASSERT_EQUAL(INITIALCOLCOUNT, pDoc->GetAllocatedColumnsCount(0));

    // Except for first 1 cell make the entire first row bold.
    assert(INITIALCOLCOUNT >= 1);
    goToCell("B1:" + pDoc->MaxColAsString() + "1");
    dispatchCommand(mxComponent, ".uno:Bold", {});

    // That shouldn't need allocating more columns, just changing the default attribute.
    CPPUNIT_ASSERT_EQUAL(INITIALCOLCOUNT, pDoc->GetAllocatedColumnsCount(0));
    vcl::Font aFont;
    pDoc->GetPattern(pDoc->MaxCol(), 0, 0)->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());

    goToCell("A2:CV2"); // first 100 cells in row 2
    dispatchCommand(mxComponent, ".uno:Bold", {});
    // These need to be explicitly allocated.
    CPPUNIT_ASSERT_EQUAL(SCCOL(100), pDoc->GetAllocatedColumnsCount(0));
    pDoc->GetPattern(99, 1, 0)->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
    pDoc->GetPattern(100, 1, 0)->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should not be bold", WEIGHT_NORMAL, aFont.GetWeight());

    goToCell("CW3:" + pDoc->MaxColAsString() + "3"); // All but first 100 cells in row 3.
    dispatchCommand(mxComponent, ".uno:Bold", {});
    // First 100 columns need to be allocated to not be bold, the rest should be handled
    // by the default attribute.
    CPPUNIT_ASSERT_EQUAL(SCCOL(100), pDoc->GetAllocatedColumnsCount(0));
    pDoc->GetPattern(99, 2, 0)->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should not be bold", WEIGHT_NORMAL, aFont.GetWeight());
    pDoc->GetPattern(100, 2, 0)->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
