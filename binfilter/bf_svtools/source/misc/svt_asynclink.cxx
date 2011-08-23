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

#include <asynclink.hxx>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>

//--------------------------------------------------------------------

namespace binfilter
{

void AsynchronLink::CreateMutex()
{
    if( !_pMutex ) _pMutex = new osl::Mutex;
}

void AsynchronLink::Call( void* pObj, BOOL
#ifdef DBG_UTIL
bAllowDoubles
#endif
, BOOL bUseTimer )
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
    if( _pDeleted ) *_pDeleted = TRUE;
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
    _bInCall = TRUE;
    BOOL bDeleted = FALSE;
    _pDeleted = &bDeleted;
    _aLink.Call( pArg );
    if( !bDeleted )
    {
        _bInCall = FALSE;
        _pDeleted = 0;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
