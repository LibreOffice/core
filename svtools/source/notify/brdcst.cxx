/*************************************************************************
 *
 *  $RCSfile: brdcst.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "hint.hxx"
#include "smplhint.hxx"
#include "lstner.hxx"

SV_DECL_PTRARR( SfxListenerArr_Impl, SfxListener*, 0, 2 );

#define _SFX_BRDCST_CXX
#include "brdcst.hxx"

#include <segmentc.hxx>

SEG_EOFGLOBALS()

//====================================================================
DBG_NAME(SfxBroadcaster);
TYPEINIT0(SfxBroadcaster);

//====================================================================

//====================================================================
// broadcast immedeately

#pragma SEG_FUNCDEF(brdcst_01)

void SfxBroadcaster::Broadcast( const SfxHint &rHint )
{
    DBG_CHKTHIS(SfxBroadcaster, 0);

    // is anybody to notify?
    if ( aListeners.Count() /*! || aGlobListeners.Count() */ )
    {
        // determine the type only once, because of its expensiveness
        const TypeId& rBCType = Type();
        const TypeId& rHintType = rHint.Type();

        // notify all registered listeners exactly once
        for ( USHORT n = 0; n < aListeners.Count(); ++n )
        {
            SfxListener* pListener = aListeners[n];
            if ( pListener )
                pListener->SFX_NOTIFY( *this, rBCType, rHint, rHintType );
        }
    }
}

//--------------------------------------------------------------------

// broadcast after a timeout

#pragma SEG_FUNCDEF(brdcst_02)

void SfxBroadcaster::BroadcastDelayed( const SfxHint& rHint )
{
    DBG_WARNING( "not implemented" );
    Broadcast(rHint);
}
//--------------------------------------------------------------------

// broadcast in idle-handler

#pragma SEG_FUNCDEF(brdcst_03)

void SfxBroadcaster::BroadcastInIdle( const SfxHint& rHint )
{
    DBG_WARNING( "not implemented" );
    Broadcast(rHint);
}
//--------------------------------------------------------------------

// unregister all listeners

#pragma SEG_FUNCDEF(brdcst_04)

SfxBroadcaster::~SfxBroadcaster()
{
    DBG_DTOR(SfxBroadcaster, 0);

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    // remove all still registered listeners
    for ( USHORT nPos = 0; nPos < aListeners.Count(); ++nPos )
    {
        SfxListener *pListener = aListeners[nPos];
        if ( pListener )
            pListener->RemoveBroadcaster_Impl(*this);
    }
}

//--------------------------------------------------------------------

// simple ctor of class SfxBroadcaster

#pragma SEG_FUNCDEF(brdcst_05)

SfxBroadcaster::SfxBroadcaster()
{
    DBG_CTOR(SfxBroadcaster, 0);
}

//--------------------------------------------------------------------

// copy ctor of class SfxBroadcaster

#pragma SEG_FUNCDEF(brdcst_06)

SfxBroadcaster::SfxBroadcaster( const SfxBroadcaster &rBC )
{
    DBG_CTOR(SfxBroadcaster, 0);

    for ( USHORT n = 0; n < rBC.aListeners.Count(); ++n )
    {
        SfxListener *pListener = rBC.aListeners[n];
        if ( pListener )
            pListener->StartListening( *this );
    }
}

//--------------------------------------------------------------------

// add a new SfxListener to the list

#pragma SEG_FUNCDEF(brdcst_07)
#if SUPD>=501
BOOL
#else
void
#endif
     SfxBroadcaster::AddListener( SfxListener& rListener )
{
    DBG_CHKTHIS(SfxBroadcaster, 0);
    const SfxListener *pListener = &rListener;
    const SfxListener *pNull = 0;
    USHORT nFreePos = aListeners.GetPos( pNull );
    if ( nFreePos < aListeners.Count() )
        aListeners.GetData()[nFreePos] = pListener;
    else if ( aListeners.Count() < (USHRT_MAX-1) )
        aListeners.Insert( pListener, aListeners.Count() );
    else
    {
        DBG_ERROR( "array overflow" );
#if SUPD>=501
        return FALSE;
#endif
    }

    DBG_ASSERT( USHRT_MAX != aListeners.GetPos(pListener),
                "AddListener failed" );
#if SUPD>=501
    return TRUE;
#endif
}

//--------------------------------------------------------------------

// called, if no more listeners exists

#pragma SEG_FUNCDEF(brdcst_08)

void SfxBroadcaster::ListenersGone()
{
    DBG_CHKTHIS(SfxBroadcaster,0);
}

//--------------------------------------------------------------------

// forward a notification to all registered listeners

#pragma SEG_FUNCDEF(brdcst_09)

void SfxBroadcaster::SFX_FORWARD(SfxBroadcaster& rBC, const TypeId& rBCType,
                               const SfxHint& rHint, const TypeId& rHintType)
{
    const USHORT nCount = aListeners.Count();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        SfxListener *pListener = aListeners[i];
        if ( pListener )
            pListener->SFX_NOTIFY( rBC, rBCType, rHint, rHintType);
    }
}

//--------------------------------------------------------------------

// remove one SfxListener from the list

#pragma SEG_FUNCDEF(brdcst_0a)

void SfxBroadcaster::RemoveListener( SfxListener& rListener )
{
    {DBG_CHKTHIS(SfxBroadcaster, 0);}
    const SfxListener *pListener = &rListener;
    USHORT nPos = aListeners.GetPos(pListener);
    DBG_ASSERT( nPos != USHRT_MAX, "RemoveListener: Listener unknown" );
    aListeners.GetData()[nPos] = 0;
    if ( !HasListeners() )
        ListenersGone();
}

//--------------------------------------------------------------------

#pragma SEG_FUNCDEF(brdcst_0b)

BOOL SfxBroadcaster::HasListeners() const
{
    for ( USHORT n = 0; n < aListeners.Count(); ++n )
        if ( aListeners.GetObject(n) != 0 )
            return TRUE;
    return FALSE;
}

//--------------------------------------------------------------------

#pragma SEG_EOFMODULE

