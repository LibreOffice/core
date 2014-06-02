/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_CELLPROPERTIES_HXX
#define INCLUDED_TEST_SHEET_CELLPROPERTIES_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST CellProperties
{
public:
    virtual css::uno::Reference < css::uno::XInterface > init() = 0;

    void testVertJustify();
    void testRotateReference();

protected:
    ~CellProperties() {}
};

}

#endif // INCLUDED_TEST_SHEET_CELLPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
