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

#include <precomp.h>
#include <tokens/tkpstama.hxx>

// NOT FULLY DECLARED SERVICES
// #include <srcfind.hxx>
#include <tokens/stmstarr.hxx>
//#include <parseinc.hxx>
#include <tools/tkpchars.hxx>


const intt  C_nStatuslistResizeValue = 32;
const intt  C_nTopStatus = 0;

StateMachine::StateMachine( intt            in_nStatusSize,
                            intt            in_nInitial_StatusListSize )
    :   pStati(new StmStatus*[in_nInitial_StatusListSize]),
        nCurrentStatus(C_nTopStatus),
        nPeekedStatus(C_nTopStatus),
        nStatusSize(in_nStatusSize),
        nNrofStati(0),
        nStatiSpace(in_nInitial_StatusListSize)
{
    csv_assert(in_nStatusSize > 0);
    csv_assert(in_nInitial_StatusListSize > 0);

    memset(pStati, 0, sizeof(StmStatus*) * nStatiSpace);
}

intt
StateMachine::AddStatus(StmStatus * let_dpStatus)
{
    if (nNrofStati == nStatiSpace)
    {
        ResizeStati();
    }
    pStati[nNrofStati] = let_dpStatus;
    return nNrofStati++;
}

void
StateMachine::AddToken( const char *        in_sToken,
                        TextToken::F_CRTOK  in_fTokenCreateFunction,
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
        StmStatus & rPst = Status(nPeekedStatus);
        if ( rPst.IsADefault() OR rPst.AsBounds() != 0 )
        {
            nPeekedStatus = AddStatus( new StmArrayStatus(nStatusSize, in_aBranches, 0, false ) );
            CurrentStatus().SetBranch( *pChar, nPeekedStatus );
        }
        nCurrentStatus = nPeekedStatus;
    }   // end for
    StmArrayStatus & rLastStatus = CurrentStatus();
    rLastStatus.SetTokenCreateFunction(in_fTokenCreateFunction);
    for (intt i = 0; i < nStatusSize; i++)
    {
        if (Status(rLastStatus.NextBy(i)).AsBounds() != 0)
            rLastStatus.SetBranch(i,in_nBoundsStatus);
    }   // end for
}

StateMachine::~StateMachine()
{
    for (intt i = 0; i < nNrofStati; i++)
    {
        delete pStati[i];
    }
    delete [] pStati;
}

StmBoundsStatus &
StateMachine::GetCharChain( TextToken::F_CRTOK &    o_nTokenCreateFunction,
                            CharacterSource &       io_rText )
{
    nCurrentStatus = C_nTopStatus;

    Peek(io_rText.CurChar());
    while (BoundsStatus() == 0)
    {
        nCurrentStatus = nPeekedStatus;
        Peek(io_rText.MoveOn());
    }
    o_nTokenCreateFunction = CurrentStatus().TokenCreateFunction();

    return *BoundsStatus();
}

void
StateMachine::ResizeStati()
{
    intt nNewSize = nStatiSpace + C_nStatuslistResizeValue;
    intt i = 0;
    StatusList pNewStati = new StmStatus*[nNewSize];

    for ( ; i < nNrofStati; i++)
    {
        pNewStati[i] = pStati[i];
    }
    memset( pNewStati+i,
            0,
            (nNewSize-i) * sizeof(StmStatus*) );

    delete [] pStati;
    pStati = pNewStati;
    nStatiSpace = nNewSize;
}

StmStatus &
StateMachine::Status(intt in_nStatusNr) const
{
    csv_assert( csv::in_range(intt(0), in_nStatusNr, intt(nNrofStati)) );
    return *pStati[in_nStatusNr];
}

StmArrayStatus &
StateMachine::CurrentStatus() const
{
    StmArrayStatus * pCurSt = Status(nCurrentStatus).AsArray();

    csv_assert(pCurSt != 0);
//  if(pCurSt == 0)
//      csv_assert(false);
    return *pCurSt;
}

StmBoundsStatus *
StateMachine::BoundsStatus() const
{
    return Status(nPeekedStatus).AsBounds();
}

void
StateMachine::Peek(intt in_nBranch)
{
    StmArrayStatus & rSt = CurrentStatus();
    nPeekedStatus = rSt.NextBy(in_nBranch);
}
