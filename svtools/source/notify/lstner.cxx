/*************************************************************************
 *
 *  $RCSfile: lstner.cxx,v $
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

#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "hint.hxx"
#include "brdcst.hxx"

SV_DECL_PTRARR( SfxBroadcasterArr_Impl, SfxBroadcaster*, 0, 2 );

#define _SFX_LSTNER_CXX
#include "lstner.hxx"

//====================================================================
DBG_NAME(SfxListener);
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

    for ( USHORT n = 0; n < rListener.aBCs.Count(); ++n )
        StartListening( *rListener.aBCs[n] );
}
//--------------------------------------------------------------------

// unregisteres the SfxListener from its SfxBroadcasters

SfxListener::~SfxListener()
{
    DBG_DTOR(SfxListener, 0);

    // unregister at all remainding broadcasters
    for ( USHORT nPos = 0; nPos < aBCs.Count(); ++nPos )
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

BOOL SfxListener::StartListening( SfxBroadcaster& rBroadcaster, BOOL bPreventDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !bPreventDups || !IsListening( rBroadcaster ) )
    {
#if SUPD>=501
        if ( rBroadcaster.AddListener(*this) )
#else
        rBroadcaster.AddListener(*this);
#endif
        {
            const SfxBroadcaster *pBC = &rBroadcaster;
            aBCs.Insert( pBC, aBCs.Count() );

            DBG_ASSERT( IsListening(rBroadcaster), "StartListening failed" );
            return TRUE;
        }

    }
    return FALSE;
}

//--------------------------------------------------------------------

// unregisteres at a specific SfxBroadcaster

BOOL SfxListener::EndListening( SfxBroadcaster& rBroadcaster, BOOL bAllDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !IsListening( rBroadcaster ) )
        return FALSE;

    do
    {
        rBroadcaster.RemoveListener(*this);
        const SfxBroadcaster *pBC = &rBroadcaster;
        aBCs.Remove( aBCs.GetPos(pBC), 1 );
    }
    while ( bAllDups && IsListening( rBroadcaster ) );
    return TRUE;
}

//--------------------------------------------------------------------

// unregisteres at a specific SfxBroadcaster by index

void SfxListener::EndListening( USHORT nNo )
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

BOOL SfxListener::IsListening( SfxBroadcaster& rBroadcaster ) const
{
    const SfxBroadcaster *pBC = &rBroadcaster;
    return USHRT_MAX != aBCs.GetPos( pBC );
}

//--------------------------------------------------------------------

// base implementation of notification handler

void SfxListener::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                              const SfxHint& rHint, const TypeId& rHintType )
{
    const SfxBroadcaster *pBC = &rBC;
    DBG_ASSERT( USHRT_MAX != aBCs.GetPos(pBC),
                "notification from unregistered broadcaster" );
}

