/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <test/sheet/xsheetannotation.hxx>

#include <com/sun/star/table/CellAddress.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSheetAnnotation::testGetPosition()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    table::CellAddress xResultCellAddress = aSheetAnnotation->getPosition();

    
    table::CellAddress xExpectedCellAddress;
    xExpectedCellAddress.Sheet = 0;
    xExpectedCellAddress.Row = 1;
    xExpectedCellAddress.Column = 2;

    CPPUNIT_ASSERT_MESSAGE("Wrong SHEET reference position", xResultCellAddress.Sheet == xExpectedCellAddress.Sheet);
    CPPUNIT_ASSERT_MESSAGE("Wrong COLUMN reference position", xResultCellAddress.Column == xExpectedCellAddress.Column);
    CPPUNIT_ASSERT_MESSAGE("Wrong ROW reference position", xResultCellAddress.Row == xExpectedCellAddress.Row);
}

void XSheetAnnotation::testGetAuthor()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    OUString aAuthor = aSheetAnnotation->getAuthor();

    CPPUNIT_ASSERT_MESSAGE("Wrong author", aAuthor.equals("LG"));
}
void XSheetAnnotation::testGetDate()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    OUString aDate = aSheetAnnotation->getDate();

    CPPUNIT_ASSERT_MESSAGE("Wrong date", aDate.equals("01/17/2013"));
}
void XSheetAnnotation::testGetIsVisible()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    bool isVisible = aSheetAnnotation->getIsVisible();

    CPPUNIT_ASSERT_MESSAGE("Wrong visible state", isVisible == true);
}
void XSheetAnnotation::testSetIsVisible()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    aSheetAnnotation->setIsVisible(false);
    bool isVisible = aSheetAnnotation->getIsVisible();

    CPPUNIT_ASSERT_MESSAGE("Visible state not changed", isVisible == false);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
