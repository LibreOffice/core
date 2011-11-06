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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>

#include "document.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "globstr.hrc"
#include "scextopt.hxx"
#include "progress.hxx"
#include "rangenam.hxx"
#include "editutil.hxx"

#include "excrecds.hxx"
#include "root.hxx"
#include "imp_op.hxx"
#include "excimp8.hxx"
#include "otlnbuff.hxx"
#include "xcl97rec.hxx"
#include "formel.hxx"
#include "xilink.hxx"
#include "xecontent.hxx"

// - ALLGEMEINE ----------------------------------------------------------

RootData::RootData( void )
{
    eDateiTyp = BiffX;
    pExtSheetBuff = NULL;
    pShrfmlaBuff = NULL;
    pExtNameBuff = NULL;
    pFmlaConverter = NULL;

    pAutoFilterBuffer = NULL;
    pPrintRanges = new _ScRangeListTabs;
    pPrintTitles = new _ScRangeListTabs;

    pTabId = NULL;
    pUserBViewList = NULL;

    pIR = NULL;
    pER = NULL;
}

RootData::~RootData()
{
    delete pExtSheetBuff;
    delete pShrfmlaBuff;
    delete pExtNameBuff;
    delete pAutoFilterBuffer;
    delete pPrintRanges;
    delete pPrintTitles;
}




XclImpOutlineBuffer::XclImpOutlineBuffer( SCSIZE nNewSize )
{
    DBG_ASSERT( nNewSize > 0, "-OutlineBuffer::Ctor: nNewSize == 0!" );

    nSize = nNewSize + 1;
    pLevel = new sal_uInt8[ nSize ];
    pOuted = new sal_Bool[ nSize ];
    pHidden = new sal_Bool[ nSize ];
    pOutlineArray = NULL;

    Reset();
}


XclImpOutlineBuffer::~XclImpOutlineBuffer()
{
    delete[] pLevel;
    delete[] pOuted;
    delete[] pHidden;
}


void XclImpOutlineBuffer::SetLevel( SCSIZE nIndex, sal_uInt8 nVal, sal_Bool bOuted, sal_Bool bHidden )
{
    if( nIndex < nSize )
    {
        pLevel[ nIndex ] = nVal;
        pOuted[ nIndex ] = bOuted;
        pHidden[ nIndex ] = bHidden;

        if( nIndex > nLast )
            nLast = nIndex;
        if( nVal > nMaxLevel )
            nMaxLevel = nVal;
    }
}


void XclImpOutlineBuffer::SetOutlineArray( ScOutlineArray* pOArray )
{
    pOutlineArray = pOArray;
}


// transtorm xcl-outline into SC-outline
void XclImpOutlineBuffer::MakeScOutline( void )
{
    if( !pOutlineArray || !HasOutline() )
        return;

    const sal_uInt16    nNumLev         = 8;
    sal_Bool            bPreOutedLevel  = sal_False;
    sal_uInt8           nCurrLevel      = 0;
    sal_Bool            bMakeHidden[ nNumLev ];
    sal_Bool            bMakeVisible[ nNumLev + 1 ];

    sal_uInt16 nLevel;
    for( nLevel = 0; nLevel < nNumLev; ++nLevel )
        bMakeHidden[ nLevel ] = sal_False;
    for( nLevel = 0; nLevel <= nNumLev; ++nLevel )
        bMakeVisible[ nLevel ] = sal_True;
    if( nLast < (nSize - 1) )
        nLast++;

    // search for hidden attributes at end of level, move them to begin
    if( bButtonNormal )
    {
        for( sal_uInt8 nWorkLevel = 1; nWorkLevel <= nMaxLevel; nWorkLevel++ )
        {
            sal_uInt16  nStartPos       = 0;
            sal_uInt8    nCurrLevel2 = 0;
            sal_uInt8   nPrevLevel  = 0;

            for( SCSIZE nC = 0 ; nC <= nLast ; nC++ )
            {
                nPrevLevel = nCurrLevel2;
                nCurrLevel2 = pLevel[ nC ];
                if( (nPrevLevel < nWorkLevel) && (nCurrLevel2 >= nWorkLevel) )
                    nStartPos = static_cast< sal_uInt16 >( nC );
                else if( (nPrevLevel >= nWorkLevel) && (nCurrLevel2 < nWorkLevel) )
                {
                    if( pOuted[ nC ] && pHidden[ nStartPos ] )
                    {
                        if( nStartPos )
                            pOuted[ nStartPos - 1 ] = sal_True;
                        else
                            bPreOutedLevel = sal_True;
                        pOuted[ nC ] = sal_False;
                    }
                }
            }
        }
    }
    else
        bPreOutedLevel = pHidden[ 0 ];

    // generate SC outlines
    sal_uInt16  nPrevC;
    sal_uInt16  nStart[ nNumLev ];
    sal_Bool    bDummy;
    sal_Bool    bPrevOuted  = bPreOutedLevel;
    sal_Bool    bCurrHidden = sal_False;
    sal_Bool    bPrevHidden = sal_False;

    for( SCSIZE nC = 0; nC <= nLast; nC++ )
    {
        sal_uInt8 nWorkLevel = pLevel[ nC ];

        nPrevC      = static_cast< sal_uInt16 >( nC ? nC - 1 : 0 );
        bPrevHidden = bCurrHidden;
        bCurrHidden = pHidden[ nC ];

        // open new levels
        while( nWorkLevel > nCurrLevel )
        {
            nCurrLevel++;
            bMakeHidden[ nCurrLevel ] = bPrevOuted;
            bMakeVisible[ nCurrLevel + 1 ] =
                bMakeVisible[ nCurrLevel ] && !bMakeHidden[ nCurrLevel ];
            nStart[ nCurrLevel ] = static_cast< sal_uInt16 >( nC );
        }
        // close levels
        while( nWorkLevel < nCurrLevel )
        {
            sal_Bool bLastLevel     = (nWorkLevel == (nCurrLevel - 1));
            sal_Bool bRealHidden    = (bMakeHidden[ nCurrLevel ] && bPrevHidden );
            sal_Bool bRealVisible   = (bMakeVisible[ nCurrLevel ] ||
                                    (!bCurrHidden && bLastLevel));

            pOutlineArray->Insert( nStart[ nCurrLevel ], nPrevC , bDummy,
                bRealHidden, bRealVisible );
            nCurrLevel--;
        }

        bPrevOuted = pOuted[ nC ];
    }
}


void XclImpOutlineBuffer::SetLevelRange( SCSIZE nF, SCSIZE nL, sal_uInt8 nVal,
                                    sal_Bool bOuted, sal_Bool bHidden )
{
    DBG_ASSERT( nF <= nL, "+OutlineBuffer::SetLevelRange(): Last < First!" );

    if( nL < nSize )
    {
        if( nL > nLast )
            nLast = nL;

        sal_uInt8*  pLevelCount;
        sal_uInt8*  pLast;
        sal_Bool*   pOutedCount;
        sal_Bool*   pHiddenCount;

        pLevelCount = &pLevel[ nF ];
        pLast = &pLevel[ nL ];
        pOutedCount = &pOuted[ nF ];
        pHiddenCount = &pHidden[ nF ];

        while( pLevelCount <= pLast )
        {
            *( pLevelCount++ ) = nVal;
            *( pOutedCount++ ) = bOuted;
            *( pHiddenCount++ ) = bHidden;
        }

        if( nVal > nMaxLevel )
            nMaxLevel = nVal;
    }
}


void XclImpOutlineBuffer::Reset( void )
{
    for( SCSIZE nC = 0 ; nC < nSize ; nC++  )
    {
        pLevel[ nC ] = 0;
        pOuted[ nC ] = pHidden[ nC ] = sal_False;
    }
    nLast = 0;
    nMaxLevel = 0;
}


//___________________________________________________________________


ExcScenarioCell::ExcScenarioCell( const sal_uInt16 nC, const sal_uInt16 nR ) : nCol( nC ), nRow( nR )
{
}


void ExcScenarioCell::SetValue( const String& r )
{
    aValue = r;
}




#define EXCSCAPPEND(EXCSCCELL)  (List::Insert(EXCSCCELL,LIST_APPEND))
#define EXCSCFIRST()            ((ExcScenarioCell*)List::First())
#define EXCSCNEXT()             ((ExcScenarioCell*)List::Next())


ExcScenario::ExcScenario( XclImpStream& rIn, const RootData& rR ) : nTab( rR.pIR->GetCurrScTab() )
{
    sal_uInt16          nCref;
    sal_uInt8           nName, nComment;

    rIn >> nCref;
    rIn >> nProtected;
    rIn.Ignore( 1 );                // Hide
    rIn >> nName >> nComment;
    rIn.Ignore( 1 );       // statt nUser!

    if( nName )
        pName = new String( rIn.ReadUniString( nName ) );
    else
    {
        pName = new String( RTL_CONSTASCII_USTRINGPARAM( "Scenery" ) );
        rIn.Ignore( 1 );
    }

    pUserName = new String( rIn.ReadUniString() );

    if( nComment )
        pComment = new String( rIn.ReadUniString() );
    else
        pComment = new String;

    sal_uInt16          n = nCref;
    sal_uInt16          nC, nR;
    while( n )
    {
        rIn >> nR >> nC;

        EXCSCAPPEND( new ExcScenarioCell( nC, nR ) );

        n--;
    }

    n = nCref;
    ExcScenarioCell*    p = EXCSCFIRST();
    while( p )
    {
        p->SetValue( rIn.ReadUniString() );

        p = EXCSCNEXT();
    }
}


ExcScenario::~ExcScenario()
{
    ExcScenarioCell*    p = EXCSCFIRST();

    while( p )
    {
        delete p;
        p = EXCSCNEXT();
    }

    if( pName )
        delete pName;
    if( pComment )
        delete pComment;
    if( pUserName )
        delete pUserName;
}


void ExcScenario::Apply( const XclImpRoot& rRoot, const sal_Bool bLast )
{
    ScDocument&         r = rRoot.GetDoc();
    ExcScenarioCell*    p = EXCSCFIRST();
    String              aSzenName( *pName );
    sal_uInt16              nNewTab = nTab + 1;

    if( !r.InsertTab( nNewTab, aSzenName ) )
        return;

    r.SetScenario( nNewTab, sal_True );
    // #112621# do not show scenario frames
    r.SetScenarioData( nNewTab, *pComment, COL_LIGHTGRAY, /*SC_SCENARIO_SHOWFRAME|*/SC_SCENARIO_COPYALL|(nProtected ? SC_SCENARIO_PROTECT : 0) );

    while( p )
    {
        sal_uInt16          nCol = p->nCol;
        sal_uInt16          nRow = p->nRow;
        String          aVal = p->GetValue();

        r.ApplyFlagsTab( nCol, nRow, nCol, nRow, nNewTab, SC_MF_SCENARIO );

        r.SetString( nCol, nRow, nNewTab, aVal );

        p = EXCSCNEXT();
    }

    if( bLast )
        r.SetActiveScenario( nNewTab, sal_True );

    // #111896# modify what the Active tab is set to if the new
    // scenario tab occurs before the active tab.
    ScExtDocSettings& rDocSett = rRoot.GetExtDocOptions().GetDocSettings();
    if( (static_cast< SCCOL >( nTab ) < rDocSett.mnDisplTab) && (rDocSett.mnDisplTab < MAXTAB) )
        ++rDocSett.mnDisplTab;
    rRoot.GetTabInfo().InsertScTab( nNewTab );
}




ExcScenarioList::~ExcScenarioList()
{
    ExcScenario*    p = _First();

    while( p )
    {
        delete p;
        p = _Next();
    }
}


void ExcScenarioList::Apply( const XclImpRoot& rRoot )
{
    ExcScenario*    p = _Last();
    sal_uInt16          n = ( sal_uInt16 ) Count();

    while( p )
    {
        n--;
        p->Apply( rRoot, ( sal_Bool ) ( n == nLastScenario ) );
        p = _Prev();
    }
}


