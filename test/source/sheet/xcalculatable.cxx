/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcalculatable.hxx>

#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/table/XCell.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XCalculatable::testEnableAutomaticCaclulation()
{
    uno::Reference<sheet::XCalculatable> xCalculatable(init(), UNO_QUERY_THROW);

    bool bIsAutomaticCalculationEnabled = xCalculatable->isAutomaticCalculationEnabled();
    xCalculatable->enableAutomaticCalculation(!bIsAutomaticCalculationEnabled);
    bool bIsAutomaticCalculationEnabledNew = xCalculatable->isAutomaticCalculationEnabled();
    CPPUNIT_ASSERT_MESSAGE("Unable to set enableAutomaticCalculation()",
                           bIsAutomaticCalculationEnabled != bIsAutomaticCalculationEnabledNew);
}

void XCalculatable::testCalculate()
{
    uno::Reference<sheet::XCalculatable> xCalculatable(init(), UNO_QUERY_THROW);
    xCalculatable->enableAutomaticCalculation(false);

    uno::Sequence<uno::Reference<table::XCell>> xCells = getXCells();

    double dProduct = xCells[2]->getValue();
    double dFactor1 = xCells[0]->getValue();
    xCells[0]->setValue(dFactor1 + 1.0);
    double dProduct2 = xCells[2]->getValue();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculated products are equal", dProduct, dProduct2, 0.5);

    xCalculatable->calculate();
    dProduct2 = xCells[2]->getValue();
    CPPUNIT_ASSERT_MESSAGE("Calculated products are not equal", dProduct != dProduct2);
}

void XCalculatable::testCalculateAll()
{
    uno::Reference<sheet::XCalculatable> xCalculatable(init(), UNO_QUERY_THROW);
    xCalculatable->enableAutomaticCalculation(false);

    uno::Sequence<uno::Reference<table::XCell>> xCells = getXCells();

    double dProduct = xCells[2]->getValue();
    double dFactor1 = xCells[0]->getValue();
    xCells[0]->setValue(dFactor1 + 1.0);
    double dProduct2 = xCells[2]->getValue();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Calculated products are equal", dProduct, dProduct2, 0.5);

    xCalculatable->calculateAll();
    dProduct2 = xCells[2]->getValue();
    CPPUNIT_ASSERT_MESSAGE("Calculated products are not equal", dProduct != dProduct2);
    xCalculatable->calculateAll();
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
