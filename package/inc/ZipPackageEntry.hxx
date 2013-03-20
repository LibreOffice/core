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
#pragma once
#if 1

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <ZipEntry.hxx>
#include <cppuhelper/implbase5.hxx>

class ZipPackageFolder;

class ZipPackageEntry : public cppu::WeakImplHelper5
<
    com::sun::star::container::XNamed,
    com::sun::star::container::XChild,
    com::sun::star::lang::XUnoTunnel,
    com::sun::star::beans::XPropertySet,
    com::sun::star::lang::XServiceInfo
>
{
protected:
    ::rtl::OUString msName;
    bool mbIsFolder:1;
    bool mbAllowRemoveOnInsert:1;
    // com::sun::star::uno::Reference < com::sun::star::container::XNameContainer > xParent;
    ::rtl::OUString     sMediaType;
    ZipPackageFolder * pParent;
public:
    ZipEntry aEntry;
    ZipPackageEntry ( bool bNewFolder = sal_False );
    virtual ~ZipPackageEntry( void );

    ::rtl::OUString & GetMediaType () { return sMediaType; }
    void SetMediaType ( const ::rtl::OUString & sNewType) { sMediaType = sNewType; }
    void doSetParent ( ZipPackageFolder * pNewParent, sal_Bool bInsert );
    bool IsFolder ( ) { return mbIsFolder; }
    ZipPackageFolder* GetParent ( ) { return pParent; }
    void SetFolder ( bool bSetFolder ) { mbIsFolder = bSetFolder; }

    void clearParent ( void )
    {
        // xParent.clear();
        pParent = NULL;
    }
    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);
    // XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent )
        throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException) = 0;
    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
