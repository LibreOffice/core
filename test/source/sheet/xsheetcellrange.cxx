/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetcellrange.hxx>

#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSheetCellRange::testGetSpreadsheet()
{
    uno::Reference<sheet::XSheetCellRange> xSheetCellRange(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet = xSheetCellRange->getSpreadsheet();

    CPPUNIT_ASSERT_MESSAGE("Unable to get spreadsheet", xSheet.is());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
