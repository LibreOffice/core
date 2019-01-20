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

#include <memory>
#include <scitems.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/svxrtf.hxx>
#include <vcl/outdev.hxx>
#include <svtools/rtftoken.h>
#include <osl/diagnose.h>
#include <svl/itempool.hxx>

#include <rtfparse.hxx>

#define SC_RTFTWIPTOL 10        // 10 Twips tolerance when determining columns

ScRTFParser::ScRTFParser( EditEngine* pEditP ) :
        ScEEParser( pEditP ),
        mnCurPos(0),
        pActDefault( nullptr ),
        pDefMerge( nullptr ),
        nStartAdjust( sal_uLong(~0) ),
        nLastWidth(0),
        bNewDef( false )
{
    // RTF default FontSize 12Pt
    long nMM = OutputDevice::LogicToLogic( 12, MapUnit::MapPoint, MapUnit::Map100thMM );
    pPool->SetPoolDefaultItem( SvxFontHeightItem( nMM, 100, EE_CHAR_FONTHEIGHT ) );
    // Free-flying pInsDefault
    pInsDefault.reset( new ScRTFCellDefault( pPool ) );
}

ScRTFParser::~ScRTFParser()
{
    pInsDefault.reset();
    maDefaultList.clear();
}

ErrCode ScRTFParser::Read( SvStream& rStream, const OUString& rBaseURL )
{
    Link<RtfImportInfo&,void> aOldLink = pEdit->GetRtfImportHdl();
    pEdit->SetRtfImportHdl( LINK( this, ScRTFParser, RTFImportHdl ) );
    ErrCode nErr = pEdit->Read( rStream, rBaseURL, EETextFormat::Rtf );
    if ( nRtfLastToken == RTF_PAR )
    {
        if ( !maList.empty() )
        {
            auto& pE = maList.back();
            if (    // Completely empty
                (  pE->aSel.nStartPara == pE->aSel.nEndPara
                   && pE->aSel.nStartPos  == pE->aSel.nEndPos
                )
                ||  // Empty paragraph
                (  pE->aSel.nStartPara + 1 == pE->aSel.nEndPara
                   && pE->aSel.nStartPos      == pEdit->GetTextLen( pE->aSel.nStartPara )
                   && pE->aSel.nEndPos        == 0
                )
               )
            {   // Don't take over the last paragraph
                maList.pop_back();
            }
        }
    }
    ColAdjust();
    pEdit->SetRtfImportHdl( aOldLink );
    return nErr;
}

void ScRTFParser::EntryEnd( ScEEParseEntry* pE, const ESelection& aSel )
{
    // Paragraph -2 strips the attached empty paragraph
    pE->aSel.nEndPara = aSel.nEndPara - 2;
    // Although it's called nEndPos, the last one is position + 1
    pE->aSel.nEndPos = pEdit->GetTextLen( aSel.nEndPara - 1 );
}

inline void ScRTFParser::NextRow()
{
    if ( nRowMax < ++nRowCnt )
        nRowMax = nRowCnt;
}

bool ScRTFParser::SeekTwips( sal_uInt16 nTwips, SCCOL* pCol )
{
    ScRTFColTwips::const_iterator it = aColTwips.find( nTwips );
    bool bFound = it != aColTwips.end();
    sal_uInt16 nPos = it - aColTwips.begin();
    *pCol = static_cast<SCCOL>(nPos);
    if ( bFound )
        return true;
    sal_uInt16 nCount = aColTwips.size();
    if ( !nCount )
        return false;
    SCCOL nCol = *pCol;
    // nCol is insertion position; the next one higher up is there (or not)
    if ( nCol < static_cast<SCCOL>(nCount) && ((aColTwips[nCol] - SC_RTFTWIPTOL) <= nTwips) )
        return true;
    // Not smaller than everything else? Then compare with the next lower one
    else if ( nCol != 0 && ((aColTwips[nCol-1] + SC_RTFTWIPTOL) >= nTwips) )
    {
        (*pCol)--;
        return true;
    }
    return false;
}

void ScRTFParser::ColAdjust()
{
    if ( nStartAdjust != sal_uLong(~0) )
    {
        SCCOL nCol = 0;
        for (size_t i = nStartAdjust, nListSize = maList.size(); i < nListSize; ++i)
        {
            auto& pE = maList[i];
            if ( pE->nCol == 0 )
                nCol = 0;
            pE->nCol = nCol;
            if ( pE->nColOverlap > 1 )
                nCol = nCol + pE->nColOverlap; // Merged cells with \clmrg
            else
            {
                SeekTwips( pE->nTwips, &nCol );
                if ( ++nCol <= pE->nCol )
                    nCol = pE->nCol + 1; // Moved cell X
                pE->nColOverlap = nCol - pE->nCol; // Merged cells without \clmrg
            }
            if ( nCol > nColMax )
                nColMax = nCol;
        }
        nStartAdjust = sal_uLong(~0);
        aColTwips.clear();
    }
}

IMPL_LINK( ScRTFParser, RTFImportHdl, RtfImportInfo&, rInfo, void )
{
    switch ( rInfo.eState )
    {
        case RtfImportState::NextToken:
            ProcToken( &rInfo );
            break;
        case RtfImportState::UnknownAttr:
            ProcToken( &rInfo );
            break;
        case RtfImportState::Start:
        {
            SvxRTFParser* pParser = static_cast<SvxRTFParser*>(rInfo.pParser);
            pParser->SetAttrPool( pPool );
            RTFPardAttrMapIds& rMap = pParser->GetPardMap();
            rMap.nBrush = ATTR_BACKGROUND;
            rMap.nBox = ATTR_BORDER;
            rMap.nShadow = ATTR_SHADOW;
        }
            break;
        case RtfImportState::End:
            if ( rInfo.aSelection.nEndPos )
            {   // If still text: create last paragraph
                pActDefault = nullptr;
                rInfo.nToken = RTF_PAR;
                // EditEngine did not attach an empty paragraph anymore
                // which EntryEnd could strip
                rInfo.aSelection.nEndPara++;
                ProcToken( &rInfo );
            }
            break;
        case RtfImportState::SetAttr:
            break;
        case RtfImportState::InsertText:
            break;
        case RtfImportState::InsertPara:
            break;
        default:
            OSL_FAIL("unknown ImportInfo.eState");
    }
}

// Bad behavior:
// For RTF_INTBL or respectively at the start of the first RTF_CELL
// after RTF_CELLX if there was no RTF_INTBL
void ScRTFParser::NewCellRow()
{
    if ( bNewDef )
    {
        bNewDef = false;
        // Not flush on the right? => new table
        if ( nLastWidth && !maDefaultList.empty() )
        {
            const ScRTFCellDefault& rD = *maDefaultList.back().get();
            if (rD.nTwips != nLastWidth)
            {
                SCCOL n1, n2;
                if ( !(  SeekTwips( nLastWidth, &n1 )
                      && SeekTwips( rD.nTwips, &n2 )
                      && n1 == n2
                      )
                )
                {
                    ColAdjust();
                }
            }
        }
        // Build up TwipCols only after nLastWidth comparison!
        for (std::unique_ptr<ScRTFCellDefault> & pCellDefault : maDefaultList)
        {
            const ScRTFCellDefault& rD = *pCellDefault;
            SCCOL nCol;
            if ( !SeekTwips(rD.nTwips, &nCol) )
                aColTwips.insert( rD.nTwips );
        }
    }
    pDefMerge = nullptr;
    pActDefault = maDefaultList.empty() ? nullptr : maDefaultList[0].get();
    mnCurPos = 0;
    OSL_ENSURE( pActDefault, "NewCellRow: pActDefault==0" );
}

/*
    SW:
    ~~~
    [\par]
    \trowd \cellx \cellx ...
    \intbl \cell \cell ...
    \row
    [\par]
    [\trowd \cellx \cellx ...]
    \intbl \cell \cell ...
    \row
    [\par]

    M$-Word:
    ~~~~~~~~
    [\par]
    \trowd \cellx \cellx ...
    \intbl \cell \cell ...
    \intbl \row
    [\par]
    [\trowd \cellx \cellx ...]
    \intbl \cell \cell ...
    \intbl \row
    [\par]

 */

void ScRTFParser::ProcToken( RtfImportInfo* pInfo )
{
    switch ( pInfo->nToken )
    {
        case RTF_TROWD:         // denotes table row default, before RTF_CELLX
        {
            if (!maDefaultList.empty())
                nLastWidth = maDefaultList.back()->nTwips;

            nColCnt = 0;
            if (pActDefault != pInsDefault.get())
                pActDefault = nullptr;
            maDefaultList.clear();
            pDefMerge = nullptr;
            nRtfLastToken = pInfo->nToken;
            mnCurPos = 0;
        }
        break;
        case RTF_CLMGF:         // The first cell of cells to be merged
        {
            pDefMerge = pInsDefault.get();
            nRtfLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLMRG:         // A cell to be merged with the preceding cell
        {
            if (!pDefMerge && !maDefaultList.empty())
            {
                pDefMerge = maDefaultList.back().get();
                mnCurPos = maDefaultList.size() - 1;
            }
            OSL_ENSURE( pDefMerge, "RTF_CLMRG: pDefMerge==0" );
            if ( pDefMerge ) // Else broken RTF
                pDefMerge->nColOverlap++;   // multiple successive ones possible
            pInsDefault->nColOverlap = 0;   // Flag: ignore these
            nRtfLastToken = pInfo->nToken;
        }
        break;
        case RTF_CELLX:         // closes cell default
        {
            bNewDef = true;
            pInsDefault->nCol = nColCnt;
            pInsDefault->nTwips = pInfo->nTokenValue; // Right cell border
            maDefaultList.push_back( std::move(pInsDefault) );
            // New free-flying pInsDefault
            pInsDefault.reset( new ScRTFCellDefault( pPool ) );
            if ( ++nColCnt > nColMax )
                nColMax = nColCnt;
            nRtfLastToken = pInfo->nToken;
        }
        break;
        case RTF_INTBL:         // before the first RTF_CELL
        {
            // Once over NextToken and once over UnknownAttrToken
            // or e.g. \intbl ... \cell \pard \intbl ... \cell
            if ( nRtfLastToken != RTF_INTBL && nRtfLastToken != RTF_CELL && nRtfLastToken != RTF_PAR )
            {
                NewCellRow();
                nRtfLastToken = pInfo->nToken;
            }
        }
        break;
        case RTF_CELL:          // denotes the end of a cell.
        {
            OSL_ENSURE( pActDefault, "RTF_CELL: pActDefault==0" );
            if ( bNewDef || !pActDefault )
                NewCellRow();    // before was no \intbl, bad behavior
            // Broken RTF? Let's save what we can
            if ( !pActDefault )
                pActDefault = pInsDefault.get();
            if ( pActDefault->nColOverlap > 0 )
            {   // Not merged with preceding
                mxActEntry->nCol = pActDefault->nCol;
                mxActEntry->nColOverlap = pActDefault->nColOverlap;
                mxActEntry->nTwips = pActDefault->nTwips;
                mxActEntry->nRow = nRowCnt;
                mxActEntry->aItemSet.Set(pActDefault->aItemSet);
                EntryEnd(mxActEntry.get(), pInfo->aSelection);

                if ( nStartAdjust == sal_uLong(~0) )
                    nStartAdjust = maList.size();
                maList.push_back(mxActEntry);
                NewActEntry(mxActEntry.get()); // New free-flying mxActEntry
            }
            else
            {   // Assign current Twips to MergeCell
                if ( !maList.empty() )
                {
                    auto& pE = maList.back();
                    pE->nTwips = pActDefault->nTwips;
                }
                // Adjust selection of free-flying mxActEntry
                // Paragraph -1 due to separated text in EditEngine during parsing
                mxActEntry->aSel.nStartPara = pInfo->aSelection.nEndPara - 1;
            }

            pActDefault = nullptr;
            if (!maDefaultList.empty() && (mnCurPos+1) < maDefaultList.size())
                pActDefault = maDefaultList[++mnCurPos].get();

            nRtfLastToken = pInfo->nToken;
        }
        break;
        case RTF_ROW:           // denotes the end of a row
        {
            NextRow();
            nRtfLastToken = pInfo->nToken;
        }
        break;
        case RTF_PAR:           // Paragraph
        {
            if ( !pActDefault )
            {   // text not in table
                ColAdjust();    // close the processing table
                mxActEntry->nCol = 0;
                mxActEntry->nRow = nRowCnt;
                EntryEnd(mxActEntry.get(), pInfo->aSelection);
                maList.push_back(mxActEntry);
                NewActEntry(mxActEntry.get());   // new mxActEntry
                NextRow();
            }
            nRtfLastToken = pInfo->nToken;
        }
        break;
        default:
        {   // do not set nRtfLastToken
            switch ( pInfo->nToken & ~(0xff | RTF_TABLEDEF) )
            {
                case RTF_SHADINGDEF:
                    static_cast<SvxRTFParser*>(pInfo->pParser)->ReadBackgroundAttr(
                        pInfo->nToken, pInsDefault->aItemSet, true );
                break;
                case RTF_BRDRDEF:
                    static_cast<SvxRTFParser*>(pInfo->pParser)->ReadBorderAttr(
                        pInfo->nToken, pInsDefault->aItemSet, true );
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
