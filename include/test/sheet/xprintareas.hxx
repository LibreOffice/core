/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XPRINTAREAS_HXX
#define INCLUDED_TEST_SHEET_XPRINTAREAS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XPrintAreas
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    void testSetAndGetPrintTitleColumns();   //getter and setter in one test case
    void testSetAndGetPrintTitleRows();

    void testGetPrintAreas();           // returns a sequence containing all print areas of the sheet.
    void testSetPrintAreas();           // sets the print areas of the sheet.
    void testGetTitleColumns();         // returns the range that is specified as title columns range.
    void testSetTitleColumns();         // specifies a range of columns as title columns range.
    void testGetTitleRows();            // returns the range that is specified as title rows range.
    void testSetTitleRows();            // specifies a range of rows as title rows range.

protected:
    ~XPrintAreas() {}
};

}

#endif // INCLUDED_TEST_SHEET_XPRINTAREAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
