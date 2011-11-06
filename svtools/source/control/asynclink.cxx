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
#include "precompiled_svtools.hxx"

#include <svtools/asynclink.hxx>
#include <vos/mutex.hxx>
#include <tools/debug.hxx>
#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>

//--------------------------------------------------------------------
namespace svtools {

void AsynchronLink::CreateMutex()
{
    if( !_pMutex ) _pMutex = new vos::OMutex;
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
