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
#ifndef _SVX_UNOSHGRP_HXX
#define _SVX_UNOSHGRP_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <cppuhelper/implbase3.hxx>
#include "svx/svxdllapi.h"

class SvxShapeCollectionMutex
{
public:
    ::osl::Mutex maMutex;
};

SVX_DLLPUBLIC com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SvxShapeCollection_NewInstance() throw();

/***********************************************************************
*                                                                      *
***********************************************************************/
class SVX_DLLPUBLIC SvxShapeCollection :    public ::cppu::WeakAggImplHelper3<
                                            ::com::sun::star::drawing::XShapes,
                                            ::com::sun::star::lang::XServiceInfo,
                                            ::com::sun::star::lang::XComponent
                                            >,
                            public SvxShapeCollectionMutex
{
private:
    cppu::OInterfaceContainerHelper maShapeContainer;

    cppu::OBroadcastHelper mrBHelper;

    SVX_DLLPRIVATE virtual void disposing() throw();

public:
    SvxShapeCollection() throw();
    virtual ~SvxShapeCollection() throw();

    // XInterface
    virtual void SAL_CALL release() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();
    static ::rtl::OUString getImplementationName_Static();
};

::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SvxShapeCollection_createInstance( const com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
