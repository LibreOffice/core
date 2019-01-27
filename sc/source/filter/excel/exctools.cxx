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

#include <osl/diagnose.h>
#include <document.hxx>
#include <attrib.hxx>
#include <scextopt.hxx>
#include <olinetab.hxx>

#include <root.hxx>
#include <excimp8.hxx>
#include <namebuff.hxx>
#include <otlnbuff.hxx>
#include <formel.hxx>
#include <xilink.hxx>

#include <memory>
#include <vector>

RootData::RootData()
{
    eDateiTyp = BiffX;
    pFmlaConverter = nullptr;

    pPrintRanges.reset( new ScRangeListTabs );
    pPrintTitles.reset( new ScRangeListTabs );

    pTabId = nullptr;
    pUserBViewList = nullptr;

    pIR = nullptr;
    pER = nullptr;
    pColRowBuff = nullptr;
}

RootData::~RootData()
{
    pExtSheetBuff.reset();
    pShrfmlaBuff.reset();
    pExtNameBuff.reset();
    pAutoFilterBuffer.reset();
    pPrintRanges.reset();
    pPrintTitles.reset();
}

XclImpOutlineBuffer::XclImpOutlineBuffer( SCSIZE nNewSize ) :
    maLevels(0, nNewSize, 0),
    mpOutlineArray(nullptr),
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
    for (const auto& [nPos, nLevel] : maLevels)
    {
        if (nPos >= mnEndPos)
        {
            // Don't go beyond the max allowed position.
            OSL_ENSURE(aOutlineStack.empty(), "XclImpOutlineBuffer::MakeScOutline: outline stack not empty but expected to be.");
            break;
        }
        sal_uInt8 nCurLevel = static_cast<sal_uInt8>(aOutlineStack.size());
        if (nLevel > nCurLevel)
        {
            for (sal_uInt8 i = 0; i < nLevel - nCurLevel; ++i)
                aOutlineStack.push_back(nPos);
        }
        else
        {
            OSL_ENSURE(nLevel <= nCurLevel, "XclImpOutlineBuffer::MakeScOutline: unexpected level!");
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

                bool bDummy;
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

ExcScenarioCell::ExcScenarioCell( const sal_uInt16 nC, const sal_uInt16 nR )
    : nCol( nC ), nRow( nR )
{
}

ExcScenario::ExcScenario( XclImpStream& rIn, const RootData& rR )
    : nTab( rR.pIR->GetCurrScTab() )
{
    sal_uInt16          nCref;
    sal_uInt8           nName, nComment;

    nCref = rIn.ReaduInt16();
    nProtected = rIn.ReaduInt8();
    rIn.Ignore( 1 );                // Hide
    nName = rIn.ReaduInt8();
    nComment = rIn.ReaduInt8();
    rIn.Ignore( 1 );       // instead of nUser!

    if( nName )
        aName = rIn.ReadUniString( nName );
    else
    {
        aName = "Scenery";
        rIn.Ignore( 1 );
    }

    rIn.ReadUniString(); // username

    if( nComment )
        aComment = rIn.ReadUniString();

    sal_uInt16          n = nCref;
    sal_uInt16          nC, nR;
    aEntries.reserve(n);
    while( n )
    {
        nR = rIn.ReaduInt16();
        nC = rIn.ReaduInt16();

        aEntries.emplace_back( nC, nR );

        n--;
    }

    for (auto& rEntry : aEntries)
        rEntry.SetValue(rIn.ReadUniString());
}

void ExcScenario::Apply( const XclImpRoot& rRoot, const bool bLast )
{
    ScDocument&         r = rRoot.GetDoc();
    OUString            aSzenName( aName );
    sal_uInt16          nNewTab = nTab + 1;

    if( !r.InsertTab( nNewTab, aSzenName ) )
        return;

    r.SetScenario( nNewTab, true );
    // do not show scenario frames
    const ScScenarioFlags nFlags = ScScenarioFlags::CopyAll
                                 | (nProtected ? ScScenarioFlags::Protected : ScScenarioFlags::NONE);
                              /* | ScScenarioFlags::ShowFrame*/
    r.SetScenarioData( nNewTab, aComment, COL_LIGHTGRAY, nFlags);

    for (const auto& rEntry : aEntries)
    {
        sal_uInt16 nCol = rEntry.nCol;
        sal_uInt16 nRow = rEntry.nRow;
        OUString aVal = rEntry.GetValue();

        r.ApplyFlagsTab( nCol, nRow, nCol, nRow, nNewTab, ScMF::Scenario );

        r.SetString( nCol, nRow, nNewTab, aVal );
    }

    if( bLast )
        r.SetActiveScenario( nNewTab, true );

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

    std::vector< std::unique_ptr<ExcScenario> >::reverse_iterator iter;
    for (iter = aEntries.rbegin(); iter != aEntries.rend(); ++iter)
    {
        n--;
        (*iter)->Apply(rRoot, n == nLastScenario);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
