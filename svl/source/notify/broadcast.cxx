/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svl/listener.hxx>
#include <svl/listeneriter.hxx>
#include <svl/broadcast.hxx>
#include <svl/smplhint.hxx>


TYPEINIT0(SvtBroadcaster);


// simple ctor of class SvtBroadcaster

SvtBroadcaster::SvtBroadcaster()
    : pRoot( 0 )
{
}


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


// broadcast immedeately

void SvtBroadcaster::Broadcast( const SfxHint &rHint )
{
    // is anybody to notify?
    if( HasListeners() /* && !IsModifyLocked()*/ )
    {
//      LockModify();
//      bInModify = sal_True;

        SvtListenerIter aIter( *this );
        SvtListener* pLast = aIter.GoStart();
        if( pLast )
            do {
                pLast->Notify( *this, rHint );
                if( !HasListeners() )       // all gone ??
                    break;
            } while( 0 != ( pLast = aIter.GoNext() ));

//      bInModify = sal_False;
//      UnlockModify();
    }
}



// called, if no more listeners exists

void SvtBroadcaster::ListenersGone()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
