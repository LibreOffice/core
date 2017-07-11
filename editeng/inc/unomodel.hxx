/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#ifndef INCLUDED_EDITENG_UNOMODEL_HXX
#define INCLUDED_EDITENG_UNOMODEL_HXX

#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase4.hxx>

class EDITENG_DLLPUBLIC SvxSimpleUnoModel : public cppu::WeakAggImplHelper4<
                                    css::frame::XModel,
                                    css::ucb::XAnyCompareFactory,
                                    css::style::XStyleFamiliesSupplier,
                                    css::lang::XMultiServiceFactory >
{
public:
    SvxSimpleUnoModel();

    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;

    // XStyleFamiliesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getStyleFamilies(  ) override;

    // XAnyCompareFactory
    virtual css::uno::Reference< css::ucb::XAnyCompare > SAL_CALL createAnyCompareByName( const OUString& PropertyName ) override;

    // XModel
    virtual sal_Bool SAL_CALL attachResource( const OUString& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual OUString SAL_CALL getURL(  ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs(  ) override;
    virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual void SAL_CALL lockControllers(  ) override;
    virtual void SAL_CALL unlockControllers(  ) override;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) override;
    virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController(  ) override;
    virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& xController ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection(  ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
