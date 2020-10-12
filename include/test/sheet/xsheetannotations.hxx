/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XSHEETANNOTATIONS_HXX
#define INCLUDED_TEST_SHEET_XSHEETANNOTATIONS_HXX

#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <test/testdllapi.hxx>
#include <tools/long.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XSheetAnnotations
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;
    virtual css::uno::Reference< css::sheet::XSheetAnnotations> getAnnotations(tools::Long nIndex) = 0;

    // XSheetAnnotations
    void testIndex();
    void testInsertNew();
    void testRemoveByIndex();
    void testCount();

protected:
    ~XSheetAnnotations() {}
};

}

#endif // INCLUDED_TEST_SHEET_XSHEETANNOTATIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
