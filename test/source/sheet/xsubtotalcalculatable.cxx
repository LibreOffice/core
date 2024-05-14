/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsubtotalcalculatable.hxx>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void XSubTotalCalculatable::testCreateSubTotalDescriptor()
{
    uno::Reference< sheet::XSubTotalCalculatable > xSTC(init(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalDescriptor > xSTD = xSTC->createSubTotalDescriptor(true);

    uno::Sequence< sheet::SubTotalColumn > xCols{ { /* Column   */ 5,
                                                    /* Function */ sheet::GeneralFunction_SUM } };

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to create XSubTotalDescriptor", xSTD->addNew(xCols, 1));
}

void XSubTotalCalculatable::testApplyRemoveSubTotals()
{
    uno::Reference< sheet::XSpreadsheet > xSheet(getXSpreadsheet(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSubTotalCalculatable > xSTC(xSheet, UNO_QUERY_THROW);

    uno::Reference< sheet::XSubTotalDescriptor > xSTD = xSTC->createSubTotalDescriptor(true);
    uno::Sequence< sheet::SubTotalColumn > xCols{ { /* Column   */ 0,
                                                    /* Function */ sheet::GeneralFunction_SUM } };
    xSTD->addNew(xCols, 1);

    xSheet->getCellByPosition(0, 0)->setFormula(u"first"_ustr);
    xSheet->getCellByPosition(1, 0)->setFormula(u"second"_ustr);
    xSheet->getCellByPosition(0, 3)->setFormula(u""_ustr);
    xSheet->getCellByPosition(0, 1)->setValue(5);
    xSheet->getCellByPosition(0, 2)->setValue(5);
    xSheet->getCellByPosition(1, 1)->setValue(17);
    xSheet->getCellByPosition(1, 2)->setValue(17);

    xSTC->applySubTotals(xSTD, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to apply SubTotals",
                                 u"=SUBTOTAL(9;$A$2:$A$3)"_ustr,
                                 xSheet->getCellByPosition(0, 3)->getFormula());

    xSTC->removeSubTotals();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to remove SubTotals",
                                 u""_ustr,
                                 xSheet->getCellByPosition(0, 3)->getFormula());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
