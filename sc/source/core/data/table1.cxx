/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: table1.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:31:16 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



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
#include <svx/algitem.hxx>
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

ScTable::ScTable( ScDocument* pDoc, SCTAB nNewTab, const String& rNewName,
                    BOOL bColInfo, BOOL bRowInfo ) :
    aName( rNewName ),
    bScenario( FALSE ),
    bLayoutRTL( FALSE ),
    bLoadingRTL( FALSE ),
    nLinkMode( 0 ),
    aPageStyle( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) ),
    bPageSizeValid( FALSE ),
    nRepeatStartX( SCCOL_REPEAT_NONE ),
    nRepeatStartY( SCROW_REPEAT_NONE ),
    bProtected( FALSE ),
    pColWidth( NULL ),
    pRowHeight( NULL ),
    pColFlags( NULL ),
    pRowFlags( NULL ),
    pOutlineTable( NULL ),
    bTableAreaValid( FALSE ),
    bVisible( TRUE ),
    nTab( nNewTab ),
    nRecalcLvl( 0 ),
    pDocument( pDoc ),
    pSearchParam( NULL ),
    pSearchText ( NULL ),
    pSortCollator( NULL ),
    bPrintEntireSheet( FALSE ),
    pRepeatColRange( NULL ),
    pRepeatRowRange( NULL ),
    nLockCount( 0 ),
    pScenarioRanges( NULL ),
    aScenarioColor( COL_LIGHTGRAY ),
    nScenarioFlags( 0 ),
    bActiveScenario( FALSE )
{

    if (bColInfo)
    {
        pColWidth  = new USHORT[ MAXCOL+1 ];
        pColFlags  = new BYTE[ MAXCOL+1 ];

        for (SCCOL i=0; i<=MAXCOL; i++)
        {
            pColWidth[i] = STD_COL_WIDTH;
            pColFlags[i] = 0;
        }
    }

    if (bRowInfo)
    {
        pRowHeight = new ScSummableCompressedArray< SCROW, USHORT>( MAXROW, ScGlobal::nStdRowHeight);
        pRowFlags  = new ScBitMaskCompressedArray< SCROW, BYTE>( MAXROW, 0);
    }

    if ( pDocument->IsDocVisible() )
    {
        //  when a sheet is added to a visible document,
        //  initialize its RTL flag from the system locale
        bLayoutRTL = ScGlobal::IsSystemRTL();
    }

    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    if (pDrawLayer)
    {
        if ( pDrawLayer->ScAddPage( nTab ) )    // FALSE (not inserted) during Undo
        {
            pDrawLayer->ScRenamePage( nTab, aName );
            ULONG nx = (ULONG) ((double) (MAXCOL+1) * STD_COL_WIDTH           * HMM_PER_TWIPS );
            ULONG ny = (ULONG) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
            pDrawLayer->SetPageSize( static_cast<sal_uInt16>(nTab), Size( nx, ny ) );
        }
    }

    for (SCCOL k=0; k<=MAXCOL; k++)
        aCol[k].Init( k, nTab, pDocument );
}

ScTable::~ScTable()
{
    if (!pDocument->IsInDtorClear())
    {
        //  nicht im dtor die Pages in der falschen Reihenfolge loeschen
        //  (nTab stimmt dann als Page-Number nicht!)
        //  In ScDocument::Clear wird hinterher per Clear am Draw Layer alles geloescht.

        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->ScRemovePage( nTab );
    }

    delete[] pColWidth;
    delete[] pColFlags;
    delete pRowHeight;
    delete pRowFlags;
    delete pOutlineTable;
    delete pSearchParam;
    delete pSearchText;
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
    aUpperName.Erase();         // invalidated if the name is changed
}

const String& ScTable::GetUpperName() const
{
    if ( !aUpperName.Len() && aName.Len() )
        aUpperName = ScGlobal::pCharClass->upper( aName );
    return aUpperName;
}

void ScTable::SetVisible( BOOL bVis )
{
    bVisible = bVis;
}

void ScTable::SetLayoutRTL( BOOL bSet )
{
    bLayoutRTL = bSet;
}

void ScTable::SetLoadingRTL( BOOL bSet )
{
    bLoadingRTL = bSet;
}

void ScTable::SetScenario( BOOL bFlag )
{
    bScenario = bFlag;
}

void ScTable::SetLink( BYTE nMode,
                        const String& rDoc, const String& rFlt, const String& rOpt,
                        const String& rTab, ULONG nRefreshDelay )
{
    nLinkMode = nMode;
    aLinkDoc = rDoc;        // Datei
    aLinkFlt = rFlt;        // Filter
    aLinkOpt = rOpt;        // Filter-Optionen
    aLinkTab = rTab;        // Tabellenname in Quelldatei
    nLinkRefreshDelay = nRefreshDelay;  // refresh delay in seconds, 0==off
}

USHORT ScTable::GetOptimalColWidth( SCCOL nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bFormula, const ScMarkData* pMarkData,
                                    BOOL bSimpleTextImport )
{
    return aCol[nCol].GetOptimalColWidth( pDev, nPPTX, nPPTY, rZoomX, rZoomY,
        bFormula, STD_COL_WIDTH - STD_EXTRA_WIDTH, pMarkData, bSimpleTextImport );
}

long ScTable::GetNeededSize( SCCOL nCol, SCROW nRow,
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

BOOL ScTable::SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, USHORT nExtra,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                BOOL bForce, ScProgress* pOuterProgress, ULONG nProgressStart )
{
    DBG_ASSERT( nExtra==0 || bForce, "autom. OptimalHeight mit Extra" );

    if ( !pDocument->IsAdjustHeightEnabled() )
    {
        return FALSE;
    }

    BOOL    bChanged = FALSE;
    SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    ScProgress* pProgress = NULL;
    if ( pOuterProgress )
        pProgress = pOuterProgress;
    else if ( nCount > 1 )
        pProgress = new ScProgress( pDocument->GetDocumentShell(),
                            ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), GetWeightedCount() );

    USHORT* pHeight = new USHORT[nCount];                   // Twips !
    memset( pHeight, 0, sizeof(USHORT) * nCount );

    //  zuerst einmal ueber den ganzen Bereich
    //  (mit der letzten Spalte in der Hoffnung, dass die am ehesten noch auf
    //   Standard formatiert ist)

    aCol[MAXCOL].GetOptimalHeight(
            nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce, 0, 0 );

    //  daraus Standardhoehe suchen, die im unteren Bereich gilt

    USHORT nMinHeight = pHeight[nCount-1];
    SCSIZE nPos = nCount-1;
    while ( nPos && pHeight[nPos-1] >= nMinHeight )
        --nPos;
    SCROW nMinStart = nStartRow + nPos;

    ULONG nWeightedCount = 0;
    for (SCCOL nCol=0; nCol<MAXCOL; nCol++)     // MAXCOL schon oben
    {
        aCol[nCol].GetOptimalHeight(
            nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce,
            nMinHeight, nMinStart );

        if (pProgress)
        {
            ULONG nWeight = aCol[nCol].GetWeightedCount();
            if (nWeight)        // nochmal denselben Status muss auch nicht sein
            {
                nWeightedCount += nWeight;
                pProgress->SetState( nWeightedCount + nProgressStart );
            }
        }
    }

    SCROW nRngStart = 0;
    SCROW nRngEnd = 0;
    USHORT nLast = 0;
    for (SCSIZE i=0; i<nCount; i++)
    {
        size_t nIndex;
        SCROW nRegionEndRow;
        BYTE nRowFlag = pRowFlags->GetValue( nStartRow+i, nIndex, nRegionEndRow );
        if ( nRegionEndRow > nEndRow )
            nRegionEndRow = nEndRow;
        SCSIZE nMoreRows = nRegionEndRow - ( nStartRow+i );     // additional equal rows after first

        bool bAutoSize = ((nRowFlag & CR_MANUALSIZE) == 0);
        if ( bAutoSize || bForce )
        {
            if (nExtra)
            {
                if (bAutoSize)
                    pRowFlags->SetValue( nStartRow+i, nRegionEndRow, nRowFlag | CR_MANUALSIZE);
            }
            else if (!bAutoSize)
                pRowFlags->SetValue( nStartRow+i, nRegionEndRow, nRowFlag & ~CR_MANUALSIZE);

            for (SCSIZE nInner = i; nInner <= i + nMoreRows; ++nInner)
            {
                if (nLast)
                {
                    if (pHeight[nInner]+nExtra == nLast)
                        nRngEnd = nStartRow+nInner;
                    else
                    {
                        bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
                        nLast = 0;
                    }
                }
                if (!nLast)
                {
                    nLast = pHeight[nInner]+nExtra;
                    nRngStart = nStartRow+nInner;
                    nRngEnd = nStartRow+nInner;
                }
            }
        }
        else
        {
            if (nLast)
                bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
            nLast = 0;
        }
        i += nMoreRows;     // already handled - skip
    }
    if (nLast)
        bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );

    delete[] pHeight;
    if ( pProgress != pOuterProgress )
        delete pProgress;

    return bChanged;
}

BOOL ScTable::GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
    BOOL bFound = FALSE;
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    for (SCCOL i=0; i<=MAXCOL; i++)
        if (!aCol[i].IsEmptyVisData(TRUE))      // TRUE = Notizen zaehlen auch
        {
            bFound = TRUE;
            nMaxX = i;
            SCROW nColY = aCol[i].GetLastVisDataPos(TRUE);
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
}

BOOL ScTable::GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const
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
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    for (SCCOL i=0; i<=MAXCOL; i++)
        if (!aCol[i].IsEmpty())
        {
            bFound = TRUE;
            nMaxX = i;
            SCCOL nColY = aCol[i].GetLastEntryPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
*/

const SCCOL SC_COLUMNS_STOP = 30;

BOOL ScTable::GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, BOOL bNotes ) const
{
    BOOL bFound = FALSE;
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    SCCOL i;

    for (i=0; i<=MAXCOL; i++)               // Daten testen
        if (!aCol[i].IsEmptyVisData(bNotes))
        {
            bFound = TRUE;
            if (i>nMaxX)
                nMaxX = i;
            SCROW nColY = aCol[i].GetLastVisDataPos(bNotes);
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    SCCOL nMaxDataX = nMaxX;

    for (i=0; i<=MAXCOL; i++)               // Attribute testen
    {
        SCROW nLastRow;
        if (aCol[i].GetLastVisibleAttr( nLastRow ))
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

    if ( nMaxX < nMaxDataX )
    {
        nMaxX = nMaxDataX;
    }
    else if ( nMaxX > nMaxDataX )
    {
        SCCOL nAttrStartX = nMaxDataX + 1;
        while ( nAttrStartX < MAXCOL )
        {
            SCCOL nAttrEndX = nAttrStartX;
            while ( nAttrEndX < MAXCOL && aCol[nAttrStartX].IsVisibleAttrEqual(aCol[nAttrEndX+1]) )
                ++nAttrEndX;
            if ( nAttrEndX + 1 - nAttrStartX >= SC_COLUMNS_STOP )
            {
                // found equally-formatted columns behind data -> stop before these columns
                nMaxX = nAttrStartX - 1;

                // also don't include default-formatted columns before that
                SCROW nDummyRow;
                while ( nMaxX > nMaxDataX && !aCol[nMaxX].GetLastVisibleAttr( nDummyRow ) )
                    --nMaxX;
                break;
            }
            nAttrStartX = nAttrEndX + 1;
        }
    }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
}

BOOL ScTable::GetPrintAreaHor( SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rEndCol, BOOL /* bNotes */ ) const
{
    BOOL bFound = FALSE;
    SCCOL nMaxX = 0;
    SCCOL i;

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

BOOL ScTable::GetPrintAreaVer( SCCOL nStartCol, SCCOL nEndCol,
                                SCROW& rEndRow, BOOL bNotes ) const
{
    BOOL bFound = FALSE;
    SCROW nMaxY = 0;
    SCCOL i;

    for (i=nStartCol; i<=nEndCol; i++)              // Attribute testen
    {
        SCROW nLastRow;
        if (aCol[i].GetLastVisibleAttr( nLastRow ))
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
            SCROW nColY = aCol[i].GetLastVisDataPos(bNotes);
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    rEndRow = nMaxY;
    return bFound;
}

BOOL ScTable::GetDataStart( SCCOL& rStartCol, SCROW& rStartRow ) const
{
    BOOL bFound = FALSE;
    SCCOL nMinX = MAXCOL;
    SCROW nMinY = MAXROW;
    SCCOL i;

    for (i=0; i<=MAXCOL; i++)                   // Attribute testen
    {
        SCROW nFirstRow;
        if (aCol[i].GetFirstVisibleAttr( nFirstRow ))
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
            SCROW nColY = aCol[i].GetFirstVisDataPos(TRUE);
            if (nColY < nMinY)
                nMinY = nColY;
        }

    rStartCol = nMinX;
    rStartRow = nMinY;
    return bFound;
}

void ScTable::GetDataArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow,
                            BOOL bIncludeOld )
{
    BOOL bLeft       = FALSE;
    BOOL bRight  = FALSE;
    BOOL bTop        = FALSE;
    BOOL bBottom = FALSE;
    BOOL bChanged;
    BOOL bFound;
    SCCOL i;
    SCROW nTest;

    do
    {
        bChanged = FALSE;

        SCROW nStart = rStartRow;
        SCROW nEnd = rEndRow;
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

SCSIZE ScTable::GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, ScDirection eDir )
{
    SCSIZE nCount = 0;
    SCCOL nCol;
    if ((eDir == DIR_BOTTOM) || (eDir == DIR_TOP))
    {
        nCount = static_cast<SCSIZE>(nEndRow - nStartRow);
        for (nCol = nStartCol; nCol <= nEndCol; nCol++)
            nCount = Min(nCount, aCol[nCol].GetEmptyLinesInBlock(nStartRow, nEndRow, eDir));
    }
    else if (eDir == DIR_RIGHT)
    {
        nCol = nEndCol;
        while (((SCsCOL)nCol >= (SCsCOL)nStartCol) &&
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

BOOL ScTable::IsEmptyLine( SCROW nRow, SCCOL nStartCol, SCCOL nEndCol )
{
    BOOL bFound = FALSE;
    for (SCCOL i=nStartCol; i<=nEndCol && !bFound; i++)
        if (aCol[i].HasDataAt(nRow))
            bFound = TRUE;
    return !bFound;
}

void ScTable::LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow )
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

void ScTable::FindAreaPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY )
{
    if (nMovX)
    {
        SCsCOL nNewCol = (SCsCOL) rCol;
        BOOL bThere = aCol[nNewCol].HasVisibleDataAt(rRow);
        BOOL bFnd;
        if (bThere)
        {
            do
            {
                nNewCol = sal::static_int_cast<SCsCOL>( nNewCol + nMovX );
                bFnd = (nNewCol>=0 && nNewCol<=MAXCOL) ? aCol[nNewCol].HasVisibleDataAt(rRow) : FALSE;
            }
            while (bFnd);
            nNewCol = sal::static_int_cast<SCsCOL>( nNewCol - nMovX );

            if (nNewCol == (SCsCOL)rCol)
                bThere = FALSE;
        }

        if (!bThere)
        {
            do
            {
                nNewCol = sal::static_int_cast<SCsCOL>( nNewCol + nMovX );
                bFnd = (nNewCol>=0 && nNewCol<=MAXCOL) ? aCol[nNewCol].HasVisibleDataAt(rRow) : TRUE;
            }
            while (!bFnd);
        }

        if (nNewCol<0) nNewCol=0;
        if (nNewCol>MAXCOL) nNewCol=MAXCOL;
        rCol = (SCCOL) nNewCol;
    }

    if (nMovY)
        aCol[rCol].FindDataAreaPos(rRow,nMovY);
}

BOOL ScTable::ValidNextPos( SCCOL nCol, SCROW nRow, const ScMarkData& rMark,
                                BOOL bMarked, BOOL bUnprotected )
{
    if (!ValidCol(nCol) || !ValidRow(nRow))
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

        if ( pRowFlags && ( pRowFlags->GetValue(nRow) & CR_HIDDEN ) )
            return FALSE;
        if ( pColFlags && ( pColFlags[nCol] & CR_HIDDEN ) )
            return FALSE;
    }

    return TRUE;
}

void ScTable::GetNextPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark )
{
    if (bUnprotected && !IsProtected())     // Tabelle ueberhaupt geschuetzt?
        bUnprotected = FALSE;

    USHORT nWrap = 0;
    SCsCOL nCol = rCol;
    SCsROW nRow = rRow;

    nCol = sal::static_int_cast<SCsCOL>( nCol + nMovX );
    nRow = sal::static_int_cast<SCsROW>( nRow + nMovY );

    DBG_ASSERT( !nMovY || !bUnprotected,
                "GetNextPos mit bUnprotected horizontal nicht implementiert" );

    if ( nMovY && bMarked )
    {
        BOOL bUp = ( nMovY < 0 );
        nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        while ( VALIDROW(nRow) && pRowFlags && (pRowFlags->GetValue(nRow) & CR_HIDDEN) )
        {
            //  #53697# ausgeblendete ueberspringen (s.o.)
            nRow += nMovY;
            nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        }

        while ( nRow < 0 || nRow > MAXROW )
        {
            nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );
            while ( VALIDCOL(nCol) && pColFlags && (pColFlags[nCol] & CR_HIDDEN) )
                nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );   //  #53697# skip hidden rows (see above)
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
            while ( VALIDROW(nRow) && pRowFlags && (pRowFlags->GetValue(nRow) & CR_HIDDEN) )
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
            SCsROW* pNextRows = new SCsROW[MAXCOL+1];
            SCCOL i;

            if ( nMovX > 0 )                            //  vorwaerts
            {
                for (i=0; i<=MAXCOL; i++)
                    pNextRows[i] = (i<nCol) ? (nRow+1) : nRow;
                do
                {
                    SCsROW nNextRow = pNextRows[nCol] + 1;
                    if ( bMarked )
                        nNextRow = rMark.GetNextMarked( nCol, nNextRow, FALSE );
                    if ( bUnprotected )
                        nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, FALSE );
                    pNextRows[nCol] = nNextRow;

                    SCsROW nMinRow = MAXROW+1;
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
                        nCol = 0;
                        nRow = 0;
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
                    SCsROW nNextRow = pNextRows[nCol] - 1;
                    if ( bMarked )
                        nNextRow = rMark.GetNextMarked( nCol, nNextRow, TRUE );
                    if ( bUnprotected )
                        nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, TRUE );
                    pNextRows[nCol] = nNextRow;

                    SCsROW nMaxRow = -1;
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

BOOL ScTable::GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark )
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
            SCROW nStart = (SCROW) rArray.GetNextMarked( (SCsROW) rRow, FALSE );
            if ( nStart <= MAXROW )
            {
                SCROW nEnd = rArray.GetMarkEnd( nStart, FALSE );
                ScColumnIterator aColIter( &aCol[rCol], nStart, nEnd );
                SCROW nCellRow;
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

void ScTable::UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       // only within the table
    {
        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if ( eUpdateRefMode != URM_COPY && pDrawLayer )
        {
            if ( eUpdateRefMode == URM_MOVE )
            {                                               // source range
                nCol1 = sal::static_int_cast<SCCOL>( nCol1 - nDx );
                nRow1 = sal::static_int_cast<SCROW>( nRow1 - nDy );
                nCol2 = sal::static_int_cast<SCCOL>( nCol2 - nDx );
                nRow2 = sal::static_int_cast<SCROW>( nRow2 - nDy );
            }
            pDrawLayer->MoveArea( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy,
                                    (eUpdateRefMode == URM_INSDEL) );
        }
    }
}

void ScTable::UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2, SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                     ScDocument* pUndoDoc, BOOL bIncludeDraw )
{
    SCCOL i;
    SCCOL iMax;
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
        SCTAB nSTab = nTab;
        SCTAB nETab = nTab;
        SCCOL nSCol = 0;
        SCROW nSRow = 0;
        SCCOL nECol = 0;
        SCROW nERow = 0;
        BOOL bRecalcPages = FALSE;

        for ( ScRangeVec::iterator aIt = aPrintRanges.begin(), aEnd = aPrintRanges.end(); aIt != aEnd; ++aIt )
        {
            nSCol = aIt->aStart.Col();
            nSRow = aIt->aStart.Row();
            nECol = aIt->aEnd.Col();
            nERow = aIt->aEnd.Row();

            // do not try to modify sheet index of print range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *aIt = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = TRUE;
            }
        }

        if ( pRepeatColRange )
        {
            nSCol = pRepeatColRange->aStart.Col();
            nSRow = pRepeatColRange->aStart.Row();
            nECol = pRepeatColRange->aEnd.Col();
            nERow = pRepeatColRange->aEnd.Row();

            // do not try to modify sheet index of repeat range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatColRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = TRUE;
                nRepeatStartX = nSCol;  // fuer UpdatePageBreaks
                nRepeatEndX = nECol;
            }
        }

        if ( pRepeatRowRange )
        {
            nSCol = pRepeatRowRange->aStart.Col();
            nSRow = pRepeatRowRange->aStart.Row();
            nECol = pRepeatRowRange->aEnd.Col();
            nERow = pRepeatRowRange->aEnd.Row();

            // do not try to modify sheet index of repeat range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatRowRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
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
    for ( SCCOL i=0; i<=MAXCOL; i++ )
        aCol[i].UpdateTranspose( rSource, rDest, pUndoDoc );
}

void ScTable::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    for ( SCCOL i=0; i<=MAXCOL; i++ )
        aCol[i].UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScTable::UpdateInsertTab(SCTAB nTable)
{
    if (nTab >= nTable) nTab++;
    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTab(nTable);
}

void ScTable::UpdateInsertTabOnlyCells(SCTAB nTable)
{
    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTabOnlyCells(nTable);
}

void ScTable::UpdateDeleteTab( SCTAB nTable, BOOL bIsMove, ScTable* pRefUndo )
{
    if (nTab > nTable) nTab--;

    SCCOL i;
    if (pRefUndo)
        for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, &pRefUndo->aCol[i]);
    else
        for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, NULL);
}

void ScTable::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo,
        ScProgress& rProgress )
{
    nTab = nTabNo;
    for ( SCCOL i=0; i <= MAXCOL; i++ )
    {
        aCol[i].UpdateMoveTab( nOldPos, nNewPos, nTabNo );
        rProgress.SetState( rProgress.GetState() + aCol[i].GetCodeCount() );
    }
}

void ScTable::UpdateCompile( BOOL bForceIfNameInUse )
{
    for (SCCOL i=0; i <= MAXCOL; i++)
    {
        aCol[i].UpdateCompile( bForceIfNameInUse );
    }
}

void ScTable::SetTabNo(SCTAB nNewTab)
{
    nTab = nNewTab;
    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].SetTabNo(nNewTab);
}

BOOL ScTable::IsRangeNameInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                               USHORT nIndex) const
{
    BOOL bInUse = FALSE;
    for (SCCOL i = nCol1; !bInUse && (i <= nCol2) && (ValidCol(i)); i++)
        bInUse = aCol[i].IsRangeNameInUse(nRow1, nRow2, nIndex);
    return bInUse;
}

void ScTable::FindRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                               std::set<USHORT>& rIndexes) const
{
    for (SCCOL i = nCol1; i <= nCol2 && ValidCol(i); i++)
        aCol[i].FindRangeNamesInUse(nRow1, nRow2, rIndexes);
}

void ScTable::ReplaceRangeNamesInUse(SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScIndexMap& rMap )
{
    for (SCCOL i = nCol1; i <= nCol2 && (ValidCol(i)); i++)
    {
        aCol[i].ReplaceRangeNamesInUse( nRow1, nRow2, rMap );
    }
}

void ScTable::ExtendPrintArea( OutputDevice* pDev,
                    SCCOL /* nStartCol */, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow )
{
    if ( !pColFlags || !pRowFlags )
    {
        DBG_ERROR("keine ColInfo oder RowInfo in ExtendPrintArea");
        return;
    }

    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aPix1000.X() / 1000.0;
    double nPPTY = aPix1000.Y() / 1000.0;

    BOOL bEmpty[MAXCOLCOUNT];
    for (SCCOL i=0; i<=MAXCOL; i++)
        bEmpty[i] = ( aCol[i].GetCellCount() == 0 );

    SCSIZE nIndex;
    SCCOL nPrintCol = rEndCol;
    SCSIZE nRowFlagsIndex;
    SCROW nRowFlagsEndRow;
    BYTE nRowFlag = pRowFlags->GetValue( nStartRow, nRowFlagsIndex, nRowFlagsEndRow);
    for (SCROW nRow = nStartRow; nRow<=nEndRow; nRow++)
    {
        if (nRow > nRowFlagsEndRow)
            nRowFlag = pRowFlags->GetNextValue( nRowFlagsIndex, nRowFlagsEndRow);
        if ( ( nRowFlag & CR_HIDDEN ) == 0 )
        {
            SCCOL nDataCol = rEndCol;
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

                        long nMissing = nTwips - nDocW;
                        if ( nMissing > 0 )
                        {
                            //  look at alignment

                            const ScPatternAttr* pPattern = GetPattern( nDataCol, nRow );
                            const SfxItemSet* pCondSet = NULL;
                            if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
                                pCondSet = pDocument->GetCondResult( nDataCol, nRow, nTab );

                            SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem( ATTR_HOR_JUSTIFY, pCondSet )).GetValue();
                            if ( eHorJust == SVX_HOR_JUSTIFY_CENTER )
                                nMissing /= 2;                          // distributed into both directions
                            else
                            {
                                // STANDARD is LEFT (only text is handled here)
                                BOOL bRight = ( eHorJust == SVX_HOR_JUSTIFY_RIGHT );
                                if ( IsLayoutRTL() )
                                    bRight = !bRight;
                                if ( bRight )
                                    nMissing = 0;       // extended only to the left (logical)
                            }
                        }

                        SCCOL nCol = nDataCol;
                        while (nMissing > 0 && nCol < MAXCOL)
                        {
                            ++nCol;
                            nMissing -= GetColWidth( nCol );
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

void ScTable::DoColResize( SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd )
{
    for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
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

void ScTable::ClearPrintRanges()
{
    aPrintRanges.clear();
    bPrintEntireSheet = FALSE;
}

void ScTable::AddPrintRange( const ScRange& rNew )
{
    bPrintEntireSheet = FALSE;
    if( aPrintRanges.size() < 0xFFFF )
        aPrintRanges.push_back( rNew );
}

void ScTable::SetPrintRange( const ScRange& rNew )
{
    ClearPrintRanges();
    AddPrintRange( rNew );
}

void ScTable::SetPrintEntireSheet()
{
    if( !IsPrintEntireSheet() )
    {
        ClearPrintRanges();
        bPrintEntireSheet = TRUE;
    }
}

const ScRange* ScTable::GetPrintRange(USHORT nPos) const
{
    return (nPos < GetPrintRangeCount()) ? &aPrintRanges[ nPos ] : NULL;
}

void ScTable::FillPrintSaver( ScPrintSaverTab& rSaveTab ) const
{
    rSaveTab.SetAreas( aPrintRanges, bPrintEntireSheet );
    rSaveTab.SetRepeat( pRepeatColRange, pRepeatRowRange );
}

void ScTable::RestorePrintRanges( const ScPrintSaverTab& rSaveTab )
{
    aPrintRanges = rSaveTab.GetPrintRanges();
    bPrintEntireSheet = rSaveTab.IsEntireSheet();
    SetRepeatColRange( rSaveTab.GetRepeatCol() );
    SetRepeatRowRange( rSaveTab.GetRepeatRow() );

    UpdatePageBreaks(NULL);
}





