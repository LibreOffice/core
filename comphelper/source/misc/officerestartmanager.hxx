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

#ifndef INCLUDED_COMPHELPER_SOURCE_MISC_OFFICERESTARTMANAGER_HXX
#define INCLUDED_COMPHELPER_SOURCE_MISC_OFFICERESTARTMANAGER_HXX

#include <com/sun/star/task/XRestartManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XCallback.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase3.hxx>

namespace comphelper
{

class OOfficeRestartManager : public ::cppu::WeakImplHelper3< ::com::sun::star::task::XRestartManager
                                                            , ::com::sun::star::awt::XCallback
                                                            , ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    bool m_bOfficeInitialized;
    bool m_bRestartRequested;

public:
    OOfficeRestartManager( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext )
    : m_xContext( xContext )
    , m_bOfficeInitialized( false )
    , m_bRestartRequested( false )
    {}

    virtual ~OOfficeRestartManager()
    {}

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL
            getSupportedServiceNames_static();

    static OUString SAL_CALL getImplementationName_static();

    static OUString SAL_CALL getSingletonName_static();

    static OUString SAL_CALL getServiceName_static();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

// XRestartManager
    virtual void SAL_CALL requestRestart( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xInteractionHandler ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isRestartRequested( sal_Bool bInitialized ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XCallback
    virtual void SAL_CALL notify( const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
