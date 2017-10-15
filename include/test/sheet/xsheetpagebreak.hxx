/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XSHEETPAGEBREAK_HXX
#define INCLUDED_TEST_SHEET_XSHEETPAGEBREAK_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/sheet/XSheetPageBreak.hpp>
#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XSheetPageBreak
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    void testGetColumnPageBreaks();
    void testGetRowPageBreaks();
    void testRemoveAllManualPageBreaks();

protected:
    ~XSheetPageBreak() {}
};

}

#endif // INCLUDED_TEST_SHEET_XSHEETPAGEBREAK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
