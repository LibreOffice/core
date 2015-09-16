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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/svxrtf.hxx>
#include <vcl/outdev.hxx>
#include <svtools/rtftoken.h>

#include "rtfparse.hxx"
#include "global.hxx"
#include "document.hxx"
#include "docpool.hxx"

#define SC_RTFTWIPTOL 10        // 10 Twips tolerance when determining columns

ScRTFParser::ScRTFParser( EditEngine* pEditP ) :
        ScEEParser( pEditP ),
        mnCurPos(0),
        pColTwips( new ScRTFColTwips ),
        pActDefault( NULL ),
        pDefMerge( NULL ),
        nStartAdjust( (sal_uLong)~0 ),
        nLastWidth(0),
        bNewDef( false )
{
    // RTF default FontSize 12Pt
    long nMM = OutputDevice::LogicToLogic( 12, MAP_POINT, MAP_100TH_MM );
    pPool->SetPoolDefaultItem( SvxFontHeightItem( nMM, 100, EE_CHAR_FONTHEIGHT ) );
    // Free-flying pInsDefault
    pInsDefault = new ScRTFCellDefault( pPool );
}

ScRTFParser::~ScRTFParser()
{
    delete pInsDefault;
    delete pColTwips;
    maDefaultList.clear();
}

sal_uLong ScRTFParser::Read( SvStream& rStream, const OUString& rBaseURL )
{
    Link<ImportInfo&,void> aOldLink = pEdit->GetImportHdl();
    pEdit->SetImportHdl( LINK( this, ScRTFParser, RTFImportHdl ) );
    sal_uLong nErr = pEdit->Read( rStream, rBaseURL, EE_FORMAT_RTF );
    if ( nLastToken == RTF_PAR )
    {
        if ( !maList.empty() )
        {
            ScEEParseEntry* pE = maList.back();
            if (    // Completely empty
                (  (  pE->aSel.nStartPara == pE->aSel.nEndPara
                   && pE->aSel.nStartPos  == pE->aSel.nEndPos
                   )
                ||  // Empty paragraph
                   (  pE->aSel.nStartPara + 1 == pE->aSel.nEndPara
                   && pE->aSel.nStartPos      == pEdit->GetTextLen( pE->aSel.nStartPara )
                   && pE->aSel.nEndPos        == 0
                   )
                )
               )
            {   // Don't take over the last paragraph
                maList.pop_back();
            }
        }
    }
    ColAdjust();
    pEdit->SetImportHdl( aOldLink );
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
    ScRTFColTwips::const_iterator it = pColTwips->find( nTwips );
    bool bFound = it != pColTwips->end();
    sal_uInt16 nPos = it - pColTwips->begin();
    *pCol = static_cast<SCCOL>(nPos);
    if ( bFound )
        return true;
    sal_uInt16 nCount = pColTwips->size();
    if ( !nCount )
        return false;
    SCCOL nCol = *pCol;
    // nCol is insertion position; the next one higher up is there (or not)
    if ( nCol < static_cast<SCCOL>(nCount) && (((*pColTwips)[nCol] - SC_RTFTWIPTOL) <= nTwips) )
        return true;
    // Not smaller than everything else? Then compare with the next lower one
    else if ( nCol != 0 && (((*pColTwips)[nCol-1] + SC_RTFTWIPTOL) >= nTwips) )
    {
        (*pCol)--;
        return true;
    }
    return false;
}

void ScRTFParser::ColAdjust()
{
    if ( nStartAdjust != (sal_uLong)~0 )
    {
        SCCOL nCol = 0;
        ScEEParseEntry* pE;
        for ( size_t i = nStartAdjust, nListSize = maList.size(); i < nListSize; ++ i )
        {
            pE = maList[ i ];
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
        nStartAdjust = (sal_uLong)~0;
        pColTwips->clear();
    }
}

IMPL_LINK_TYPED( ScRTFParser, RTFImportHdl, ImportInfo&, rInfo, void )
{
    switch ( rInfo.eState )
    {
        case RTFIMP_NEXTTOKEN:
            ProcToken( &rInfo );
            break;
        case RTFIMP_UNKNOWNATTR:
            ProcToken( &rInfo );
            break;
        case RTFIMP_START:
        {
            SvxRTFParser* pParser = static_cast<SvxRTFParser*>(rInfo.pParser);
            pParser->SetAttrPool( pPool );
            RTFPardAttrMapIds& rMap = pParser->GetPardMap();
            rMap.nBrush = ATTR_BACKGROUND;
            rMap.nBox = ATTR_BORDER;
            rMap.nShadow = ATTR_SHADOW;
        }
            break;
        case RTFIMP_END:
            if ( rInfo.aSelection.nEndPos )
            {   // If still text: create last paragraph
                pActDefault = NULL;
                rInfo.nToken = RTF_PAR;
                // EditEngine did not attach an empty paragraph anymore
                // which EntryEnd could strip
                rInfo.aSelection.nEndPara++;
                ProcToken( &rInfo );
            }
            break;
        case RTFIMP_SETATTR:
            break;
        case RTFIMP_INSERTTEXT:
            break;
        case RTFIMP_INSERTPARA:
            break;
        default:
            OSL_FAIL("unknown ImportInfo.eState");
    }
}

// Bad behavior:
// For RTF_INTBL or respectively at the start of the first RTF_CELL
// after RTF_CELLX if there was no RTF_INTBL
void ScRTFParser::NewCellRow( ImportInfo* /*pInfo*/ )
{
    if ( bNewDef )
    {
        bNewDef = false;
        // Not flush on the right? => new table
        if ( nLastWidth && !maDefaultList.empty() )
        {
            const ScRTFCellDefault& rD = maDefaultList.back();
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
        for ( size_t i = 0, n = maDefaultList.size(); i < n; ++i )
        {
            const ScRTFCellDefault& rD = maDefaultList[i];
            SCCOL nCol;
            if ( !SeekTwips(rD.nTwips, &nCol) )
                pColTwips->insert( rD.nTwips );
        }
    }
    pDefMerge = NULL;
    pActDefault = maDefaultList.empty() ? NULL : &maDefaultList[0];
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

void ScRTFParser::ProcToken( ImportInfo* pInfo )
{
    switch ( pInfo->nToken )
    {
        case RTF_TROWD:         // denotes table row defauls, before RTF_CELLX
        {
            if (!maDefaultList.empty())
                nLastWidth = maDefaultList.back().nTwips;

            nColCnt = 0;
            maDefaultList.clear();
            pDefMerge = NULL;
            nLastToken = pInfo->nToken;
            mnCurPos = 0;
        }
        break;
        case RTF_CLMGF:         // The first cell of cells to be merged
        {
            pDefMerge = pInsDefault;
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLMRG:         // A cell to be merged with the preceding cell
        {
            if (!pDefMerge && !maDefaultList.empty())
            {
                pDefMerge = &maDefaultList.back();
                mnCurPos = maDefaultList.size() - 1;
            }
            OSL_ENSURE( pDefMerge, "RTF_CLMRG: pDefMerge==0" );
            if ( pDefMerge ) // Else broken RTF
                pDefMerge->nColOverlap++;   // multiple successive ones possible
            pInsDefault->nColOverlap = 0;   // Flag: ignore these
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_CELLX:         // closes cell default
        {
            bNewDef = true;
            pInsDefault->nCol = nColCnt;
            pInsDefault->nTwips = pInfo->nTokenValue; // Right cell border
            maDefaultList.push_back( pInsDefault );
            // New free-flying pInsDefault
            pInsDefault = new ScRTFCellDefault( pPool );
            if ( ++nColCnt > nColMax )
                nColMax = nColCnt;
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_INTBL:         // before the first RTF_CELL
        {
            // Once over NextToken and once over UnknownAttrToken
            // or e.g. \intbl ... \cell \pard \intbl ... \cell
            if ( nLastToken != RTF_INTBL && nLastToken != RTF_CELL && nLastToken != RTF_PAR )
            {
                NewCellRow( pInfo );
                nLastToken = pInfo->nToken;
            }
        }
        break;
        case RTF_CELL:          // denotes the end of a cell.
        {
            OSL_ENSURE( pActDefault, "RTF_CELL: pActDefault==0" );
            if ( bNewDef || !pActDefault )
                NewCellRow( pInfo );    // davor war kein \intbl, bad behavior
            // Broken RTF? Let's save what we can
            if ( !pActDefault )
                pActDefault = pInsDefault;
            if ( pActDefault->nColOverlap > 0 )
            {   // Not merged with preceding
                pActEntry->nCol = pActDefault->nCol;
                pActEntry->nColOverlap = pActDefault->nColOverlap;
                pActEntry->nTwips = pActDefault->nTwips;
                pActEntry->nRow = nRowCnt;
                pActEntry->aItemSet.Set( pActDefault->aItemSet );
                EntryEnd( pActEntry, pInfo->aSelection );

                if ( nStartAdjust == (sal_uLong)~0 )
                    nStartAdjust = maList.size();
                maList.push_back( pActEntry );
                NewActEntry( pActEntry ); // New free-flying pActEntry
            }
            else
            {   // Assign current Twips to MergeCell
                if ( !maList.empty() )
                {
                    ScEEParseEntry* pE = maList.back();
                    pE->nTwips = pActDefault->nTwips;
                }
                // Adjust selection of free-flying pActEntry
                // Paragraph -1 due to separated text in EditEngine during parsing
                pActEntry->aSel.nStartPara = pInfo->aSelection.nEndPara - 1;
            }

            pActDefault = NULL;
            if (!maDefaultList.empty() && (mnCurPos+1) < maDefaultList.size())
                pActDefault = &maDefaultList[++mnCurPos];

            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_ROW:           // denotes the end of a row
        {
            NextRow();
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_PAR:           // Paragraph
        {
            if ( !pActDefault )
            {   // text not in table
                ColAdjust();    // close the processing table
                pActEntry->nCol = 0;
                pActEntry->nRow = nRowCnt;
                EntryEnd( pActEntry, pInfo->aSelection );
                maList.push_back( pActEntry );
                NewActEntry( pActEntry );   // new pActEntry
                NextRow();
            }
            nLastToken = pInfo->nToken;
        }
        break;
        default:
        {   // do not set nLastToken
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
