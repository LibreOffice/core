/*************************************************************************
 *
 *  $RCSfile: listener.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-14 18:52:07 $
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

void SvtListener::Notify( SvtBroadcaster& rBC, const SfxHint& rHint )
{
    DBG_ASSERT( IsListening( rBC ),
                "notification from unregistered broadcaster" );
}


