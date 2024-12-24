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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Any.h>
#include <utility>

struct TestMacroInfo
{
    OUString sFileBaseName;
    OUString sMacroUrl;
};

class BasicDLL;
class SvStream;

namespace test
{
class Directories;
}

namespace com::sun::star::security
{
class XCertificate;
}
namespace com::sun::star::xml::crypto
{
class XSecurityEnvironment;
}

namespace unotest
{
class OOO_DLLPUBLIC_UNOTEST MacrosTest
{
public:
    class Resetter
    {
    private:
        std::function<void()> m_Func;

    public:
        Resetter(std::function<void()> aFunc)
            : m_Func(std::move(aFunc))
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

    css::uno::Reference<css::lang::XComponent>
    loadFromDesktop(const OUString& rURL, const OUString& rDocService = OUString(),
                    const css::uno::Sequence<css::beans::PropertyValue>& rExtra_args
                    = css::uno::Sequence<css::beans::PropertyValue>());

    static css::uno::Any
    dispatchCommand(const css::uno::Reference<css::lang::XComponent>& xComponent,
                    const OUString& rCommand,
                    const css::uno::Sequence<css::beans::PropertyValue>& rPropertyValues);

    static css::uno::Any
    queryDispatchStatus(css::uno::Reference<css::lang::XComponent> const& xComponent,
                        css::uno::Reference<css::uno::XComponentContext> const& xContext,
                        OUString const& rURL);

    /// Opens rStreamName from rTempFile, assuming it's a ZIP storage.
    static std::unique_ptr<SvStream> parseExportStream(const OUString& url,
                                                       const OUString& rStreamName);

    // note: there is no tearDownX509
    void setUpX509(const test::Directories& rDirectories, const OUString& rTestName);
    static void setUpGpg(const test::Directories& rDirectories, std::u16string_view rTestName);
    static void tearDownGpg();

    static bool IsValid(const css::uno::Reference<css::security::XCertificate>& cert,
                        const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& env);
    static css::uno::Reference<css::security::XCertificate> GetValidCertificate(
        const css::uno::Sequence<css::uno::Reference<css::security::XCertificate>>& certs,
        const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& env,
        const css::uno::Sequence<css::beans::PropertyValue>& rFilterData = {});

protected:
    css::uno::Reference<css::frame::XDesktop2> mxDesktop;

private:
    std::unique_ptr<BasicDLL> mpDll;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
