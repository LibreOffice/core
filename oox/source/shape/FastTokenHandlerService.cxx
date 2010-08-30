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

#include "FastTokenHandlerService.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include "oox/helper/helper.hxx"

namespace oox {
namespace shape {

// ============================================================================

using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

OUString SAL_CALL FastTokenHandlerService_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.FastTokenHandlerService" );
}

Sequence< OUString > SAL_CALL FastTokenHandlerService_getSupportedServiceNames()
{
    Sequence< OUString > s( 1 );
    s[ 0 ] = CREATE_OUSTRING( "com.sun.star.xml.sax.FastTokenHandler" );
    return s;
}

Reference< XInterface > SAL_CALL FastTokenHandlerService_createInstance( const Reference< XComponentContext >& rxContext ) throw (Exception)
{
    return static_cast< ::cppu::OWeakObject* >( new FastTokenHandlerService( rxContext ) );
}

// ============================================================================

FastTokenHandlerService::FastTokenHandlerService( const Reference< XComponentContext >& rxContext ) :
    m_xContext( rxContext )
{
}

FastTokenHandlerService::~FastTokenHandlerService()
{
}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL FastTokenHandlerService::getImplementationName() throw (RuntimeException)
{
    return FastTokenHandlerService_getImplementationName();
}

sal_Bool SAL_CALL FastTokenHandlerService::supportsService( const OUString& serviceName ) throw (RuntimeException)
{
    Sequence< OUString > serviceNames = FastTokenHandlerService_getSupportedServiceNames();
    for( sal_Int32 i = 0; i < serviceNames.getLength(); ++i )
    {
        if( serviceNames[ i ] == serviceName )
            return sal_True;
    }
    return sal_False;
}

Sequence< OUString > SAL_CALL FastTokenHandlerService::getSupportedServiceNames() throw (RuntimeException)
{
    return FastTokenHandlerService_getSupportedServiceNames();
}

// ::com::sun::star::xml::sax::XFastTokenHandler:
sal_Int32 SAL_CALL FastTokenHandlerService::getToken( const OUString& Identifier ) throw (RuntimeException)
{
    return mFastTokenHandler.getToken(Identifier);
}

OUString SAL_CALL FastTokenHandlerService::getIdentifier( sal_Int32 Token ) throw (RuntimeException)
{
    return mFastTokenHandler.getIdentifier(Token);
}

Sequence< sal_Int8 > SAL_CALL FastTokenHandlerService::getUTF8Identifier( sal_Int32 Token ) throw (RuntimeException)
{
    return mFastTokenHandler.getUTF8Identifier(Token);
}

sal_Int32 SAL_CALL FastTokenHandlerService::getTokenFromUTF8( const Sequence< sal_Int8 >& Identifier ) throw (RuntimeException)
{
    return mFastTokenHandler.getTokenFromUTF8(Identifier);
}

// ============================================================================

} // namspace shape
} // namspace oox
