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

#ifndef INCLUDED_SHELL_SOURCE_CMDMAIL_CMDMAILSUPPL_HXX
#define INCLUDED_SHELL_SOURCE_CMDMAIL_CMDMAILSUPPL_HXX

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/system/XSimpleMailClient.hpp>

#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>


// class declaration


class CmdMailSuppl :
    public  cppu::WeakImplHelper<
        com::sun::star::system::XSimpleMailClientSupplier,
        com::sun::star::system::XSimpleMailClient,
        com::sun::star::lang::XServiceInfo >
{

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xConfigurationProvider;

public:
    explicit CmdMailSuppl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );


    // XSimpleMailClientSupplier


    virtual ::com::sun::star::uno::Reference< ::com::sun::star::system::XSimpleMailClient > SAL_CALL querySimpleMailClient(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;


    // XSimpleMailClient


    virtual ::com::sun::star::uno::Reference< ::com::sun::star::system::XSimpleMailMessage > SAL_CALL createSimpleMailMessage(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL sendSimpleMailMessage( const ::com::sun::star::uno::Reference< ::com::sun::star::system::XSimpleMailMessage >& xSimpleMailMessage, sal_Int32 aFlag )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;


    // XServiceInfo


    virtual OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
