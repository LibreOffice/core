/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <address.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <tabvwsh.hxx>

#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <comphelper/propertyvalue.hxx>

using namespace css;

namespace
{

class CalculateSheetTest : public ScModelTestBase
{
public:
    CalculateSheetTest()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }

protected:
    cpo::uno::Any calculateSheet(sal_Int32 nTab)
    {
        cpo::uno::Sequence<beans::PropertyValue> aArguments{ comphelper::makePropertyValue(
            u"Tab"_ustr, nTab) };
        return dispatchCommand(mxComponent, u".uno:CalculateSheet"_ustr, aArguments);
    }
};

constexpr SCCOL nInputColumn = 0;
constexpr SCCOL nFormulaColumn = 1;

// The value planted over a formula's cached result. Far enough from any
// result the sheets below produce that a stale cell is unmistakable.
constexpr double fStaleResult = 999.0;

ScAddress inputCell(SCTAB nTab) { return ScAddress(nInputColumn, 0, nTab); }

ScAddress formulaCell(SCTAB nTab) { return ScAddress(nFormulaColumn, 0, nTab); }

// A1 holds fInput and B1 multiplies it by ten.
void fillSheet(ScDocument& rDocument, SCTAB nTab, double fInput)
{
    rDocument.SetValue(inputCell(nTab), fInput);
    rDocument.SetString(formulaCell(nTab), u"=A1*10"_ustr);
    CPPUNIT_ASSERT_EQUAL(fInput * 10.0, rDocument.GetValue(formulaCell(nTab)));
}

// Overwrite a formula's cached result while leaving the cell clean. Nothing
// short of a forced recalculation puts the real value back.
void plantStaleResult(ScDocument& rDocument, SCTAB nTab)
{
    ScFormulaCell* pCell = rDocument.GetFormulaCell(formulaCell(nTab));
    CPPUNIT_ASSERT(pCell);
    pCell->SetResultDouble(fStaleResult);
    CPPUNIT_ASSERT_EQUAL(fStaleResult, rDocument.GetValue(formulaCell(nTab)));
}
}

CPPUNIT_TEST_FIXTURE(CalculateSheetTest, testFormulasOnTheNamedSheetRunAgain)
{
    // Naming a sheet calculates it whether or not anything on it is dirty.
    createScDoc();
    ScDocument* pDocument = getScDoc();
    fillSheet(*pDocument, 0, 2.0);

    // The formula is clean, so only a forced recalculation reaches it.
    plantStaleResult(*pDocument, 0);

    calculateSheet(0);

    CPPUNIT_ASSERT_EQUAL(20.0, pDocument->GetValue(formulaCell(0)));
}

CPPUNIT_TEST_FIXTURE(CalculateSheetTest, testFormulasOnOtherSheetsAreLeftAlone)
{
    // Calculating one sheet leaves the formulas on every other sheet as they were.
    createScDoc();
    ScDocument* pDocument = getScDoc();
    insertNewSheet(*pDocument);
    fillSheet(*pDocument, 0, 2.0);
    fillSheet(*pDocument, 1, 3.0);

    plantStaleResult(*pDocument, 0);
    plantStaleResult(*pDocument, 1);

    calculateSheet(0);

    CPPUNIT_ASSERT_EQUAL(20.0, pDocument->GetValue(formulaCell(0)));
    CPPUNIT_ASSERT_EQUAL(fStaleResult, pDocument->GetValue(formulaCell(1)));

    // The second sheet catches up when it is named in its own turn.
    calculateSheet(1);

    CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(formulaCell(1)));
}

CPPUNIT_TEST_FIXTURE(CalculateSheetTest, testASheetIndexTheDocumentLacksChangesNothing)
{
    // A sheet index past the last sheet, or a negative one, calculates nothing at all.
    createScDoc();
    ScDocument* pDocument = getScDoc();
    fillSheet(*pDocument, 0, 2.0);
    plantStaleResult(*pDocument, 0);

    calculateSheet(pDocument->GetTableCount());
    CPPUNIT_ASSERT_EQUAL(fStaleResult, pDocument->GetValue(formulaCell(0)));

    calculateSheet(-1);
    CPPUNIT_ASSERT_EQUAL(fStaleResult, pDocument->GetValue(formulaCell(0)));
}

CPPUNIT_TEST_FIXTURE(CalculateSheetTest, testWithoutASheetArgumentTheVisibleSheetRuns)
{
    // The command falls back to the sheet on screen when the caller names none.
    createScDoc();
    ScDocument* pDocument = getScDoc();
    insertNewSheet(*pDocument);
    fillSheet(*pDocument, 0, 2.0);
    fillSheet(*pDocument, 1, 3.0);

    plantStaleResult(*pDocument, 0);
    plantStaleResult(*pDocument, 1);

    getViewShell()->SetTabNo(1);
    dispatchCommand(mxComponent, u".uno:CalculateSheet"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(fStaleResult, pDocument->GetValue(formulaCell(0)));
    CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(formulaCell(1)));
}

CPPUNIT_TEST_FIXTURE(CalculateSheetTest, testTheCommandAnswersWithTheElapsedTime)
{
    // A run answers the caller with the time its calculation took.
    createScDoc();
    ScDocument* pDocument = getScDoc();
    fillSheet(*pDocument, 0, 2.0);

    frame::DispatchResultEvent aEvent;
    CPPUNIT_ASSERT(calculateSheet(0) >>= aEvent);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(frame::DispatchResultState::SUCCESS), aEvent.State);

    // A whole number of microseconds, which is what makes the command usable
    // as a timer. Round-tripping the text catches anything else.
    OUString aElapsed;
    CPPUNIT_ASSERT(aEvent.Result >>= aElapsed);
    const OUString aRoundTripped(OUString::number(aElapsed.toInt64()));
    CPPUNIT_ASSERT_EQUAL(aRoundTripped, aElapsed);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
