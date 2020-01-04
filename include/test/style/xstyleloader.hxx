/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_STYLE_XSTYLELOADER_HXX
#define INCLUDED_TEST_STYLE_XSTYLELOADER_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ustring.hxx>
#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XStyleLoader
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual css::uno::Reference<css::sheet::XSpreadsheetDocument> getTargetDoc() = 0;
    virtual css::uno::Reference<css::lang::XComponent> getSourceComponent() = 0;
    virtual OUString getTestURL() = 0;

    // XStyleLoader
    void testLoadStylesFromURL();
    // XStyleLoader2
    void testLoadStylesFromDocument();

private:
    static void checkStyleProperties(
        css::uno::Reference<css::style::XStyleFamiliesSupplier> const& xFamilySupplier);

protected:
    ~XStyleLoader() {}
};

} // namespace apitest

#endif // INCLUDED_TEST_STYLE_XSTYLELOADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
