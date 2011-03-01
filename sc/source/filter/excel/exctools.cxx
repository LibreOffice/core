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

#include <vector>

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


XclImpOutlineBuffer::XclImpOutlineBuffer( SCSIZE nNewSize ) :
    maLevels(0, nNewSize, 0),
    mpOutlineArray(NULL),
    mnEndPos(nNewSize),
    mnMaxLevel(0),
    mbButtonAfter(true)
{
}

XclImpOutlineBuffer::~XclImpOutlineBuffer()
{
}

void XclImpOutlineBuffer::SetLevel( SCSIZE nIndex, sal_uInt8 nVal, bool bCollapsed )
{
    maLevels.insert_back(nIndex, nIndex+1, nVal);
    if (nVal > mnMaxLevel)
        mnMaxLevel = nVal;
    if (bCollapsed)
        maCollapsedPosSet.insert(nIndex);
}

void XclImpOutlineBuffer::SetOutlineArray( ScOutlineArray* pOArray )
{
    mpOutlineArray = pOArray;
}

void XclImpOutlineBuffer::MakeScOutline()
{
    if (!mpOutlineArray)
        return;

    ::std::vector<SCSIZE> aOutlineStack;
    aOutlineStack.reserve(mnMaxLevel);
    OutlineLevels::const_iterator itr = maLevels.begin(), itrEnd = maLevels.end();
    for (; itr != itrEnd; ++itr)
    {
        SCSIZE nPos = itr->first;
        if (nPos >= mnEndPos)
        {
            // Don't go beyond the max allowed position.
            DBG_ASSERT(aOutlineStack.empty(), "XclImpOutlineBuffer::MakeScOutline: outline stack not empty but expected to be.");
            break;
        }
        sal_uInt8 nLevel = itr->second;
        sal_uInt8 nCurLevel = static_cast<sal_uInt8>(aOutlineStack.size());
        if (nLevel > nCurLevel)
        {
            for (sal_uInt8 i = 0; i < nLevel - nCurLevel; ++i)
                aOutlineStack.push_back(nPos);
        }
        else
        {
            DBG_ASSERT(nLevel < nCurLevel, "XclImpOutlineBuffer::MakeScOutline: unexpected level!");
            for (sal_uInt8 i = 0; i < nCurLevel - nLevel; ++i)
            {
                if (aOutlineStack.empty())
                {
                    // Something is wrong.
                    return;
                }
                SCSIZE nFirstPos = aOutlineStack.back();
                aOutlineStack.pop_back();
                bool bCollapsed = false;
                if (mbButtonAfter)
                    bCollapsed = maCollapsedPosSet.count(nPos) > 0;
                else if (nFirstPos > 0)
                    bCollapsed = maCollapsedPosSet.count(nFirstPos-1) > 0;

                BOOL bDummy;
                mpOutlineArray->Insert(nFirstPos, nPos-1, bDummy, bCollapsed);
            }
        }
    }
}

void XclImpOutlineBuffer::SetLevelRange( SCSIZE nF, SCSIZE nL, sal_uInt8 nVal, bool bCollapsed )
{
    if (nF > nL)
        // invalid range
        return;

    maLevels.insert_back(nF, nL+1, nVal);

    if (bCollapsed)
        maCollapsedPosSet.insert(nF);
}

void XclImpOutlineBuffer::SetButtonMode( bool bRightOrUnder )
{
    mbButtonAfter = bRightOrUnder;
}

ExcScenarioCell::ExcScenarioCell( const UINT16 nC, const UINT16 nR )
    : nCol( nC ), nRow( nR )
{
}

ExcScenario::ExcScenario( XclImpStream& rIn, const RootData& rR )
    : nTab( rR.pIR->GetCurrScTab() )
{
    UINT16          nCref;
    UINT8           nName, nComment;

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

    UINT16          n = nCref;
    UINT16          nC, nR;
    while( n )
    {
        rIn >> nR >> nC;

        aEntries.push_back(new ExcScenarioCell( nC, nR ));

        n--;
    }

    n = nCref;

    boost::ptr_vector<ExcScenarioCell>::iterator iter;
    for (iter = aEntries.begin(); iter != aEntries.end(); ++iter)
        iter->SetValue(rIn.ReadUniString());
}

ExcScenario::~ExcScenario()
{
    if( pName )
        delete pName;
    if( pComment )
        delete pComment;
    if( pUserName )
        delete pUserName;
}

void ExcScenario::Apply( const XclImpRoot& rRoot, const BOOL bLast )
{
    ScDocument&         r = rRoot.GetDoc();
    String              aSzenName( *pName );
    UINT16              nNewTab = nTab + 1;

    if( !r.InsertTab( nNewTab, aSzenName ) )
        return;

    r.SetScenario( nNewTab, TRUE );
    // do not show scenario frames
    r.SetScenarioData( nNewTab, *pComment, COL_LIGHTGRAY, /*SC_SCENARIO_SHOWFRAME|*/SC_SCENARIO_COPYALL|(nProtected ? SC_SCENARIO_PROTECT : 0) );

    boost::ptr_vector<ExcScenarioCell>::const_iterator iter;
    for (iter = aEntries.begin(); iter != aEntries.end(); ++iter)
    {
        UINT16 nCol = iter->nCol;
        UINT16 nRow = iter->nRow;
        String aVal = iter->GetValue();

        r.ApplyFlagsTab( nCol, nRow, nCol, nRow, nNewTab, SC_MF_SCENARIO );

        r.SetString( nCol, nRow, nNewTab, aVal );
    }

    if( bLast )
        r.SetActiveScenario( nNewTab, TRUE );

    // modify what the Active tab is set to if the new
    // scenario tab occurs before the active tab.
    ScExtDocSettings& rDocSett = rRoot.GetExtDocOptions().GetDocSettings();
    if( (static_cast< SCCOL >( nTab ) < rDocSett.mnDisplTab) && (rDocSett.mnDisplTab < MAXTAB) )
        ++rDocSett.mnDisplTab;
    rRoot.GetTabInfo().InsertScTab( nNewTab );
}

void ExcScenarioList::Apply( const XclImpRoot& rRoot )
{
    sal_uInt16 n = static_cast<sal_uInt16>(aEntries.size());

    boost::ptr_vector<ExcScenario>::reverse_iterator iter;
    for (iter = aEntries.rbegin(); iter != aEntries.rend(); ++iter)
    {
        n--;
        iter->Apply(rRoot, n == nLastScenario);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
