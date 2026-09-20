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

#include <sal/config.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

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
    explicit DialogModelProvider(cpo::uno::Reference< cpo::uno::XComponentContext > const & context);
private:
    // css::lang::XInitialization:
    virtual void initialize(const cpo::uno::Sequence< cpo::uno::Any > & aArguments) override;

    // css::container::XElementAccess:
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // css::container::XNameAccess:
    virtual cpo::uno::Any getByName(const OUString & aName) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName(const OUString & aName) override;

    // css::container::XNameReplace:
    virtual void replaceByName(const OUString & aName, const cpo::uno::Any & aElement) override;

    // css::container::XNameContainer:
    virtual void insertByName(const OUString & aName, const cpo::uno::Any & aElement) override;
    virtual void removeByName(const OUString & Name) override;

    // css::lang::XServiceInfo:
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString & ServiceName) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

private:
    DialogModelProvider(const DialogModelProvider &) = delete;
    DialogModelProvider& operator=(const DialogModelProvider &) = delete;

    // destructor is private and will be called indirectly by the release call    virtual ~DialogModelProvider() {}

    cpo::uno::Reference< cpo::uno::XComponentContext >      m_xContext;
    cpo::uno::Reference< css::container::XNameContainer>    m_xDialogModel;
    cpo::uno::Reference< css::beans::XPropertySet>          m_xDialogModelProp;
};
} // closing anonymous implementation namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
