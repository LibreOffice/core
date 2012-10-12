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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editview.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/printer.hxx>

#include <svx/svdview.hxx>
#include "tabvwsh.hxx"

#include "gridwin.hxx"
#include "viewdata.hxx"
#include "output.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "patattr.hxx"          // InvertSimple
#include "dbdata.hxx"
#include "docoptio.hxx"
#include "notemark.hxx"
#include "dbfunc.hxx"           // oder GetPageBreakData an die ViewData
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "rfindlst.hxx"
#include "hiranges.hxx"
#include "pagedata.hxx"
#include "docpool.hxx"
#include "globstr.hrc"
#include "docsh.hxx"            // oder GetSfxInPlaceObject
#include "cbutton.hxx"
#include "invmerge.hxx"
#include "editutil.hxx"
#include "inputopt.hxx"
#include "fillinfo.hxx"
#include "dpcontrol.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"
#include "sc.hrc"
#include <vcl/virdev.hxx>

// #i74769#
#include <svx/sdrpaintwindow.hxx>

//------------------------------------------------------------------------

static void lcl_LimitRect( Rectangle& rRect, const Rectangle& rVisible )
{
    if ( rRect.Top()    < rVisible.Top()-1 )    rRect.Top()    = rVisible.Top()-1;
    if ( rRect.Bottom() > rVisible.Bottom()+1 ) rRect.Bottom() = rVisible.Bottom()+1;

    // auch wenn das inner-Rectangle nicht sichtbar ist, muss evtl.
    // die Titelzeile gezeichnet werden, darum kein Rueckgabewert mehr.
    // Wenn's weit daneben liegt, wird lcl_DrawOneFrame erst gar nicht gerufen.
}

static void lcl_DrawOneFrame( OutputDevice* pDev, const Rectangle& rInnerPixel,
                        const String& rTitle, const Color& rColor, sal_Bool bTextBelow,
                        double nPPTX, double nPPTY, const Fraction& rZoomY,
                        ScDocument* pDoc, ScViewData* pButtonViewData, sal_Bool bLayoutRTL )
{
    //  pButtonViewData wird nur benutzt, um die Button-Groesse zu setzen,
    //  darf ansonsten NULL sein!

    Rectangle aInner = rInnerPixel;
    if ( bLayoutRTL )
    {
        aInner.Left() = rInnerPixel.Right();
        aInner.Right() = rInnerPixel.Left();
    }

    Rectangle aVisible( Point(0,0), pDev->GetOutputSizePixel() );
    lcl_LimitRect( aInner, aVisible );

    Rectangle aOuter = aInner;
    long nHor = (long) ( SC_SCENARIO_HSPACE * nPPTX );
    long nVer = (long) ( SC_SCENARIO_VSPACE * nPPTY );
    aOuter.Left()   -= nHor;
    aOuter.Right()  += nHor;
    aOuter.Top()    -= nVer;
    aOuter.Bottom() += nVer;

    //  use ScPatternAttr::GetFont only for font size
    Font aAttrFont;
    ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
                                    GetFont(aAttrFont,SC_AUTOCOL_BLACK,pDev,&rZoomY);

    //  everything else from application font
    Font aAppFont = pDev->GetSettings().GetStyleSettings().GetAppFont();
    aAppFont.SetSize( aAttrFont.GetSize() );

    aAppFont.SetAlign( ALIGN_TOP );
    pDev->SetFont( aAppFont );

    Size aTextSize( pDev->GetTextWidth( rTitle ), pDev->GetTextHeight() );

    if ( bTextBelow )
        aOuter.Bottom() += aTextSize.Height();
    else
        aOuter.Top()    -= aTextSize.Height();

    pDev->SetLineColor();
    pDev->SetFillColor( rColor );
    //  links, oben, rechts, unten
    pDev->DrawRect( Rectangle( aOuter.Left(),  aOuter.Top(),    aInner.Left(),  aOuter.Bottom() ) );
    pDev->DrawRect( Rectangle( aOuter.Left(),  aOuter.Top(),    aOuter.Right(), aInner.Top()    ) );
    pDev->DrawRect( Rectangle( aInner.Right(), aOuter.Top(),    aOuter.Right(), aOuter.Bottom() ) );
    pDev->DrawRect( Rectangle( aOuter.Left(),  aInner.Bottom(), aOuter.Right(), aOuter.Bottom() ) );

    long nButtonY = bTextBelow ? aInner.Bottom() : aOuter.Top();

    ScDDComboBoxButton aComboButton((Window*)pDev);
    aComboButton.SetOptSizePixel();
    long nBWidth  = ( aComboButton.GetSizePixel().Width() * rZoomY.GetNumerator() )
                        / rZoomY.GetDenominator();
    long nBHeight = nVer + aTextSize.Height() + 1;
    Size aButSize( nBWidth, nBHeight );
    long nButtonPos = bLayoutRTL ? aOuter.Left() : aOuter.Right()-nBWidth+1;
    aComboButton.Draw( Point(nButtonPos, nButtonY), aButSize, false );
    if (pButtonViewData)
        pButtonViewData->SetScenButSize( aButSize );

    long nTextStart = bLayoutRTL ? aInner.Right() - aTextSize.Width() + 1 : aInner.Left();

    sal_Bool bWasClip = false;
    Region aOldClip;
    sal_Bool bClip = ( aTextSize.Width() > aOuter.Right() - nBWidth - aInner.Left() );
    if ( bClip )
    {
        if (pDev->IsClipRegion())
        {
            bWasClip = sal_True;
            aOldClip = pDev->GetActiveClipRegion();
        }
        long nClipStartX = bLayoutRTL ? aOuter.Left() + nBWidth : aInner.Left();
        long nClipEndX = bLayoutRTL ? aInner.Right() : aOuter.Right() - nBWidth;
        pDev->SetClipRegion( Rectangle( nClipStartX, nButtonY + nVer/2,
                                nClipEndX, nButtonY + nVer/2 + aTextSize.Height() ) );
    }

    pDev->DrawText( Point( nTextStart, nButtonY + nVer/2 ), rTitle );

    if ( bClip )
    {
        if ( bWasClip )
            pDev->SetClipRegion(aOldClip);
        else
            pDev->SetClipRegion();
    }

    pDev->SetFillColor();
    pDev->SetLineColor( COL_BLACK );
    pDev->DrawRect( aInner );
    pDev->DrawRect( aOuter );
}

static void lcl_DrawScenarioFrames( OutputDevice* pDev, ScViewData* pViewData, ScSplitPos eWhich,
                            SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    SCTAB nTabCount = pDoc->GetTableCount();
    if ( nTab+1<nTabCount && pDoc->IsScenario(nTab+1) && !pDoc->IsScenario(nTab) )
    {
        if ( nX1 > 0 ) --nX1;
        if ( nY1>=2 ) nY1 -= 2;             // Hack: Titelzeile beruehrt zwei Zellen
        else if ( nY1 > 0 ) --nY1;
        if ( nX2 < MAXCOL ) ++nX2;
        if ( nY2 < MAXROW-1 ) nY2 += 2;     // Hack: Titelzeile beruehrt zwei Zellen
        else if ( nY2 < MAXROW ) ++nY2;
        ScRange aViewRange( nX1,nY1,nTab, nX2,nY2,nTab );

        //! Ranges an der Table cachen!!!!

        ScMarkData aMarks;
        for (SCTAB i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
            pDoc->MarkScenario( i, nTab, aMarks, false, SC_SCENARIO_SHOWFRAME );
        ScRangeListRef xRanges = new ScRangeList;
        aMarks.FillRangeListWithMarks( xRanges, false );

        sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
        long nLayoutSign = bLayoutRTL ? -1 : 1;

        for (size_t j = 0, n = xRanges->size(); j < n; ++j)
        {
            ScRange aRange = *(*xRanges)[j];
            //  Szenario-Rahmen immer dann auf zusammengefasste Zellen erweitern, wenn
            //  dadurch keine neuen nicht-ueberdeckten Zellen mit umrandet werden
            pDoc->ExtendTotalMerge( aRange );

            //! -> Repaint beim Zusammenfassen erweitern !!!

            if ( aRange.Intersects( aViewRange ) )          //! Platz fuer Text/Button?
            {
                Point aStartPos = pViewData->GetScrPos(
                                    aRange.aStart.Col(), aRange.aStart.Row(), eWhich, sal_True );
                Point aEndPos = pViewData->GetScrPos(
                                    aRange.aEnd.Col()+1, aRange.aEnd.Row()+1, eWhich, sal_True );
                //  on the grid:
                aStartPos.X() -= nLayoutSign;
                aStartPos.Y() -= 1;
                aEndPos.X() -= nLayoutSign;
                aEndPos.Y() -= 1;

                sal_Bool bTextBelow = ( aRange.aStart.Row() == 0 );

                rtl::OUString aCurrent;
                Color aColor( COL_LIGHTGRAY );
                for (SCTAB nAct=nTab+1; nAct<nTabCount && pDoc->IsScenario(nAct); nAct++)
                    if ( pDoc->IsActiveScenario(nAct) && pDoc->HasScenarioRange(nAct,aRange) )
                    {
                        rtl::OUString aDummyComment;
                        sal_uInt16 nDummyFlags;
                        pDoc->GetName( nAct, aCurrent );
                        pDoc->GetScenarioData( nAct, aDummyComment, aColor, nDummyFlags );
                    }

                if (aCurrent.isEmpty())
                    aCurrent = ScGlobal::GetRscString( STR_EMPTYDATA );

                //! eigener Text "(keins)" statt "(leer)" ???

                lcl_DrawOneFrame( pDev, Rectangle( aStartPos, aEndPos ),
                                    aCurrent, aColor, bTextBelow,
                                    pViewData->GetPPTX(), pViewData->GetPPTY(), pViewData->GetZoomY(),
                                    pDoc, pViewData, bLayoutRTL );
            }
        }
    }
}

//------------------------------------------------------------------------

static void lcl_DrawHighlight( ScOutputData& rOutputData, ScViewData* pViewData,
                        const std::vector<ScHighlightEntry>& rHighlightRanges )
{
    SCTAB nTab = pViewData->GetTabNo();
    std::vector<ScHighlightEntry>::const_iterator pIter;
    for ( pIter = rHighlightRanges.begin(); pIter != rHighlightRanges.end(); ++pIter)
    {
        ScRange aRange = pIter->aRef;
        if ( nTab >= aRange.aStart.Tab() && nTab <= aRange.aEnd.Tab() )
        {
            rOutputData.DrawRefMark(
                                aRange.aStart.Col(), aRange.aStart.Row(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(),
                                pIter->aColor, false );
        }
    }
}

//------------------------------------------------------------------------

void ScGridWindow::DoInvertRect( const Rectangle& rPixel )
{
    if ( rPixel == aInvertRect )
        aInvertRect = Rectangle();      // aufheben
    else
    {
        OSL_ENSURE( aInvertRect.IsEmpty(), "DoInvertRect nicht paarig" );

        aInvertRect = rPixel;           // neues Rechteck merken
    }

    UpdateHeaderOverlay();      // uses aInvertRect
}

//------------------------------------------------------------------------

void ScGridWindow::PrePaint()
{
    // forward PrePaint to DrawingLayer
    ScTabViewShell* pTabViewShell = pViewData->GetViewShell();

    if(pTabViewShell)
    {
        SdrView* pDrawView = pTabViewShell->GetSdrView();

        if(pDrawView)
        {
            pDrawView->PrePaint();
        }
    }
}

//------------------------------------------------------------------------

void ScGridWindow::Paint( const Rectangle& rRect )
{
    ScDocument* pDoc = pViewData->GetDocument();
    if ( pDoc->IsInInterpreter() )
    {
        //  via Reschedule, interpretierende Zellen nicht nochmal anstossen
        //  hier kein Invalidate, sonst kommt z.B. eine Error-Box nie an die Reihe
        //  (Bug 36381). Durch bNeedsRepaint wird spaeter alles nochmal gemalt.

        if ( bNeedsRepaint )
        {
            //! Rechtecke zusammenfassen?
            aRepaintPixel = Rectangle();            // mehrfach -> alles painten
        }
        else
        {
            bNeedsRepaint = true;
            aRepaintPixel = LogicToPixel(rRect);    // nur betroffenen Bereich
        }
        return;
    }

    // #i117893# If GetSizePixel needs to call the resize handler, the resulting nested Paint call
    // (possibly for a larger rectangle) has to be allowed. Call GetSizePixel before setting bIsInPaint.
    GetSizePixel();

    if (bIsInPaint)
        return;

    bIsInPaint = true;

    Rectangle aPixRect = LogicToPixel( rRect );

    SCCOL nX1 = pViewData->GetPosX(eHWhich);
    SCROW nY1 = pViewData->GetPosY(eVWhich);

    SCTAB nTab = pViewData->GetTabNo();

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    Rectangle aMirroredPixel = aPixRect;
    if ( pDoc->IsLayoutRTL( nTab ) )
    {
        //  mirror and swap
        long nWidth = GetSizePixel().Width();
        aMirroredPixel.Left()  = nWidth - 1 - aPixRect.Right();
        aMirroredPixel.Right() = nWidth - 1 - aPixRect.Left();
    }

    long nScrX = ScViewData::ToPixel( pDoc->GetColWidth( nX1, nTab ), nPPTX );
    while ( nScrX <= aMirroredPixel.Left() && nX1 < MAXCOL )
    {
        ++nX1;
        nScrX += ScViewData::ToPixel( pDoc->GetColWidth( nX1, nTab ), nPPTX );
    }
    SCCOL nX2 = nX1;
    while ( nScrX <= aMirroredPixel.Right() && nX2 < MAXCOL )
    {
        ++nX2;
        nScrX += ScViewData::ToPixel( pDoc->GetColWidth( nX2, nTab ), nPPTX );
    }

    long nScrY = 0;
    ScViewData::AddPixelsWhile( nScrY, aPixRect.Top(), nY1, MAXROW, nPPTY, pDoc, nTab);
    SCROW nY2 = nY1;
    if (nScrY <= aPixRect.Bottom() && nY2 < MAXROW)
    {
        ++nY2;
        ScViewData::AddPixelsWhile( nScrY, aPixRect.Bottom(), nY2, MAXROW, nPPTY, pDoc, nTab);
    }

    Draw( nX1,nY1,nX2,nY2, SC_UPDATE_MARKS );           // nicht weiterzeichnen

    bIsInPaint = false;
}

//
//  Draw  ----------------------------------------------------------------
//

void ScGridWindow::Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, ScUpdateMode eMode )
{
    ScModule* pScMod = SC_MOD();
    sal_Bool bTextWysiwyg = pScMod->GetInputOptions().GetTextWysiwyg();

    if (pViewData->IsMinimized())
        return;

    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    OSL_ENSURE( ValidCol(nX2) && ValidRow(nY2), "GridWin Draw Bereich zu gross" );

    UpdateVisibleRange();

    if (nX2 < maVisibleRange.mnCol1 || nY2 < maVisibleRange.mnRow1)
        return;
                    // unsichtbar
    if (nX1 < maVisibleRange.mnCol1)
        nX1 = maVisibleRange.mnCol1;
    if (nY1 < maVisibleRange.mnRow1)
        nY1 = maVisibleRange.mnRow1;

    if (nX1 > maVisibleRange.mnCol2 || nY1 > maVisibleRange.mnRow2)
        return;

    if (nX2 > maVisibleRange.mnCol2)
        nX2 = maVisibleRange.mnCol2;
    if (nY2 > maVisibleRange.mnRow2)
        nY2 = maVisibleRange.mnRow2;

    if ( eMode != SC_UPDATE_MARKS && nX2 < maVisibleRange.mnCol2)
        nX2 = maVisibleRange.mnCol2;                                // zum Weiterzeichnen

        //  ab hier kein return mehr

    ++nPaintCount;                  // merken, dass gemalt wird (wichtig beim Invertieren)

    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();

    pDoc->ExtendHidden( nX1, nY1, nX2, nY2, nTab );

    Point aScrPos = pViewData->GetScrPos( nX1, nY1, eWhich );
    long nMirrorWidth = GetSizePixel().Width();
    sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    long nLayoutSign = bLayoutRTL ? -1 : 1;
    if ( bLayoutRTL )
    {
        long nEndPixel = pViewData->GetScrPos( nX2+1, maVisibleRange.mnRow1, eWhich ).X();
        nMirrorWidth = aScrPos.X() - nEndPixel;
        aScrPos.X() = nEndPixel + 1;
    }

    long nScrX = aScrPos.X();
    long nScrY = aScrPos.Y();

    SCCOL nCurX = pViewData->GetCurX();
    SCROW nCurY = pViewData->GetCurY();
    SCCOL nCurEndX = nCurX;
    SCROW nCurEndY = nCurY;
    pDoc->ExtendMerge( nCurX, nCurY, nCurEndX, nCurEndY, nTab );
    sal_Bool bCurVis = nCursorHideCount==0 &&
                    ( nCurEndX+1 >= nX1 && nCurX <= nX2+1 && nCurEndY+1 >= nY1 && nCurY <= nY2+1 );

    //  AutoFill-Anfasser
    if ( !bCurVis && nCursorHideCount==0 && bAutoMarkVisible && aAutoMarkPos.Tab() == nTab &&
            ( aAutoMarkPos.Col() != nCurX || aAutoMarkPos.Row() != nCurY ) )
    {
        SCCOL nHdlX = aAutoMarkPos.Col();
        SCROW nHdlY = aAutoMarkPos.Row();
        pDoc->ExtendMerge( nHdlX, nHdlY, nHdlX, nHdlY, nTab );
        bCurVis = ( nHdlX+1 >= nX1 && nHdlX <= nX2 && nHdlY+1 >= nY1 && nHdlY <= nY2 );
        //  links und oben ist nicht betroffen

        //! AutoFill-Anfasser alleine (ohne Cursor) zeichnen ???
    }

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    const ScViewOptions& rOpts = pViewData->GetOptions();
    sal_Bool bFormulaMode = rOpts.GetOption( VOPT_FORMULAS );
    sal_Bool bMarkClipped = rOpts.GetOption( VOPT_CLIPMARKS );

        // Datenblock

    ScTableInfo aTabInfo;
    pDoc->FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nTab,
                                        nPPTX, nPPTY, false, bFormulaMode,
                                        &pViewData->GetMarkData() );

    //--------------------------------------------------------------------

    Fraction aZoomX = pViewData->GetZoomX();
    Fraction aZoomY = pViewData->GetZoomY();
    ScOutputData aOutputData( this, OUTTYPE_WINDOW, aTabInfo, pDoc, nTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nPPTX, nPPTY,
                                &aZoomX, &aZoomY );

    aOutputData.SetMirrorWidth( nMirrorWidth );         // needed for RTL

    std::auto_ptr< VirtualDevice > xFmtVirtDev;
    sal_Bool bLogicText = bTextWysiwyg;                     // call DrawStrings in logic MapMode?

    if ( bTextWysiwyg )
    {
        //  use printer for text formatting

        OutputDevice* pFmtDev = pDoc->GetPrinter();
        pFmtDev->SetMapMode( pViewData->GetLogicMode(eWhich) );
        aOutputData.SetFmtDevice( pFmtDev );
    }
    else if ( aZoomX != aZoomY && pViewData->IsOle() )
    {
        //  #i45033# For OLE inplace editing with different zoom factors,
        //  use a virtual device with 1/100th mm as text formatting reference

        xFmtVirtDev.reset( new VirtualDevice );
        xFmtVirtDev->SetMapMode( MAP_100TH_MM );
        aOutputData.SetFmtDevice( xFmtVirtDev.get() );

        bLogicText = sal_True;                      // use logic MapMode
    }

    const svtools::ColorConfig& rColorCfg = pScMod->GetColorConfig();
    Color aGridColor( rColorCfg.GetColorValue( svtools::CALCGRID, false ).nColor );
    if ( aGridColor.GetColor() == COL_TRANSPARENT )
    {
        //  use view options' grid color only if color config has "automatic" color
        aGridColor = rOpts.GetGridColor();
    }

    aOutputData.SetSyntaxMode       ( pViewData->IsSyntaxMode() );
    aOutputData.SetGridColor        ( aGridColor );
    aOutputData.SetShowNullValues   ( rOpts.GetOption( VOPT_NULLVALS ) );
    aOutputData.SetShowFormulas     ( bFormulaMode );
    aOutputData.SetShowSpellErrors  ( pDoc->GetDocOptions().IsAutoSpell() );
    aOutputData.SetMarkClipped      ( bMarkClipped );

    aOutputData.SetUseStyleColor( true );       // always set in table view

    aOutputData.SetEditObject( GetEditObject() );
    aOutputData.SetViewShell( pViewData->GetViewShell() );

    sal_Bool bGrid = rOpts.GetOption( VOPT_GRID ) && pViewData->GetShowGrid();
    sal_Bool bGridFirst = !rOpts.GetOption( VOPT_GRID_ONTOP );

    sal_Bool bPage = rOpts.GetOption( VOPT_PAGEBREAKS );

    if ( eMode == SC_UPDATE_CHANGED )
    {
        aOutputData.FindChanged();
        aOutputData.SetSingleGrid(sal_True);
    }

    sal_Bool bPageMode = pViewData->IsPagebreakMode();
    if (bPageMode)                                      // nach FindChanged
    {
        // SetPagebreakMode initialisiert auch bPrinted Flags
        aOutputData.SetPagebreakMode( pViewData->GetView()->GetPageBreakData() );
    }

    EditView*   pEditView = NULL;
    sal_Bool        bEditMode = pViewData->HasEditView(eWhich);
    if ( bEditMode && pViewData->GetRefTabNo() == nTab )
    {
        SCCOL nEditCol;
        SCROW nEditRow;
        pViewData->GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        SCCOL nEditEndCol = pViewData->GetEditEndCol();
        SCROW nEditEndRow = pViewData->GetEditEndRow();

        if ( nEditEndCol >= nX1 && nEditCol <= nX2 && nEditEndRow >= nY1 && nEditRow <= nY2 )
            aOutputData.SetEditCell( nEditCol, nEditRow );
        else
            bEditMode = false;
    }

    // define drawing layer map mode and paint rectangle
    const MapMode aDrawMode = GetDrawMapMode();
    Rectangle aDrawingRectLogic;

    {
        // get drawing pixel rect
        Rectangle aDrawingRectPixel(Point(nScrX, nScrY), Size(aOutputData.GetScrW(), aOutputData.GetScrH()));

        // correct for border (left/right)
        if(MAXCOL == nX2)
        {
            if(bLayoutRTL)
            {
                aDrawingRectPixel.Left() = 0L;
            }
            else
            {
                aDrawingRectPixel.Right() = GetOutputSizePixel().getWidth();
            }
        }

        // correct for border (bottom)
        if(MAXROW == nY2)
        {
            aDrawingRectPixel.Bottom() = GetOutputSizePixel().getHeight();
        }

        // get logic positions
        aDrawingRectLogic = PixelToLogic(aDrawingRectPixel, aDrawMode);
    }

    OutputDevice* pContentDev = this;       // device for document content, used by overlay manager
    SdrPaintWindow* pTargetPaintWindow = 0; // #i74769# work with SdrPaintWindow directly

    {
        // init redraw
        ScTabViewShell* pTabViewShell = pViewData->GetViewShell();

        if(pTabViewShell)
        {
            MapMode aCurrentMapMode(pContentDev->GetMapMode());
            pContentDev->SetMapMode(aDrawMode);
            SdrView* pDrawView = pTabViewShell->GetSdrView();

            if(pDrawView)
            {
                // #i74769# Use new BeginDrawLayers() interface
                Region aDrawingRegion(aDrawingRectLogic);
                pTargetPaintWindow = pDrawView->BeginDrawLayers(this, aDrawingRegion);
                OSL_ENSURE(pTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

                // #i74769# get target device from SdrPaintWindow, this may be the prerender
                // device now, too.
                pContentDev = &(pTargetPaintWindow->GetTargetOutputDevice());
                aOutputData.SetContentDevice( pContentDev );
            }

            pContentDev->SetMapMode(aCurrentMapMode);
        }
    }

    //  Rand (Wiese) (Pixel)
    if ( nX2==MAXCOL || nY2==MAXROW )
    {
        // save MapMode and set to pixel
        MapMode aCurrentMapMode(pContentDev->GetMapMode());
        pContentDev->SetMapMode(MAP_PIXEL);

        Rectangle aPixRect = Rectangle( Point(), GetOutputSizePixel() );
        pContentDev->SetFillColor( rColorCfg.GetColorValue(svtools::APPBACKGROUND).nColor );
        pContentDev->SetLineColor();
        if ( nX2==MAXCOL )
        {
            Rectangle aDrawRect( aPixRect );
            if ( bLayoutRTL )
                aDrawRect.Right() = nScrX - 1;
            else
                aDrawRect.Left() = nScrX + aOutputData.GetScrW();
            if (aDrawRect.Right() >= aDrawRect.Left())
                pContentDev->DrawRect( aDrawRect );
        }
        if ( nY2==MAXROW )
        {
            Rectangle aDrawRect( aPixRect );
            aDrawRect.Top() = nScrY + aOutputData.GetScrH();
            if ( nX2==MAXCOL )
            {
                // no double painting of the corner
                if ( bLayoutRTL )
                    aDrawRect.Left() = nScrX;
                else
                    aDrawRect.Right() = nScrX + aOutputData.GetScrW() - 1;
            }
            if (aDrawRect.Bottom() >= aDrawRect.Top())
                pContentDev->DrawRect( aDrawRect );
        }

        // restore MapMode
        pContentDev->SetMapMode(aCurrentMapMode);
    }

    if ( pDoc->HasBackgroundDraw( nTab, aDrawingRectLogic ) )
    {
        pContentDev->SetMapMode(MAP_PIXEL);
        aOutputData.DrawClear();

            // Drawing Hintergrund

        pContentDev->SetMapMode(aDrawMode);
        DrawRedraw( aOutputData, eMode, SC_LAYER_BACK );
    }
    else
        aOutputData.SetSolidBackground(sal_True);

    pContentDev->SetMapMode(MAP_PIXEL);
    aOutputData.DrawDocumentBackground();

    if ( bGridFirst && ( bGrid || bPage ) )
        aOutputData.DrawGrid( bGrid, bPage );

    aOutputData.DrawBackground();

    if ( !bGridFirst && ( bGrid || bPage ) )
        aOutputData.DrawGrid( bGrid, bPage );

    if ( bPageMode )
    {
        // DrawPagePreview draws complete lines/page numbers, must always be clipped
        if ( aOutputData.SetChangedClip() )
        {
            DrawPagePreview(nX1,nY1,nX2,nY2, pContentDev);
            pContentDev->SetClipRegion();
        }
    }

    aOutputData.DrawShadow();
    aOutputData.DrawFrame();
    if ( !bLogicText )
        aOutputData.DrawStrings(false);     // in pixel MapMode

    // edit cells and printer-metrics text must be before the buttons
    // (DataPilot buttons contain labels in UI font)

    pContentDev->SetMapMode(pViewData->GetLogicMode(eWhich));
    if ( bLogicText )
        aOutputData.DrawStrings(sal_True);      // in logic MapMode if bTextWysiwyg is set
    aOutputData.DrawEdit(sal_True);
    pContentDev->SetMapMode(MAP_PIXEL);

        // Autofilter- und Pivot-Buttons

    DrawButtons( nX1, nY1, nX2, nY2, aTabInfo, pContentDev );          // Pixel

        // Notiz-Anzeiger

    if ( rOpts.GetOption( VOPT_NOTES ) )
        aOutputData.DrawNoteMarks();

    aOutputData.DrawClipMarks();

    //  Szenario / ChangeTracking muss auf jeden Fall nach DrawGrid sein, auch bei !bGridFirst

    //! Test, ob ChangeTrack-Anzeige aktiv ist
    //! Szenario-Rahmen per View-Optionen abschaltbar?

    SCTAB nTabCount = pDoc->GetTableCount();
    const std::vector<ScHighlightEntry> &rHigh = pViewData->GetView()->GetHighlightRanges();
    sal_Bool bHasScenario = ( nTab+1<nTabCount && pDoc->IsScenario(nTab+1) && !pDoc->IsScenario(nTab) );
    sal_Bool bHasChange = ( pDoc->GetChangeTrack() != NULL );

    if ( bHasChange || bHasScenario || !rHigh.empty() )
    {

        //! SetChangedClip() mit DrawMarks() zusammenfassen?? (anderer MapMode!)

        sal_Bool bAny = sal_True;
        if (eMode == SC_UPDATE_CHANGED)
            bAny = aOutputData.SetChangedClip();
        if (bAny)
        {
            if ( bHasChange )
                aOutputData.DrawChangeTrack();

            if ( bHasScenario )
                lcl_DrawScenarioFrames( pContentDev, pViewData, eWhich, nX1,nY1,nX2,nY2 );

            lcl_DrawHighlight( aOutputData, pViewData, rHigh );

            if (eMode == SC_UPDATE_CHANGED)
                pContentDev->SetClipRegion();
        }
    }

        // Drawing Vordergrund

    pContentDev->SetMapMode(aDrawMode);

    DrawRedraw( aOutputData, eMode, SC_LAYER_FRONT );
    DrawRedraw( aOutputData, eMode, SC_LAYER_INTERN );
    DrawSdrGrid( aDrawingRectLogic, pContentDev );

    if (!bIsInScroll)                               // Drawing Markierungen
    {
        if(eMode == SC_UPDATE_CHANGED && aOutputData.SetChangedClip())
        {
            pContentDev->SetClipRegion();
        }
    }

    pContentDev->SetMapMode(MAP_PIXEL);

    if ( pViewData->IsRefMode() && nTab >= pViewData->GetRefStartZ() && nTab <= pViewData->GetRefEndZ() )
    {
        Color aRefColor( rColorCfg.GetColorValue(svtools::CALCREFERENCE).nColor );
        aOutputData.DrawRefMark( pViewData->GetRefStartX(), pViewData->GetRefStartY(),
                                 pViewData->GetRefEndX(), pViewData->GetRefEndY(),
                                 aRefColor, false );
    }

        //  Range-Finder

    ScInputHandler* pHdl = pScMod->GetInputHdl( pViewData->GetViewShell() );
    if (pHdl)
    {
        ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
        if ( pRangeFinder && !pRangeFinder->IsHidden() &&
                pRangeFinder->GetDocName() == pDocSh->GetTitle() )
        {
            sal_uInt16 nCount = (sal_uInt16)pRangeFinder->Count();
            for (sal_uInt16 i=0; i<nCount; i++)
            {
                ScRangeFindData* pData = pRangeFinder->GetObject(i);

                ScRange aRef = pData->aRef;
                aRef.Justify();
                if ( aRef.aStart.Tab() >= nTab && aRef.aEnd.Tab() <= nTab )
                    aOutputData.DrawRefMark( aRef.aStart.Col(), aRef.aStart.Row(),
                                            aRef.aEnd.Col(), aRef.aEnd.Row(),
                                            Color( ScRangeFindList::GetColorName( i ) ),
                                            sal_True );
            }
        }
    }

    {
        // end redraw
        ScTabViewShell* pTabViewShell = pViewData->GetViewShell();

        if(pTabViewShell)
        {
            MapMode aCurrentMapMode(pContentDev->GetMapMode());
            pContentDev->SetMapMode(aDrawMode);
            SdrView* pDrawView = pTabViewShell->GetSdrView();

            if(pDrawView)
            {
                // #i74769# work with SdrPaintWindow directly
                pDrawView->EndDrawLayers(*pTargetPaintWindow, true);
            }

            pContentDev->SetMapMode(aCurrentMapMode);
        }
    }

    //  InPlace Edit-View
    // moved after EndDrawLayers() to get it outside the overlay buffer and
    // on top of everything
    if ( bEditMode && (pViewData->GetRefTabNo() == pViewData->GetTabNo()) )
    {
        //! use pContentDev for EditView?
        SetMapMode(MAP_PIXEL);
        SCCOL nCol1 = pViewData->GetEditStartCol();
        SCROW nRow1 = pViewData->GetEditStartRow();
        SCCOL nCol2 = pViewData->GetEditEndCol();
        SCROW nRow2 = pViewData->GetEditEndRow();
        SetLineColor();
        SetFillColor( pEditView->GetBackgroundColor() );
        Point aStart = pViewData->GetScrPos( nCol1, nRow1, eWhich );
        Point aEnd = pViewData->GetScrPos( nCol2+1, nRow2+1, eWhich );
        aEnd.X() -= 2 * nLayoutSign;        // don't overwrite grid
        aEnd.Y() -= 2;
        DrawRect( Rectangle( aStart,aEnd ) );

        SetMapMode(pViewData->GetLogicMode());
        pEditView->Paint( PixelToLogic( Rectangle( Point( nScrX, nScrY ),
                            Size( aOutputData.GetScrW(), aOutputData.GetScrH() ) ) ) );
        SetMapMode(MAP_PIXEL);
    }

    if (pViewData->HasEditView(eWhich))
    {
        // flush OverlayManager before changing the MapMode
        flushOverlayManager();

        // set MapMode for text edit
        SetMapMode(pViewData->GetLogicMode());
    }
    else
        SetMapMode(aDrawMode);

    if ( pNoteMarker )
        pNoteMarker->Draw();        // ueber den Cursor, im Drawing-MapMode

    //
    //  Wenn waehrend des Paint etwas invertiert wurde (Selektion geaendert aus Basic-Macro),
    //  ist das jetzt durcheinandergekommen und es muss neu gemalt werden
    //

    OSL_ENSURE(nPaintCount, "nPaintCount falsch");
    --nPaintCount;
    if (!nPaintCount)
        CheckNeedsRepaint();

    // Flag drawn formula cells "unchanged".
    pDoc->ResetChanged(ScRange(nX1,nY1,nTab,nX2,nY2,nTab));
}

void ScGridWindow::CheckNeedsRepaint()
{
    //  called at the end of painting, and from timer after background text width calculation

    if (bNeedsRepaint)
    {
        bNeedsRepaint = false;
        if (aRepaintPixel.IsEmpty())
            Invalidate();
        else
            Invalidate(PixelToLogic(aRepaintPixel));
        aRepaintPixel = Rectangle();

        // selection function in status bar might also be invalid
        SfxBindings& rBindings = pViewData->GetBindings();
        rBindings.Invalidate( SID_STATUS_SUM );
        rBindings.Invalidate( SID_ATTR_SIZE );
        rBindings.Invalidate( SID_TABLE_CELL );
    }
}

void ScGridWindow::DrawPagePreview( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, OutputDevice* pContentDev )
{
    ScPageBreakData* pPageData = pViewData->GetView()->GetPageBreakData();
    if (pPageData)
    {
        ScDocument* pDoc = pViewData->GetDocument();
        SCTAB nTab = pViewData->GetTabNo();
        Size aWinSize = GetOutputSizePixel();
        const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
        Color aManual( rColorCfg.GetColorValue(svtools::CALCPAGEBREAKMANUAL).nColor );
        Color aAutomatic( rColorCfg.GetColorValue(svtools::CALCPAGEBREAK).nColor );

        String aPageStr = ScGlobal::GetRscString( STR_PGNUM );
        if ( nPageScript == 0 )
        {
            //  get script type of translated "Page" string only once
            nPageScript = pDoc->GetStringScriptType( aPageStr );
            if (nPageScript == 0)
                nPageScript = ScGlobal::GetDefaultScriptType();
        }

        Font aFont;
        ScEditEngineDefaulter* pEditEng = NULL;
        const ScPatternAttr& rDefPattern = ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN));
        if ( nPageScript == SCRIPTTYPE_LATIN )
        {
            //  use single font and call DrawText directly
            rDefPattern.GetFont( aFont, SC_AUTOCOL_BLACK );
            aFont.SetColor( Color( COL_LIGHTGRAY ) );
            //  font size is set as needed
        }
        else
        {
            //  use EditEngine to draw mixed-script string
            pEditEng = new ScEditEngineDefaulter( EditEngine::CreatePool(), sal_True );
            pEditEng->SetRefMapMode( pContentDev->GetMapMode() );
            SfxItemSet* pEditDefaults = new SfxItemSet( pEditEng->GetEmptyItemSet() );
            rDefPattern.FillEditItemSet( pEditDefaults );
            pEditDefaults->Put( SvxColorItem( Color( COL_LIGHTGRAY ), EE_CHAR_COLOR ) );
            pEditEng->SetDefaults( pEditDefaults );
        }

        sal_uInt16 nCount = sal::static_int_cast<sal_uInt16>( pPageData->GetCount() );
        for (sal_uInt16 nPos=0; nPos<nCount; nPos++)
        {
            ScPrintRangeData& rData = pPageData->GetData(nPos);
            ScRange aRange = rData.GetPrintRange();
            if ( aRange.aStart.Col() <= nX2+1  && aRange.aEnd.Col()+1 >= nX1 &&
                 aRange.aStart.Row() <= nY2+1 && aRange.aEnd.Row()+1 >= nY1 )
            {
                //  3 Pixel Rahmen um den Druckbereich
                //  (mittlerer Pixel auf den Gitterlinien)

                pContentDev->SetLineColor();
                if (rData.IsAutomatic())
                    pContentDev->SetFillColor( aAutomatic );
                else
                    pContentDev->SetFillColor( aManual );

                Point aStart = pViewData->GetScrPos(
                                    aRange.aStart.Col(), aRange.aStart.Row(), eWhich, sal_True );
                Point aEnd = pViewData->GetScrPos(
                                    aRange.aEnd.Col() + 1, aRange.aEnd.Row() + 1, eWhich, sal_True );
                aStart.X() -= 2;
                aStart.Y() -= 2;

                //  Ueberlaeufe verhindern:
                if ( aStart.X() < -10 ) aStart.X() = -10;
                if ( aStart.Y() < -10 ) aStart.Y() = -10;
                if ( aEnd.X() > aWinSize.Width() + 10 )
                    aEnd.X() = aWinSize.Width() + 10;
                if ( aEnd.Y() > aWinSize.Height() + 10 )
                    aEnd.Y() = aWinSize.Height() + 10;

                pContentDev->DrawRect( Rectangle( aStart, Point(aEnd.X(),aStart.Y()+2) ) );
                pContentDev->DrawRect( Rectangle( aStart, Point(aStart.X()+2,aEnd.Y()) ) );
                pContentDev->DrawRect( Rectangle( Point(aStart.X(),aEnd.Y()-2), aEnd ) );
                pContentDev->DrawRect( Rectangle( Point(aEnd.X()-2,aStart.Y()), aEnd ) );

                //  Seitenumbrueche
                //! anders darstellen (gestrichelt ????)

                size_t nColBreaks = rData.GetPagesX();
                const SCCOL* pColEnd = rData.GetPageEndX();
                size_t nColPos;
                for (nColPos=0; nColPos+1<nColBreaks; nColPos++)
                {
                    SCCOL nBreak = pColEnd[nColPos]+1;
                    if ( nBreak >= nX1 && nBreak <= nX2+1 )
                    {
                        //! hidden suchen
                        if (pDoc->HasColBreak(nBreak, nTab) & BREAK_MANUAL)
                            pContentDev->SetFillColor( aManual );
                        else
                            pContentDev->SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        nBreak, aRange.aStart.Row(), eWhich, sal_True );
                        pContentDev->DrawRect( Rectangle( aBreak.X()-1, aStart.Y(), aBreak.X(), aEnd.Y() ) );
                    }
                }

                size_t nRowBreaks = rData.GetPagesY();
                const SCROW* pRowEnd = rData.GetPageEndY();
                size_t nRowPos;
                for (nRowPos=0; nRowPos+1<nRowBreaks; nRowPos++)
                {
                    SCROW nBreak = pRowEnd[nRowPos]+1;
                    if ( nBreak >= nY1 && nBreak <= nY2+1 )
                    {
                        //! hidden suchen
                        if (pDoc->HasRowBreak(nBreak, nTab) & BREAK_MANUAL)
                            pContentDev->SetFillColor( aManual );
                        else
                            pContentDev->SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        aRange.aStart.Col(), nBreak, eWhich, sal_True );
                        pContentDev->DrawRect( Rectangle( aStart.X(), aBreak.Y()-1, aEnd.X(), aBreak.Y() ) );
                    }
                }

                //  Seitenzahlen

                SCROW nPrStartY = aRange.aStart.Row();
                for (nRowPos=0; nRowPos<nRowBreaks; nRowPos++)
                {
                    SCROW nPrEndY = pRowEnd[nRowPos];
                    if ( nPrEndY >= nY1 && nPrStartY <= nY2 )
                    {
                        SCCOL nPrStartX = aRange.aStart.Col();
                        for (nColPos=0; nColPos<nColBreaks; nColPos++)
                        {
                            SCCOL nPrEndX = pColEnd[nColPos];
                            if ( nPrEndX >= nX1 && nPrStartX <= nX2 )
                            {
                                Point aPageStart = pViewData->GetScrPos(
                                                        nPrStartX, nPrStartY, eWhich, sal_True );
                                Point aPageEnd = pViewData->GetScrPos(
                                                        nPrEndX+1,nPrEndY+1, eWhich, sal_True );

                                long nPageNo = rData.GetFirstPage();
                                if ( rData.IsTopDown() )
                                    nPageNo += ((long)nColPos)*nRowBreaks+nRowPos;
                                else
                                    nPageNo += ((long)nRowPos)*nColBreaks+nColPos;

                                String aThisPageStr = aPageStr; // Don't modify the original string.
                                aThisPageStr.SearchAndReplaceAscii("%1", String::CreateFromInt32(nPageNo));

                                if ( pEditEng )
                                {
                                    //  find right font size with EditEngine
                                    long nHeight = 100;
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
                                    pEditEng->SetText( aThisPageStr );
                                    Size aSize100( pEditEng->CalcTextWidth(), pEditEng->GetTextHeight() );

                                    //  40% of width or 60% of height
                                    long nSizeX = 40 * ( aPageEnd.X() - aPageStart.X() ) / aSize100.Width();
                                    long nSizeY = 60 * ( aPageEnd.Y() - aPageStart.Y() ) / aSize100.Height();
                                    nHeight = Min(nSizeX,nSizeY);
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );

                                    //  centered output with EditEngine
                                    Size aTextSize( pEditEng->CalcTextWidth(), pEditEng->GetTextHeight() );
                                    Point aPos( (aPageStart.X()+aPageEnd.X()-aTextSize.Width())/2,
                                                (aPageStart.Y()+aPageEnd.Y()-aTextSize.Height())/2 );
                                    pEditEng->Draw( pContentDev, aPos );
                                }
                                else
                                {
                                    //  find right font size for DrawText
                                    aFont.SetSize( Size( 0,100 ) );
                                    pContentDev->SetFont( aFont );
                                    Size aSize100( pContentDev->GetTextWidth( aThisPageStr ), pContentDev->GetTextHeight() );

                                    //  40% of width or 60% of height
                                    long nSizeX = 40 * ( aPageEnd.X() - aPageStart.X() ) / aSize100.Width();
                                    long nSizeY = 60 * ( aPageEnd.Y() - aPageStart.Y() ) / aSize100.Height();
                                    aFont.SetSize( Size( 0,Min(nSizeX,nSizeY) ) );
                                    pContentDev->SetFont( aFont );

                                    //  centered output with DrawText
                                    Size aTextSize( pContentDev->GetTextWidth( aThisPageStr ), pContentDev->GetTextHeight() );
                                    Point aPos( (aPageStart.X()+aPageEnd.X()-aTextSize.Width())/2,
                                                (aPageStart.Y()+aPageEnd.Y()-aTextSize.Height())/2 );
                                    pContentDev->DrawText( aPos, aThisPageStr );
                                }
                            }
                            nPrStartX = nPrEndX + 1;
                        }
                    }
                    nPrStartY = nPrEndY + 1;
                }
            }
        }

        delete pEditEng;
    }
}

void ScGridWindow::DrawButtons( SCCOL nX1, SCROW /*nY1*/, SCCOL nX2, SCROW /*nY2*/, ScTableInfo& rTabInfo, OutputDevice* pContentDev )
{
    aComboButton.SetOutputDevice( pContentDev );

    ScDocument* pDoc = pViewData->GetDocument();
    ScDPFieldButton aCellBtn(pContentDev, &GetSettings().GetStyleSettings(), &pViewData->GetZoomX(), &pViewData->GetZoomY(), pDoc);

    SCCOL nCol;
    SCROW nRow;
    SCSIZE nArrY;
    SCSIZE nQuery;
    SCTAB           nTab = pViewData->GetTabNo();
    ScDBData*       pDBData = NULL;
    ScQueryParam*   pQueryParam = NULL;

    RowInfo*        pRowInfo = rTabInfo.mpRowInfo;
    sal_uInt16          nArrCount = rTabInfo.mnArrCount;

    sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );

    Point aOldPos  = aComboButton.GetPosPixel();    // Zustand fuer MouseDown/Up
    Size  aOldSize = aComboButton.GetSizePixel();   // merken

    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        if ( pRowInfo[nArrY].bAutoFilter && pRowInfo[nArrY].bChanged )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];

            nRow = pThisRowInfo->nRowNo;


            for (nCol=nX1; nCol<=nX2; nCol++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nCol+1];
                if ( pInfo->bAutoFilter && !pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    if (!pQueryParam)
                        pQueryParam = new ScQueryParam;

                    sal_Bool bNewData = sal_True;
                    if (pDBData)
                    {
                        SCCOL nStartCol;
                        SCROW nStartRow;
                        SCCOL nEndCol;
                        SCROW nEndRow;
                        SCTAB nAreaTab;
                        pDBData->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
                        if ( nCol >= nStartCol && nCol <= nEndCol &&
                             nRow >= nStartRow && nRow <= nEndRow )
                            bNewData = false;
                    }
                    if (bNewData)
                    {
                        pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab );
                        if (pDBData)
                            pDBData->GetQueryParam( *pQueryParam );
                        else
                        {
                            // can also be part of DataPilot table
                            // OSL_FAIL("Auto-Filter-Button ohne DBData");
                        }
                    }

                    //  pQueryParam kann nur MAXQUERY Eintraege enthalten

                    sal_Bool bSimpleQuery = sal_True;
                    sal_Bool bColumnFound = false;
                    if (!pQueryParam->bInplace)
                        bSimpleQuery = false;
                    SCSIZE nCount = pQueryParam->GetEntryCount();
                    for (nQuery = 0; nQuery < nCount && bSimpleQuery; ++nQuery)
                        if (pQueryParam->GetEntry(nQuery).bDoQuery)
                        {
                            //  hier nicht auf EQUAL beschraenken
                            //  (auch bei ">1" soll der Spaltenkopf blau werden)

                            if (pQueryParam->GetEntry(nQuery).nField == nCol)
                                bColumnFound = sal_True;
                            if (nQuery > 0)
                                if (pQueryParam->GetEntry(nQuery).eConnect != SC_AND)
                                    bSimpleQuery = false;
                        }

                    bool bArrowState = bSimpleQuery && bColumnFound;
                    long    nSizeX;
                    long    nSizeY;
                    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
                    Point aScrPos = pViewData->GetScrPos( nCol, nRow, eWhich );

                    aCellBtn.setBoundingBox(aScrPos, Size(nSizeX-1, nSizeY-1), bLayoutRTL);
                    aCellBtn.setPopupLeft(bLayoutRTL);   // #i114944# AutoFilter button is left-aligned in RTL
                    aCellBtn.setDrawBaseButton(false);
                    aCellBtn.setDrawPopupButton(true);
                    aCellBtn.setHasHiddenMember(bArrowState);
                    aCellBtn.draw();
                }
            }
        }

        if ( pRowInfo[nArrY].bPushButton && pRowInfo[nArrY].bChanged )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            nRow = pThisRowInfo->nRowNo;
            for (nCol=nX1; nCol<=nX2; nCol++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nCol+1];
                if ( pInfo->bPushButton && !pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    Point aScrPos = pViewData->GetScrPos( nCol, nRow, eWhich );
                    long nSizeX;
                    long nSizeY;
                    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
                    long nPosX = aScrPos.X();
                    long nPosY = aScrPos.Y();
                    // bLayoutRTL is handled in setBoundingBox

                    String aStr;
                    pDoc->GetString(nCol, nRow, nTab, aStr);
                    aCellBtn.setText(aStr);
                    aCellBtn.setBoundingBox(Point(nPosX, nPosY), Size(nSizeX-1, nSizeY-1), bLayoutRTL);
                    aCellBtn.setPopupLeft(false);   // DataPilot popup is always right-aligned for now
                    aCellBtn.setDrawBaseButton(true);
                    aCellBtn.setDrawPopupButton(pInfo->bPopupButton);
                    aCellBtn.setHasHiddenMember(pInfo->bFilterActive);
                    aCellBtn.draw();
                }
            }
        }

        if ( bListValButton && pRowInfo[nArrY].nRowNo == aListValPos.Row() && pRowInfo[nArrY].bChanged )
        {
            Rectangle aRect = GetListValButtonRect( aListValPos );
            aComboButton.SetPosPixel( aRect.TopLeft() );
            aComboButton.SetSizePixel( aRect.GetSize() );
            pContentDev->SetClipRegion( aRect );
            aComboButton.Draw( false, false );
            pContentDev->SetClipRegion();           // always called from Draw() without clip region
            aComboButton.SetPosPixel( aOldPos );    // restore old state
            aComboButton.SetSizePixel( aOldSize );  // for MouseUp/Down (AutoFilter)
        }
    }

    delete pQueryParam;
    aComboButton.SetOutputDevice( this );
}

Rectangle ScGridWindow::GetListValButtonRect( const ScAddress& rButtonPos )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    ScDDComboBoxButton aButton( this );             // for optimal size
    Size aBtnSize = aButton.GetSizePixel();

    SCCOL nCol = rButtonPos.Col();
    SCROW nRow = rButtonPos.Row();

    long nCellSizeX;    // width of this cell, including merged
    long nDummy;
    pViewData->GetMergeSizePixel( nCol, nRow, nCellSizeX, nDummy );

    // for height, only the cell's row is used, excluding merged cells
    long nCellSizeY = ScViewData::ToPixel( pDoc->GetRowHeight( nRow, nTab ), pViewData->GetPPTY() );
    long nAvailable = nCellSizeX;

    //  left edge of next cell if there is a non-hidden next column
    SCCOL nNextCol = nCol + 1;
    const ScMergeAttr* pMerge = static_cast<const ScMergeAttr*>(pDoc->GetAttr( nCol,nRow,nTab, ATTR_MERGE ));
    if ( pMerge->GetColMerge() > 1 )
        nNextCol = nCol + pMerge->GetColMerge();    // next cell after the merged area
    while ( nNextCol <= MAXCOL && pDoc->ColHidden(nNextCol, nTab) )
        ++nNextCol;
    sal_Bool bNextCell = ( nNextCol <= MAXCOL );
    if ( bNextCell )
        nAvailable = ScViewData::ToPixel( pDoc->GetColWidth( nNextCol, nTab ), pViewData->GetPPTX() );

    if ( nAvailable < aBtnSize.Width() )
        aBtnSize.Width() = nAvailable;
    if ( nCellSizeY < aBtnSize.Height() )
        aBtnSize.Height() = nCellSizeY;

    Point aPos = pViewData->GetScrPos( nCol, nRow, eWhich, sal_True );
    aPos.X() += nCellSizeX * nLayoutSign;               // start of next cell
    if (!bNextCell)
        aPos.X() -= aBtnSize.Width() * nLayoutSign;     // right edge of cell if next cell not available
    aPos.Y() += nCellSizeY - aBtnSize.Height();
    // X remains at the left edge

    if ( bLayoutRTL )
        aPos.X() -= aBtnSize.Width()-1;     // align right edge of button with cell border

    return Rectangle( aPos, aBtnSize );
}

bool ScGridWindow::IsAutoFilterActive( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScDocument*     pDoc    = pViewData->GetDocument();
    ScDBData*       pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab );
    ScQueryParam    aQueryParam;

    if ( pDBData )
        pDBData->GetQueryParam( aQueryParam );
    else
    {
        OSL_FAIL("Auto-Filter-Button ohne DBData");
    }

    bool    bSimpleQuery = true;
    bool    bColumnFound = false;
    SCSIZE  nQuery;

    if ( !aQueryParam.bInplace )
        bSimpleQuery = false;

    //  aQueryParam kann nur MAXQUERY Eintraege enthalten

    SCSIZE nCount = aQueryParam.GetEntryCount();
    for (nQuery = 0; nQuery < nCount && bSimpleQuery; ++nQuery)
        if ( aQueryParam.GetEntry(nQuery).bDoQuery )
        {
            if (aQueryParam.GetEntry(nQuery).nField == nCol)
                bColumnFound = sal_True;

            if (nQuery > 0)
                if (aQueryParam.GetEntry(nQuery).eConnect != SC_AND)
                    bSimpleQuery = false;
        }

    return ( bSimpleQuery && bColumnFound );
}

void ScGridWindow::GetSelectionRects( ::std::vector< Rectangle >& rPixelRects )
{
    ScMarkData aMultiMark( pViewData->GetMarkData() );
    aMultiMark.SetMarking( false );
    aMultiMark.MarkToMulti();

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();

    sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    if ( !aMultiMark.IsMultiMarked() )
        return;

    ScRange aMultiRange;
    aMultiMark.GetMultiMarkArea( aMultiRange );
    SCCOL nX1 = aMultiRange.aStart.Col();
    SCROW nY1 = aMultiRange.aStart.Row();
    SCCOL nX2 = aMultiRange.aEnd.Col();
    SCROW nY2 = aMultiRange.aEnd.Row();

    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    sal_Bool bTestMerge = sal_True;
    bool bRepeat = true;

    SCCOL nTestX2 = nX2;
    SCROW nTestY2 = nY2;
    if (bTestMerge)
        pDoc->ExtendMerge( nX1,nY1, nTestX2,nTestY2, nTab );

    SCCOL nPosX = pViewData->GetPosX( eHWhich );
    SCROW nPosY = pViewData->GetPosY( eVWhich );
    if (nTestX2 < nPosX || nTestY2 < nPosY)
        return;                                         // unsichtbar
    SCCOL nRealX1 = nX1;
    if (nX1 < nPosX)
        nX1 = nPosX;
    if (nY1 < nPosY)
        nY1 = nPosY;

    SCCOL nXRight = nPosX + pViewData->VisibleCellsX(eHWhich);
    if (nXRight > MAXCOL) nXRight = MAXCOL;
    SCROW nYBottom = nPosY + pViewData->VisibleCellsY(eVWhich);
    if (nYBottom > MAXROW) nYBottom = MAXROW;

    if (nX1 > nXRight || nY1 > nYBottom)
        return;                                         // unsichtbar
    if (nX2 > nXRight) nX2 = nXRight;
    if (nY2 > nYBottom) nY2 = nYBottom;

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    ScInvertMerger aInvert( &rPixelRects );

    Point aScrPos = pViewData->GetScrPos( nX1, nY1, eWhich );
    long nScrY = aScrPos.Y();
    sal_Bool bWasHidden = false;
    for (SCROW nY=nY1; nY<=nY2; nY++)
    {
        sal_Bool bFirstRow = ( nY == nPosY );                       // first visible row?
        sal_Bool bDoHidden = false;                                 // versteckte nachholen ?
        sal_uInt16 nHeightTwips = pDoc->GetRowHeight( nY,nTab );
        sal_Bool bDoRow = ( nHeightTwips != 0 );
        if (bDoRow)
        {
            if (bTestMerge)
                if (bWasHidden)                 // auf versteckte zusammengefasste testen
                {
                    bDoHidden = sal_True;
                    bDoRow = sal_True;
                }

            bWasHidden = false;
        }
        else
        {
            bWasHidden = sal_True;
            if (bTestMerge)
                if (nY==nY2)
                    bDoRow = sal_True;              // letzte Zeile aus Block
        }

        if ( bDoRow )
        {
            SCCOL nLoopEndX = nX2;
            if (nX2 < nX1)                      // Rest von zusammengefasst
            {
                SCCOL nStartX = nX1;
                while ( ((const ScMergeFlagAttr*)pDoc->
                            GetAttr(nStartX,nY,nTab,ATTR_MERGE_FLAG))->IsHorOverlapped() )
                    --nStartX;
                if (nStartX <= nX2)
                    nLoopEndX = nX1;
            }

            long nEndY = nScrY + ScViewData::ToPixel( nHeightTwips, nPPTY ) - 1;
            long nScrX = aScrPos.X();
            for (SCCOL nX=nX1; nX<=nLoopEndX; nX++)
            {
                long nWidth = ScViewData::ToPixel( pDoc->GetColWidth( nX,nTab ), nPPTX );
                if ( nWidth > 0 )
                {
                    long nEndX = nScrX + ( nWidth - 1 ) * nLayoutSign;
                    if (bTestMerge)
                    {
                        SCROW nThisY = nY;
                        const ScPatternAttr* pPattern = pDoc->GetPattern( nX, nY, nTab );
                        const ScMergeFlagAttr* pMergeFlag = (const ScMergeFlagAttr*) &pPattern->
                                                                        GetItem(ATTR_MERGE_FLAG);
                        if ( pMergeFlag->IsVerOverlapped() && ( bDoHidden || bFirstRow ) )
                        {
                            while ( pMergeFlag->IsVerOverlapped() && nThisY > 0 &&
                                    (pDoc->RowHidden(nThisY-1, nTab) || bFirstRow) )
                            {
                                --nThisY;
                                pPattern = pDoc->GetPattern( nX, nThisY, nTab );
                                pMergeFlag = (const ScMergeFlagAttr*) &pPattern->GetItem(ATTR_MERGE_FLAG);
                            }
                        }

                        // nur Rest von zusammengefasster zu sehen ?
                        SCCOL nThisX = nX;
                        if ( pMergeFlag->IsHorOverlapped() && nX == nPosX && nX > nRealX1 )
                        {
                            while ( pMergeFlag->IsHorOverlapped() )
                            {
                                --nThisX;
                                pPattern = pDoc->GetPattern( nThisX, nThisY, nTab );
                                pMergeFlag = (const ScMergeFlagAttr*) &pPattern->GetItem(ATTR_MERGE_FLAG);
                            }
                        }

                        if ( aMultiMark.IsCellMarked( nThisX, nThisY, sal_True ) == bRepeat )
                        {
                            if ( !pMergeFlag->IsOverlapped() )
                            {
                                ScMergeAttr* pMerge = (ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
                                if (pMerge->GetColMerge() > 0 || pMerge->GetRowMerge() > 0)
                                {
                                    Point aEndPos = pViewData->GetScrPos(
                                            nThisX + pMerge->GetColMerge(),
                                            nThisY + pMerge->GetRowMerge(), eWhich );
                                    if ( aEndPos.X() * nLayoutSign > nScrX * nLayoutSign && aEndPos.Y() > nScrY )
                                    {
                                        aInvert.AddRect( Rectangle( nScrX,nScrY,
                                                    aEndPos.X()-nLayoutSign,aEndPos.Y()-1 ) );
                                    }
                                }
                                else if ( nEndX * nLayoutSign >= nScrX * nLayoutSign && nEndY >= nScrY )
                                {
                                    aInvert.AddRect( Rectangle( nScrX,nScrY,nEndX,nEndY ) );
                                }
                            }
                        }
                    }
                    else        // !bTestMerge
                    {
                        if ( aMultiMark.IsCellMarked( nX, nY, sal_True ) == bRepeat &&
                                                nEndX * nLayoutSign >= nScrX * nLayoutSign && nEndY >= nScrY )
                        {
                            aInvert.AddRect( Rectangle( nScrX,nScrY,nEndX,nEndY ) );
                        }
                    }

                    nScrX = nEndX + nLayoutSign;
                }
            }
            nScrY = nEndY + 1;
        }
    }
}

// -------------------------------------------------------------------------

void ScGridWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged(rDCEvt);

    if ( (rDCEvt.GetType() == DATACHANGED_PRINTER) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( rDCEvt.GetType() == DATACHANGED_FONTS && eWhich == pViewData->GetActivePart() )
            pViewData->GetDocShell()->UpdateFontList();

        if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
             (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        {
            if ( eWhich == pViewData->GetActivePart() )     // only once for the view
            {
                ScTabView* pView = pViewData->GetView();

                //  update scale in case the UI ScreenZoom has changed
                ScGlobal::UpdatePPT(this);
                pView->RecalcPPT();

                //  RepeatResize in case scroll bar sizes have changed
                pView->RepeatResize();
                pView->UpdateAllOverlays();

                //  invalidate cell attribs in input handler, in case the
                //  EditEngine BackgroundColor has to be changed
                if ( pViewData->IsActive() )
                {
                    ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
                    if (pHdl)
                        pHdl->ForgetLastPattern();
                }
            }
        }

        Invalidate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
