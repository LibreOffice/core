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

#ifndef INCLUDED_SCRIPTING_SOURCE_DLGPROV_DIALOGMODELPROVIDER_HXX
#define INCLUDED_SCRIPTING_SOURCE_DLGPROV_DIALOGMODELPROVIDER_HXX

#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include <cppuhelper/implbase.hxx>
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"

/// anonymous implementation namespace
namespace dlgprov{

class DialogModelProvider:
    public ::cppu::WeakImplHelper<
        css::lang::XInitialization,
        css::container::XNameContainer,
        css::beans::XPropertySet,
        css::lang::XServiceInfo>
{
public:
    explicit DialogModelProvider(css::uno::Reference< css::uno::XComponentContext > const & context);
private:
    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) override;

    // css::container::XElementAccess:
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // css::container::XNameAccess:
    virtual css::uno::Any SAL_CALL getByName(const OUString & aName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString & aName) override;

    // css::container::XNameReplace:
    virtual void SAL_CALL replaceByName(const OUString & aName, const css::uno::Any & aElement) override;

    // css::container::XNameContainer:
    virtual void SAL_CALL insertByName(const OUString & aName, const css::uno::Any & aElement) override;
    virtual void SAL_CALL removeByName(const OUString & Name) override;

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    DialogModelProvider(const DialogModelProvider &) = delete;
    DialogModelProvider& operator=(const DialogModelProvider &) = delete;

    // destructor is private and will be called indirectly by the release call    virtual ~DialogModelProvider() {}

    css::uno::Reference< css::uno::XComponentContext >      m_xContext;
    css::uno::Reference< css::container::XNameContainer>    m_xDialogModel;
    css::uno::Reference< css::beans::XPropertySet>          m_xDialogModelProp;
};
} // closing anonymous implementation namespace

#endif // INCLUDED_SCRIPTING_SOURCE_DLGPROV_DIALOGMODELPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
