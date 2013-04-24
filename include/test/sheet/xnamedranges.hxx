/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XNamedRanges
{
public:
    // remove default entry
    XNamedRanges();
    // removes given entry
    XNamedRanges(const OUString& rNameToRemove);

    virtual ~XNamedRanges();

    virtual css::uno::Reference< css::uno::XInterface > init(sal_Int32 nSheets = 0) = 0;

    // XNamedRanges
    void testAddNewByName();
    void testAddNewFromTitles();
    void testRemoveByName();
    void testOutputList();

protected:
    css::uno::Reference< css::sheet::XSpreadsheet > xSheet;

private:
    OUString maNameToRemove;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
