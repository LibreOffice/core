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

#include "oox/core/fasttokenhandler.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include "oox/helper/helper.hxx"
#include "oox/token/tokenmap.hxx"

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

OUString SAL_CALL FastTokenHandler_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.core.FastTokenHandler" );
}

Sequence< OUString > SAL_CALL FastTokenHandler_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = CREATE_OUSTRING( "com.sun.star.xml.sax.FastTokenHandler" );
    return aServiceNames;
}

Reference< XInterface > SAL_CALL FastTokenHandler_createInstance( const Reference< XComponentContext >& /*rxContext*/ ) throw (Exception)
{
    return static_cast< ::cppu::OWeakObject* >( new FastTokenHandler );
}

// ============================================================================

FastTokenHandler::FastTokenHandler() :
    mrTokenMap( StaticTokenMap::get() )
{
}

FastTokenHandler::~FastTokenHandler()
{
}

// XServiceInfo

OUString SAL_CALL FastTokenHandler::getImplementationName() throw (RuntimeException)
{
    return FastTokenHandler_getImplementationName();
}

sal_Bool SAL_CALL FastTokenHandler::supportsService( const OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< OUString > aServiceNames = FastTokenHandler_getSupportedServiceNames();
    for( sal_Int32 nIndex = 0, nLength = aServiceNames.getLength(); nIndex < nLength; ++nIndex )
        if( aServiceNames[ nIndex ] == rServiceName )
            return sal_True;
    return sal_False;
}

Sequence< OUString > SAL_CALL FastTokenHandler::getSupportedServiceNames() throw (RuntimeException)
{
    return FastTokenHandler_getSupportedServiceNames();
}

// XFastTokenHandler

sal_Int32 FastTokenHandler::getToken( const OUString& rIdentifier ) throw( RuntimeException )
{
    return mrTokenMap.getTokenFromUnicode( rIdentifier );
}

OUString FastTokenHandler::getIdentifier( sal_Int32 nToken ) throw( RuntimeException )
{
    return mrTokenMap.getUnicodeTokenName( nToken );
}

Sequence< sal_Int8 > FastTokenHandler::getUTF8Identifier( sal_Int32 nToken ) throw( RuntimeException )
{
    return mrTokenMap.getUtf8TokenName( nToken );
}

sal_Int32 FastTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& rIdentifier ) throw( RuntimeException )
{
    return mrTokenMap.getTokenFromUtf8( rIdentifier );
}

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */