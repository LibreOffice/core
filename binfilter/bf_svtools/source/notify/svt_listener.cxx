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

// MARKER(update_precomp.py): autogen include statement, do not remove

#include <tools/debug.hxx>

#include "broadcast.hxx"
#include "listener.hxx"
#include "listenerbase.hxx"
#include "listeneriter.hxx"

namespace binfilter
{

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

#ifdef DBG_UTIL

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

#endif

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


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
