/*************************************************************************
 *
 *  $RCSfile: rtfparse.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#include <svx/editeng.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/svxrtf.hxx>
#include <vcl/outdev.hxx>
#include <svtools/rtftoken.h>

#define SC_RTFPARSE_CXX
#include "rtfparse.hxx"
#include "global.hxx"
#include "document.hxx"
#include "docpool.hxx"

#define SC_RTFTWIPTOL 10        // 10 Twips Toleranz bei Spaltenbestimmung



SV_IMPL_VARARR_SORT( ScRTFColTwips, ULONG );



ScRTFParser::ScRTFParser( EditEngine* pEditP ) :
        ScEEParser( pEditP ),
        pDefaultList( new ScRTFDefaultList ),
        pColTwips( new ScRTFColTwips ),
        pActDefault( NULL ),
        pDefMerge( NULL ),
        nStartAdjust( (ULONG)~0 ),
        nLastWidth(0),
        bNewDef( FALSE )
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


ULONG ScRTFParser::Read( SvStream& rStream )
{
    Link aOldLink = pEdit->GetImportHdl();
    pEdit->SetImportHdl( LINK( this, ScRTFParser, RTFImportHdl ) );
    ULONG nErr = pEdit->Read( rStream, EE_FORMAT_RTF );
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


BOOL ScRTFParser::SeekTwips( USHORT nTwips, USHORT* pCol )
{
    if ( pColTwips->Seek_Entry( nTwips, pCol ) )
        return TRUE;
    USHORT nCount = pColTwips->Count();
    if ( !nCount )
        return FALSE;
    USHORT nCol = *pCol;
    // nCol ist Einfuegeposition, da liegt der Naechsthoehere (oder auch nicht)
    if ( nCol < nCount && (((*pColTwips)[nCol] - SC_RTFTWIPTOL) <= nTwips) )
        return TRUE;
    // nicht kleiner als alles andere? dann mit Naechstniedrigerem vergleichen
    else if ( nCol && (((*pColTwips)[nCol-1] + SC_RTFTWIPTOL) >= nTwips) )
    {
        (*pCol)--;
        return TRUE;
    }
    return FALSE;
}


void ScRTFParser::ColAdjust()
{
    if ( nStartAdjust != (ULONG)~0 )
    {
        USHORT nCol = 0;
        ScEEParseEntry* pE;
        pE = pList->Seek( nStartAdjust );
        while ( pE )
        {
            if ( pE->nCol == 0 )
                nCol = 0;
            pE->nCol = nCol;
            if ( pE->nColOverlap > 1 )
                nCol += pE->nColOverlap;        // merged cells mit \clmrg
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
        nStartAdjust = (ULONG)~0;
        pColTwips->Remove( (USHORT)0, pColTwips->Count() );
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
void ScRTFParser::NewCellRow( ImportInfo* pInfo )
{
    if ( bNewDef )
    {
        ScRTFCellDefault* pD;
        bNewDef = FALSE;
        // rechts nicht buendig? => neue Tabelle
        if ( nLastWidth
          && (pD = pDefaultList->Last()) && pD->nTwips != nLastWidth )
        {
            USHORT n1, n2;
            if ( !( SeekTwips( nLastWidth, &n1 )
                && SeekTwips( pD->nTwips, &n2 ) && n1 == n2) )
                ColAdjust();
        }
        // TwipCols aufbauen, erst nach nLastWidth Vergleich!
        for ( pD = pDefaultList->First(); pD; pD = pDefaultList->Next() )
        {
            USHORT n;
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
        case RTF_TROWD:         // TableROWDefault, vor erstem RTF_CELLX
        {
            if ( pD = pDefaultList->Last() )
                nLastWidth = pD->nTwips;
            nColCnt = 0;
            for ( pD = pDefaultList->First(); pD; pD = pDefaultList->Next() )
                delete pD;
            pDefaultList->Clear();
            pDefMerge = NULL;
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLMGF:         // CeLlMerGeFirst, die MergeCell
        {
            pDefMerge = pInsDefault;
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_CLMRG:         // CeLlMeRGe, Zelle mit vorheriger merged
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
        case RTF_CELLX:         // schliesst Zelldefault
        {
            bNewDef = TRUE;
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
        case RTF_INTBL:         // vor erstem RTF_CELL
        {
            // einmal ueber NextToken und einmal ueber UnknownAttrToken
            // oder z.B. \intbl ... \cell \pard \intbl ... \cell
            if ( nLastToken != RTF_INTBL && nLastToken != RTF_CELL )
            {
                NewCellRow( pInfo );
                nLastToken = pInfo->nToken;
            }
        }
        break;
        case RTF_CELL:          // schliesst Zelle
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

                if ( nStartAdjust == (ULONG)~0 )
                    nStartAdjust = pList->Count();
                pList->Insert( pActEntry, LIST_APPEND );
                NewActEntry( pActEntry );   // neuer freifliegender pActEntry
            }
            else
            {   // aktuelle Twips der MergeCell zuweisen
                if ( pE = pList->Last() )
                    pE->nTwips = pActDefault->nTwips;
                // Selection des freifliegenden pActEntry anpassen
                // Paragraph -1 wg. Textaufbruch in EditEngine waehrend Parse
                pActEntry->aSel.nStartPara = pInfo->aSelection.nEndPara - 1;
            }
            pActDefault = pDefaultList->Next();
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_ROW:           // nach letzter Zelle in Row
        {
            NextRow();
            nLastToken = pInfo->nToken;
        }
        break;
        case RTF_PAR:           // Paragraph
        {
            if ( !pActDefault )
            {   // Text nicht in Tabelle
                ColAdjust();    // bisherige Tabelle beenden
                pActEntry->nCol = 0;
                pActEntry->nRow = nRowCnt;
                EntryEnd( pActEntry, pInfo->aSelection );
                pList->Insert( pActEntry, LIST_APPEND );
                NewActEntry( pActEntry );   // neuer freifliegender pActEntry
                NextRow();
            }
            nLastToken = pInfo->nToken;
        }
        break;
        default:
        {   // nLastToken nicht setzen!
            switch ( pInfo->nToken & ~(0xff | RTF_TABLEDEF) )
            {
                case RTF_SHADINGDEF:
                    ((SvxRTFParser*)pInfo->pParser)->ReadBackgroundAttr(
                        pInfo->nToken, pInsDefault->aItemSet, TRUE );
                break;
                case RTF_BRDRDEF:
                    ((SvxRTFParser*)pInfo->pParser)->ReadBorderAttr(
                        pInfo->nToken, pInsDefault->aItemSet, TRUE );
                break;
            }
        }
    }
}




