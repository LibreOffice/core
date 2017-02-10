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

#ifndef INCLUDED_SD_INC_STLFAMILY_HXX
#define INCLUDED_SD_INC_STLFAMILY_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <rtl/ref.hxx>

#include <cppuhelper/implbase.hxx>

#include <svl/style.hxx>

#include <editeng/unoipset.hxx>

class SdStyleSheet;
class SdPage;
struct SdStyleFamilyImpl;

class SdStyleFamily : public ::cppu::WeakImplHelper< css::container::XNameContainer, css::container::XNamed, css::container::XIndexAccess, css::lang::XSingleServiceFactory,  css::lang::XServiceInfo, css::lang::XComponent, css::beans::XPropertySet >
{
public:
    /// creates the style family for the given SfxStyleFamily
    SdStyleFamily( const rtl::Reference< SfxStyleSheetPool >& xPool, SfxStyleFamily nFamily );

    /// creates the presentation family for the given masterpage
    SdStyleFamily( const rtl::Reference< SfxStyleSheetPool >& xPool, const SdPage* pMasterPage );

    virtual ~SdStyleFamily() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // XSingleServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(  ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName,const css::uno::Reference<css::beans::XVetoableChangeListener>&aListener ) override;

private:
    /// @throws css::uno::RuntimeException
    void throwIfDisposed() const;
    /// @throws css::lang::IllegalArgumentException
    SdStyleSheet* GetValidNewSheet( const css::uno::Any& rElement );
    /// @throws css::container::NoSuchElementException
    /// @throws css::lang::WrappedTargetException
    SdStyleSheet* GetSheetByName( const OUString& rName );

    SfxStyleFamily mnFamily;
    rtl::Reference< SfxStyleSheetPool > mxPool;
    std::unique_ptr<SdStyleFamilyImpl>  mpImpl;
};

typedef rtl::Reference< SdStyleFamily > SdStyleFamilyRef;

#endif // INCLUDED_SD_INC_STLFAMILY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
