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

#include "manager.hxx"
#include "player.hxx"

#include <tools/urlobj.hxx>

#define AVMEDIA_WIN_MANAGER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Manager_DirectX"
#define AVMEDIA_WIN_MANAGER_SERVICENAME "com.sun.star.media.Manager"

using namespace ::com::sun::star;

namespace avmedia { namespace win {
// ----------------
// - Manager -
// ----------------

Manager::Manager( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr )
{
}

// ------------------------------------------------------------------------------

Manager::~Manager()
{
}

// ------------------------------------------------------------------------------

uno::Reference< media::XPlayer > SAL_CALL Manager::createPlayer( const OUString& rURL )
    throw (uno::RuntimeException)
{
    Player*                             pPlayer( new Player( mxMgr ) );
    uno::Reference< media::XPlayer >    xRet( pPlayer );
    const INetURLObject                 aURL( rURL );

    if( !pPlayer->create( aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) )  )
        xRet = uno::Reference< media::XPlayer >();

    return xRet;
}

// ------------------------------------------------------------------------------

OUString SAL_CALL Manager::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return OUString( AVMEDIA_WIN_MANAGER_IMPLEMENTATIONNAME );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Manager::supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName == AVMEDIA_WIN_MANAGER_SERVICENAME;
}

// ------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL Manager::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_WIN_MANAGER_SERVICENAME ;

    return aRet;
}

} // namespace win
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
