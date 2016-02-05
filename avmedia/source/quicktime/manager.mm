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

#include "manager.hxx"
#include "player.hxx"
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

namespace avmedia { namespace quicktime {

// - Manager -


Manager::Manager( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr )
{
    SAL_INFO( "avmedia.quicktime", "Manager::Manager" );
}



Manager::~Manager()
{
}



css::uno::Reference< media::XPlayer > SAL_CALL Manager::createPlayer( const OUString& rURL )
    throw (css::uno::RuntimeException)
{
    Player*                             pPlayer( new Player( mxMgr ) );
    css::uno::Reference< media::XPlayer >    xRet( pPlayer );
    INetURLObject                       aURL( rURL );

    SAL_INFO( "avmedia.quicktime", "Manager::createPlayer" );

    if( !pPlayer->create( aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) )  )
        xRet.clear();

    return xRet;
}



OUString SAL_CALL Manager::getImplementationName(  )
    throw (css::uno::RuntimeException)
{
    return OUString( AVMEDIA_QUICKTIME_MANAGER_IMPLEMENTATIONNAME );
}



sal_Bool SAL_CALL Manager::supportsService( const OUString& ServiceName )
    throw (css::uno::RuntimeException)
{
    return ( ServiceName == AVMEDIA_QUICKTIME_MANAGER_SERVICENAME );
}



css::uno::Sequence< OUString > SAL_CALL Manager::getSupportedServiceNames(  )
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence<OUString> aRet { AVMEDIA_QUICKTIME_MANAGER_SERVICENAME };

    return aRet;
}

} // namespace quicktime
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
