/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMain.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <sal/types.h>

class Base : public cppu::WeakAggImplHelper1<css::lang::XInitialization>
{
public:
    void SAL_CALL initialize(css::uno::Sequence<css::uno::Any> const& aArguments) override;
};

class Good : public Base, public css::lang::XMain
{
public:
    css::uno::Any SAL_CALL queryInterface(css::uno::Type const& aType) override
    {
        return Base::queryInterface(aType);
    }
};

class Bad : public cppu::ImplInheritanceHelper<Base, css::lang::XMain>
{
public:
    sal_Int32 SAL_CALL run(css::uno::Sequence<OUString> const& aArguments) override;
};

// expected-error@cppuhelper/implbase.hxx:* {{'ImplInheritanceHelper<Base, com::sun::star::lang::XMain>' derives from XAggregation, but its implementation of queryInterface does not delegate to an appropriate base class queryInterface [loplugin:unoaggregation]}}
Bad bad; //make sure Bad's base cppu::ImplInheritanceHelper<Base, css::lang::XMain> is instantiated

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
