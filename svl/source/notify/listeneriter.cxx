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
#include <svl/broadcast.hxx>
#include <svl/listener.hxx>

SvtListenerIter* SvtListenerIter::pListenerIters = 0;

SvtListenerIter::SvtListenerIter( SvtBroadcaster& rBrdcst )
    : rRoot( rBrdcst )
{
    // hinten einketten!
    pNxtIter = 0;
    if( pListenerIters )
    {
        SvtListenerIter* pTmp = pListenerIters;
        while( pTmp->pNxtIter )
            pTmp = pTmp->pNxtIter;
        pTmp->pNxtIter = this;
    }
    else
        pListenerIters = this;

    pAkt = rRoot.pRoot;
    pDelNext = pAkt;
}



SvtListenerIter::~SvtListenerIter()
{
    if( pListenerIters )
    {
        if( pListenerIters == this )
            pListenerIters = pNxtIter;
        else
        {
            SvtListenerIter* pTmp = pListenerIters;
            while( pTmp->pNxtIter != this )
                if( 0 == ( pTmp = pTmp->pNxtIter ) )
                    return ;
            pTmp->pNxtIter = pNxtIter;
        }
    }
}

void SvtListenerIter::RemoveListener( SvtListenerBase& rDel,
                                        SvtListenerBase* pNext )
{
    // Update the ListenerIter
    SvtListenerIter* pTmp = pListenerIters;
    while( pTmp )
    {
        if( pTmp->pAkt == &rDel || pTmp->pDelNext == &rDel )
            pTmp->pDelNext = pNext;
        pTmp = pTmp->pNxtIter;
    }
}

SvtListener* SvtListenerIter::GoNext()
{
    if( pDelNext == pAkt )
    {
        pAkt = pAkt->GetRight();
        pDelNext = pAkt;
    }
    else
        pAkt = pDelNext;
    return pAkt ? pAkt->GetListener() : 0;
}

SvtListener* SvtListenerIter::GoStart()         // zum Anfang des Baums
{
    pAkt = rRoot.pRoot;
    if( pAkt )
        while( pAkt->GetLeft() )
            pAkt = pAkt->GetLeft();
    pDelNext = pAkt;
    return pAkt ? pAkt->GetListener() : 0;
}

SvtListener* SvtListenerIter::GetCurr() const   // returns the current
{
    return pDelNext ? pDelNext->GetListener() : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
