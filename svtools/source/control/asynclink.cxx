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


#include <asynclink.hxx>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>

//--------------------------------------------------------------------
namespace svtools {

void AsynchronLink::CreateMutex()
{
    if( !_pMutex ) _pMutex = new osl::Mutex;
}

void AsynchronLink::Call( void* pObj, sal_Bool
#ifdef DBG_UTIL
bAllowDoubles
#endif
, sal_Bool bUseTimer )
{
#ifdef DBG_UTIL
    if ( bUseTimer || !_bInCall )
        DBG_WARNING( "Recursives Call. Eher ueber Timer. TLX Fragen" );
#endif
    if( _aLink.IsSet() )
    {
        _pArg = pObj;
        DBG_ASSERT( bAllowDoubles ||
                    ( !_nEventId && ( !_pTimer || !_pTimer->IsActive() ) ),
                    "Schon ein Call unterwegs" );
        if( _nEventId )
        {
            if( _pMutex ) _pMutex->acquire();
            Application::RemoveUserEvent( _nEventId );
            if( _pMutex ) _pMutex->release();
        }
        if( _pTimer )_pTimer->Stop();
        if( bUseTimer )
        {
            if( !_pTimer )
            {
                _pTimer = new Timer;
                _pTimer->SetTimeout( 0 );
                _pTimer->SetTimeoutHdl( STATIC_LINK(
                    this, AsynchronLink, HandleCall) );
            }
            _pTimer->Start();
        }
        else
        {
            if( _pMutex ) _pMutex->acquire();
            Application::PostUserEvent( _nEventId, STATIC_LINK( this, AsynchronLink, HandleCall), 0 );
            if( _pMutex ) _pMutex->release();
        }
    }
}

AsynchronLink::~AsynchronLink()
{
    if( _nEventId )
    {
        Application::RemoveUserEvent( _nEventId );
    }
    delete _pTimer;
    if( _pDeleted ) *_pDeleted = sal_True;
    delete _pMutex;
}

IMPL_STATIC_LINK( AsynchronLink, HandleCall, void*, EMPTYARG )
{
    if( pThis->_pMutex ) pThis->_pMutex->acquire();
    pThis->_nEventId = 0;
    if( pThis->_pMutex ) pThis->_pMutex->release();
    pThis->Call_Impl( pThis->_pArg );
    return 0;
}

void AsynchronLink::ClearPendingCall()
{
    if( _pMutex ) _pMutex->acquire();
    if( _nEventId )
    {
        Application::RemoveUserEvent( _nEventId );
        _nEventId = 0;
    }
    if( _pMutex ) _pMutex->release();
    if( _pTimer ) _pTimer->Stop();
}

void AsynchronLink::Call_Impl( void* pArg )
{
    _bInCall = sal_True;
    sal_Bool bDeleted = sal_False;
    _pDeleted = &bDeleted;
    _aLink.Call( pArg );
    if( !bDeleted )
    {
        _bInCall = sal_False;
        _pDeleted = 0;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
