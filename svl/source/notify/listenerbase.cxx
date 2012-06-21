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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
