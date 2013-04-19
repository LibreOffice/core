/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xspreadsheetdocument.hxx>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

void XSpreadsheetDocument::testGetSheets()
{
    uno::Reference< sheet::XSpreadsheetDocument > xSpreadsheetDoc(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheets > xSheets = xSpreadsheetDoc->getSheets();
    CPPUNIT_ASSERT(xSheets.is());

    uno::Reference< container::XIndexAccess > xIA(xSheets, UNO_QUERY_THROW);
    CPPUNIT_ASSERT( xIA->getCount() == mnSheets );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
