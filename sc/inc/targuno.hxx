/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_TARGUNO_HXX
#define SC_TARGUNO_HXX

#include <svl/lstner.hxx>
#include <tools/string.hxx>
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
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>

class ScDocShell;


#define SC_LINKTARGETTYPE_SHEET     0
#define SC_LINKTARGETTYPE_RANGENAME 1
#define SC_LINKTARGETTYPE_DBAREA    2

#define SC_LINKTARGETTYPE_COUNT     3

#define SCLINKTARGET_SERVICE        "com.sun.star.document.LinkTarget"


//! Graphic / OleObject (need separate collections!)


class ScLinkTargetTypesObj : public ::cppu::WeakImplHelper2<
                                ::com::sun::star::container::XNameAccess,
                                ::com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    String                  aNames[SC_LINKTARGETTYPE_COUNT];

public:
                            ScLinkTargetTypesObj(ScDocShell* pDocSh);
    virtual                 ~ScLinkTargetTypesObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL     getByName(const ::rtl::OUString& aName)
                                throw(  ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasByName(const ::rtl::OUString& aName) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL        getElementType(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasElements(void) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL            getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};


class ScLinkTargetTypeObj : public ::cppu::WeakImplHelper3<
                                ::com::sun::star::beans::XPropertySet,
                                ::com::sun::star::document::XLinkTargetSupplier,
                                ::com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    sal_uInt16              nType;
    String                  aName;

public:
                            ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT);
    virtual                 ~ScLinkTargetTypeObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    static void             SetLinkTargetBitmap( ::com::sun::star::uno::Any& rRet, sal_uInt16 nType );

                            // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL  getPropertySetInfo(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   setPropertyValue(const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::beans::PropertyVetoException,
                                         ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL     getPropertyValue(const ::rtl::OUString& PropertyName)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           addPropertyChangeListener(const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & xListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           removePropertyChangeListener(const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           addVetoableChangeListener(const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           removeVetoableChangeListener(const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener)
                                throw(  ::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::document::XLinkTargetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL  getLinks(void) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL            getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};


class ScLinkTargetsObj : public ::cppu::WeakImplHelper2<
                            ::com::sun::star::container::XNameAccess,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >            xCollection;

public:
                            ScLinkTargetsObj( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & rColl );
    virtual                 ~ScLinkTargetsObj();

                            // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL         getByName(const ::rtl::OUString& aName)
                                throw(  ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                         ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasByName(const ::rtl::OUString& aName) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL        getElementType(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           hasElements(void) throw( ::com::sun::star::uno::RuntimeException );

                            // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL            getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL      getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
