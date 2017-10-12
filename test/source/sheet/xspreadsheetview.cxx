/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xspreadsheetview.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void XSpreadsheetView::testGetSetActiveSheet()
{
    uno::Reference< sheet::XSpreadsheetView > xView(init(), UNO_QUERY_THROW);

    uno::Reference< sheet::XSpreadsheet > xActiveSheet = xView->getActiveSheet();
    CPPUNIT_ASSERT_MESSAGE("Unable to get default active sheet", xActiveSheet.is());

    uno::Reference< sheet::XSpreadsheet > xNewSheet = xActiveSheet;
    CPPUNIT_ASSERT_MESSAGE("no sheet", xNewSheet.is());
    uno::Reference< container::XNamed > xNamedNewSheet(xNewSheet->getSpreadsheet(), UNO_QUERY_THROW);
    xNamedNewSheet->setName("NewSheet");
    xView->setActiveSheet(xNewSheet);
    uno::Reference< sheet::XSpreadsheet > xNewActiveSheet = xView->getActiveSheet();
    CPPUNIT_ASSERT_MESSAGE("Unable to get new active sheet", xNewActiveSheet.is());

    //uno::Reference< beans::XPropertySet > xPropSetActSheet(xActiveSheet->getSpreadsheet(), UNO_QUERY_THROW);
    //OUString aName;
    //CPPUNIT_ASSERT(xPropSetActSheet->getPropertyValue("CodeName") >>= aName);
    uno::Reference< container::XNamed > xNamedAct(xActiveSheet->getSpreadsheet(), UNO_QUERY_THROW);
    uno::Reference< container::XNamed > xNamedNew(xNewActiveSheet->getSpreadsheet(), UNO_QUERY_THROW);

    //uno::Reference< beans::XPropertySet > xPropSetNewSheet(xNewActiveSheet->getSpreadsheet(), UNO_QUERY_THROW);
    //OUString nName;
    //CPPUNIT_ASSERT(xPropSetNewSheet->getPropertyValue("CodeName") >>= nName);

    CPPUNIT_ASSERT_MESSAGE("Default and new active sheet are not different",
                           xActiveSheet != xNewActiveSheet);
    //CPPUNIT_ASSERT_MESSAGE("Default and new active sheet are not different",
                           //aName != nName);
    //CPPUNIT_ASSERT_MESSAGE("Default and new active sheet are not different",
                           //xNamedAct->getName() != xNamedNew->getName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default and new active sheet are not different",
                           xNamedAct->getName(), xNamedNew->getName());
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Default ))and new active sheet are not different",
                           //OUString("Test"), xNamedNew->getName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default and new active sheet are not different",
                           xNamedAct->getName(), OUString("Test"));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
