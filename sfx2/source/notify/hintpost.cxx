/*************************************************************************
 *
 *  $RCSfile: hintpost.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "arrdecl.hxx"
#include "hintpost.hxx"
#include "app.hxx"
#include "sfxtypes.hxx"

//====================================================================

void SfxHintPoster::RegisterEvent()
{
    DBG_MEMTEST();
}

//--------------------------------------------------------------------

SfxHintPoster::SfxHintPoster()
{
    RegisterEvent();
}

//--------------------------------------------------------------------

SfxHintPoster::SfxHintPoster( const GenLink& rLink ):
    aLink(rLink)
{
}


//--------------------------------------------------------------------

SfxHintPoster::~SfxHintPoster()
{
}

//--------------------------------------------------------------------

void SfxHintPoster::Post( SfxHint* pHintToPost )
{
    GetpApp()->PostUserEvent( ( LINK(this, SfxHintPoster, DoEvent_Impl) ), pHintToPost );
    AddRef();
}

//--------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxHintPoster, DoEvent_Impl, SfxHint *, pPostedHint )
{
    DBG_MEMTEST();
    Event( pPostedHint );
    ReleaseRef();
    return 0;
}
IMPL_LINK_INLINE_END( SfxHintPoster, DoEvent_Impl, SfxHint *, pPostedHint )

//--------------------------------------------------------------------

void SfxHintPoster::Event( SfxHint* pPostedHint )
{
    aLink.Call( pPostedHint );
}

//--------------------------------------------------------------------

void SfxHintPoster::SetEventHdl( const GenLink& rLink )
{
    DBG_MEMTEST();
    aLink = rLink;
}


#define LOG( x )
#if 0
#define LOG( x )                                                \
{                                                               \
    SvFileStream aStrm( "f:\\temp\\log", STREAM_READWRITE );    \
    aStrm.Seek( STREAM_SEEK_TO_END );                           \
    aStrm << x.GetStr() << '\n';                                \
}
#endif


//--------------------------------------------------------------------

void AsynchronLink::CreateMutex()
{
    if( !_pMutex ) _pMutex = new NAMESPACE_VOS( OMutex );
}

void AsynchronLink::Call( void* pObj, BOOL bAllowDoubles, BOOL bUseTimer )
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
            LOG( String( "Remove " ) + String( _nEventId ) );
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
            LOG( String( "Posted " ) + String( _nEventId ) );
            if( _pMutex ) _pMutex->release();
        }
    }
}

AsynchronLink::~AsynchronLink()
{
    if( _nEventId )
    {
        LOG( String( "Remove " ) + String( _nEventId ) );
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

SV_IMPL_PTRARR( AsynchronLinkArr_Impl, AsynchronLink* );


