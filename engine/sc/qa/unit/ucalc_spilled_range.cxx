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

#include <compiler.hxx>
#include <formulacell.hxx>
#include <patattr.hxx>
#include <scitems.hxx>
#include <scopetools.hxx>
#include <tokenarray.hxx>

#include <formula/errorcodes.hxx>
#include <i18nlangtag/lang.h>
#include <svl/intitem.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>

#include <memory>

using namespace formula;

class TestSpilledRange : public ScUcalcTestBase
{
};

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperator)
{
    // The # postfix operator on the origin of a 1x4 matrix formula
    // expands to the spill range, so SUM(A1#) totals all four values.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(1, 0, 0), 10.0);
    m_pDoc->SetValue(ScAddress(1, 1, 0), 20.0);
    m_pDoc->SetValue(ScAddress(1, 2, 0), 30.0);
    m_pDoc->SetValue(ScAddress(1, 3, 0), 40.0);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 3, aMark, u"=B1:B4"_ustr);

    ScFormulaCell* pFC = m_pDoc->GetFormulaCell(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(ScMatrixMode::Formula, pFC->GetMatrixFlag());
    SCCOL nCols = 0;
    SCROW nRows = 0;
    pFC->GetMatColsRows(nCols, nRows);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCols);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), nRows);

    m_pDoc->SetString(ScAddress(2, 0, 0), u"=SUM(A1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(100.0, m_pDoc->GetValue(ScAddress(2, 0, 0)));

    m_pDoc->SetString(ScAddress(2, 1, 0), u"=COUNT(A1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(4.0, m_pDoc->GetValue(ScAddress(2, 1, 0)));

    CPPUNIT_ASSERT_EQUAL(u"=SUM(A1#)"_ustr, m_pDoc->GetFormula(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorOnNumberCell)
{
    // # on a plain value cell reports #REF!. The operator requires
    // its operand to be the master cell of a dynamic-array formula.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(0, 0, 0), 42.0);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"=A1#"_ustr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoRef, m_pDoc->GetErrCode(ScAddress(1, 0, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorOnStringCell)
{
    // # on a plain string cell reports #REF!.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"hello"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"=A1#"_ustr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoRef, m_pDoc->GetErrCode(ScAddress(1, 0, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorOnNonMasterFormula)
{
    // # on a plain formula cell that is not a dynamic-array master
    // falls back to that cell's value. Only a master triggers the
    // spill expansion.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"=1+2"_ustr);
    m_pDoc->SetString(ScAddress(1, 0, 0), u"=A1#"_ustr);
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(1, 0, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorImplicitIntersection)
{
    // A # reference in a cell that does not promote to a dynamic-array
    // master reduces by implicit intersection on the spill range. The
    // cell takes the slot of the spill range aligned with the formula
    // position, or #VALUE! when the formula sits outside the spill
    // range. Cells set through SetString are not eligible for
    // promotion, so they take this path.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    // Master at A1 spills =B1:B3 with 10, 20 and 30 into A1:A3.
    m_pDoc->SetValue(ScAddress(1, 0, 0), 10.0);
    m_pDoc->SetValue(ScAddress(1, 1, 0), 20.0);
    m_pDoc->SetValue(ScAddress(1, 2, 0), 30.0);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 2, aMark, u"=B1:B3"_ustr);

    // The reference cells E1..E3 sit in rows aligned with the spill
    // range. Each picks the slot of A1# matching its own row. E4
    // sits one row past the spill range, so the cell reports
    // #VALUE! through implicit intersection failure.
    m_pDoc->SetString(ScAddress(4, 0, 0), u"=A1#"_ustr);
    m_pDoc->SetString(ScAddress(4, 1, 0), u"=A1#"_ustr);
    m_pDoc->SetString(ScAddress(4, 2, 0), u"=A1#"_ustr);
    m_pDoc->SetString(ScAddress(4, 3, 0), u"=A1#"_ustr);

    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoValue, m_pDoc->GetErrCode(ScAddress(4, 3, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorRespills)
{
    // A freshly typed =A1# intends an array result, so it promotes to
    // a dynamic-array master and spills the referenced range again
    // instead of collapsing to the row-aligned value.
    m_pDoc->SetAutoCalc(false);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(1, 0, 0), 10.0);
    m_pDoc->SetValue(ScAddress(1, 1, 0), 20.0);
    m_pDoc->SetValue(ScAddress(1, 2, 0), 30.0);

    // Master at A1 spills =B1:B3 into A1:A3.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 2, aMark, u"=B1:B3"_ustr);

    // Enter =A1# at E1 the way the UI does: eligible for promotion.
    ScAddress aRefPos(4, 0, 0);
    ScCompiler aComp(*m_pDoc, aRefPos, m_pDoc->GetGrammar(), false, false);
    std::unique_ptr<ScTokenArray> pCode = aComp.CompileString(u"=A1#"_ustr);
    auto pRef = new ScFormulaCell(*m_pDoc, aRefPos, std::move(pCode));
    pRef->SetAutoDynamicArrayEligible(true);
    m_pDoc->SetFormulaCell(aRefPos, pRef);

    m_pDoc->SetAutoCalc(true);
    m_pDoc->CalcAll();

    // E1 became a master covering three rows and spilled its values
    // into E1:E3.
    CPPUNIT_ASSERT(pRef->IsDynamicArrayMaster());
    SCCOL nCols = 0;
    SCROW nRows = 0;
    pRef->GetMatColsRows(nCols, nRows);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCCOL>(1), nCols);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), nRows);
    CPPUNIT_ASSERT_EQUAL(10.0, m_pDoc->GetValue(ScAddress(4, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, m_pDoc->GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, m_pDoc->GetValue(ScAddress(4, 2, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorIsRef)
{
    // A # spilled-range reference answers ISREF as a reference, even
    // though it evaluates to a matrix. A plain computed matrix is not
    // a reference.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(1, 0, 0), 10.0);
    m_pDoc->SetValue(ScAddress(1, 1, 0), 20.0);

    // Master at A1 spills =B1:B2 into A1:A2.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 1, aMark, u"=B1:B2"_ustr);

    m_pDoc->SetString(ScAddress(4, 0, 0), u"=ISREF(A1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(4, 0, 0)));

    // A matrix that carries no source range stays a non-reference.
    m_pDoc->SetString(ScAddress(4, 1, 0), u"=ISREF(MUNIT(2))"_ustr);
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(4, 1, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorIsFormula)
{
    // ISFORMULA over a # spilled-range reference reports per cell of the
    // spill range, the same as over the plain range: the master is a
    // formula, the cells it spilled into are not.
    m_pDoc->SetAutoCalc(false);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"=1+10"_ustr);
    m_pDoc->SetString(ScAddress(0, 1, 0), u"=1+20"_ustr);
    m_pDoc->SetString(ScAddress(0, 2, 0), u"=1+30"_ustr);

    // B2 spills =A1:A3 into B2:B4 as a dynamic-array master.
    ScAddress aPos(1, 1, 0);
    ScCompiler aComp(*m_pDoc, aPos, m_pDoc->GetGrammar(), false, false);
    std::unique_ptr<ScTokenArray> pCode = aComp.CompileString(u"=A1:A3"_ustr);
    auto pCell = new ScFormulaCell(*m_pDoc, aPos, std::move(pCode));
    pCell->SetAutoDynamicArrayEligible(true);
    m_pDoc->SetFormulaCell(aPos, pCell);
    m_pDoc->SetAutoCalc(true);
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT(pCell->IsDynamicArrayMaster());

    // In an array context, ISFORMULA(B2#) gives one formula (the master)
    // then two non-formula spilled cells, matching ISFORMULA(B2:B4).
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(3, 0, 3, 2, aMark, u"=ISFORMULA(B2#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(3, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(3, 2, 0)));

    // In a plain cell it reduces by implicit intersection to the
    // row-aligned cell: the master row is a formula, a spilled row is
    // not.
    m_pDoc->SetString(ScAddress(5, 1, 0), u"=ISFORMULA(B2#)"_ustr);
    m_pDoc->SetString(ScAddress(5, 2, 0), u"=ISFORMULA(B2#)"_ustr);
    m_pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(1.0, m_pDoc->GetValue(ScAddress(5, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, m_pDoc->GetValue(ScAddress(5, 2, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorInheritsFormat)
{
    // A # reference that collapses to a single value keeps the number
    // format of the spill-range cell it resolves to, the same as a
    // plain reference to that cell would. A fractional value also
    // survives without being truncated to a whole number.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(1, 0, 0), 43831.5);
    m_pDoc->SetValue(ScAddress(1, 1, 0), 43832.5);
    m_pDoc->SetValue(ScAddress(1, 2, 0), 43833.5);

    // Master at A1 spills =B1:B3 into A1:A3.
    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 2, aMark, u"=B1:B3"_ustr);

    // Give the spill cells a date format.
    SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
    sal_uInt32 nDateFormat = pFormatter->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_ENGLISH_US);
    ScPatternAttr aFormatAttr(m_pDoc->getCellAttributeHelper());
    aFormatAttr.ItemSetPut(SfxUInt32Item(ATTR_VALUE_FORMAT, nDateFormat));
    m_pDoc->ApplyPatternAreaTab(0, 0, 0, 2, 0, aFormatAttr);

    // E2 collapses A1# to the row-aligned slot, keeping its value and
    // date format.
    m_pDoc->SetString(ScAddress(4, 1, 0), u"=A1#"_ustr);
    CPPUNIT_ASSERT_EQUAL(43832.5, m_pDoc->GetValue(ScAddress(4, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(SvNumFormatType::DATE,
                         pFormatter->GetType(m_pDoc->GetNumberFormat(4, 1, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorMultiColumn)
{
    // # on the origin of a 2x3 matrix expands to all six cells.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(2, 0, 0), 1.0);
    m_pDoc->SetValue(ScAddress(3, 0, 0), 2.0);
    m_pDoc->SetValue(ScAddress(2, 1, 0), 3.0);
    m_pDoc->SetValue(ScAddress(3, 1, 0), 4.0);
    m_pDoc->SetValue(ScAddress(2, 2, 0), 5.0);
    m_pDoc->SetValue(ScAddress(3, 2, 0), 6.0);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 1, 2, aMark, u"=C1:D3"_ustr);

    m_pDoc->SetString(ScAddress(4, 0, 0), u"=SUM(A1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(21.0, m_pDoc->GetValue(ScAddress(4, 0, 0)));

    m_pDoc->SetString(ScAddress(4, 1, 0), u"=ROWS(A1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(3.0, m_pDoc->GetValue(ScAddress(4, 1, 0)));

    m_pDoc->SetString(ScAddress(4, 2, 0), u"=COLUMNS(A1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(2.0, m_pDoc->GetValue(ScAddress(4, 2, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorOnNonMasterMatrixCell)
{
    // # on a non-master slot of a spilled matrix reports #REF!. Only
    // the master cell of the dynamic-array formula is a valid operand
    // for the spill expansion.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(1, 0, 0), 10.0);
    m_pDoc->SetValue(ScAddress(1, 1, 0), 20.0);
    m_pDoc->SetValue(ScAddress(1, 2, 0), 30.0);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 2, aMark, u"=B1:B3"_ustr);

    // A2 is a non-master slot of the A1:A3 spill range, so =A2# is
    // an illegal reference.
    m_pDoc->SetString(ScAddress(2, 0, 0), u"=A2#"_ustr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoRef, m_pDoc->GetErrCode(ScAddress(2, 0, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorEmptyCell)
{
    // # on an empty cell reports #REF!. An empty cell is not a
    // dynamic-array master, so the operator has no spill range
    // to expand.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetString(ScAddress(1, 0, 0), u"=A1#"_ustr);
    CPPUNIT_ASSERT_EQUAL(FormulaError::NoRef, m_pDoc->GetErrCode(ScAddress(1, 0, 0)));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorInExpression)
{
    // # composes with arithmetic and aggregating functions.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetValue(ScAddress(1, 0, 0), 5.0);
    m_pDoc->SetValue(ScAddress(1, 1, 0), 15.0);

    ScMarkData aMark(m_pDoc->GetSheetLimits());
    aMark.SelectOneTable(0);
    m_pDoc->InsertMatrixFormula(0, 0, 0, 1, aMark, u"=B1:B2"_ustr);

    m_pDoc->SetString(ScAddress(2, 0, 0), u"=SUM(A1#)+100"_ustr);
    CPPUNIT_ASSERT_EQUAL(120.0, m_pDoc->GetValue(ScAddress(2, 0, 0)));

    CPPUNIT_ASSERT_EQUAL(u"=SUM(A1#)+100"_ustr, m_pDoc->GetFormula(2, 0, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_TEST_FIXTURE(TestSpilledRange, testSpilledRangeOperatorFormulaStringRoundtrip)
{
    // The # operator round-trips through GetFormula across several
    // common shapes: standalone, inside SUM/AVERAGE, and on both
    // operands of a binary expression.
    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true);
    m_pDoc->InsertTab(0, u"Sheet1"_ustr);

    m_pDoc->SetString(ScAddress(0, 0, 0), u"=A2#"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"=A2#"_ustr, m_pDoc->GetFormula(0, 0, 0));

    m_pDoc->SetString(ScAddress(0, 1, 0), u"=SUM(B1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(B1#)"_ustr, m_pDoc->GetFormula(0, 1, 0));

    m_pDoc->SetString(ScAddress(0, 2, 0), u"=AVERAGE(C5#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"=AVERAGE(C5#)"_ustr, m_pDoc->GetFormula(0, 2, 0));

    m_pDoc->SetString(ScAddress(0, 3, 0), u"=MAX(D1#)+MIN(D1#)"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"=MAX(D1#)+MIN(D1#)"_ustr, m_pDoc->GetFormula(0, 3, 0));

    m_pDoc->DeleteTab(0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
