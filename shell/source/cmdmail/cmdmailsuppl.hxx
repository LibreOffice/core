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

#ifndef _CMDMAILSUPPL_HXX_
#define _CMDMAILSUPPL_HXX_

//----------------------------------------------------------
//  includes of other projects
//----------------------------------------------------------

#include <cppuhelper/implbase3.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/system/XSimpleMailClient.hpp>

#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class CmdMailSuppl :
    public  cppu::WeakImplHelper3<
        com::sun::star::system::XSimpleMailClientSupplier,
        com::sun::star::system::XSimpleMailClient,
        com::sun::star::lang::XServiceInfo >
{

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xConfigurationProvider;

public:
    CmdMailSuppl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

    //------------------------------------------------
    // XSimpleMailClientSupplier
    //------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::system::XSimpleMailClient > SAL_CALL querySimpleMailClient(  )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XSimpleMailClient
    //------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::system::XSimpleMailMessage > SAL_CALL createSimpleMailMessage(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL sendSimpleMailMessage( const ::com::sun::star::uno::Reference< ::com::sun::star::system::XSimpleMailMessage >& xSimpleMailMessage, sal_Int32 aFlag )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
