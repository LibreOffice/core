/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/testuno/XArgumentStore.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace
{
class ConstructorsTest
    : public cppu::WeakImplHelper<css::testuno::XArgumentStore, css::lang::XServiceInfo>
{
public:
    explicit ConstructorsTest(css::uno::Sequence<css::uno::Any> const& args)
        : m_aArgs(args)
    {
    }

    OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.testuno.Constructors"_ustr;
    }

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.testuno.ImplicitConstructor"_ustr,
                 u"com.sun.star.testuno.ExplicitConstructors"_ustr };
    }

    css::uno::Sequence<css::uno::Any> SAL_CALL getArguments() override { return m_aArgs; }

private:
    css::uno::Sequence<css::uno::Any> m_aArgs;
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_testuno_Constructors_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new ConstructorsTest(args));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
