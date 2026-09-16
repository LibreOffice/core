/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <rtl/ustring.hxx>
#include <unotest/detail/unotestdllapi.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <cpo/uno/Any.h>
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
    MacrosTest();
    ~MacrosTest();

    cpo::uno::Reference<css::lang::XComponent>
    loadFromDesktop(const OUString& rURL, const OUString& rDocService = OUString(),
                    const cpo::uno::Sequence<css::beans::PropertyValue>& rExtra_args
                    = cpo::uno::Sequence<css::beans::PropertyValue>());

    static cpo::uno::Any
    dispatchCommand(const cpo::uno::Reference<css::lang::XComponent>& xComponent,
                    const OUString& rCommand,
                    const cpo::uno::Sequence<css::beans::PropertyValue>& rPropertyValues);

    static cpo::uno::Any
    queryDispatchStatus(cpo::uno::Reference<css::lang::XComponent> const& xComponent,
                        cpo::uno::Reference<cpo::uno::XComponentContext> const& xContext,
                        OUString const& rURL);

    /// Opens rStreamName from rTempFile, assuming it's a ZIP storage.
    static std::unique_ptr<SvStream> parseExportStream(const OUString& url,
                                                       const OUString& rStreamName);

    // note: there is no tearDownX509
    void setUpX509(const test::Directories& rDirectories, const OUString& rTestName);

    static bool IsValid(const cpo::uno::Reference<css::security::XCertificate>& cert,
                        const cpo::uno::Reference<css::xml::crypto::XSecurityEnvironment>& env);
    static cpo::uno::Reference<css::security::XCertificate> GetValidCertificate(
        const cpo::uno::Sequence<cpo::uno::Reference<css::security::XCertificate>>& certs,
        const cpo::uno::Reference<css::xml::crypto::XSecurityEnvironment>& env,
        const cpo::uno::Sequence<css::beans::PropertyValue>& rFilterData = {});

protected:
    cpo::uno::Reference<css::frame::XDesktop> mxDesktop;

private:
    std::unique_ptr<BasicDLL> mpDll;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
