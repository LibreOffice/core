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
                                ::com::sun::star::container::XNameAccess,
                                ::com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    OUString                aNames[SC_LINKTARGETTYPE_COUNT];

public:
                            ScLinkTargetTypesObj(ScDocShell* pDocSh);
    virtual                 ~ScLinkTargetTypesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL     getByName(const OUString& aName)
                                throw(  ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL      getElementNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasByName(const OUString& aName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

                            // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL        getElementType() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasElements() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

                            // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL      getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

class ScLinkTargetTypeObj : public ::cppu::WeakImplHelper<
                                ::com::sun::star::beans::XPropertySet,
                                ::com::sun::star::document::XLinkTargetSupplier,
                                ::com::sun::star::lang::XServiceInfo >,
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

    static void             SetLinkTargetBitmap( ::com::sun::star::uno::Any& rRet, sal_uInt16 nType );

                            // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL  getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL   setPropertyValue(const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::beans::PropertyVetoException,
                                         ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL     getPropertyValue(const OUString& PropertyName)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           addPropertyChangeListener(const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & xListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           removePropertyChangeListener(const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           addVetoableChangeListener(const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           removeVetoableChangeListener(const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception ) override;

                            // ::com::sun::star::document::XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL  getLinks() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

                            // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

class ScLinkTargetsObj : public ::cppu::WeakImplHelper<
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >            xCollection;

public:
                            ScLinkTargetsObj( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & rColl );
    virtual                 ~ScLinkTargetsObj();

                            // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL         getByName(const OUString& aName)
                                throw(  ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                         ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL      getElementNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasByName(const OUString& aName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

                            // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL        getElementType() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           hasElements() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

                            // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL      getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
