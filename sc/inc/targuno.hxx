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

#ifndef INCLUDED_SC_INC_TARGUNO_HXX
#define INCLUDED_SC_INC_TARGUNO_HXX

#include <svl/lstner.hxx>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/implbase.hxx>

class ScDocShell;

#define SC_LINKTARGETTYPE_SHEET     0
#define SC_LINKTARGETTYPE_RANGENAME 1
#define SC_LINKTARGETTYPE_DBAREA    2

#define SC_LINKTARGETTYPE_COUNT     3

#define SCLINKTARGET_SERVICE        "com.sun.star.document.LinkTarget"

//! Graphic / OleObject (need separate collections!)

class ScLinkTargetTypesObj : public ::cppu::WeakImplHelper<
                                css::container::XNameAccess,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    OUString                aNames[SC_LINKTARGETTYPE_COUNT];

public:
                            ScLinkTargetTypesObj(ScDocShell* pDocSh);
    virtual                 ~ScLinkTargetTypesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // css::container::XNameAccess
    virtual css::uno::Any SAL_CALL     getByName(const OUString& aName)
                                throw(  css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasByName(const OUString& aName) throw( css::uno::RuntimeException, std::exception ) override;

                            // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL        getElementType() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasElements() throw( css::uno::RuntimeException, std::exception ) override;

                            // css::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;
};

class ScLinkTargetTypeObj : public ::cppu::WeakImplHelper<
                                css::beans::XPropertySet,
                                css::document::XLinkTargetSupplier,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    sal_uInt16              nType;
    OUString                aName;

public:
                            ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT);
    virtual                 ~ScLinkTargetTypeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    static void             SetLinkTargetBitmap( css::uno::Any& rRet, sal_uInt16 nType );

                            // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL  getPropertySetInfo() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL   setPropertyValue(const OUString& aPropertyName,
                                    const css::uno::Any& aValue)
                                throw(  css::beans::UnknownPropertyException,
                                        css::beans::PropertyVetoException,
                                         css::lang::IllegalArgumentException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL     getPropertyValue(const OUString& PropertyName)
                                throw(  css::beans::UnknownPropertyException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           addPropertyChangeListener(const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener > & xListener)
                                throw(  css::beans::UnknownPropertyException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           removePropertyChangeListener(const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener)
                                throw(  css::beans::UnknownPropertyException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           addVetoableChangeListener(const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener)
                                throw(  css::beans::UnknownPropertyException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           removeVetoableChangeListener(const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener)
                                throw(  css::beans::UnknownPropertyException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception ) override;

                            // css::document::XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL  getLinks() throw( css::uno::RuntimeException, std::exception ) override;

                            // css::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;
};

class ScLinkTargetsObj : public ::cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    css::uno::Reference< css::container::XNameAccess >            xCollection;

public:
                            ScLinkTargetsObj( const css::uno::Reference< css::container::XNameAccess > & rColl );
    virtual                 ~ScLinkTargetsObj();

                            // css::container::XNameAccess
    virtual css::uno::Any SAL_CALL         getByName(const OUString& aName)
                                throw(  css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                         css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasByName(const OUString& aName) throw( css::uno::RuntimeException, std::exception ) override;

                            // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL        getElementType() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasElements() throw( css::uno::RuntimeException, std::exception ) override;

                            // css::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
