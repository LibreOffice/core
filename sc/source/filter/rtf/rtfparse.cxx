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

#define SC_RTFTWIPTOL 10        // 10 Twips Toleranz bei Spaltenbestimmung



SV_IMPL_VARARR_SORT( ScRTFColTwips, sal_uLong );



ScRTFParser::ScRTFParser( EditEngine* pEditP ) :
        ScEEParser( pEditP ),
        pDefaultList( new ScRTFDefaultList ),
        pColTwips( new ScRTFColTwips ),
        pActDefault( NULL ),
        pDefMerge( NULL ),
        nStartAdjust( (sal_uLong)~0 ),
        nLastWidth(0),
        bNewDef( sal_False )
{
    // RTF default FontSize 12Pt
    long nMM = OutputDevice::LogicToLogic( 12, MAP_POINT, MAP_100TH_MM );
    pPool->SetPoolDefaultItem( SvxFontHeightItem( nMM, 100, EE_CHAR_FONTHEIGHT ) );
    // freifliegender pInsDefault
    pInsDefault = new ScRTFCellDefault( pPool );
}


ScRTFParser::~ScRTFParser()
{
    delete pInsDefault;
    delete pColTwips;
    for ( ScRTFCellDefault* pD = pDefaultList->First(); pD; pD = pDefaultList->Next() )
        delete pD;
    delete pDefaultList;
}


sal_uLong ScRTFParser::Read( SvStream& rStream, const String& rBaseURL )
{
    Link aOldLink = pEdit->GetImportHdl();
    pEdit->SetImportHdl( LINK( this, ScRTFParser, RTFImportHdl ) );
    sal_uLong nErr = pEdit->Read( rStream, rBaseURL, EE_FORMAT_RTF );
    if ( nLastToken == RTF_PAR )
    {
        ScEEParseEntry* pE = pList->Last();
        if ( pE
                // komplett leer
            && (( pE->aSel.nStartPara == pE->aSel.nEndPara
                    && pE->aSel.nStartPos == pE->aSel.nEndPos)
                // leerer Paragraph
                || ( pE->aSel.nStartPara + 1 == pE->aSel.nEndPara
                    && pE->aSel.nStartPos == pEdit->GetTextLen( pE->aSel.nStartPara )
                    && pE->aSel.nEndPos == 0 )) )
        {   // den letzten leeren Absatz nicht uebernehmen
            pList->Remove();
            delete pE;
        }
    }
    ColAdjust();
    pEdit->SetImportHdl( aOldLink );
    return nErr;
}


void ScRTFParser::EntryEnd( ScEEParseEntry* pE, const ESelection& aSel )
{
    // Paragraph -2 stript den angehaengten leeren Paragraph
    pE->aSel.nEndPara = aSel.nEndPara - 2;
    // obwohl das nEndPos heisst, ist das letzte Position + 1
    pE->aSel.nEndPos = pEdit->GetTextLen( aSel.nEndPara - 1 );
}


inline void ScRTFParser::NextRow()
{
    if ( nRowMax < ++nRowCnt )
        nRowMax = nRowCnt;
}


sal_Bool ScRTFParser::SeekTwips( sal_uInt16 nTwips, SCCOL* pCol )
{
    sal_uInt16 nPos;
    sal_Bool bFound = pColTwips->Seek_Entry( nTwips, &nPos );
    *pCol = static_cast<SCCOL>(nPos);
    if ( bFound )
        return sal_True;
    sal_uInt16 nCount = pColTwips->Count();
    if ( !nCount )
        return sal_False;
    SCCOL nCol = *pCol;
    // nCol ist Einfuegeposition, da liegt der Naechsthoehere (oder auch nicht)
    if ( nCol < static_cast<SCCOL>(nCount) && (((*pColTwips)[nCol] - SC_RTFTWIPTOL) <= nTwips) )
        return sal_True;
    // nicht kleiner als alles andere? dann mit Naechstniedrigerem vergleichen
    else if ( nCol != 0 && (((*pColTwips)[nCol-1] + SC_RTFTWIPTOL) >= nTwips) )
    {
        (*pCol)--;
        return sal_True;
    }
    return sal_False;
}


void ScRTFParser::ColAdjust()
{
    if ( nStartAdjust != (sal_uLong)~0 )
    {
        SCCOL nCol = 0;
        ScEEParseEntry* pE;
        pE = pList->Seek( nStartAdjust );
        while ( pE )
        {
            if ( pE->nCol == 0 )
                nCol = 0;
            pE->nCol = nCol;
            if ( pE->nColOverlap > 1 )
                nCol = nCol + pE->nColOverlap;       // merged cells mit \clmrg
            else
            {
                SeekTwips( pE->nTwips, &nCol );
                if ( ++nCol <= pE->nCol )
                    nCol = pE->nCol + 1;        // verschobene Zell-X
                pE->nColOverlap = nCol - pE->nCol;      // merged cells ohne \clmrg
            }
            if ( nCol > nColMax )
                nColMax = nCol;
            pE = pList->Next();
        }
        nStartAdjust = (sal_uLong)~0;
        pColTwips->Remove( (sal_uInt16)0, pColTwips->Count() );
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
            {   // falls noch Text: letzten Absatz erzeugen
                pActDefault = NULL;
                pInfo->nToken = RTF_PAR;
                // EditEngine hat keinen leeren Paragraph mehr angehaengt
                // den EntryEnd strippen koennte
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
            DBG_ERRORFILE("unknown ImportInfo.eState");
    }
    return 0;
}


// bei RTF_INTBL bzw. am Anfang von erstem RTF_CELL nach RTF_CELLX wenn es
// kein RTF_INTBL gab, bad behavior
void ScRTFParser::NewCellRow( ImportInfo* /*pInfo*/ )
{
    if ( bNewDef )
    {
        ScRTFCellDefault* pD;
        bNewDef = sal_False;
        // rechts nicht buendig? => neue Tabelle
        if ( nLastWidth
          && ((pD = pDefaultList->Last()) != 0) && pD->nTwips != nLastWidth )
        {
            SCCOL n1, n2;
            if ( !( SeekTwips( nLastWidth, &n1 )
                && SeekTwips( pD->nTwips, &n2 ) && n1 == n2) )
                ColAdjust();
        }
        // TwipCols aufbauen, erst nach nLastWidth Vergleich!
        for ( pD = pDefaultList->First(); pD; pD = pDefaultList->Next() )
        {
            SCCOL n;
            if ( !SeekTwips( pD->nTwips, &n ) )
                pColTwips->Insert( pD->nTwips );
        }
    }
    pDefMerge = NULL;
    pActDefault = pDefaultList->First();
    DBG_ASSERT( pActDefault, "NewCellRow: pActDefault==0" );
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
    ScRTFCellDefault* pD;
    ScEEParseEntry* pE;
    switch ( pInfo->nToken )
    {
        case RTF_TROWD:         // denotes table row defauls, before RTF_CELLX
        {
            if ( (pD = pDefaultList->Last()) != 0 )
                nLastWidth = pD->nTwips;
            nColCnt = 0;
            for ( pD = pDefaultList->First(); pD; pD = pDefaultList->Next() )
                delete pD;
            pDefaultList->Clear();
            pDefMerge = NULL;
            nLastToken = pInfo->nToken;
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
            if ( !pDefMerge )
                pDefMerge = pDefaultList->Last();
            DBG_ASSERT( pDefMerge, "RTF_CLMRG: pDefMerge==0" );
            if ( pDefMerge )        // sonst rottes RTF
                pDefMerge->nColOverlap++;   // mehrere nacheinander moeglich
            pInsDefault->nColOverlap = 0;   // Flag: ignoriere diese
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_CELLX:         // closes cell default
        {
            bNewDef = sal_True;
            pInsDefault->nCol = nColCnt;
            pInsDefault->nTwips = pInfo->nTokenValue;   // rechter Zellenrand
            pDefaultList->Insert( pInsDefault, LIST_APPEND );
            // neuer freifliegender pInsDefault
            pInsDefault = new ScRTFCellDefault( pPool );
            if ( ++nColCnt > nColMax )
                nColMax = nColCnt;
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_INTBL:         // before the first RTF_CELL
        {
            // einmal ueber NextToken und einmal ueber UnknownAttrToken
            // oder z.B. \intbl ... \cell \pard \intbl ... \cell
            if ( nLastToken != RTF_INTBL && nLastToken != RTF_CELL && nLastToken != RTF_PAR )
            {
                NewCellRow( pInfo );
                nLastToken = pInfo->nToken;
            }
        }
        break;
        case RTF_CELL:          // denotes the end of a cell.
        {
            DBG_ASSERT( pActDefault, "RTF_CELL: pActDefault==0" );
            if ( bNewDef || !pActDefault )
                NewCellRow( pInfo );    // davor war kein \intbl, bad behavior
            // rottes RTF? retten was zu retten ist
            if ( !pActDefault )
                pActDefault = pInsDefault;
            if ( pActDefault->nColOverlap > 0 )
            {   // nicht merged mit vorheriger
                pActEntry->nCol = pActDefault->nCol;
                pActEntry->nColOverlap = pActDefault->nColOverlap;
                pActEntry->nTwips = pActDefault->nTwips;
                pActEntry->nRow = nRowCnt;
                pActEntry->aItemSet.Set( pActDefault->aItemSet );
                EntryEnd( pActEntry, pInfo->aSelection );

                if ( nStartAdjust == (sal_uLong)~0 )
                    nStartAdjust = pList->Count();
                pList->Insert( pActEntry, LIST_APPEND );
                NewActEntry( pActEntry );   // neuer freifliegender pActEntry
            }
            else
            {   // aktuelle Twips der MergeCell zuweisen
                if ( (pE = pList->Last()) != 0 )
                    pE->nTwips = pActDefault->nTwips;
                // Selection des freifliegenden pActEntry anpassen
                // Paragraph -1 wg. Textaufbruch in EditEngine waehrend Parse
                pActEntry->aSel.nStartPara = pInfo->aSelection.nEndPara - 1;
            }
            pActDefault = pDefaultList->Next();
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_ROW:           // means the end of a row
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
                pList->Insert( pActEntry, LIST_APPEND );
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




