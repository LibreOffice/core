/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listeneriter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:19:30 $
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

#include "listenerbase.hxx"
#include "listeneriter.hxx"
#include "broadcast.hxx"
#include "listener.hxx"

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

