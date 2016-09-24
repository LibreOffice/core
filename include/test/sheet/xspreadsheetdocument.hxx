/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XSPREADSHEETDOCUMENT_HXX
#define INCLUDED_TEST_SHEET_XSPREADSHEETDOCUMENT_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <test/testdllapi.hxx>

namespace apitest {

class XSpreadsheetDocument
{
public:
    XSpreadsheetDocument(sal_Int32 nSheets) : mnSheets(nSheets) {}

    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    void testGetSheets();

protected:
    ~XSpreadsheetDocument() {}

private:
    sal_Int32 mnSheets;
};

}

#endif // INCLUDED_TEST_SHEET_XSPREADSHEETDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
