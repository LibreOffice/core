/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
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
        aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.MimeContentTypeFactory");
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
    return OUString::createFromAscii( MIMECONTENTTYPEFACTORY_IMPL_NAME );
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
