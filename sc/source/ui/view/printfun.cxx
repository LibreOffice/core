/*************************************************************************
 *
 *  $RCSfile: printfun.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:09 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/svxids.hrc>
#include <svx/adjitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/editstat.hxx>     // EE_CNTRL_RTFSTYLESHEETS
#include <svx/fmview.hxx>
#include <svx/lrspitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/pbinitem.hxx>
#include <svx/shaditem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/ulspitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/progress.hxx>
#include <tools/multisel.hxx>
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifdef MAC
#define RGBColor MAC_RGBColor
#include <svx/xoutbmp.hxx>
#undef RGBColor
#else
#include <svx/xoutbmp.hxx>
#endif


//#if defined( WIN ) || defined( WNT )
//#include <svwin.h>
//#endif


#include "editutil.hxx"
#include "docsh.hxx"
#include "output.hxx"
#include "viewdata.hxx"
#include "viewopti.hxx"
#include "stlpool.hxx"
#include "pagepar.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "dociter.hxx"
#include "cell.hxx"
#include "drawutil.hxx"
#include "globstr.hrc"
#include "scresid.hxx"
#include "sc.hrc"
#include "pagedata.hxx"

#define _PRINTFUN_CXX
#include "printfun.hxx"



#define ZOOM_MIN    10

#define GET_BOOL(set,which)   ((const SfxBoolItem&)(set)->Get((which))).GetValue()
#define GET_USHORT(set,which) ((const SfxUInt16Item&)(set)->Get((which))).GetValue()
#define GET_SHOW(set,which)   ( VOBJ_MODE_SHOW == ScVObjMode( ((const ScViewObjectModeItem&)(set)->Get((which))).GetValue()) )

//------------------------------------------------------------------------

ScPageRowEntry::ScPageRowEntry(const ScPageRowEntry& r)
{
    nStartRow = r.nStartRow;
    nEndRow   = r.nEndRow;
    nPagesX   = r.nPagesX;
    if (r.pHidden && nPagesX)
    {
        pHidden = new BOOL[nPagesX];
        memcpy( pHidden, r.pHidden, nPagesX * sizeof(BOOL) );
    }
    else
        pHidden = NULL;
}

const ScPageRowEntry& ScPageRowEntry::operator=(const ScPageRowEntry& r)
{
    delete[] pHidden;

    nStartRow = r.nStartRow;
    nEndRow   = r.nEndRow;
    nPagesX   = r.nPagesX;
    if (r.pHidden && nPagesX)
    {
        pHidden = new BOOL[nPagesX];
        memcpy( pHidden, r.pHidden, nPagesX * sizeof(BOOL) );
    }
    else
        pHidden = NULL;

    return *this;
}

void ScPageRowEntry::SetPagesX(USHORT nNew)
{
    if (pHidden)
    {
        DBG_ERROR("SetPagesX nicht nach SetHidden");
        delete[] pHidden;
        pHidden = NULL;
    }
    nPagesX = nNew;
}

void ScPageRowEntry::SetHidden(USHORT nX)
{
    if ( nX < nPagesX )
    {
        if ( nX+1 == nPagesX )  // letzte Seite?
            --nPagesX;
        else
        {
            if (!pHidden)
            {
                pHidden = new BOOL[nPagesX];
                memset( pHidden, FALSE, nPagesX * sizeof(BOOL) );
            }
            pHidden[nX] = TRUE;
        }
    }
}

BOOL ScPageRowEntry::IsHidden(USHORT nX) const
{
    return nX>=nPagesX || ( pHidden && pHidden[nX] );       //! inline?
}

USHORT ScPageRowEntry::CountVisible() const
{
    if ( pHidden )
    {
        USHORT nVis = 0;
        for (USHORT i=0; i<nPagesX; i++)
            if (!pHidden[i])
                ++nVis;
        return nVis;
    }
    else
        return nPagesX;
}

//------------------------------------------------------------------------

long lcl_LineTotal(const SvxBorderLine* pLine)
{
    return pLine ? ( pLine->GetOutWidth() + pLine->GetInWidth() + pLine->GetDistance() ) : 0;
}

void ScPrintFunc::Construct()
{
    pDoc = pDocShell->GetDocument();

    SfxPrinter* pDocPrinter = pDoc->GetPrinter();   // auch fuer Preview den Drucker nehmen
    if (pDocPrinter)
        aOldPrinterMode = pDocPrinter->GetMapMode();

    //  einheitlicher MapMode ueber alle Aufrufe (z.B. Repaint !!!),
    //  weil die EditEngine sonst unterschiedliche Texthoehen liefert
    pDev->SetMapMode(MAP_PIXEL);

    pPageEndX = NULL;
    pPageEndY = NULL;
    pPageRows = NULL;
    pBorderItem = NULL;
    pBackgroundItem = NULL;
    pShadowItem = NULL;

    pEditEngine = NULL;
    pEditDefaults = NULL;

    ScStyleSheetPool* pStylePool    = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet  = pStylePool->Find(
                                            pDoc->GetPageStyle( nPrintTab ),
                                            SFX_STYLE_FAMILY_PAGE );
    if (pStyleSheet)
        pParamSet = &pStyleSheet->GetItemSet();
    else
    {
        DBG_ERROR("Seitenvorlage nicht gefunden" );
        pParamSet = NULL;
    }

    if (!bState)
        nZoom = 100;
    nManualZoom = 100;
    bClearWin = FALSE;

    InitParam();

    pPageData = NULL;       // wird nur zur Initialisierung gebraucht
}

ScPrintFunc::ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, USHORT nTab,
                            long nPage, long nDocP, const ScRange* pArea,
                            ScPageBreakData* pData )
    :   pDocShell           ( pShell ),
        pPrinter            ( pNewPrinter ),
        pDrawView           ( NULL ),
        nPrintTab           ( nTab ),
        nPageStart          ( nPage ),
        nDocPages           ( nDocP ),
        pUserArea           ( pArea ),
        pPageData           ( pData ),
        nTotalPages         ( 0 ),
        nTabPages           ( 0 ),
        bState              ( FALSE ),
        bPrintCurrentTable  ( FALSE ),
        bMultiArea          ( FALSE )
{
    pDev = pPrinter;
    aSrcOffset = pPrinter->PixelToLogic( pPrinter->GetPageOffsetPixel(), MAP_100TH_MM );
    Construct();
}

ScPrintFunc::ScPrintFunc( ScDocShell* pShell, Window* pWindow, USHORT nTab,
                            long nPage, long nDocP, const ScRange* pArea )
    :   pDocShell           ( pShell ),
        pPrinter            ( NULL ),
        pDrawView           ( NULL ),
        nPrintTab           ( nTab ),
        nPageStart          ( nPage ),
        nDocPages           ( nDocP ),
        pUserArea           ( pArea ),
        pPageData           ( NULL ),
        nTotalPages         ( 0 ),
        nTabPages           ( 0 ),
        bState              ( FALSE ),
        bPrintCurrentTable  ( FALSE ),
        bMultiArea          ( FALSE )
{
    pDev = pWindow;
    Construct();
}

ScPrintFunc::ScPrintFunc( ScDocShell* pShell, Window* pWindow,
                                 const ScPrintState& rState )
    :   pDocShell           ( pShell ),
        pPrinter            ( NULL ),
        pDrawView           ( NULL ),
        pUserArea           ( NULL ),
        pPageData           ( NULL ),
        bPrintCurrentTable  ( FALSE ),
        bMultiArea          ( FALSE )
{
    pDev = pWindow;

    nPrintTab   = rState.nPrintTab;
    nStartCol   = rState.nStartCol;
    nStartRow   = rState.nStartRow;
    nEndCol     = rState.nEndCol;
    nEndRow     = rState.nEndRow;
    nZoom       = rState.nZoom;
    nPagesX     = rState.nPagesX;
    nPagesY     = rState.nPagesY;
    nTabPages   = rState.nTabPages;
    nTotalPages = rState.nTotalPages;
    nPageStart  = rState.nPageStart;
    nDocPages   = rState.nDocPages;
    bState      = TRUE;

    Construct();
}

void ScPrintFunc::GetPrintState( ScPrintState& rState )
{
    rState.nPrintTab    = nPrintTab;
    rState.nStartCol    = nStartCol;
    rState.nStartRow    = nStartRow;
    rState.nEndCol      = nEndCol;
    rState.nEndRow      = nEndRow;
    rState.nZoom        = nZoom;
    rState.nPagesX      = nPagesX;
    rState.nPagesY      = nPagesY;
    rState.nTabPages    = nTabPages;
    rState.nTotalPages  = nTotalPages;
    rState.nPageStart   = nPageStart;
    rState.nDocPages    = nDocPages;
}

void ScPrintFunc::FillPageData()
{
    if (pPageData)
    {
        USHORT nCount = pPageData->GetCount();
        ScPrintRangeData& rData = pPageData->GetData(nCount);       // hochzaehlen

        rData.SetPrintRange( ScRange( nStartCol, nStartRow, nPrintTab,
                                        nEndCol, nEndRow, nPrintTab ) );
        rData.SetPagesX( nPagesX, pPageEndX );
        rData.SetPagesY( nTotalY, pPageEndY );

        //  Einstellungen
        rData.SetTopDown( aTableParam.bTopDown );
        rData.SetAutomatic( !aAreaParam.bPrintArea );
    }
}

ScPrintFunc::~ScPrintFunc()
{
    ScTripel* pTripel = (ScTripel*) aNotePosList.First();
    while (pTripel)
    {
        delete pTripel;
        pTripel = (ScTripel*) aNotePosList.Next();
    }
    aNotePosList.Clear();

    delete[] pPageEndX;
    delete[] pPageEndY;
    delete[] pPageRows;
    delete pEditDefaults;
    delete pEditEngine;

    //  Druckereinstellungen werden jetzt von aussen wiederhergestellt

    //  #64294# Fuer DrawingLayer/Charts muss der MapMode am Drucker (RefDevice) immer stimmen
    SfxPrinter* pDocPrinter = pDoc->GetPrinter();   // auch fuer Preview den Drucker nehmen
    if (pDocPrinter)
        pDocPrinter->SetMapMode(aOldPrinterMode);
}

void ScPrintFunc::SetDrawView( FmFormView* pNew )
{
    pDrawView = pNew;
}

//
//          Ausgabe auf Device (static)
//
//      wird benutzt fuer:
//      -   Clipboard/Bitmap
//      -   Ole-Object (DocShell::Draw)
//      -   Vorschau bei Vorlagen

void ScPrintFunc::DrawToDev( ScDocument* pDoc, OutputDevice* pDev, double nPrintFactor,
                            const Rectangle& rBound, ScViewData* pViewData, BOOL bMetaFile )
{
    //! nPrintFactor auswerten !!!

    USHORT nTab = 0;
    if (pViewData)
        nTab = pViewData->GetTabNo();

    BOOL bDoGrid, bNullVal, bFormula;
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ), SFX_STYLE_FAMILY_PAGE );
    if (pStyleSheet)
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        bDoGrid  = ((const SfxBoolItem&)rSet.Get(ATTR_PAGE_GRID)).GetValue();
        bNullVal = ((const SfxBoolItem&)rSet.Get(ATTR_PAGE_NULLVALS)).GetValue();
        bFormula = ((const SfxBoolItem&)rSet.Get(ATTR_PAGE_FORMULAS)).GetValue();
    }
    else
    {
        const ScViewOptions& rOpt = pDoc->GetViewOptions();
        bDoGrid  = rOpt.GetOption(VOPT_GRID);
        bNullVal = rOpt.GetOption(VOPT_NULLVALS);
        bFormula = rOpt.GetOption(VOPT_FORMULAS);
    }

    MapMode aMode = pDev->GetMapMode();

    Rectangle aRect = rBound;

    if (aRect.Right() < aRect.Left() || aRect.Bottom() < aRect.Top())
        aRect = Rectangle( Point(), pDev->GetOutputSize() );

    USHORT nX1 = 0;
    USHORT nY1 = 0;
    USHORT nX2 = OLE_STD_CELLS_X - 1;
    USHORT nY2 = OLE_STD_CELLS_Y - 1;
    if (bMetaFile)
    {
        ScRange aRange = pDoc->GetRange( nTab, rBound );
        nX1 = aRange.aStart.Col();
        nY1 = aRange.aStart.Row();
        nX2 = aRange.aEnd.Col();
        nY2 = aRange.aEnd.Row();
    }
    else if (pViewData)
    {
        ScSplitPos eWhich = pViewData->GetActivePart();
        ScHSplitPos eHWhich = WhichH(eWhich);
        ScVSplitPos eVWhich = WhichV(eWhich);
        nX1 = pViewData->GetPosX(eHWhich);
        nY1 = pViewData->GetPosY(eVWhich);
        nX2 = nX1 + pViewData->VisibleCellsX(eHWhich);
        if (nX2>nX1) --nX2;
        nY2 = nY1 + pViewData->VisibleCellsY(eVWhich);
        if (nY2>nY1) --nY2;
    }

    if (nX1 > MAXCOL) nX1 = MAXCOL;
    if (nX2 > MAXCOL) nX2 = MAXCOL;
    if (nY1 > MAXROW) nY1 = MAXROW;
    if (nY2 > MAXROW) nY2 = MAXROW;

    USHORT i;
    long nDevSizeX = aRect.Right()-aRect.Left()+1;
    long nDevSizeY = aRect.Bottom()-aRect.Top()+1;

    Rectangle aLines;
    ScRange aRange( nX1,nY1,nTab, nX2,nY2,nTab );
    BOOL bAddLines = pDoc->HasLines( aRange, aLines );

    long nTwipsSizeX = 0;
    for (i=nX1; i<=nX2; i++)
        nTwipsSizeX += pDoc->GetColWidth( i, nTab );
    long nTwipsSizeY = 0;
    for (i=nY1; i<=nY2; i++)
        nTwipsSizeY += pDoc->GetRowHeight( i, nTab );

    //  wenn keine Linien, dann trotzdem Platz fuer den Aussenrahmen (20 Twips = 1pt)
    //  (HasLines initalisiert aLines auf 0,0,0,0)
    nTwipsSizeX += aLines.Left() + Max( aLines.Right(), 20L );
    nTwipsSizeY += aLines.Top() +  Max( aLines.Bottom(), 20L );

    double nScaleX = (double) nDevSizeX / nTwipsSizeX;
    double nScaleY = (double) nDevSizeY / nTwipsSizeY;

                            //!     Flag bei FillInfo uebergeben !!!!!
    ScTripel aEStart;
    ScTripel aEEnd;
    BOOL bEmbed = pDoc->IsEmbedded();
    if (bEmbed)
    {
        pDoc->GetEmbedded(aEStart,aEEnd);
        pDoc->ResetEmbedded();
    }

    //  Daten zusammenstellen

    RowInfo* pRowInfo = new RowInfo[ROWINFO_MAX];
    USHORT nArrCount = pDoc->FillInfo( pRowInfo, nX1, nY1, nX2, nY2, nTab,
                                        nScaleX, nScaleY, FALSE, bFormula );

    if (bEmbed)
        pDoc->SetEmbedded(aEStart,aEEnd);

/*  if (!bMetaFile)
        pDev->SetMapMode(MAP_PIXEL);
*/
    long nScrX = aRect.Left();
    long nScrY = aRect.Top();

    //  Wenn keine Linien, trotzdem Platz fuer Gitterlinien lassen
    //  (werden sonst abgeschnitten)
    long nAddX = (long)( aLines.Left() * nScaleX );
    nScrX += ( nAddX ? nAddX : 1 );
    long nAddY = (long)( aLines.Top() * nScaleY );
    nScrY += ( nAddY ? nAddY : 1 );

    ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, pRowInfo, nArrCount, pDoc, nTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nScaleX, nScaleY );
    aOutputData.SetMetaFileMode(bMetaFile);
    aOutputData.SetShowNullValues(bNullVal);
    aOutputData.SetShowFormulas(bFormula);

    if ( bMetaFile && pDev->GetOutDevType() == OUTDEV_VIRDEV )
        aOutputData.SetSnapPixel();

    Point aLogStart = pDev->PixelToLogic( Point(nScrX,nScrY), MAP_100TH_MM );
    long nLogStX = aLogStart.X();
    long nLogStY = aLogStart.Y();

    //!     nZoom fuer GetFont in OutputData ???

    if (!bMetaFile && pViewData)
        pDev->SetMapMode(pViewData->GetLogicMode(pViewData->GetActivePart()));
    aOutputData.DrawingLayer(SC_LAYER_BACK,SC_OBJECTS_ALL,nLogStX,nLogStY);
    if (!bMetaFile && pViewData)
        pDev->SetMapMode(aMode);

    aOutputData.DrawBackground();

#ifdef OS2
    if (bMetaFile && !bDoGrid)
    {
                    // unter OS2 fuer Metafiles gesamte Flaeche benutzen,
                    // weil sonst die Groesse nicht erkannt wird
        pDev->SetLineColor();
        pDev->SetFillColor();
        pDev->DrawRect( Rectangle( nScrX,nScrY,
                        nScrX+aOutputData.GetScrW(), nScrY+aOutputData.GetScrH() ) );
    }
#endif

    aOutputData.DrawShadow();
    aOutputData.DrawFrame();
    aOutputData.DrawStrings();

    if (!bMetaFile && pViewData)
        pDev->SetMapMode(pViewData->GetLogicMode(pViewData->GetActivePart()));

    aOutputData.DrawEdit(!bMetaFile);

    if (bDoGrid)
    {
        if (!bMetaFile && pViewData)
            pDev->SetMapMode(aMode);

        aOutputData.DrawGrid( TRUE, FALSE );    // keine Seitenumbrueche

        pDev->SetLineColor( COL_BLACK );

        Size aOne = pDev->PixelToLogic( Size(1,1) );
        if (bMetaFile)
            aOne = Size(1,1);   // kompatibel zu DrawGrid
        pDev->DrawLine( Point(nScrX,nScrY), Point(nScrX,nScrY+aOutputData.GetScrH()-aOne.Width() ) );
        pDev->DrawLine( Point(nScrX,nScrY), Point(nScrX+aOutputData.GetScrW()-aOne.Height(),nScrY ) );
    }

    aOutputData.DrawingLayer(SC_LAYER_FRONT,SC_OBJECTS_ALL,nLogStX,nLogStY);
    aOutputData.DrawingLayer(SC_LAYER_INTERN,SC_OBJECTS_ALL,nLogStX,nLogStY);

    for (i=0; i<nArrCount; i++)
        delete[] pRowInfo[i].pCellInfo;
    delete[] pRowInfo;
}

//
//          Drucken
//

void lcl_FillHFParam( ScPrintHFParam& rParam, const SfxItemSet* pHFSet )
{
    //  nDistance muss vorher unterschiedlich initalisiert sein

    if ( pHFSet == NULL )
    {
        rParam.bEnable  = FALSE;
        rParam.pBorder  = NULL;
        rParam.pBack    = NULL;
        rParam.pShadow  = NULL;
    }
    else
    {
        rParam.bEnable  = ((const SfxBoolItem&) pHFSet->Get(ATTR_PAGE_ON)).GetValue();
        rParam.bDynamic = ((const SfxBoolItem&) pHFSet->Get(ATTR_PAGE_DYNAMIC)).GetValue();
        rParam.bShared  = ((const SfxBoolItem&) pHFSet->Get(ATTR_PAGE_SHARED)).GetValue();
        rParam.nHeight  = ((const SvxSizeItem&) pHFSet->Get(ATTR_PAGE_SIZE)).GetSize().Height();
        const SvxLRSpaceItem* pHFLR = &(const SvxLRSpaceItem&) pHFSet->Get(ATTR_LRSPACE);
        long nTmp;
        nTmp = pHFLR->GetLeft();
        rParam.nLeft = nTmp < 0 ? 0 : USHORT(nTmp);
        nTmp = pHFLR->GetRight();
        rParam.nRight = nTmp < 0 ? 0 : USHORT(nTmp);
        rParam.pBorder  = (const SvxBoxItem*)   &pHFSet->Get(ATTR_BORDER);
        rParam.pBack    = (const SvxBrushItem*) &pHFSet->Get(ATTR_BACKGROUND);
        rParam.pShadow  = (const SvxShadowItem*)&pHFSet->Get(ATTR_SHADOW);;

//  jetzt doch wieder schon im Dialog:
//      rParam.nHeight += rParam.nDistance;             // nicht mehr im Dialog ???

        if (rParam.pBorder)
            rParam.nHeight += lcl_LineTotal( rParam.pBorder->GetTop() ) +
                              lcl_LineTotal( rParam.pBorder->GetBottom() );

        rParam.nManHeight = rParam.nHeight;
    }

    if (!rParam.bEnable)
        rParam.nHeight = 0;
}

//  bNew = TRUE:    benutzten Bereich aus dem Dokument suchen
//  bNew = FALSE:   nur ganze Zeilen/Spalten begrenzen

BOOL ScPrintFunc::AdjustPrintArea( BOOL bNew )
{
    USHORT nOldEndCol = nEndCol;    // nur wichtig bei !bNew
    USHORT nOldEndRow = nEndRow;
    BOOL bChangeCol = TRUE;         // bei bNew werden beide angepasst
    BOOL bChangeRow = TRUE;

    BOOL bNotes = aTableParam.bNotes;
    if ( bNew )
    {
        nStartCol = nStartRow = 0;
        if (!pDoc->GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes ))
            return FALSE;   // nix
    }
    else
    {
        BOOL bFound = TRUE;
        bChangeCol = ( nStartCol == 0 && nEndCol == MAXCOL );
        bChangeRow = ( nStartRow == 0 && nEndRow == MAXROW );
        if ( bChangeCol && bChangeRow )
            bFound = pDoc->GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes );
        else if ( bChangeCol )
            bFound = pDoc->GetPrintAreaHor( nPrintTab, nStartRow, nEndRow, nEndCol, bNotes );
        else if ( bChangeRow )
            bFound = pDoc->GetPrintAreaVer( nPrintTab, nStartCol, nEndCol, nEndRow, bNotes );

        if (!bFound)
            return FALSE;   // leer
    }

    pDoc->ExtendMerge( nStartCol,nStartRow, nEndCol,nEndRow, nPrintTab,
                        FALSE, TRUE );      // kein Refresh, incl. Attrs

    if ( bChangeCol )
    {
        OutputDevice* pRefDev = pDoc->GetPrinter();     // auch fuer Preview den Drucker nehmen
        pRefDev->SetMapMode( MAP_PIXEL );               // wichtig fuer GetNeededSize

        pDoc->ExtendPrintArea( pRefDev,
                            nPrintTab, nStartCol, nStartRow, nEndCol, nEndRow );
        //  nEndCol wird veraendert
    }

    if ( nEndCol < MAXCOL && pDoc->HasAttrib(
                    nEndCol,nStartRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HASATTR_SHADOW_RIGHT ) )
        ++nEndCol;
    if ( nEndRow < MAXROW && pDoc->HasAttrib(
                    nStartCol,nEndRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HASATTR_SHADOW_DOWN ) )
        ++nEndRow;

    if (!bChangeCol) nEndCol = nOldEndCol;
    if (!bChangeRow) nEndRow = nOldEndRow;

    return TRUE;
}

long ScPrintFunc::TextHeight( const EditTextObject* pObject )
{
    if (!pObject)
        return 0;

//  pEditEngine->SetPageNo( nTotalPages );
    pEditEngine->SetText( *pObject );

    return (long) pEditEngine->GetTextHeight();
}

//  nZoom muss gesetzt sein !!!
//  und der entsprechende Twip-MapMode eingestellt

void ScPrintFunc::UpdateHFHeight( ScPrintHFParam& rParam )
{
    DBG_ASSERT( aPageSize.Width(), "UpdateHFHeight ohne aPageSize");

    if (rParam.bEnable && rParam.bDynamic)
    {
        //  nHeight aus Inhalten berechnen

        MakeEditEngine();
        long nPaperWidth = ( aPageSize.Width() - nLeftMargin - nRightMargin -
                                rParam.nLeft - rParam.nRight ) * 100 / nZoom;
        if (rParam.pBorder)
            nPaperWidth -= ( 2*rParam.pBorder->GetDistance() +
                             lcl_LineTotal(rParam.pBorder->GetLeft()) +
                             lcl_LineTotal(rParam.pBorder->GetRight()) ) * 100 / nZoom;

        if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
            nPaperWidth -= ( rParam.pShadow->CalcShadowSpace(SHADOW_LEFT) +
                             rParam.pShadow->CalcShadowSpace(SHADOW_RIGHT) ) * 100L / nZoom;

        pEditEngine->SetPaperSize( Size( nPaperWidth, 10000 ) );

        long nMaxHeight = 0;
        if ( rParam.pLeft )
        {
            nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pLeft->GetLeftArea() ) );
            nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pLeft->GetCenterArea() ) );
            nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pLeft->GetRightArea() ) );
        }
        if ( rParam.pRight )
        {
            nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pRight->GetLeftArea() ) );
            nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pRight->GetCenterArea() ) );
            nMaxHeight = Max( nMaxHeight, TextHeight( rParam.pRight->GetRightArea() ) );
        }

        rParam.nHeight = nMaxHeight + rParam.nDistance;
        if (rParam.pBorder)
            rParam.nHeight += 2*rParam.pBorder->GetDistance() +
                              lcl_LineTotal( rParam.pBorder->GetTop() ) +
                              lcl_LineTotal( rParam.pBorder->GetBottom() );
        if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
            rParam.nHeight += rParam.pShadow->CalcShadowSpace(SHADOW_TOP) +
                              rParam.pShadow->CalcShadowSpace(SHADOW_BOTTOM);

        if (rParam.nHeight < rParam.nManHeight)
            rParam.nHeight = rParam.nManHeight;         // eingestelltes Minimum
    }
}

void ScPrintFunc::InitParam()
{
    if (!pParamSet)
        return;

                                // TabPage "Seite"
    const SvxLRSpaceItem* pLRItem = (const SvxLRSpaceItem*) &pParamSet->Get( ATTR_LRSPACE );
    long nTmp;
    nTmp = pLRItem->GetLeft();
    nLeftMargin = nTmp < 0 ? 0 : USHORT(nTmp);
    nTmp = pLRItem->GetRight();
    nRightMargin = nTmp < 0 ? 0 : USHORT(nTmp);
    const SvxULSpaceItem* pULItem = (const SvxULSpaceItem*) &pParamSet->Get( ATTR_ULSPACE );
    nTopMargin    = pULItem->GetUpper();
    nBottomMargin = pULItem->GetLower();

    const SvxPageItem* pPageItem = (const SvxPageItem*) &pParamSet->Get( ATTR_PAGE );
    nPageUsage          = pPageItem->GetPageUsage();
    bLandscape          = pPageItem->IsLandscape();
    aFieldData.eNumType = pPageItem->GetNumType();

    bCenterHor = ((const SfxBoolItem&) pParamSet->Get(ATTR_PAGE_HORCENTER)).GetValue();
    bCenterVer = ((const SfxBoolItem&) pParamSet->Get(ATTR_PAGE_VERCENTER)).GetValue();

    aPageSize = ((const SvxSizeItem&) pParamSet->Get(ATTR_PAGE_SIZE)).GetSize();
    if ( !aPageSize.Width() || !aPageSize.Height() )
    {
        DBG_ERROR("PageSize Null ?!?!?");
        aPageSize = SvxPaperInfo::GetPaperSize( SVX_PAPER_A4 );
    }

    pBorderItem     = (const SvxBoxItem*)    &pParamSet->Get(ATTR_BORDER);
    pBackgroundItem = (const SvxBrushItem*)  &pParamSet->Get(ATTR_BACKGROUND);
    pShadowItem     = (const SvxShadowItem*) &pParamSet->Get(ATTR_SHADOW);

                                // TabPage "Kopfzeile"

    aHdr.pLeft      = (const ScPageHFItem*) &pParamSet->Get(ATTR_PAGE_HEADERLEFT);      // Inhalt
    aHdr.pRight     = (const ScPageHFItem*) &pParamSet->Get(ATTR_PAGE_HEADERRIGHT);

    const SvxSetItem* pHeaderSetItem;
    const SfxItemSet* pHeaderSet = NULL;
    if ( pParamSet->GetItemState( ATTR_PAGE_HEADERSET, FALSE,
                            (const SfxPoolItem**)&pHeaderSetItem ) == SFX_ITEM_SET )
    {
        pHeaderSet = &pHeaderSetItem->GetItemSet();
                                                        // Kopfzeile hat unteren Abstand
        aHdr.nDistance  = ((const SvxULSpaceItem&) pHeaderSet->Get(ATTR_ULSPACE)).GetLower();
    }
    lcl_FillHFParam( aHdr, pHeaderSet );

                                // TabPage "Fusszeile"

    aFtr.pLeft      = (const ScPageHFItem*) &pParamSet->Get(ATTR_PAGE_FOOTERLEFT);      // Inhalt
    aFtr.pRight     = (const ScPageHFItem*) &pParamSet->Get(ATTR_PAGE_FOOTERRIGHT);

    const SvxSetItem* pFooterSetItem;
    const SfxItemSet* pFooterSet = NULL;
    if ( pParamSet->GetItemState( ATTR_PAGE_FOOTERSET, FALSE,
                            (const SfxPoolItem**)&pFooterSetItem ) == SFX_ITEM_SET )
    {
        pFooterSet = &pFooterSetItem->GetItemSet();
                                                        // Fusszeile hat oberen Abstand
        aFtr.nDistance  = ((const SvxULSpaceItem&) pFooterSet->Get(ATTR_ULSPACE)).GetUpper();
    }
    lcl_FillHFParam( aFtr, pFooterSet );

    //------------------------------------------------------
    // Table-/Area-Params aus einzelnen Items zusammenbauen:
    //------------------------------------------------------
    // TabPage "Tabelle"

    const SfxUInt16Item* pScaleItem         = NULL;
    const SfxUInt16Item* pScaleToPagesItem  = NULL;
    SfxItemState         eState;

    eState = pParamSet->GetItemState( ATTR_PAGE_SCALE, FALSE,
                                      (const SfxPoolItem**)&pScaleItem );
    if ( SFX_ITEM_DEFAULT == eState )
        pScaleItem = (const SfxUInt16Item*)
                     &pParamSet->GetPool()->
                        GetDefaultItem( ATTR_PAGE_SCALE );

    eState = pParamSet->GetItemState( ATTR_PAGE_SCALETOPAGES, FALSE,
                                      (const SfxPoolItem**)&pScaleToPagesItem );
    if ( SFX_ITEM_DEFAULT == eState )
        pScaleToPagesItem = (const SfxUInt16Item*)
                            &pParamSet->GetPool()->
                                GetDefaultItem( ATTR_PAGE_SCALETOPAGES );

    DBG_ASSERT( pScaleItem && pScaleToPagesItem, "Missing ScaleItem! :-/" );

    aTableParam.bNotes          = GET_BOOL(pParamSet,ATTR_PAGE_NOTES);
    aTableParam.bGrid           = GET_BOOL(pParamSet,ATTR_PAGE_GRID);
    aTableParam.bHeaders        = GET_BOOL(pParamSet,ATTR_PAGE_HEADERS);
    aTableParam.bFormulas       = GET_BOOL(pParamSet,ATTR_PAGE_FORMULAS);
    aTableParam.bNullVals       = GET_BOOL(pParamSet,ATTR_PAGE_NULLVALS);
    aTableParam.bCharts         = GET_SHOW(pParamSet,ATTR_PAGE_CHARTS);
    aTableParam.bObjects        = GET_SHOW(pParamSet,ATTR_PAGE_OBJECTS);
    aTableParam.bDrawings       = GET_SHOW(pParamSet,ATTR_PAGE_DRAWINGS);
    aTableParam.bTopDown        = GET_BOOL(pParamSet,ATTR_PAGE_TOPDOWN);
    aTableParam.bLeftRight      = !aTableParam.bLeftRight;
    aTableParam.nFirstPageNo    = GET_USHORT(pParamSet,ATTR_PAGE_FIRSTPAGENO);
    if (!aTableParam.nFirstPageNo)
        aTableParam.nFirstPageNo = (USHORT) nPageStart;     // von vorheriger Tabelle

    if ( pScaleItem && pScaleToPagesItem )
    {
        UINT16  nScaleAll     = pScaleItem->GetValue();
        UINT16  nScaleToPages = pScaleToPagesItem->GetValue();

        aTableParam.bScaleNone      = (nScaleAll     == 100);
        aTableParam.bScaleAll       = (nScaleAll      > 0  );
        aTableParam.bScalePageNum   = (nScaleToPages  > 0  );
        aTableParam.nScaleAll       = nScaleAll;
        aTableParam.nScalePageNum   = nScaleToPages;
    }
    else
    {
        aTableParam.bScaleNone      = TRUE;
        aTableParam.bScaleAll       = FALSE;
        aTableParam.bScalePageNum   = FALSE;
        aTableParam.nScaleAll       = 0;
        aTableParam.nScalePageNum   = 0;
    }

    aTableParam.bSkipEmpty = FALSE;     //! aus Seitenvorlage !!!!!!!!
//  aTableParam.bSkipEmpty = TRUE;      //! Test !!!!!!!!!!!!!!!!
    if ( pPageData )
        aTableParam.bSkipEmpty = FALSE;
    // Wenn pPageData gesetzt ist, interessieren fuer die Umbruch-Vorschau
    // nur die Umbrueche, leere Seiten werden nicht speziell behandelt

    //------------------------------------------------------
    // TabPage "Bereiche":
    //------------------------------------------------------

    //! alle PrintAreas der Tabelle durchgehen !!!
    const ScRange*  pPrintArea = pDoc->GetPrintRange( nPrintTab, 0 );
    const ScRange*  pRepeatCol = pDoc->GetRepeatColRange( nPrintTab );
    const ScRange*  pRepeatRow = pDoc->GetRepeatRowRange( nPrintTab );

    //  ATTR_PAGE_PRINTTABLES wird ignoriert

    if ( pUserArea )                // UserArea (Selektion) hat Vorrang
    {
        bPrintCurrentTable    =
        aAreaParam.bPrintArea = TRUE;                   // Selektion
        aAreaParam.aPrintArea = *pUserArea;

        //  Die Tabellen-Abfrage ist schon in DocShell::Print, hier immer
        aAreaParam.aPrintArea.aStart.SetTab(nPrintTab);
        aAreaParam.aPrintArea.aEnd.SetTab(nPrintTab);

//      lcl_LimitRange( aAreaParam.aPrintArea, nPrintTab );         // ganze Zeilen/Spalten...
    }
    else if ( pDoc->HasPrintRange() )
    {
        if ( pPrintArea )                               // mindestens eine gesetzt ?
        {
            bPrintCurrentTable    =
            aAreaParam.bPrintArea = TRUE;
            aAreaParam.aPrintArea = *pPrintArea;

            bMultiArea = ( pDoc->GetPrintRangeCount(nPrintTab) > 1 );
        }
        else
        {
            aAreaParam.bPrintArea = TRUE;
            bPrintCurrentTable = FALSE;
        }
    }
    else
    {
        //  #74834# don't print hidden tables if there's no print range defined there
        if ( pDoc->IsVisible( nPrintTab ) )
        {
            aAreaParam.bPrintArea = FALSE;
            bPrintCurrentTable = TRUE;
        }
        else
        {
            aAreaParam.bPrintArea = TRUE;   // otherwise the table is always counted
            bPrintCurrentTable = FALSE;
        }
    }

    if ( pRepeatCol )
    {
        aAreaParam.bRepeatCol = TRUE;
        aAreaParam.aRepeatCol = *pRepeatCol;
        nRepeatStartCol = pRepeatCol->aStart.Col();
        nRepeatEndCol   = pRepeatCol->aEnd  .Col();
    }
    else
    {
        aAreaParam.bRepeatCol = FALSE;
        nRepeatStartCol = nRepeatEndCol = REPEAT_NONE;
    }

    if ( pRepeatRow )
    {
        aAreaParam.bRepeatRow = TRUE;
        aAreaParam.aRepeatRow = *pRepeatRow;
        nRepeatStartRow = pRepeatRow->aStart.Row();
        nRepeatEndRow   = pRepeatRow->aEnd  .Row();
    }
    else
    {
        aAreaParam.bRepeatRow = FALSE;
        nRepeatStartRow = nRepeatEndRow = REPEAT_NONE;
    }

            //
            //  Seiten aufteilen
            //

    if (!bState)
    {
        nTabPages = CountPages();                                   // berechnet auch Zoom
        nTotalPages = nTabPages;
        nTotalPages += CountNotePages();
    }
    else
    {
        CalcPages();            // nur Umbrueche suchen
        CountNotePages();       // Notizen zaehlen, auch wenn Seitenzahl schon bekannt
    }

    if (nDocPages)
        aFieldData.nTotalPages = nDocPages;
    else
        aFieldData.nTotalPages = nTotalPages;

    SetDateTime( Date(), Time() );

    aFieldData.aTitle       = pDocShell->GetTitle();
    aFieldData.aLongDocName = pDocShell->GetMedium()->GetName();
    if ( !aFieldData.aLongDocName.Len() )
        aFieldData.aLongDocName = aFieldData.aTitle;
    aFieldData.aShortDocName= INetURLObject( aFieldData.aLongDocName ).GetName();

    //  Druckereinstellungen (Orientation, Paper) jetzt erst bei DoPrint
}

Size ScPrintFunc::GetDataSize() const
{
    Size aSize = aPageSize;
    aSize.Width()  -= nLeftMargin + nRightMargin;
    aSize.Height() -= nTopMargin + nBottomMargin;
    aSize.Height() -= aHdr.nHeight + aFtr.nHeight;
    return aSize;
}

void ScPrintFunc::GetScaleData( Size& rPhysSize, long& rDocHdr, long& rDocFtr )
{
    rPhysSize = aPageSize;
    rPhysSize.Width()  -= nLeftMargin + nRightMargin;
    rPhysSize.Height() -= nTopMargin + nBottomMargin;

    rDocHdr = aHdr.nHeight;
    rDocFtr = aFtr.nHeight;
}

void ScPrintFunc::SetDateTime( const Date& rDate, const Time& rTime )
{
    aFieldData.aDate = rDate;
    aFieldData.aTime = rTime;
}

void lcl_DrawGraphic( const Graphic &rGraphic, OutputDevice *pOut,
                        const Rectangle &rGrf, const Rectangle &rOut )
{
    const FASTBOOL bNotInside = !rOut.IsInside( rGrf );
    if ( bNotInside )
    {
        pOut->Push();
        pOut->IntersectClipRegion( rOut );
    }

    ((Graphic&)rGraphic).Draw( pOut, rGrf.TopLeft(), rGrf.GetSize() );

    if ( bNotInside )
        pOut->Pop();
}

void lcl_DrawGraphic( const Bitmap& rBitmap, OutputDevice *pOut,
                        const Rectangle &rGrf, const Rectangle &rOut )
{
    const FASTBOOL bNotInside = !rOut.IsInside( rGrf );
    if ( bNotInside )
    {
        pOut->Push();
        pOut->IntersectClipRegion( rOut );
    }

    pOut->DrawBitmap( rGrf.TopLeft(), rGrf.GetSize(), rBitmap );

    if ( bNotInside )
        pOut->Pop();
}

void lcl_DrawGraphic( const SvxBrushItem &rBrush, OutputDevice *pOut,
                        const Rectangle &rOrg, const Rectangle &rOut )
{
    Size aGrfSize(0,0);
    const Graphic *pGraphic = rBrush.GetGraphic();
    SvxGraphicPosition ePos;
    if ( pGraphic && pGraphic->IsSupportedGraphic() )
    {
        const MapMode aMapMM( MAP_100TH_MM );
        if ( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
        {
            MapMode aOldMap( pOut->GetMapMode() );
            pOut->SetMapMode( aMapMM );
            aGrfSize = pOut->PixelToLogic( pGraphic->GetPrefSize() );
            pOut->SetMapMode( aOldMap );
        }
        else
            aGrfSize = OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                    pGraphic->GetPrefMapMode(), aMapMM );
        ePos = rBrush.GetGraphicPos();
    }
    else
        ePos = GPOS_NONE;

    Point aPos;
    Size aSize = aGrfSize;

    FASTBOOL bDraw = TRUE;
//  FASTBOOL bRetouche = TRUE;
    switch ( ePos )
    {
        case GPOS_LT: aPos = rOrg.TopLeft();
                      break;
        case GPOS_MT: aPos.Y() = rOrg.Top();
                      aPos.X() = rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2;
                      break;
        case GPOS_RT: aPos.Y() = rOrg.Top();
                      aPos.X() = rOrg.Right() - aGrfSize.Width();
                      break;

        case GPOS_LM: aPos.Y() = rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2;
                      aPos.X() = rOrg.Left();
                      break;
        case GPOS_MM: aPos.Y() = rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2;
                      aPos.X() = rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2;
                      break;
        case GPOS_RM: aPos.Y() = rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2;
                      aPos.X() = rOrg.Right() - aGrfSize.Width();
                      break;

        case GPOS_LB: aPos.Y() = rOrg.Bottom() - aGrfSize.Height();
                      aPos.X() = rOrg.Left();
                      break;
        case GPOS_MB: aPos.Y() = rOrg.Bottom() - aGrfSize.Height();
                      aPos.X() = rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2;
                      break;
        case GPOS_RB: aPos.Y() = rOrg.Bottom() - aGrfSize.Height();
                      aPos.X() = rOrg.Right() - aGrfSize.Width();
                      break;

        case GPOS_AREA:
                      aPos = rOrg.TopLeft();
                      aSize = rOrg.GetSize();
//                    bRetouche = FALSE;
                      break;
        case GPOS_TILED:
                    {
                        const FASTBOOL bUseQuickBmp = OUTDEV_PRINTER != pOut->GetOutDevType() &&
                                                          !pOut->GetConnectMetaFile();
                        Bitmap* pQuickDrawBmp = NULL;
                        if (bUseQuickBmp)
                        {
                            pQuickDrawBmp = new Bitmap;
                            *pQuickDrawBmp = XOutBitmap::CreateQuickDrawBitmapEx(
                                             *pGraphic, *pOut, pOut->GetMapMode(),
                                             aGrfSize, Point(), aGrfSize ).GetBitmap();
                        }
                        aPos  = rOrg.TopLeft();
                        aSize = aGrfSize;
                        do
                        {
                            do
                            {
                                Rectangle aGrf( aPos,aSize );
                                if ( aGrf.IsOver( rOut ) )
                                {
                                    if (bUseQuickBmp)
                                        lcl_DrawGraphic( *pQuickDrawBmp, pOut, aGrf, rOut );
                                    else
                                        lcl_DrawGraphic( *pGraphic, pOut, aGrf, rOut );
                                }
                                aPos.X() += aGrfSize.Width();
                            }
                            while ( aPos.X() < rOut.Right() );

                            aPos.X() = rOrg.Left();
                            aPos.Y() += aGrfSize.Height();

                        }
                        while ( aPos.Y() < rOut.Bottom() ) ;
                        bDraw = FALSE;
//                      bRetouche = FALSE;

                        delete pQuickDrawBmp;
                    }
                    break;

        case GPOS_NONE:
                      bDraw = FALSE;
                      break;

        default: DBG_ASSERT( !pOut, "new Graphic position?" );
    }
    Rectangle aGrf( aPos,aSize );
    if ( bDraw && aGrf.IsOver( rOut ) )
    {
        lcl_DrawGraphic( *pGraphic, pOut, aGrf, rOut );
    }
}

//  Rahmen wird nach innen gezeichnet

void ScPrintFunc::DrawBorder( long nScrX, long nScrY, long nScrW, long nScrH,
                                const SvxBoxItem* pBorderData, const SvxBrushItem* pBackground,
                                const SvxShadowItem* pShadow )
{
    //!     direkte Ausgabe aus SvxBoxItem !!!

    if (pBorderData)
        if ( !pBorderData->GetTop() && !pBorderData->GetBottom() && !pBorderData->GetLeft() &&
                                        !pBorderData->GetRight() )
            pBorderData = NULL;

    if (!pBorderData && !pBackground && !pShadow)
        return;                                     // nichts zu tun

    long nLeft   = 0;
    long nRight  = 0;
    long nTop    = 0;
    long nBottom = 0;

    //  aFrameRect - aussen um die Umrandung, ohne Schatten
    if ( pShadow && pShadow->GetLocation() != SVX_SHADOW_NONE )
    {
        nLeft   += (long) ( pShadow->CalcShadowSpace(SHADOW_LEFT)   * nScaleX );
        nRight  += (long) ( pShadow->CalcShadowSpace(SHADOW_RIGHT)  * nScaleX );
        nTop    += (long) ( pShadow->CalcShadowSpace(SHADOW_TOP)    * nScaleY );
        nBottom += (long) ( pShadow->CalcShadowSpace(SHADOW_BOTTOM) * nScaleY );
    }
    Rectangle aFrameRect( Point(nScrX+nLeft, nScrY+nTop),
                          Size(nScrW-nLeft-nRight, nScrH-nTop-nBottom) );

    //  Mitte der Umrandung, um Linien ueber OutputData zu zeichnen:
    if (pBorderData)
    {
        nLeft   += (long) ( lcl_LineTotal(pBorderData->GetLeft())   * nScaleX / 2 );
        nRight  += (long) ( lcl_LineTotal(pBorderData->GetRight())  * nScaleX / 2 );
        nTop    += (long) ( lcl_LineTotal(pBorderData->GetTop())    * nScaleY / 2 );
        nBottom += (long) ( lcl_LineTotal(pBorderData->GetBottom()) * nScaleY / 2 );
    }
    long nEffHeight = nScrH - nTop - nBottom;
    long nEffWidth = nScrW - nLeft - nRight;
    if (nEffHeight<=0 || nEffWidth<=0)
        return;                                         // leer

    if (pBackground)
    {
//      Rectangle aBackRect( Point(nScrX+nLeft, nScrY+nTop), Size(nEffWidth,nEffHeight) );
        if (pBackground->GetGraphicPos() != GPOS_NONE)
            lcl_DrawGraphic( *pBackground, pDev, aFrameRect, aFrameRect );
        else
        {
            pDev->SetFillColor(pBackground->GetColor());
            pDev->SetLineColor();
            pDev->DrawRect(aFrameRect);
        }
    }

    if ( pShadow && pShadow->GetLocation() != SVX_SHADOW_NONE )
    {
        pDev->SetFillColor(pShadow->GetColor());
        pDev->SetLineColor();
        long nShadowX = (long) ( pShadow->GetWidth() * nScaleX );
        long nShadowY = (long) ( pShadow->GetWidth() * nScaleY );
        switch (pShadow->GetLocation())
        {
            case SVX_SHADOW_TOPLEFT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()-nShadowX, aFrameRect.Top() ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Left(), aFrameRect.Bottom()-nShadowY ) );
                break;
            case SVX_SHADOW_TOPRIGHT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()+nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Top() ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Right(), aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()-nShadowY ) );
                break;
            case SVX_SHADOW_BOTTOMLEFT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Bottom(),
                        aFrameRect.Right()-nShadowX, aFrameRect.Bottom()+nShadowY ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()+nShadowY,
                        aFrameRect.Left(), aFrameRect.Bottom()+nShadowY ) );
                break;
            case SVX_SHADOW_BOTTOMRIGHT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()+nShadowX, aFrameRect.Bottom(),
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()+nShadowY ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Right(), aFrameRect.Top()+nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()+nShadowY ) );
                break;
        }
    }

    USHORT i;
    if (pBorderData)
    {
        ScDocument* pBorderDoc = new ScDocument( SCDOCMODE_UNDO );
        pBorderDoc->InitUndo( pDoc, 0,0, TRUE,TRUE );
        if (pBorderData)
            pBorderDoc->ApplyAttr( 0,0,0, *pBorderData );

        RowInfo* pRowInfo = new RowInfo[ROWINFO_MAX];
        USHORT nArrCount = pBorderDoc->FillInfo( pRowInfo, 0,0, 0,0, 0,
                                            nScaleX, nScaleY, FALSE, FALSE );
        DBG_ASSERT(nArrCount,"nArrCount == 0");

        pRowInfo[1].nHeight = (USHORT) nEffHeight;
        pRowInfo[0].pCellInfo[1].nWidth =
            pRowInfo[1].pCellInfo[1].nWidth = (USHORT) nEffWidth;

        ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, pRowInfo, nArrCount, pBorderDoc, 0,
                                    nScrX+nLeft, nScrY+nTop, 0,0, 0,0, nScaleX, nScaleY );

//      pDev->SetMapMode(aTwipMode);

        if (pBorderData)
            aOutputData.DrawFrame();

        for (i=0; i<nArrCount; i++)
            delete[] pRowInfo[i].pCellInfo;
        delete[] pRowInfo;

        delete pBorderDoc;
    }
}

void ScPrintFunc::PrintColHdr( USHORT nX1, USHORT nX2, long nScrX, long nScrY )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    long nHeight = (long) (PRINT_HEADER_HEIGHT * nScaleY);
    long nEndY = nScrY + nHeight - nOneY;

    long nPosX = nScrX - nOneX;
    long nPosY = nScrY - nOneY;
    String aText;

    for (USHORT nCol=nX1; nCol<=nX2; nCol++)
    {
        USHORT nDocW = pDoc->GetColWidth( nCol, nPrintTab );
        if (nDocW)
        {
            long nWidth = (long) (nDocW * nScaleX);
            long nEndX = nPosX + nWidth;

            pDev->DrawRect( Rectangle( nPosX,nPosY,nEndX,nEndY ) );

            if ( nCol < 26 )
                aText = (sal_Unicode) ( 'A' + nCol );
            else
            {
                aText = (sal_Unicode) ( 'A' + ( nCol / 26 ) - 1 );
                aText += (sal_Unicode) ( 'A' + ( nCol % 26 ) );
            }
            long nTextWidth = pDev->GetTextWidth(aText);
            long nTextHeight = pDev->GetTextHeight();
            long nAddX = ( nWidth  - nTextWidth  ) / 2;
            long nAddY = ( nHeight - nTextHeight ) / 2;
            pDev->DrawText( Point( nPosX+nAddX,nPosY+nAddY ), aText );

            nPosX = nEndX;
        }
    }
}

void ScPrintFunc::PrintRowHdr( USHORT nY1, USHORT nY2, long nScrX, long nScrY )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    long nWidth = (long) (PRINT_HEADER_WIDTH * nScaleX);
    long nEndX = nScrX + nWidth - nOneX;

    long nPosX = nScrX - nOneX;
    long nPosY = nScrY - nOneY;
    String aText;

    for (USHORT nRow=nY1; nRow<=nY2; nRow++)
    {
        USHORT nDocH = pDoc->FastGetRowHeight( nRow, nPrintTab );
        if (nDocH)
        {
            long nHeight = (long) (nDocH * nScaleY);
            long nEndY = nPosY + nHeight;

            pDev->DrawRect( Rectangle( nPosX,nPosY,nEndX,nEndY ) );

            aText = String::CreateFromInt32( nRow+1 );
            long nTextWidth = pDev->GetTextWidth(aText);
            long nTextHeight = pDev->GetTextHeight();
            long nAddX = ( nWidth  - nTextWidth  ) / 2;
            long nAddY = ( nHeight - nTextHeight ) / 2;
            pDev->DrawText( Point( nPosX+nAddX,nPosY+nAddY ), aText );

            nPosY = nEndY;
        }
    }
}

void lcl_HidePrint( RowInfo* pRowInfo, USHORT nArrCount, USHORT nX1, USHORT nX2 )
{
    for (USHORT nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        for (USHORT nX=nX1; nX<=nX2; nX++)
        {
            const CellInfo& rCellInfo = pThisRowInfo->pCellInfo[nX+1];
            if (!rCellInfo.bEmptyCellText)
                if (((const ScProtectionAttr&)rCellInfo.pPatternAttr->
                            GetItem(ATTR_PROTECTION, rCellInfo.pConditionSet)).GetHidePrint())
                {
                    pThisRowInfo->pCellInfo[nX+1].pCell          = NULL;
                    pThisRowInfo->pCellInfo[nX+1].bEmptyCellText = TRUE;
                }
        }
    }
}

void ScPrintFunc::PrintArea( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                                long nScrX, long nScrY,
                                BOOL bShLeft, BOOL bShTop, BOOL bShRight, BOOL bShBottom )
{
    USHORT i;

                            //!     Flag bei FillInfo uebergeben !!!!!
    ScTripel aEStart;
    ScTripel aEEnd;
    BOOL bEmbed = pDoc->IsEmbedded();
    if (bEmbed)
    {
        pDoc->GetEmbedded(aEStart,aEEnd);
        pDoc->ResetEmbedded();
    }

    Point aPos = OutputDevice::LogicToLogic(Point(nScrX,nScrY), aOffsetMode, aLogicMode);
    long nLogStX = aPos.X();
    long nLogStY = aPos.Y();

                    //  Daten zusammenstellen

    RowInfo* pRowInfo = new RowInfo[ROWINFO_MAX];
    USHORT nArrCount = pDoc->FillInfo( pRowInfo, nX1, nY1, nX2, nY2, nPrintTab,
                                        nScaleX, nScaleY, TRUE, aTableParam.bFormulas );
    lcl_HidePrint( pRowInfo, nArrCount, nX1, nX2 );

    if (bEmbed)
        pDoc->SetEmbedded(aEStart,aEEnd);

    ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, pRowInfo, nArrCount, pDoc, nPrintTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nScaleX, nScaleY );

    if (nObjectFlags)
    {
        pDev->SetMapMode(aLogicMode);
        //  hier kein Clipping setzen (Mapmode wird verschoben)
        aOutputData.DrawingLayer(SC_LAYER_BACK,nObjectFlags,nLogStX,nLogStY);
    }

    pDev->SetMapMode(aOffsetMode);

    aOutputData.SetShowFormulas( aTableParam.bFormulas );
    aOutputData.SetShowNullValues( aTableParam.bNullVals );

    if (!pPrinter)
    {
        OutputDevice* pRefDev = pDoc->GetPrinter();     // auch fuer Preview den Drucker nehmen
        Fraction aPrintFrac( nZoom, 100 );              // ohne nManualZoom
        //  MapMode, wie er beim Drucken herauskommen wuerde:
        pRefDev->SetMapMode( MapMode( MAP_100TH_MM, Point(), aPrintFrac, aPrintFrac ) );
        aOutputData.SetRefDevice( pRefDev );
    }

//  aOutputData.SetMetaFileMode(TRUE);
    aOutputData.DrawBackground();

    pDev->SetClipRegion( Rectangle( aPos, Size( aOutputData.GetScrW(), aOutputData.GetScrH() ) ) );
    SFX_APP()->SpoilDemoOutput( *pDev, OutputDevice::LogicToLogic(
                                aPageRect, aTwipMode, aOffsetMode ) );
    pDev->SetClipRegion();
//
    aOutputData.SetMetaFileMode(FALSE);
    aOutputData.DrawExtraShadow( bShLeft, bShTop, bShRight, bShBottom );
    aOutputData.DrawFrame();
    aOutputData.DrawStrings();

//  pDev->SetMapMode(aLogicMode);
    aOutputData.DrawEdit(FALSE,1.0,1.0);

//  pDev->SetMapMode(aOffsetMode);
    if (aTableParam.bGrid)
        aOutputData.DrawGrid( TRUE, FALSE );    // keine Seitenumbrueche

/*!!!!!!!!!!!       Notizen in Tabelle markieren ??????????????????????????

    if (aTableParam.bNotes)
    {
        pDev->SetMapMode(aOffsetMode);
        aOutputData.PrintNoteMarks(aNotePosList);
        pDev->SetMapMode(aLogicMode);
    }
*/

//  pDev->SetMapMode(aDrawMode);
    if (nObjectFlags)
        aOutputData.DrawingLayer(SC_LAYER_FRONT,nObjectFlags,nLogStX,nLogStY);
    aOutputData.DrawingLayer(SC_LAYER_INTERN,SC_OBJECTS_ALL,nLogStX,nLogStY);

    if ( pDrawView && (nObjectFlags & SC_OBJECTS_DRAWING) )
    {
        SdrPageView* pPV = pDrawView->GetPageViewPgNum(nPrintTab);
        DBG_ASSERT(pPV, "keine PageView fuer gedruckte Tabelle");
        if (pPV)
        {
            Rectangle aLogicRect = pDoc->GetMMRect( nX1,nY1, nX2,nY2, nPrintTab );
            Point aControlOffset( nLogStX - aLogicRect.Left(), nLogStY - aLogicRect.Top() );
            MapMode aControlMode( MAP_100TH_MM, aControlOffset,
                                    aLogicMode.GetScaleX(), aLogicMode.GetScaleY() );
            pDev->SetMapMode( aControlMode );
            pPV->RedrawOneLayer( SC_LAYER_CONTROLS, aLogicRect );
        }
    }

    for (i=0; i<nArrCount; i++)
        delete[] pRowInfo[i].pCellInfo;
    delete[] pRowInfo;
}

BOOL ScPrintFunc::IsMirror( long nPageNo )          // Raender spiegeln ?
{
    SvxPageUsage eUsage = (SvxPageUsage) ( nPageUsage & 0x000f );
    return ( eUsage == SVX_PAGE_MIRROR && (nPageNo & 1) );
}

BOOL ScPrintFunc::IsLeft( long nPageNo )            // linke Fussnoten ?
{
    SvxPageUsage eUsage = (SvxPageUsage) ( nPageUsage & 0x000f );
    BOOL bLeft;
    if (eUsage == SVX_PAGE_LEFT)
        bLeft = TRUE;
    else if (eUsage == SVX_PAGE_RIGHT)
        bLeft = FALSE;
    else
        bLeft = (nPageNo & 1) != 0;
    return bLeft;
}

void ScPrintFunc::MakeTableString()
{
    pDoc->GetName( nPrintTab, aFieldData.aTabName );
}

String lcl_CmdStr( USHORT nId )
{
    String aDel( ScGlobal::GetRscString( STR_HFCMD_DELIMITER ) );
    String aCmd = aDel;
    aCmd += ScGlobal::GetRscString( nId );
    aCmd += aDel;
    return aCmd;
}

void ScPrintFunc::MakeEditEngine()
{
    if (!pEditEngine)
    {
        Font aDefFont;
        ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).GetFont(aDefFont);
        //  #69193# dont use font color, because background color is not used
        //! there's no way to set the background for note pages
        aDefFont.SetColor( COL_BLACK );

        //  can't use document's edit engine pool here,
        //  because pool must have twips as default metric
        pEditEngine = new ScHeaderEditEngine( EditEngine::CreatePool(), TRUE );

        pEditEngine->EnableUndo(FALSE);
        pEditEngine->SetRefDevice( pDev );
        pEditEngine->SetWordDelimiters(
                ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );
        pEditEngine->SetControlWord( pEditEngine->GetControlWord() & ~EE_CNTRL_RTFSTYLESHEETS );

        //  Default-Set fuer Ausrichtung
        pEditDefaults = new SfxItemSet( pEditEngine->GetEmptyItemSet() );

        EditEngine::SetFontInfoInItemSet( *pEditDefaults, aDefFont );
    }

    pEditEngine->SetData( aFieldData );     // Seitennummer etc. setzen
}

//  nStartY = logic
void ScPrintFunc::PrintHF( long nPageNo, const ScPrintHFParam& rParam, long nStartY )
{
    pDev->SetMapMode( aTwipMode );          // Kopf-/Fusszeilen in Twips

    BOOL bLeft = IsLeft(nPageNo) && !rParam.bShared;
    const ScPageHFItem* pHFItem = bLeft ? rParam.pLeft : rParam.pRight;

    long nLineStartX = aPageRect.Left()  + rParam.nLeft;
    long nLineEndX   = aPageRect.Right() - rParam.nRight;
    long nLineWidth  = nLineEndX - nLineStartX + 1;

    //  Edit-Engine

    Point aStart( nLineStartX, nStartY );
    Size aPaperSize( nLineWidth, rParam.nHeight-rParam.nDistance );
    if ( rParam.pBorder )
    {
        long nLeft = lcl_LineTotal( rParam.pBorder->GetLeft() ) + rParam.pBorder->GetDistance();
        long nTop = lcl_LineTotal( rParam.pBorder->GetTop() ) + rParam.pBorder->GetDistance();
        aStart.X() += nLeft;
        aStart.Y() += nTop;
        aPaperSize.Width() -= nLeft + lcl_LineTotal( rParam.pBorder->GetRight() ) + rParam.pBorder->GetDistance();
        aPaperSize.Height() -= nTop + lcl_LineTotal( rParam.pBorder->GetBottom() ) + rParam.pBorder->GetDistance();
    }

    if ( rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE )
    {
        long nLeft  = rParam.pShadow->CalcShadowSpace(SHADOW_LEFT);
        long nTop   = rParam.pShadow->CalcShadowSpace(SHADOW_TOP);
        aStart.X() += nLeft;
        aStart.Y() += nTop;
        aPaperSize.Width() -= nLeft + rParam.pShadow->CalcShadowSpace(SHADOW_RIGHT);
        aPaperSize.Height() -= nTop + rParam.pShadow->CalcShadowSpace(SHADOW_BOTTOM);
    }

    aFieldData.nPageNo = nPageNo+aTableParam.nFirstPageNo;
    MakeEditEngine();

    pEditEngine->SetPaperSize(aPaperSize);
    const EditTextObject* pObject;

    //  Rahmen / Hintergrund

    Point aBorderStart( nLineStartX, nStartY );
    Size aBorderSize( nLineWidth, rParam.nHeight-rParam.nDistance );
    if ( rParam.bDynamic )
    {
        //  hier nochmal anpassen, wegen geraden/ungeraden Kopf/Fusszeilen
        //  und evtl. anderen Umbruechen durch Variablen (Seitennummer etc.)

        long nMaxHeight = 0;
        nMaxHeight = Max( nMaxHeight, TextHeight( pHFItem->GetLeftArea() ) );
        nMaxHeight = Max( nMaxHeight, TextHeight( pHFItem->GetCenterArea() ) );
        nMaxHeight = Max( nMaxHeight, TextHeight( pHFItem->GetRightArea() ) );
        if (rParam.pBorder)
            nMaxHeight += lcl_LineTotal( rParam.pBorder->GetTop() ) +
                          lcl_LineTotal( rParam.pBorder->GetBottom() ) +
                                    2*rParam.pBorder->GetDistance();
        if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
            nMaxHeight += rParam.pShadow->CalcShadowSpace(SHADOW_TOP) +
                          rParam.pShadow->CalcShadowSpace(SHADOW_BOTTOM);

        if (nMaxHeight < rParam.nManHeight-rParam.nDistance)
            nMaxHeight = rParam.nManHeight-rParam.nDistance;        // eingestelltes Minimum

        aBorderSize.Height() = nMaxHeight;
    }

    double nOldScaleX = nScaleX;
    double nOldScaleY = nScaleY;
    nScaleX = nScaleY = 1.0;            // direkt in Twips ausgeben
    DrawBorder( aBorderStart.X(), aBorderStart.Y(), aBorderSize.Width(), aBorderSize.Height(),
                    rParam.pBorder, rParam.pBack, rParam.pShadow );
    nScaleX = nOldScaleX;
    nScaleY = nOldScaleY;

    //  Clipping fuer Text

    pDev->SetClipRegion( Rectangle( aStart, aPaperSize ) );

    //  links

    pObject = pHFItem->GetLeftArea();
    if (pObject)
    {
        pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
        pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, FALSE );
        Point aDraw = aStart;
        long nDif = aPaperSize.Height() - (long) pEditEngine->GetTextHeight();
        if (nDif > 0)
            aDraw.Y() += nDif / 2;
        pEditEngine->Draw( pDev, aDraw, 0 );
    }

    //  Mitte

    pObject = pHFItem->GetCenterArea();
    if (pObject)
    {
        pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
        pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, FALSE );
        Point aDraw = aStart;
        long nDif = aPaperSize.Height() - (long) pEditEngine->GetTextHeight();
        if (nDif > 0)
            aDraw.Y() += nDif / 2;
        pEditEngine->Draw( pDev, aDraw, 0 );
    }

    //  rechts

    pObject = pHFItem->GetRightArea();
    if (pObject)
    {
        pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
        pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, FALSE );
        Point aDraw = aStart;
        long nDif = aPaperSize.Height() - (long) pEditEngine->GetTextHeight();
        if (nDif > 0)
            aDraw.Y() += nDif / 2;
        pEditEngine->Draw( pDev, aDraw, 0 );
    }

    pDev->SetClipRegion();
}

long ScPrintFunc::DoNotes( long nNoteStart, BOOL bDoPrint )
{
    if (bDoPrint)
        pDev->SetMapMode(aTwipMode);

    MakeEditEngine();
    pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
    pEditEngine->SetDefaults( *pEditDefaults );

    Font aMarkFont;
    ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).GetFont(aMarkFont);
//? aMarkFont.SetWeight( WEIGHT_BOLD );
    pDev->SetFont( aMarkFont );
    long nMarkLen = pDev->GetTextWidth(
            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("GW99999:")));
    // ohne Space, weil's eh selten so weit kommt

    Size aDataSize = aPageRect.GetSize();
    if ( nMarkLen > aDataSize.Width() / 2 )     // alles viel zu klein?
        nMarkLen = aDataSize.Width() / 2;       // Seite bruederlich aufteilen
    aDataSize.Width() -= nMarkLen;

    pEditEngine->SetPaperSize( aDataSize );
    long nPosX = aPageRect.Left() + nMarkLen;
    long nPosY = aPageRect.Top();

    long nCount = 0;
    BOOL bOk;
    do
    {
        bOk = FALSE;
        ScTripel* pPos = (ScTripel*) aNotePosList.GetObject( nNoteStart+nCount );
        if (pPos)
        {
            ScBaseCell* pCell;
            pDoc->GetCell( pPos->GetCol(), pPos->GetRow(), pPos->GetTab(), pCell );
            const ScPostIt* pNote = pCell->GetNotePtr();
            if (pNote)
            {
                String aString = pNote->GetText();
                pEditEngine->SetText(aString);
                long nTextHeight = pEditEngine->GetTextHeight();
                if ( nPosY + nTextHeight < aPageRect.Bottom() )
                {
                    if (bDoPrint)
                    {
                        pEditEngine->Draw( pDev, Point( nPosX, nPosY ), 0 );

                        ScAddress aAddress( pPos->GetCol(), pPos->GetRow(), pPos->GetTab() );
                        String aMarkStr;
                        aAddress.Format( aMarkStr, SCA_VALID, pDoc );
                        aMarkStr += ':';

                        //  Zellposition auch per EditEngine, damit die Position stimmt
                        pEditEngine->SetText(aMarkStr);
                        pEditEngine->Draw( pDev, Point( aPageRect.Left(), nPosY ), 0 );
                    }

                    nPosY += nTextHeight;
                    nPosY += 200;                   // Abstand
                    ++nCount;
                    bOk = TRUE;
                }
            }
        }
    }
    while (bOk);

    return nCount;
}

long ScPrintFunc::PrintNotes( long nPageNo, long nNoteStart, BOOL bDoPrint )
{
    if ( nNoteStart >= (long) aNotePosList.Count() || !aTableParam.bNotes )
        return 0;

    if ( bDoPrint && bClearWin )
    {
        //! mit PrintPage zusammenfassen !!!

        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor();
        pDev->SetFillColor(rStyleSettings.GetWindowColor());
        pDev->DrawRect(Rectangle(Point(),
                Size((long)(aPageSize.Width() * nScaleX * 100 / nZoom),
                     (long)(aPageSize.Height() * nScaleY * 100 / nZoom))));
    }


    //      aPageRect auf linke / rechte Seiten anpassen

    Rectangle aTempRect = Rectangle( Point(), aPageSize );
    if (IsMirror(nPageNo))
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nRightMargin ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nLeftMargin  ) * 100 / nZoom;
    }
    else
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nLeftMargin  ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nRightMargin ) * 100 / nZoom;
    }

    if (bDoPrint)
    {
        if (pPrinter)
            pPrinter->StartPage();

        //  Kopf- und Fusszeilen

        if (aHdr.bEnable)
        {
            long nHeaderY = aPageRect.Top()-aHdr.nHeight;
            PrintHF( nPageNo, aHdr, nHeaderY );
        }
        if (aFtr.bEnable)
        {
            long nFooterY = aPageRect.Bottom()+aFtr.nDistance;
            PrintHF( nPageNo, aFtr, nFooterY );
        }
    }

    long nCount = DoNotes( nNoteStart, bDoPrint );

    if (pPrinter && bDoPrint)
        pPrinter->EndPage();

    return nCount;
}

void ScPrintFunc::PrintPage( long nPageNo, USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 )
{
    //  nPageNo - Seitennummer innerhalb einer "Startseite"-Einstellung

    if (bClearWin)
    {
        //  muss genau zum Zeichnen des Rahmens in preview.cxx passen !!!

        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor();
        pDev->SetFillColor(rStyleSettings.GetWindowColor());
        pDev->DrawRect(Rectangle(Point(),
                Size((long)(aPageSize.Width() * nScaleX * 100 / nZoom),
                     (long)(aPageSize.Height() * nScaleY * 100 / nZoom))));
    }


    //      aPageRect auf linke / rechte Seiten anpassen

    Rectangle aTempRect = Rectangle( Point(), aPageSize );
    if (IsMirror(nPageNo))
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nRightMargin ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nLeftMargin  ) * 100 / nZoom;
    }
    else
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nLeftMargin  ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nRightMargin ) * 100 / nZoom;
    }

    if ( aAreaParam.bRepeatCol )
        if ( nX1 > nRepeatStartCol && nX1 <= nRepeatEndCol )
            nX1 = nRepeatEndCol + 1;
    BOOL bDoRepCol = (aAreaParam.bRepeatCol && nX1 > nRepeatEndCol);
    if ( aAreaParam.bRepeatRow )
        if ( nY1 > nRepeatStartRow && nY1 <= nRepeatEndRow )
            nY1 = nRepeatEndRow + 1;
    BOOL bDoRepRow = (aAreaParam.bRepeatRow && nY1 > nRepeatEndRow);

    nObjectFlags = 0;
    if ( aTableParam.bDrawings )
        nObjectFlags |= SC_OBJECTS_DRAWING;
    if ( aTableParam.bObjects )
        nObjectFlags |= SC_OBJECTS_OLE;
    if ( aTableParam.bCharts )
        nObjectFlags |= SC_OBJECTS_CHARTS;

    USHORT i;

    if (pPrinter)
        pPrinter->StartPage();

    //  Kopf- und Fusszeilen (ohne Zentrierung)

    if (aHdr.bEnable)
    {
        long nHeaderY = aPageRect.Top()-aHdr.nHeight;
        PrintHF( nPageNo, aHdr, nHeaderY );
    }
    if (aFtr.bEnable)
    {
        long nFooterY = aPageRect.Bottom()+aFtr.nDistance;
        PrintHF( nPageNo, aFtr, nFooterY );
    }

    //  Position ( Raender / zentrieren )

    long nLeftSpace = aPageRect.Left();     // Document-Twips
    long nTopSpace  = aPageRect.Top();
    if ( bCenterHor )
    {
        long nDataWidth = 0;
        for (i=nX1; i<=nX2; i++)
            nDataWidth += pDoc->GetColWidth( i,nPrintTab );
        if (bDoRepCol)
            for (i=nRepeatStartCol; i<=nRepeatEndCol; i++)
                nDataWidth += pDoc->GetColWidth( i,nPrintTab );
        if (aTableParam.bHeaders)
            nDataWidth += (long) PRINT_HEADER_WIDTH;
        if (pBorderItem)
            nDataWidth += 2 * pBorderItem->GetDistance();       //! Linenstaerke ???
        if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
            nDataWidth += pShadowItem->CalcShadowSpace(SHADOW_LEFT) +
                           pShadowItem->CalcShadowSpace(SHADOW_RIGHT);
        nLeftSpace += ( aPageRect.GetWidth() - nDataWidth ) / 2;
        if (pBorderItem)
            nLeftSpace -= lcl_LineTotal(pBorderItem->GetLeft());
    }
    if ( bCenterVer )
    {
        long nDataHeight = 0;
        for (i=nY1; i<=nY2; i++)
            nDataHeight += pDoc->FastGetRowHeight( i,nPrintTab );
        if (bDoRepRow)
            for (i=nRepeatStartRow; i<=nRepeatEndRow; i++)
                nDataHeight += pDoc->FastGetRowHeight( i,nPrintTab );
        if (aTableParam.bHeaders)
            nDataHeight += (long) PRINT_HEADER_HEIGHT;
        if (pBorderItem)
            nDataHeight += 2 * pBorderItem->GetDistance();      //! Linenstaerke ???
        if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
            nDataHeight += pShadowItem->CalcShadowSpace(SHADOW_TOP) +
                           pShadowItem->CalcShadowSpace(SHADOW_BOTTOM);
        nTopSpace += ( aPageRect.GetHeight() - nDataHeight ) / 2;
        if (pBorderItem)
            nTopSpace -= lcl_LineTotal(pBorderItem->GetTop());
    }

    //  Seite aufteilen

    long nStartX = ((long) ( nLeftSpace * nScaleX ));
    long nStartY = ((long) ( nTopSpace  * nScaleY ));
//      nStartX -= aOffset.X();         // schon im MapMode
//      nStartY -= aOffset.Y();

    long nInnerStartX = nStartX;
    long nInnerStartY = nStartY;
    if (pBorderItem)
    {
        nInnerStartX += (long) ( ( lcl_LineTotal(pBorderItem->GetLeft()) +
                                    pBorderItem->GetDistance() ) * nScaleX );
        nInnerStartY += (long) ( ( lcl_LineTotal(pBorderItem->GetTop()) +
                                    pBorderItem->GetDistance() ) * nScaleY );
    }
    if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
    {
        nInnerStartX += (long) ( pShadowItem->CalcShadowSpace(SHADOW_LEFT) * nScaleX );
        nInnerStartY += (long) ( pShadowItem->CalcShadowSpace(SHADOW_TOP) * nScaleY );
    }
    long nRepStartX = nInnerStartX;
    long nRepStartY = nInnerStartY;
    if (aTableParam.bHeaders)
    {
        nRepStartX += (long) (PRINT_HEADER_WIDTH * nScaleX);
        nRepStartY += (long) (PRINT_HEADER_HEIGHT * nScaleY);
    }
    long nDataX = nRepStartX;
    long nDataY = nRepStartY;
    if (bDoRepCol)
        for (i=nRepeatStartCol; i<=nRepeatEndCol; i++)
            nDataX += (long) (pDoc->GetColWidth(i,nPrintTab) * nScaleX);
    if (bDoRepRow)
        for (i=nRepeatStartRow; i<=nRepeatEndRow; i++)
            nDataY += (long) (pDoc->FastGetRowHeight(i,nPrintTab) * nScaleY);
    long nEndX = nDataX;
    long nEndY = nDataY;
    for (i=nX1; i<=nX2; i++)
        nEndX += (long) (pDoc->GetColWidth(i,nPrintTab) * nScaleX);
    for (i=nY1; i<=nY2; i++)
        nEndY += (long) (pDoc->FastGetRowHeight(i,nPrintTab) * nScaleY);

    //  Seiten-Rahmen / Hintergrund

    //! nEndX/Y anpassen

    long nBorderEndX = nEndX;
    long nBorderEndY = nEndY;
    if (pBorderItem)
    {
        nBorderEndX += (long) ( ( lcl_LineTotal(pBorderItem->GetRight()) +
                                    pBorderItem->GetDistance() ) * nScaleX );
        nBorderEndY += (long) ( ( lcl_LineTotal(pBorderItem->GetBottom()) +
                                    pBorderItem->GetDistance() ) * nScaleY );
    }
    if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
    {
        nBorderEndX += (long) ( pShadowItem->CalcShadowSpace(SHADOW_RIGHT) * nScaleX );
        nBorderEndY += (long) ( pShadowItem->CalcShadowSpace(SHADOW_BOTTOM) * nScaleY );
    }
    pDev->SetMapMode( aOffsetMode );
    DrawBorder( nStartX, nStartY, nBorderEndX-nStartX, nBorderEndY-nStartY,
                    pBorderItem, pBackgroundItem, pShadowItem );

    pDev->SetMapMode( aTwipMode );
    SFX_APP()->SpoilDemoOutput( *pDev, aPageRect );
    pDev->SetMapMode( aOffsetMode );

    //  Wiederholungszeilen/Spalten ausgeben

    if (bDoRepCol && bDoRepRow)
        PrintArea( nRepeatStartCol,nRepeatStartRow, nRepeatEndCol,nRepeatEndRow,
                        nRepStartX,nRepStartY, TRUE,TRUE,FALSE,FALSE );
    if (bDoRepCol)
        PrintArea( nRepeatStartCol,nY1, nRepeatEndCol,nY2, nRepStartX,nDataY,
                    TRUE,!bDoRepRow,FALSE,TRUE );
    if (bDoRepRow)
        PrintArea( nX1,nRepeatStartRow, nX2,nRepeatEndRow, nDataX,nRepStartY,
                    !bDoRepCol,TRUE,TRUE,FALSE );

    //  Daten ausgeben

    PrintArea( nX1,nY1, nX2,nY2, nDataX,nDataY, !bDoRepCol,!bDoRepRow,TRUE,TRUE );

    //  Spalten-/Zeilenkoepfe ausgeben
    //  nach den Daten (ueber evtl. weitergezeichneten Schatten)

    if (aTableParam.bHeaders)
    {
        pDev->SetLineColor( COL_BLACK );
        pDev->SetFillColor();

        ScPatternAttr aPattern( pDoc->GetPool() );
        Font aFont;
//      aPattern.GetFont( aFont, pDev, (USHORT)((nZoom * (long) nManualZoom) / 100) );
        aPattern.GetFont( aFont, pDev );
        pDev->SetFont( aFont );

        if (bDoRepCol)
            PrintColHdr( nRepeatStartCol,nRepeatEndCol, nRepStartX,nInnerStartY );
        PrintColHdr( nX1,nX2, nDataX,nInnerStartY );
        if (bDoRepRow)
            PrintRowHdr( nRepeatStartRow,nRepeatEndRow, nInnerStartX,nRepStartY );
        PrintRowHdr( nY1,nY2, nInnerStartX,nDataY );
    }

    //  einfacher Rahmen

    if (aTableParam.bGrid || aTableParam.bHeaders)
    {
        Size aOnePixel = pDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        long nOneY = aOnePixel.Height();

        long nLeftX = nInnerStartX-nOneX;
        long nTopY  = nInnerStartY-nOneY;
        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor( COL_BLACK );
        pDev->SetFillColor();
        pDev->DrawRect( Rectangle( nLeftX, nTopY, nEndX-nOneX, nEndY-nOneY ) );
        //  nEndX/Y ohne Rahmen-Anpassung
    }

    if (pPrinter)
        pPrinter->EndPage();
}

void ScPrintFunc::SetOffset( const Point& rOfs )
{
    aSrcOffset = rOfs;
}

void ScPrintFunc::SetManualZoom( USHORT nNewZoom )
{
    nManualZoom = nNewZoom;
}

void ScPrintFunc::SetClearFlag( BOOL bFlag )
{
    bClearWin = bFlag;
}

//
//  UpdatePages wird nur von aussen gerufen, um die Umbrueche fuer die Anzeige
//  richtig zu setzen - immer ohne UserArea
//

BOOL ScPrintFunc::UpdatePages()
{
    if (!pParamSet)
        return FALSE;

    //  Zoom

    nZoom = 100;
    if (aTableParam.bScalePageNum)
        nZoom = ZOOM_MIN;                       // stimmt fuer Umbrueche
    else if (aTableParam.bScaleAll)
    {
        nZoom = aTableParam.nScaleAll;
        if ( nZoom <= ZOOM_MIN )
            nZoom = ZOOM_MIN;
    }

    String aName = pDoc->GetPageStyle( nPrintTab );
    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
        if ( nTab==nPrintTab || pDoc->GetPageStyle(nTab)==aName )
        {
            //  Wiederholungszeilen / Spalten
            pDoc->SetRepeatArea( nTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );

            //  Umbrueche setzen
            ResetBreaks(nTab);
            pDocShell->PostPaint(0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID);
        }

    return TRUE;
}

long ScPrintFunc::CountPages()                          // setzt auch nPagesX, nPagesY
{
    BOOL bAreaOk = FALSE;

    if (pDoc->HasTable( nPrintTab ))
    {
        if (aAreaParam.bPrintArea)                          // Druckbereich angegeben?
        {
            if ( bPrintCurrentTable )
            {
                ScRange& rRange = aAreaParam.aPrintArea;

                //  hier kein Vergleich der Tabellen mehr, die Area gilt immer fuer diese Tabelle
                //  wenn hier verglichen werden soll, muss die Tabelle der Druckbereiche beim
                //  Einfuegen von Tabellen etc. angepasst werden !

                nStartCol = rRange.aStart.Col();
                nStartRow = rRange.aStart.Row();
                nEndCol   = rRange.aEnd  .Col();
                nEndRow   = rRange.aEnd  .Row();
                bAreaOk   = AdjustPrintArea(FALSE);         // begrenzen
            }
            else
                bAreaOk = FALSE;
        }
        else                                                // aus Dokument suchen
            bAreaOk = AdjustPrintArea(TRUE);
    }

    if (bAreaOk)
    {
        long nPages = 0;
        USHORT nY;
        if (bMultiArea)
        {
            USHORT nRCount = pDoc->GetPrintRangeCount( nPrintTab );
            for (USHORT i=0; i<nRCount; i++)
            {
                CalcZoom(i);
                if ( aTableParam.bSkipEmpty )
                    for (nY=0; nY<nPagesY; nY++)
                        nPages += pPageRows[nY].CountVisible();
                else
                    nPages += ((long) nPagesX) * nPagesY;
                if ( pPageData )
                    FillPageData();
            }
        }
        else
        {
            CalcZoom(RANGENO_NORANGE);                      // Zoom berechnen
            if ( aTableParam.bSkipEmpty )
                for (nY=0; nY<nPagesY; nY++)
                    nPages += pPageRows[nY].CountVisible();
            else
                nPages += ((long) nPagesX) * nPagesY;
            if ( pPageData )
                FillPageData();
        }
        return nPages;
    }
    else
    {
//      nZoom = 100;                        // nZoom auf letztem Wert stehenlassen !!!
        nPagesX = nPagesY = nTotalY = 0;
        return 0;
    }
}

long ScPrintFunc::CountNotePages()
{
    if ( !aTableParam.bNotes || !bPrintCurrentTable )
        return 0;

    long nCount=0;
    USHORT nCol;
    USHORT nRow;

    BOOL bError = FALSE;
    if (!aAreaParam.bPrintArea)
        bError = !AdjustPrintArea(TRUE);            // komplett aus Dok suchen

    USHORT nRepeats = 1;                            // wie oft durchgehen ?
    if (bMultiArea)
        nRepeats = pDoc->GetPrintRangeCount(nPrintTab);
    if (bError)
        nRepeats = 0;

    for (USHORT nStep=0; nStep<nRepeats; nStep++)
    {
        BOOL bDoThis = TRUE;
        if (bMultiArea)             // alle Areas durchgehen
        {
            const ScRange* pThisRange = pDoc->GetPrintRange( nPrintTab, nStep );
            if ( pThisRange )
            {
                nStartCol = pThisRange->aStart.Col();
                nStartRow = pThisRange->aStart.Row();
                nEndCol   = pThisRange->aEnd  .Col();
                nEndRow   = pThisRange->aEnd  .Row();
                bDoThis = AdjustPrintArea(FALSE);
            }
        }

        if (bDoThis)
        {
            ScHorizontalCellIterator aIter( pDoc, nPrintTab, nStartCol,nStartRow, nEndCol,nEndRow );
            ScBaseCell* pCell = aIter.GetNext( nCol, nRow );
            while (pCell)
            {
                if (pCell->GetNotePtr())
                {
                    aNotePosList.Insert( new ScTripel( nCol,nRow,nPrintTab ), LIST_APPEND );
                    ++nCount;
                }

                pCell = aIter.GetNext( nCol, nRow );
            }
        }
    }

    long nPages = 0;
    long nNoteNr = 0;
    long nNoteAdd;
    do
    {
        nNoteAdd = PrintNotes( nPages, nNoteNr, FALSE );
        if (nNoteAdd)
        {
            nNoteNr += nNoteAdd;
            ++nPages;
        }
    }
    while (nNoteAdd);

    return nPages;
}

void ScPrintFunc::InitModes()               // aus nZoom etc. die MapModes setzen
{
    aOffset = Point( aSrcOffset.X()*100/nZoom, aSrcOffset.Y()*100/nZoom );

    long nEffZoom = nZoom * (long) nManualZoom;

//  nScaleX = nScaleY = 1.0;            // Ausgabe in Twips
    nScaleX = nScaleY = HMM_PER_TWIPS;  // Ausgabe in 1/100 mm

    Fraction aZoomFract( nEffZoom,10000 );
    Fraction aHorFract = aZoomFract;

    if (!pPrinter)                                          // Massstab anpassen fuer Preview
    {
        double nFact = pDocShell->GetOutputFactor();
        aHorFract = Fraction( (long)( nEffZoom / nFact ), 10000 );
    }

    aLogicMode = MapMode( MAP_100TH_MM, Point(), aHorFract, aZoomFract );

    Point aLogicOfs( -aOffset.X(), -aOffset.Y() );
    aOffsetMode = MapMode( MAP_100TH_MM, aLogicOfs, aHorFract, aZoomFract );

    Point aTwipsOfs( (long) ( -aOffset.X() / nScaleX + 0.5 ), (long) ( -aOffset.Y() / nScaleY + 0.5 ) );
    aTwipMode = MapMode( MAP_TWIP, aTwipsOfs, aHorFract, aZoomFract );
}

//--------------------------------------------------------------------

void ScPrintFunc::ApplyPrintSettings()
{
    if ( pPrinter )
    {
        //
        //  Printer zum Drucken umstellen
        //

        Size aEnumSize = aPageSize;
        if ( bLandscape )
        {
            long nTemp = aEnumSize.Width();
            aEnumSize.Width() = aEnumSize.Height();
            aEnumSize.Height() = nTemp;
        }
        Paper ePaper = SvxPaperInfo::GetSvPaper( aEnumSize, MAP_TWIP, TRUE );
        USHORT nPaperBin = ((const SvxPaperBinItem&)pParamSet->Get(ATTR_PAGE_PAPERBIN)).GetValue();

        pPrinter->SetPaper( ePaper );
        if ( PAPER_USER == ePaper )
        {
            MapMode aPrinterMode = pPrinter->GetMapMode();
            MapMode aLocalMode( MAP_TWIP );
            pPrinter->SetMapMode( aLocalMode );
            pPrinter->SetPaperSizeUser( aEnumSize );
            pPrinter->SetMapMode( aPrinterMode );
        }
        pPrinter->SetPaperBin( nPaperBin );
        pPrinter->SetOrientation( bLandscape ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT );
    }
}

//--------------------------------------------------------------------
//  rPageRanges   = Range fuer alle Tabellen
//  nStartPage    = in rPageRanges beginnen bei nStartPage
//  nDisplayStart = lfd. Nummer fuer Anzeige der Seitennummer

long ScPrintFunc::DoPrint( const MultiSelection& rPageRanges,
                                long nStartPage, long nDisplayStart,
                                SfxProgress* pProgress )
{
    DBG_ASSERT(pDev,"Device == NULL");
    if (!pParamSet)
        return 0;

    if ( pPrinter )
        ApplyPrintSettings();

    //--------------------------------------------------------------------

    InitModes();

    MakeTableString();

    if ( pProgress )
        pProgress->SetText( String( ScResId( SCSTR_STAT_PRINT ) ) );

    //--------------------------------------------------------------------

    long nPageNo = 0;
    long nPrinted = 0;
    long nEndPage = rPageRanges.GetTotalRange().Max();

    USHORT nRepeats = 1;                    // wie oft durchgehen ?
    if (bMultiArea)
        nRepeats = pDoc->GetPrintRangeCount(nPrintTab);
    for (USHORT nStep=0; nStep<nRepeats; nStep++)
    {
        if (bMultiArea)                     // Bereich neu belegen ?
        {
            CalcZoom(nStep);                // setzt auch nStartCol etc. neu
            InitModes();
        }

        USHORT nX1;
        USHORT nY1;
        USHORT nX2;
        USHORT nY2;
        USHORT nCountX;
        USHORT nCountY;

        if (aTableParam.bTopDown)                           // von oben nach unten
        {
            nX1 = nStartCol;
            for (nCountX=0; nCountX<nPagesX; nCountX++)
            {
                nX2 = pPageEndX[nCountX];
                for (nCountY=0; nCountY<nPagesY; nCountY++)
                {
                    nY1 = pPageRows[nCountY].GetStartRow();
                    nY2 = pPageRows[nCountY].GetEndRow();
                    if ( !aTableParam.bSkipEmpty || !pPageRows[nCountY].IsHidden(nCountX) )
                    {
                        if ( rPageRanges.IsSelected( nPageNo+nStartPage+1 ) )
                        {
                            // PrintPage( nPageNo+nStartPage, nX1, nY1, nX2, nY2 );
                            PrintPage( nPageNo+nDisplayStart, nX1, nY1, nX2, nY2 );

                            if ( pProgress )
                            {
                                pProgress->SetState( nPageNo+nStartPage+1, nEndPage );
                                pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
                            }
                            ++nPrinted;
                        }
                        ++nPageNo;
                    }
                }
                nX1 = nX2 + 1;
            }
        }
        else                                                // von links nach rechts
        {
            for (nCountY=0; nCountY<nPagesY; nCountY++)
            {
                nY1 = pPageRows[nCountY].GetStartRow();
                nY2 = pPageRows[nCountY].GetEndRow();
                nX1 = nStartCol;
                for (nCountX=0; nCountX<nPagesX; nCountX++)
                {
                    nX2 = pPageEndX[nCountX];
                    if ( !aTableParam.bSkipEmpty || !pPageRows[nCountY].IsHidden(nCountX) )
                    {
                        if ( rPageRanges.IsSelected( nPageNo+nStartPage+1 ) )
                        {
                            // PrintPage( nPageNo+nStartPage, nX1, nY1, nX2, nY2 );
                            PrintPage( nPageNo+nDisplayStart, nX1, nY1, nX2, nY2 );

                            if ( pProgress )
                            {
                                pProgress->SetState( nPageNo+nStartPage+1, nEndPage );
                                pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
                            }
                            ++nPrinted;
                        }
                        ++nPageNo;
                    }
                    nX1 = nX2 + 1;
                }
            }
        }
    }

    aFieldData.aTabName = ScGlobal::GetRscString( STR_NOTES );

    long nNoteNr = 0;
    long nNoteAdd;
    do
    {
        if ( nPageNo+nStartPage <= nEndPage )
        {
            BOOL bPageSelected = rPageRanges.IsSelected( nPageNo+nStartPage+1 );
            nNoteAdd = PrintNotes( nPageNo+nStartPage, nNoteNr, bPageSelected );
            if ( nNoteAdd )
            {
                nNoteNr += nNoteAdd;
                if ( pProgress && bPageSelected )
                {
                    pProgress->SetState( nPageNo+nStartPage+1, nEndPage );
                    pProgress->Reschedule(); //Mag der Anwender noch oder hat er genug?
                }
                if (bPageSelected)
                    ++nPrinted;
                ++nPageNo;
            }
        }
        else
            nNoteAdd = 0;
    }
    while (nNoteAdd);

    if ( bMultiArea )
        ResetBreaks(nPrintTab);                         // Breaks fuer Anzeige richtig

    return nPrinted;
}

void ScPrintFunc::CalcZoom( USHORT nRangeNo )                       // Zoom berechnen
{
    USHORT nRCount = pDoc->GetPrintRangeCount( nPrintTab );
    const ScRange* pThisRange = NULL;
    if ( nRangeNo != RANGENO_NORANGE || nRangeNo < nRCount )
        pThisRange = pDoc->GetPrintRange( nPrintTab, nRangeNo );
    if ( pThisRange )
    {
        nStartCol = pThisRange->aStart.Col();
        nStartRow = pThisRange->aStart.Row();
        nEndCol   = pThisRange->aEnd  .Col();
        nEndRow   = pThisRange->aEnd  .Row();
    }

    if (!AdjustPrintArea(FALSE))                        // leer
    {
        nZoom = 100;
        nPagesX = nPagesY = nTotalY = 0;
        return;
    }

    pDoc->SetRepeatArea( nPrintTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );

    if (aTableParam.bScalePageNum)
    {
        nZoom = 100;
        BOOL bFound = FALSE;
        USHORT nPagesToFit = aTableParam.nScalePageNum;
        while (!bFound)
        {
            CalcPages();
            if ( nPagesX * nPagesY <= nPagesToFit || nZoom <= ZOOM_MIN )
                bFound = TRUE;
            else
                --nZoom;
        }
    }
    else if (aTableParam.bScaleAll)
    {
        nZoom = aTableParam.nScaleAll;
        if ( nZoom <= ZOOM_MIN )
            nZoom = ZOOM_MIN;
        CalcPages();
    }
    else
    {
        DBG_ASSERT( aTableParam.bScaleNone, "kein Scale-Flag gesetzt" );
        nZoom = 100;
        CalcPages();
    }
}

Size ScPrintFunc::GetDocPageSize()
{
                        // Hoehe Kopf-/Fusszeile anpassen

    InitModes();                            // aTwipMode aus nZoom initialisieren
    pDev->SetMapMode( aTwipMode );          // Kopf-/Fusszeilen in Twips
    UpdateHFHeight( aHdr );
    UpdateHFHeight( aFtr );

                        // Seitengroesse in Document-Twips
                        //  Berechnung Left / Right auch in PrintPage

    aPageRect = Rectangle( Point(), aPageSize );
    aPageRect.Left()   = ( aPageRect.Left()   + nLeftMargin                  ) * 100 / nZoom;
    aPageRect.Right()  = ( aPageRect.Right()  - nRightMargin                 ) * 100 / nZoom;
    aPageRect.Top()    = ( aPageRect.Top()    + nTopMargin    ) * 100 / nZoom + aHdr.nHeight;
    aPageRect.Bottom() = ( aPageRect.Bottom() - nBottomMargin ) * 100 / nZoom - aFtr.nHeight;

    Size aDocPageSize = aPageRect.GetSize();
    if (aTableParam.bHeaders)
    {
        aDocPageSize.Width()  -= (long) PRINT_HEADER_WIDTH;
        aDocPageSize.Height() -= (long) PRINT_HEADER_HEIGHT;
    }
    if (pBorderItem)
    {
        aDocPageSize.Width()  -= lcl_LineTotal(pBorderItem->GetLeft()) +
                                 lcl_LineTotal(pBorderItem->GetRight()) +
                                 2 * pBorderItem->GetDistance();
        aDocPageSize.Height() -= lcl_LineTotal(pBorderItem->GetTop()) +
                                 lcl_LineTotal(pBorderItem->GetBottom()) +
                                 2 * pBorderItem->GetDistance();
    }
    if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
    {
        aDocPageSize.Width()  -= pShadowItem->CalcShadowSpace(SHADOW_LEFT) +
                                 pShadowItem->CalcShadowSpace(SHADOW_RIGHT);
        aDocPageSize.Height() -= pShadowItem->CalcShadowSpace(SHADOW_TOP) +
                                 pShadowItem->CalcShadowSpace(SHADOW_BOTTOM);
    }
    return aDocPageSize;
}

void ScPrintFunc::ResetBreaks( USHORT nTab )            // Breaks fuer Anzeige richtig setzen
{
    pDoc->SetPageSize( nTab, GetDocPageSize() );
    pDoc->UpdatePageBreaks( nTab, NULL );
}

void lcl_SetHidden( ScDocument* pDoc, USHORT nPrintTab, ScPageRowEntry& rPageRowEntry,
                    USHORT nStartCol, const USHORT* pPageEndX )
{
    USHORT nPagesX   = rPageRowEntry.GetPagesX();
    USHORT nStartRow = rPageRowEntry.GetStartRow();
    USHORT nEndRow   = rPageRowEntry.GetEndRow();

    BOOL bLeftIsEmpty = FALSE;
    ScRange aTempRange;
    Rectangle aTempRect = pDoc->GetMMRect( 0,0, 0,0, 0 );

    for (USHORT i=0; i<nPagesX; i++)
    {
        USHORT nEndCol = pPageEndX[i];
        if ( pDoc->IsPrintEmpty( nPrintTab, nStartCol, nStartRow, nEndCol, nEndRow,
                                    bLeftIsEmpty, &aTempRange, &aTempRect ) )
        {
            rPageRowEntry.SetHidden(i);
            bLeftIsEmpty = TRUE;
        }
        else
            bLeftIsEmpty = FALSE;

        nStartCol = nEndCol+1;
    }
}

void ScPrintFunc::CalcPages()               // berechnet aPageRect und Seiten aus nZoom
{
    if (!pPageEndX) pPageEndX = new USHORT[MAXCOL+1];
    if (!pPageEndY) pPageEndY = new USHORT[MAXROW+1];
    if (!pPageRows) pPageRows = new ScPageRowEntry[MAXROW+1];   //! vorher zaehlen !!!!

    pDoc->SetPageSize( nPrintTab, GetDocPageSize() );
    if (aAreaParam.bPrintArea)
    {
        ScRange aRange( nStartCol, nStartRow, nPrintTab, nEndCol, nEndRow, nPrintTab );
        pDoc->UpdatePageBreaks( nPrintTab, &aRange );
    }
    else
        pDoc->UpdatePageBreaks( nPrintTab, NULL );      // sonst wird das Ende markiert

    //
    //  Seiteneinteilung nach Umbruechen in Col/RowFlags
    //  Von mehreren Umbruechen in einem ausgeblendeten Bereich zaehlt nur einer.
    //

    USHORT i;
    nPagesX = 0;
    nPagesY = 0;
    nTotalY = 0;

    BOOL bVisCol = FALSE;
    for (i=nStartCol; i<=nEndCol; i++)
    {
        BYTE nFlags = pDoc->GetColFlags(i,nPrintTab);
        if ( i>nStartCol && bVisCol && (nFlags & CR_PAGEBREAK) )
        {
            pPageEndX[nPagesX] = i-1;
            ++nPagesX;
            bVisCol = FALSE;
        }
        if (!(nFlags & CR_HIDDEN))
            bVisCol = TRUE;
    }
    if (bVisCol)    // auch am Ende keine leeren Seiten
    {
        pPageEndX[nPagesX] = nEndCol;
        ++nPagesX;
    }

    BOOL bVisRow = FALSE;
    USHORT nPageStartRow = nStartRow;
    for (i=nStartRow; i<=nEndRow; i++)
    {
        BYTE nFlags = pDoc->GetRowFlags(i,nPrintTab);
        if ( i>nStartRow && bVisRow && (nFlags & CR_PAGEBREAK) )
        {
            pPageEndY[nTotalY] = i-1;
            ++nTotalY;

            if ( !aTableParam.bSkipEmpty ||
                    !pDoc->IsPrintEmpty( nPrintTab, nStartCol, nPageStartRow, nEndCol, i-1 ) )
            {
                pPageRows[nPagesY].SetStartRow( nPageStartRow );
                pPageRows[nPagesY].SetEndRow( i-1 );
                pPageRows[nPagesY].SetPagesX( nPagesX );
                if (aTableParam.bSkipEmpty)
                    lcl_SetHidden( pDoc, nPrintTab, pPageRows[nPagesY], nStartCol, pPageEndX );
                ++nPagesY;
            }

            nPageStartRow = i;
            bVisRow = FALSE;
        }
        if (!(nFlags & CR_HIDDEN))
            bVisRow = TRUE;
    }
    if (bVisRow)
    {
        pPageEndY[nTotalY] = nEndRow;
        ++nTotalY;

        if ( !aTableParam.bSkipEmpty ||
                !pDoc->IsPrintEmpty( nPrintTab, nStartCol, nPageStartRow, nEndCol, nEndRow ) )
        {
            pPageRows[nPagesY].SetStartRow( nPageStartRow );
            pPageRows[nPagesY].SetEndRow( nEndRow );
            pPageRows[nPagesY].SetPagesX( nPagesX );
            if (aTableParam.bSkipEmpty)
                lcl_SetHidden( pDoc, nPrintTab, pPageRows[nPagesY], nStartCol, pPageEndX );
            ++nPagesY;
        }
    }
}

//------------------------------------------------------------------------
//  class ScJobSetup
//------------------------------------------------------------------------

ScJobSetup::ScJobSetup( SfxPrinter* pPrinter )
{
    eOrientation = pPrinter->GetOrientation();
    nPaperBin    = pPrinter->GetPaperBin();
    ePaper       = pPrinter->GetPaper();

    if ( PAPER_USER == ePaper )
    {
        aUserSize = pPrinter->GetPaperSize();
        aUserMapMode = pPrinter->GetMapMode();
    }
};





