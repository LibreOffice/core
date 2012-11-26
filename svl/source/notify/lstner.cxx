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

#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <svl/hint.hxx>
#include <svl/brdcst.hxx>

SV_DECL_PTRARR( SfxBroadcasterArr_Impl, SfxBroadcaster*, 0, 2 )

#define _SFX_LSTNER_CXX
#include <svl/lstner.hxx>

//====================================================================
DBG_NAME(SfxListener)
// simple ctor of class SfxListener

SfxListener::SfxListener()
{
    DBG_CTOR(SfxListener, 0);
}
//--------------------------------------------------------------------

// copy ctor of class SfxListener

SfxListener::SfxListener( const SfxListener &rListener )
{
    DBG_CTOR(SfxListener, 0);

    for ( sal_uInt16 n = 0; n < rListener.aBCs.Count(); ++n )
        StartListening( *rListener.aBCs[n] );
}
//--------------------------------------------------------------------

// unregisteres the SfxListener from its SfxBroadcasters

SfxListener::~SfxListener()
{
    DBG_DTOR(SfxListener, 0);

    // unregister at all remainding broadcasters
    for ( sal_uInt16 nPos = 0; nPos < aBCs.Count(); ++nPos )
    {
        SfxBroadcaster *pBC = aBCs[nPos];
        pBC->RemoveListener(*this);
    }
}

//--------------------------------------------------------------------

// unregisteres at a specific SfxBroadcaster

void SfxListener::RemoveBroadcaster_Impl( SfxBroadcaster& rBC )
{
    DBG_CHKTHIS(SfxListener, 0);

    const SfxBroadcaster *pBC = &rBC;
    aBCs.Remove( aBCs.GetPos(pBC), 1 );
}

//--------------------------------------------------------------------

// registeres at a specific SfxBroadcaster

sal_Bool SfxListener::StartListening( SfxBroadcaster& rBroadcaster, sal_Bool bPreventDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !bPreventDups || !IsListening( rBroadcaster ) )
    {
        if ( rBroadcaster.AddListener(*this) )
        {
            const SfxBroadcaster *pBC = &rBroadcaster;
            aBCs.Insert( pBC, aBCs.Count() );

            DBG_ASSERT( IsListening(rBroadcaster), "StartListening failed" );
            return sal_True;
        }

    }
    return sal_False;
}

//--------------------------------------------------------------------

// unregisteres at a specific SfxBroadcaster

sal_Bool SfxListener::EndListening( SfxBroadcaster& rBroadcaster, sal_Bool bAllDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !IsListening( rBroadcaster ) )
        return sal_False;

    do
    {
        rBroadcaster.RemoveListener(*this);
        const SfxBroadcaster *pBC = &rBroadcaster;
        aBCs.Remove( aBCs.GetPos(pBC), 1 );
    }
    while ( bAllDups && IsListening( rBroadcaster ) );
    return sal_True;
}

//--------------------------------------------------------------------

// unregisteres at a specific SfxBroadcaster by index

void SfxListener::EndListening( sal_uInt16 nNo )
{
    DBG_CHKTHIS(SfxListener, 0);

    SfxBroadcaster *pBC = aBCs.GetObject(nNo);
    pBC->RemoveListener(*this);
    aBCs.Remove( nNo, 1 );
}

//--------------------------------------------------------------------

// unregisteres all Broadcasters

void SfxListener::EndListeningAll()
{
    DBG_CHKTHIS(SfxListener, 0);

    // MI: bei Optimierung beachten: Seiteneffekte von RemoveListener beachten!
    while ( aBCs.Count() )
    {
        SfxBroadcaster *pBC = aBCs.GetObject(0);
        pBC->RemoveListener(*this);
        aBCs.Remove( 0, 1 );
    }
}

//--------------------------------------------------------------------

sal_Bool SfxListener::IsListening( SfxBroadcaster& rBroadcaster ) const
{
    const SfxBroadcaster *pBC = &rBroadcaster;
    return USHRT_MAX != aBCs.GetPos( pBC );
}

//--------------------------------------------------------------------

// base implementation of notification handler

#ifdef DBG_UTIL
void SfxListener::Notify( SfxBroadcaster& rBC, const SfxHint& )
#else
void SfxListener::Notify( SfxBroadcaster&, const SfxHint& )
#endif
{
    #ifdef DBG_UTIL
    const SfxBroadcaster *pBC = &rBC;
    DBG_ASSERT( USHRT_MAX != aBCs.GetPos(pBC),
                "notification from unregistered broadcaster" );
    #endif
}

