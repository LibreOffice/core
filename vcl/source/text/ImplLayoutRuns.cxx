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

#include <ImplLayoutRuns.hxx>

void ImplLayoutRuns::AddPos( int nCharPos, bool bRTL )
{
    // check if charpos could extend current run
    int nIndex = maRuns.size();
    if( nIndex >= 2 )
    {
        int nRunPos0 = maRuns[ nIndex-2 ];
        int nRunPos1 = maRuns[ nIndex-1 ];
        if( ((nCharPos + int(bRTL)) == nRunPos1) && ((nRunPos0 > nRunPos1) == bRTL) )
        {
            // extend current run by new charpos
            maRuns[ nIndex-1 ] = nCharPos + int(!bRTL);
            return;
        }
        // ignore new charpos when it is in current run
        if( (nRunPos0 <= nCharPos) && (nCharPos < nRunPos1) )
            return;
        if( (nRunPos1 <= nCharPos) && (nCharPos < nRunPos0) )
            return;
    }

    // else append a new run consisting of the new charpos
    maRuns.push_back( nCharPos + (bRTL ? 1 : 0) );
    maRuns.push_back( nCharPos + (bRTL ? 0 : 1) );
}

void ImplLayoutRuns::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    if( nCharPos0 == nCharPos1 )
        return;

    // swap if needed
    if( bRTL == (nCharPos0 < nCharPos1) )
        std::swap( nCharPos0, nCharPos1 );

    if (maRuns.size() >= 2 && nCharPos0 == maRuns[maRuns.size() - 2] && nCharPos1 == maRuns[maRuns.size() - 1])
    {
        //this run is the same as the last
        return;
    }

    // append new run
    maRuns.push_back( nCharPos0 );
    maRuns.push_back( nCharPos1 );
}

bool ImplLayoutRuns::PosIsInRun( int nCharPos ) const
{
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
        return false;

    int nMinCharPos = maRuns[ mnRunIndex+0 ];
    int nEndCharPos = maRuns[ mnRunIndex+1 ];
    if( nMinCharPos > nEndCharPos ) // reversed in RTL case
        std::swap( nMinCharPos, nEndCharPos );

    if( nCharPos < nMinCharPos )
        return false;
    if( nCharPos >= nEndCharPos )
        return false;
    return true;
}

bool ImplLayoutRuns::PosIsInAnyRun( int nCharPos ) const
{
    bool bRet = false;
    int nRunIndex = mnRunIndex;

    ImplLayoutRuns *pThis = const_cast<ImplLayoutRuns*>(this);

    pThis->ResetPos();

    for (size_t i = 0; i < maRuns.size(); i+=2)
    {
        bRet = PosIsInRun( nCharPos );
        if( bRet )
            break;
        pThis->NextRun();
    }

    pThis->mnRunIndex = nRunIndex;
    return bRet;
}

bool ImplLayoutRuns::GetNextPos( int* nCharPos, bool* bRightToLeft )
{
    // negative nCharPos => reset to first run
    if( *nCharPos < 0 )
        mnRunIndex = 0;

    // return false when all runs completed
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
        return false;

    int nRunPos0 = maRuns[ mnRunIndex+0 ];
    int nRunPos1 = maRuns[ mnRunIndex+1 ];
    *bRightToLeft = (nRunPos0 > nRunPos1);

    if( *nCharPos < 0 )
    {
        // get first valid nCharPos in run
        *nCharPos = nRunPos0;
    }
    else
    {
        // advance to next nCharPos for LTR case
        if( !*bRightToLeft )
            ++(*nCharPos);

        // advance to next run if current run is completed
        if( *nCharPos == nRunPos1 )
        {
            if( (mnRunIndex += 2) >= static_cast<int>(maRuns.size()) )
                return false;
            nRunPos0 = maRuns[ mnRunIndex+0 ];
            nRunPos1 = maRuns[ mnRunIndex+1 ];
            *bRightToLeft = (nRunPos0 > nRunPos1);
            *nCharPos = nRunPos0;
        }
    }

    // advance to next nCharPos for RTL case
    if( *bRightToLeft )
        --(*nCharPos);

    return true;
}

bool ImplLayoutRuns::GetRun( int* nMinRunPos, int* nEndRunPos, bool* bRightToLeft ) const
{
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
        return false;

    int nRunPos0 = maRuns[ mnRunIndex+0 ];
    int nRunPos1 = maRuns[ mnRunIndex+1 ];
    *bRightToLeft = (nRunPos1 < nRunPos0) ;
    if( !*bRightToLeft )
    {
        *nMinRunPos = nRunPos0;
        *nEndRunPos = nRunPos1;
    }
    else
    {
        *nMinRunPos = nRunPos1;
        *nEndRunPos = nRunPos0;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
