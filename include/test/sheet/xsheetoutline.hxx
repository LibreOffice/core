/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XSheetOutline
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    // XSheetOutline
    void testHideDetail();
    void testShowDetail();
    void testShowLevel();
    void testUngroup();
    void testGroup();
    void testAutoOutline();
    void testClearOutline();

protected:
    ~XSheetOutline() {}
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
