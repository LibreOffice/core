/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetcondition.hxx>

#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSheetCondition::testGetSetFormula1()
{
    uno::Reference<sheet::XSheetCondition> xSheetCondition(init(), UNO_QUERY_THROW);
    OUString aFormula1;
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to get Formula1",
                                    aFormula1 = xSheetCondition->getFormula1());

    xSheetCondition->setFormula1(u"$Sheet1.$C$"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Unable to set Formula1", aFormula1 != xSheetCondition->getFormula1());
}

void XSheetCondition::testGetSetFormula2()
{
    uno::Reference<sheet::XSheetCondition> xSheetCondition(init(), UNO_QUERY_THROW);
    OUString aFormula2;
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to get Formula2",
                                    aFormula2 = xSheetCondition->getFormula2());

    xSheetCondition->setFormula2(u"$Sheet1.$A$"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Unable to set Formula2", aFormula2 != xSheetCondition->getFormula2());
}

void XSheetCondition::testGetSetOperator()
{
    uno::Reference<sheet::XSheetCondition> xSheetCondition(init(), UNO_QUERY_THROW);
    sheet::ConditionOperator aOperator;
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to get Operator",
                                    aOperator = xSheetCondition->getOperator());

    xSheetCondition->setOperator(sheet::ConditionOperator_BETWEEN);
    sheet::ConditionOperator aOperatorNew = xSheetCondition->getOperator();
    CPPUNIT_ASSERT_MESSAGE("Unable to set Operator", aOperator != aOperatorNew);
}

void XSheetCondition::testGetSetSourcePosition()
{
    uno::Reference<sheet::XSheetCondition> xSheetCondition(init(), UNO_QUERY_THROW);
    table::CellAddress aSourcePosition(0, 0, 0);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to get SourcePosition",
                                    aSourcePosition = xSheetCondition->getSourcePosition());

    xSheetCondition->setSourcePosition(table::CellAddress(0, 1, 1));
    table::CellAddress aSourcePositionNew = xSheetCondition->getSourcePosition();
    CPPUNIT_ASSERT_MESSAGE("Unable to set SourcePosition",
                           aSourcePosition.Row != aSourcePositionNew.Row);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
