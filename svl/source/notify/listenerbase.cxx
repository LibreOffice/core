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
#include "listenerbase.hxx"
#include <svl/listeneriter.hxx>
#include <svl/listener.hxx>
#include <svl/broadcast.hxx>


SvtListenerBase::SvtListenerBase( SvtListener& rLst,
                                  SvtBroadcaster& rBroadcaster )
    : pLeft( 0 ), pRight( 0 ),
    pBroadcaster( &rBroadcaster ), pListener( &rLst )
{
    pNext = rLst.pBrdCastLst;
    rLst.pBrdCastLst = this;

    if( pBroadcaster->pRoot )
    {
        // set ever behind the root
        pRight = pBroadcaster->pRoot->pRight;
        pBroadcaster->pRoot->pRight = this;
        this->pLeft = pBroadcaster->pRoot;
        if( pRight )
            pRight->pLeft = this;
    }
    else
        pBroadcaster->pRoot = this;
}

SvtListenerBase::~SvtListenerBase()
{
    SvtListenerBase *pR = pRight, *pL = pLeft;
    if( pBroadcaster->pRoot )
        pBroadcaster->pRoot = pL ? pL : pR;

    if( pL )
        pL->pRight = pR;
    if( pR )
        pR->pLeft = pL;

    SvtListenerIter::RemoveListener( *this, pR );

    if( !pBroadcaster->pRoot )
        pBroadcaster->ListenersGone();
}


