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

#include <svl/listener.hxx>
#include <svl/listeneriter.hxx>
#include <svl/broadcast.hxx>
#include <svl/smplhint.hxx>


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
//      bInModify = sal_True;

        SvtListenerIter aIter( rBC );
        SvtListener* pLast = aIter.GoStart();
        if( pLast )
            do {
                pLast->Notify( rBC, rHint );
                if( !rBC.HasListeners() )       // all gone ??
                    break;
            } while( 0 != ( pLast = aIter.GoNext() ));

//      bInModify = sal_False;
//      UnlockModify();
    }
}



