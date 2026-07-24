/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>

class SvxSimpleUnoModel final : public cppu::WeakImplHelper<
                                    css::frame::XModel,
                                    css::ucb::XAnyCompareFactory,
                                    css::style::XStyleFamiliesSupplier,
                                    css::lang::XMultiServiceFactory >
{
public:
    SvxSimpleUnoModel();

    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > createInstanceWithArguments( const OUString& ServiceSpecifier, const cpo::uno::Sequence< cpo::uno::Any >& Arguments ) override;
    virtual cpo::uno::Sequence< OUString > getAvailableServiceNames(  ) override;

    // XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > getStyleFamilies(  ) override;

    // XAnyCompareFactory
    virtual css::uno::Reference< css::ucb::XAnyCompare > createAnyCompareByName( const OUString& PropertyName ) override;

    // XModel
    virtual bool attachResource( const OUString& aURL, const cpo::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual OUString getURL(  ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getArgs(  ) override;
    virtual void connectController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual void disconnectController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual void lockControllers(  ) override;
    virtual void unlockControllers(  ) override;
    virtual bool hasControllersLocked(  ) override;
    virtual css::uno::Reference< css::frame::XController > getCurrentController(  ) override;
    virtual void setCurrentController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual css::uno::Reference< css::uno::XInterface > getCurrentSelection(  ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
