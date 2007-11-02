/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkpstam2.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:19:37 $
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

    // Durch existierende Stati durchhangeln:
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
    intt i = 0;
    StatusList pNewStati = new StmStatu2*[nNewSize];

    for ( ; i < nNrofStati; i++)
    {
        pNewStati[i] = pStati[i];
    }
    memset( pNewStati+i,
            0,
            (nNewSize-i) * sizeof(StmStatu2*) );

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

void
StateMachin2::PrintOut()
{
    const intt anzahl = nNrofStati;
    for (int i = 0; i < anzahl; i++)
    {
        Cout() << i << ':';
        StmArrayStatu2 * pArrSt = pStati[i]->AsArray();
        if (pArrSt != 0)
        {
            Cout() << Endl();
            for (int b = 0; b < 128; b++)
            {
                Cout().width(4);
                Cout() << pArrSt->NextBy(b);
                if (b%16 == 15)
                    Cout() << Endl();
            }
        }
        else if (pStati[i]->AsBounds() != 0)
        {
            Cout() << "Bounds ";
        }
        else
            Cout() << "Error! ";
        Cout() << (pStati[i]->IsADefault() ? "DEF" : "---")
             << Endl();
    }   // for
}

