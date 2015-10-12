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
    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::container::XElementAccess:
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::container::XNameAccess:
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const OUString & aName) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName(const OUString & aName) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::container::XNameReplace:
    virtual void SAL_CALL replaceByName(const OUString & aName, const ::com::sun::star::uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::container::XNameContainer:
    virtual void SAL_CALL insertByName(const OUString & aName, const ::com::sun::star::uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeByName(const OUString & Name) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
