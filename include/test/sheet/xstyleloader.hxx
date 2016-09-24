/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XSTYLELOADER_HXX
#define INCLUDED_TEST_SHEET_XSTYLELOADER_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/style/XStyleLoader2.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XStyleLoader
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    virtual OUString getTestURL() = 0;
    virtual css::uno::Reference< css::lang::XComponent  > getTargetComponent() = 0;
    virtual css::uno::Reference< css::lang::XComponent > getSourceComponent() = 0;

    // XStyleLoader
    void testLoadStylesFromURL();
    // XStyleLoader2
    void testLoadStylesFromDocument();

private:
    static void checkStyleProperties(css::uno::Reference< css::style::XStyleFamiliesSupplier > const & xFamilySupplier);

protected:
    ~XStyleLoader() {}

};

}

#endif // INCLUDED_TEST_SHEET_XSTYLELOADER_HXX
