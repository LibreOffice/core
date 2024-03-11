/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../helper/qahelper.hxx"
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <unotools/syslocaleoptions.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/scheduler.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <condformathelper.hxx>
#include <conditio.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <dpobject.hxx>
#include <formulaopt.hxx>
#include <inputopt.hxx>
#include <postit.hxx>
#include <rangeutl.hxx>
#include <scmod.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>

using namespace ::com::sun::star;

class ScUiCalcTest : public ScModelTestBase
{
public:
    ScUiCalcTest();
};

ScUiCalcTest::ScUiCalcTest()
    : ScModelTestBase("sc/qa/unit/uicalc/data")
{
}

static void lcl_AssertConditionalFormatList(ScDocument& rDoc, size_t nSize,
                                            std::unordered_map<OUString, OUString>& rExpectedValues)
{
    ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT_EQUAL(nSize, pList->size());

    OUString sRangeStr;
    for (const auto& rItem : *pList)
    {
        const ScRangeList& aRange = rItem->GetRange();
        aRange.Format(sRangeStr, ScRefFlags::VALID, rDoc, rDoc.GetAddressConvention());
        CPPUNIT_ASSERT_MESSAGE(OString(sRangeStr.toUtf8() + " not found").getStr(),
                               rExpectedValues.count(sRangeStr));
        CPPUNIT_ASSERT_EQUAL(rExpectedValues[sRangeStr],
                             ScCondFormatHelper::GetExpression(*rItem, aRange.GetTopLeftCorner()));
    }
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

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf142854_GridVisibilityImportXlsxInHeadlessMode)
{
    // Tests are running in Headless mode
    // Import an ods file with 'Hide' global grid visibility setting.
    createScDoc("tdf126541_GridOffGlobally.ods");
    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(!pDoc->GetViewOptions().GetOption(VOPT_GRID));

    // To avoid regression, in headless mode leave the bug tdf126541
    // It means Sheet based grid line visibility setting will overwrite the global setting.
    // If there is only 1 sheet in the document, it will not result visible problems.
    createScDoc("tdf126541_GridOff.xlsx");
    pDoc = getScDoc();
    CPPUNIT_ASSERT(!pDoc->GetViewOptions().GetOption(VOPT_GRID));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testExternalReferences)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"2015");
    insertStringToCell("A2", u"2015");
    insertStringToCell("A3", u"2015");

    insertStringToCell("B1", u"1");
    insertStringToCell("B2", u"1");
    insertStringToCell("B3", u"2");

    insertStringToCell("C1", u"10");
    insertStringToCell("C2", u"20");
    insertStringToCell("C3", u"5");

    insertStringToCell("D1", u"BIG FISH");
    insertStringToCell("D2", u"FISHFISH");
    insertStringToCell("D3", u"FISHY");

    // Save the document
    save("calc8");

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    // Insert the references to the external document
    {
        // tdf#115162
        OUString aFormula = "=SUMIFS('" + maTempFile.GetURL() + "'#$Sheet1.C1:C3,'"
                            + maTempFile.GetURL() + "'#$Sheet1.B1:B3,1,'" + maTempFile.GetURL()
                            + "'#$Sheet1.A1:A3,2015)";
        insertStringToCell("A1", aFormula);

        // tdf#115162: Without the fix in place, this test would have failed with
        // - Expected: 30
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("30"), pDoc->GetString(ScAddress(0, 0, 0)));
    }

    {
        // tdf#114820
        OUString aFormula = "=VLOOKUP('" + maTempFile.GetURL() + "'#$Sheet1.A1;'"
                            + maTempFile.GetURL() + "'#$Sheet1.A1:B3,2,0)";
        insertStringToCell("A1", aFormula);

        // Without the fix in place, this test would have failed with
        // - Expected: 1
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(0, 0, 0)));
    }

    {
        // tdf#116149
        OUString aFormula = "=VAR('" + maTempFile.GetURL() + "'#$Sheet1.C1;'" + maTempFile.GetURL()
                            + "'#$Sheet1.C2)";
        insertStringToCell("A1", aFormula);

        // Without the fix in place, this test would have failed with
        // - Expected: 50
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("50"), pDoc->GetString(ScAddress(0, 0, 0)));
    }

    {
        // tdf#100847
        // Use an empty cell
        OUString aFormula = "=+'" + maTempFile.GetURL() + "'#$Sheet1.A1000";
        insertStringToCell("A1", aFormula);

        aFormula = "=+'" + maTempFile.GetURL() + "'#$Sheet1.A1000*1";
        insertStringToCell("B1", aFormula);

        aFormula = "=+N('" + maTempFile.GetURL() + "'#$Sheet1.A1000)*1";
        insertStringToCell("C1", aFormula);

        CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(0, 0, 0)));

        // Without the fix in place, this test would have failed with
        // - Expected: 0
        // - Actual  : #VALUE!
        CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(1, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(2, 0, 0)));
    }

    {
        //tdf#36387
        OUString aAndFormula = "=AND('" + maTempFile.GetURL() + "'#$Sheet1.A1:C1)";
        insertStringToCell("A1", aAndFormula);

        OUString aOrFormula = "=OR('" + maTempFile.GetURL() + "'#$Sheet1.A1:C1)";
        insertStringToCell("B1", aOrFormula);

        // Without the fix in place, this test would have failed with
        // - Expected: TRUE
        // - Actual  : Err:504
        CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), pDoc->GetString(ScAddress(1, 0, 0)));
    }

    {
        //tdf#113898
        OUString aAndFormula = "=SUMPRODUCT(NOT(ISERROR(FIND(\"FISH\";'" + maTempFile.GetURL()
                               + "'#$Sheet1.D1:D3))))";
        insertStringToCell("A1", aAndFormula);

        // Without the fix in place, this test would have failed with
        // - Expected: 3
        // - Actual  : 1
        CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(ScAddress(0, 0, 0)));
    }
}

#if !defined(MACOSX) && !defined(_WIN32) //FIXME
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf151886)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    // Set the system locale to German
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("de-DE");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    insertStringToCell("A1", u"=1,1");

    insertStringToCell("A2", u"=1.1");

    CPPUNIT_ASSERT_EQUAL(OUString("1,1"), pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("#NAME?"), pDoc->GetString(0, 1, 0));

    // change UseEnglishFuncName to true
    ScDocShell* pDocSh = getScDocShell();
    ScFormulaOptions aFormulaOptions = SC_MOD()->GetFormulaOptions();
    bool bOldStatus = aFormulaOptions.GetUseEnglishFuncName();
    aFormulaOptions.SetUseEnglishFuncName(true);
    pDocSh->SetFormulaOptions(aFormulaOptions);

    insertStringToCell("A1", u"=1,1");

    insertStringToCell("A2", u"=1.1");

    // Without the fix in place, this test would have failed with
    // - Expected: 1,1
    // - Actual  : #NAME?
    CPPUNIT_ASSERT_EQUAL(OUString("1,1"), pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("#NAME?"), pDoc->GetString(0, 1, 0));

    aFormulaOptions.SetUseEnglishFuncName(bOldStatus);
    pDocSh->SetFormulaOptions(aFormulaOptions);
}
#endif

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf103994)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");
    insertStringToCell("B1", u"2");

    // Save the document
    save("calc8");

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    // Insert the reference to the external document
    OUString aFormula = "='" + maTempFile.GetURL() + "'#$Sheet1.A1";
    insertStringToCell("A1", aFormula);

    CPPUNIT_ASSERT_EQUAL(aFormula, pDoc->GetFormula(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    goToCell("A1");

    dispatchCommand(mxComponent, ".uno:Cut", {});

    goToCell("B1");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without the fix in place, this test would have failed with
    // - Expected: ='file:///tmp/lu124171irlmb.tmp'#$Sheet1.A1
    // - Actual  : ='file:///tmp/lu124171irlmb.tmp'#$Sheet1.B1
    CPPUNIT_ASSERT_EQUAL(aFormula, pDoc->GetFormula(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf113541)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"50");

    // Save the document
    save("calc8");

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    // Change grammar to Excel A1
    FormulaGrammarSwitch aFGSwitch(pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_A1);

    // Insert the reference to the external document
    OUString aFormula = "=['" + maTempFile.GetURL() + "']Sheet1!A1";
    insertStringToCell("A1", aFormula);

    // Without the fix in place, this test would have failed with
    // - Expected: 50
    // - Actual  : Err:507
    CPPUNIT_ASSERT_EQUAL(OUString("50"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf126577)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

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
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"A");
    insertStringToCell("A2", u"B");

    // Add a new comment to A1 and A2
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Comment")) } });

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A2", pDoc->HasNote(ScAddress(0, 1, 0)));

    goToCell("A1");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    for (size_t i = 0; i < 10; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("B"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be no note on A2", !pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Undo", {});

        CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("B"), pDoc->GetString(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A2", pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Redo", {});
    }

    dispatchCommand(mxComponent, ".uno:Undo", {});

    goToCell("A1:A2");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    for (size_t i = 0; i < 10; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be no note on A1", !pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be no note on A2", !pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Undo", {});

        CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString("B"), pDoc->GetString(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT_MESSAGE("There should be a note on A2", pDoc->HasNote(ScAddress(0, 1, 0)));

        dispatchCommand(mxComponent, ".uno:Redo", {});
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf63805)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"2012-10-31");

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

    CPPUNIT_ASSERT_EQUAL(OUString("2012-10-31"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf147894)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

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
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"=COUNTA(B:B)");
    insertStringToCell("A3", u"Range");
    insertStringToCell("A4", u"Range");
    insertStringToCell("A5", u"Range");
    insertStringToCell("A6", u"Range");
    insertStringToCell("A7", u"Range");
    insertStringToCell("A8", u"Range");
    insertStringToCell("B6", u"Test");

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    goToCell("A3:A8");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});

    //type Control-D/Fill Down
    dispatchCommand(mxComponent, ".uno:FillDown", {});

    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

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
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell("A3:A4");

    dispatchCommand(mxComponent, ".uno:HideRow", {});

    insertStringToCell("A2", u"1");

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
    createScDoc();
    ScDocument* pDoc = getScDoc();

    css::uno::Reference<css::sheet::XGlobalSheetSettings> xGlobalSheetSettings
        = css::sheet::GlobalSheetSettings::create(::comphelper::getProcessComponentContext());
    bool bOldValue = xGlobalSheetSettings->getDoAutoComplete();

    xGlobalSheetSettings->setDoAutoComplete(true);

    insertStringToCell("A1", u"ABC");

    insertStringToCell("A2", u"A");

    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), pDoc->GetString(ScAddress(0, 1, 0)));

    xGlobalSheetSettings->setDoAutoComplete(false);

    insertStringToCell("A3", u"A");

    // Without the fix in place, this test would have failed with
    // - Expected: A
    // - Actual  : ABC
    CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(0, 2, 0)));

    // Restore the previous value
    xGlobalSheetSettings->setDoAutoComplete(bOldValue);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf56036)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell("A1");

    typeString(u"=SUM( 1 + 2 ");

    // Insert Newline
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_MOD1 | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    typeString(u"+ 3)");

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : Err:501
    CPPUNIT_ASSERT_EQUAL(OUString("6"), pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf154174_repeat_empty)
{
    createScDoc();

    insertStringToCell("A1", u"aaaa");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | KEY_SHIFT | awt::Key::Y);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_MOD1 | KEY_SHIFT | awt::Key::Y);
    Scheduler::ProcessEventsToIdle();

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(OUString("aaaa"), pDoc->GetString(ScAddress(0, 1, 0)));

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::F2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::F2);
    Scheduler::ProcessEventsToIdle();

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
    Scheduler::ProcessEventsToIdle();

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: aaaa
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("aaaa"), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf119162)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell("A1");

    typeString(u"Test");

    // Insert Newline
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
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

    CPPUNIT_ASSERT_EQUAL(OUString(u"Test" + OUStringChar(u'\xA')),
                         pDoc->GetString(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, ".uno:ChangeCaseToLower", {});

    CPPUNIT_ASSERT_EQUAL(OUString(u"test" + OUStringChar(u'\xA')),
                         pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf90579)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"2300");
    insertStringToCell("A2", u"Libre");
    insertStringToCell("B1", u"10");
    insertStringToCell("B2", u"Office");
    insertStringToCell("C1", u"=SUM(A1:B1)");
    insertStringToCell("C2", u"=A2&B2");

    CPPUNIT_ASSERT_EQUAL(OUString("2310"), pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice"), pDoc->GetString(ScAddress(2, 1, 0)));

    goToCell("C1:C2");

    dispatchCommand(mxComponent, ".uno:ConvertFormulaToValue", {});

    CPPUNIT_ASSERT_EQUAL(OUString("2310"), pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice"), pDoc->GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetFormula(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetFormula(2, 1, 0));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("2310"), pDoc->GetString(ScAddress(2, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice"), pDoc->GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A1:B1)"), pDoc->GetFormula(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("=A2&B2"), pDoc->GetFormula(2, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf124820)
{
    createScDoc("tdf124820.xlsx");
    ScDocument* pDoc = getScDoc();

    goToCell("B2");

    dispatchCommand(mxComponent, ".uno:Strikeout", {});

    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();

    vcl::Font aFont;
    const ScPatternAttr* pPattern = pDoc->GetPattern(1, 1, 0);
    pPattern->fillFontOnly(aFont);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be striked out", STRIKEOUT_SINGLE,
                                 aFont.GetStrikeout());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf132026)
{
    createScDoc("tdf132026.ods");
    ScDocument* pDoc = getScDoc();
    std::vector<std::u16string_view> aChars{ u"=", u"+", u"-" };
    std::vector<sal_uInt16> aDirections{ KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT };

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    for (auto aChar = aChars.begin(); aChar != aChars.end(); ++aChar)
    {
        for (size_t i = 0; i < aDirections.size(); ++i)
        {
            goToCell("B2");
            typeString(*aChar);

            sal_uInt16 nDir = aDirections[i];
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, nDir);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, nDir);
            Scheduler::ProcessEventsToIdle();

            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
            Scheduler::ProcessEventsToIdle();

            sal_Int16 nSign = (*aChar == u"-") ? -1 : 1;
            sal_Int16 nExpected = nSign * (i + 1);
            OUString sExpectedResult = OUString::number(nExpected);
            CPPUNIT_ASSERT_EQUAL(sExpectedResult, pDoc->GetString(ScAddress(1, 1, 0)));

            goToCell("E2");
            typeString(*aChar);

            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, nDir);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, nDir);
            Scheduler::ProcessEventsToIdle();

            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
            Scheduler::ProcessEventsToIdle();

            // Without the fix in place, this test would have failed with
            // - Expected: =
            // - Actual  : =E1
            CPPUNIT_ASSERT_EQUAL(OUString(*aChar), pDoc->GetString(ScAddress(4, 1, 0)));
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf152037)
{
    createScDoc("tdf152037.xlsx");

    insertStringToCell("C2", u"abcdefghi");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: abcdefghi
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("abcdefghi"), pDoc->GetString(ScAddress(2, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf119155)
{
    createScDoc("tdf119155.xlsx");
    ScDocument* pDoc = getScDoc();

    goToCell("C2:C14");

    // Without the fix in place, this test would have hung here
    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Collagene Expert Targeted Wrinkle Corrector Unboxed 10 Ml"),
                         pDoc->GetString(ScAddress(2, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Vitamina Suractivee Hand Cream 400 Ml"),
                         pDoc->GetString(ScAddress(2, 13, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf146795)
{
    createScDoc("tdf146795.ods");
    ScDocument* pDoc = getScDoc();

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    goToCell("B2");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Move to B3
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Select the three following cells
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});

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
    createScDoc("tdf147744.ods");
    ScDocument* pDoc = getScDoc();

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    goToCell("A2");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Move to A3
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Select the following cell
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});

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
    createScDoc("tdf138432.ods");
    ScDocument* pDoc = getScDoc();

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

    goToCell("A2");

    typeString(u"=");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
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
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A2000", u"Test");

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), pDoc->GetString(ScAddress(0, 1999, 0)));

    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: Test
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("Test"), pDoc->GetString(ScAddress(0, 1999, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf145085)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertArrayToCell("A1", u"=HYPERLINK(\"a\";\"b\")");

    CPPUNIT_ASSERT_EQUAL(OUString("b"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : a
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf154125)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertArrayToCell("A1", u"=INDEX({11;21},0,1)");

    CPPUNIT_ASSERT_EQUAL(OUString("11"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: 21
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("21"), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf159046)
{
    createScDoc("tdf159046.ods");
    ScDocument* pDoc = getScDoc();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();

    std::map<SCROW, std::vector<SdrObject*>> aObjects
        = pDrawLayer->GetObjectsAnchoredToRange(0, 0, 6, 8);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aObjects.size());

    aObjects = pDrawLayer->GetObjectsAnchoredToRange(0, 1, 6, 8);
    CPPUNIT_ASSERT_EQUAL(size_t(0), aObjects.size());

    goToCell("A2:F10");
    dispatchCommand(mxComponent, ".uno:Cut", {});

    goToCell("B2");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without the fix in place, this test would have crashed here
    saveAndReload("calc8");

    pDoc = getScDoc();
    pDrawLayer = pDoc->GetDrawLayer();

    aObjects = pDrawLayer->GetObjectsAnchoredToRange(0, 0, 6, 8);
    CPPUNIT_ASSERT_EQUAL(size_t(0), aObjects.size());

    aObjects = pDrawLayer->GetObjectsAnchoredToRange(0, 1, 6, 8);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aObjects.size());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf148863)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertArrayToCell("A1", u"=TRANSPOSE(IF({0|0|0}=0;RANDBETWEEN.NV(1;1000000)))");

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
    createScDoc("tdf144244.ods");
    ScDocument* pDoc = getScDoc();

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    uno::Reference<drawing::XDrawPage> xPage(pModelObj->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xPage->getCount());

    // Select column A and B
    goToCell("A:B");

    dispatchCommand(mxComponent, ".uno:DeleteColumns", {});

    CPPUNIT_ASSERT_EQUAL(OUString("x"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have crashed
    saveAndReload("calc8");
    pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("x"), pDoc->GetString(ScAddress(0, 0, 0)));

    xPage.set(pModelObj->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xPage->getCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf153669)
{
    createScDoc("tdf153669.ods");
    ScDocument* pDoc = getScDoc();

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    insertStringToCell("E2", u"100");

    CPPUNIT_ASSERT_EQUAL(OUString("110.00"), pDoc->GetString(ScAddress(5, 1, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("15.00"), pDoc->GetString(ScAddress(5, 1, 0)));

    goToCell("E7");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("F7");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("text"), pDoc->GetString(ScAddress(5, 6, 0)));

    insertStringToCell("E2", u"100");

    // Without the fix in place, this test would have failed with
    // - Expected: 110.00
    // - Actual  : 15.00
    CPPUNIT_ASSERT_EQUAL(OUString("110.00"), pDoc->GetString(ScAddress(5, 1, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("15.00"), pDoc->GetString(ScAddress(5, 1, 0)));

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf100582)
{
    createScDoc("tdf100582.xls");
    ScDocument* pDoc = getScDoc();

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

    saveAndReload("MS Excel 97");
    pDoc = getScDoc();

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
    createScDoc("tdf145640.ods");
    ScDocument* pDoc = getScDoc();

    // Enable sorting with update reference
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetSortRefUpdate();
    aInputOption.SetSortRefUpdate(true);
    pMod->SetInputOptions(aInputOption);

    goToCell("A2:F17");

    dispatchCommand(mxComponent, ".uno:SortDescending", {});

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A15:B15:C15:D15:E15:F15)"), pDoc->GetFormula(6, 3, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: 10
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(6, 3, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A4:B4:C4:D4:E4:F4)"), pDoc->GetFormula(6, 3, 0));
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(6, 3, 0)));

    // Restore previous status
    aInputOption.SetSortRefUpdate(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf97215)
{
    createScDoc("tdf97215.ods");
    ScDocument* pDoc = getScDoc();

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

    CPPUNIT_ASSERT_EQUAL(OUString("Inserted at bottom"), pDoc->GetString(ScAddress(0, 0, 0)));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Inserted at bottom"), pDoc->GetString(ScAddress(0, 23, 0)));

    // Restore previous status
    aInputOption.SetSortRefUpdate(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf104026)
{
    createScDoc("tdf104026.ods");
    ScDocument* pDoc = getScDoc();

    std::unordered_map<OUString, OUString> aExpectedValues
        = { { "A2", "Cell value != $Sheet1.$B2" }, { "A3", "Cell value != $Sheet1.$B3" },
            { "A4", "Cell value != $Sheet1.$B4" }, { "A5", "Cell value != $Sheet1.$B5" },
            { "A6", "Cell value != $Sheet1.$B6" }, { "A7", "Cell value != $Sheet1.$B7" } };

    lcl_AssertConditionalFormatList(*pDoc, 6, aExpectedValues);

    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Without the fix in place, this test would have failed with
    // - Expected: Cell value != $Sheet1.$B2
    // - Actual  : Cell value != $Sheet1.$B#REF!
    lcl_AssertConditionalFormatList(*pDoc, 5, aExpectedValues);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    // tdf#140330: Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 5
    lcl_AssertConditionalFormatList(*pDoc, 6, aExpectedValues);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf92963)
{
    createScDoc("tdf92963.ods");
    ScDocument* pDoc = getScDoc();

    // Disable replace cell warning
    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    bool bOldStatus = aInputOption.GetReplaceCellsWarn();
    aInputOption.SetReplaceCellsWarn(false);
    pMod->SetInputOptions(aInputOption);

    std::unordered_map<OUString, OUString> aExpectedValues
        = { { "C1", "Cell value > 14" }, { "C3", "Cell value > 14" }, { "C4", "Cell value > 14" } };

    lcl_AssertConditionalFormatList(*pDoc, 3, aExpectedValues);

    goToCell("A3:C4");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("A1:C1");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    aExpectedValues = { { "C3,C1", "Cell value > 14" }, { "C4,C2", "Cell value > 14" } };
    lcl_AssertConditionalFormatList(*pDoc, 2, aExpectedValues);

    // Restore previous status
    aInputOption.SetReplaceCellsWarn(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

#if !defined(MACOSX) && !defined(_WIN32) //FIXME
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf140151)
{
    createScDoc("tdf140151.ods");
    ScDocShell* pDocSh = getScDocShell();

    // Focus is already on the button
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the current cursor position wouldn't have changed
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B111");
}
#endif

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf68290)
{
    createScDoc("tdf68290.ods");
    ScDocShell* pDocSh = getScDocShell();

    const std::vector<OUString> aExpectedAddresses{ "L3", "L6", "L9", "L10", "L11", "L13", "L15" };

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    for (const auto& rAddress : aExpectedAddresses)
    {
        lcl_AssertCurrentCursorPosition(*pDocSh, rAddress);

        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    }

    lcl_AssertCurrentCursorPosition(*pDocSh, u"M3");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf156462)
{
    createScDoc("tdf156462.ods");
    ScDocShell* pDocSh = getScDocShell();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"G2");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | KEY_LEFT);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the cursor would have jumped to cell C2
    lcl_AssertCurrentCursorPosition(*pDocSh, u"D2");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf132057)
{
    createScDoc("tdf132057.ods");
    ScDocShell* pDocSh = getScDocShell();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"AU43");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the cursor would have jumped to cell BM1
    lcl_AssertCurrentCursorPosition(*pDocSh, u"G39");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf122232)
{
    createScDoc("tdf122232.ods");
    ScDocShell* pDocSh = getScDocShell();

    //Start with from C6. Press tabulator to reach G6.
    lcl_AssertCurrentCursorPosition(*pDocSh, u"C6");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    lcl_AssertCurrentCursorPosition(*pDocSh, u"G6");

    //without the fix, cursor would jump to C29 instead of C7.
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    lcl_AssertCurrentCursorPosition(*pDocSh, u"C7");
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf123052)
{
    createScDoc("tdf123052.ods");
    ScDocShell* pDocSh = getScDocShell();

    std::vector<OUString> aExpectedAddresses{ "F3", "D5", "E5", "F6", "A8", "E9" };

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    for (const auto& rAddress : aExpectedAddresses)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
        Scheduler::ProcessEventsToIdle();

        lcl_AssertCurrentCursorPosition(*pDocSh, rAddress);
    }

    aExpectedAddresses.pop_back();

    for (std::vector<OUString>::reverse_iterator it = aExpectedAddresses.rbegin();
         it != aExpectedAddresses.rend(); ++it)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | awt::Key::TAB);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_SHIFT | awt::Key::TAB);
        Scheduler::ProcessEventsToIdle();

        // Without the fix in place, this test would have failed here
        lcl_AssertCurrentCursorPosition(*pDocSh, *it);
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf120660)
{
    createScDoc("tdf120660.ods");
    ScDocument* pDoc = getScDoc();

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
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    goToCell("B3");
    lcl_AssertCurrentCursorPosition(*pDocSh, u"B3");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"D3");

    dispatchCommand(mxComponent, ".uno:Paste", {});

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    ScRangeList aMarkedArea = getViewShell()->GetViewData().GetMarkData().GetMarkedRanges();
    OUString aMarkedAreaString;
    ScDocument* pDoc = getScDoc();
    ScRangeStringConverter::GetStringFromRangeList(aMarkedAreaString, &aMarkedArea, pDoc,
                                                   formula::FormulaGrammar::CONV_OOO);

    // Without the fix in place, this test would have failed with
    // - Expected: Sheet1.D3:Sheet1.D4
    // - Actual  : Sheet1.A2:Sheet1.D3
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1.D3:Sheet1.D4"), aMarkedAreaString);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf157897_duplicated_print_ranges)
{
    createScDoc("tdf157897_main.ods");
    ScDocument* pDoc = getScDoc();

    const sal_uInt16 nPos = 0;
    const SCTAB nFirstTab = 0;
    // Ensure that there exists a print range in the first documents tab
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pDoc->GetPrintRangeCount(nFirstTab));

    // Reload all links and check if the print range was not duplicated
    const auto initialPrintRange = pDoc->GetPrintRange(nFirstTab, nPos);
    pDoc->GetDocumentShell()->ReloadAllLinks();

    // Without the fix in place, the print range in the linked tab will be duplicated
    CPPUNIT_ASSERT_EQUAL(initialPrintRange, pDoc->GetPrintRange(nFirstTab, nPos));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf154991)
{
    createScDoc("tdf154991.ods");
    ScDocument* pDoc = getScDoc();

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:SelectColumn", {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:HideColumn", {});
    CPPUNIT_ASSERT(pDoc->ColHidden(0, 0));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT(!pDoc->ColHidden(0, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf151752)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:SelectUnprotectedCells", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    goToCell("B1");

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(1, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf95306)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    // Use Adding Selection
    dispatchCommand(mxComponent, ".uno:StatusSelectionModeExp", {});

    goToCell("B1");
    dispatchCommand(mxComponent, ".uno:SelectColumn", {});

    dispatchCommand(mxComponent, ".uno:GoRight", {});
    dispatchCommand(mxComponent, ".uno:GoRight", {});

    dispatchCommand(mxComponent, ".uno:SelectColumn", {});

    dispatchCommand(mxComponent, ".uno:HideColumn", {});

    CPPUNIT_ASSERT(!pDoc->ColHidden(0, 0));
    CPPUNIT_ASSERT(pDoc->ColHidden(1, 0));
    CPPUNIT_ASSERT(!pDoc->ColHidden(2, 0));
    CPPUNIT_ASSERT(pDoc->ColHidden(3, 0));
    CPPUNIT_ASSERT(!pDoc->ColHidden(4, 0));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT(!pDoc->ColHidden(0, 0));
    CPPUNIT_ASSERT(!pDoc->ColHidden(1, 0));
    CPPUNIT_ASSERT(!pDoc->ColHidden(2, 0));
    CPPUNIT_ASSERT(!pDoc->ColHidden(3, 0));
    CPPUNIT_ASSERT(!pDoc->ColHidden(4, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf102525)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"2");
    insertStringToCell("A3", u"3");
    insertStringToCell("A4", u"4");

    insertArrayToCell("B1", u"=IF(A1:A4>2,1,2)");
    goToCell("B1:B4");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F4);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(1, 3, 0));

    CPPUNIT_ASSERT_EQUAL(OUString("{=IF($A$1:$A$4>2,1,2)}"), pDoc->GetFormula(1, 0, 0));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(1, 3, 0));

    CPPUNIT_ASSERT_EQUAL(OUString("{=IF(A1:A4>2,1,2)}"), pDoc->GetFormula(1, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf39650)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    FormulaGrammarSwitch aFGSwitch(pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    insertStringToCell("R3C3", u"xxx");
    insertStringToCell("R2C2", u"=R[1]C[1]");

    CPPUNIT_ASSERT_EQUAL(OUString("=R[1]C[1]"), pDoc->GetFormula(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("xxx"), pDoc->GetString(1, 1, 0));

    goToCell("B2");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F4);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("=R3C3"), pDoc->GetFormula(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("xxx"), pDoc->GetString(1, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf45020)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    goToCell("A2:A3");

    dispatchCommand(mxComponent, ".uno:HideRow", {});

    goToCell("A1");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    ScRangeList aMarkedArea = getViewShell()->GetViewData().GetMarkData().GetMarkedRanges();
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
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    insertStringToCell("A1", u"A1");
    insertStringToCell("A3", u"A3");

    // Use Adding Selection
    dispatchCommand(mxComponent, ".uno:StatusSelectionModeExp", {});

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:SelectRow", {});

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"A3");

    dispatchCommand(mxComponent, ".uno:SelectRow", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Open a new document
    createScDoc();
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without the fix in place, this test would have failed with
    // - Expected: A1
    // - Actual  : A3
    CPPUNIT_ASSERT_EQUAL(OUString("A1"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("A3"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 2, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf159373)
{
    createScDoc("tdf159373.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf86166)
{
    createScDoc("tdf86166.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf158802)
{
    createScDoc("tdf158802.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf149502_HangOnDeletingSheet1)
{
    createScDoc("tdf149502_HangOnDeletingSheet1.ods");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(4), pDoc->GetTableCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    // Before the fix in place, this test frozen here
    dispatchCommand(mxComponent, ".uno:Remove", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(3), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf149503)
{
    createScDoc("tdf149503.xls");
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});

    ScDPCollection* pDPs = pDoc->GetDPCollection();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDPs->GetCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf108292)
{
    createScDoc("tdf108292.ods");
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("2D"), pDoc->GetString(ScAddress(26, 0, 0)));
}

// Inspired from testTdf117706, test columns instead of rows
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testMultiRangeCol)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    insertStringToCell("A1", u"A1");
    insertStringToCell("C1", u"C1");

    // Use Adding Selection
    dispatchCommand(mxComponent, ".uno:StatusSelectionModeExp", {});

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:SelectColumn", {});

    dispatchCommand(mxComponent, ".uno:GoRight", {});
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    lcl_AssertCurrentCursorPosition(*pDocSh, u"C1");

    dispatchCommand(mxComponent, ".uno:SelectColumn", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Open a new document
    createScDoc();
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("A1"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("C1"), pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(2, 0, 0)));
}

// Test the call of .uno:PasteTransposed (tdf#102255)
// Note: the transpose functionality is tested in ucalc
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testPasteTransposed)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"a");
    insertStringToCell("A3", u"=A1");

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
    pPattern->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());

    goToCell("A1:A3");

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Open a new document
    createScDoc();
    pDoc = getScDoc();

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
    pPattern->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
}

// Test the call of .uno:PasteAsLink (tdf#90101)
// Note: the paste as link functionality is tested in ucalc
CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testPasteAsLink)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"1");
    insertStringToCell("A2", u"a");
    insertStringToCell("A3", u"=A1");

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
    pPattern->fillFontOnly(aFont);
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
    pPattern->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be normal (cell attributes should not be copied)",
                                 WEIGHT_NORMAL, aFont.GetWeight());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf119659)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertArrayToCell("A1:C1", u"={1,2,3}");

    insertStringToCell("A2", u"=LOOKUP(3; A1:C1; {1,2})");

    // Without the fix in place, this test would have crashed here
    CPPUNIT_ASSERT_EQUAL(OUString("#N/A"), pDoc->GetString(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf131442)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertArrayToCell("A1:A5", u"={6;4;2;5;3}");

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
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();

    ScModule* pMod = SC_MOD();
    ScInputOptions aInputOption = pMod->GetInputOptions();
    sal_uInt16 bOldStatus = aInputOption.GetMoveDir();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A1");

    aInputOption.SetMoveDir(DIR_BOTTOM);
    pMod->SetInputOptions(aInputOption);

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A2");

    aInputOption.SetMoveDir(DIR_TOP);
    pMod->SetInputOptions(aInputOption);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A1");

    aInputOption.SetMoveDir(DIR_RIGHT);
    pMod->SetInputOptions(aInputOption);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"B1");

    aInputOption.SetMoveDir(DIR_LEFT);
    pMod->SetInputOptions(aInputOption);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    lcl_AssertCurrentCursorPosition(*pDocSh, u"A1");

    // Restore previous status
    aInputOption.SetMoveDir(bOldStatus);
    pMod->SetInputOptions(aInputOption);
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf90694)
{
    createScDoc("tdf90694.ods");
    ScDocument* pDoc = getScDoc();

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
    createScDoc("tdf138710.ods");
    ScDocument* pDoc = getScDoc();

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
    createScDoc("tdf128914.ods");
    ScDocument* pDoc = getScDoc();

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
    createScDoc("tdf108654.ods");
    ScDocument* pDoc = getScDoc();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    insertNewSheet(*pDoc);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    OUString aFormula = pDoc->GetFormula(3, 126, 1);
    CPPUNIT_ASSERT_EQUAL(OUString("=VLOOKUP(C127,#REF!,D$1,0)"), aFormula);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    aFormula = pDoc->GetFormula(3, 126, 1);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aFormula);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(2), pDoc->GetTableCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SCTAB>(1), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf150219)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertNewSheet(*pDoc);

    insertStringToCell("A1", u"=$Sheet1.A1");
    goToCell("A1");

    CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(0, 0, 1)));

    dispatchCommand(mxComponent, ".uno:ShowPrecedents", {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 1)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf153790)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"=SUM($Sheet1.B1:C1)");

    CPPUNIT_ASSERT_EQUAL(OUString("0"), pDoc->GetString(ScAddress(0, 0, 0)));

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($Sheet1.B2:C2)"), pDoc->GetFormula(0, 1, 0));

    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:Cut", {});
    goToCell("A3");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without the fix in place, this test would have failed with
    // - Expected: =SUM($Sheet1.B1:C1)
    // - Actual  : =SUM($Sheet1.B1:$Sheet1.C1)
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM($Sheet1.B1:C1)"), pDoc->GetFormula(0, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf141440)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    // Insert a note to cell A1
    goToCell("A1");
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Note in A1")) } });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);

    // Insert a formula to cell A2
    insertStringToCell("A2", u"=1+1");
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(0, 1, 0)));

    // Copy content of A2 to A1 using paste special command as a formula (Flags F)
    goToCell("A2");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("A1");
    aArgs = comphelper::InitPropertySequence(
        { { "Flags", uno::Any(OUString("F")) },
          { "FormulaCommand", uno::Any(sal_uInt16(ScPasteFunc::ADD)) },
          { "SkipEmptyCells", uno::Any(false) },
          { "Transpose", uno::Any(false) },
          { "AsLink", uno::Any(false) },
          { "MoveMode", uno::Any(sal_uInt16(InsCellCmd::INS_NONE)) } });
    dispatchCommand(mxComponent, ".uno:InsertContents", aArgs);

    // Check if string in cell A2 was copied to cell A1
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(0, 0, 0)));
    // Without the fix in place, there would be no note in cell A1 after using paste special
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Note in A1"), pDoc->GetNote(ScAddress(0, 0, 0))->GetText());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf158551)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    insertStringToCell("A1", u"10");

    // Copy content of A1 to B1 using Formula Add
    goToCell("A1");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("B1");
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::InitPropertySequence(
        { { "Flags", uno::Any(OUString("SVD")) },
          { "FormulaCommand", uno::Any(sal_uInt16(ScPasteFunc::ADD)) },
          { "SkipEmptyCells", uno::Any(false) },
          { "Transpose", uno::Any(false) },
          { "AsLink", uno::Any(false) },
          { "MoveMode", uno::Any(sal_uInt16(InsCellCmd::INS_NONE)) } });

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:InsertContents", aArgs);

    CPPUNIT_ASSERT_EQUAL(OUString("10"), pDoc->GetString(ScAddress(1, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testTdf159174)
{
    createScDoc();

    dispatchCommand(mxComponent, ".uno:SelectUnprotectedCells", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::BITMAP)) } });

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    uno::Reference<drawing::XDrawPage> xPage(pModelObj->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xPage->getCount());
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testKeyboardMergeRef)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    goToCell("A1:A5");
    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});
    goToCell("A6:A10");
    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});

    goToCell("B1");
    typeString(u"=");

    goToCell("A1");

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pViewShell->IsRefInputMode());
    {
        const OUString* pInput = pViewShell->GetEditString();
        CPPUNIT_ASSERT(pInput);
        CPPUNIT_ASSERT_EQUAL(OUString("=A1:A10"), *pInput);
    }

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_UP | KEY_SHIFT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_UP | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pViewShell->IsRefInputMode());
    {
        const OUString* pInput = pViewShell->GetEditString();
        CPPUNIT_ASSERT(pInput);
        CPPUNIT_ASSERT_EQUAL(OUString("=A1:A5"), *pInput);
    }
}

CPPUNIT_TEST_FIXTURE(ScUiCalcTest, testMouseMergeRef)
{
    createScDoc();
    ScDocShell* pDocSh = getScDocShell();
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    goToCell("A1:A5");
    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});
    goToCell("A6:A10");
    dispatchCommand(mxComponent, ".uno:ToggleMergeCells", {});

    goToCell("B1");
    typeString(u"=");

    Point aA1 = pViewShell->GetViewData().GetPrintTwipsPos(0, 0);
    Point aA6 = pViewShell->GetViewData().GetPrintTwipsPos(0, 5);
    Point aA7 = pViewShell->GetViewData().GetPrintTwipsPos(0, 6);

    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aA1.X() + 5, aA1.Y() + 5, 1,
                              MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEMOVE, aA6.X() + 5, aA6.Y() + 5, 1, MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aA7.X() + 5, aA7.Y() + 5, 1, MOUSE_LEFT,
                              0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(pViewShell->IsRefInputMode());
    {
        const OUString* pInput = pViewShell->GetEditString();
        CPPUNIT_ASSERT(pInput);
        CPPUNIT_ASSERT_EQUAL(OUString("=A1:A10"), *pInput);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
