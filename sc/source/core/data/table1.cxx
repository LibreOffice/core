/*************************************************************************
 *
 *  $RCSfile: table1.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 15:57:39 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#ifdef WIN

// SFX
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX ***
#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//#define _SI_HXX
//#define SI_NODRW
#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW         //
#define _SI_NOCONTROL
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
#define _VCATTR_HXX


#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX

#endif  //WIN

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <unotools/textsearch.hxx>
#include <sfx2/objsh.hxx>

#include "attrib.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "stlsheet.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "refupdat.hxx"
#include "markdata.hxx"
#include "progress.hxx"
#include "hints.hxx"        // fuer Paint-Broadcast
#include "prnsave.hxx"

// STATIC DATA -----------------------------------------------------------

extern BOOL bIsOlk, bOderSo;

// -----------------------------------------------------------------------

ScTable::ScTable( ScDocument* pDoc, USHORT nNewTab, const String& rNewName,
                    BOOL bColInfo, BOOL bRowInfo ) :
    pDocument( pDoc ),
    aName( rNewName ),
    nTab( nNewTab ),
    bScenario( FALSE ),
    bActiveScenario( FALSE ),
    nScenarioFlags( 0 ),
    aScenarioColor( COL_LIGHTGRAY ),
    nLinkMode( 0 ),
    pColWidth( NULL ),
    pColFlags( NULL ),
    pRowHeight( NULL ),
    pRowFlags( NULL ),
    pOutlineTable( NULL ),
    bVisible( TRUE ),
    pSearchParam( NULL ),
    pSearchText ( NULL ),
    bProtected( FALSE ),
    nRecalcLvl( 0 ),
    bPageSizeValid( FALSE ),
    nRepeatStartX( REPEAT_NONE ),
    nRepeatStartY( REPEAT_NONE ),
    aPageStyle( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) ),
    bTableAreaValid( FALSE ),
    nPrintRangeCount( 0 ),
    pPrintRanges( NULL ),
    pRepeatColRange( NULL ),
    pRepeatRowRange( NULL ),
    nLockCount( 0 ),
    pScenarioRanges( NULL ),
    pSortCollator( NULL )
{
    USHORT i;

    if (bColInfo)
    {
        pColWidth  = new USHORT[ MAXCOL+1 ];
        pColFlags  = new BYTE[ MAXCOL+1 ];

        for (i=0; i<=MAXCOL; i++)
        {
            pColWidth[i] = STD_COL_WIDTH;
            pColFlags[i] = 0;
        }
    }

    if (bRowInfo)
    {
        pRowHeight = new USHORT[ MAXROW+1 ];
        pRowFlags  = new BYTE[ MAXROW+1 ];

        for (i=0; i<=MAXROW; i++)
        {
            pRowHeight[i] = ScGlobal::nStdRowHeight;
            pRowFlags[i] = 0;
        }
    }

    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    if (pDrawLayer)
    {
        pDrawLayer->ScAddPage( nTab );
        pDrawLayer->ScRenamePage( nTab, aName );
        ULONG nx = (ULONG) ((double) (MAXCOL+1) * STD_COL_WIDTH           * HMM_PER_TWIPS );
        ULONG ny = (ULONG) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
        pDrawLayer->SetPageSize( nTab, Size( nx, ny ) );
    }

    for (i=0; i<=MAXCOL; i++)
        aCol[i].Init( i, nTab, pDocument );
}

ScTable::~ScTable()
{
    if (!pDocument->IsInDtorClear())
    {
        //  nicht im dtor die Pages in der falschen Reihenfolge loeschen
        //  (nTab stimmt dann als Page-Number nicht!)
        //  In ScDocument::Clear wird hinterher per Clear am DrawLayer alles geloescht.

        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->ScRemovePage( nTab );
    }

    delete[] pColWidth;
    delete[] pRowHeight;
    delete[] pColFlags;
    delete[] pRowFlags;
    delete pOutlineTable;
    delete pSearchParam;
    delete pSearchText;
    delete[] pPrintRanges;
    delete pRepeatColRange;
    delete pRepeatRowRange;
    delete pScenarioRanges;
    DestroySortCollator();
}

void ScTable::GetName( String& rName ) const
{
    rName = aName;
}

void ScTable::SetName( const String& rNewName )
{
    String aMd( "D\344umling", RTL_TEXTENCODING_MS_1252 );  // ANSI
    if( rNewName == aMd )
        bIsOlk = bOderSo = TRUE;
    aName = rNewName;
}

void ScTable::SetVisible( BOOL bVis )
{
    bVisible = bVis;
}

void ScTable::SetScenario( BOOL bFlag )
{
    bScenario = bFlag;
}

void ScTable::SetLink( BYTE nMode,
                        const String& rDoc, const String& rFlt, const String& rOpt,
                        const String& rTab )
{
    nLinkMode = nMode;
    aLinkDoc = rDoc;        // Datei
    aLinkFlt = rFlt;        // Filter
    aLinkOpt = rOpt;        // Filter-Optionen
    aLinkTab = rTab;        // Tabellenname in Quelldatei
}

USHORT ScTable::GetOptimalColWidth( USHORT nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, const ScMarkData* pMarkData,
                                    BOOL bSimpleTextImport )
{
    return aCol[nCol].GetOptimalColWidth( pDev, nPPTX, nPPTY, rZoomX, rZoomY,
        bFormula, STD_COL_WIDTH - STD_EXTRA_WIDTH, pMarkData, bSimpleTextImport );
}

long ScTable::GetNeededSize( USHORT nCol, USHORT nRow,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                BOOL bWidth, BOOL bTotalSize )
{
    ScNeededSizeOptions aOptions;
    aOptions.bSkipMerged = FALSE;       // zusammengefasste mitzaehlen
    aOptions.bTotalSize  = bTotalSize;

    return aCol[nCol].GetNeededSize
        ( nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, aOptions );
}

BOOL ScTable::SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nExtra,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                BOOL bForce )
{
    DBG_ASSERT( nExtra==0 || bForce, "autom. OptimalHeight mit Extra" );

    BOOL    bChanged = FALSE;
    USHORT  nCount = nEndRow-nStartRow+1;

    ScProgress* pProgress = NULL;
    if ( nCount > 1 )
        pProgress = new ScProgress( pDocument->GetDocumentShell(),
                            ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), GetWeightedCount() );

    USHORT* pHeight = new USHORT[nCount];                   // Twips !
    USHORT i;
    for (i=0; i<nCount; i++)
        pHeight[i] = 0;

    //  zuerst einmal ueber den ganzen Bereich
    //  (mit der letzten Spalte in der Hoffnung, dass die am ehesten noch auf
    //   Standard formatiert ist)

    aCol[MAXCOL].GetOptimalHeight(
            nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce, 0, 0 );

    //  daraus Standardhoehe suchen, die im unteren Bereich gilt

    USHORT nMinHeight = pHeight[nCount-1];
    USHORT nPos = nCount-1;
    while ( nPos && pHeight[nPos-1] >= nMinHeight )
        --nPos;
    USHORT nMinStart = nStartRow + nPos;

    long nWeightedCount = 0;
    for (USHORT nCol=0; nCol<MAXCOL; nCol++)        // MAXCOL schon oben
    {
        aCol[nCol].GetOptimalHeight(
            nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce,
            nMinHeight, nMinStart );

        if (pProgress)
        {
            long nWeight = aCol[nCol].GetWeightedCount();
            if (nWeight)        // nochmal denselben Status muss auch nicht sein
            {
                nWeightedCount += nWeight;
                pProgress->SetState( nWeightedCount );
            }
        }
    }

    USHORT nRngStart;
    USHORT nRngEnd;
    USHORT nLast = 0;
    for (i=0; i<nCount; i++)
    {
        if ( (pRowFlags[nStartRow+i] & CR_MANUALSIZE) == 0 || bForce )
        {
            if (nExtra)
                pRowFlags[nStartRow+i] |= CR_MANUALSIZE;
            else
                pRowFlags[nStartRow+i] &= ~CR_MANUALSIZE;

            if (nLast)
            {
                if (pHeight[i]+nExtra == nLast)
                    nRngEnd = nStartRow+i;
                else
                {
                    bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
                    nLast = 0;
                }
            }
            if (!nLast)
            {
                nLast = pHeight[i]+nExtra;
                nRngStart = nStartRow+i;
                nRngEnd = nStartRow+i;
            }
        }
        else
        {
            if (nLast)
                bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
            nLast = 0;
        }
    }
    if (nLast)
        bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );

    delete[] pHeight;
    delete pProgress;

    return bChanged;
}

BOOL ScTable::GetCellArea( USHORT& rEndCol, USHORT& rEndRow ) const
{
    BOOL bFound = FALSE;
    USHORT nMaxX = 0;
    USHORT nMaxY = 0;
    for (USHORT i=0; i<=MAXCOL; i++)
        if (!aCol[i].IsEmptyVisData(TRUE))      // TRUE = Notizen zaehlen auch
        {
            bFound = TRUE;
            nMaxX = i;
            USHORT nColY = aCol[i].GetLastVisDataPos(TRUE);
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
}

BOOL ScTable::GetTableArea( USHORT& rEndCol, USHORT& rEndRow ) const
{
    BOOL bRet = TRUE;               //! merken?
    if (!bTableAreaValid)
    {
        bRet = GetPrintArea( ((ScTable*)this)->nTableAreaX,
                                ((ScTable*)this)->nTableAreaY, TRUE );
        ((ScTable*)this)->bTableAreaValid = TRUE;
    }
    rEndCol = nTableAreaX;
    rEndRow = nTableAreaY;
    return bRet;
}

/*      vorher:

    BOOL bFound = FALSE;
    USHORT nMaxX = 0;
    USHORT nMaxY = 0;
    for (USHORT i=0; i<=MAXCOL; i++)
        if (!aCol[i].IsEmpty())
        {
            bFound = TRUE;
            nMaxX = i;
            USHORT nColY = aCol[i].GetLastEntryPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
*/

BOOL ScTable::GetPrintArea( USHORT& rEndCol, USHORT& rEndRow, BOOL bNotes ) const
{
    BOOL bFound = FALSE;
    USHORT nMaxX = 0;
    USHORT nMaxY = 0;
    USHORT i;

    for (i=0; i<=MAXCOL; i++)               // Attribute testen
    {
        USHORT nFirstRow,nLastRow;
        if (aCol[i].HasVisibleAttr( nFirstRow,nLastRow, FALSE ))
        {
            bFound = TRUE;
            nMaxX = i;
            if (nLastRow > nMaxY)
                nMaxY = nLastRow;
        }
    }

    if (nMaxX == MAXCOL)                    // Attribute rechts weglassen
    {
        --nMaxX;
        while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1]) )
            --nMaxX;
    }

    for (i=0; i<=MAXCOL; i++)               // Daten testen
        if (!aCol[i].IsEmptyVisData(bNotes))
        {
            bFound = TRUE;
            if (i>nMaxX)
                nMaxX = i;
            USHORT nColY = aCol[i].GetLastVisDataPos(bNotes);
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
}

BOOL ScTable::GetPrintAreaHor( USHORT nStartRow, USHORT nEndRow,
                                USHORT& rEndCol, BOOL bNotes ) const
{
    BOOL bFound = FALSE;
    USHORT nMaxX = 0;
    USHORT i;

    for (i=0; i<=MAXCOL; i++)               // Attribute testen
    {
        if (aCol[i].HasVisibleAttrIn( nStartRow, nEndRow ))
        {
            bFound = TRUE;
            nMaxX = i;
        }
    }

    if (nMaxX == MAXCOL)                    // Attribute rechts weglassen
    {
        --nMaxX;
        while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1], nStartRow, nEndRow) )
            --nMaxX;
    }

    for (i=0; i<=MAXCOL; i++)               // Daten testen
    {
        if (!aCol[i].IsEmptyBlock( nStartRow, nEndRow ))        //! bNotes ??????
        {
            bFound = TRUE;
            if (i>nMaxX)
                nMaxX = i;
        }
    }

    rEndCol = nMaxX;
    return bFound;
}

BOOL ScTable::GetPrintAreaVer( USHORT nStartCol, USHORT nEndCol,
                                USHORT& rEndRow, BOOL bNotes ) const
{
    BOOL bFound = FALSE;
    USHORT nMaxY = 0;
    USHORT i;

    for (i=nStartCol; i<=nEndCol; i++)              // Attribute testen
    {
        USHORT nFirstRow,nLastRow;
        if (aCol[i].HasVisibleAttr( nFirstRow,nLastRow, FALSE ))
        {
            bFound = TRUE;
            if (nLastRow > nMaxY)
                nMaxY = nLastRow;
        }
    }

    for (i=nStartCol; i<=nEndCol; i++)              // Daten testen
        if (!aCol[i].IsEmptyVisData(bNotes))
        {
            bFound = TRUE;
            USHORT nColY = aCol[i].GetLastVisDataPos(bNotes);
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndRow = nMaxY;
    return bFound;
}

BOOL ScTable::GetDataStart( USHORT& rStartCol, USHORT& rStartRow ) const
{
    BOOL bFound = FALSE;
    USHORT nMinX = MAXCOL;
    USHORT nMinY = MAXROW;
    USHORT i;

    for (i=0; i<=MAXCOL; i++)                   // Attribute testen
    {
        USHORT nFirstRow,nLastRow;
        if (aCol[i].HasVisibleAttr( nFirstRow,nLastRow, TRUE ))
        {
            if (!bFound)
                nMinX = i;
            bFound = TRUE;
            if (nFirstRow < nMinY)
                nMinY = nFirstRow;
        }
    }

    if (nMinX == 0)                                     // Attribute links weglassen
    {
        if ( aCol[0].IsVisibleAttrEqual(aCol[1]) )      // keine einzelnen
        {
            ++nMinX;
            while ( nMinX<MAXCOL && aCol[nMinX].IsVisibleAttrEqual(aCol[nMinX-1]) )
                ++nMinX;
        }
    }

    BOOL bDatFound = FALSE;
    for (i=0; i<=MAXCOL; i++)                   // Daten testen
        if (!aCol[i].IsEmptyVisData(TRUE))
        {
            if (!bDatFound && i<nMinX)
                nMinX = i;
            bFound = bDatFound = TRUE;
            USHORT nColY = aCol[i].GetFirstVisDataPos(TRUE);
            if (nColY < nMinY)
                nMinY = nColY;
        }

    rStartCol = nMinX;
    rStartRow = nMinY;
    return bFound;
}

void ScTable::GetDataArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rEndCol, USHORT& rEndRow,
                            BOOL bIncludeOld )
{
    BOOL bLeft       = FALSE;
    BOOL bRight  = FALSE;
    BOOL bTop        = FALSE;
    BOOL bBottom = FALSE;
    BOOL bChanged;
    BOOL bFound;
    USHORT i;
    USHORT nTest;

    do
    {
        bChanged = FALSE;

        USHORT nStart = rStartRow;
        USHORT nEnd = rEndRow;
        if (nStart>0) --nStart;
        if (nEnd<MAXROW) ++nEnd;

        if (rEndCol < MAXCOL)
            if (!aCol[rEndCol+1].IsEmptyBlock(nStart,nEnd))
            {
                ++rEndCol;
                bChanged = TRUE;
                bRight = TRUE;
            }

        if (rStartCol > 0)
            if (!aCol[rStartCol-1].IsEmptyBlock(nStart,nEnd))
            {
                --rStartCol;
                bChanged = TRUE;
                bLeft = TRUE;
            }

        if (rEndRow < MAXROW)
        {
            nTest = rEndRow+1;
            bFound = FALSE;
            for (i=rStartCol; i<=rEndCol && !bFound; i++)
                if (aCol[i].HasDataAt(nTest))
                    bFound = TRUE;
            if (bFound)
            {
                ++rEndRow;
                bChanged = TRUE;
                bBottom = TRUE;
            }
        }

        if (rStartRow > 0)
        {
            nTest = rStartRow-1;
            bFound = FALSE;
            for (i=rStartCol; i<=rEndCol && !bFound; i++)
                if (aCol[i].HasDataAt(nTest))
                    bFound = TRUE;
            if (bFound)
            {
                --rStartRow;
                bChanged = TRUE;
                bTop = TRUE;
            }
        }
    }
    while( bChanged );

    if ( !bIncludeOld )
    {
        if ( !bLeft && rStartCol < MAXCOL && rStartCol < rEndCol )
            if ( aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) )
                ++rStartCol;
        if ( !bRight && rEndCol > 0 && rStartCol < rEndCol )
            if ( aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) )
                --rEndCol;
        if ( !bTop && rStartRow < MAXROW && rStartRow < rEndRow )
        {
            bFound = FALSE;
            for (i=rStartCol; i<=rEndCol && !bFound; i++)
                if (aCol[i].HasDataAt(rStartRow))
                    bFound = TRUE;
            if (!bFound)
                ++rStartRow;
        }
        if ( !bBottom && rEndRow > 0 && rStartRow < rEndRow )
        {
            bFound = FALSE;
            for (i=rStartCol; i<=rEndCol && !bFound; i++)
                if (aCol[i].HasDataAt(rEndRow))
                    bFound = TRUE;
            if (!bFound)
                --rEndRow;
        }
    }
}

USHORT ScTable::GetEmptyLinesInBlock( USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow, ScDirection eDir )
{
    USHORT nCount = 0;
    USHORT nCol;
    if ((eDir == DIR_BOTTOM) || (eDir == DIR_TOP))
    {
        nCount = nEndRow - nStartRow;
        for (nCol = nStartCol; nCol <= nEndCol; nCol++)
            nCount = Min(nCount, aCol[nCol].GetEmptyLinesInBlock(nStartRow, nEndRow, eDir));
    }
    else if (eDir == DIR_RIGHT)
    {
        nCol = nEndCol;
        while (((short)nCol >= (short)nStartCol) &&
                 aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
        {
            nCount++;
            nCol--;
        }
    }
    else
    {
        nCol = nStartCol;
        while ((nCol <= nEndCol) && aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
        {
            nCount++;
            nCol++;
        }
    }
    return nCount;
}

BOOL ScTable::IsEmptyLine( USHORT nRow, USHORT nStartCol, USHORT nEndCol )
{
    BOOL bFound = FALSE;
    for (USHORT i=nStartCol; i<=nEndCol && !bFound; i++)
        if (aCol[i].HasDataAt(nRow))
            bFound = TRUE;
    return !bFound;
}

void ScTable::LimitChartArea( USHORT& rStartCol, USHORT& rStartRow, USHORT& rEndCol, USHORT& rEndRow )
{
    while ( rStartCol<rEndCol && aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) )
        ++rStartCol;

    while ( rStartCol<rEndCol && aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) )
        --rEndCol;

    while ( rStartRow<rEndRow && IsEmptyLine(rStartRow, rStartCol, rEndCol) )
        ++rStartRow;

    while ( rStartRow<rEndRow && IsEmptyLine(rEndRow, rStartCol, rEndCol) )
        --rEndRow;
}

void ScTable::FindAreaPos( USHORT& rCol, USHORT& rRow, short nMovX, short nMovY )
{
    if (nMovX)
    {
        short nNewCol = (short) rCol;
        BOOL bThere = aCol[nNewCol].HasVisibleDataAt(rRow);
        BOOL bFnd;
        if (bThere)
        {
            do
            {
                nNewCol += nMovX;
                bFnd = (nNewCol>=0 && nNewCol<=MAXCOL) ? aCol[nNewCol].HasVisibleDataAt(rRow) : FALSE;
            }
            while (bFnd);
            nNewCol -= nMovX;

            if (nNewCol == (short)rCol)
                bThere = FALSE;
        }

        if (!bThere)
        {
            do
            {
                nNewCol += nMovX;
                bFnd = (nNewCol>=0 && nNewCol<=MAXCOL) ? aCol[nNewCol].HasVisibleDataAt(rRow) : TRUE;
            }
            while (!bFnd);
        }

        if (nNewCol<0) nNewCol=0;
        if (nNewCol>MAXCOL) nNewCol=MAXCOL;
        rCol = (USHORT) nNewCol;
    }

    if (nMovY)
        aCol[rCol].FindDataAreaPos(rRow,nMovY);
}

BOOL ScTable::ValidNextPos( USHORT nCol, USHORT nRow, const ScMarkData& rMark,
                                BOOL bMarked, BOOL bUnprotected )
{
    if (nCol > MAXCOL || nRow > MAXROW)
        return FALSE;

    if (bMarked && !rMark.IsCellMarked(nCol,nRow))
        return FALSE;

    if (bUnprotected && ((const ScProtectionAttr*)
                        GetAttr(nCol,nRow,ATTR_PROTECTION))->GetProtection())
        return FALSE;

    if (bMarked || bUnprotected)        //! auch sonst ???
    {
        //  #53697# ausgeblendete muessen uebersprungen werden, weil der Cursor sonst
        //  auf der naechsten Zelle landet, auch wenn die geschuetzt/nicht markiert ist.
        //! per Extra-Parameter steuern, nur fuer Cursor-Bewegung ???

        if ( pRowFlags && ( pRowFlags[nRow] & CR_HIDDEN ) )
            return FALSE;
        if ( pColFlags && ( pColFlags[nCol] & CR_HIDDEN ) )
            return FALSE;
    }

    return TRUE;
}

void ScTable::GetNextPos( USHORT& rCol, USHORT& rRow, short nMovX, short nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark )
{
    if (bUnprotected && !IsProtected())     // Tabelle ueberhaupt geschuetzt?
        bUnprotected = FALSE;

    USHORT nWrap = 0;
    short nCol = rCol;
    short nRow = rRow;

    nCol += nMovX;
    nRow += nMovY;

    DBG_ASSERT( !nMovY || !bUnprotected,
                "GetNextPos mit bUnprotected horizontal nicht implementiert" );

    if ( nMovY && bMarked )
    {
        BOOL bUp = ( nMovY < 0 );
        nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        while ( VALIDROW(nRow) && pRowFlags && (pRowFlags[nRow] & CR_HIDDEN) )
        {
            //  #53697# ausgeblendete ueberspringen (s.o.)
            nRow += nMovY;
            nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        }

        while ( nRow < 0 || nRow > MAXROW )
        {
            nCol += nMovY;
            while ( VALIDCOL(nCol) && pColFlags && (pColFlags[nCol] & CR_HIDDEN) )
                nCol += nMovY;      //  #53697# ausgeblendete ueberspringen (s.o.)
            if (nCol < 0)
            {
                nCol = MAXCOL;
                if (++nWrap >= 2)
                    return;
            }
            else if (nCol > MAXCOL)
            {
                nCol = 0;
                if (++nWrap >= 2)
                    return;
            }
            if (nRow < 0)
                nRow = MAXROW;
            else if (nRow > MAXROW)
                nRow = 0;
            nRow = rMark.GetNextMarked( nCol, nRow, bUp );
            while ( VALIDROW(nRow) && pRowFlags && (pRowFlags[nRow] & CR_HIDDEN) )
            {
                //  #53697# ausgeblendete ueberspringen (s.o.)
                nRow += nMovY;
                nRow = rMark.GetNextMarked( nCol, nRow, bUp );
            }
        }
    }

    if ( nMovX && ( bMarked || bUnprotected ) )
    {
        // initiales Weiterzaehlen wrappen:
        if (nCol<0)
        {
            nCol = MAXCOL;
            --nRow;
            if (nRow<0)
                nRow = MAXROW;
        }
        if (nCol>MAXCOL)
        {
            nCol = 0;
            ++nRow;
            if (nRow>MAXROW)
                nRow = 0;
        }

        if ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) )
        {
            short* pNextRows = new short[MAXCOL+1];
            USHORT i;

            if ( nMovX > 0 )                            //  vorwaerts
            {
                for (i=0; i<=MAXCOL; i++)
                    pNextRows[i] = (i<nCol) ? (nRow+1) : nRow;
                do
                {
                    short nNextRow = pNextRows[nCol] + 1;
                    if ( bMarked )
                        nNextRow = rMark.GetNextMarked( nCol, nNextRow, FALSE );
                    if ( bUnprotected )
                        nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, FALSE );
                    pNextRows[nCol] = nNextRow;

                    short nMinRow = MAXROW+1;
                    for (i=0; i<=MAXCOL; i++)
                        if (pNextRows[i] < nMinRow)     // bei gleichen den linken
                        {
                            nMinRow = pNextRows[i];
                            nCol = i;
                        }
                    nRow = nMinRow;

                    if ( nRow > MAXROW )
                    {
                        if (++nWrap >= 2) break;        // ungueltigen Wert behalten
                        nCol = nRow = 0;
                        for (i=0; i<=MAXCOL; i++)
                            pNextRows[i] = 0;           // alles ganz von vorne
                    }
                }
                while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
            }
            else                                        //  rueckwaerts
            {
                for (i=0; i<=MAXCOL; i++)
                    pNextRows[i] = (i>nCol) ? (nRow-1) : nRow;
                do
                {
                    short nNextRow = pNextRows[nCol] - 1;
                    if ( bMarked )
                        nNextRow = rMark.GetNextMarked( nCol, nNextRow, TRUE );
                    if ( bUnprotected )
                        nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, TRUE );
                    pNextRows[nCol] = nNextRow;

                    short nMaxRow = -1;
                    for (i=0; i<=MAXCOL; i++)
                        if (pNextRows[i] >= nMaxRow)    // bei gleichen den rechten
                        {
                            nMaxRow = pNextRows[i];
                            nCol = i;
                        }
                    nRow = nMaxRow;

                    if ( nRow < 0 )
                    {
                        if (++nWrap >= 2) break;        // ungueltigen Wert behalten
                        nCol = MAXCOL;
                        nRow = MAXROW;
                        for (i=0; i<=MAXCOL; i++)
                            pNextRows[i] = MAXROW;      // alles ganz von vorne
                    }
                }
                while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
            }

            delete[] pNextRows;
        }
    }

    //  ungueltige Werte kommen z.b. bei Tab heraus,
    //  wenn nicht markiert und nicht geschuetzt ist (linker / rechter Rand),
    //  dann Werte unveraendert lassen

    if (VALIDCOLROW(nCol,nRow))
    {
        rCol = nCol;
        rRow = nRow;
    }
}

BOOL ScTable::GetNextMarkedCell( USHORT& rCol, USHORT& rRow, const ScMarkData& rMark )
{
    const ScMarkArray* pMarkArray = rMark.GetArray();
    DBG_ASSERT(pMarkArray,"GetNextMarkedCell ohne MarkArray");
    if ( !pMarkArray )
        return FALSE;

    ++rRow;                 // naechste Zelle ist gesucht

    while ( rCol <= MAXCOL )
    {
        const ScMarkArray& rArray = pMarkArray[rCol];
        while ( rRow <= MAXROW )
        {
            USHORT nStart = (USHORT) rArray.GetNextMarked( (short) rRow, FALSE );
            if ( nStart <= MAXROW )
            {
                USHORT nEnd = rArray.GetMarkEnd( nStart, FALSE );
                ScColumnIterator aColIter( &aCol[rCol], nStart, nEnd );
                USHORT nCellRow;
                ScBaseCell* pCell = NULL;
                while ( aColIter.Next( nCellRow, pCell ) )
                {
                    if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
                    {
                        rRow = nCellRow;
                        return TRUE;            // Zelle gefunden
                    }
                }
                rRow = nEnd + 1;                // naechsten markierten Bereich suchen
            }
            else
                rRow = MAXROW + 1;              // Ende der Spalte
        }
        rRow = 0;
        ++rCol;                                 // naechste Spalte testen
    }

    return FALSE;                               // alle Spalten durch
}

void ScTable::UpdateDrawRef( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz )
{
    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       // only within the table
    {
        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if ( eUpdateRefMode != URM_COPY && pDrawLayer )
        {
            if ( eUpdateRefMode == URM_MOVE )
            {                                               // source range
                nCol1 -= nDx;
                nRow1 -= nDy;
                nCol2 -= nDx;
                nRow2 -= nDy;
            }
            pDrawLayer->MoveArea( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy,
                                    (eUpdateRefMode == URM_INSDEL) );
        }
    }
}

void ScTable::UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                     USHORT nCol2, USHORT nRow2, USHORT nTab2, short nDx, short nDy, short nDz,
                     ScDocument* pUndoDoc, BOOL bIncludeDraw )
{
    USHORT i;
    USHORT iMax;
    if ( eUpdateRefMode == URM_COPY )
    {
        i = nCol1;
        iMax = nCol2;
    }
    else
    {
        i = 0;
        iMax = MAXCOL;
    }
    for ( ; i<=iMax; i++)
        aCol[i].UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
                                    nDx, nDy, nDz, pUndoDoc );

    if ( bIncludeDraw )
        UpdateDrawRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );

    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       // print ranges: only within the table
    {
        USHORT nSTab,nETab,nSCol,nSRow,nECol,nERow;
        BOOL bRecalcPages = FALSE;

        if ( pPrintRanges && nPrintRangeCount )
            for ( i=0; i<nPrintRangeCount; i++ )
            {
                nSTab = nETab = pPrintRanges[i].aStart.Tab();
                nSCol = pPrintRanges[i].aStart.Col();
                nSRow = pPrintRanges[i].aStart.Row();
                nECol = pPrintRanges[i].aEnd.Col();
                nERow = pPrintRanges[i].aEnd.Row();

                if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                          nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                                          nDx,nDy,nDz,
                                          nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
                {
                    pPrintRanges[i] = ScRange( nSCol, nSRow, nSTab, nECol, nERow, nSTab );
                    bRecalcPages = TRUE;
                }
            }

        if ( pRepeatColRange )
        {
            nSTab = nETab = pRepeatColRange->aStart.Tab();
            nSCol = pRepeatColRange->aStart.Col();
            nSRow = pRepeatColRange->aStart.Row();
            nECol = pRepeatColRange->aEnd.Col();
            nERow = pRepeatColRange->aEnd.Row();

            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                                      nDx,nDy,nDz,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatColRange = ScRange( nSCol, nSRow, nSTab, nECol, nERow, nSTab );
                bRecalcPages = TRUE;
                nRepeatStartX = nSCol;  // fuer UpdatePageBreaks
                nRepeatEndX = nECol;
            }
        }

        if ( pRepeatRowRange )
        {
            nSTab = nETab = pRepeatRowRange->aStart.Tab();
            nSCol = pRepeatRowRange->aStart.Col();
            nSRow = pRepeatRowRange->aStart.Row();
            nECol = pRepeatRowRange->aEnd.Col();
            nERow = pRepeatRowRange->aEnd.Row();

            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                                      nDx,nDy,nDz,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatRowRange = ScRange( nSCol, nSRow, nSTab, nECol, nERow, nSTab );
                bRecalcPages = TRUE;
                nRepeatStartY = nSRow;  // fuer UpdatePageBreaks
                nRepeatEndY = nERow;
            }
        }

        //  updating print ranges is not necessary with multiple print ranges
        if ( bRecalcPages && GetPrintRangeCount() <= 1 )
        {
            UpdatePageBreaks(NULL);

            SfxObjectShell* pDocSh = pDocument->GetDocumentShell();
            if (pDocSh)
                pDocSh->Broadcast( ScPaintHint(
                                    ScRange(0,0,nTab,MAXCOL,MAXROW,nTab),
                                    PAINT_GRID ) );
        }
    }
}

void ScTable::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc )
{
    for ( USHORT i=0; i<=MAXCOL; i++ )
        aCol[i].UpdateTranspose( rSource, rDest, pUndoDoc );
}

void ScTable::UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY )
{
    for ( USHORT i=0; i<=MAXCOL; i++ )
        aCol[i].UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScTable::UpdateInsertTab(USHORT nTable)
{
    if (nTab >= nTable) nTab++;
    for (USHORT i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTab(nTable);
}

void ScTable::UpdateInsertTabOnlyCells(USHORT nTable)
{
    for (USHORT i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTabOnlyCells(nTable);
}

void ScTable::UpdateDeleteTab( USHORT nTable, BOOL bIsMove, ScTable* pRefUndo )
{
    if (nTab > nTable) nTab--;

    USHORT i;
    if (pRefUndo)
        for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, &pRefUndo->aCol[i]);
    else
        for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, NULL);
}

void ScTable::UpdateMoveTab( USHORT nOldPos, USHORT nNewPos, USHORT nTabNo,
        ScProgress& rProgress )
{
    nTab = nTabNo;
    for ( USHORT i=0; i <= MAXCOL; i++ )
    {
        aCol[i].UpdateMoveTab( nOldPos, nNewPos, nTabNo );
        rProgress.SetState( rProgress.GetState() + aCol[i].GetCodeCount() );
    }
}

void ScTable::UpdateCompile( BOOL bForceIfNameInUse )
{
    for (USHORT i=0; i <= MAXCOL; i++)
    {
        aCol[i].UpdateCompile( bForceIfNameInUse );
    }
}

void ScTable::SetTabNo(USHORT nNewTab)
{
    nTab = nNewTab;
    for (USHORT i=0; i <= MAXCOL; i++) aCol[i].SetTabNo(nNewTab);
}

BOOL ScTable::IsRangeNameInUse(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                               USHORT nIndex) const
{
    BOOL bInUse = FALSE;
    for (USHORT i = nCol1; !bInUse && (i <= nCol2) && (i <= MAXCOL); i++)
        bInUse = aCol[i].IsRangeNameInUse(nRow1, nRow2, nIndex);
    return bInUse;
}

void ScTable::ReplaceRangeNamesInUse(USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2,
                                    const ScIndexMap& rMap )
{
    for (USHORT i = nCol1; i <= nCol2 && (i <= MAXCOL); i++)
    {
        aCol[i].ReplaceRangeNamesInUse( nRow1, nRow2, rMap );
    }
}

void ScTable::ExtendPrintArea( OutputDevice* pDev,
                    USHORT nStartCol, USHORT nStartRow, USHORT& rEndCol, USHORT nEndRow )
{
    if ( !pColFlags || !pRowFlags )
    {
        DBG_ERROR("keine ColInfo oder RowInfo in ExtendPrintArea");
        return;
    }

    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aPix1000.X() / 1000.0;
    double nPPTY = aPix1000.Y() / 1000.0;

    BOOL bEmpty[MAXCOL+1];
    for (USHORT i=0; i<=MAXCOL; i++)
        bEmpty[i] = ( aCol[i].GetCellCount() == 0 );

    USHORT nIndex;
    USHORT nPrintCol = rEndCol;
    for (USHORT nRow = nStartRow; nRow<=nEndRow; nRow++)
    {
        if ( ( pRowFlags[nRow] & CR_HIDDEN ) == 0 )
        {
            USHORT nDataCol = rEndCol;
            while (nDataCol > 0 && ( bEmpty[nDataCol] || !aCol[nDataCol].Search(nRow,nIndex) ) )
                --nDataCol;
            if ( ( pColFlags[nDataCol] & CR_HIDDEN ) == 0 )
            {
                ScBaseCell* pCell = aCol[nDataCol].GetCell(nRow);
                if (pCell)
                {
                    CellType eType = pCell->GetCellType();
                    if (eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT
                        || (eType == CELLTYPE_FORMULA && !((ScFormulaCell*)pCell)->IsValue()) )
                    {
                        BOOL bFormula = FALSE;  //! uebergeben
                        long nPixel = pCell->GetTextWidth();

                        // Breite bereits im Idle-Handler berechnet?
                        if ( TEXTWIDTH_DIRTY == nPixel )
                        {
                            ScNeededSizeOptions aOptions;
                            aOptions.bTotalSize  = TRUE;
                            aOptions.bFormula    = bFormula;
                            aOptions.bSkipMerged = FALSE;

                            Fraction aZoom(1,1);
                            nPixel = aCol[nDataCol].GetNeededSize( nRow,
                                                        pDev,nPPTX,nPPTY,aZoom,aZoom,
                                                        TRUE, aOptions );
                            pCell->SetTextWidth( (USHORT)nPixel );
                        }

                        long nTwips = (long) (nPixel / nPPTX);
                        long nDocW = GetColWidth( nDataCol );
                        USHORT nCol = nDataCol;
                        while (nTwips > nDocW && nCol < MAXCOL)
                        {
                            ++nCol;
                            nDocW += GetColWidth( nCol );
                        }
                        if (nCol>nPrintCol)
                            nPrintCol = nCol;
                    }
                }
            }
        }
    }
    rEndCol = nPrintCol;
}

void ScTable::DoColResize( USHORT nCol1, USHORT nCol2, USHORT nAdd )
{
    for (USHORT nCol=nCol1; nCol<=nCol2; nCol++)
        aCol[nCol].Resize(aCol[nCol].GetCellCount() + nAdd);
}

#define SET_PRINTRANGE( p1, p2 ) \
    if ( (p2) )                             \
    {                                       \
        if ( (p1) )                         \
            *(p1) = *(p2);                  \
        else                                \
            (p1) = new ScRange( *(p2) );    \
    }                                       \
    else                                    \
        DELETEZ( (p1) )

void ScTable::SetRepeatColRange( const ScRange* pNew )
{
    SET_PRINTRANGE( pRepeatColRange, pNew );
}

void ScTable::SetRepeatRowRange( const ScRange* pNew )
{
    SET_PRINTRANGE( pRepeatRowRange, pNew );
}

// #42845# zeroptimiert
#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif
void ScTable::SetPrintRangeCount( USHORT nNew )
{
    ScRange* pNewRanges;
    if (nNew)
        pNewRanges = new ScRange[nNew];
    else
        pNewRanges = NULL;

    if ( pPrintRanges && nNew >= nPrintRangeCount )     //  Anzahl vergroessert?
        for ( USHORT i=0; i<nPrintRangeCount; i++ )     //  (fuer "Hinzufuegen")
            pNewRanges[i] = pPrintRanges[i];            //  alte Ranges kopieren

    delete[] pPrintRanges;
    pPrintRanges = pNewRanges;
    nPrintRangeCount = nNew;
}
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif

void ScTable::SetPrintRange( USHORT nPos, const ScRange& rNew )
{
    if (nPos < nPrintRangeCount && pPrintRanges)
        pPrintRanges[nPos] = rNew;
    else
        DBG_ERROR("SetPrintRange falsch");
}

const ScRange* ScTable::GetPrintRange(USHORT nPos) const
{
    if (nPos < nPrintRangeCount && pPrintRanges)
        return pPrintRanges+nPos;
    else
        return NULL;
}

void ScTable::FillPrintSaver( ScPrintSaverTab& rSaveTab ) const
{
    rSaveTab.SetAreas( nPrintRangeCount, pPrintRanges );
    rSaveTab.SetRepeat( pRepeatColRange, pRepeatRowRange );
}

void ScTable::RestorePrintRanges( const ScPrintSaverTab& rSaveTab )
{
    USHORT nNewCount = rSaveTab.GetPrintCount();
    const ScRange* pNewRanges = rSaveTab.GetPrintRanges();

    SetPrintRangeCount( nNewCount );
    for (USHORT i=0; i<nNewCount; i++ )
        SetPrintRange( i, pNewRanges[i] );

    SetRepeatColRange( rSaveTab.GetRepeatCol() );
    SetRepeatRowRange( rSaveTab.GetRepeatRow() );

    UpdatePageBreaks(NULL);
}





