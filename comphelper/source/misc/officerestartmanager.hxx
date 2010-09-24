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

#ifndef __OFFICESTARTMANAGER_HXX_
#define __OFFICESTARTMANAGER_HXX_

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

    sal_Bool m_bOfficeInitialized;
    sal_Bool m_bRestartRequested;

public:
    OOfficeRestartManager( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext )
    : m_xContext( xContext )
    , m_bOfficeInitialized( sal_False )
    , m_bRestartRequested( sal_False )
    {}

    virtual ~OOfficeRestartManager()
    {}

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
            getSupportedServiceNames_static();

    static ::rtl::OUString SAL_CALL getImplementationName_static();

    static ::rtl::OUString SAL_CALL getSingletonName_static();

    static ::rtl::OUString SAL_CALL getServiceName_static();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

// XRestartManager
    virtual void SAL_CALL requestRestart( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xInteractionHandler ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isRestartRequested( ::sal_Bool bInitialized ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// XCallback
    virtual void SAL_CALL notify( const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};

} // namespace comphelper

#endif

