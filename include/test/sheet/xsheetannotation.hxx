/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XSHEETANNOTATION_HXX
#define INCLUDED_TEST_SHEET_XSHEETANNOTATION_HXX

#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <test/testdllapi.hxx>

#include <com/sun/star/table/CellAddress.hpp>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XSheetAnnotation
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;

    // XSheetAnnotation
    void testGetPosition();
    void testGetAuthor();
    void testGetDate();
    void testGetIsVisible();
    void testSetIsVisible();

protected:
    ~XSheetAnnotation() {}

    virtual css::uno::Reference<css::sheet::XSheetAnnotation>
    getAnnotation(css::table::CellAddress&) = 0;
};
}

#endif // INCLUDED_TEST_SHEET_XSHEETANNOTATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
