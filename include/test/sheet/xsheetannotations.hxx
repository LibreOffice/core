/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XSheetAnnotations
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    // XSheetAnnotations
    void testInsertNew();
    void testRemoveByIndex();
    void testCount();

protected:
    ~XSheetAnnotations() {}

    virtual css::uno::Reference< css::sheet::XSheetAnnotations> getAnnotations(long nIndex) = 0;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
