/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTEST_MACROS_TEST_HXX
#define INCLUDED_UNOTEST_MACROS_TEST_HXX

#include <functional>

#include <rtl/ustring.hxx>
#include <unotest/detail/unotestdllapi.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Any.h>

struct TestMacroInfo
{
    OUString sFileBaseName;
    OUString sMacroUrl;
};

namespace unotest {

class OOO_DLLPUBLIC_UNOTEST MacrosTest
{
public:
    class Resetter
    {
    private:
        std::function<void ()> m_Func;

    public:
        Resetter(std::function<void ()> const& rFunc)
            : m_Func(rFunc)
        {
        }
        ~Resetter()
        {
            try
            {
                m_Func();
            }
            catch (...) // has to be reliable
            {
                fprintf(stderr, "resetter failed with exception\n");
                abort();
            }
        }
    };

    css::uno::Reference< css::lang::XComponent > loadFromDesktop(const OUString& rURL, const OUString& rDocService = OUString(),
        const css::uno::Sequence<css::beans::PropertyValue>& rExtra_args = css::uno::Sequence<css::beans::PropertyValue>() );

    static css::uno::Any
    queryDispatchStatus(css::uno::Reference<css::lang::XComponent> const& xComponent,
                        css::uno::Reference<css::uno::XComponentContext> const& xContext,
                        OUString const& rURL);

protected:
    css::uno::Reference< css::frame::XDesktop2> mxDesktop;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
