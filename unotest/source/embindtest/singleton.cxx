/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <org/libreoffice/embindtest/XStringFactory.hpp>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace
{
class SingletonTest : public cppu::WeakImplHelper<org::libreoffice::embindtest::XStringFactory,
                                                  css::lang::XServiceInfo>
{
public:
    OUString SAL_CALL getImplementationName() override
    {
        return u"org.libreoffice.comp.embindtest.Singleton"_ustr;
    }

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"org.libreoffice.embindtest.Singleton"_ustr };
    }

    OUString SAL_CALL getString() override { return "this is a string from XStringFactory"; }
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_embindtest_Singleton_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SingletonTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
