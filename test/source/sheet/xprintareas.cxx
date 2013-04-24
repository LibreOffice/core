/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xprintareas.hxx>
#include <com/sun/star/sheet/XPrintAreas.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

    void XPrintAreas::testSetAndGetPrintTitleColumns()
    {
        uno::Reference< sheet::XPrintAreas > xPrintAreas(init(), UNO_QUERY_THROW);
        sal_Bool testState = true;
        xPrintAreas->setPrintTitleColumns(testState);

        CPPUNIT_ASSERT_MESSAGE("Wrong attribution at PrintTitleColumns", xPrintAreas->getPrintTitleColumns() == testState);

    }

    void XPrintAreas::testSetAndGetPrintTitleRows()
    {
        uno::Reference< sheet::XPrintAreas > xPrintAreas(init(), UNO_QUERY_THROW);
        sal_Bool testState = true;
        xPrintAreas->setPrintTitleRows(testState);

        CPPUNIT_ASSERT_MESSAGE("Wrong attribution at PrintTitleRows", xPrintAreas->getPrintTitleRows() == testState);
    }

    // the rest is right now in progress...


}
