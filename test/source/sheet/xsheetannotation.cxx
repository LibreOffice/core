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
 * Copyright (C) 2011 Laurent Godard lgodard.libre@laposte.net (initial developer)
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

#include <test/sheet/xsheetannotation.hxx>

#include <com/sun/star/table/CellAddress.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <rtl/ustring.hxx>

using namespace com::sun::star::uno;

namespace apitest {

void XSheetAnnotation::testGetPosition()
{
    uno::Reference< sheet::XSheetAnnotation > aSheetAnnotation (init(), UNO_QUERY_THROW);
    table::CellAddress xResultCellAddress = aSheetAnnotation->getPosition();

    //expected result
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
