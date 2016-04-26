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

#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editview.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/printer.hxx>
#include <vcl/settings.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#include <svx/svdview.hxx>
#include "tabvwsh.hxx"

#include "gridwin.hxx"
#include "viewdata.hxx"
#include "output.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "dbdata.hxx"
#include "docoptio.hxx"
#include "notemark.hxx"
#include "dbfunc.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "rfindlst.hxx"
#include "hiranges.hxx"
#include "pagedata.hxx"
#include "docpool.hxx"
#include "globstr.hrc"
#include "docsh.hxx"
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
#include <svx/sdrpaintwindow.hxx>

static void lcl_LimitRect( Rectangle& rRect, const Rectangle& rVisible )
{
    if ( rRect.Top()    < rVisible.Top()-1 )    rRect.Top()    = rVisible.Top()-1;
    if ( rRect.Bottom() > rVisible.Bottom()+1 ) rRect.Bottom() = rVisible.Bottom()+1;

    // The header row must be drawn also when the inner rectangle is not visible,
    // that is why there is no return value anymore.
    // When it is far away, then lcl_DrawOneFrame is not even called.
}

static void lcl_DrawOneFrame( vcl::RenderContext* pDev, const Rectangle& rInnerPixel,
                        const OUString& rTitle, const Color& rColor, bool bTextBelow,
                        double nPPTX, double nPPTY, const Fraction& rZoomY,
                        ScDocument* pDoc, ScViewData* pButtonViewData, bool bLayoutRTL )
{
    // pButtonViewData is only used to set the button size,
    // can otherwise be NULL!

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
    vcl::Font aAttrFont;
    static_cast<const ScPatternAttr&>(pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
                                    GetFont(aAttrFont,SC_AUTOCOL_BLACK,pDev,&rZoomY);

    //  everything else from application font
    vcl::Font aAppFont = pDev->GetSettings().GetStyleSettings().GetAppFont();
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
    //  left, top, right, bottom
    pDev->DrawRect( Rectangle( aOuter.Left(),  aOuter.Top(),    aInner.Left(),  aOuter.Bottom() ) );
    pDev->DrawRect( Rectangle( aOuter.Left(),  aOuter.Top(),    aOuter.Right(), aInner.Top()    ) );
    pDev->DrawRect( Rectangle( aInner.Right(), aOuter.Top(),    aOuter.Right(), aOuter.Bottom() ) );
    pDev->DrawRect( Rectangle( aOuter.Left(),  aInner.Bottom(), aOuter.Right(), aOuter.Bottom() ) );

    long nButtonY = bTextBelow ? aInner.Bottom() : aOuter.Top();

    ScDDComboBoxButton aComboButton(pDev);
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

    bool bWasClip = false;
    vcl::Region aOldClip;
    bool bClip = ( aTextSize.Width() > aOuter.Right() - nBWidth - aInner.Left() );
    if ( bClip )
    {
        if (pDev->IsClipRegion())
        {
            bWasClip = true;
            aOldClip = pDev->GetActiveClipRegion();
        }
        long nClipStartX = bLayoutRTL ? aOuter.Left() + nBWidth : aInner.Left();
        long nClipEndX = bLayoutRTL ? aInner.Right() : aOuter.Right() - nBWidth;
        pDev->SetClipRegion( vcl::Region(Rectangle( nClipStartX, nButtonY + nVer/2,
                            nClipEndX, nButtonY + nVer/2 + aTextSize.Height())) );
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
        if ( nY1>=2 ) nY1 -= 2;             // Hack: Header row affects two cells
        else if ( nY1 > 0 ) --nY1;
        if ( nX2 < MAXCOL ) ++nX2;
        if ( nY2 < MAXROW-1 ) nY2 += 2;     // Hack: Header row affects two cells
        else if ( nY2 < MAXROW ) ++nY2;
        ScRange aViewRange( nX1,nY1,nTab, nX2,nY2,nTab );

        //! cache the ranges in table!!!!

        ScMarkData aMarks;
        for (SCTAB i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
            pDoc->MarkScenario( i, nTab, aMarks, false, SC_SCENARIO_SHOWFRAME );
        ScRangeListRef xRanges = new ScRangeList;
        aMarks.FillRangeListWithMarks( xRanges, false );

        bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
        long nLayoutSign = bLayoutRTL ? -1 : 1;

        for (size_t j = 0, n = xRanges->size(); j < n; ++j)
        {
            ScRange aRange = *(*xRanges)[j];
            // Always extend scenario frame to merged cells where no new non-covered cells
            // are framed
            pDoc->ExtendTotalMerge( aRange );

            //! -> Extend repaint when merging !!!

            if ( aRange.Intersects( aViewRange ) )          //! Space for Text/Button?
            {
                Point aStartPos = pViewData->GetScrPos(
                                    aRange.aStart.Col(), aRange.aStart.Row(), eWhich, true );
                Point aEndPos = pViewData->GetScrPos(
                                    aRange.aEnd.Col()+1, aRange.aEnd.Row()+1, eWhich, true );
                //  on the grid:
                aStartPos.X() -= nLayoutSign;
                aStartPos.Y() -= 1;
                aEndPos.X() -= nLayoutSign;
                aEndPos.Y() -= 1;

                bool bTextBelow = ( aRange.aStart.Row() == 0 );

                OUString aCurrent;
                Color aColor( COL_LIGHTGRAY );
                for (SCTAB nAct=nTab+1; nAct<nTabCount && pDoc->IsScenario(nAct); nAct++)
                    if ( pDoc->IsActiveScenario(nAct) && pDoc->HasScenarioRange(nAct,aRange) )
                    {
                        OUString aDummyComment;
                        sal_uInt16 nDummyFlags;
                        pDoc->GetName( nAct, aCurrent );
                        pDoc->GetScenarioData( nAct, aDummyComment, aColor, nDummyFlags );
                    }

                if (aCurrent.isEmpty())
                    aCurrent = ScGlobal::GetRscString( STR_EMPTYDATA );

                //! Own text "(None)" instead of "(Empty)" ???

                lcl_DrawOneFrame( pDev, Rectangle( aStartPos, aEndPos ),
                                    aCurrent, aColor, bTextBelow,
                                    pViewData->GetPPTX(), pViewData->GetPPTY(), pViewData->GetZoomY(),
                                    pDoc, pViewData, bLayoutRTL );
            }
        }
    }
}

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

void ScGridWindow::DoInvertRect( const Rectangle& rPixel )
{
    if ( rPixel == aInvertRect )
        aInvertRect = Rectangle();      // Cancel
    else
    {
        OSL_ENSURE( aInvertRect.IsEmpty(), "DoInvertRect no pairs" );

        aInvertRect = rPixel;           // Mark new rectangle
    }

    UpdateHeaderOverlay();      // uses aInvertRect
}

void ScGridWindow::PrePaint(vcl::RenderContext& /*rRenderContext*/)
{
    // forward PrePaint to DrawingLayer
    ScTabViewShell* pTabViewShell = pViewData->GetViewShell();

    if(pTabViewShell)
    {
        SdrView* pDrawView = pTabViewShell->GetSdrView();

        if (pDrawView)
        {
            pDrawView->PrePaint();
        }
    }
}

void ScGridWindow::Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect )
{
    ScDocument* pDoc = pViewData->GetDocument();
    if ( pDoc->IsInInterpreter() )
    {
        // Via Reschedule, interpretended cells do not trigger Invalidate again,
        // otherwise for instance an error box would never appear (bug 36381).
        // Later, through bNeedsRepaint everything is painted again.
        if ( bNeedsRepaint )
        {
            //! Merge Rectangle?
            aRepaintPixel = Rectangle();            // multiple -> paint all
        }
        else
        {
            bNeedsRepaint = true;
            aRepaintPixel = LogicToPixel(rRect);    // only affected ranges
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

    Draw( nX1,nY1,nX2,nY2, SC_UPDATE_MARKS ); // don't continue with painting

    bIsInPaint = false;
}

void ScGridWindow::Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, ScUpdateMode eMode )
{
    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();

    // let's ignore the normal Draw() attempts when doing the tiled rendering,
    // all the rendering should go through PaintTile() in that case.
    // TODO revisit if we can actually turn this into an assert(), and clean
    // up the callers
    if (comphelper::LibreOfficeKit::isActive())
        return;

    ScModule* pScMod = SC_MOD();
    bool bTextWysiwyg = pScMod->GetInputOptions().GetTextWysiwyg();

    if (pViewData->IsMinimized())
        return;

    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    OSL_ENSURE( ValidCol(nX2) && ValidRow(nY2), "GridWin Draw area too big" );

    UpdateVisibleRange();

    if (nX2 < maVisibleRange.mnCol1 || nY2 < maVisibleRange.mnRow1)
        return;
    // invisible
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
        nX2 = maVisibleRange.mnCol2;  // to continue painting

    // point of no return

    ++nPaintCount; // mark that painting is in progress

    SCTAB nTab = pViewData->GetTabNo();
    rDoc.ExtendHidden( nX1, nY1, nX2, nY2, nTab );

    Point aScrPos = pViewData->GetScrPos( nX1, nY1, eWhich );
    long nMirrorWidth = GetSizePixel().Width();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
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
    rDoc.ExtendMerge( nCurX, nCurY, nCurEndX, nCurEndY, nTab );
    bool bCurVis = nCursorHideCount==0 &&
                    ( nCurEndX+1 >= nX1 && nCurX <= nX2+1 && nCurEndY+1 >= nY1 && nCurY <= nY2+1 );

    //  AutoFill Handles
    if ( !bCurVis && nCursorHideCount==0 && bAutoMarkVisible && aAutoMarkPos.Tab() == nTab &&
            ( aAutoMarkPos.Col() != nCurX || aAutoMarkPos.Row() != nCurY ) )
    {
        SCCOL nHdlX = aAutoMarkPos.Col();
        SCROW nHdlY = aAutoMarkPos.Row();
        rDoc.ExtendMerge( nHdlX, nHdlY, nHdlX, nHdlY, nTab );
        // left and top is unaffected

        //! Paint AutoFill handles alone (without Cursor) ???
    }

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    const ScViewOptions& rOpts = pViewData->GetOptions();

    // data block

    ScTableInfo aTabInfo;
    rDoc.FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nTab,
                                        nPPTX, nPPTY, false, rOpts.GetOption(VOPT_FORMULAS),
                                        &pViewData->GetMarkData() );

    Fraction aZoomX = pViewData->GetZoomX();
    Fraction aZoomY = pViewData->GetZoomY();
    ScOutputData aOutputData( this, OUTTYPE_WINDOW, aTabInfo, &rDoc, nTab,
                               nScrX, nScrY, nX1, nY1, nX2, nY2, nPPTX, nPPTY,
                               &aZoomX, &aZoomY );

    aOutputData.SetMirrorWidth( nMirrorWidth ); // needed for RTL
    aOutputData.SetSpellCheckContext(mpSpellCheckCxt.get());

    ScopedVclPtr< VirtualDevice > xFmtVirtDev;
    bool bLogicText = bTextWysiwyg; // call DrawStrings in logic MapMode?

    if ( bTextWysiwyg )
    {
        //  use printer for text formatting

        OutputDevice* pFmtDev = rDoc.GetPrinter();
        pFmtDev->SetMapMode( pViewData->GetLogicMode(eWhich) );
        aOutputData.SetFmtDevice( pFmtDev );
    }
    else if ( aZoomX != aZoomY && pViewData->IsOle() )
    {
        //  #i45033# For OLE inplace editing with different zoom factors,
        //  use a virtual device with 1/100th mm as text formatting reference

        xFmtVirtDev.disposeAndReset( VclPtr<VirtualDevice>::Create() );
        xFmtVirtDev->SetMapMode( MAP_100TH_MM );
        aOutputData.SetFmtDevice( xFmtVirtDev.get() );

        bLogicText = true; // use logic MapMode
    }

    DrawContent(*this, aTabInfo, aOutputData, bLogicText, eMode);

    // If something was inverted during the Paint (selection changed from Basic Macro)
    // then this is now mixed up and has to be repainted
    OSL_ENSURE(nPaintCount, "Wrong nPaintCount");
    --nPaintCount;
    if (!nPaintCount)
        CheckNeedsRepaint();

    // Flag drawn formula cells "unchanged".
    rDoc.ResetChanged(ScRange(nX1, nY1, nTab, nX2, nY2, nTab));
    rDoc.ClearFormulaContext();
}

void ScGridWindow::DrawContent(OutputDevice &rDevice, const ScTableInfo& rTableInfo, ScOutputData& aOutputData,
        bool bLogicText, ScUpdateMode eMode)
{
    ScModule* pScMod = SC_MOD();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    const ScViewOptions& rOpts = pViewData->GetOptions();
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();

    SCTAB nTab = aOutputData.nTab;
    SCCOL nX1 = aOutputData.nX1;
    SCROW nY1 = aOutputData.nY1;
    SCCOL nX2 = aOutputData.nX2;
    SCROW nY2 = aOutputData.nY2;
    long nScrX = aOutputData.nScrX;
    long nScrY = aOutputData.nScrY;

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
    aOutputData.SetShowFormulas     ( rOpts.GetOption( VOPT_FORMULAS ) );
    aOutputData.SetShowSpellErrors  ( rDoc.GetDocOptions().IsAutoSpell() );
    aOutputData.SetMarkClipped      ( rOpts.GetOption( VOPT_CLIPMARKS ) );

    aOutputData.SetUseStyleColor( true );       // always set in table view

    aOutputData.SetEditObject( GetEditObject() );
    aOutputData.SetViewShell( pViewData->GetViewShell() );

    bool bGrid = rOpts.GetOption( VOPT_GRID ) && pViewData->GetShowGrid();
    bool bGridFirst = !rOpts.GetOption( VOPT_GRID_ONTOP );

    bool bPage = rOpts.GetOption( VOPT_PAGEBREAKS );

    if ( eMode == SC_UPDATE_CHANGED )
    {
        aOutputData.FindChanged();
        aOutputData.SetSingleGrid(true);
    }

    bool bPageMode = pViewData->IsPagebreakMode();
    if (bPageMode)                                      // after FindChanged
    {
        // SetPagebreakMode initialisiert auch bPrinted Flags
        aOutputData.SetPagebreakMode( pViewData->GetView()->GetPageBreakData() );
    }

    EditView*   pEditView = nullptr;
    bool        bEditMode = pViewData->HasEditView(eWhich);
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
    MapMode aDrawMode = GetDrawMapMode();
    if (bIsTiledRendering)
    {
        // FIXME this shouldn't be necessary once we change the entire Calc to
        // work in the logic coordinates (ideally 100ths of mm - so that it is
        // the same as editeng and drawinglayer), and get rid of all the
        // SetMapMode's and other unnecessary fun we have with pixels
        // See also ScGridWindow::GetDrawMapMode() for the rest of this hack
        aDrawMode.SetOrigin(PixelToLogic(Point(nScrX, nScrY), aDrawMode));
    }
    Rectangle aDrawingRectLogic;
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

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

    OutputDevice* pContentDev = &rDevice;   // device for document content, used by overlay manager
    SdrPaintWindow* pTargetPaintWindow = nullptr; // #i74769# work with SdrPaintWindow directly

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
                vcl::Region aDrawingRegion(aDrawingRectLogic);
                pTargetPaintWindow = pDrawView->BeginDrawLayers(pContentDev, aDrawingRegion);
                OSL_ENSURE(pTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

                if (!bIsTiledRendering)
                {
                    // #i74769# get target device from SdrPaintWindow, this may be the prerender
                    // device now, too.
                    pContentDev = &(pTargetPaintWindow->GetTargetOutputDevice());
                    aOutputData.SetContentDevice(pContentDev);
                }
            }

            pContentDev->SetMapMode(aCurrentMapMode);
        }
    }

    // edge (area) (Pixel)
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

    if ( rDoc.HasBackgroundDraw( nTab, aDrawingRectLogic ) )
    {
        pContentDev->SetMapMode(MAP_PIXEL);
        aOutputData.DrawClear();

            // drawing background

        pContentDev->SetMapMode(aDrawMode);
        DrawRedraw( aOutputData, eMode, SC_LAYER_BACK );
    }
    else
        aOutputData.SetSolidBackground(true);

    aOutputData.DrawDocumentBackground();

    if ( bGridFirst && ( bGrid || bPage ) )
        aOutputData.DrawGrid(*pContentDev, bGrid, bPage);

    aOutputData.DrawBackground(*pContentDev);

    if ( !bGridFirst && ( bGrid || bPage ) )
        aOutputData.DrawGrid(*pContentDev, bGrid, bPage);

    pContentDev->SetMapMode(MAP_PIXEL);

    if ( bPageMode )
    {
        // DrawPagePreview draws complete lines/page numbers, must always be clipped
        if ( aOutputData.SetChangedClip() )
        {
            DrawPagePreview(nX1,nY1,nX2,nY2, *pContentDev);
            pContentDev->SetClipRegion();
        }
    }

    aOutputData.DrawShadow();
    aOutputData.DrawFrame(*pContentDev);

    // Show Note Mark
    if ( rOpts.GetOption( VOPT_NOTES ) )
        aOutputData.DrawNoteMarks(*pContentDev);

    if ( !bLogicText )
        aOutputData.DrawStrings();     // in pixel MapMode

    // edit cells and printer-metrics text must be before the buttons
    // (DataPilot buttons contain labels in UI font)

    pContentDev->SetMapMode(pViewData->GetLogicMode(eWhich));
    if ( bLogicText )
        aOutputData.DrawStrings(true);      // in logic MapMode if bLogicText is set
    aOutputData.DrawEdit(true);

    // the buttons are painted in absolute coordinates
    if (bIsTiledRendering)
    {
        // Tiled offset nScrX, nScrY
        MapMode aMap( MAP_PIXEL );
        aMap.SetOrigin(Point(nScrX, nScrY));
        pContentDev->SetMapMode(aMap);
    }
    else
        pContentDev->SetMapMode(MAP_PIXEL);

        // Autofilter- and Pivot-Buttons

    DrawButtons(nX1, nX2, rTableInfo, pContentDev);          // Pixel

    pContentDev->SetMapMode(MAP_PIXEL);

    aOutputData.DrawClipMarks();

    // In any case, Szenario / ChangeTracking must happen after DrawGrid, also for !bGridFirst

    //! Test, ob ChangeTrack-Anzeige aktiv ist
    //! Szenario-Rahmen per View-Optionen abschaltbar?

    SCTAB nTabCount = rDoc.GetTableCount();
    const std::vector<ScHighlightEntry> &rHigh = pViewData->GetView()->GetHighlightRanges();
    bool bHasScenario = ( nTab+1<nTabCount && rDoc.IsScenario(nTab+1) && !rDoc.IsScenario(nTab) );
    bool bHasChange = ( rDoc.GetChangeTrack() != nullptr );

    if ( bHasChange || bHasScenario || !rHigh.empty() )
    {

        //! Merge SetChangedClip() with DrawMarks() ?? (different MapMode!)

        bool bAny = true;
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

        // Drawing foreground

    pContentDev->SetMapMode(aDrawMode);

    DrawRedraw( aOutputData, eMode, SC_LAYER_FRONT );
    DrawRedraw( aOutputData, eMode, SC_LAYER_INTERN );
    DrawSdrGrid( aDrawingRectLogic, pContentDev );

    if (!bIsInScroll)                               // Drawing marks
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

        // range finder

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
                ScRangeFindData& rData = pRangeFinder->GetObject(i);

                ScRange aRef = rData.aRef;
                aRef.PutInOrder();
                if ( aRef.aStart.Tab() >= nTab && aRef.aEnd.Tab() <= nTab )
                    aOutputData.DrawRefMark( aRef.aStart.Col(), aRef.aStart.Row(),
                                            aRef.aEnd.Col(), aRef.aEnd.Row(),
                                            Color( rData.nColorData ),
                                            true );
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

    // In-place editing - when the user is typing, we need to paint the text
    // using the editeng.
    // It's being done after EndDrawLayers() to get it outside the overlay
    // buffer and on top of everything.
    if ( bEditMode && (pViewData->GetRefTabNo() == pViewData->GetTabNo()) )
    {
        // get the coordinates of the area we need to clear (overpaint by
        // the background)
        SCCOL nCol1 = pViewData->GetEditStartCol();
        SCROW nRow1 = pViewData->GetEditStartRow();
        SCCOL nCol2 = pViewData->GetEditEndCol();
        SCROW nRow2 = pViewData->GetEditEndRow();
        rDevice.SetLineColor();
        rDevice.SetFillColor(pEditView->GetBackgroundColor());
        Point aStart = pViewData->GetScrPos( nCol1, nRow1, eWhich );
        Point aEnd = pViewData->GetScrPos( nCol2+1, nRow2+1, eWhich );

        // don't overwrite grid
        long nLayoutSign = bLayoutRTL ? -1 : 1;
        aEnd.X() -= 2 * nLayoutSign;
        aEnd.Y() -= 2;

        // toggle the cursor off if its on to ensure the cursor invert
        // background logic remains valid after the background is cleared on
        // the next cursor flash
        vcl::Cursor* pCrsr = pEditView->GetCursor();
        const bool bVisCursor = pCrsr && pCrsr->IsVisible();
        if (bVisCursor)
            pCrsr->Hide();

        // set the correct mapmode
        Rectangle aBackground(aStart, aEnd);
        if (bIsTiledRendering)
        {
            aBackground += Point(nScrX, nScrY);
            rDevice.SetMapMode(aDrawMode);
        }
        else
            rDevice.SetMapMode(pViewData->GetLogicMode());

        // paint the background
        rDevice.DrawRect(rDevice.PixelToLogic(aBackground));

        // paint the editeng text
        pEditView->Paint(rDevice.PixelToLogic(Rectangle(Point(nScrX, nScrY), Size(aOutputData.GetScrW(), aOutputData.GetScrH()))), &rDevice);
        rDevice.SetMapMode(MAP_PIXEL);

        // restore the cursor it was originally visible
        if (bVisCursor)
            pCrsr->Show();
    }

    if (pViewData->HasEditView(eWhich))
    {
        // flush OverlayManager before changing the MapMode
        flushOverlayManager();

        // set MapMode for text edit
        rDevice.SetMapMode(pViewData->GetLogicMode());
    }
    else
        rDevice.SetMapMode(aDrawMode);

    if (mpNoteMarker)
        mpNoteMarker->Draw(); // Above the cursor, in drawing map mode
}

void ScGridWindow::PaintTile( VirtualDevice& rDevice,
                              int nOutputWidth, int nOutputHeight,
                              int nTilePosX, int nTilePosY,
                              long nTileWidth, long nTileHeight )
{
    // Output size is in pixels while tile position and size are in logical units (twips).

    // Assumption: always paint the whole sheet i.e. "visible" range is always
    // from (0,0) to last data position.

    // Tile geometry is independent of the zoom level, but the output size is
    // dependent of the zoom level.  Determine the correct zoom level before
    // we start.

    // FIXME the painting works using a mixture of drawing with coordinates in
    // pixels and in logic coordinates; it should be cleaned up to use logic
    // coords only, and avoid all the SetMapMode()'s.
    // Similarly to Writer, we should set the mapmode once on the rDevice, and
    // not care about any zoom settings.

    Fraction aFracX(long(nOutputWidth * TWIPS_PER_PIXEL), nTileWidth);
    Fraction aFracY(long(nOutputHeight * TWIPS_PER_PIXEL), nTileHeight);

    // page break zoom, and aLogicMode in ScViewData
    pViewData->SetZoom(aFracX, aFracY, true);

    const double fTilePosXPixel = static_cast<double>(nTilePosX) * nOutputWidth / nTileWidth;
    const double fTilePosYPixel = static_cast<double>(nTilePosY) * nOutputHeight / nTileHeight;
    const double fTileBottomPixel = static_cast<double>(nTilePosY + nTileHeight) * nOutputHeight / nTileHeight;
    const double fTileRightPixel = static_cast<double>(nTilePosX + nTileWidth) * nOutputWidth / nTileWidth;

    SCTAB nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();

    SCCOL nStartCol = 0, nEndCol = 0;
    SCROW nStartRow = 0, nEndRow = 0;

    // size of the document including drawings, charts, etc.
    pDoc->GetTiledRenderingArea(nTab, nEndCol, nEndRow);

    const double fPPTX = pViewData->GetPPTX();
    const double fPPTY = pViewData->GetPPTY();

    // Calculate the tile's first and last rows.
    SCROW firstRow = -1;
    SCROW lastRow = -1;
    double fTopOffsetPixel = 0;

    // Find the first and last rows to paint this tile.
    sal_uInt16 nDocHeight = ScGlobal::nStdRowHeight;
    SCROW nDocHeightEndRow = -1;
    for (SCROW nY = nStartRow; nY <= nEndRow; ++nY)
    {
        if (nY > nDocHeightEndRow)
        {
            if (ValidRow(nY))
                nDocHeight = pDoc->GetRowHeight( nY, nTab, nullptr, &nDocHeightEndRow );
            else
                nDocHeight = ScGlobal::nStdRowHeight;
        }

        auto rowHeight = static_cast<sal_uInt16>(nDocHeight * fPPTY);
        if (fTopOffsetPixel + rowHeight >= fTilePosYPixel)
        {
            if (firstRow < 0)
            {
                firstRow = nY;
            }
            else if (fTopOffsetPixel + rowHeight > fTileBottomPixel)
            {
                lastRow = nY;
                break;
            }
        }

        fTopOffsetPixel += rowHeight;
    }

    firstRow = (firstRow >= 0 ? firstRow : nStartRow);
    lastRow = (lastRow >= 0 ? lastRow : nEndRow);

    // Find the first and last cols to paint this tile.
    SCCOL firstCol = -1;
    SCCOL lastCol = -1;
    double fLeftOffsetPixel = 0;
    for (SCCOL nArrCol=nStartCol+3; nArrCol<=nEndCol+2; ++nArrCol)
    {
        SCCOL nX = nArrCol-1;
        if ( ValidCol(nX) )
        {
            if (!pDoc->ColHidden(nX, nTab))
            {
                sal_uInt16 nColWidth = (sal_uInt16) (pDoc->GetColWidth( nX, nTab ) * fPPTX);
                if (!nColWidth)
                    nColWidth = 1;

                if (fLeftOffsetPixel + nColWidth >= fTilePosXPixel)
                {
                    if (firstCol < 0)
                    {
                        firstCol = nX;
                    }
                    else if (fLeftOffsetPixel + nColWidth > fTileRightPixel)
                    {
                        lastCol = nX;
                        break;
                    }
                }

                fLeftOffsetPixel += nColWidth;
            }
        }
    }

    firstCol = (firstCol >= 0 ? firstCol : nStartCol);
    lastCol = (lastCol >= 0 ? lastCol : nEndCol);

    auto capacity = std::min(nEndRow + 3, sal_Int32(1002));
    ScTableInfo aTabInfo(capacity);
    pDoc->FillInfo(aTabInfo, nStartCol, nStartRow, lastCol, lastRow, nTab, fPPTX, fPPTY, false, false, NULL);

    ScOutputData aOutputData(&rDevice, OUTTYPE_WINDOW, aTabInfo, pDoc, nTab,
            -fTilePosXPixel, -fTilePosYPixel, nStartCol, firstRow, lastCol, lastRow,
            fPPTX, fPPTY);

    // setup the SdrPage so that drawinglayer works correctly
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    std::unique_ptr<FmFormView> pDrawView;
    if (pModel)
    {
        pDrawView.reset(new FmFormView(pModel, &rDevice));
        pDrawView->ShowSdrPage(pDrawView->GetModel()->GetPage(nTab));
        aOutputData.SetDrawView( pDrawView.get() );
    }

    // draw the content
    DrawContent(rDevice, aTabInfo, aOutputData, true, SC_UPDATE_ALL);
}

void ScGridWindow::LogicInvalidate(const Rectangle* pRectangle)
{
    OString sRectangle;
    if (!pRectangle)
        sRectangle = "EMPTY";
    else
    {
        Rectangle aRectangle(*pRectangle);
        // When dragging shapes the map mode is disabled.
        if (IsMapModeEnabled())
        {
            if (GetMapMode().GetMapUnit() == MAP_100TH_MM)
                aRectangle = OutputDevice::LogicToLogic(aRectangle, MAP_100TH_MM, MAP_TWIP);
        }
        else
            aRectangle = PixelToLogic(aRectangle, MapMode(MAP_TWIP));
        sRectangle = aRectangle.toString();
    }

    if (comphelper::LibreOfficeKit::isViewCallback())
    {
        ScTabViewShell* pViewShell = pViewData->GetViewShell();
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, sRectangle.getStr());
    }
    else
        pViewData->GetDocument()->GetDrawLayer()->libreOfficeKitCallback(LOK_CALLBACK_INVALIDATE_TILES, sRectangle.getStr());
}

void ScGridWindow::SetCellSelectionPixel(int nType, int nPixelX, int nPixelY)
{
    ScTabView* pTabView = pViewData->GetView();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    ScInputHandler* pInputHandler = SC_MOD()->GetInputHdl(pViewShell);

    if (pInputHandler && pInputHandler->IsInputMode())
    {
        // we need to switch off the editeng
        ScTabView::UpdateInputLine();
        pViewShell->UpdateInputHandler();
    }

    if (nType == LOK_SETTEXTSELECTION_RESET)
    {
        pTabView->DoneBlockMode();
        return;
    }

    // obtain the current selection
    ScRangeList aRangeList = pViewData->GetMarkData().GetMarkedRanges();

    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    SCTAB nTab1, nTab2;

    bool bWasEmpty = false;
    if (aRangeList.empty())
    {
        nCol1 = nCol2 = pViewData->GetCurX();
        nRow1 = nRow2 = pViewData->GetCurY();
        bWasEmpty = true;
    }
    else
        aRangeList.Combine().GetVars(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);

    // convert the coordinates to column/row
    SCsCOL nNewPosX;
    SCsROW nNewPosY;
    SCTAB nTab = pViewData->GetTabNo();
    pViewData->GetPosFromPixel(nPixelX, nPixelY, eWhich, nNewPosX, nNewPosY);

    // change the selection
    switch (nType)
    {
        case LOK_SETTEXTSELECTION_START:
            if (nNewPosX != nCol1 || nNewPosY != nRow1 || bWasEmpty)
            {
                pTabView->SetCursor(nNewPosX, nNewPosY);
                pTabView->DoneBlockMode();
                pTabView->InitBlockMode(nNewPosX, nNewPosY, nTab, true);
                pTabView->MarkCursor(nCol2, nRow2, nTab);
            }
            break;
        case LOK_SETTEXTSELECTION_END:
            if (nNewPosX != nCol2 || nNewPosY != nRow2 || bWasEmpty)
            {
                pTabView->SetCursor(nCol1, nRow1);
                pTabView->DoneBlockMode();
                pTabView->InitBlockMode(nCol1, nRow1, nTab, true);
                pTabView->MarkCursor(nNewPosX, nNewPosY, nTab);
            }
            break;
        default:
            assert(false);
            break;
    }
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

void ScGridWindow::DrawPagePreview( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, vcl::RenderContext& rRenderContext)
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

        OUString aPageStr = ScGlobal::GetRscString( STR_PGNUM );
        if ( nPageScript == SvtScriptType::NONE )
        {
            //  get script type of translated "Page" string only once
            nPageScript = pDoc->GetStringScriptType( aPageStr );
            if (nPageScript == SvtScriptType::NONE)
                nPageScript = ScGlobal::GetDefaultScriptType();
        }

        vcl::Font aFont;
        std::unique_ptr<ScEditEngineDefaulter> pEditEng;
        const ScPatternAttr& rDefPattern = static_cast<const ScPatternAttr&>(pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN));
        if ( nPageScript == SvtScriptType::LATIN )
        {
            //  use single font and call DrawText directly
            rDefPattern.GetFont( aFont, SC_AUTOCOL_BLACK );
            aFont.SetColor( Color( COL_LIGHTGRAY ) );
            //  font size is set as needed
        }
        else
        {
            //  use EditEngine to draw mixed-script string
            pEditEng.reset(new ScEditEngineDefaulter( EditEngine::CreatePool(), true ));
            pEditEng->SetRefMapMode(rRenderContext.GetMapMode());
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
                // 3 pixel frame around the print area
                //  (middle pixel on the grid lines)

                rRenderContext.SetLineColor();
                if (rData.IsAutomatic())
                    rRenderContext.SetFillColor( aAutomatic );
                else
                    rRenderContext.SetFillColor( aManual );

                Point aStart = pViewData->GetScrPos(
                                    aRange.aStart.Col(), aRange.aStart.Row(), eWhich, true );
                Point aEnd = pViewData->GetScrPos(
                                    aRange.aEnd.Col() + 1, aRange.aEnd.Row() + 1, eWhich, true );
                aStart.X() -= 2;
                aStart.Y() -= 2;

                // Prevent overflows:
                if ( aStart.X() < -10 ) aStart.X() = -10;
                if ( aStart.Y() < -10 ) aStart.Y() = -10;
                if ( aEnd.X() > aWinSize.Width() + 10 )
                    aEnd.X() = aWinSize.Width() + 10;
                if ( aEnd.Y() > aWinSize.Height() + 10 )
                    aEnd.Y() = aWinSize.Height() + 10;

                rRenderContext.DrawRect( Rectangle( aStart, Point(aEnd.X(),aStart.Y()+2) ) );
                rRenderContext.DrawRect( Rectangle( aStart, Point(aStart.X()+2,aEnd.Y()) ) );
                rRenderContext.DrawRect( Rectangle( Point(aStart.X(),aEnd.Y()-2), aEnd ) );
                rRenderContext.DrawRect( Rectangle( Point(aEnd.X()-2,aStart.Y()), aEnd ) );

                // Page breaks
                //! Display differently (dashed ????)

                size_t nColBreaks = rData.GetPagesX();
                const SCCOL* pColEnd = rData.GetPageEndX();
                size_t nColPos;
                for (nColPos=0; nColPos+1<nColBreaks; nColPos++)
                {
                    SCCOL nBreak = pColEnd[nColPos]+1;
                    if ( nBreak >= nX1 && nBreak <= nX2+1 )
                    {
                        //! Search for hidden
                        if (pDoc->HasColBreak(nBreak, nTab) & BREAK_MANUAL)
                            rRenderContext.SetFillColor( aManual );
                        else
                            rRenderContext.SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        nBreak, aRange.aStart.Row(), eWhich, true );
                        rRenderContext.DrawRect( Rectangle( aBreak.X()-1, aStart.Y(), aBreak.X(), aEnd.Y() ) );
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
                        //! Search for hidden
                        if (pDoc->HasRowBreak(nBreak, nTab) & BREAK_MANUAL)
                            rRenderContext.SetFillColor( aManual );
                        else
                            rRenderContext.SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        aRange.aStart.Col(), nBreak, eWhich, true );
                        rRenderContext.DrawRect( Rectangle( aStart.X(), aBreak.Y()-1, aEnd.X(), aBreak.Y() ) );
                    }
                }

                // Page numbers

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
                                                        nPrStartX, nPrStartY, eWhich, true );
                                Point aPageEnd = pViewData->GetScrPos(
                                                        nPrEndX+1,nPrEndY+1, eWhich, true );

                                long nPageNo = rData.GetFirstPage();
                                if ( rData.IsTopDown() )
                                    nPageNo += ((long)nColPos)*nRowBreaks+nRowPos;
                                else
                                    nPageNo += ((long)nRowPos)*nColBreaks+nColPos;

                                OUString aThisPageStr = aPageStr.replaceFirst("%1", OUString::number(nPageNo));

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
                                    nHeight = std::min(nSizeX,nSizeY);
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
                                    pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );

                                    //  centered output with EditEngine
                                    Size aTextSize( pEditEng->CalcTextWidth(), pEditEng->GetTextHeight() );
                                    Point aPos( (aPageStart.X()+aPageEnd.X()-aTextSize.Width())/2,
                                                (aPageStart.Y()+aPageEnd.Y()-aTextSize.Height())/2 );
                                    pEditEng->Draw( &rRenderContext, aPos );
                                }
                                else
                                {
                                    //  find right font size for DrawText
                                    aFont.SetSize( Size( 0,100 ) );
                                    rRenderContext.SetFont( aFont );
                                    Size aSize100(rRenderContext.GetTextWidth( aThisPageStr ), rRenderContext.GetTextHeight() );

                                    //  40% of width or 60% of height
                                    long nSizeX = 40 * ( aPageEnd.X() - aPageStart.X() ) / aSize100.Width();
                                    long nSizeY = 60 * ( aPageEnd.Y() - aPageStart.Y() ) / aSize100.Height();
                                    aFont.SetSize( Size( 0,std::min(nSizeX,nSizeY) ) );
                                    rRenderContext.SetFont( aFont );

                                    //  centered output with DrawText
                                    Size aTextSize(rRenderContext.GetTextWidth( aThisPageStr ), rRenderContext.GetTextHeight() );
                                    Point aPos( (aPageStart.X()+aPageEnd.X()-aTextSize.Width())/2,
                                                (aPageStart.Y()+aPageEnd.Y()-aTextSize.Height())/2 );
                                    rRenderContext.DrawText( aPos, aThisPageStr );
                                }
                            }
                            nPrStartX = nPrEndX + 1;
                        }
                    }
                    nPrStartY = nPrEndY + 1;
                }
            }
        }
    }
}

void ScGridWindow::DrawButtons(SCCOL nX1, SCCOL nX2, const ScTableInfo& rTabInfo, OutputDevice* pContentDev)
{
    aComboButton.SetOutputDevice( pContentDev );

    ScDocument* pDoc = pViewData->GetDocument();
    ScDPFieldButton aCellBtn(pContentDev, &GetSettings().GetStyleSettings(), &pViewData->GetZoomX(), &pViewData->GetZoomY(), pDoc);

    SCCOL nCol;
    SCROW nRow;
    SCSIZE nArrY;
    SCSIZE nQuery;
    SCTAB           nTab = pViewData->GetTabNo();
    ScDBData*       pDBData = nullptr;
    std::unique_ptr<ScQueryParam> pQueryParam;

    RowInfo*        pRowInfo = rTabInfo.mpRowInfo;
    sal_uInt16          nArrCount = rTabInfo.mnArrCount;

    bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );

    Point aOldPos  = aComboButton.GetPosPixel();    // store state for MouseDown/Up
    Size  aOldSize = aComboButton.GetSizePixel();

    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        if ( pRowInfo[nArrY].bAutoFilter && pRowInfo[nArrY].bChanged )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];

            nRow = pThisRowInfo->nRowNo;

            for (nCol=nX1; nCol<=nX2; nCol++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nCol+1];
                //if several columns merged on a row, there should be only one auto button at the end of the columns.
                //if several rows merged on a column, the button may be in the middle, so "!pInfo->bVOverlapped" should not be used
                if ( pInfo->bAutoFilter && !pInfo->bHOverlapped )
                {
                    if (!pQueryParam)
                        pQueryParam.reset(new ScQueryParam);

                    bool bNewData = true;
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
                        pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab, ScDBDataPortion::AREA );
                        if (pDBData)
                            pDBData->GetQueryParam( *pQueryParam );
                        else
                        {
                            // can also be part of DataPilot table
                        }
                    }

                    //  pQueryParam can only include MAXQUERY entries

                    bool bSimpleQuery = true;
                    bool bColumnFound = false;
                    if (!pQueryParam->bInplace)
                        bSimpleQuery = false;
                    SCSIZE nCount = pQueryParam->GetEntryCount();
                    for (nQuery = 0; nQuery < nCount && bSimpleQuery; ++nQuery)
                        if (pQueryParam->GetEntry(nQuery).bDoQuery)
                        {
                            //  Do no restrict to EQUAL here
                            //  (Column head should become blue also when ">1")

                            if (pQueryParam->GetEntry(nQuery).nField == nCol)
                                bColumnFound = true;
                            if (nQuery > 0)
                                if (pQueryParam->GetEntry(nQuery).eConnect != SC_AND)
                                    bSimpleQuery = false;
                        }

                    bool bArrowState = bSimpleQuery && bColumnFound;
                    long    nSizeX;
                    long    nSizeY;
                    SCCOL nStartCol= nCol;
                    SCROW nStartRow = nRow;
                    //if address(nCol,nRow) is not the start pos of the merge area, the value of the nSizeX will be incorrect, it will be the length of the cell.
                    //should first get the start pos of the merge area, then get the nSizeX through the start pos.
                    pDoc->ExtendOverlapped(nStartCol, nStartRow,nCol, nRow, nTab);//get nStartCol,nStartRow
                    pViewData->GetMergeSizePixel( nStartCol, nStartRow, nSizeX, nSizeY );//get nSizeX
                    nSizeY = ScViewData::ToPixel(pDoc->GetRowHeight(nRow, nTab), pViewData->GetPPTY());
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

        if ( pRowInfo[nArrY].bPivotButton && pRowInfo[nArrY].bChanged )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            nRow = pThisRowInfo->nRowNo;
            for (nCol=nX1; nCol<=nX2; nCol++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nCol+1];
                if (pInfo->bHOverlapped || pInfo->bVOverlapped)
                    continue;

                Point aScrPos = pViewData->GetScrPos( nCol, nRow, eWhich );
                long nSizeX;
                long nSizeY;
                pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
                long nPosX = aScrPos.X();
                long nPosY = aScrPos.Y();
                // bLayoutRTL is handled in setBoundingBox

                OUString aStr = pDoc->GetString(nCol, nRow, nTab);
                aCellBtn.setText(aStr);
                aCellBtn.setBoundingBox(Point(nPosX, nPosY), Size(nSizeX-1, nSizeY-1), bLayoutRTL);
                aCellBtn.setPopupLeft(false);   // DataPilot popup is always right-aligned for now
                aCellBtn.setDrawBaseButton(pInfo->bPivotButton);
                aCellBtn.setDrawPopupButton(pInfo->bPivotPopupButton);
                aCellBtn.setHasHiddenMember(pInfo->bFilterActive);
                aCellBtn.draw();
            }
        }

        if ( bListValButton && pRowInfo[nArrY].nRowNo == aListValPos.Row() && pRowInfo[nArrY].bChanged )
        {
            Rectangle aRect = GetListValButtonRect( aListValPos );
            aComboButton.SetPosPixel( aRect.TopLeft() );
            aComboButton.SetSizePixel( aRect.GetSize() );
            pContentDev->SetClipRegion(vcl::Region(aRect));
            aComboButton.Draw( false );
            pContentDev->SetClipRegion();           // always called from Draw() without clip region
            aComboButton.SetPosPixel( aOldPos );    // restore old state
            aComboButton.SetSizePixel( aOldSize );  // for MouseUp/Down (AutoFilter)
        }
    }

    pQueryParam.reset();
    aComboButton.SetOutputDevice( this );
}

Rectangle ScGridWindow::GetListValButtonRect( const ScAddress& rButtonPos )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
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
    bool bNextCell = ( nNextCol <= MAXCOL );
    if ( bNextCell )
        nAvailable = ScViewData::ToPixel( pDoc->GetColWidth( nNextCol, nTab ), pViewData->GetPPTX() );

    if ( nAvailable < aBtnSize.Width() )
        aBtnSize.Width() = nAvailable;
    if ( nCellSizeY < aBtnSize.Height() )
        aBtnSize.Height() = nCellSizeY;

    Point aPos = pViewData->GetScrPos( nCol, nRow, eWhich, true );
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
    ScDBData*       pDBData = pDoc->GetDBAtCursor( nCol, nRow, nTab, ScDBDataPortion::AREA );
    ScQueryParam    aQueryParam;

    if ( pDBData )
        pDBData->GetQueryParam( aQueryParam );
    else
    {
        OSL_FAIL("Auto filter button without DBData");
    }

    bool    bSimpleQuery = true;
    bool    bColumnFound = false;
    SCSIZE  nQuery;

    if ( !aQueryParam.bInplace )
        bSimpleQuery = false;

    // aQueryParam can only include MAXQUERY entries

    SCSIZE nCount = aQueryParam.GetEntryCount();
    for (nQuery = 0; nQuery < nCount && bSimpleQuery; ++nQuery)
        if ( aQueryParam.GetEntry(nQuery).bDoQuery )
        {
            if (aQueryParam.GetEntry(nQuery).nField == nCol)
                bColumnFound = true;

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

    bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
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

    bool bTestMerge = true;
    bool bRepeat = true;

    SCCOL nTestX2 = nX2;
    SCROW nTestY2 = nY2;
    if (bTestMerge)
        pDoc->ExtendMerge( nX1,nY1, nTestX2,nTestY2, nTab );

    SCCOL nPosX = pViewData->GetPosX( eHWhich );
    SCROW nPosY = pViewData->GetPosY( eVWhich );
    // is the selection visible at all?
    if (nTestX2 < nPosX || nTestY2 < nPosY)
        return;
    SCCOL nRealX1 = nX1;
    if (nX1 < nPosX)
        nX1 = nPosX;
    if (nY1 < nPosY)
        nY1 = nPosY;

    if (!comphelper::LibreOfficeKit::isActive())
    {
        // limit the selection to only what is visible on the screen
        SCCOL nXRight = nPosX + pViewData->VisibleCellsX(eHWhich);
        if (nXRight > MAXCOL)
            nXRight = MAXCOL;

        SCROW nYBottom = nPosY + pViewData->VisibleCellsY(eVWhich);
        if (nYBottom > MAXROW)
            nYBottom = MAXROW;

        // is the selection visible at all?
        if (nX1 > nXRight || nY1 > nYBottom)
            return;

        if (nX2 > nXRight)
            nX2 = nXRight;
        if (nY2 > nYBottom)
            nY2 = nYBottom;
    }
    else
    {
        SCCOL nMaxTiledCol;
        SCROW nMaxTiledRow;
        pDoc->GetTiledRenderingArea(nTab, nMaxTiledCol, nMaxTiledRow);

        if (nX2 > nMaxTiledCol)
            nX2 = nMaxTiledCol;
        if (nY2 > nMaxTiledRow)
            nY2 = nMaxTiledRow;
    }

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    ScInvertMerger aInvert( &rPixelRects );

    Point aScrPos = pViewData->GetScrPos( nX1, nY1, eWhich );
    long nScrY = aScrPos.Y();
    bool bWasHidden = false;
    for (SCROW nY=nY1; nY<=nY2; nY++)
    {
        bool bFirstRow = ( nY == nPosY );                       // first visible row?
        bool bDoHidden = false;                                 // repeat hidden ?
        sal_uInt16 nHeightTwips = pDoc->GetRowHeight( nY,nTab );
        bool bDoRow = ( nHeightTwips != 0 );
        if (bDoRow)
        {
            if (bTestMerge)
                if (bWasHidden)                 // test hidden merge
                {
                    bDoHidden = true;
                    bDoRow = true;
                }

            bWasHidden = false;
        }
        else
        {
            bWasHidden = true;
            if (bTestMerge)
                if (nY==nY2)
                    bDoRow = true;              // last cell of the block
        }

        if ( bDoRow )
        {
            SCCOL nLoopEndX = nX2;
            if (nX2 < nX1)                      // the rest of the merge
            {
                SCCOL nStartX = nX1;
                while ( static_cast<const ScMergeFlagAttr*>(pDoc->
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
                        const ScMergeFlagAttr* pMergeFlag = static_cast<const ScMergeFlagAttr*>( &pPattern->
                                                                        GetItem(ATTR_MERGE_FLAG) );
                        if ( pMergeFlag->IsVerOverlapped() && ( bDoHidden || bFirstRow ) )
                        {
                            while ( pMergeFlag->IsVerOverlapped() && nThisY > 0 &&
                                    (pDoc->RowHidden(nThisY-1, nTab) || bFirstRow) )
                            {
                                --nThisY;
                                pPattern = pDoc->GetPattern( nX, nThisY, nTab );
                                pMergeFlag = static_cast<const ScMergeFlagAttr*>( &pPattern->GetItem(ATTR_MERGE_FLAG) );
                            }
                        }

                        // only the rest of the merged is seen ?
                        SCCOL nThisX = nX;
                        if ( pMergeFlag->IsHorOverlapped() && nX == nPosX && nX > nRealX1 )
                        {
                            while ( pMergeFlag->IsHorOverlapped() )
                            {
                                --nThisX;
                                pPattern = pDoc->GetPattern( nThisX, nThisY, nTab );
                                pMergeFlag = static_cast<const ScMergeFlagAttr*>( &pPattern->GetItem(ATTR_MERGE_FLAG) );
                            }
                        }

                        if ( aMultiMark.IsCellMarked( nThisX, nThisY, true ) == bRepeat )
                        {
                            if ( !pMergeFlag->IsOverlapped() )
                            {
                                const ScMergeAttr* pMerge = static_cast<const ScMergeAttr*>(&pPattern->GetItem(ATTR_MERGE));
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
                        if ( aMultiMark.IsCellMarked( nX, nY, true ) == bRepeat &&
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

void ScGridWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged(rDCEvt);

    if ( (rDCEvt.GetType() == DataChangedEventType::PRINTER) ||
         (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        if ( rDCEvt.GetType() == DataChangedEventType::FONTS && eWhich == pViewData->GetActivePart() )
            pViewData->GetDocShell()->UpdateFontList();

        if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
             (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
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
