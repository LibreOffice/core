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

#include <tools/debug.hxx>
#include <svl/broadcast.hxx>
#include <svl/listener.hxx>
#include "listenerbase.hxx"
#include <svl/listeneriter.hxx>


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

sal_Bool SvtListener::StartListening( SvtBroadcaster& rBroadcaster )
{
    const SvtListenerBase* pLst = pBrdCastLst;
    while( pLst )
    {
        if( &rBroadcaster == pLst->GetBroadcaster() )
        {
            // double, than return
            return sal_False;
        }
        pLst = pLst->GetNext();
    }
    new SvtListenerBase( *this, rBroadcaster );
    return sal_True;
}

//--------------------------------------------------------------------

// unregisteres at a specific SvtBroadcaster

sal_Bool SvtListener::EndListening( SvtBroadcaster& rBroadcaster )
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
            return sal_True;
        }
        pPrev = pLst;
        pLst = pLst->GetNext();
    }
    return sal_False;
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

sal_Bool SvtListener::IsListening( SvtBroadcaster& rBroadcaster ) const
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

void SvtListener::Notify( SvtBroadcaster&
#ifdef DBG_UTIL
rBC
#endif
, const SfxHint& )
{
    DBG_ASSERT( IsListening( rBC ),
                "notification from unregistered broadcaster" );
}


