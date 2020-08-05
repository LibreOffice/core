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

#include <sal/config.h>

#include <memory>
#include <functional>
#include <config_gpgme.h>
#include <rtl/ustring.hxx>
#include <unotest/detail/unotestdllapi.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

struct TestMacroInfo
{
    OUString sFileBaseName;
    OUString sMacroUrl;
};

class BasicDLL;

namespace test
{
class Directories;
}

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

    MacrosTest();
    ~MacrosTest();

    css::uno::Reference< css::lang::XComponent > loadFromDesktop(const OUString& rURL, const OUString& rDocService = OUString(),
        const css::uno::Sequence<css::beans::PropertyValue>& rExtra_args = css::uno::Sequence<css::beans::PropertyValue>() );

    static void
    dispatchCommand(const css::uno::Reference<css::lang::XComponent>& xComponent,
                    const OUString& rCommand,
                    const css::uno::Sequence<css::beans::PropertyValue>& rPropertyValues);

    void setUpNssGpg(const test::Directories& rDirectories, const OUString& rTestName);
    void tearDownNssGpg();

protected:
    css::uno::Reference< css::frame::XDesktop2> mxDesktop;

private:
    std::unique_ptr<BasicDLL> mpDll;
#if HAVE_GPGCONF_SOCKETDIR
    OString m_gpgconfCommandPrefix;
#endif
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
