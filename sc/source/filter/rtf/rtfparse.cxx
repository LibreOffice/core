/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include <editeng/editeng.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/svxrtf.hxx>
#include <vcl/outdev.hxx>
#include <svtools/rtftoken.h>

#define SC_RTFPARSE_CXX
#include "rtfparse.hxx"
#include "global.hxx"
#include "document.hxx"
#include "docpool.hxx"

#define SC_RTFTWIPTOL 10        




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
    
    long nMM = OutputDevice::LogicToLogic( 12, MAP_POINT, MAP_100TH_MM );
    pPool->SetPoolDefaultItem( SvxFontHeightItem( nMM, 100, EE_CHAR_FONTHEIGHT ) );
    
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
    Link aOldLink = pEdit->GetImportHdl();
    pEdit->SetImportHdl( LINK( this, ScRTFParser, RTFImportHdl ) );
    sal_uLong nErr = pEdit->Read( rStream, rBaseURL, EE_FORMAT_RTF );
    if ( nLastToken == RTF_PAR )
    {
        if ( !maList.empty() )
        {
            ScEEParseEntry* pE = maList.back();
            if (    
                (  (  pE->aSel.nStartPara == pE->aSel.nEndPara
                   && pE->aSel.nStartPos  == pE->aSel.nEndPos
                   )
                ||  
                   (  pE->aSel.nStartPara + 1 == pE->aSel.nEndPara
                   && pE->aSel.nStartPos      == pEdit->GetTextLen( pE->aSel.nStartPara )
                   && pE->aSel.nEndPos        == 0
                   )
                )
               )
            {   
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
    
    pE->aSel.nEndPara = aSel.nEndPara - 2;
    
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
    sal_Bool bFound = it != pColTwips->end();
    sal_uInt16 nPos = it - pColTwips->begin();
    *pCol = static_cast<SCCOL>(nPos);
    if ( bFound )
        return true;
    sal_uInt16 nCount = pColTwips->size();
    if ( !nCount )
        return false;
    SCCOL nCol = *pCol;
    
    if ( nCol < static_cast<SCCOL>(nCount) && (((*pColTwips)[nCol] - SC_RTFTWIPTOL) <= nTwips) )
        return true;
    
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
                nCol = nCol + pE->nColOverlap; 
            else
            {
                SeekTwips( pE->nTwips, &nCol );
                if ( ++nCol <= pE->nCol )
                    nCol = pE->nCol + 1; 
                pE->nColOverlap = nCol - pE->nCol; 
            }
            if ( nCol > nColMax )
                nColMax = nCol;
        }
        nStartAdjust = (sal_uLong)~0;
        pColTwips->clear();
    }
}


IMPL_LINK( ScRTFParser, RTFImportHdl, ImportInfo*, pInfo )
{
    switch ( pInfo->eState )
    {
        case RTFIMP_NEXTTOKEN:
            ProcToken( pInfo );
            break;
        case RTFIMP_UNKNOWNATTR:
            ProcToken( pInfo );
            break;
        case RTFIMP_START:
        {
            SvxRTFParser* pParser = (SvxRTFParser*) pInfo->pParser;
            pParser->SetAttrPool( pPool );
            RTFPardAttrMapIds& rMap = pParser->GetPardMap();
            rMap.nBrush = ATTR_BACKGROUND;
            rMap.nBox = ATTR_BORDER;
            rMap.nShadow = ATTR_SHADOW;
        }
            break;
        case RTFIMP_END:
            if ( pInfo->aSelection.nEndPos )
            {   
                pActDefault = NULL;
                pInfo->nToken = RTF_PAR;
                
                
                pInfo->aSelection.nEndPara++;
                ProcToken( pInfo );
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
    return 0;
}




void ScRTFParser::NewCellRow( ImportInfo* /*pInfo*/ )
{
    if ( bNewDef )
    {
        bNewDef = false;
        
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
    ScEEParseEntry* pE;
    switch ( pInfo->nToken )
    {
        case RTF_TROWD:         
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
        case RTF_CLMGF:         
        {
            pDefMerge = pInsDefault;
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLMRG:         
        {
            if (!pDefMerge && !maDefaultList.empty())
            {
                pDefMerge = &maDefaultList.back();
                mnCurPos = maDefaultList.size() - 1;
            }
            OSL_ENSURE( pDefMerge, "RTF_CLMRG: pDefMerge==0" );
            if ( pDefMerge ) 
                pDefMerge->nColOverlap++;   
            pInsDefault->nColOverlap = 0;   
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_CELLX:         
        {
            bNewDef = true;
            pInsDefault->nCol = nColCnt;
            pInsDefault->nTwips = pInfo->nTokenValue; 
            maDefaultList.push_back( pInsDefault );
            
            pInsDefault = new ScRTFCellDefault( pPool );
            if ( ++nColCnt > nColMax )
                nColMax = nColCnt;
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_INTBL:         
        {
            
            
            if ( nLastToken != RTF_INTBL && nLastToken != RTF_CELL && nLastToken != RTF_PAR )
            {
                NewCellRow( pInfo );
                nLastToken = pInfo->nToken;
            }
        }
        break;
        case RTF_CELL:          
        {
            OSL_ENSURE( pActDefault, "RTF_CELL: pActDefault==0" );
            if ( bNewDef || !pActDefault )
                NewCellRow( pInfo );    
            
            if ( !pActDefault )
                pActDefault = pInsDefault;
            if ( pActDefault->nColOverlap > 0 )
            {   
                pActEntry->nCol = pActDefault->nCol;
                pActEntry->nColOverlap = pActDefault->nColOverlap;
                pActEntry->nTwips = pActDefault->nTwips;
                pActEntry->nRow = nRowCnt;
                pActEntry->aItemSet.Set( pActDefault->aItemSet );
                EntryEnd( pActEntry, pInfo->aSelection );

                if ( nStartAdjust == (sal_uLong)~0 )
                    nStartAdjust = maList.size();
                maList.push_back( pActEntry );
                NewActEntry( pActEntry ); 
            }
            else
            {   
                if ( !maList.empty() )
                {
                    pE = maList.back();
                    pE->nTwips = pActDefault->nTwips;
                }
                
                
                pActEntry->aSel.nStartPara = pInfo->aSelection.nEndPara - 1;
            }

            pActDefault = NULL;
            if (!maDefaultList.empty() && (mnCurPos+1) < maDefaultList.size())
                pActDefault = &maDefaultList[++mnCurPos];

            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_ROW:           
        {
            NextRow();
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_PAR:           
        {
            if ( !pActDefault )
            {   
                ColAdjust();    
                pActEntry->nCol = 0;
                pActEntry->nRow = nRowCnt;
                EntryEnd( pActEntry, pInfo->aSelection );
                maList.push_back( pActEntry );
                NewActEntry( pActEntry );   
                NextRow();
            }
            nLastToken = pInfo->nToken;
        }
        break;
        default:
        {   
            switch ( pInfo->nToken & ~(0xff | RTF_TABLEDEF) )
            {
                case RTF_SHADINGDEF:
                    ((SvxRTFParser*)pInfo->pParser)->ReadBackgroundAttr(
                        pInfo->nToken, pInsDefault->aItemSet, sal_True );
                break;
                case RTF_BRDRDEF:
                    ((SvxRTFParser*)pInfo->pParser)->ReadBorderAttr(
                        pInfo->nToken, pInsDefault->aItemSet, sal_True );
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
