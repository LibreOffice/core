/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asynclink.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:36:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <asynclink.hxx>

#ifndef  _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef  _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef   _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef   _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//--------------------------------------------------------------------
namespace svtools {

void AsynchronLink::CreateMutex()
{
    if( !_pMutex ) _pMutex = new vos::OMutex;
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

void AsynchronLink::ForcePendingCall()
{
    ClearPendingCall();
    Call_Impl( _pArg );
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
