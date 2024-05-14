/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetannotation.hxx>

#include <com/sun/star/table/CellAddress.hpp>

#include <cppunit/TestAssert.h>
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSheetAnnotation::testGetPosition()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    table::CellAddress aResultCellAddress = aSheetAnnotation->getPosition();

    //expected result
    table::CellAddress aExpectedCellAddress;
    aExpectedCellAddress.Sheet = 0;
    aExpectedCellAddress.Row = 1;
    aExpectedCellAddress.Column = 2;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong SHEET reference position", aExpectedCellAddress.Sheet, aResultCellAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong COLUMN reference position", aExpectedCellAddress.Column, aResultCellAddress.Column);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ROW reference position", aExpectedCellAddress.Row, aResultCellAddress.Row);
}

void XSheetAnnotation::testGetAuthor()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    OUString aAuthor = aSheetAnnotation->getAuthor();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong author", u"LG"_ustr, aAuthor);
}
void XSheetAnnotation::testGetDate()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    OUString aDate = aSheetAnnotation->getDate();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong date", u"01/17/2013"_ustr, aDate);
}
void XSheetAnnotation::testGetIsVisible()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    bool isVisible = aSheetAnnotation->getIsVisible();

    CPPUNIT_ASSERT_MESSAGE("Wrong visible state", isVisible);
}
void XSheetAnnotation::testSetIsVisible()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    aSheetAnnotation->setIsVisible(false);
    bool isVisible = aSheetAnnotation->getIsVisible();

    CPPUNIT_ASSERT_MESSAGE("Visible state not changed", !isVisible);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
