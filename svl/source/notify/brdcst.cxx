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
#include "precompiled_svl.hxx"

#ifndef GCC
#endif
#include <tools/debug.hxx>

#include <svl/hint.hxx>
#include <svl/smplhint.hxx>
#include <svl/lstner.hxx>

SV_DECL_PTRARR( SfxListenerArr_Impl, SfxListener*, 0, 2 )

#define _SFX_BRDCST_CXX
#include <svl/brdcst.hxx>

//====================================================================
DBG_NAME(SfxBroadcaster)

//====================================================================
// broadcast immediately


void SfxBroadcaster::Broadcast( const SfxHint &rHint )
{
    DBG_CHKTHIS(SfxBroadcaster, 0);

    // is anybody to notify?
    if ( aListeners.Count() /*! || aGlobListeners.Count() */ )
    {
        // notify all registered listeners exactly once
        for ( sal_uInt16 n = 0; n < aListeners.Count(); ++n )
        {
            SfxListener* pListener = aListeners[n];
            if ( pListener )
                pListener->Notify( *this, rHint );
        }
    }
}

//--------------------------------------------------------------------

// broadcast after a timeout


void SfxBroadcaster::BroadcastDelayed( const SfxHint& rHint )
{
    DBG_WARNING( "not implemented" );
    Broadcast(rHint);
}
//--------------------------------------------------------------------

// broadcast in idle-handler

void SfxBroadcaster::BroadcastInIdle( const SfxHint& rHint )
{
    DBG_WARNING( "not implemented" );
    Broadcast(rHint);
}
//--------------------------------------------------------------------

// unregister all listeners

SfxBroadcaster::~SfxBroadcaster()
{
    DBG_DTOR(SfxBroadcaster, 0);

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    // remove all still registered listeners
    for ( sal_uInt16 nPos = 0; nPos < aListeners.Count(); ++nPos )
    {
        SfxListener *pListener = aListeners[nPos];
        if ( pListener )
            pListener->RemoveBroadcaster_Impl(*this);
    }
}

//--------------------------------------------------------------------

// simple ctor of class SfxBroadcaster

SfxBroadcaster::SfxBroadcaster()
{
    DBG_CTOR(SfxBroadcaster, 0);
}

//--------------------------------------------------------------------

// copy ctor of class SfxBroadcaster


SfxBroadcaster::SfxBroadcaster( const SfxBroadcaster &rBC )
{
    DBG_CTOR(SfxBroadcaster, 0);

    for ( sal_uInt16 n = 0; n < rBC.aListeners.Count(); ++n )
    {
        SfxListener *pListener = rBC.aListeners[n];
        if ( pListener )
            pListener->StartListening( *this );
    }
}

//--------------------------------------------------------------------

// add a new SfxListener to the list

sal_Bool SfxBroadcaster::AddListener( SfxListener& rListener )
{
    DBG_CHKTHIS(SfxBroadcaster, 0);
    const SfxListener *pListener = &rListener;
    const SfxListener *pNull = 0;
    sal_uInt16 nFreePos = aListeners.GetPos( pNull );
    if ( nFreePos < aListeners.Count() )
        aListeners.GetData()[nFreePos] = pListener;
    else if ( aListeners.Count() < (USHRT_MAX-1) )
        aListeners.Insert( pListener, aListeners.Count() );
    else
    {
        DBG_ERROR( "array overflow" );
        return sal_False;
    }

    DBG_ASSERT( USHRT_MAX != aListeners.GetPos(pListener),
                "AddListener failed" );
    return sal_True;
}

//--------------------------------------------------------------------

// called, if no more listeners exists

void SfxBroadcaster::ListenersGone()
{
    DBG_CHKTHIS(SfxBroadcaster,0);
}

//--------------------------------------------------------------------

// forward a notification to all registered listeners

void SfxBroadcaster::Forward(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    const sal_uInt16 nCount = aListeners.Count();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        SfxListener *pListener = aListeners[i];
        if ( pListener )
            pListener->Notify( rBC, rHint );
    }
}

//--------------------------------------------------------------------

// remove one SfxListener from the list

void SfxBroadcaster::RemoveListener( SfxListener& rListener )
{
    {DBG_CHKTHIS(SfxBroadcaster, 0);}
    const SfxListener *pListener = &rListener;
    sal_uInt16 nPos = aListeners.GetPos(pListener);
    DBG_ASSERT( nPos != USHRT_MAX, "RemoveListener: Listener unknown" );
    aListeners.GetData()[nPos] = 0;
    if ( !HasListeners() )
        ListenersGone();
}

//--------------------------------------------------------------------

sal_Bool SfxBroadcaster::HasListeners() const
{
    for ( sal_uInt16 n = 0; n < aListeners.Count(); ++n )
        if ( aListeners.GetObject(n) != 0 )
            return sal_True;
    return sal_False;
}

//--------------------------------------------------------------------
