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
    sheet::GoalResult res = pModelObj->seekGoal(aFormulaCell, aVariableCell, "100");
    CPPUNIT_ASSERT_EQUAL(0.0, res.Result);
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

    sheet::GoalResult res = pModelObj->seekGoal(aFormulaCell, aVariableCell, "2");
    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(4.0, res.Result);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
