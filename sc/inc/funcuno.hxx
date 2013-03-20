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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase3.hxx>
#include <svl/lstner.hxx>

class ScDocument;
class ScDocOptions;


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    ScFunctionAccess_CreateInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >& );


class ScTempDocCache
{
private:
    ScDocument*     pDoc;
    sal_Bool            bInUse;

public:
                ScTempDocCache();
                ~ScTempDocCache();

    ScDocument* GetDocument() const     { return pDoc; }
    sal_Bool        IsInUse() const         { return bInUse; }
    void        SetInUse( sal_Bool bSet )   { bInUse = bSet; }

    void        SetDocument( ScDocument* pNew );
    void        Clear();
};

class ScFunctionAccess : public cppu::WeakImplHelper3<
                                        com::sun::star::sheet::XFunctionAccess,
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::lang::XServiceInfo>,
                         public SfxListener
{
private:
    ScTempDocCache  aDocCache;
    ScDocOptions*   pOptions;
    SfxItemPropertyMap aPropertyMap;
    bool            mbArray;
    bool            mbValid;

public:
                            ScFunctionAccess();
    virtual                 ~ScFunctionAccess();

    static ::rtl::OUString  getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XFunctionAccess
    virtual ::com::sun::star::uno::Any SAL_CALL callFunction(
                                    const ::rtl::OUString& aName,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::uno::Any >& aArguments )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
