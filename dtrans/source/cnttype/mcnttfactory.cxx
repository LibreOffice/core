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

#include <osl/diagnose.h>
#include "mcnttfactory.hxx"
#include "mcnttype.hxx"

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define MIMECONTENTTYPEFACTORY_IMPL_NAME  "com.sun.star.datatransfer.MimeCntTypeFactory"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    Sequence< OUString > SAL_CALL MimeContentTypeFactory_getSupportedServiceNames( )
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString("com.sun.star.datatransfer.MimeContentTypeFactory");
        return aRet;
    }
}

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CMimeContentTypeFactory::CMimeContentTypeFactory( const Reference< XMultiServiceFactory >& rSrvMgr ) :
    m_SrvMgr( rSrvMgr )
{
}

//------------------------------------------------------------------------
// createMimeContentType
//------------------------------------------------------------------------

Reference< XMimeContentType > CMimeContentTypeFactory::createMimeContentType( const OUString& aContentType )
    throw( IllegalArgumentException, RuntimeException )
{
    MutexGuard aGuard( m_aMutex );
    return Reference< XMimeContentType >( new CMimeContentType( aContentType ) );
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CMimeContentTypeFactory::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString( MIMECONTENTTYPEFACTORY_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CMimeContentTypeFactory::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = MimeContentTypeFactory_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CMimeContentTypeFactory::getSupportedServiceNames( )
    throw( RuntimeException )
{
    return MimeContentTypeFactory_getSupportedServiceNames( );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
