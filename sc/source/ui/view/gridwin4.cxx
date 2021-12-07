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
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
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
                        ScDocument& rDoc, ScViewData& rButtonViewData, bool bLayoutRTL )
{
    // rButtonViewData is only used to set the button size,

    tools::Rectangle aInner = rInnerPixel;
    if ( bLayoutRTL )
    {
        aInner.SetLeft( rInnerPixel.Right() );
        aInner.SetRight( rInnerPixel.Left() );
    }

    tools::Rectangle aVisible( Point(0,0), pDev->GetOutputSizePixel() );
    lcl_LimitRect( aInner, aVisible );

    tools::Rectangle aOuter = aInner;
    tools::Long nHor = static_cast<tools::Long>( SC_SCENARIO_HSPACE * nPPTX );
    tools::Long nVer = static_cast<tools::Long>( SC_SCENARIO_VSPACE * nPPTY );
    aOuter.AdjustLeft( -nHor );
    aOuter.AdjustRight(nHor );
    aOuter.AdjustTop( -nVer );
    aOuter.AdjustBottom(nVer );

    //  use ScPatternAttr::GetFont only for font size
    vcl::Font aAttrFont;
    rDoc.GetPool()->GetDefaultItem(ATTR_PATTERN).
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

    tools::Long nButtonY = bTextBelow ? aInner.Bottom() : aOuter.Top();

    ScDDComboBoxButton aComboButton(pDev);
    aComboButton.SetOptSizePixel();
    tools::Long nBWidth  = tools::Long(aComboButton.GetSizePixel().Width() * rZoomY);
    tools::Long nBHeight = nVer + aTextSize.Height() + 1;
    Size aButSize( nBWidth, nBHeight );
    tools::Long nButtonPos = bLayoutRTL ? aOuter.Left() : aOuter.Right()-nBWidth+1;
    aComboButton.Draw( Point(nButtonPos, nButtonY), aButSize );
    rButtonViewData.SetScenButSize( aButSize );

    tools::Long nTextStart = bLayoutRTL ? aInner.Right() - aTextSize.Width() + 1 : aInner.Left();

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
        tools::Long nClipStartX = bLayoutRTL ? aOuter.Left() + nBWidth : aInner.Left();
        tools::Long nClipEndX = bLayoutRTL ? aInner.Right() : aOuter.Right() - nBWidth;
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

static void lcl_DrawScenarioFrames( OutputDevice* pDev, ScViewData& rViewData, ScSplitPos eWhich,
                            SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 )
{
    ScDocument& rDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();
    SCTAB nTabCount = rDoc.GetTableCount();
    if ( nTab+1 >= nTabCount || !rDoc.IsScenario(nTab+1) || rDoc.IsScenario(nTab) )
        return;

    if ( nX1 > 0 ) --nX1;
    if ( nY1>=2 ) nY1 -= 2;             // Hack: Header row affects two cells
    else if ( nY1 > 0 ) --nY1;
    if ( nX2 < rDoc.MaxCol() ) ++nX2;
    if ( nY2 < rDoc.MaxRow()-1 ) nY2 += 2;     // Hack: Header row affects two cells
    else if ( nY2 < rDoc.MaxRow() ) ++nY2;
    ScRange aViewRange( nX1,nY1,nTab, nX2,nY2,nTab );

    //! cache the ranges in table!!!!

    ScMarkData aMarks(rDoc.GetSheetLimits());
    for (SCTAB i=nTab+1; i<nTabCount && rDoc.IsScenario(i); i++)
        rDoc.MarkScenario( i, nTab, aMarks, false, ScScenarioFlags::ShowFrame );
    ScRangeListRef xRanges = new ScRangeList;
    aMarks.FillRangeListWithMarks( xRanges.get(), false );

    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    for (size_t j = 0, n = xRanges->size(); j < n; ++j)
    {
        ScRange aRange = (*xRanges)[j];
        // Always extend scenario frame to merged cells where no new non-covered cells
        // are framed
        rDoc.ExtendTotalMerge( aRange );

        //! -> Extend repaint when merging !!!

        if ( aRange.Intersects( aViewRange ) )          //! Space for Text/Button?
        {
            Point aStartPos = rViewData.GetScrPos(
                                aRange.aStart.Col(), aRange.aStart.Row(), eWhich, true );
            Point aEndPos = rViewData.GetScrPos(
                                aRange.aEnd.Col()+1, aRange.aEnd.Row()+1, eWhich, true );
            //  on the grid:
            aStartPos.AdjustX( -nLayoutSign );
            aStartPos.AdjustY( -1 );
            aEndPos.AdjustX( -nLayoutSign );
            aEndPos.AdjustY( -1 );

            bool bTextBelow = ( aRange.aStart.Row() == 0 );

            OUString aCurrent;
            Color aColor( COL_LIGHTGRAY );
            for (SCTAB nAct=nTab+1; nAct<nTabCount && rDoc.IsScenario(nAct); nAct++)
                if ( rDoc.IsActiveScenario(nAct) && rDoc.HasScenarioRange(nAct,aRange) )
                {
                    OUString aDummyComment;
                    ScScenarioFlags nDummyFlags;
                    rDoc.GetName( nAct, aCurrent );
                    rDoc.GetScenarioData( nAct, aDummyComment, aColor, nDummyFlags );
                }

            if (aCurrent.isEmpty())
                aCurrent = ScResId( STR_EMPTYDATA );

            //! Own text "(None)" instead of "(Empty)" ???

            lcl_DrawOneFrame( pDev, tools::Rectangle( aStartPos, aEndPos ),
                                aCurrent, aColor, bTextBelow,
                                rViewData.GetPPTX(), rViewData.GetPPTY(), rViewData.GetZoomY(),
                                rDoc, rViewData, bLayoutRTL );
        }
    }
}

static void lcl_DrawHighlight( ScOutputData& rOutputData, const ScViewData& rViewData,
                        const std::vector<ScHighlightEntry>& rHighlightRanges )
{
    SCTAB nTab = rViewData.GetTabNo();
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

// Calculates top-left offset to be applied based on margins and indent.
static void lcl_GetEditAreaTLOffset(tools::Long& nOffsetX, tools::Long& nOffsetY, const ScAddress& rAddr,
                                    const ScViewData& rViewData, ScDocument& rDoc)
{
    tools::Long nLeftMargin = 0;
    tools::Long nTopMargin = 0;
    tools::Long nIndent = 0;
    tools::Long nDummy = 0;
    ScEditUtil aEUtil(&rDoc, rAddr.Col(), rAddr.Row(), rAddr.Tab(),
        Point(0, 0), nullptr, rViewData.GetPPTX(),
        rViewData.GetPPTY(), Fraction(1.0), Fraction(1.0),
        false /* bPrintTwips */);
    const ScPatternAttr* pPattern = rDoc.GetPattern(rAddr);
    nIndent = aEUtil.GetIndent(pPattern);
    aEUtil.GetMargins(pPattern, nLeftMargin, nTopMargin, nDummy, nDummy);
    nOffsetX = nIndent + nLeftMargin;
    nOffsetY = nTopMargin;
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
    ScTabViewShell* pTabViewShell = mrViewData.GetViewShell();

    if(pTabViewShell)
    {
        SdrView* pDrawView = pTabViewShell->GetScDrawView();

        if (pDrawView)
        {
            pDrawView->PrePaint();
        }
    }
}

bool ScGridWindow::NeedLOKCursorInvalidation(const tools::Rectangle& rCursorRect,
        const Fraction aScaleX, const Fraction aScaleY)
{
    // Don't see the need for a map as there will be only a few zoom levels
    // and as of now X and Y zooms in online are the same.
    for (auto& rEntry : maLOKLastCursor)
    {
        if (aScaleX == rEntry.aScaleX && aScaleY == rEntry.aScaleY)
        {
            if (rCursorRect == rEntry.aRect)
                return false; // No change

            // Update and allow invalidate.
            rEntry.aRect = rCursorRect;
            return true;
        }
    }

    maLOKLastCursor.push_back(LOKCursorEntry{aScaleX, aScaleY, rCursorRect});
    return true;
}

void ScGridWindow::InvalidateLOKViewCursor(const tools::Rectangle& rCursorRect,
        const Fraction aScaleX, const Fraction aScaleY)
{
    if (!NeedLOKCursorInvalidation(rCursorRect, aScaleX, aScaleY))
        return;

    ScTabViewShell* pThisViewShell = mrViewData.GetViewShell();
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();

    while (pViewShell)
    {
        if (pViewShell != pThisViewShell && pViewShell->GetDocId() == pThisViewShell->GetDocId())
        {
            ScTabViewShell* pOtherViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
            if (pOtherViewShell)
            {
                ScViewData& rOtherViewData = pOtherViewShell->GetViewData();
                Fraction aZoomX = rOtherViewData.GetZoomX();
                Fraction aZoomY = rOtherViewData.GetZoomY();
                if (aZoomX == aScaleX && aZoomY == aScaleY)
                {
                    SfxLokHelper::notifyOtherView(pThisViewShell, pOtherViewShell,
                            LOK_CALLBACK_INVALIDATE_VIEW_CURSOR, "rectangle", rCursorRect.toString());
                }
            }
        }

        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void ScGridWindow::Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect )
{
    ScDocument& rDoc = mrViewData.GetDocument();
    if ( rDoc.IsInInterpreter() )
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

    SCCOL nX1 = mrViewData.GetPosX(eHWhich);
    SCROW nY1 = mrViewData.GetPosY(eVWhich);

    SCTAB nTab = mrViewData.GetTabNo();

    double nPPTX = mrViewData.GetPPTX();
    double nPPTY = mrViewData.GetPPTY();

    tools::Rectangle aMirroredPixel = aPixRect;
    if ( rDoc.IsLayoutRTL( nTab ) )
    {
        //  mirror and swap
        tools::Long nWidth = GetSizePixel().Width();
        aMirroredPixel.SetLeft( nWidth - 1 - aPixRect.Right() );
        aMirroredPixel.SetRight( nWidth - 1 - aPixRect.Left() );
    }

    tools::Long nScrX = ScViewData::ToPixel( rDoc.GetColWidth( nX1, nTab ), nPPTX );
    while ( nScrX <= aMirroredPixel.Left() && nX1 < rDoc.MaxCol() )
    {
        ++nX1;
        nScrX += ScViewData::ToPixel( rDoc.GetColWidth( nX1, nTab ), nPPTX );
    }
    SCCOL nX2 = nX1;
    while ( nScrX <= aMirroredPixel.Right() && nX2 < rDoc.MaxCol() )
    {
        ++nX2;
        nScrX += ScViewData::ToPixel( rDoc.GetColWidth( nX2, nTab ), nPPTX );
    }

    tools::Long nScrY = 0;
    ScViewData::AddPixelsWhile( nScrY, aPixRect.Top(), nY1, rDoc.MaxRow(), nPPTY, &rDoc, nTab);
    SCROW nY2 = nY1;
    if (nScrY <= aPixRect.Bottom() && nY2 < rDoc.MaxRow())
    {
        ++nY2;
        ScViewData::AddPixelsWhile( nScrY, aPixRect.Bottom(), nY2, rDoc.MaxRow(), nPPTY, &rDoc, nTab);
    }

    Draw( nX1,nY1,nX2,nY2, ScUpdateMode::Marks ); // don't continue with painting

    bIsInPaint = false;
}

void ScGridWindow::Draw( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, ScUpdateMode eMode )
{
    ScDocument& rDoc = mrViewData.GetDocument();

    // let's ignore the normal Draw() attempts when doing the tiled rendering,
    // all the rendering should go through PaintTile() in that case.
    // TODO revisit if we can actually turn this into an assert(), and clean
    // up the callers
    if (comphelper::LibreOfficeKit::isActive())
        return;

    ScModule* pScMod = SC_MOD();
    bool bTextWysiwyg = pScMod->GetInputOptions().GetTextWysiwyg();

    if (mrViewData.IsMinimized())
        return;

    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    OSL_ENSURE( rDoc.ValidCol(nX2) && rDoc.ValidRow(nY2), "GridWin Draw area too big" );

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

    SCTAB nTab = mrViewData.GetTabNo();
    rDoc.ExtendHidden( nX1, nY1, nX2, nY2, nTab );

    Point aScrPos = mrViewData.GetScrPos( nX1, nY1, eWhich );
    tools::Long nMirrorWidth = GetSizePixel().Width();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
    if ( bLayoutRTL )
    {
        tools::Long nEndPixel = mrViewData.GetScrPos( nX2+1, maVisibleRange.mnRow1, eWhich ).X();
        nMirrorWidth = aScrPos.X() - nEndPixel;
        aScrPos.setX( nEndPixel + 1 );
    }

    tools::Long nScrX = aScrPos.X();
    tools::Long nScrY = aScrPos.Y();

    SCCOL nCurX = mrViewData.GetCurX();
    SCROW nCurY = mrViewData.GetCurY();
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

    double nPPTX = mrViewData.GetPPTX();
    double nPPTY = mrViewData.GetPPTY();

    const ScViewOptions& rOpts = mrViewData.GetOptions();

    // data block

    ScTableInfo aTabInfo;
    rDoc.FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nTab,
                   nPPTX, nPPTY, false, rOpts.GetOption(VOPT_FORMULAS),
                   &mrViewData.GetMarkData() );

    Fraction aZoomX = mrViewData.GetZoomX();
    Fraction aZoomY = mrViewData.GetZoomY();
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
        pFmtDev->SetMapMode( mrViewData.GetLogicMode(eWhich) );
        aOutputData.SetFmtDevice( pFmtDev );
    }
    else if ( aZoomX != aZoomY && mrViewData.IsOle() )
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

namespace {

class SuppressEditViewMessagesGuard
{
public:
    SuppressEditViewMessagesGuard(EditView& rEditView) :
        mrEditView(rEditView),
        mbOrigSuppressFlag(rEditView.IsSuppressLOKMessages())
    {
        if (!mbOrigSuppressFlag)
            mrEditView.SuppressLOKMessages(true);
    }

    ~SuppressEditViewMessagesGuard()
    {
        if (mrEditView.IsSuppressLOKMessages() != mbOrigSuppressFlag)
            mrEditView.SuppressLOKMessages(mbOrigSuppressFlag);
    }

private:
    EditView& mrEditView;
    const bool mbOrigSuppressFlag;
};

}

void ScGridWindow::DrawContent(OutputDevice &rDevice, const ScTableInfo& rTableInfo, ScOutputData& aOutputData,
        bool bLogicText)
{
    ScModule* pScMod = SC_MOD();
    ScDocument& rDoc = mrViewData.GetDocument();
    const ScViewOptions& rOpts = mrViewData.GetOptions();
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();
    bool bNoBackgroundAndGrid = bIsTiledRendering
                                && comphelper::LibreOfficeKit::isCompatFlagSet(
                                       comphelper::LibreOfficeKit::Compat::scNoGridBackground);

    SCTAB nTab = aOutputData.nTab;
    SCCOL nX1 = aOutputData.nX1;
    SCROW nY1 = aOutputData.nY1;
    SCCOL nX2 = aOutputData.nX2;
    SCROW nY2 = aOutputData.nY2;
    tools::Long nScrX = aOutputData.nScrX;
    tools::Long nScrY = aOutputData.nScrY;

    const svtools::ColorConfig& rColorCfg = pScMod->GetColorConfig();
    Color aGridColor( rColorCfg.GetColorValue( svtools::CALCGRID, false ).nColor );
    if ( aGridColor == COL_TRANSPARENT )
    {
        //  use view options' grid color only if color config has "automatic" color
        aGridColor = rOpts.GetGridColor();
    }

    aOutputData.SetSyntaxMode       ( mrViewData.IsSyntaxMode() );
    aOutputData.SetGridColor        ( aGridColor );
    aOutputData.SetShowNullValues   ( rOpts.GetOption( VOPT_NULLVALS ) );
    aOutputData.SetShowFormulas     ( rOpts.GetOption( VOPT_FORMULAS ) );
    aOutputData.SetShowSpellErrors  ( rDoc.GetDocOptions().IsAutoSpell() );
    aOutputData.SetMarkClipped      ( rOpts.GetOption( VOPT_CLIPMARKS ) );

    aOutputData.SetUseStyleColor( true );       // always set in table view

    aOutputData.SetViewShell( mrViewData.GetViewShell() );

    bool bGrid = rOpts.GetOption( VOPT_GRID ) && mrViewData.GetShowGrid();
    bool bGridFirst = !rOpts.GetOption( VOPT_GRID_ONTOP );

    bool bPage = rOpts.GetOption( VOPT_PAGEBREAKS ) && !bIsTiledRendering;

    bool bPageMode = mrViewData.IsPagebreakMode();
    if (bPageMode)                                      // after FindChanged
    {
        // SetPagebreakMode also initializes bPrinted Flags
        aOutputData.SetPagebreakMode( mrViewData.GetView()->GetPageBreakData() );
    }

    EditView*   pEditView = nullptr;
    bool        bEditMode = mrViewData.HasEditView(eWhich);
    if ( bEditMode && mrViewData.GetRefTabNo() == nTab )
    {
        SCCOL nEditCol;
        SCROW nEditRow;
        mrViewData.GetEditView( eWhich, pEditView, nEditCol, nEditRow );
        SCCOL nEditEndCol = mrViewData.GetEditEndCol();
        SCROW nEditEndRow = mrViewData.GetEditEndRow();

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

    bool bInPlaceEditing = bEditMode && (mrViewData.GetRefTabNo() == mrViewData.GetTabNo());
    vcl::Cursor* pInPlaceCrsr = nullptr;
    bool bInPlaceVisCursor = false;
    if (bInPlaceEditing)
    {
        // toggle the cursor off if its on to ensure the cursor invert
        // background logic remains valid after the background is cleared on
        // the next cursor flash
        pInPlaceCrsr = pEditView->GetCursor();
        bInPlaceVisCursor = pInPlaceCrsr && pInPlaceCrsr->IsVisible();
        if (bInPlaceVisCursor)
            pInPlaceCrsr->Hide();
    }

    OutputDevice* pContentDev = &rDevice;   // device for document content, used by overlay manager
    SdrPaintWindow* pTargetPaintWindow = nullptr; // #i74769# work with SdrPaintWindow directly

    {
        // init redraw
        ScTabViewShell* pTabViewShell = mrViewData.GetViewShell();

        if(pTabViewShell)
        {
            MapMode aCurrentMapMode(pContentDev->GetMapMode());
            pContentDev->SetMapMode(aDrawMode);
            SdrView* pDrawView = pTabViewShell->GetScDrawView();

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

    if (bGridFirst && (bGrid || bPage))
    {
        // Draw lines in background color cover over lok client grid lines in merged cell areas if bNoBackgroundAndGrid is set.
        if (bNoBackgroundAndGrid)
            aOutputData.DrawGrid(*pContentDev, false /* bGrid */, false /* bPage */, true /* bMergeCover */);
        else
            aOutputData.DrawGrid(*pContentDev, bGrid, bPage);
    }

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

    pContentDev->SetMapMode(mrViewData.GetLogicMode(eWhich));
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
    const std::vector<ScHighlightEntry> &rHigh = mrViewData.GetView()->GetHighlightRanges();
    bool bHasScenario = ( nTab+1<nTabCount && rDoc.IsScenario(nTab+1) && !rDoc.IsScenario(nTab) );
    bool bHasChange = ( rDoc.GetChangeTrack() != nullptr );

    if ( bHasChange || bHasScenario || !rHigh.empty() )
    {
        //! Merge SetChangedClip() with DrawMarks() ?? (different MapMode!)

        if ( bHasChange )
            aOutputData.DrawChangeTrack();

        if ( bHasScenario )
            lcl_DrawScenarioFrames( pContentDev, mrViewData, eWhich, nX1,nY1,nX2,nY2 );

        lcl_DrawHighlight( aOutputData, mrViewData, rHigh );
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

    if ( mrViewData.IsRefMode() && nTab >= mrViewData.GetRefStartZ() && nTab <= mrViewData.GetRefEndZ() )
    {
        Color aRefColor( rColorCfg.GetColorValue(svtools::CALCREFERENCE).nColor );
        aOutputData.DrawRefMark( mrViewData.GetRefStartX(), mrViewData.GetRefStartY(),
                                mrViewData.GetRefEndX(), mrViewData.GetRefEndY(),
                                aRefColor, false );
    }

        // range finder

    ScInputHandler* pHdl = pScMod->GetInputHdl( mrViewData.GetViewShell() );
    if (pHdl)
    {
        ScDocShell* pDocSh = mrViewData.GetDocShell();
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
        ScTabViewShell* pTabViewShell = mrViewData.GetViewShell();

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

            SdrView* pDrawView = pTabViewShell->GetScDrawView();

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
        ScTabViewShell* pThisViewShell = mrViewData.GetViewShell();
        SfxViewShell* pViewShell = SfxViewShell::GetFirst();

        while (pViewShell)
        {
            if (pViewShell != pThisViewShell && pViewShell->GetDocId() == pThisViewShell->GetDocId())
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
                            tools::Long nScreenX = aOutputData.nScrX;
                            tools::Long nScreenY = aOutputData.nScrY;

                            rDevice.SetLineColor();
                            rDevice.SetFillColor(pOtherEditView->GetBackgroundColor());
                            Point aStart = mrViewData.GetScrPos( nCol1, nRow1, eOtherWhich );
                            Point aEnd = mrViewData.GetScrPos( nCol2+1, nRow2+1, eOtherWhich );

                            // don't overwrite grid
                            tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;
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
                            // keep into account the zoom factor
                            aOrigin = Point((aOrigin.getX() * twipFactor) / static_cast<double>(aDrawMode.GetScaleX()),
                                            (aOrigin.getY() * twipFactor) / static_cast<double>(aDrawMode.GetScaleY()));

                            MapMode aNew = rDevice.GetMapMode();
                            aNew.SetOrigin(aOrigin);
                            rDevice.SetMapMode(aNew);

                            // paint the background
                            rDevice.DrawRect(rDevice.PixelToLogic(aBackground));

                            tools::Rectangle aEditRect(aBackground);
                            tools::Long nOffsetX = 0, nOffsetY = 0;
                            // Get top-left offset because of margin and indent.
                            lcl_GetEditAreaTLOffset(nOffsetX, nOffsetY, ScAddress(nCol1, nRow1, nTab), mrViewData, rDoc);
                            aEditRect.AdjustLeft(nOffsetX + 1);
                            aEditRect.AdjustTop(nOffsetY + 1);

                            // EditView has an 'output area' which is used to clip the 'paint area' we provide below.
                            // So they need to be in the same coordinates/units. This is tied to the mapmode of the gridwin
                            // attached to the EditView, so we have to change its mapmode too (temporarily). We save the
                            // original mapmode and 'output area' and roll them back when we finish painting to rDevice.
                            vcl::Window* pOtherWin = pOtherEditView->GetWindow();
                            const tools::Rectangle aOrigOutputArea(pOtherEditView->GetOutputArea()); // Not in pixels.
                            const MapMode aOrigMapMode = pOtherWin->GetMapMode();
                            pOtherWin->SetMapMode(rDevice.GetMapMode());

                            // Avoid sending wrong cursor/selection messages by the 'other' view, as the output-area is going
                            // to be tweaked temporarily to match the current view's zoom.
                            SuppressEditViewMessagesGuard aGuard(*pOtherEditView);

                            pOtherEditView->SetOutputArea(rDevice.PixelToLogic(aEditRect));
                            pOtherEditView->Paint(rDevice.PixelToLogic(aEditRect), &rDevice);

                            // Rollback the mapmode and 'output area'.
                            pOtherWin->SetMapMode(aOrigMapMode);
                            pOtherEditView->SetOutputArea(aOrigOutputArea);
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
    if (bInPlaceEditing)
    {
        // get the coordinates of the area we need to clear (overpaint by
        // the background)
        SCCOL nCol1 = mrViewData.GetEditStartCol();
        SCROW nRow1 = mrViewData.GetEditStartRow();
        SCCOL nCol2 = mrViewData.GetEditEndCol();
        SCROW nRow2 = mrViewData.GetEditEndRow();
        rDevice.SetLineColor();
        rDevice.SetFillColor(pEditView->GetBackgroundColor());
        Point aStart = mrViewData.GetScrPos( nCol1, nRow1, eWhich );
        Point aEnd = mrViewData.GetScrPos( nCol2+1, nRow2+1, eWhich );

        // don't overwrite grid
        tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;
        aEnd.AdjustX( -(2 * nLayoutSign) );
        aEnd.AdjustY( -2 );

        // set the correct mapmode
        tools::Rectangle aBackground(aStart, aEnd);
        tools::Rectangle aBGAbs(aStart, aEnd);

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
            rDevice.SetMapMode(mrViewData.GetLogicMode());

        if (bIsTiledRendering)
        {
            Point aOrigin = aOriginalMode.GetOrigin();
            aOrigin.setX(aOrigin.getX() / TWIPS_PER_PIXEL + nScrX);
            aOrigin.setY(aOrigin.getY() / TWIPS_PER_PIXEL + nScrY);
            static const double twipFactor = 15 * 1.76388889; // 26.45833335
            // keep into account the zoom factor
            aOrigin = Point((aOrigin.getX() * twipFactor) / static_cast<double>(aDrawMode.GetScaleX()),
                            (aOrigin.getY() * twipFactor) / static_cast<double>(aDrawMode.GetScaleY()));
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
        if (bIsTiledRendering)
        {
            tools::Rectangle aEditRect(aBackground);
            tools::Long nOffsetX = 0, nOffsetY = 0;
            // Get top-left offset because of margin and indent.
            lcl_GetEditAreaTLOffset(nOffsetX, nOffsetY, ScAddress(nCol1, nRow1, mrViewData.GetTabNo()), mrViewData, rDoc);
            aEditRect.AdjustLeft(nOffsetX + 1);
            aEditRect.AdjustTop(nOffsetY + 1);

            // EditView has an 'output area' which is used to clip the paint area we provide below.
            // So they need to be in the same coordinates/units. This is tied to the mapmode of the gridwin
            // attached to the EditView, so we have to change its mapmode too (temporarily). We save the
            // original mapmode and 'output area' and roll them back when we finish painting to rDevice.
            const tools::Rectangle aOrigOutputArea(pEditView->GetOutputArea()); // Not in pixels.
            const MapMode aOrigMapMode = GetMapMode();
            SetMapMode(rDevice.GetMapMode());

            // Avoid sending wrong cursor/selection messages by the current view, as the output-area is going
            // to be tweaked temporarily to match other view's zoom. (This does not affect the manual
            // cursor-messaging done in the non print-twips mode)
            SuppressEditViewMessagesGuard aGuard(*pEditView);

            pEditView->SetOutputArea(rDevice.PixelToLogic(aEditRect));
            pEditView->Paint(rDevice.PixelToLogic(aEditRect), &rDevice);

            // EditView will do the cursor notifications correctly if we're in
            // print-twips messaging mode.
            if (!comphelper::LibreOfficeKit::isCompatFlagSet(
                    comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
            {
                // Now we need to get relative cursor position within the editview.
                // This is for sending the pixel-aligned twips position of the cursor to the specific views with
                // the same given zoom level.
                tools::Rectangle aCursorRect = pEditView->GetEditCursor();
                Point aCursPos = OutputDevice::LogicToLogic(aCursorRect.TopLeft(),
                        MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));

                const MapMode& rDevMM = rDevice.GetMapMode();
                MapMode aMM(MapUnit::MapTwip);
                aMM.SetScaleX(rDevMM.GetScaleX());
                aMM.SetScaleY(rDevMM.GetScaleY());

                aBGAbs.AdjustLeft(1);
                aBGAbs.AdjustTop(1);
                aCursorRect = OutputDevice::PixelToLogic(aBGAbs, aMM);
                aCursorRect.setWidth(0);
                aCursorRect.Move(aCursPos.getX(), 0);
                // Sends view cursor position to views of all matching zooms if needed (avoids duplicates).
                InvalidateLOKViewCursor(aCursorRect, aMM.GetScaleX(), aMM.GetScaleY());
            }

            // Rollback the mapmode and 'output area'.
            SetMapMode(aOrigMapMode);
            pEditView->SetOutputArea(aOrigOutputArea);
        }
        else
        {
            tools::Rectangle aEditRect(Point(nScrX, nScrY), Size(aOutputData.GetScrW(), aOutputData.GetScrH()));
            pEditView->Paint(rDevice.PixelToLogic(aEditRect), &rDevice);
        }

        rDevice.SetMapMode(MapMode(MapUnit::MapPixel));

        // restore the cursor it was originally visible
        if (bInPlaceVisCursor)
            pInPlaceCrsr->Show();
    }

    if (mrViewData.HasEditView(eWhich))
    {
        // flush OverlayManager before changing the MapMode
        flushOverlayManager();

        // set MapMode for text edit
        rDevice.SetMapMode(mrViewData.GetLogicMode());
    }
    else
        rDevice.SetMapMode(aDrawMode);

    if (mpNoteMarker)
        mpNoteMarker->Draw(); // Above the cursor, in drawing map mode
}

namespace
{
    template<typename IndexType>
    void lcl_getBoundingRowColumnforTile(ScViewData& rViewData,
            tools::Long nTileStartPosPx, tools::Long nTileEndPosPx,
            sal_Int32& nTopLeftTileOffset, sal_Int32& nTopLeftTileOrigin,
            sal_Int32& nTopLeftTileIndex, sal_Int32& nBottomRightTileIndex)
    {
        const bool bColumnHeader = std::is_same<IndexType, SCCOL>::value;

        SCTAB nTab = rViewData.GetTabNo();

        IndexType nStartIndex = -1;
        IndexType nEndIndex = -1;
        tools::Long nStartPosPx = 0;
        tools::Long nEndPosPx = 0;

        ScPositionHelper& rPositionHelper =
                bColumnHeader ? rViewData.GetLOKWidthHelper() : rViewData.GetLOKHeightHelper();
        const auto& rStartNearest = rPositionHelper.getNearestByPosition(nTileStartPosPx);
        const auto& rEndNearest = rPositionHelper.getNearestByPosition(nTileEndPosPx);

        ScBoundsProvider aBoundsProvider(rViewData, nTab, bColumnHeader);
        aBoundsProvider.Compute(rStartNearest, rEndNearest, nTileStartPosPx, nTileEndPosPx);
        aBoundsProvider.GetStartIndexAndPosition(nStartIndex, nStartPosPx); ++nStartIndex;
        aBoundsProvider.GetEndIndexAndPosition(nEndIndex, nEndPosPx);

        nTopLeftTileOffset = nTileStartPosPx - nStartPosPx;
        nTopLeftTileOrigin = nStartPosPx;
        nTopLeftTileIndex = nStartIndex;
        nBottomRightTileIndex = nEndIndex;
    }

    class ScLOKProxyObjectContact final : public sdr::contact::ObjectContactOfPageView
    {
    private:
        ScDrawView* pScDrawView;

    public:
        explicit ScLOKProxyObjectContact(
            ScDrawView* pDrawView,
            SdrPageWindow& rPageWindow,
            const char* pDebugName) :
            ObjectContactOfPageView(rPageWindow, pDebugName),
            pScDrawView(pDrawView)
        {
        }

        virtual bool supportsGridOffsets() const override { return true; }

        virtual void calculateGridOffsetForViewOjectContact(
            basegfx::B2DVector& rTarget,
            const sdr::contact::ViewObjectContact& rClient) const override
        {
            if (!pScDrawView)
                return;

            SdrPageView* pPageView(pScDrawView->GetSdrPageView());
            if (!pPageView)
                return;

            SdrPageWindow* pSdrPageWindow = pPageView->GetPageWindow(0);
            if (!pSdrPageWindow)
                return;

            sdr::contact::ObjectContact& rObjContact(pSdrPageWindow->GetObjectContact());

            SdrObject* pTargetSdrObject(rClient.GetViewContact().TryToGetSdrObject());
            if (pTargetSdrObject)
                rTarget = pTargetSdrObject->GetViewContact().GetViewObjectContact(rObjContact).getGridOffset();
        }
    };

    class ScLOKDrawView : public FmFormView
    {
    public:
        ScLOKDrawView(OutputDevice* pOut, ScViewData& rData) :
            FmFormView(*rData.GetDocument().GetDrawLayer(), pOut),
            pScDrawView(rData.GetScDrawView())
        {
        }

        virtual sdr::contact::ObjectContact* createViewSpecificObjectContact(
                SdrPageWindow& rPageWindow, const char* pDebugName) const override
        {
            if (!pScDrawView)
                return SdrView::createViewSpecificObjectContact(rPageWindow, pDebugName);

            return new ScLOKProxyObjectContact(pScDrawView, rPageWindow, pDebugName);
        }

    private:
        ScDrawView* pScDrawView;
    };
} // anonymous namespace

void ScGridWindow::PaintTile( VirtualDevice& rDevice,
                              int nOutputWidth, int nOutputHeight,
                              int nTilePosX, int nTilePosY,
                              tools::Long nTileWidth, tools::Long nTileHeight )
{
    Fraction origZoomX = mrViewData.GetZoomX();
    Fraction origZoomY = mrViewData.GetZoomY();

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

    Fraction aFracX(tools::Long(nOutputWidth * TWIPS_PER_PIXEL), nTileWidth);
    Fraction aFracY(tools::Long(nOutputHeight * TWIPS_PER_PIXEL), nTileHeight);

    const bool bChangeZoom = (aFracX !=  origZoomX || aFracY != origZoomY);

    // page break zoom, and aLogicMode in ScViewData
    // FIXME: there are issues when SetZoom is called conditionally.
    mrViewData.SetZoom(aFracX, aFracY, true);
    if (bChangeZoom)
    {
        if (ScDrawView* pDrawView = mrViewData.GetScDrawView())
            pDrawView->resetGridOffsetsForAllSdrPageViews();
    }

    const double fTilePosXPixel = static_cast<double>(nTilePosX) * nOutputWidth / nTileWidth;
    const double fTilePosYPixel = static_cast<double>(nTilePosY) * nOutputHeight / nTileHeight;
    const double fTileBottomPixel = static_cast<double>(nTilePosY + nTileHeight) * nOutputHeight / nTileHeight;
    const double fTileRightPixel = static_cast<double>(nTilePosX + nTileWidth) * nOutputWidth / nTileWidth;

    SCTAB nTab = mrViewData.GetTabNo();
    ScDocument& rDoc = mrViewData.GetDocument();

    const double fPPTX = mrViewData.GetPPTX();
    const double fPPTY = mrViewData.GetPPTY();

    // find approximate col/row offsets of nearby.
    sal_Int32 nTopLeftTileRowOffset = 0;
    sal_Int32 nTopLeftTileColOffset = 0;
    sal_Int32 nTopLeftTileRowOrigin = 0;
    sal_Int32 nTopLeftTileColOrigin = 0;

    sal_Int32 nTopLeftTileRow = 0;
    sal_Int32 nTopLeftTileCol = 0;
    sal_Int32 nBottomRightTileRow = 0;
    sal_Int32 nBottomRightTileCol = 0;

    lcl_getBoundingRowColumnforTile<SCROW>(mrViewData,
            fTilePosYPixel, fTileBottomPixel,
            nTopLeftTileRowOffset, nTopLeftTileRowOrigin,
            nTopLeftTileRow, nBottomRightTileRow);

    lcl_getBoundingRowColumnforTile<SCCOL>(mrViewData,
            fTilePosXPixel, fTileRightPixel,
            nTopLeftTileColOffset, nTopLeftTileColOrigin,
            nTopLeftTileCol, nBottomRightTileCol);

    // Enlarge
    nBottomRightTileCol++;
    nBottomRightTileRow++;

    if (nBottomRightTileCol > rDoc.MaxCol())
        nBottomRightTileCol = rDoc.MaxCol();

    if (nBottomRightTileRow > MAXTILEDROW)
        nBottomRightTileRow = MAXTILEDROW;

    // size of the document including drawings, charts, etc.
    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    rDoc.GetTiledRenderingArea(nTab, nEndCol, nEndRow);

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
    rDoc.FillInfo(aTabInfo, nTopLeftTileCol, nTopLeftTileRow,
                   nBottomRightTileCol, nBottomRightTileRow,
                   nTab, fPPTX, fPPTY, false, false);

// FIXME: is this called some
//        Point aScrPos = mrViewData.GetScrPos( nX1, nY1, eWhich );

    ScOutputData aOutputData(&rDevice, OUTTYPE_WINDOW, aTabInfo, &rDoc, nTab,
                             -nTopLeftTileColOffset,
                             -nTopLeftTileRowOffset,
                             nTopLeftTileCol, nTopLeftTileRow,
                             nBottomRightTileCol, nBottomRightTileRow,
                             fPPTX, fPPTY, nullptr, nullptr);

    // setup the SdrPage so that drawinglayer works correctly
    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    if (pModel)
    {
        bool bPrintTwipsMsgs = comphelper::LibreOfficeKit::isCompatFlagSet(
                comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);
        mpLOKDrawView.reset(bPrintTwipsMsgs ?
            new ScLOKDrawView(
                &rDevice,
                mrViewData) :
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
    rDoc.ResetChanged(ScRange(nTopLeftTileCol, nTopLeftTileRow, nTab, nBottomRightTileCol, nBottomRightTileRow, nTab));
    rDoc.PrepareFormulaCalc();

    mrViewData.SetZoom(origZoomX, origZoomY, true);
    if (bChangeZoom)
    {
        if (ScDrawView* pDrawView = mrViewData.GetScDrawView())
            pDrawView->resetGridOffsetsForAllSdrPageViews();
    }
}

void ScGridWindow::LogicInvalidate(const tools::Rectangle* pRectangle)
{
    tools::Rectangle aRectangle;
    tools::Rectangle* pResultRectangle;
    if (!pRectangle)
        pResultRectangle = nullptr;
    else
    {
        aRectangle = *pRectangle;
        // When dragging shapes the map mode is disabled.
        if (IsMapModeEnabled())
        {
            if (GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                aRectangle = OutputDevice::LogicToLogic(aRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
        }
        else
            aRectangle = PixelToLogic(aRectangle, MapMode(MapUnit::MapTwip));
        pResultRectangle = &aRectangle;
    }

    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
    SfxLokHelper::notifyInvalidation(pViewShell, pResultRectangle);
}

void ScGridWindow::SetCellSelectionPixel(int nType, int nPixelX, int nPixelY)
{
    ScTabView* pTabView = mrViewData.GetView();
    ScTabViewShell* pViewShell = mrViewData.GetViewShell();
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
    ScRangeList aRangeList = mrViewData.GetMarkData().GetMarkedRanges();

    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    SCTAB nTab1, nTab2;

    bool bWasEmpty = false;
    if (aRangeList.empty())
    {
        nCol1 = nCol2 = mrViewData.GetCurX();
        nRow1 = nRow2 = mrViewData.GetCurY();
        bWasEmpty = true;
    }
    else
        aRangeList.Combine().GetVars(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);

    // convert the coordinates to column/row
    SCCOL nNewPosX;
    SCROW nNewPosY;
    SCTAB nTab = mrViewData.GetTabNo();
    mrViewData.GetPosFromPixel(nPixelX, nPixelY, eWhich, nNewPosX, nNewPosY);

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

    if (!bNeedsRepaint)
        return;

    bNeedsRepaint = false;
    if (aRepaintPixel.IsEmpty())
        Invalidate();
    else
        Invalidate(PixelToLogic(aRepaintPixel));
    aRepaintPixel = tools::Rectangle();

    // selection function in status bar might also be invalid
    SfxBindings& rBindings = mrViewData.GetBindings();
    rBindings.Invalidate( SID_STATUS_SUM );
    rBindings.Invalidate( SID_ATTR_SIZE );
    rBindings.Invalidate( SID_TABLE_CELL );
}

void ScGridWindow::DrawPagePreview( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2, vcl::RenderContext& rRenderContext)
{
    ScPageBreakData* pPageData = mrViewData.GetView()->GetPageBreakData();
    if (!pPageData)
        return;

    ScDocument& rDoc = mrViewData.GetDocument();
    SCTAB nTab = mrViewData.GetTabNo();
    Size aWinSize = GetOutputSizePixel();
    const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
    Color aManual( rColorCfg.GetColorValue(svtools::CALCPAGEBREAKMANUAL).nColor );
    Color aAutomatic( rColorCfg.GetColorValue(svtools::CALCPAGEBREAK).nColor );

    OUString aPageStr = ScResId( STR_PGNUM );
    if ( nPageScript == SvtScriptType::NONE )
    {
        //  get script type of translated "Page" string only once
        nPageScript = rDoc.GetStringScriptType( aPageStr );
        if (nPageScript == SvtScriptType::NONE)
            nPageScript = ScGlobal::GetDefaultScriptType();
    }

    vcl::Font aFont;
    std::unique_ptr<ScEditEngineDefaulter> pEditEng;
    const ScPatternAttr& rDefPattern = rDoc.GetPool()->GetDefaultItem(ATTR_PATTERN);
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

            Point aStart = mrViewData.GetScrPos(
                                aRange.aStart.Col(), aRange.aStart.Row(), eWhich, true );
            Point aEnd = mrViewData.GetScrPos(
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
                    if (rDoc.HasColBreak(nBreak, nTab) & ScBreakType::Manual)
                        rRenderContext.SetFillColor( aManual );
                    else
                        rRenderContext.SetFillColor( aAutomatic );
                    Point aBreak = mrViewData.GetScrPos(
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
                    if (rDoc.HasRowBreak(nBreak, nTab) & ScBreakType::Manual)
                        rRenderContext.SetFillColor( aManual );
                    else
                        rRenderContext.SetFillColor( aAutomatic );
                    Point aBreak = mrViewData.GetScrPos(
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
                            Point aPageStart = mrViewData.GetScrPos(
                                                    nPrStartX, nPrStartY, eWhich, true );
                            Point aPageEnd = mrViewData.GetScrPos(
                                                    nPrEndX+1,nPrEndY+1, eWhich, true );

                            tools::Long nPageNo = rData.GetFirstPage();
                            if ( rData.IsTopDown() )
                                nPageNo += static_cast<tools::Long>(nColPos)*nRowBreaks+nRowPos;
                            else
                                nPageNo += static_cast<tools::Long>(nRowPos)*nColBreaks+nColPos;

                            OUString aThisPageStr = aPageStr.replaceFirst("%1", OUString::number(nPageNo));

                            if ( pEditEng )
                            {
                                //  find right font size with EditEngine
                                tools::Long nHeight = 100;
                                pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
                                pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
                                pEditEng->SetDefaultItem( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
                                pEditEng->SetTextCurrentDefaults( aThisPageStr );
                                Size aSize100( pEditEng->CalcTextWidth(), pEditEng->GetTextHeight() );

                                //  40% of width or 60% of height
                                tools::Long nSizeX = 40 * ( aPageEnd.X() - aPageStart.X() ) / aSize100.Width();
                                tools::Long nSizeY = 60 * ( aPageEnd.Y() - aPageStart.Y() ) / aSize100.Height();
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
                                tools::Long nSizeX = 40 * ( aPageEnd.X() - aPageStart.X() ) / aSize100.Width();
                                tools::Long nSizeY = 60 * ( aPageEnd.Y() - aPageStart.Y() ) / aSize100.Height();
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

void ScGridWindow::DrawButtons(SCCOL nX1, SCCOL nX2, const ScTableInfo& rTabInfo, OutputDevice* pContentDev)
{
    aComboButton.SetOutputDevice( pContentDev );

    ScDocument& rDoc = mrViewData.GetDocument();
    ScDPFieldButton aCellBtn(pContentDev, &GetSettings().GetStyleSettings(), &mrViewData.GetZoomY(), &rDoc);

    SCCOL nCol;
    SCROW nRow;
    SCSIZE nArrY;
    SCSIZE nQuery;
    SCTAB           nTab = mrViewData.GetTabNo();
    ScDBData*       pDBData = nullptr;
    std::unique_ptr<ScQueryParam> pQueryParam;

    RowInfo*        pRowInfo = rTabInfo.mpRowInfo.get();
    sal_uInt16      nArrCount = rTabInfo.mnArrCount;

    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

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
                        pDBData = rDoc.GetDBAtCursor( nCol, nRow, nTab, ScDBDataPortion::AREA );
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
                    tools::Long    nSizeX;
                    tools::Long    nSizeY;
                    SCCOL nStartCol= nCol;
                    SCROW nStartRow = nRow;
                    //if address(nCol,nRow) is not the start pos of the merge area, the value of the nSizeX will be incorrect, it will be the length of the cell.
                    //should first get the start pos of the merge area, then get the nSizeX through the start pos.
                    rDoc.ExtendOverlapped(nStartCol, nStartRow,nCol, nRow, nTab);//get nStartCol,nStartRow
                    mrViewData.GetMergeSizePixel( nStartCol, nStartRow, nSizeX, nSizeY );//get nSizeX
                    nSizeY = ScViewData::ToPixel(rDoc.GetRowHeight(nRow, nTab), mrViewData.GetPPTY());
                    Point aScrPos = mrViewData.GetScrPos( nCol, nRow, eWhich );

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

                Point aScrPos = mrViewData.GetScrPos( nCol, nRow, eWhich );
                tools::Long nSizeX;
                tools::Long nSizeY;
                mrViewData.GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
                tools::Long nPosX = aScrPos.X();
                tools::Long nPosY = aScrPos.Y();
                // bLayoutRTL is handled in setBoundingBox

                OUString aStr = rDoc.GetString(nCol, nRow, nTab);
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
    ScDocument& rDoc = mrViewData.GetDocument();
    SCTAB nTab = mrViewData.GetTabNo();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    ScDDComboBoxButton aButton( this );             // for optimal size
    Size aBtnSize = aButton.GetSizePixel();

    SCCOL nCol = rButtonPos.Col();
    SCROW nRow = rButtonPos.Row();

    tools::Long nCellSizeX;    // width of this cell, including merged
    tools::Long nDummy;
    mrViewData.GetMergeSizePixel( nCol, nRow, nCellSizeX, nDummy );

    // for height, only the cell's row is used, excluding merged cells
    tools::Long nCellSizeY = ScViewData::ToPixel( rDoc.GetRowHeight( nRow, nTab ), mrViewData.GetPPTY() );
    tools::Long nAvailable = nCellSizeX;

    //  left edge of next cell if there is a non-hidden next column
    SCCOL nNextCol = nCol + 1;
    const ScMergeAttr* pMerge = rDoc.GetAttr( nCol,nRow,nTab, ATTR_MERGE );
    if ( pMerge->GetColMerge() > 1 )
        nNextCol = nCol + pMerge->GetColMerge();    // next cell after the merged area
    while ( nNextCol <= rDoc.MaxCol() && rDoc.ColHidden(nNextCol, nTab) )
        ++nNextCol;
    bool bNextCell = ( nNextCol <= rDoc.MaxCol() );
    if ( bNextCell )
        nAvailable = ScViewData::ToPixel( rDoc.GetColWidth( nNextCol, nTab ), mrViewData.GetPPTX() );

    if ( nAvailable < aBtnSize.Width() )
        aBtnSize.setWidth( nAvailable );
    if ( nCellSizeY < aBtnSize.Height() )
        aBtnSize.setHeight( nCellSizeY );

    Point aPos = mrViewData.GetScrPos( nCol, nRow, eWhich, true );
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
    ScDocument&     rDoc    = mrViewData.GetDocument();
    ScDBData*       pDBData = rDoc.GetDBAtCursor( nCol, nRow, nTab, ScDBDataPortion::AREA );
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

void ScGridWindow::GetSelectionRects( ::std::vector< tools::Rectangle >& rPixelRects ) const
{
    GetPixelRectsFor( mrViewData.GetMarkData(), rPixelRects );
}

void ScGridWindow::GetSelectionRectsPrintTwips(::std::vector< tools::Rectangle >& rRects) const
{
    GetRectsAnyFor(mrViewData.GetMarkData(), rRects, true);
}

/// convert rMarkData into pixel rectangles for this view
void ScGridWindow::GetPixelRectsFor( const ScMarkData &rMarkData,
                                     ::std::vector< tools::Rectangle >& rPixelRects ) const
{
    GetRectsAnyFor(rMarkData, rPixelRects, false);
}

void ScGridWindow::GetRectsAnyFor(const ScMarkData &rMarkData,
                                  ::std::vector< tools::Rectangle >& rRects,
                                  bool bInPrintTwips) const
{
    ScDocument& rDoc = mrViewData.GetDocument();
    SCTAB nTab = mrViewData.GetTabNo();
    double nPPTX = mrViewData.GetPPTX();
    double nPPTY = mrViewData.GetPPTY();
    bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    ScMarkData aMultiMark( rMarkData );
    aMultiMark.SetMarking( false );

    if (!aMultiMark.IsMultiMarked())
    {
        // simple range case - simplify calculation
        ScRange aSimpleRange;
        aMultiMark.GetMarkArea( aSimpleRange );

        aMultiMark.MarkToMulti();
        if ( !aMultiMark.IsMultiMarked() )
            return;

        SCCOL nX1 = aSimpleRange.aStart.Col();
        SCROW nY1 = aSimpleRange.aStart.Row();
        SCCOL nX2 = aSimpleRange.aEnd.Col();
        SCROW nY2 = aSimpleRange.aEnd.Row();

        PutInOrder( nX1, nX2 );
        PutInOrder( nY1, nY2 );

        SCCOL nPosX = mrViewData.GetPosX( eHWhich );
        SCROW nPosY = mrViewData.GetPosY( eVWhich );
        // is the selection visible at all?
        if (nX2 < nPosX || nY2 < nPosY)
            return;

        Point aScrStartPos = bInPrintTwips ? mrViewData.GetPrintTwipsPos(nX1, nY1) :
            mrViewData.GetScrPos(nX1, nY1, eWhich);

        tools::Long nStartX = aScrStartPos.X();
        tools::Long nStartY = aScrStartPos.Y();

        Point aScrEndPos = bInPrintTwips ? mrViewData.GetPrintTwipsPos(nX2, nY2) :
            mrViewData.GetScrPos(nX2, nY2, eWhich);

        tools::Long nWidthTwips = rDoc.GetColWidth(nX2, nTab);
        const tools::Long nWidth = bInPrintTwips ?
            nWidthTwips : ScViewData::ToPixel(nWidthTwips, nPPTX);
        tools::Long nEndX = aScrEndPos.X() + (nWidth - 1) * nLayoutSign;

        sal_uInt16 nHeightTwips = rDoc.GetRowHeight( nY2, nTab );
        const tools::Long nHeight = bInPrintTwips ?
            nHeightTwips : ScViewData::ToPixel(nHeightTwips, nPPTY);
        tools::Long nEndY = aScrEndPos.Y() + nHeight - 1;

        ScInvertMerger aInvert( &rRects );
        aInvert.AddRect( tools::Rectangle( nStartX, nStartY, nEndX, nEndY ) );

        return;
    }

    aMultiMark.MarkToMulti();
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

    rDoc.ExtendMerge( nX1,nY1, nTestX2,nTestY2, nTab );

    SCCOL nPosX = mrViewData.GetPosX( eHWhich );
    SCROW nPosY = mrViewData.GetPosY( eVWhich );
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
        SCCOL nXRight = nPosX + mrViewData.VisibleCellsX(eHWhich);
        if (nXRight > rDoc.MaxCol())
            nXRight = rDoc.MaxCol();

        SCROW nYBottom = nPosY + mrViewData.VisibleCellsY(eVWhich);
        if (nYBottom > rDoc.MaxRow())
            nYBottom = rDoc.MaxRow();

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
        rDoc.GetTiledRenderingArea(nTab, nMaxTiledCol, nMaxTiledRow);

        if (nX2 > nMaxTiledCol)
            nX2 = nMaxTiledCol;
        if (nY2 > nMaxTiledRow)
            nY2 = nMaxTiledRow;
    }

    ScInvertMerger aInvert( &rRects );

    Point aScrPos = bInPrintTwips ? mrViewData.GetPrintTwipsPos(nX1, nY1) :
            mrViewData.GetScrPos(nX1, nY1, eWhich);
    tools::Long nScrY = aScrPos.Y();
    bool bWasHidden = false;
    for (SCROW nY=nY1; nY<=nY2; nY++)
    {
        bool bFirstRow = ( nY == nPosY );                       // first visible row?
        bool bDoHidden = false;                                 // repeat hidden ?
        sal_uInt16 nHeightTwips = rDoc.GetRowHeight( nY,nTab );
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
                while ( rDoc.GetAttr(nStartX,nY,nTab,ATTR_MERGE_FLAG)->IsHorOverlapped() )
                    --nStartX;
                if (nStartX <= nX2)
                    nLoopEndX = nX1;
            }

            const tools::Long nHeight = bInPrintTwips ?
                    nHeightTwips : ScViewData::ToPixel(nHeightTwips, nPPTY);
            tools::Long nEndY = nScrY + nHeight - 1;
            tools::Long nScrX = aScrPos.X();
            for (SCCOL nX=nX1; nX<=nLoopEndX; nX++)
            {
                tools::Long nWidth = rDoc.GetColWidth(nX, nTab);
                if (!bInPrintTwips)
                    nWidth = ScViewData::ToPixel(nWidth, nPPTX);

                if ( nWidth > 0 )
                {
                    tools::Long nEndX = nScrX + ( nWidth - 1 ) * nLayoutSign;

                    SCROW nThisY = nY;
                    const ScPatternAttr* pPattern = rDoc.GetPattern( nX, nY, nTab );
                    const ScMergeFlagAttr* pMergeFlag = &pPattern->GetItem(ATTR_MERGE_FLAG);
                    if ( pMergeFlag->IsVerOverlapped() && ( bDoHidden || bFirstRow ) )
                    {
                        while ( pMergeFlag->IsVerOverlapped() && nThisY > 0 &&
                                (rDoc.RowHidden(nThisY-1, nTab) || bFirstRow) )
                        {
                            --nThisY;
                            pPattern = rDoc.GetPattern( nX, nThisY, nTab );
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
                            pPattern = rDoc.GetPattern( nThisX, nThisY, nTab );
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
                                const SCCOL nEndColMerge = nThisX + pMerge->GetColMerge();
                                const SCROW nEndRowMerge = nThisY + pMerge->GetRowMerge();
                                Point aEndPos = bInPrintTwips ?
                                        mrViewData.GetPrintTwipsPos(nEndColMerge, nEndRowMerge) :
                                        mrViewData.GetScrPos(nEndColMerge, nEndRowMerge, eWhich);
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

    if ( !((rDCEvt.GetType() == DataChangedEventType::PRINTER) ||
         (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))) )
        return;

    if ( rDCEvt.GetType() == DataChangedEventType::FONTS && eWhich == mrViewData.GetActivePart() )
        mrViewData.GetDocShell()->UpdateFontList();

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        if ( eWhich == mrViewData.GetActivePart() )     // only once for the view
        {
            ScTabView* pView = mrViewData.GetView();

            pView->RecalcPPT();

            //  RepeatResize in case scroll bar sizes have changed
            pView->RepeatResize();
            pView->UpdateAllOverlays();

            //  invalidate cell attribs in input handler, in case the
            //  EditEngine BackgroundColor has to be changed
            if ( mrViewData.IsActive() )
            {
                ScInputHandler* pHdl = SC_MOD()->GetInputHdl();
                if (pHdl)
                    pHdl->ForgetLastPattern();
            }
        }
    }

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
