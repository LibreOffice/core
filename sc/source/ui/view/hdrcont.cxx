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

#include <sfx2/dispatch.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <svtools/colorcfg.hxx>
#include <osl/diagnose.h>

#include <tabvwsh.hxx>
#include <hdrcont.hxx>
#include <dbdata.hxx>
#include <scmod.hxx>
#include <inputopt.hxx>
#include <gridmerg.hxx>
#include <document.hxx>
#include <markdata.hxx>
#include <tabview.hxx>
#include <viewdata.hxx>
#include <columnspanset.hxx>

#define SC_DRAG_MIN     2

//  passes in paint
//  (selection left/right must be first because the continuous lines
//  are partly overwritten later)

#define SC_HDRPAINT_SEL_BOTTOM  4
#define SC_HDRPAINT_BOTTOM      5
#define SC_HDRPAINT_TEXT        6
#define SC_HDRPAINT_COUNT       7

ScHeaderControl::ScHeaderControl( vcl::Window* pParent, SelectionEngine* pSelectionEngine,
                                  SCCOLROW nNewSize, bool bNewVertical, ScTabView* pTab ) :
            Window      ( pParent ),
            pSelEngine  ( pSelectionEngine ),
            aShowHelpTimer("sc HeaderControl Popover Timer"),
            bVertical   ( bNewVertical ),
            nSize       ( nNewSize ),
            nMarkStart  ( 0 ),
            nMarkEnd    ( 0 ),
            bMarkRange  ( false ),
            bDragging   ( false ),
            nDragNo     ( 0 ),
            nDragStart  ( 0 ),
            nDragPos    ( 0 ),
            nTipVisible ( nullptr ),
            bDragMoved  ( false ),
            bIgnoreMove ( false ),
            bInRefMode  ( false ),
            pTabView    ( pTab )
{
    // RTL: no default mirroring for this window, the spreadsheet itself
    // is also not mirrored
    // mirror the vertical window for correct border drawing
    // table layout depends on sheet format, not UI setting, so the
    // borders of the vertical window have to be handled manually, too.
    EnableRTL( false );

    aNormFont = GetFont();
    aNormFont.SetTransparent( true );       //! hard-set WEIGHT_NORMAL ???
    aBoldFont = aNormFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );
    aAutoFilterFont = aNormFont;

    SetFont(aBoldFont);
    bBoldSet = true;
    bAutoFilterSet = false;

    Size aSize = LogicToPixel( Size(
        GetTextWidth("8888"),
        GetTextHeight() ) );
    aSize.AdjustWidth(4 );    // place for highlight border
    aSize.AdjustHeight(3 );
    SetSizePixel( aSize );

    nWidth = nSmallWidth = aSize.Width();
    nBigWidth = LogicToPixel( Size( GetTextWidth("8888888"), 0 ) ).Width() + 5;

    aShowHelpTimer.SetInvokeHandler(LINK(this, ScHeaderControl, ShowDragHelpHdl));
    aShowHelpTimer.SetTimeout(GetSettings().GetMouseSettings().GetDoubleClickTime());

    SetBackground();
}

void ScHeaderControl::dispose()
{
    aShowHelpTimer.Stop();
    vcl::Window::dispose();
}

void ScHeaderControl::SetWidth( tools::Long nNew )
{
    OSL_ENSURE( bVertical, "SetWidth works only on row headers" );
    if ( nNew != nWidth )
    {
        Size aSize( nNew, GetSizePixel().Height() );
        SetSizePixel( aSize );

        nWidth = nNew;

        Invalidate();
    }
}

ScHeaderControl::~ScHeaderControl()
{
}

void ScHeaderControl::DoPaint( SCCOLROW nStart, SCCOLROW nEnd )
{
    bool bLayoutRTL = IsLayoutRTL();
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    tools::Rectangle aRect( Point(0,0), GetOutputSizePixel() );
    if ( bVertical )
    {
        aRect.SetTop( GetScrPos( nStart )-nLayoutSign );      // extra pixel for line at top of selection
        aRect.SetBottom( GetScrPos( nEnd+1 )-nLayoutSign );
    }
    else
    {
        aRect.SetLeft( GetScrPos( nStart )-nLayoutSign );     // extra pixel for line left of selection
        aRect.SetRight( GetScrPos( nEnd+1 )-nLayoutSign );
    }
    Invalidate(aRect);
}

void ScHeaderControl::SetMark( bool bNewSet, SCCOLROW nNewStart, SCCOLROW nNewEnd )
{
    bool bEnabled = SC_MOD()->GetInputOptions().GetMarkHeader();    //! cache?
    if (!bEnabled)
        bNewSet = false;

    bool bOldSet       = bMarkRange;
    SCCOLROW nOldStart = nMarkStart;
    SCCOLROW nOldEnd   = nMarkEnd;
    PutInOrder( nNewStart, nNewEnd );
    bMarkRange = bNewSet;
    nMarkStart = nNewStart;
    nMarkEnd   = nNewEnd;

    //  Paint

    if ( bNewSet )
    {
        if ( bOldSet )
        {
            if ( nNewStart == nOldStart )
            {
                if ( nNewEnd != nOldEnd )
                    DoPaint( std::min( nNewEnd, nOldEnd ) + 1, std::max( nNewEnd, nOldEnd ) );
            }
            else if ( nNewEnd == nOldEnd )
                DoPaint( std::min( nNewStart, nOldStart ), std::max( nNewStart, nOldStart ) - 1 );
            else if ( nNewStart > nOldEnd || nNewEnd < nOldStart )
            {
                //  two areas
                DoPaint( nOldStart, nOldEnd );
                DoPaint( nNewStart, nNewEnd );
            }
            else //  somehow overlapping... (it is not often)
                DoPaint( std::min( nNewStart, nOldStart ), std::max( nNewEnd, nOldEnd ) );
        }
        else
            DoPaint( nNewStart, nNewEnd );      //  completely new selection
    }
    else if ( bOldSet )
        DoPaint( nOldStart, nOldEnd );          //  cancel selection
}

tools::Long ScHeaderControl::GetScrPos( SCCOLROW nEntryNo ) const
{
    tools::Long nScrPos;

    tools::Long nMax = ( bVertical ? GetOutputSizePixel().Height() : GetOutputSizePixel().Width() ) + 1;
    if (nEntryNo >= nSize)
        nScrPos = nMax;
    else
    {
        nScrPos = 0;
        for (SCCOLROW i=GetPos(); i<nEntryNo && nScrPos<nMax; i++)
        {
            sal_uInt16 nAdd = GetEntrySize(i);
            if (nAdd)
                nScrPos += nAdd;
            else
            {
                SCCOLROW nHidden = GetHiddenCount(i);
                if (nHidden > 0)
                    i += nHidden - 1;
            }
        }
    }

    if ( IsLayoutRTL() )
        nScrPos = nMax - nScrPos - 2;

    return nScrPos;
}

void ScHeaderControl::Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect )
{
    // It is important for VCL to have few calls, that is why the outer lines are
    // grouped together

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    bool bHighContrast = rStyleSettings.GetHighContrastMode();
    bool bDark = rStyleSettings.GetFaceColor().IsDark();
    // Use the same distinction for bDark as in Window::DrawSelectionBackground

    Color aTextColor = rStyleSettings.GetButtonTextColor();
    Color aSelTextColor = rStyleSettings.GetHighlightTextColor();
    Color aAFilterTextColor = COL_LIGHTBLUE;    // color of filtered row numbers
    aNormFont.SetColor( aTextColor );
    aAutoFilterFont.SetColor(aAFilterTextColor);
    if ( bHighContrast )
        aBoldFont.SetColor( aTextColor );
    else
        aBoldFont.SetColor( aSelTextColor );

    if (bAutoFilterSet)
        SetTextColor(aAFilterTextColor);
    else
        SetTextColor((bBoldSet && !bHighContrast) ? aSelTextColor : aTextColor);

    Color aSelLineColor = rStyleSettings.GetHighlightColor();
    aSelLineColor.Merge( COL_BLACK, 0xe0 );        // darken just a little bit

    bool bLayoutRTL = IsLayoutRTL();
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;
    bool bMirrored = IsMirrored();

    OUString            aString;
    sal_uInt16          nBarSize;
    Point               aScrPos;
    Size                aTextSize;

    if (bVertical)
        nBarSize = static_cast<sal_uInt16>(GetSizePixel().Width());
    else
        nBarSize = static_cast<sal_uInt16>(GetSizePixel().Height());

    SCCOLROW    nPos = GetPos();

    tools::Long nPStart = bVertical ? rRect.Top() : rRect.Left();
    tools::Long nPEnd = bVertical ? rRect.Bottom() : rRect.Right();

    tools::Long nTransStart = nPEnd + 1;
    tools::Long nTransEnd = 0;

    tools::Long nInitScrPos = 0;
    if ( bLayoutRTL )
    {
        tools::Long nTemp = nPStart;       // swap nPStart / nPEnd
        nPStart = nPEnd;
        nPEnd = nTemp;
        nTemp = nTransStart;        // swap nTransStart / nTransEnd
        nTransStart = nTransEnd;
        nTransEnd = nTemp;
        if ( bVertical )            // start loops from the end
            nInitScrPos = GetSizePixel().Height() - 1;
        else
            nInitScrPos = GetSizePixel().Width() - 1;
    }

    // complete the painting of the outer lines
    // first find the end of the last cell

    tools::Long nLineEnd = nInitScrPos - nLayoutSign;

    for (SCCOLROW i=nPos; i<nSize; i++)
    {
        sal_uInt16 nSizePix = GetEntrySize( i );
        if (nSizePix)
        {
            nLineEnd += nSizePix * nLayoutSign;

            if ( bMarkRange && i >= nMarkStart && i <= nMarkEnd )
            {
                tools::Long nLineStart = nLineEnd - ( nSizePix - 1 ) * nLayoutSign;
                if ( nLineStart * nLayoutSign < nTransStart * nLayoutSign )
                    nTransStart = nLineStart;
                if ( nLineEnd * nLayoutSign > nTransEnd * nLayoutSign )
                    nTransEnd = nLineEnd;
            }

            if ( nLineEnd * nLayoutSign > nPEnd * nLayoutSign )
            {
                nLineEnd = nPEnd;
                break;
            }
        }
        else
        {
            SCCOLROW nHidden = GetHiddenCount(i);
            if (nHidden > 0)
                i += nHidden - 1;
        }
    }

    //  background is different for entry area and behind the entries

    tools::Rectangle aFillRect;
    GetOutDev()->SetLineColor();

    if ( nLineEnd * nLayoutSign >= nInitScrPos * nLayoutSign )
    {
        GetOutDev()->SetFillColor( rStyleSettings.GetFaceColor() );
        if ( bVertical )
            aFillRect = tools::Rectangle( 0, nInitScrPos, nBarSize-1, nLineEnd );
        else
            aFillRect = tools::Rectangle( nInitScrPos, 0, nLineEnd, nBarSize-1 );
        GetOutDev()->DrawRect( aFillRect );
    }

    if ( nLineEnd * nLayoutSign < nPEnd * nLayoutSign )
    {
        GetOutDev()->SetFillColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::APPBACKGROUND).nColor );
        if ( bVertical )
            aFillRect = tools::Rectangle( 0, nLineEnd+nLayoutSign, nBarSize-1, nPEnd );
        else
            aFillRect = tools::Rectangle( nLineEnd+nLayoutSign, 0, nPEnd, nBarSize-1 );
        GetOutDev()->DrawRect( aFillRect );
    }

    if ( nLineEnd * nLayoutSign >= nPStart * nLayoutSign )
    {
        if ( nTransEnd * nLayoutSign >= nTransStart * nLayoutSign )
        {
            if (bVertical)
                aFillRect = tools::Rectangle( 0, nTransStart, nBarSize-1, nTransEnd );
            else
                aFillRect = tools::Rectangle( nTransStart, 0, nTransEnd, nBarSize-1 );

            if ( bHighContrast )
            {
                if ( bDark )
                {
                    //  solid grey background for dark face color is drawn before lines
                    GetOutDev()->SetLineColor();
                    GetOutDev()->SetFillColor( COL_LIGHTGRAY );
                    GetOutDev()->DrawRect( aFillRect );
                }
            }
            else
            {
                // background for selection
                GetOutDev()->SetLineColor();
                GetOutDev()->SetFillColor( rStyleSettings.GetHighlightColor() );
                GetOutDev()->DrawRect( aFillRect );
            }
        }

        GetOutDev()->SetLineColor( rStyleSettings.GetDarkShadowColor() );
        if (bVertical)
        {
            tools::Long nDarkPos = bMirrored ? 0 : nBarSize-1;
            GetOutDev()->DrawLine( Point( nDarkPos, nPStart ), Point( nDarkPos, nLineEnd ) );
        }
        else
            GetOutDev()->DrawLine( Point( nPStart, nBarSize-1 ), Point( nLineEnd, nBarSize-1 ) );

        // line in different color for selection
        if ( nTransEnd * nLayoutSign >= nTransStart * nLayoutSign && !bHighContrast )
        {
            GetOutDev()->SetLineColor( aSelLineColor );
            if (bVertical)
            {
                tools::Long nDarkPos = bMirrored ? 0 : nBarSize-1;
                GetOutDev()->DrawLine( Point( nDarkPos, nTransStart ), Point( nDarkPos, nTransEnd ) );
            }
            else
                GetOutDev()->DrawLine( Point( nTransStart, nBarSize-1 ), Point( nTransEnd, nBarSize-1 ) );
        }
    }

    // tdf#89841 Use blue row numbers when Autofilter selected
    std::vector<sc::ColRowSpan> aSpans;
    if (bVertical && pTabView)
    {
        SCTAB nTab = pTabView->GetViewData().GetTabNo();
        ScDocument& rDoc = pTabView->GetViewData().GetDocument();

        ScDBData* pDBData = rDoc.GetAnonymousDBData(nTab);
        if (pDBData && pDBData->HasAutoFilter())
        {
            SCSIZE nSelected = 0;
            SCSIZE nTotal = 0;
            pDBData->GetFilterSelCount(nSelected, nTotal);
            if (nTotal > nSelected)
            {
                ScRange aRange;
                pDBData->GetArea(aRange);
                SCCOLROW nStartRow = static_cast<SCCOLROW>(aRange.aStart.Row());
                SCCOLROW nEndRow = static_cast<SCCOLROW>(aRange.aEnd.Row());
                if (pDBData->HasHeader())
                    nStartRow++;
                aSpans.push_back(sc::ColRowSpan(nStartRow, nEndRow));
            }
        }

        ScDBCollection* pDocColl = rDoc.GetDBCollection();
        if (!pDocColl->empty())
        {
            ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
            for (const auto& rxDB : rDBs)
            {
                if (rxDB->GetTab() == nTab && rxDB->HasAutoFilter())
                {
                    SCSIZE nSelected = 0;
                    SCSIZE nTotal = 0;
                    rxDB->GetFilterSelCount(nSelected, nTotal);
                    if (nTotal > nSelected)
                    {
                        ScRange aRange;
                        rxDB->GetArea(aRange);
                        SCCOLROW nStartRow = static_cast<SCCOLROW>(aRange.aStart.Row());
                        SCCOLROW nEndRow = static_cast<SCCOLROW>(aRange.aEnd.Row());
                        if (rxDB->HasHeader())
                            nStartRow++;
                        aSpans.push_back(sc::ColRowSpan(nStartRow, nEndRow));
                    }
                }
            }
        }
    }

    //  loop through entries several times to avoid changing the line color too often
    //  and to allow merging of lines

    ScGridMerger aGrid( GetOutDev(), 1, 1 );

    //  start at SC_HDRPAINT_BOTTOM instead of 0 - selection doesn't get different
    //  borders, light border at top isn't used anymore
    //  use SC_HDRPAINT_SEL_BOTTOM for different color

    for (sal_uInt16 nPass = SC_HDRPAINT_SEL_BOTTOM; nPass < SC_HDRPAINT_COUNT; nPass++)
    {
        //  set line color etc. before entry loop
        switch ( nPass )
        {
            case SC_HDRPAINT_SEL_BOTTOM:
                // same as non-selected for high contrast
                GetOutDev()->SetLineColor( bHighContrast ? rStyleSettings.GetDarkShadowColor() : aSelLineColor );
                break;
            case SC_HDRPAINT_BOTTOM:
                GetOutDev()->SetLineColor( rStyleSettings.GetDarkShadowColor() );
                break;
            case SC_HDRPAINT_TEXT:
                // DrawSelectionBackground is used only for high contrast on light background
                if ( nTransEnd * nLayoutSign >= nTransStart * nLayoutSign && bHighContrast && !bDark )
                {
                    //  Transparent selection background is drawn after lines, before text.
                    //  Use DrawSelectionBackground to make sure there is a visible
                    //  difference. The case of a dark face color, where DrawSelectionBackground
                    //  would just paint over the lines, is handled separately (bDark).
                    //  Otherwise, GetHighlightColor is used with 80% transparency.
                    //  The window's background color (SetBackground) has to be the background
                    //  of the cell area, for the contrast comparison in DrawSelectionBackground.

                    tools::Rectangle aTransRect;
                    if (bVertical)
                        aTransRect = tools::Rectangle( 0, nTransStart, nBarSize-1, nTransEnd );
                    else
                        aTransRect = tools::Rectangle( nTransStart, 0, nTransEnd, nBarSize-1 );
                    SetBackground( rStyleSettings.GetFaceColor() );
                    DrawSelectionBackground( aTransRect, 0, true, false );
                    SetBackground();
                }
                break;
        }

        SCCOLROW    nCount=0;
        tools::Long    nScrPos=nInitScrPos;
        do
        {
            if (bVertical)
                aScrPos = Point( 0, nScrPos );
            else
                aScrPos = Point( nScrPos, 0 );

            SCCOLROW    nEntryNo = nCount + nPos;
            if ( nEntryNo >= nSize )                // rDoc.MaxCol()/rDoc.MaxRow()
                nScrPos = nPEnd + nLayoutSign;      //  beyond nPEnd -> stop
            else
            {
                sal_uInt16 nSizePix = GetEntrySize( nEntryNo );

                if (nSizePix == 0)
                {
                    SCCOLROW nHidden = GetHiddenCount(nEntryNo);
                    if (nHidden > 0)
                        nCount += nHidden - 1;
                }
                else if ((nScrPos+nSizePix*nLayoutSign)*nLayoutSign >= nPStart*nLayoutSign)
                {
                    Point aEndPos(aScrPos);
                    if (bVertical)
                        aEndPos = Point( aScrPos.X()+nBarSize-1, aScrPos.Y()+(nSizePix-1)*nLayoutSign );
                    else
                        aEndPos = Point( aScrPos.X()+(nSizePix-1)*nLayoutSign, aScrPos.Y()+nBarSize-1 );

                    bool bMark = bMarkRange && nEntryNo >= nMarkStart && nEntryNo <= nMarkEnd;
                    bool bNextToMark = bMarkRange && nEntryNo + 1 >= nMarkStart && nEntryNo <= nMarkEnd;

                    switch ( nPass )
                    {
                        case SC_HDRPAINT_SEL_BOTTOM:
                        case SC_HDRPAINT_BOTTOM:
                            if ( nPass == ( bNextToMark ? SC_HDRPAINT_SEL_BOTTOM : SC_HDRPAINT_BOTTOM ) )
                            {
                                if (bVertical)
                                    aGrid.AddHorLine(/* here we work in pixels */ true, aScrPos.X(), aEndPos.X(), aEndPos.Y());
                                else
                                    aGrid.AddVerLine(/* here we work in pixels */ true, aEndPos.X(), aScrPos.Y(), aEndPos.Y());

                                //  thick bottom for hidden rows
                                //  (drawn directly, without aGrid)
                                if ( nEntryNo+1 < nSize )
                                    if ( GetEntrySize(nEntryNo+1)==0 )
                                    {
                                        if (bVertical)
                                            GetOutDev()->DrawLine( Point(aScrPos.X(),aEndPos.Y()-nLayoutSign),
                                                      Point(aEndPos.X(),aEndPos.Y()-nLayoutSign) );
                                        else
                                            GetOutDev()->DrawLine( Point(aEndPos.X()-nLayoutSign,aScrPos.Y()),
                                                      Point(aEndPos.X()-nLayoutSign,aEndPos.Y()) );
                                    }
                            }
                            break;

                        case SC_HDRPAINT_TEXT:
                            if ( nSizePix > 1 )     // minimal check for small columns/rows
                            {
                                if (bVertical)
                                {
                                    bool bAutoFilterPos = false;
                                    for (const auto& rSpan : aSpans)
                                    {
                                        if (nEntryNo >= rSpan.mnStart && nEntryNo <= rSpan.mnEnd)
                                        {
                                            bAutoFilterPos = true;
                                            break;
                                        }
                                    }

                                    if (bMark != bBoldSet || bAutoFilterPos != bAutoFilterSet)
                                    {
                                        if (bMark)
                                            SetFont(aBoldFont);
                                        else if (bAutoFilterPos)
                                            SetFont(aAutoFilterFont);
                                        else
                                            SetFont(aNormFont);
                                        bBoldSet = bMark;
                                        bAutoFilterSet = bAutoFilterPos && !bMark;
                                    }
                                }
                                else
                                {
                                    if (bMark != bBoldSet)
                                    {
                                        if (bMark)
                                            SetFont(aBoldFont);
                                        else
                                            SetFont(aNormFont);
                                        bBoldSet = bMark;
                                    }
                                }

                                aString = GetEntryText( nEntryNo );
                                aTextSize.setWidth( GetTextWidth( aString ) );
                                aTextSize.setHeight( GetTextHeight() );

                                Point aTxtPos(aScrPos);
                                if (bVertical)
                                {
                                    aTxtPos.AdjustX((nBarSize-aTextSize.Width())/2 );
                                    aTxtPos.AdjustY((nSizePix*nLayoutSign-aTextSize.Height())/2 );
                                    if ( bMirrored )
                                        aTxtPos.AdjustX(1 );   // dark border is left instead of right
                                }
                                else
                                {
                                    aTxtPos.AdjustX((nSizePix*nLayoutSign-aTextSize.Width()+1)/2 );
                                    aTxtPos.AdjustY((nBarSize-aTextSize.Height())/2 );
                                }
                                GetOutDev()->DrawText( aTxtPos, aString );
                            }
                            break;
                    }

                    // when selecting the complete row/column:
                    //  InvertRect( Rectangle( aScrPos, aEndPos ) );
                }
                nScrPos += nSizePix * nLayoutSign;      // also if before the visible area
            }
            ++nCount;
        }
        while ( nScrPos * nLayoutSign <= nPEnd * nLayoutSign );

        aGrid.Flush();
    }
}

SCCOLROW ScHeaderControl::GetMousePos(const Point& rPos, bool& rBorder) const
{
    bool        bFound = false;
    SCCOLROW    nPos = GetPos();
    SCCOLROW    nHitNo = nPos;
    SCCOLROW    nEntryNo = 1 + nPos;
    tools::Long    nScrPos;
    tools::Long    nMousePos = bVertical ? rPos.Y() : rPos.X();
    tools::Long    nDif;
    Size    aSize = GetOutputSizePixel();
    tools::Long    nWinSize = bVertical ? aSize.Height() : aSize.Width();

    bool bLayoutRTL = IsLayoutRTL();
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;
    tools::Long nEndPos = bLayoutRTL ? -1 : nWinSize;

    nScrPos = GetScrPos( nPos ) - nLayoutSign;
    do
    {
        if (nEntryNo > nSize)
            nScrPos = nEndPos + nLayoutSign;
        else
            nScrPos += GetEntrySize( nEntryNo - 1 ) * nLayoutSign;      //! GetHiddenCount() ??

        nDif = nMousePos - nScrPos;
        if (nDif >= -2 && nDif <= 2)
        {
            bFound = true;
            nHitNo=nEntryNo-1;
        }
        else if (nDif * nLayoutSign >= 0 && nEntryNo < nSize)
            nHitNo = nEntryNo;
        ++nEntryNo;
    }
    while ( nScrPos * nLayoutSign < nEndPos * nLayoutSign && nDif * nLayoutSign > 0 );

    rBorder = bFound;
    return nHitNo;
}

bool ScHeaderControl::IsSelectionAllowed(SCCOLROW nPos) const
{
    ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    if (!pViewSh)
        return false;

    ScViewData& rViewData = pViewSh->GetViewData();
    sal_uInt16 nTab = rViewData.GetTabNo();
    ScDocument& rDoc = rViewData.GetDocument();
    const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
    bool bSelectAllowed = true;
    if ( pProtect && pProtect->isProtected() )
    {
        // This sheet is protected.  Check if a context menu is allowed on this cell.
        bool bCellsProtected = false;
        if (bVertical)
        {
            // row header
            SCROW nRPos = static_cast<SCROW>(nPos);
            bCellsProtected = rDoc.HasAttrib(0, nRPos, nTab, rDoc.MaxCol(), nRPos, nTab, HasAttrFlags::Protected);
        }
        else
        {
            // column header
            SCCOL nCPos = static_cast<SCCOL>(nPos);
            bCellsProtected = rDoc.HasAttrib(nCPos, 0, nTab, nCPos, rDoc.MaxRow(), nTab, HasAttrFlags::Protected);
        }

        bool bSelProtected   = pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bool bSelUnprotected = pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);

        if (bCellsProtected)
            bSelectAllowed = bSelProtected;
        else
            bSelectAllowed = bSelUnprotected;
    }
    return bSelectAllowed;
}

void ScHeaderControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (IsDisabled())
        return;

    bIgnoreMove = false;
    SelectWindow();

    bool bIsBorder;
    SCCOLROW nHitNo = GetMousePos(rMEvt.GetPosPixel(), bIsBorder);
    if (!IsSelectionAllowed(nHitNo))
        return;
    if ( ! rMEvt.IsLeft() )
        return;
    if ( SC_MOD()->IsFormulaMode() )
    {
        if( !pTabView )
            return;
        SCTAB nTab = pTabView->GetViewData().GetTabNo();
        if( !rMEvt.IsShift() )
            pTabView->DoneRefMode( rMEvt.IsMod1() );
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
        ScDocument& rDoc = pViewSh->GetViewData().GetDocument();
        if( !bVertical )
        {
            pTabView->InitRefMode( nHitNo, 0, nTab, SC_REFTYPE_REF );
            pTabView->UpdateRef( nHitNo, rDoc.MaxRow(), nTab );
        }
        else
        {
            pTabView->InitRefMode( 0, nHitNo, nTab, SC_REFTYPE_REF );
            pTabView->UpdateRef( rDoc.MaxCol(), nHitNo, nTab );
        }
        bInRefMode = true;
        return;
    }
    if ( bIsBorder && ResizeAllowed() )
    {
        nDragNo = nHitNo;
        sal_uInt16 nClicks = rMEvt.GetClicks();
        if ( nClicks && nClicks%2==0 )
        {
            SetEntrySize( nDragNo, HDR_SIZE_OPTIMUM );
            SetPointer( PointerStyle::Arrow );
        }
        else
        {
            if (bVertical)
                nDragStart = rMEvt.GetPosPixel().Y();
            else
                nDragStart = rMEvt.GetPosPixel().X();
            nDragPos = nDragStart;
            // tdf#140833 launch help tip to show after the double click time has expired
            // so under gtk the popover isn't active when the double click is processed
            // by gtk because under load on wayland the double click is getting handled
            // by something else and getting sent to the window underneath our window
            aShowHelpTimer.Start();
            DrawInvert( nDragPos );

            StartTracking();
            bDragging = true;
            bDragMoved = false;
        }
    }
    else
    {
        pSelEngine->SetWindow( this );
        tools::Rectangle aVis( Point(), GetOutputSizePixel() );
        if (bVertical)
        {
            aVis.SetLeft( LONG_MIN );
            aVis.SetRight( LONG_MAX );
        }
        else
        {
            aVis.SetTop( LONG_MIN );
            aVis.SetBottom( LONG_MAX );
        }
        pSelEngine->SetVisibleArea( aVis );

        SetMarking( true );     //  must precede SelMouseButtonDown
        pSelEngine->SelMouseButtonDown( rMEvt );

        //  In column/row headers a simple click already is a selection.
        //  -> Call SelMouseMove to ensure CreateAnchor is called (and DestroyAnchor
        //  if the next click is somewhere else with Control key).
        pSelEngine->SelMouseMove( rMEvt );

        if (IsMouseCaptured())
        {
            // tracking instead of CaptureMouse, so it can be cancelled cleanly
            //! someday SelectionEngine itself should call StartTracking!?!
            ReleaseMouse();
            StartTracking();
        }
    }
}

void ScHeaderControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( IsDisabled() )
        return;

    if ( SC_MOD()->IsFormulaMode() )
    {
        SC_MOD()->EndReference();
        bInRefMode = false;
        return;
    }

    SetMarking( false );
    bIgnoreMove = false;

    if ( bDragging )
    {
        DrawInvert( nDragPos );
        ReleaseMouse();
        HideDragHelp();
        bDragging = false;

        tools::Long nScrPos    = GetScrPos( nDragNo );
        tools::Long nMousePos  = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
        bool bLayoutRTL = IsLayoutRTL();
        tools::Long nNewWidth  = bLayoutRTL ? ( nScrPos - nMousePos + 1 )
                                     : ( nMousePos + 2 - nScrPos );

        if ( nNewWidth < 0 /* && !IsSelected(nDragNo) */ )
        {
            SCCOLROW nStart = 0;
            SCCOLROW nEnd = nDragNo;
            while (nNewWidth < 0)
            {
                nStart = nDragNo;
                if (nDragNo>0)
                {
                    --nDragNo;
                    nNewWidth += GetEntrySize( nDragNo );   //! GetHiddenCount() ???
                }
                else
                    nNewWidth = 0;
            }
            HideEntries( nStart, nEnd );
        }
        else
        {
            if (bDragMoved)
                SetEntrySize( nDragNo, static_cast<sal_uInt16>(nNewWidth) );
        }
    }
    else
    {
        pSelEngine->SelMouseButtonUp( rMEvt );
        ReleaseMouse();
    }
}

void ScHeaderControl::MouseMove( const MouseEvent& rMEvt )
{
    if ( IsDisabled() )
    {
        SetPointer( PointerStyle::Arrow );
        return;
    }

    if ( bInRefMode && rMEvt.IsLeft() && SC_MOD()->IsFormulaMode() )
    {
        if( !pTabView )
            return;
        bool bTmp;
        SCCOLROW nHitNo = GetMousePos(rMEvt.GetPosPixel(), bTmp);
        SCTAB nTab = pTabView->GetViewData().GetTabNo();
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
        ScDocument& rDoc = pViewSh->GetViewData().GetDocument();
        if( !bVertical )
            pTabView->UpdateRef( nHitNo, rDoc.MaxRow(), nTab );
        else
            pTabView->UpdateRef( rDoc.MaxCol(), nHitNo, nTab );

        return;
    }

    if ( bDragging )
    {
        tools::Long nNewPos = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
        if ( nNewPos != nDragPos )
        {
            DrawInvert( nDragPos );
            nDragPos = nNewPos;
            ShowDragHelp();
            DrawInvert( nDragPos );

            if (nDragPos <= nDragStart-SC_DRAG_MIN || nDragPos >= nDragStart+SC_DRAG_MIN)
                bDragMoved = true;
        }
    }
    else
    {
        bool bIsBorder;
        (void)GetMousePos(rMEvt.GetPosPixel(), bIsBorder);

        if ( bIsBorder && rMEvt.GetButtons()==0 && ResizeAllowed() )
            SetPointer( bVertical ? PointerStyle::VSizeBar : PointerStyle::HSizeBar );
        else
            SetPointer( PointerStyle::Arrow );

        if (!bIgnoreMove)
            pSelEngine->SelMouseMove( rMEvt );
    }
}

void ScHeaderControl::Tracking( const TrackingEvent& rTEvt )
{
    // Distribute the tracking events to the various MouseEvents, because
    // SelectionEngine does not know anything about Tracking

    if ( rTEvt.IsTrackingCanceled() )
        StopMarking();
    else if ( rTEvt.IsTrackingEnded() )
        MouseButtonUp( rTEvt.GetMouseEvent() );
    else
        MouseMove( rTEvt.GetMouseEvent() );
}

void ScHeaderControl::Command( const CommandEvent& rCEvt )
{
    CommandEventId nCmd = rCEvt.GetCommand();
    if ( nCmd == CommandEventId::ContextMenu )
    {
        StopMarking();      // finish selection / dragging

        // execute popup menu

        ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( SfxViewShell::Current() );
        if ( pViewSh )
        {
            if ( rCEvt.IsMouseEvent() )
            {
                // #i18735# select the column/row under the mouse pointer
                ScViewData& rViewData = pViewSh->GetViewData();

                SelectWindow();     // also deselects drawing objects, stops draw text edit
                if ( rViewData.HasEditView( rViewData.GetActivePart() ) )
                    SC_MOD()->InputEnterHandler();  // always end edit mode

                bool bBorder;
                SCCOLROW nPos = GetMousePos(rCEvt.GetMousePosPixel(), bBorder );
                if (!IsSelectionAllowed(nPos))
                    // Selecting this cell is not allowed, neither is context menu.
                    return;

                SCTAB nTab = rViewData.GetTabNo();
                ScDocument& rDoc = pViewSh->GetViewData().GetDocument();
                ScRange aNewRange;
                if ( bVertical )
                    aNewRange = ScRange( 0, sal::static_int_cast<SCROW>(nPos), nTab,
                                         rDoc.MaxCol(), sal::static_int_cast<SCROW>(nPos), nTab );
                else
                    aNewRange = ScRange( sal::static_int_cast<SCCOL>(nPos), 0, nTab,
                                         sal::static_int_cast<SCCOL>(nPos), rDoc.MaxRow(), nTab );

                // see if any part of the range is already selected
                ScRangeList aRanges;
                rViewData.GetMarkData().FillRangeListWithMarks( &aRanges, false );
                bool bSelected = aRanges.Intersects(aNewRange);

                // select the range if no part of it was selected
                if ( !bSelected )
                    pViewSh->MarkRange( aNewRange );
            }

            pViewSh->GetDispatcher()->ExecutePopup( bVertical ? OUString( "rowheader" ) : OUString( "colheader" ) );
        }
    }
    else if ( nCmd == CommandEventId::StartDrag )
    {
        pSelEngine->Command( rCEvt );
    }
}

void ScHeaderControl::StopMarking()
{
    if ( bDragging )
    {
        DrawInvert( nDragPos );
        HideDragHelp();
        bDragging = false;
    }

    SetMarking( false );
    bIgnoreMove = true;

    //  don't call pSelEngine->Reset, so selection across the parts of
    //  a split/frozen view is possible
    if (IsMouseCaptured())
        ReleaseMouse();
}

IMPL_LINK_NOARG(ScHeaderControl, ShowDragHelpHdl, Timer*, void)
{
    ShowDragHelp();
}

void ScHeaderControl::ShowDragHelp()
{
    aShowHelpTimer.Stop();
    if (!Help::IsQuickHelpEnabled())
        return;

    tools::Long nScrPos    = GetScrPos( nDragNo );
    bool bLayoutRTL = IsLayoutRTL();
    tools::Long nVal = bLayoutRTL ? ( nScrPos - nDragPos + 1 )
                           : ( nDragPos + 2 - nScrPos );

    OUString aHelpStr = GetDragHelp( nVal );
    Point aPos = OutputToScreenPixel( Point(0,0) );
    Size aSize = GetSizePixel();

    Point aMousePos = OutputToScreenPixel(GetPointerPosPixel());

    tools::Rectangle aRect;
    QuickHelpFlags nAlign;
    if (!bVertical)
    {
        // above
        aRect.SetLeft( aMousePos.X() );
        aRect.SetTop( aPos.Y() - 4 );
        nAlign       = QuickHelpFlags::Bottom|QuickHelpFlags::Center;
    }
    else
    {
        // top right
        aRect.SetLeft( aPos.X() + aSize.Width() + 8 );
        aRect.SetTop( aMousePos.Y() - 2 );
        nAlign       = QuickHelpFlags::Left|QuickHelpFlags::Bottom;
    }

    aRect.SetRight( aRect.Left() );
    aRect.SetBottom( aRect.Top() );

    if (nTipVisible)
        Help::HidePopover(this, nTipVisible);
    nTipVisible = Help::ShowPopover(this, aRect, aHelpStr, nAlign);
}

void ScHeaderControl::HideDragHelp()
{
    aShowHelpTimer.Stop();
    if (nTipVisible)
    {
        Help::HidePopover(this, nTipVisible);
        nTipVisible = nullptr;
    }
}

void ScHeaderControl::RequestHelp( const HelpEvent& rHEvt )
{
    //  If the own QuickHelp is displayed, don't let RequestHelp remove it

    bool bOwn = bDragging && Help::IsQuickHelpEnabled();
    if (!bOwn)
        Window::RequestHelp(rHEvt);
}

//                  dummies for virtual methods

SCCOLROW ScHeaderControl::GetHiddenCount( SCCOLROW nEntryNo ) const
{
    SCCOLROW nHidden = 0;
    while ( nEntryNo < nSize && GetEntrySize( nEntryNo ) == 0 )
    {
        ++nEntryNo;
        ++nHidden;
    }
    return nHidden;
}

bool ScHeaderControl::IsLayoutRTL() const
{
    return false;
}

bool ScHeaderControl::IsMirrored() const
{
    return false;
}

bool ScHeaderControl::IsDisabled() const
{
    return false;
}

bool ScHeaderControl::ResizeAllowed() const
{
    return true;
}

void ScHeaderControl::SelectWindow()
{
}

void ScHeaderControl::DrawInvert( tools::Long /* nDragPos */ )
{
}

OUString ScHeaderControl::GetDragHelp( tools::Long /* nVal */ )
{
    return OUString();
}

void ScHeaderControl::SetMarking( bool /* bSet */ )
{
}

void ScHeaderControl::GetMarkRange(SCCOLROW& rStart, SCCOLROW& rEnd) const
{
    rStart = nMarkStart;
    rEnd = nMarkEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
