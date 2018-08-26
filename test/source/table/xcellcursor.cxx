/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* This file incorporates work covered by the following license notice:
*
*   Licensed to the Apache Software Foundation (ASF) under one or more
*   contributor license agreements. See the NOTICE file distributed
*   with this work for additional information regarding copyright
*   ownership. The ASF licenses this file to you under the Apache
*   License, Version 2.0 (the "License"); you may not use this file
*   except in compliance with the License. You may obtain a copy of
*   the License at http://www.apache.org/licenses/LICENSE-2.0 .
*/

#include <test/cppunitasserthelper.hxx>
#include <test/table/xcellcursor.hxx>

#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XCellCursor::testGoToNext()
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startCol = aCellRangeAddr.StartColumn;

    xCellCursor->gotoNext();

    aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startCol2 = aCellRangeAddr.StartColumn;

    CPPUNIT_ASSERT_MESSAGE("Sucessfully able to go to Next", startCol != startCol2);
}

void XCellCursor::testGoToOffset();
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::xCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startRow = aCellRangeAddr.StartRow;
    const sal_Int32 startCol = aCellRangeAddr.StartColumn;

    xCellCursor->gotoOffset(4, 4);

    aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    const sal_Int32 startRow2 = aCellRangeAddr.StartRow;
    const sal_Int32 startCol2 = aCellRangeAddr.StartColumn;

    CPPUNIT_ASSERT_MESSAGE("Successfully able to go to Offset",
                           (startCol != startCol2) || (startRow == startRow2));
}

void XCellCursor::testGoToPrevious();
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::xCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    int sal_Int32 StartCol = aCellRangeAddr.StartColumn;

    xCellCursor->gotoPrevious();

    aCellRangeAddr = xCellRangeAddressable->getRangeAddress();
    int sal_Int32 startCol2 = aCellRangeAddr.StartColumn;
    CPPUNIT_ASSERT_MESSAGE("Successfully able to go to Next", startCol != startCol2);
}

void XCellCursor::testGoToStart();
{
    uno::Reference<table::XCellCursor> xCellCursor(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::xCellRangeAddressable> xCellRangeAddressable(xCellCursor,
                                                                       UNO_QUERY_THROW);
    xCellRange->gotoStart();

    table::CellRangeAddress aCellRangeAddr = xCellRangeAddressable->getRangeAddress();

    int sal_Int32 startRow = aCellRangeAddr.StartRow;
    int sal_Int32 startCol = aCellRangeAddr.StartCol;
    int sal_Int32 endRow = aCellRangeAddr.EndRow;
    int sal_Int32 endCol = aCellRangeAddr.EndColumn;
    CPPUNIT_ASSERT_MESSAGE("Successfully able to got to Start",
                           (startCol == endCol) && (endRow == startRow));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
