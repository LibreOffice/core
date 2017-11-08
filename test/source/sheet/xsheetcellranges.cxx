/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetcellranges.hxx>

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSheetCellRanges::testGetCells()
{
    uno::Reference<sheet::XSheetCellRanges> xSheetCellRanges(init(), UNO_QUERY_THROW);
    uno::Reference<container::XEnumerationAccess> xEA = xSheetCellRanges->getCells();

    CPPUNIT_ASSERT_MESSAGE("Unable to get cells", xEA->hasElements());
}

void XSheetCellRanges::testGetRangeAddresses()
{
    uno::Reference<sheet::XSheetCellRanges> xSheetCellRanges(init(), UNO_QUERY_THROW);
    uno::Sequence<table::CellRangeAddress> aCellRangeAddress
        = xSheetCellRanges->getRangeAddresses();
    CPPUNIT_ASSERT_MESSAGE("Unable to get RangeAddresses", aCellRangeAddress.getLength() > 2);
}

void XSheetCellRanges::testGetRangeAddressesAsString()
{
    uno::Reference<sheet::XSheetCellRanges> xSheetCellRanges(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to get RangeAddresses (string)",
                           xSheetCellRanges->getRangeAddressesAsString().indexOf("C1:D4") > 0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
