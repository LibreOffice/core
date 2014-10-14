/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XCALCSEARCH_HXX
#define INCLUDED_TEST_SHEET_XCALCSEARCH_HXX

#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XCalcSearch
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    virtual void startPerfInstrumentation() = 0;
    virtual void endPerfInstrumentation(const char* message) = 0;

    // XSearchable
    void testSheetFindAll();

protected:
    ~XCalcSearch() {}

};

}

#endif // INCLUDED_TEST_SHEET_XCALCSEARCH_HXX