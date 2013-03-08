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

#include <precomp.h>
#include <tokens/tkpstam2.hxx>

// NOT FULLY DECLARED SERVICES
#include <tokens/stmstar2.hxx>
#include <tools/tkpchars.hxx>


const intt  C_nStatuslistResizeValue = 32;
const intt  C_nTopStatus = 0;

StateMachin2::StateMachin2( intt            in_nStatusSize,
                            intt            in_nInitial_StatusListSize )
    :   pStati(new StmStatu2*[in_nInitial_StatusListSize]),
        nCurrentStatus(C_nTopStatus),
        nPeekedStatus(C_nTopStatus),
        nStatusSize(in_nStatusSize),
        nNrofStati(0),
        nStatiSpace(in_nInitial_StatusListSize)
{
    csv_assert(in_nStatusSize > 0);
    csv_assert(in_nInitial_StatusListSize > 0);

    memset(pStati, 0, sizeof(StmStatu2*) * nStatiSpace);
}

intt
StateMachin2::AddStatus(StmStatu2 * let_dpStatus)
{
    if (nNrofStati == nStatiSpace)
    {
        ResizeStati();
    }
    pStati[nNrofStati] = let_dpStatus;
    return nNrofStati++;
}

void
StateMachin2::AddToken( const char *        in_sToken,
                        UINT16              in_nTokenId,
                        const INT16 *       in_aBranches,
                        INT16               in_nBoundsStatus )
{
    if (csv::no_str(in_sToken))
        return;

    nCurrentStatus = 0;
    nPeekedStatus = 0;

    for ( const char * pChar = in_sToken;
          *pChar != NULCH;
          ++pChar )
    {
        Peek(*pChar);
        StmStatu2 & rPst = Status(nPeekedStatus);
        if ( rPst.IsADefault() OR rPst.AsBounds() != 0 )
        {
            nPeekedStatus = AddStatus( new StmArrayStatu2(nStatusSize, in_aBranches, 0, false ) );
            CurrentStatus().SetBranch( *pChar, nPeekedStatus );
        }
        nCurrentStatus = nPeekedStatus;
    }   // end for
    StmArrayStatu2 & rLastStatus = CurrentStatus();
    rLastStatus.SetTokenId(in_nTokenId);
    for (intt i = 0; i < nStatusSize; i++)
    {
        if (Status(rLastStatus.NextBy(i)).AsBounds() != 0)
            rLastStatus.SetBranch(i,in_nBoundsStatus);
    }   // end for
}

StateMachin2::~StateMachin2()
{
    for (intt i = 0; i < nNrofStati; i++)
    {
        delete pStati[i];
    }
    delete [] pStati;
}

StmBoundsStatu2 &
StateMachin2::GetCharChain( UINT16 &           o_nTokenId,
                            CharacterSource &  io_rText )
{
    nCurrentStatus = C_nTopStatus;
    Peek(io_rText.CurChar());
    while (BoundsStatus() == 0)
    {
        nCurrentStatus = nPeekedStatus;
        Peek(io_rText.MoveOn());
    }
    o_nTokenId = CurrentStatus().TokenId();

    return *BoundsStatus();
}

void
StateMachin2::ResizeStati()
{
    intt nNewSize = nStatiSpace + C_nStatuslistResizeValue;
    StatusList pNewStati = new StmStatu2*[nNewSize];

    memcpy( pNewStati, pStati, nNrofStati * sizeof(StmStatu2*) );
    memset( pNewStati+nNrofStati, 0, (nNewSize-nNrofStati) * sizeof(StmStatu2*) );

    delete [] pStati;
    pStati = pNewStati;
    nStatiSpace = nNewSize;
}

StmStatu2 &
StateMachin2::Status(intt in_nStatusNr) const
{
    csv_assert( csv::in_range(intt(0), in_nStatusNr, intt(nNrofStati)) );
    return *pStati[in_nStatusNr];
}

StmArrayStatu2 &
StateMachin2::CurrentStatus() const
{
    StmArrayStatu2 * pCurSt = Status(nCurrentStatus).AsArray();
    if (pCurSt == 0)
    {
        csv_assert(false);
    }
    return *pCurSt;
}

StmBoundsStatu2 *
StateMachin2::BoundsStatus() const
{
    return Status(nPeekedStatus).AsBounds();
}

void
StateMachin2::Peek(intt in_nBranch)
{
    StmArrayStatu2 & rSt = CurrentStatus();
    nPeekedStatus = rSt.NextBy(in_nBranch);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
