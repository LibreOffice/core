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

#include <com/sun/star/logging/XSimpleLogRing.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase3.hxx>

#define SIMPLELOGRING_SIZE 256

namespace comphelper
{

class OSimpleLogRing : public ::cppu::WeakImplHelper3< ::com::sun::star::logging::XSimpleLogRing,
                                                           ::com::sun::star::lang::XInitialization,
                                                           ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aMessages;

    sal_Bool m_bInitialized;
    sal_Bool m_bFull;
    sal_Int32 m_nPos;

public:
    OSimpleLogRing();
    virtual ~OSimpleLogRing();

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
            getSupportedServiceNames_static();

    static ::rtl::OUString SAL_CALL getImplementationName_static();

    static ::rtl::OUString SAL_CALL getSingletonName_static();

    static ::rtl::OUString SAL_CALL getServiceName_static();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

// XSimpleLogRing
    virtual void SAL_CALL logString( const ::rtl::OUString& aMessage ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCollectedLog() throw (::com::sun::star::uno::RuntimeException);

// XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};

} // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
