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


#ifndef _MCNTTFACTORY_HXX_
#define _MCNTTFACTORY_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>

class CMimeContentTypeFactory : public
    cppu::WeakImplHelper2< com::sun::star::datatransfer::XMimeContentTypeFactory, \
    com::sun::star::lang::XServiceInfo >
{

public:
    CMimeContentTypeFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSrvMgr );

    //------------------------------------------------
    // XMimeContentTypeFactory
    //------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentType > SAL_CALL createMimeContentType( const ::rtl::OUString& aContentType )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

private:
    ::osl::Mutex                                                                           m_aMutex;
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_SrvMgr;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
