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
#include "precompiled_svl.hxx"
#include <tools/debug.hxx>

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


SvtListener* SvtListenerIter::GoPrev()
{
    if( pDelNext == pAkt )
        pAkt = pAkt->GetLeft();
    else
        pAkt = pDelNext->GetLeft();
    pDelNext = pAkt;
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


SvtListener* SvtListenerIter::GoEnd()           // zum End des Baums
{
    pAkt = pDelNext;
    if( !pAkt )
        pAkt = rRoot.pRoot;
    if( pAkt )
        while( pAkt->GetRight() )
            pAkt = pAkt->GetRight();
    pDelNext = pAkt;
    return pAkt ? pAkt->GetListener() : 0;
}



SvtListener* SvtListenerIter::First( TypeId nType )
{
    aSrchId = nType;
    GoStart();
    if( pAkt )
        do {
            if( pAkt->GetListener()->IsA( aSrchId ) )
                break;

            if( pDelNext == pAkt )
            {
                pAkt = pAkt->GetRight();
                pDelNext = pAkt;
            }
            else
                pAkt = pDelNext;

        } while( pAkt );
    return pAkt ? pAkt->GetListener() : 0;
}


SvtListener* SvtListenerIter::Next()
{
    do {
        // erstmal zum naechsten
        if( pDelNext == pAkt )
        {
            pAkt = pAkt->GetRight();
            pDelNext = pAkt;
        }
        else
            pAkt = pDelNext;

        if( pAkt && pAkt->GetListener()->IsA( aSrchId ) )
            break;
    } while( pAkt );
    return pAkt ? pAkt->GetListener() : 0;
}


SvtListener* SvtListenerIter::GoRoot()      // wieder ab Root anfangen
{
    pDelNext = pAkt = rRoot.pRoot;
    return pAkt ? pAkt->GetListener() : 0;
}

SvtListener* SvtListenerIter::GetCurr() const   // returns the current
{
    return pDelNext ? pDelNext->GetListener() : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
