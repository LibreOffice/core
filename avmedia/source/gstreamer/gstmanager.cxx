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

#include "gstmanager.hxx"
#include "gstplayer.hxx"

using namespace ::com::sun::star;

namespace avmedia
{
namespace gst
{
// ----------------
// - Manager -
// ----------------

Manager::Manager( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr )
{}

// ------------------------------------------------------------------------------

Manager::~Manager()
{}

// ------------------------------------------------------------------------------

uno::Reference< media::XPlayer > SAL_CALL Manager::createPlayer( const ::rtl::OUString& rURL )
     throw( uno::RuntimeException )
{
    return( ::avmedia::gst::Player::create( rURL ) );
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Manager::getImplementationName()
     throw( uno::RuntimeException )
{
    return( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GSTREAMER_MANAGER_IMPLEMENTATIONNAME ) ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Manager::supportsService( const ::rtl::OUString& ServiceName )
     throw( uno::RuntimeException )
{
    return( ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( AVMEDIA_GSTREAMER_MANAGER_SERVICENAME ) ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Manager::getSupportedServiceNames()
     throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aRet( 1 );
    aRet[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GSTREAMER_MANAGER_SERVICENAME ) );

    return( aRet );
}
} // namespace gst
} // namespace avmedia
