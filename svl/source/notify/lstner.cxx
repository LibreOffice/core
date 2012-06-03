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


#include <tools/debug.hxx>

#include <svl/hint.hxx>
#include <svl/brdcst.hxx>

#include <svl/lstner.hxx>
#include <algorithm>

//====================================================================
DBG_NAME(SfxListener)
TYPEINIT0(SfxListener);

//====================================================================
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

    for ( sal_uInt16 n = 0; n < rListener.aBCs.size(); ++n )
        StartListening( *rListener.aBCs[n] );
}
//--------------------------------------------------------------------

// unregisters the SfxListener from its SfxBroadcasters

SfxListener::~SfxListener()
{
    DBG_DTOR(SfxListener, 0);

    // unregister at all remaining broadcasters
    for ( sal_uInt16 nPos = 0; nPos < aBCs.size(); ++nPos )
    {
        SfxBroadcaster *pBC = aBCs[nPos];
        pBC->RemoveListener(*this);
    }
}

//--------------------------------------------------------------------

// unregisters a specific SfxBroadcaster

void SfxListener::RemoveBroadcaster_Impl( SfxBroadcaster& rBroadcaster )
{
    DBG_CHKTHIS(SfxListener, 0);

    aBCs.erase( std::find( aBCs.begin(), aBCs.end(), &rBroadcaster ) );
}

//--------------------------------------------------------------------

// registers a specific SfxBroadcaster

sal_Bool SfxListener::StartListening( SfxBroadcaster& rBroadcaster, sal_Bool bPreventDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !bPreventDups || !IsListening( rBroadcaster ) )
    {
        if ( rBroadcaster.AddListener(*this) )
        {
            aBCs.push_back( &rBroadcaster );

            DBG_ASSERT( IsListening(rBroadcaster), "StartListening failed" );
            return sal_True;
        }

    }
    return sal_False;
}

//--------------------------------------------------------------------

// unregisters a specific SfxBroadcaster

sal_Bool SfxListener::EndListening( SfxBroadcaster& rBroadcaster, sal_Bool bAllDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !IsListening( rBroadcaster ) )
        return sal_False;

    do
    {
        rBroadcaster.RemoveListener(*this);
        aBCs.erase( std::find( aBCs.begin(), aBCs.end(), &rBroadcaster ) );
    }
    while ( bAllDups && IsListening( rBroadcaster ) );
    return sal_True;
}

//--------------------------------------------------------------------

// unregisters all Broadcasters

void SfxListener::EndListeningAll()
{
    DBG_CHKTHIS(SfxListener, 0);

    // MI: bei Optimierung beachten: Seiteneffekte von RemoveListener beachten!
    while ( !aBCs.empty() )
    {
        SfxBroadcaster *pBC = aBCs.front();
        pBC->RemoveListener(*this);
        aBCs.pop_front();
    }
}

//--------------------------------------------------------------------

sal_Bool SfxListener::IsListening( SfxBroadcaster& rBroadcaster ) const
{
    return aBCs.end() != std::find( aBCs.begin(), aBCs.end(), &rBroadcaster );
}

//--------------------------------------------------------------------

// base implementation of notification handler

#ifdef DBG_UTIL
void SfxListener::Notify( SfxBroadcaster& rBroadcaster, const SfxHint& )
#else
void SfxListener::Notify( SfxBroadcaster&, const SfxHint& )
#endif
{
    #ifdef DBG_UTIL
    DBG_ASSERT(aBCs.end() != std::find(aBCs.begin(), aBCs.end(), &rBroadcaster),
                "notification from unregistered broadcaster" );
    #endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
