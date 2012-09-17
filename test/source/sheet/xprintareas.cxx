/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Artur Dorda <artur.dorda+libo@gmail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/sheet/xprintareas.hxx>
#include <com/sun/star/sheet/XPrintAreas.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <rtl/ustring.hxx>

using namespace com::sun::star::uno;

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
