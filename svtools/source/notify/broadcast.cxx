/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: broadcast.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:18:07 $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "listener.hxx"
#include "listeneriter.hxx"
#include "broadcast.hxx"
#include "smplhint.hxx"


//====================================================================
TYPEINIT0(SvtBroadcaster);

//====================================================================

// simple ctor of class SvtBroadcaster

SvtBroadcaster::SvtBroadcaster()
    : pRoot( 0 )
{
}

//--------------------------------------------------------------------

// copy ctor of class SvtBroadcaster

SvtBroadcaster::SvtBroadcaster( const SvtBroadcaster &rBC )
    : pRoot( 0 )
{
    SvtListenerIter aIter( (SvtBroadcaster&)rBC );
    SvtListener* pLast = aIter.GoStart();
    if( pLast )
        do {
            pLast->StartListening( *this );
        } while( 0 != ( pLast = aIter.GoNext() ));
}

//--------------------------------------------------------------------

// unregister all listeners

SvtBroadcaster::~SvtBroadcaster()
{
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    SvtListenerIter aIter( *this );
    SvtListener* pLast = aIter.GoStart();
    if( pLast )
        do {
            pLast->EndListening( *this );
            if( !HasListeners() )       // all gone ??
                break;
        } while( 0 != ( pLast = aIter.GoNext() ));
}

//--------------------------------------------------------------------

// broadcast immedeately

void SvtBroadcaster::Broadcast( const SfxHint &rHint )
{
    // is anybody to notify?
    if( HasListeners() /* && !IsModifyLocked()*/ )
    {
//      LockModify();
//      bInModify = TRUE;

        SvtListenerIter aIter( *this );
        SvtListener* pLast = aIter.GoStart();
        if( pLast )
            do {
                pLast->Notify( *this, rHint );
                if( !HasListeners() )       // all gone ??
                    break;
            } while( 0 != ( pLast = aIter.GoNext() ));

//      bInModify = FALSE;
//      UnlockModify();
    }
}

//--------------------------------------------------------------------


// called, if no more listeners exists

void SvtBroadcaster::ListenersGone()
{
}

//--------------------------------------------------------------------

// forward a notification to all registered listeners

void SvtBroadcaster::Forward( SvtBroadcaster& rBC, const SfxHint& rHint )
{
    // is anybody to notify?
    if( rBC.HasListeners() /* && !IsModifyLocked()*/ )
    {
//      LockModify();
//      bInModify = TRUE;

        SvtListenerIter aIter( rBC );
        SvtListener* pLast = aIter.GoStart();
        if( pLast )
            do {
                pLast->Notify( rBC, rHint );
                if( !rBC.HasListeners() )       // all gone ??
                    break;
            } while( 0 != ( pLast = aIter.GoNext() ));

//      bInModify = FALSE;
//      UnlockModify();
    }
}



