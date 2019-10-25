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

#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editview.hxx>
#include <editeng/fhgtitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/printer.hxx>
#include <vcl/cursor.hxx>
#include <vcl/settings.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

#include <svx/svdview.hxx>
#include <tabvwsh.hxx>

#include <gridwin.hxx>
#include <viewdata.hxx>
#include <output.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <dbdata.hxx>
#include <docoptio.hxx>
#include <notemark.hxx>
#include <dbfunc.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <rfindlst.hxx>
#include <hiranges.hxx>
#include <pagedata.hxx>
#include <docpool.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <docsh.hxx>
#include <cbutton.hxx>
#include <invmerge.hxx>
#include <editutil.hxx>
#include <inputopt.hxx>
#include <fillinfo.hxx>
#include <dpcontrol.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <markdata.hxx>
#include <sc.hrc>
#include <vcl/virdev.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <drwlayer.hxx>
#include <printfun.hxx>

static void lcl_LimitRect( tools::Rectangle& rRect, const tools::Rectangle& rVisible )
{
    if ( rRect.Top()    < rVisible.Top()-1 )    rRect.SetTop( rVisible.Top()-1 );
    if ( rRect.Bottom() > rVisible.Bottom()+1 ) rRect.SetBottom( rVisible.Bottom()+1 );

    // The header row must be drawn also when the inner rectangle is not visible,
    // that is why there is no return value anymore.
    // When it is far away, then lcl_DrawOneFrame is not even called.
}

static void lcl_DrawOneFrame( vcl::RenderContext* pDev, const tools::Rectangle& rInnerPixel,
                        const OUString& rTitle, const Color& rColor, bool bTextBelow,
                        double nPPTX, double nPPTY, const Fraction& rZoomY,
                        ScDocument* pDoc, ScViewData* pButtonViewData, bool bLayoutRTL )
{
    // pButtonViewData is only used to set the button size,
    // can otherwise be NULL!

    tools::Rectangle aInner = rInnerPixel;
    if ( bLayoutRTL )
    {
        aInner.SetLeft( rInnerPixel.Right() );
        aInner.SetRight( rInnerPixel.Left() );
    }

    tools::Rectangle aVisible( Point(0,0), pDev->GetOutputSizePixel() );
    lcl_LimitRect( aInner, aVisible );

    tools::Rectangle aOuter = aInner;
    long nHor = static_cast<long>( SC_SCENARIO_HSPACE * nPPTX );
    long nVer = static_cast<long>( SC_SCENARIO_VSPACE * nPPTY );
    aOuter.AdjustLeft( -nHor );
    aOuter.AdjustRight(nHor );
    aOuter.AdjustTop( -nVer );
    aOuter.AdjustBottom(nVer );

    //  use ScPatternAttr::GetFont only for font size
    vcl::Font aAttrFont;
    pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN).
                                    GetFont(aAttrFont,SC_AUTOCOL_BLACK,pDev,&rZoomY);

    //  everything else from application font
    vcl::Font aAppFont = pDev->GetSettings().GetStyleSettings().GetAppFont();
    aAppFont.SetFontSize( aAttrFont.GetFontSize() );

    aAppFont.SetAlignment( ALIGN_TOP );
    pDev->SetFont( aAppFont );

    Size aTextSize( pDev->GetTextWidth( rTitle ), pDev->GetTextHeight() );

    if ( bTextBelow )
        aOuter.AdjustBottom(aTextSize.Height() );
    else
        aOuter.AdjustTop( -(aTextSize.Height()) );

    pDev->SetLineColor();
    pDev->SetFillColor( rColor );
    //  left, top, right, bottom
    pDev->DrawRect( tools::Rectangle( aOuter.Left(),  aOuter.Top(),    aInner.Left(),  aOuter.Bottom() ) );
    pDev->DrawRect( tools::Rectangle( aOuter.Left(),  aOuter.Top(),    aOuter.Right(), aInner.Top()    ) );
    pDev->DrawRect( tools::Rectangle( aInner.Right(), aOuter.Top(),    aOuter.Right(), aOuter.Bottom() ) );
    pDev->DrawRect( tools::Rectangle( aOuter.Left(),  aInner.Bottom(), aOuter.Right(), aOuter.Bottom() ) );

    long nButtonY = bTextBelow ? aInner.Bottom() : aOuter.Top();

    ScDDComboBoxButton aComboButton(pDev);
    aComboButton.SetOptSizePixel();
    long nBWidth  = long(aComboButton.GetSizePixel().Width() * rZoomY);
    long nBHeight = nVer + aTextSize.Height() + 1;
    Size aButSize( nBWidth, nBHeight );
    long nButtonPos = bLayoutRTL ? aOuter.Left() : aOuter.Right()-nBWidth+1;
    aComboButton.Draw( Point(nButtonPos, nButtonY), aButSize );
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
        pDev->SetClipRegion( vcl::Region(tools::Rectangle( nClipStartX, nButtonY + nVer/2,
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
        if ( nX2 < pDoc->MaxCol() ) ++nX2;
        if ( nY2 < pDoc->MaxRow()-1 ) nY2 += 2;     // Hack: Header row affects two cells
        else if ( nY2 < pDoc->MaxRow() ) ++nY2;
        ScRange aViewRange( nX1,nY1,nTab, nX2,nY2,nTab );

        //! cache the ranges in table!!!!

        ScMarkData aMarks;
        for (SCTAB i=nTab+1; i<nTabCount && pDoc->IsScenario(i); i++)
            pDoc->MarkScenario( i, nTab, aMarks, false, ScScenarioFlags::ShowFrame );
        ScRangeListRef xRanges = new ScRangeList;
        aMarks.FillRangeListWithMarks( xRanges.get(), false );

        bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
        long nLayoutSign = bLayoutRTL ? -1 : 1;

        for (size_t j = 0, n = xRanges->size(); j < n; ++j)
        {
            ScRange aRange = (*xRanges)[j];
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
                aStartPos.AdjustX( -nLayoutSign );
                aStartPos.AdjustY( -1 );
                aEndPos.AdjustX( -nLayoutSign );
                aEndPos.AdjustY( -1 );

                bool bTextBelow = ( aRange.aStart.Row() == 0 );

                OUString aCurrent;
                Color aColor( COL_LIGHTGRAY );
                for (SCTAB nAct=nTab+1; nAct<nTabCount && pDoc->IsScenario(nAct); nAct++)
                    if ( pDoc->IsActiveScenario(nAct) && pDoc->HasScenarioRange(nAct,aRange) )
                    {
                        OUString aDummyComment;
                        ScScenarioFlags nDummyFlags;
                        pDoc->GetName( nAct, aCurrent );
                        pDoc->GetScenarioData( nAct, aDummyComment, aColor, nDummyFlags );
                    }

                if (aCurrent.isEmpty())
                    aCurrent = ScResId( STR_EMPTYDATA );

                //! Own text "(None)" instead of "(Empty)" ???

                lcl_DrawOneFrame( pDev, tools::Rectangle( aStartPos, aEndPos ),
                                    aCurrent, aColor, bTextBelow,
                                    pViewData->GetPPTX(), pViewData->GetPPTY(), pViewData->GetZoomY(),
                                    pDoc, pViewData, bLayoutRTL );
            }
        }
    }
}

static void lcl_DrawHighlight( ScOutputData& rOutputData, const ScViewData* pViewData,
                        const std::vector<ScHighlightEntry>& rHighlightRanges )
{
    SCTAB nTab = pViewData->GetTabNo();
    for ( const auto& rHighlightRange : rHighlightRanges)
    {
        ScRange aRange = rHighlightRange.aRef;
        if ( nTab >= aRange.aStart.Tab() && nTab <= aRange.aEnd.Tab() )
        {
            rOutputData.DrawRefMark(
                                aRange.aStart.Col(), aRange.aStart.Row(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(),
                                rHighlightRange.aColor, false );
        }
    }
}

void ScGridWindow::DoInvertRect( const tools::Rectangle& rPixel )
{
    if ( rPixel == aInvertRect )
        aInvertRect = tools::Rectangle();      // Cancel
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

void ScGridWindow::Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect )
{
    ScDocument* pDoc = pViewData->GetDocument();
    if ( pDoc->IsInInterpreter() )
    {
        // Via Reschedule, interpreted cells do not trigger Invalidate again,
        // otherwise for instance an error box would never appear (bug 36381).
        // Later, through bNeedsRepaint everything is painted again.
        if ( bNeedsRepaint )
        {
            //! Merge Rectangle?
            aRepaintPixel = tools::Rectangle();            // multiple -> paint all
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

    tools::Rectangle aPixRect = LogicToPixel( rRect );

    SCCOL nX1 = pViewData->GetPosX(eHWhich);
    SCROW nY1 = pViewData->GetPosY(eVWhich);

    SCTAB nTab = pViewData->GetTabNo();

    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTY();

    tools::Rectangle aMirroredPixel = aPixRect;
    if ( pDoc->IsLayoutRTL( nTab ) )
    {
        //  mirror and swap
        long nWidth = GetSizePixel().Width();
        aMirroredPixel.SetLeft( nWidth - 1 - aPixRect.Right() );
        aMirroredPixel.SetRight( nWidth - 1 - aPixRect.Left() );
    }

    long nScrX = ScViewData::ToPixel( pDoc->GetColWidth( nX1, nTab ), nPPTX );
    while ( nScrX <= aMirroredPixel.Left() && nX1 < pDoc->MaxCol() )
    {
        ++nX1;
        nScrX += ScViewData::ToPixel( pDoc->GetColWidth( nX1, nTab ), nPPTX );
    }
    SCCOL nX2 = nX1;
    while ( nScrX <= aMirroredPixel.Right() && nX2 < pDoc->MaxCol() )
    {
        ++nX2;
        nScrX += ScViewData::ToPixel( pDoc->GetColWidth( nX2, nTab ), nPPTX );
    }

    long nScrY = 0;
    ScViewData::AddPixelsWhile( nScrY, aPixRect.Top(), nY1, pDoc->MaxRow(), nPPTY, pDoc, nTab);
    SCROW nY2 = nY1;
    if (nScrY <= aPixRect.Bottom() && nY2 < pDoc->MaxRow())
    {
        ++nY2;
        ScViewData::AddPixelsWhile( nScrY, aPixRect.Bottom(), nY2, pDoc->MaxRow(), nPPTY, pDoc, nTab);
    }

    Draw( nX1,nY1,nX2,nY2, ScUpdateMode::Marks ); // don't continue with painting

    bIsInPaint = false;
}

void ScGridWindow::Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, ScUpdateMode eMode )
{
    ScDocument& rDoc = *pViewData->GetDocument();

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

    if ( eMode != ScUpdateMode::Marks && nX2 < maVisibleRange.mnCol2)
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
        aScrPos.setX( nEndPixel + 1 );
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
        xFmtVirtDev->SetMapMode(MapMode(MapUnit::Map100thMM));
        aOutputData.SetFmtDevice( xFmtVirtDev.get() );

        bLogicText = true; // use logic MapMode
    }

    DrawContent(*this, aTabInfo, aOutputData, bLogicText);

    // If something was inverted during the Paint (selection changed from Basic Macro)
    // then this is now mixed up and has to be repainted
    OSL_ENSURE(nPaintCount, "Wrong nPaintCount");
    --nPaintCount;
    if (!nPaintCount)
        CheckNeedsRepaint();

    // Flag drawn formula cells "unchanged".
    rDoc.ResetChanged(ScRange(nX1, nY1, nTab, nX2, nY2, nTab));
    rDoc.PrepareFormulaCalc();
}

void ScGridWindow::DrawContent(OutputDevice &rDevice, const ScTableInfo& rTableInfo, ScOutputData& aOutputData,
        bool bLogicText)
{
    ScModule* pScMod = SC_MOD();
    ScDocument& rDoc = *pViewData->GetDocument();
    const ScViewOptions& rOpts = pViewData->GetOptions();
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();
    bool bNoBackgroundAndGrid = bIsTiledRendering
                                && comphelper::LibreOfficeKit::isCompatFlagSet(
                                       comphelper::LibreOfficeKit::Compat::scNoGridBackground);

    SCTAB nTab = aOutputData.nTab;
    SCCOL nX1 = aOutputData.nX1;
    SCROW nY1 = aOutputData.nY1;
    SCCOL nX2 = aOutputData.nX2;
    SCROW nY2 = aOutputData.nY2;
    long nScrX = aOutputData.nScrX;
    long nScrY = aOutputData.nScrY;

    const svtools::ColorConfig& rColorCfg = pScMod->GetColorConfig();
    Color aGridColor( rColorCfg.GetColorValue( svtools::CALCGRID, false ).nColor );
    if ( aGridColor == COL_TRANSPARENT )
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

    aOutputData.SetViewShell( pViewData->GetViewShell() );

    bool bGrid = rOpts.GetOption( VOPT_GRID ) && pViewData->GetShowGrid();
    bool bGridFirst = !rOpts.GetOption( VOPT_GRID_ONTOP );

    bool bPage = rOpts.GetOption( VOPT_PAGEBREAKS ) && !bIsTiledRendering;
    // tdf#124983, if option LibreOfficeDev Calc/View/Visual Aids/Page breaks
    // is enabled, breaks should be visible. If the document is opened the first
    // time, the breaks are not calculated yet, so this initialization is
    // done here.
    if (bPage)
    {
        std::set<SCCOL> aColBreaks;
        std::set<SCROW> aRowBreaks;
        rDoc.GetAllColBreaks(aColBreaks, nTab, true, false);
        rDoc.GetAllRowBreaks(aRowBreaks, nTab, true, false);
        if (aColBreaks.size() == 0 || aRowBreaks.size() == 0)
        {
            ScDocShell* pDocSh = pViewData->GetDocShell();
            ScPrintFunc aPrintFunc(pDocSh, pDocSh->GetPrinter(), nTab);
            aPrintFunc.UpdatePages();
        }
    }

    bool bPageMode = pViewData->IsPagebreakMode();
    if (bPageMode)                                      // after FindChanged
    {
        // SetPagebreakMode also initializes bPrinted Flags
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

    const MapMode aOriginalMode = rDevice.GetMapMode();

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
    tools::Rectangle aDrawingRectLogic;
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

    {
        // get drawing pixel rect
        tools::Rectangle aDrawingRectPixel(Point(nScrX, nScrY), Size(aOutputData.GetScrW(), aOutputData.GetScrH()));

        // correct for border (left/right)
        if(rDoc.MaxCol() == nX2)
        {
            if(bLayoutRTL)
            {
                aDrawingRectPixel.SetLeft( 0 );
            }
            else
            {
                aDrawingRectPixel.SetRight( GetOutputSizePixel().getWidth() );
            }
        }

        // correct for border (bottom)
        if(rDoc.MaxRow() == nY2)
        {
            aDrawingRectPixel.SetBottom( GetOutputSizePixel().getHeight() );
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
    if ( nX2==rDoc.MaxCol() || nY2==rDoc.MaxRow() )
    {
        // save MapMode and set to pixel
        MapMode aCurrentMapMode(pContentDev->GetMapMode());
        pContentDev->SetMapMode(MapMode(MapUnit::MapPixel));

        tools::Rectangle aPixRect( Point(), GetOutputSizePixel() );
        pContentDev->SetFillColor( rColorCfg.GetColorValue(svtools::APPBACKGROUND).nColor );
        pContentDev->SetLineColor();
        if ( nX2==rDoc.MaxCol() )
        {
            tools::Rectangle aDrawRect( aPixRect );
            if ( bLayoutRTL )
                aDrawRect.SetRight( nScrX - 1 );
            else
                aDrawRect.SetLeft( nScrX + aOutputData.GetScrW() );
            if (aDrawRect.Right() >= aDrawRect.Left())
                pContentDev->DrawRect( aDrawRect );
        }
        if ( nY2==rDoc.MaxRow() )
        {
            tools::Rectangle aDrawRect( aPixRect );
            aDrawRect.SetTop( nScrY + aOutputData.GetScrH() );
            if ( nX2==rDoc.MaxCol() )
            {
                // no double painting of the corner
                if ( bLayoutRTL )
                    aDrawRect.SetLeft( nScrX );
                else
                    aDrawRect.SetRight( nScrX + aOutputData.GetScrW() - 1 );
            }
            if (aDrawRect.Bottom() >= aDrawRect.Top())
                pContentDev->DrawRect( aDrawRect );
        }

        // restore MapMode
        pContentDev->SetMapMode(aCurrentMapMode);
    }

    if ( rDoc.HasBackgroundDraw( nTab, aDrawingRectLogic ) )
    {
        pContentDev->SetMapMode(MapMode(MapUnit::MapPixel));
        aOutputData.DrawClear();

            // drawing background

        pContentDev->SetMapMode(aDrawMode);
        DrawRedraw( aOutputData, SC_LAYER_BACK );
    }
    else
        aOutputData.SetSolidBackground(!bNoBackgroundAndGrid);

    aOutputData.DrawDocumentBackground();

    if (bGridFirst && (bGrid || bPage) && !bNoBackgroundAndGrid)
        aOutputData.DrawGrid(*pContentDev, bGrid, bPage);

    aOutputData.DrawBackground(*pContentDev);

    if (!bGridFirst && (bGrid || bPage) && !bNoBackgroundAndGrid)
        aOutputData.DrawGrid(*pContentDev, bGrid, bPage);

    pContentDev->SetMapMode(MapMode(MapUnit::MapPixel));

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
        MapMode aMap( MapUnit::MapPixel );
        Point aOrigin = aOriginalMode.GetOrigin();
        aOrigin.setX(aOrigin.getX() / TWIPS_PER_PIXEL + nScrX);
        aOrigin.setY(aOrigin.getY() / TWIPS_PER_PIXEL + nScrY);
        aMap.SetOrigin(aOrigin);
        pContentDev->SetMapMode(aMap);
    }
    else
        pContentDev->SetMapMode(MapMode(MapUnit::MapPixel));

        // Autofilter- and Pivot-Buttons

    DrawButtons(nX1, nX2, rTableInfo, pContentDev);          // Pixel

    pContentDev->SetMapMode(MapMode(MapUnit::MapPixel));

    aOutputData.DrawClipMarks();

    // In any case, Scenario / ChangeTracking must happen after DrawGrid, also for !bGridFirst

    //! test if ChangeTrack display is active
    //! Disable scenario frame via view option?

    SCTAB nTabCount = rDoc.GetTableCount();
    const std::vector<ScHighlightEntry> &rHigh = pViewData->GetView()->GetHighlightRanges();
    bool bHasScenario = ( nTab+1<nTabCount && rDoc.IsScenario(nTab+1) && !rDoc.IsScenario(nTab) );
    bool bHasChange = ( rDoc.GetChangeTrack() != nullptr );

    if ( bHasChange || bHasScenario || !rHigh.empty() )
    {
        //! Merge SetChangedClip() with DrawMarks() ?? (different MapMode!)

        if ( bHasChange )
            aOutputData.DrawChangeTrack();

        if ( bHasScenario )
            lcl_DrawScenarioFrames( pContentDev, pViewData, eWhich, nX1,nY1,nX2,nY2 );

        lcl_DrawHighlight( aOutputData, pViewData, rHigh );
    }

        // Drawing foreground

    pContentDev->SetMapMode(aDrawMode);

    // Bitmaps and buttons are in absolute pixel coordinates.
    const MapMode aOrig = pContentDev->GetMapMode();
    if (bIsTiledRendering)
    {
        Point aOrigin = aOriginalMode.GetOrigin();
        Size aPixelOffset(aOrigin.getX() / TWIPS_PER_PIXEL, aOrigin.getY() / TWIPS_PER_PIXEL);
        pContentDev->SetPixelOffset(aPixelOffset);
        comphelper::LibreOfficeKit::setLocalRendering();
    }

    DrawRedraw( aOutputData, SC_LAYER_FRONT );
    DrawRedraw( aOutputData, SC_LAYER_INTERN );
    DrawSdrGrid( aDrawingRectLogic, pContentDev );

    if (bIsTiledRendering)
    {
        pContentDev->SetPixelOffset(Size());
        pContentDev->SetMapMode(aOrig);
    }

    pContentDev->SetMapMode(MapMode(MapUnit::MapPixel));

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
        ScDocShell* pDocSh = pViewData->GetDocShell();
        ScRangeFindList* pRangeFinder = pHdl->GetRangeFindList();
        if ( pRangeFinder && !pRangeFinder->IsHidden() &&
                pRangeFinder->GetDocName() == pDocSh->GetTitle() )
        {
            sal_uInt16 nCount = static_cast<sal_uInt16>(pRangeFinder->Count());
            for (sal_uInt16 i=0; i<nCount; i++)
            {
                ScRangeFindData& rData = pRangeFinder->GetObject(i);

                ScRange aRef = rData.aRef;
                aRef.PutInOrder();
                if ( aRef.aStart.Tab() >= nTab && aRef.aEnd.Tab() <= nTab )
                    aOutputData.DrawRefMark( aRef.aStart.Col(), aRef.aStart.Row(),
                                            aRef.aEnd.Col(), aRef.aEnd.Row(),
                                            rData.nColor,
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

            if (bIsTiledRendering)
            {
                Point aOrigin = aOriginalMode.GetOrigin();
                aOrigin.setX(aOrigin.getX() / TWIPS_PER_PIXEL + aOutputData.nScrX);
                aOrigin.setY(aOrigin.getY() / TWIPS_PER_PIXEL + aOutputData.nScrY);
                const double twipFactor = 15 * 1.76388889; // 26.45833335
                aOrigin = Point(aOrigin.getX() * twipFactor,
                                aOrigin.getY() * twipFactor);
                MapMode aNew = rDevice.GetMapMode();
                aNew.SetOrigin(aOrigin);
                rDevice.SetMapMode(aNew);
            }

            SdrView* pDrawView = pTabViewShell->GetSdrView();

            if(pDrawView)
            {
                // #i74769# work with SdrPaintWindow directly
                pDrawView->EndDrawLayers(*pTargetPaintWindow, true);
            }

            pContentDev->SetMapMode(aCurrentMapMode);
        }
    }

    // paint in-place editing on other views
    if (bIsTiledRendering)
    {
        ScTabViewShell* pThisViewShell = pViewData->GetViewShell();
        SfxViewShell* pViewShell = SfxViewShell::GetFirst();

        while (pViewShell)
        {
            if (pViewShell != pThisViewShell)
            {
                ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
                if (pTabViewShell)
                {
                    ScViewData& rOtherViewData = pTabViewShell->GetViewData();
                    ScSplitPos eOtherWhich = rOtherViewData.GetEditActivePart();

                    bool bOtherEditMode = rOtherViewData.HasEditView(eOtherWhich);
                    SCCOL nCol1 = rOtherViewData.GetEditStartCol();
                    SCROW nRow1 = rOtherViewData.GetEditStartRow();
                    SCCOL nCol2 = rOtherViewData.GetEditEndCol();
                    SCROW nRow2 = rOtherViewData.GetEditEndRow();
                    bOtherEditMode = bOtherEditMode
                            && ( nCol2 >= nX1 && nCol1 <= nX2 && nRow2 >= nY1 && nRow1 <= nY2 );
                    if (bOtherEditMode && rOtherViewData.GetRefTabNo() == nTab)
                    {
                        EditView* pOtherEditView = rOtherViewData.GetEditView(eOtherWhich);
                        if (pOtherEditView)
                        {
                            long nScreenX = aOutputData.nScrX;
                            long nScreenY = aOutputData.nScrY;
                            long nScreenW = aOutputData.GetScrW();
                            long nScreenH = aOutputData.GetScrH();

                            rDevice.SetLineColor();
                            rDevice.SetFillColor(pOtherEditView->GetBackgroundColor());
                            Point aStart = rOtherViewData.GetScrPos( nCol1, nRow1, eOtherWhich );
                            Point aEnd = rOtherViewData.GetScrPos( nCol2+1, nRow2+1, eOtherWhich );

                            // don't overwrite grid
                            long nLayoutSign = bLayoutRTL ? -1 : 1;
                            aEnd.AdjustX( -(2 * nLayoutSign) );
                            aEnd.AdjustY( -2 );

                            tools::Rectangle aBackground(aStart, aEnd);

                            // Need to draw the background in absolute coords.
                            Point aOrigin = aOriginalMode.GetOrigin();
                            aOrigin.setX(aOrigin.getX() / TWIPS_PER_PIXEL + nScreenX);
                            aOrigin.setY(aOrigin.getY() / TWIPS_PER_PIXEL + nScreenY);
                            aBackground += aOrigin;
                            rDevice.SetMapMode(aDrawMode);

                            static const double twipFactor = 15 * 1.76388889; // 26.45833335
                            aOrigin = Point(aOrigin.getX() * twipFactor,
                                            aOrigin.getY() * twipFactor);
                            MapMode aNew = rDevice.GetMapMode();
                            aNew.SetOrigin(aOrigin);
                            rDevice.SetMapMode(aNew);

                            // paint the background
                            rDevice.DrawRect(rDevice.PixelToLogic(aBackground));

                            tools::Rectangle aEditRect(Point(nScreenX, nScreenY), Size(nScreenW, nScreenH));
                            pOtherEditView->Paint(rDevice.PixelToLogic(aEditRect), &rDevice);
                            rDevice.SetMapMode(MapMode(MapUnit::MapPixel));
                        }
                    }
                }
            }

            pViewShell = SfxViewShell::GetNext(*pViewShell);
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
        aEnd.AdjustX( -(2 * nLayoutSign) );
        aEnd.AdjustY( -2 );

        // toggle the cursor off if its on to ensure the cursor invert
        // background logic remains valid after the background is cleared on
        // the next cursor flash
        vcl::Cursor* pCrsr = pEditView->GetCursor();
        const bool bVisCursor = pCrsr && pCrsr->IsVisible();
        if (bVisCursor)
            pCrsr->Hide();

        // set the correct mapmode
        tools::Rectangle aBackground(aStart, aEnd);
        if (bIsTiledRendering)
        {
            // Need to draw the background in absolute coords.
            Point aOrigin = aOriginalMode.GetOrigin();
            aOrigin.setX(aOrigin.getX() / TWIPS_PER_PIXEL + nScrX);
            aOrigin.setY(aOrigin.getY() / TWIPS_PER_PIXEL + nScrY);
            aBackground += aOrigin;
            rDevice.SetMapMode(aDrawMode);
        }
        else
            rDevice.SetMapMode(pViewData->GetLogicMode());

        if (bIsTiledRendering)
        {
            Point aOrigin = aOriginalMode.GetOrigin();
            aOrigin.setX(aOrigin.getX() / TWIPS_PER_PIXEL + nScrX);
            aOrigin.setY(aOrigin.getY() / TWIPS_PER_PIXEL + nScrY);
            static const double twipFactor = 15 * 1.76388889; // 26.45833335
            aOrigin = Point(aOrigin.getX() * twipFactor,
                            aOrigin.getY() * twipFactor);
            MapMode aNew = rDevice.GetMapMode();
            aNew.SetOrigin(aOrigin);
            rDevice.SetMapMode(aNew);
        }

        // paint the background
        tools::Rectangle aLogicRect(rDevice.PixelToLogic(aBackground));
        //tdf#100925, rhbz#1283420, Draw some text here, to get
        //X11CairoTextRender::getCairoContext called, so that the forced read
        //from the underlying X Drawable gets it to sync.
        rDevice.DrawText(aLogicRect.BottomLeft(), " ");
        rDevice.DrawRect(aLogicRect);

        // paint the editeng text
        tools::Rectangle aEditRect(Point(nScrX, nScrY), Size(aOutputData.GetScrW(), aOutputData.GetScrH()));
        pEditView->Paint(rDevice.PixelToLogic(aEditRect), &rDevice);
        rDevice.SetMapMode(MapMode(MapUnit::MapPixel));

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

namespace
{
    template<typename IndexType>
    void lcl_getBoundingRowColumnforTile(ScViewData* pViewData,
            long nTileStartPosPx, long nTileEndPosPx,
            sal_Int32& nTopLeftTileOffset, sal_Int32& nTopLeftTileOrigin,
            sal_Int32& nTopLeftTileIndex, sal_Int32& nBottomRightTileIndex)
    {
        const bool bColumnHeader = std::is_same<IndexType, SCCOL>::value;

        SCTAB nTab = pViewData->GetTabNo();
        ScDocument* pDoc = pViewData->GetDocument();

        IndexType nStartIndex = -1;
        IndexType nEndIndex = -1;
        long nStartPosPx = 0;
        long nEndPosPx = 0;

        ScPositionHelper& rPositionHelper =
                bColumnHeader ? pViewData->GetLOKWidthHelper() : pViewData->GetLOKHeightHelper();
        const auto& rStartNearest = rPositionHelper.getNearestByPosition(nTileStartPosPx);
        const auto& rEndNearest = rPositionHelper.getNearestByPosition(nTileEndPosPx);

        ScBoundsProvider aBoundsProvider(pDoc, nTab, bColumnHeader);
        aBoundsProvider.Compute(rStartNearest, rEndNearest, nTileStartPosPx, nTileEndPosPx);
        aBoundsProvider.GetStartIndexAndPosition(nStartIndex, nStartPosPx); ++nStartIndex;
        aBoundsProvider.GetEndIndexAndPosition(nEndIndex, nEndPosPx);

        nTopLeftTileOffset = nTileStartPosPx - nStartPosPx;
        nTopLeftTileOrigin = nStartPosPx;
        nTopLeftTileIndex = nStartIndex;
        nBottomRightTileIndex = nEndIndex;
    }
} // anonymous namespace

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
    //
    // But until that happens, we actually draw everything at 100%, and only
    // set cairo's or CoreGraphic's scale factor accordingly, so that everything
    // is painted  bigger or smaller. This is different to what Calc's internal
    // scaling would do - because that one is trying to fit the lines between
    // cells to integer multiples of pixels.
    //
    // See also desktop/source/lib/init.cxx for details, where we have to set
    // the stuff accordingly for the VirtualDevice creation.

    // page break zoom, and aLogicMode in ScViewData - hardcode that to what
    // we mean as 100% (256px tiles meaning 3840 twips)
    Fraction aFracX(long(256 * TWIPS_PER_PIXEL), 3840);
    Fraction aFracY(long(256 * TWIPS_PER_PIXEL), 3840);
    pViewData->SetZoom(aFracX, aFracY, true);

    // Cairo or CoreGraphics scales for us, we have to compensate for that,
    // otherwise we are painting too far away
    const double fDPIScale = comphelper::LibreOfficeKit::getDPIScale();

    const double fTilePosXPixel = static_cast<double>(nTilePosX) * nOutputWidth / (nTileWidth * fDPIScale);
    const double fTilePosYPixel = static_cast<double>(nTilePosY) * nOutputHeight / (nTileHeight * fDPIScale);
    const double fTileBottomPixel = static_cast<double>(nTilePosY + nTileHeight) * nOutputHeight / (nTileHeight * fDPIScale);
    const double fTileRightPixel = static_cast<double>(nTilePosX + nTileWidth) * nOutputWidth / (nTileWidth * fDPIScale);

    SCTAB nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();

    const double fPPTX = pViewData->GetPPTX();
    const double fPPTY = pViewData->GetPPTY();

    // find approximate col/row offsets of nearby.
    sal_Int32 nTopLeftTileRowOffset = 0;
    sal_Int32 nTopLeftTileColOffset = 0;
    sal_Int32 nTopLeftTileRowOrigin = 0;
    sal_Int32 nTopLeftTileColOrigin = 0;

    sal_Int32 nTopLeftTileRow = 0;
    sal_Int32 nTopLeftTileCol = 0;
    sal_Int32 nBottomRightTileRow = 0;
    sal_Int32 nBottomRightTileCol = 0;

    lcl_getBoundingRowColumnforTile<SCROW>(pViewData,
            fTilePosYPixel, fTileBottomPixel,
            nTopLeftTileRowOffset, nTopLeftTileRowOrigin,
            nTopLeftTileRow, nBottomRightTileRow);

    lcl_getBoundingRowColumnforTile<SCCOL>(pViewData,
            fTilePosXPixel, fTileRightPixel,
            nTopLeftTileColOffset, nTopLeftTileColOrigin,
            nTopLeftTileCol, nBottomRightTileCol);

    // Enlarge
    nBottomRightTileCol++;
    nBottomRightTileRow++;

    if (nBottomRightTileCol > pDoc->MaxCol())
        nBottomRightTileCol = pDoc->MaxCol();

    if (nBottomRightTileRow > MAXTILEDROW)
        nBottomRightTileRow = MAXTILEDROW;

    // size of the document including drawings, charts, etc.
    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    pDoc->GetTiledRenderingArea(nTab, nEndCol, nEndRow);

    if (nEndCol < nBottomRightTileCol)
        nEndCol = nBottomRightTileCol;

    if (nEndRow < nBottomRightTileRow)
        nEndRow = nBottomRightTileRow;

    nTopLeftTileCol = std::max<sal_Int32>(nTopLeftTileCol, 0);
    nTopLeftTileRow = std::max<sal_Int32>(nTopLeftTileRow, 0);
    nTopLeftTileColOrigin = nTopLeftTileColOrigin * TWIPS_PER_PIXEL;
    nTopLeftTileRowOrigin = nTopLeftTileRowOrigin * TWIPS_PER_PIXEL;

    // Checkout -> 'rDoc.ExtendMerge' ... if we miss merged cells.

    // Origin must be the offset of the first col and row
    // containing our top-left pixel.
    const MapMode aOriginalMode = rDevice.GetMapMode();
    MapMode aAbsMode = aOriginalMode;
    const Point aOrigin(-nTopLeftTileColOrigin, -nTopLeftTileRowOrigin);
    aAbsMode.SetOrigin(aOrigin);
    rDevice.SetMapMode(aAbsMode);

    ScTableInfo aTabInfo(nEndRow + 3);
    pDoc->FillInfo(aTabInfo, nTopLeftTileCol, nTopLeftTileRow,
                   nBottomRightTileCol, nBottomRightTileRow,
                   nTab, fPPTX, fPPTY, false, false);

// FIXME: is this called some
//        Point aScrPos = pViewData->GetScrPos( nX1, nY1, eWhich );

    ScOutputData aOutputData(&rDevice, OUTTYPE_WINDOW, aTabInfo, pDoc, nTab,
                             -nTopLeftTileColOffset,
                             -nTopLeftTileRowOffset,
                             nTopLeftTileCol, nTopLeftTileRow,
                             nBottomRightTileCol, nBottomRightTileRow,
                             fPPTX, fPPTY, nullptr, nullptr);

    // setup the SdrPage so that drawinglayer works correctly
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (pModel)
    {
        mpLOKDrawView.reset(
            new FmFormView(
                *pModel,
                &rDevice));
        mpLOKDrawView->ShowSdrPage(mpLOKDrawView->GetModel()->GetPage(nTab));
        aOutputData.SetDrawView(mpLOKDrawView.get());
        aOutputData.SetSpellCheckContext(mpSpellCheckCxt.get());
    }

    // draw the content
    DrawContent(rDevice, aTabInfo, aOutputData, true);

    rDevice.SetMapMode(aOriginalMode);

    // Flag drawn formula cells "unchanged".
    pDoc->ResetChanged(ScRange(nTopLeftTileCol, nTopLeftTileRow, nTab, nBottomRightTileCol, nBottomRightTileRow, nTab));
    pDoc->PrepareFormulaCalc();
}

void ScGridWindow::LogicInvalidate(const tools::Rectangle* pRectangle)
{
    OString sRectangle;
    if (!pRectangle)
        sRectangle = "EMPTY";
    else
    {
        tools::Rectangle aRectangle(*pRectangle);
        // When dragging shapes the map mode is disabled.
        if (IsMapModeEnabled())
        {
            if (GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                aRectangle = OutputDevice::LogicToLogic(aRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
        }
        else
            aRectangle = PixelToLogic(aRectangle, MapMode(MapUnit::MapTwip));
        sRectangle = aRectangle.toString();
    }

    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    SfxLokHelper::notifyInvalidation(pViewShell, sRectangle);
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
    SCCOL nNewPosX;
    SCROW nNewPosY;
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
        aRepaintPixel = tools::Rectangle();

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

        OUString aPageStr = ScResId( STR_PGNUM );
        if ( nPageScript == SvtScriptType::NONE )
        {
            //  get script type of translated "Page" string only once
            nPageScript = pDoc->GetStringScriptType( aPageStr );
            if (nPageScript == SvtScriptType::NONE)
                nPageScript = ScGlobal::GetDefaultScriptType();
        }

        vcl::Font aFont;
        std::unique_ptr<ScEditEngineDefaulter> pEditEng;
        const ScPatternAttr& rDefPattern = pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN);
        if ( nPageScript == SvtScriptType::LATIN )
        {
            //  use single font and call DrawText directly
            rDefPattern.GetFont( aFont, SC_AUTOCOL_BLACK );
            aFont.SetColor( COL_LIGHTGRAY );
            //  font size is set as needed
        }
        else
        {
            //  use EditEngine to draw mixed-script string
            pEditEng.reset(new ScEditEngineDefaulter( EditEngine::CreatePool(), true ));
            pEditEng->SetRefMapMode(rRenderContext.GetMapMode());
            auto pEditDefaults = std::make_unique<SfxItemSet>( pEditEng->GetEmptyItemSet() );
            rDefPattern.FillEditItemSet( pEditDefaults.get() );
            pEditDefaults->Put( SvxColorItem( COL_LIGHTGRAY, EE_CHAR_COLOR ) );
            pEditEng->SetDefaults( std::move(pEditDefaults) );
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
                aStart.AdjustX( -2 );
                aStart.AdjustY( -2 );

                // Prevent overflows:
                if ( aStart.X() < -10 ) aStart.setX( -10 );
                if ( aStart.Y() < -10 ) aStart.setY( -10 );
                if ( aEnd.X() > aWinSize.Width() + 10 )
                    aEnd.setX( aWinSize.Width() + 10 );
                if ( aEnd.Y() > aWinSize.Height() + 10 )
                    aEnd.setY( aWinSize.Height() + 10 );

                rRenderContext.DrawRect( tools::Rectangle( aStart, Point(aEnd.X(),aStart.Y()+2) ) );
                rRenderContext.DrawRect( tools::Rectangle( aStart, Point(aStart.X()+2,aEnd.Y()) ) );
                rRenderContext.DrawRect( tools::Rectangle( Point(aStart.X(),aEnd.Y()-2), aEnd ) );
                rRenderContext.DrawRect( tools::Rectangle( Point(aEnd.X()-2,aStart.Y()), aEnd ) );

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
                        if (pDoc->HasColBreak(nBreak, nTab) & ScBreakType::Manual)
                            rRenderContext.SetFillColor( aManual );
                        else
                            rRenderContext.SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        nBreak, aRange.aStart.Row(), eWhich, true );
                        rRenderContext.DrawRect( tools::Rectangle( aBreak.X()-1, aStart.Y(), aBreak.X(), aEnd.Y() ) );
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
                        if (pDoc->HasRowBreak(nBreak, nTab) & ScBreakType::Manual)
                            rRenderContext.SetFillColor( aManual );
                        else
                            rRenderContext.SetFillColor( aAutomatic );
                        Point aBreak = pViewData->GetScrPos(
                                        aRange.aStart.Col(), nBreak, eWhich, true );
                        rRenderContext.DrawRect( tools::Rectangle( aStart.X(), aBreak.Y()-1, aEnd.X(), aBreak.Y() ) );
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
                                    nPageNo += static_cast<long>(nColPos)*nRowBreaks+nRowPos;
                                else
                                    nPageNo += static_cast<long>(nRowPos)*nColBreaks+nColPos;

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
                                    aFont.SetFontSize( Size( 0,100 ) );
                                    rRenderContext.SetFont( aFont );
                                    Size aSize100(rRenderContext.GetTextWidth( aThisPageStr ), rRenderContext.GetTextHeight() );

                                    //  40% of width or 60% of height
                                    long nSizeX = 40 * ( aPageEnd.X() - aPageStart.X() ) / aSize100.Width();
                                    long nSizeY = 60 * ( aPageEnd.Y() - aPageStart.Y() ) / aSize100.Height();
                                    aFont.SetFontSize( Size( 0,std::min(nSizeX,nSizeY) ) );
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
    ScDPFieldButton aCellBtn(pContentDev, &GetSettings().GetStyleSettings(), &pViewData->GetZoomY(), pDoc);

    SCCOL nCol;
    SCROW nRow;
    SCSIZE nArrY;
    SCSIZE nQuery;
    SCTAB           nTab = pViewData->GetTabNo();
    ScDBData*       pDBData = nullptr;
    std::unique_ptr<ScQueryParam> pQueryParam;

    RowInfo*        pRowInfo = rTabInfo.mpRowInfo.get();
    sal_uInt16      nArrCount = rTabInfo.mnArrCount;

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

        if ( !comphelper::LibreOfficeKit::isActive() && bListValButton && pRowInfo[nArrY].nRowNo == aListValPos.Row() && pRowInfo[nArrY].bChanged )
        {
            tools::Rectangle aRect = GetListValButtonRect( aListValPos );
            aComboButton.SetPosPixel( aRect.TopLeft() );
            aComboButton.SetSizePixel( aRect.GetSize() );
            pContentDev->SetClipRegion(vcl::Region(aRect));
            aComboButton.Draw();
            pContentDev->SetClipRegion();           // always called from Draw() without clip region
            aComboButton.SetPosPixel( aOldPos );    // restore old state
            aComboButton.SetSizePixel( aOldSize );  // for MouseUp/Down (AutoFilter)
        }
    }

    pQueryParam.reset();
    aComboButton.SetOutputDevice( this );
}

tools::Rectangle ScGridWindow::GetListValButtonRect( const ScAddress& rButtonPos )
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
    const ScMergeAttr* pMerge = pDoc->GetAttr( nCol,nRow,nTab, ATTR_MERGE );
    if ( pMerge->GetColMerge() > 1 )
        nNextCol = nCol + pMerge->GetColMerge();    // next cell after the merged area
    while ( nNextCol <= pDoc->MaxCol() && pDoc->ColHidden(nNextCol, nTab) )
        ++nNextCol;
    bool bNextCell = ( nNextCol <= pDoc->MaxCol() );
    if ( bNextCell )
        nAvailable = ScViewData::ToPixel( pDoc->GetColWidth( nNextCol, nTab ), pViewData->GetPPTX() );

    if ( nAvailable < aBtnSize.Width() )
        aBtnSize.setWidth( nAvailable );
    if ( nCellSizeY < aBtnSize.Height() )
        aBtnSize.setHeight( nCellSizeY );

    Point aPos = pViewData->GetScrPos( nCol, nRow, eWhich, true );
    aPos.AdjustX(nCellSizeX * nLayoutSign );               // start of next cell
    if (!bNextCell)
        aPos.AdjustX( -(aBtnSize.Width() * nLayoutSign) );     // right edge of cell if next cell not available
    aPos.AdjustY(nCellSizeY - aBtnSize.Height() );
    // X remains at the left edge

    if ( bLayoutRTL )
        aPos.AdjustX( -(aBtnSize.Width()-1) );     // align right edge of button with cell border

    return tools::Rectangle( aPos, aBtnSize );
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

void ScGridWindow::GetSelectionRects( ::std::vector< tools::Rectangle >& rPixelRects )
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

    SCCOL nTestX2 = nX2;
    SCROW nTestY2 = nY2;

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
        if (nXRight > pDoc->MaxCol())
            nXRight = pDoc->MaxCol();

        SCROW nYBottom = nPosY + pViewData->VisibleCellsY(eVWhich);
        if (nYBottom > pDoc->MaxRow())
            nYBottom = pDoc->MaxRow();

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
            if (nY==nY2)
                bDoRow = true;              // last cell of the block
        }

        if ( bDoRow )
        {
            SCCOL nLoopEndX = nX2;
            if (nX2 < nX1)                      // the rest of the merge
            {
                SCCOL nStartX = nX1;
                while ( pDoc->GetAttr(nStartX,nY,nTab,ATTR_MERGE_FLAG)->IsHorOverlapped() )
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

                    SCROW nThisY = nY;
                    const ScPatternAttr* pPattern = pDoc->GetPattern( nX, nY, nTab );
                    const ScMergeFlagAttr* pMergeFlag = &pPattern->GetItem(ATTR_MERGE_FLAG);
                    if ( pMergeFlag->IsVerOverlapped() && ( bDoHidden || bFirstRow ) )
                    {
                        while ( pMergeFlag->IsVerOverlapped() && nThisY > 0 &&
                                (pDoc->RowHidden(nThisY-1, nTab) || bFirstRow) )
                        {
                            --nThisY;
                            pPattern = pDoc->GetPattern( nX, nThisY, nTab );
                            pMergeFlag = &pPattern->GetItem(ATTR_MERGE_FLAG);
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
                            pMergeFlag = &pPattern->GetItem(ATTR_MERGE_FLAG);
                        }
                    }

                    if ( aMultiMark.IsCellMarked( nThisX, nThisY, true ) )
                    {
                        if ( !pMergeFlag->IsOverlapped() )
                        {
                            const ScMergeAttr* pMerge = &pPattern->GetItem(ATTR_MERGE);
                            if (pMerge->GetColMerge() > 0 || pMerge->GetRowMerge() > 0)
                            {
                                Point aEndPos = pViewData->GetScrPos(
                                        nThisX + pMerge->GetColMerge(),
                                        nThisY + pMerge->GetRowMerge(), eWhich );
                                if ( aEndPos.X() * nLayoutSign > nScrX * nLayoutSign && aEndPos.Y() > nScrY )
                                {
                                    aInvert.AddRect( tools::Rectangle( nScrX,nScrY,
                                                aEndPos.X()-nLayoutSign,aEndPos.Y()-1 ) );
                                }
                            }
                            else if ( nEndX * nLayoutSign >= nScrX * nLayoutSign && nEndY >= nScrY )
                            {
                                aInvert.AddRect( tools::Rectangle( nScrX,nScrY,nEndX,nEndY ) );
                            }
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
