/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: exctools.cxx,v $
 *
 *  $Revision: 1.56 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 11:42:36 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>

#include "document.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "globstr.hrc"
#include "scextopt.hxx"
#include "progress.hxx"
#include "rangenam.hxx"
#include "editutil.hxx"

#include "root.hxx"
#include "imp_op.hxx"
#include "excimp8.hxx"
#include "otlnbuff.hxx"
#include "excrecds.hxx"
#include "xcl97rec.hxx"
#include "formel.hxx"

#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif

// - ALLGEMEINE ----------------------------------------------------------

RootData::RootData( void )
{
    eDateiTyp = BiffX;
    pExtSheetBuff = NULL;
    pTabNameBuff = NULL;
    pShrfmlaBuff = NULL;
    pExtNameBuff = NULL;
    pFmlaConverter = NULL;

    pAutoFilterBuffer = NULL;
    pPrintRanges = new _ScRangeListTabs;
    pPrintTitles = new _ScRangeListTabs;

    pTabId = NULL;
    pUserBViewList = NULL;

    pObjRecs = NULL;
    pEscher = NULL;

    pIR = NULL;
    pER = NULL;
}

RootData::~RootData()
{
    delete pExtSheetBuff;
    delete pTabNameBuff;
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
    pLevel = new BYTE[ nSize ];
    pOuted = new BOOL[ nSize ];
    pHidden = new BOOL[ nSize ];
    pOutlineArray = NULL;

    Reset();
}


XclImpOutlineBuffer::~XclImpOutlineBuffer()
{
    delete[] pLevel;
    delete[] pOuted;
    delete[] pHidden;
}


void XclImpOutlineBuffer::SetLevel( SCSIZE nIndex, BYTE nVal, BOOL bOuted, BOOL bHidden )
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

    const UINT16    nNumLev         = 8;
    BOOL            bPreOutedLevel  = FALSE;
    BYTE            nCurrLevel      = 0;
    BOOL            bMakeHidden[ nNumLev ];
    BOOL            bMakeVisible[ nNumLev + 1 ];

    sal_uInt16 nLevel;
    for( nLevel = 0; nLevel < nNumLev; ++nLevel )
        bMakeHidden[ nLevel ] = FALSE;
    for( nLevel = 0; nLevel <= nNumLev; ++nLevel )
        bMakeVisible[ nLevel ] = TRUE;
    if( nLast < (nSize - 1) )
        nLast++;

    // search for hidden attributes at end of level, move them to begin
    if( bButtonNormal )
    {
        for( BYTE nWorkLevel = 1; nWorkLevel <= nMaxLevel; nWorkLevel++ )
        {
            UINT16  nStartPos       = 0;
            BYTE    nCurrLevel  = 0;
            BYTE    nPrevLevel  = 0;

            for( SCSIZE nC = 0 ; nC <= nLast ; nC++ )
            {
                nPrevLevel = nCurrLevel;
                nCurrLevel = pLevel[ nC ];
                if( (nPrevLevel < nWorkLevel) && (nCurrLevel >= nWorkLevel) )
                    nStartPos = static_cast< sal_uInt16 >( nC );
                else if( (nPrevLevel >= nWorkLevel) && (nCurrLevel < nWorkLevel) )
                {
                    if( pOuted[ nC ] && pHidden[ nStartPos ] )
                    {
                        if( nStartPos )
                            pOuted[ nStartPos - 1 ] = TRUE;
                        else
                            bPreOutedLevel = TRUE;
                        pOuted[ nC ] = FALSE;
                    }
                }
            }
        }
    }
    else
        bPreOutedLevel = pHidden[ 0 ];

    // generate SC outlines
    UINT16  nPrevC;
    UINT16  nStart[ nNumLev ];
    BOOL    bDummy;
    BOOL    bPrevOuted  = bPreOutedLevel;
    BOOL    bCurrHidden = FALSE;
    BOOL    bPrevHidden = FALSE;

    for( SCSIZE nC = 0; nC <= nLast; nC++ )
    {
        BYTE nWorkLevel = pLevel[ nC ];

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
            BOOL bLastLevel     = (nWorkLevel == (nCurrLevel - 1));
            BOOL bRealHidden    = (bMakeHidden[ nCurrLevel ] && bPrevHidden );
            BOOL bRealVisible   = (bMakeVisible[ nCurrLevel ] ||
                                    (!bCurrHidden && bLastLevel));

            pOutlineArray->Insert( nStart[ nCurrLevel ], nPrevC , bDummy,
                bRealHidden, bRealVisible );
            nCurrLevel--;
        }

        bPrevOuted = pOuted[ nC ];
    }
}


void XclImpOutlineBuffer::SetLevelRange( SCSIZE nF, SCSIZE nL, BYTE nVal,
                                    BOOL bOuted, BOOL bHidden )
{
    DBG_ASSERT( nF <= nL, "+OutlineBuffer::SetLevelRange(): Last < First!" );

    if( nL < nSize )
    {
        if( nL > nLast )
            nLast = nL;

        BYTE*   pLevelCount;
        BYTE*   pLast;
        BOOL*   pOutedCount;
        BOOL*   pHiddenCount;

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
        pOuted[ nC ] = pHidden[ nC ] = FALSE;
    }
    nLast = 0;
    nMaxLevel = 0;
}


void XclImpOutlineBuffer::Reset( ScOutlineArray *pOArray )
{
    for( SCSIZE nC = 0 ; nC < nSize ; nC++  )
    {
        pLevel[ nC ] = 0;
        pOuted[ nC ] = pHidden[ nC ] = FALSE;
    }
    pOutlineArray = pOArray;
}




//___________________________________________________________________


ExcScenarioCell::ExcScenarioCell( const UINT16 nC, const UINT16 nR ) : nCol( nC ), nRow( nR )
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
    UINT16          nCref;
    UINT8           nName, nComment;

    rIn >> nCref;
    rIn >> nProtected;
    rIn.Ignore( 1 );                // Hide
    rIn >> nName >> nComment;
    rIn.Ignore( 1 );        // statt nUser!

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

    UINT16          n = nCref;
    UINT16          nC, nR;
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


void ExcScenario::Apply( const XclImpRoot& rRoot, const BOOL bLast )
{
    UINT32              nNumCells = List::Count();

    ScDocument&         r = rRoot.GetDoc();
    ExcScenarioCell*    p = EXCSCFIRST();
    String              aSzenName( *pName );
    ScfTools::ConvertToScSheetName( aSzenName );
    UINT16              nNewTab = nTab + 1;

    if( !r.InsertTab( nNewTab, aSzenName ) )
        return;

    r.SetScenario( nNewTab, TRUE );
    // #112621# do not show scenario frames
    r.SetScenarioData( nNewTab, *pComment, COL_LIGHTGRAY, /*SC_SCENARIO_SHOWFRAME|*/SC_SCENARIO_COPYALL|(nProtected ? SC_SCENARIO_PROTECT : 0) );

    while( p )
    {
        UINT16          nCol = p->nCol;
        UINT16          nRow = p->nRow;
        String          aVal = p->GetValue();

        r.ApplyFlagsTab( nCol, nRow, nCol, nRow, nNewTab, SC_MF_SCENARIO );

        r.SetString( nCol, nRow, nNewTab, aVal );

        p = EXCSCNEXT();
    }

    if( bLast )
        r.SetActiveScenario( nNewTab, TRUE );

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
    UINT16          n = ( UINT16 ) Count();

    while( p )
    {
        n--;
        p->Apply( rRoot, ( BOOL ) ( n == nLastScenario ) );
        p = _Prev();
    }
}


