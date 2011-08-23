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

#include "listener.hxx"
#include "listeneriter.hxx"
#include "broadcast.hxx"
#include <bf_svtools/smplhint.hxx>

namespace binfilter
{

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
            if( !HasListeners() )		// all gone ??
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
//		LockModify();
//		bInModify = TRUE;

        SvtListenerIter aIter( *this );
        SvtListener* pLast = aIter.GoStart();
        if( pLast )
            do {
                pLast->Notify( *this, rHint );
                if( !HasListeners() )		// all gone ??
                    break;
            } while( 0 != ( pLast = aIter.GoNext() ));

//		bInModify = FALSE;
//		UnlockModify();
    }
}

//--------------------------------------------------------------------


// called, if no more listeners exists

void SvtBroadcaster::ListenersGone()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
