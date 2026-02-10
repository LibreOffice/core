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
#include <org/libreoffice/embindtest/XArgumentStore.hpp>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace
{
class ConstructorsTest : public cppu::WeakImplHelper<org::libreoffice::embindtest::XArgumentStore,
                                                     css::lang::XServiceInfo>
{
public:
    explicit ConstructorsTest(css::uno::Sequence<css::uno::Any> const& args)
        : m_aArgs(args)
    {
    }

    OUString SAL_CALL getImplementationName() override
    {
        return u"org.libreoffice.comp.embindtest.Constructors"_ustr;
    }

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"org.libreoffice.embindtest.ImplicitConstructor"_ustr,
                 u"org.libreoffice.embindtest.ExplicitConstructors"_ustr };
    }

    css::uno::Sequence<css::uno::Any> SAL_CALL getArguments() override { return m_aArgs; }

private:
    css::uno::Sequence<css::uno::Any> m_aArgs;
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_embindtest_Constructors_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new ConstructorsTest(args));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
