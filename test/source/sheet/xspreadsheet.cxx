/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xspreadsheet.hxx>

#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace apitest {

void XSpreadsheet::testCreateCursor()
{
    uno::Reference< sheet::XSpreadsheet > xSheet(init(), UNO_QUERY_THROW);

    uno::Reference< sheet::XSheetCellCursor > xCursor(xSheet->createCursor(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to create XSheetCellCursor", xCursor.is());
}

void XSpreadsheet::testCreateCursorByRange()
{
    uno::Reference< sheet::XSpreadsheet > xSheet(init(), UNO_QUERY_THROW);

    uno::Reference< sheet::XSheetCellRange > xCellRange(xSheet->getCellRangeByPosition(1, 1, 2, 3), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellCursor > xCursor(xSheet->createCursorByRange(xCellRange), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to create XSheetCellCursor", xCursor.is());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
