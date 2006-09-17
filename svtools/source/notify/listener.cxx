/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listener.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:19:02 $
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
#ifndef GCC
#endif

#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "broadcast.hxx"
#include "listener.hxx"
#include "listenerbase.hxx"
#include "listeneriter.hxx"


//====================================================================
TYPEINIT0(SvtListener);

//====================================================================
// simple ctor of class SvtListener

SvtListener::SvtListener()
    : pBrdCastLst( 0 )
{
}
//--------------------------------------------------------------------

// copy ctor of class SvtListener

SvtListener::SvtListener( const SvtListener &rListener )
    : pBrdCastLst( 0 )
{
    SvtListenerBase* pLst = rListener.pBrdCastLst;
    while( pLst )
    {
        new SvtListenerBase( *this, *pLst->GetBroadcaster() );
        pLst = pLst->GetNext();
    }
}
//--------------------------------------------------------------------

// unregisteres the SvtListener from its SvtBroadcasters

SvtListener::~SvtListener()
{
    EndListeningAll();
}

//--------------------------------------------------------------------

// registeres at a specific SvtBroadcaster

BOOL SvtListener::StartListening( SvtBroadcaster& rBroadcaster )
{
    const SvtListenerBase* pLst = pBrdCastLst;
    while( pLst )
    {
        if( &rBroadcaster == pLst->GetBroadcaster() )
        {
            // double, than return
            return FALSE;
        }
        pLst = pLst->GetNext();
    }
    new SvtListenerBase( *this, rBroadcaster );
    return TRUE;
}

//--------------------------------------------------------------------

// unregisteres at a specific SvtBroadcaster

BOOL SvtListener::EndListening( SvtBroadcaster& rBroadcaster )
{
    SvtListenerBase *pLst = pBrdCastLst, *pPrev = pLst;
    while( pLst )
    {
        if( &rBroadcaster == pLst->GetBroadcaster() )
        {
            if( pBrdCastLst == pLst )
                pBrdCastLst = pLst->GetNext();
            else
                pPrev->SetNext( pLst->GetNext() );

            delete pLst;
            return TRUE;
        }
        pPrev = pLst;
        pLst = pLst->GetNext();
    }
    return FALSE;
}

//--------------------------------------------------------------------

// unregisteres all Broadcasters

void SvtListener::EndListeningAll()
{
    SvtListenerBase *pLst = pBrdCastLst;
    while( pLst )
    {
        SvtListenerBase *pDel = pLst;
        pLst = pLst->GetNext();

        delete pDel;
    }
    pBrdCastLst = 0;
}

//--------------------------------------------------------------------

BOOL SvtListener::IsListening( SvtBroadcaster& rBroadcaster ) const
{
    const SvtListenerBase *pLst = pBrdCastLst;
    while( pLst )
    {
        if( &rBroadcaster == pLst->GetBroadcaster() )
            break;
        pLst = pLst->GetNext();
    }
    return 0 != pLst;
}

//--------------------------------------------------------------------

// base implementation of notification handler

void SvtListener::Notify( SvtBroadcaster&
#ifdef DBG_UTIL
rBC
#endif
, const SfxHint& )
{
    DBG_ASSERT( IsListening( rBC ),
                "notification from unregistered broadcaster" );
}


