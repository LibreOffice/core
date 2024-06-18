/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "helper/qahelper.hxx"
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;

class ScGoalSeekTest : public ScModelTestBase
{
public:
    ScGoalSeekTest();
};

ScGoalSeekTest::ScGoalSeekTest()
    : ScModelTestBase(u"/sc/qa/unit/data/"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(ScGoalSeekTest, testTdf161511)
{
    createScDoc();

    table::CellAddress aVariableCell;
    aVariableCell.Sheet = 0;
    aVariableCell.Row = 0;
    aVariableCell.Column = 3;
    table::CellAddress aFormulaCell;
    aFormulaCell.Sheet = 0;
    aFormulaCell.Row = 0;
    aFormulaCell.Column = 4;

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    // Without the fix in place, this test would have crashed
    sheet::GoalResult res = pModelObj->seekGoal(aFormulaCell, aVariableCell, u"100"_ustr);
    CPPUNIT_ASSERT_EQUAL(0.0, res.Result);
    CPPUNIT_ASSERT_EQUAL(DBL_MAX, res.Divergence);
}

CPPUNIT_TEST_FIXTURE(ScGoalSeekTest, testTdf37341)
{
    createScDoc("ods/tdf37341.ods");

    // E7
    table::CellAddress aVariableCell;
    aVariableCell.Sheet = 0;
    aVariableCell.Row = 6;
    aVariableCell.Column = 4;

    // F111
    table::CellAddress aFormulaCell;
    aFormulaCell.Sheet = 0;
    aFormulaCell.Row = 110;
    aFormulaCell.Column = 5;

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    // Without the fix in place, this test would have hung here
    sheet::GoalResult res = pModelObj->seekGoal(aFormulaCell, aVariableCell, "0");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11778.08775, res.Result, 0.0001);
    CPPUNIT_ASSERT_EQUAL(DBL_MAX, res.Divergence);
}

CPPUNIT_TEST_FIXTURE(ScGoalSeekTest, testTdf68034)
{
    createScDoc();

    insertStringToCell(u"A1"_ustr, u"=SQRT(B1)");

    table::CellAddress aVariableCell;
    aVariableCell.Sheet = 0;
    aVariableCell.Row = 0;
    aVariableCell.Column = 1;
    table::CellAddress aFormulaCell;
    aFormulaCell.Sheet = 0;
    aFormulaCell.Row = 0;
    aFormulaCell.Column = 0;

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    sheet::GoalResult res = pModelObj->seekGoal(aFormulaCell, aVariableCell, u"2"_ustr);
    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(4.0, res.Result);
    CPPUNIT_ASSERT_EQUAL(0.0, res.Divergence);
}

CPPUNIT_TEST_FIXTURE(ScGoalSeekTest, testTdf161616)
{
    createScDoc();

    insertStringToCell(u"A1"_ustr, u"250");
    insertStringToCell(u"B1"_ustr, u"0.25");
    insertStringToCell(u"C1"_ustr, u"200");
    insertStringToCell(u"D1"_ustr, u"= A1 * B1 / C1");

    table::CellAddress aVariableCell;
    aVariableCell.Sheet = 0;
    aVariableCell.Row = 0;
    aVariableCell.Column = 3;
    table::CellAddress aFormulaCell;
    aFormulaCell.Sheet = 0;
    aFormulaCell.Row = 0;
    aFormulaCell.Column = 4;

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    sheet::GoalResult res = pModelObj->seekGoal(aFormulaCell, aVariableCell, "100");
    CPPUNIT_ASSERT_EQUAL(0.0, res.Result);
    CPPUNIT_ASSERT_EQUAL(DBL_MAX, res.Divergence);

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(u"250"_ustr, pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"0.25"_ustr, pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(u"200"_ustr, pDoc->GetString(ScAddress(2, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: 0.3125
    // - Actual  : #N/A
    CPPUNIT_ASSERT_EQUAL(u"0.3125"_ustr, pDoc->GetString(ScAddress(3, 0, 0)));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
