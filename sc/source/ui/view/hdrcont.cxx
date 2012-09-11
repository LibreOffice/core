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

#include <sfx2/dispatch.hxx>
#include <vcl/help.hxx>
#include <tools/poly.hxx>
#include <svtools/colorcfg.hxx>

#include "scresid.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "hdrcont.hxx"
#include "scmod.hxx"        // Optionen
#include "inputopt.hxx"     // Optionen
#include "gridmerg.hxx"
#include "document.hxx"
#include "markdata.hxx"

// -----------------------------------------------------------------------

#define SC_DRAG_MIN     2

//  passes in paint
//  (selection left/right must be first because the continuous lines
//  are partly overwritten later)

#define SC_HDRPAINT_SEL_RIGHT   0
#define SC_HDRPAINT_SEL_LEFT    1
#define SC_HDRPAINT_TOP         2
#define SC_HDRPAINT_SEL_TOP     3
#define SC_HDRPAINT_SEL_BOTTOM  4
#define SC_HDRPAINT_BOTTOM      5
#define SC_HDRPAINT_TEXT        6
#define SC_HDRPAINT_COUNT       7

//==================================================================

ScHeaderControl::ScHeaderControl( Window* pParent, SelectionEngine* pSelectionEngine,
                                    SCCOLROW nNewSize, bool bNewVertical ) :
            Window      ( pParent ),
            pSelEngine  ( pSelectionEngine ),
            bVertical   ( bNewVertical ),
            nSize       ( nNewSize ),
            nMarkStart  ( 0 ),
            nMarkEnd    ( 0 ),
            bMarkRange  ( false ),
            bDragging   ( false ),
            bIgnoreMove ( false )
{
    // --- RTL --- no default mirroring for this window, the spreadsheet itself
    // is also not mirrored
    // mirror the vertical window for correct border drawing
    // table layout depends on sheet format, not UI setting, so the
    // borders of the vertical window have to be handled manually, too.
    EnableRTL( false );

    aNormFont = GetFont();
    aNormFont.SetTransparent( sal_True );       //! WEIGHT_NORMAL hart setzen ???
    aBoldFont = aNormFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );

    SetFont(aBoldFont);
    bBoldSet = sal_True;

    Size aSize = LogicToPixel( Size(
        GetTextWidth(rtl::OUString("8888")),
        GetTextHeight() ) );
    aSize.Width()  += 4;    // Platz fuer hervorgehobene Umrandung
    aSize.Height() += 3;
    SetSizePixel( aSize );

    nWidth = nSmallWidth = aSize.Width();
    nBigWidth = LogicToPixel( Size( GetTextWidth(rtl::OUString("8888888")), 0 ) ).Width() + 5;

    SetBackground();    // sonst Probleme auf OS/2 !?!?!
}

void ScHeaderControl::SetWidth( long nNew )
{
    OSL_ENSURE( bVertical, "SetDigits nur fuer Zeilenkoepfe erlaubt" );
    if ( nNew != nWidth )
    {
        Size aSize( nNew, GetSizePixel().Height() );    // Hoehe nicht aendern
        SetSizePixel( aSize );

        nWidth = nNew;

        Invalidate();       // neu zentrieren
    }
}

ScHeaderControl::~ScHeaderControl()
{
}

void ScHeaderControl::DoPaint( SCCOLROW nStart, SCCOLROW nEnd )
{
    sal_Bool bLayoutRTL = IsLayoutRTL();
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    Rectangle aRect( Point(0,0), GetOutputSizePixel() );
    if ( bVertical )
    {
        aRect.Top() = GetScrPos( nStart )-nLayoutSign;      // extra pixel for line at top of selection
        aRect.Bottom() = GetScrPos( nEnd+1 )-nLayoutSign;
    }
    else
    {
        aRect.Left() = GetScrPos( nStart )-nLayoutSign;     // extra pixel for line left of selection
        aRect.Right() = GetScrPos( nEnd+1 )-nLayoutSign;
    }
    Invalidate(aRect);
}

void ScHeaderControl::SetMark( sal_Bool bNewSet, SCCOLROW nNewStart, SCCOLROW nNewEnd )
{
    sal_Bool bEnabled = SC_MOD()->GetInputOptions().GetMarkHeader();    //! cachen?
    if (!bEnabled)
        bNewSet = false;

    //  Variablen setzen

    sal_Bool bOldSet     = bMarkRange;
    SCCOLROW nOldStart = nMarkStart;
    SCCOLROW nOldEnd     = nMarkEnd;
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
                    DoPaint( Min( nNewEnd, nOldEnd ) + 1, Max( nNewEnd, nOldEnd ) );
                // sonst nix
            }
            else if ( nNewEnd == nOldEnd )
                DoPaint( Min( nNewStart, nOldStart ), Max( nNewStart, nOldStart ) - 1 );
            else if ( nNewStart > nOldEnd || nNewEnd < nOldStart )
            {
                //  zwei Bereiche...
                DoPaint( nOldStart, nOldEnd );
                DoPaint( nNewStart, nNewEnd );
            }
            else                //  irgendwie ueberlappend... (kommt eh nicht oft vor)
                DoPaint( Min( nNewStart, nOldStart ), Max( nNewEnd, nOldEnd ) );
        }
        else
            DoPaint( nNewStart, nNewEnd );      //  komplett neu
    }
    else if ( bOldSet )
        DoPaint( nOldStart, nOldEnd );          //  komplett aufheben

    //  sonst war nix, is nix
}

long ScHeaderControl::GetScrPos( SCCOLROW nEntryNo )
{
    long nScrPos;

    long nMax = ( bVertical ? GetOutputSizePixel().Height() : GetOutputSizePixel().Width() ) + 1;
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

// draw a rectangle across the window's width/height, with the outer part in a lighter color

void ScHeaderControl::DrawShadedRect( long nStart, long nEnd, const Color& rBaseColor )
{
    Color aWhite( COL_WHITE );

    Color aInner( rBaseColor );             // highlight color, unchanged
    Color aCenter( rBaseColor );
    aCenter.Merge( aWhite, 0xd0 );          // lighten up a bit
    Color aOuter( rBaseColor );
    aOuter.Merge( aWhite, 0xa0 );           // lighten up more

    if ( IsMirrored() )
        std::swap( aInner, aOuter );        // just swap colors instead of positions

    Size aWinSize = GetSizePixel();
    long nBarSize = bVertical ? aWinSize.Width() : aWinSize.Height();
    long nCenterPos = (nBarSize / 2) - 1;

    SetLineColor();
    SetFillColor( aOuter );
    if (bVertical)
        DrawRect( Rectangle( 0, nStart, nCenterPos-1, nEnd ) );
    else
        DrawRect( Rectangle( nStart, 0, nEnd, nCenterPos-1 ) );
    SetFillColor( aCenter );
    if (bVertical)
        DrawRect( Rectangle( nCenterPos, nStart, nCenterPos, nEnd ) );
    else
        DrawRect( Rectangle( nStart, nCenterPos, nEnd, nCenterPos ) );
    SetFillColor( aInner );
    if (bVertical)
        DrawRect( Rectangle( nCenterPos+1, nStart, nBarSize-1, nEnd ) );
    else
        DrawRect( Rectangle( nStart, nCenterPos+1, nEnd, nBarSize-1 ) );
}

//
//      Paint
//

void ScHeaderControl::Paint( const Rectangle& rRect )
{
    //  fuer VCL ist es wichtig, wenig Aufrufe zu haben, darum werden die aeusseren
    //  Linien zusammengefasst

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    sal_Bool bHighContrast = rStyleSettings.GetHighContrastMode();
    sal_Bool bDark = rStyleSettings.GetFaceColor().IsDark();
    // Use the same distinction for bDark as in Window::DrawSelectionBackground

    Color aTextColor = rStyleSettings.GetButtonTextColor();
    Color aSelTextColor = rStyleSettings.GetHighlightTextColor();
    aNormFont.SetColor( aTextColor );
    if ( bHighContrast )
        aBoldFont.SetColor( aTextColor );
    else
        aBoldFont.SetColor( aSelTextColor );
    SetTextColor( ( bBoldSet && !bHighContrast ) ? aSelTextColor : aTextColor );

    Color aBlack( COL_BLACK );
    Color aSelLineColor = rStyleSettings.GetHighlightColor();
    aSelLineColor.Merge( aBlack, 0xe0 );        // darken just a little bit

    sal_Bool bLayoutRTL = IsLayoutRTL();
    long nLayoutSign = bLayoutRTL ? -1 : 1;
    sal_Bool bMirrored = IsMirrored();

    String              aString;
    sal_uInt16              nBarSize;
    Point               aScrPos;
    Size                aTextSize;

    if (bVertical)
        nBarSize = (sal_uInt16) GetSizePixel().Width();
    else
        nBarSize = (sal_uInt16) GetSizePixel().Height();

    SCCOLROW    nPos = GetPos();

    long nPStart = bVertical ? rRect.Top() : rRect.Left();
    long nPEnd = bVertical ? rRect.Bottom() : rRect.Right();

    long nTransStart = nPEnd + 1;
    long nTransEnd = 0;

    long nInitScrPos = 0;
    if ( bLayoutRTL )
    {
        long nTemp = nPStart;       // swap nPStart / nPEnd
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

    //  aeussere Linien komplett durchzeichnen
    //  Zuerst Ende der letzten Zelle finden

    long nLineEnd = nInitScrPos - nLayoutSign;

    for (SCCOLROW i=nPos; i<nSize; i++)
    {
        sal_uInt16 nSizePix = GetEntrySize( i );
        if (nSizePix)
        {
            nLineEnd += nSizePix * nLayoutSign;

            if ( bMarkRange && i >= nMarkStart && i <= nMarkEnd )
            {
                long nLineStart = nLineEnd - ( nSizePix - 1 ) * nLayoutSign;
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

    Rectangle aFillRect;
    SetLineColor();

    if ( nLineEnd * nLayoutSign >= nInitScrPos * nLayoutSign )
    {
        if ( bHighContrast )
        {
            // high contrast: single-color background
            SetFillColor( rStyleSettings.GetFaceColor() );
            if ( bVertical )
                aFillRect = Rectangle( 0, nInitScrPos, nBarSize-1, nLineEnd );
            else
                aFillRect = Rectangle( nInitScrPos, 0, nLineEnd, nBarSize-1 );
            DrawRect( aFillRect );
        }
        else
        {
            // normal: 3-part background
            DrawShadedRect( nInitScrPos, nLineEnd, rStyleSettings.GetFaceColor() );
        }
    }

    if ( nLineEnd * nLayoutSign < nPEnd * nLayoutSign )
    {
        SetFillColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::APPBACKGROUND).nColor );
        if ( bVertical )
            aFillRect = Rectangle( 0, nLineEnd+nLayoutSign, nBarSize-1, nPEnd );
        else
            aFillRect = Rectangle( nLineEnd+nLayoutSign, 0, nPEnd, nBarSize-1 );
        DrawRect( aFillRect );
    }

    if ( nLineEnd * nLayoutSign >= nPStart * nLayoutSign )
    {
        if ( nTransEnd * nLayoutSign >= nTransStart * nLayoutSign )
        {
            if ( bHighContrast )
            {
                if ( bDark )
                {
                    //  solid grey background for dark face color is drawn before lines

                    SetLineColor();
                    SetFillColor( COL_LIGHTGRAY );
                    if (bVertical)
                        DrawRect( Rectangle( 0, nTransStart, nBarSize-1, nTransEnd ) );
                    else
                        DrawRect( Rectangle( nTransStart, 0, nTransEnd, nBarSize-1 ) );
                }
            }
            else
            {
                // background for selection

                DrawShadedRect( nTransStart, nTransEnd, rStyleSettings.GetHighlightColor() );
            }
        }

        SetLineColor( rStyleSettings.GetDarkShadowColor() );
        if (bVertical)
        {
            long nDarkPos = bMirrored ? 0 : nBarSize-1;
            DrawLine( Point( nDarkPos, nPStart ), Point( nDarkPos, nLineEnd ) );
        }
        else
            DrawLine( Point( nPStart, nBarSize-1 ), Point( nLineEnd, nBarSize-1 ) );

        // line in different color for selection
        if ( nTransEnd * nLayoutSign >= nTransStart * nLayoutSign && !bHighContrast )
        {
            SetLineColor( aSelLineColor );
            if (bVertical)
            {
                long nDarkPos = bMirrored ? 0 : nBarSize-1;
                DrawLine( Point( nDarkPos, nTransStart ), Point( nDarkPos, nTransEnd ) );
            }
            else
                DrawLine( Point( nTransStart, nBarSize-1 ), Point( nTransEnd, nBarSize-1 ) );
        }
    }

    //
    //  loop through entries several times to avoid changing the line color too often
    //  and to allow merging of lines
    //

    ScGridMerger aGrid( this, 1, 1 );

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
                SetLineColor( bHighContrast ? rStyleSettings.GetDarkShadowColor() : aSelLineColor );
                break;
            case SC_HDRPAINT_BOTTOM:
                SetLineColor( rStyleSettings.GetDarkShadowColor() );
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

                    Rectangle aTransRect;
                    if (bVertical)
                        aTransRect = Rectangle( 0, nTransStart, nBarSize-1, nTransEnd );
                    else
                        aTransRect = Rectangle( nTransStart, 0, nTransEnd, nBarSize-1 );
                    SetBackground( Color( rStyleSettings.GetFaceColor() ) );
                    DrawSelectionBackground( aTransRect, 0, sal_True, false, false );
                    SetBackground();
                }
                break;
        }

        SCCOLROW    nCount=0;
        long    nScrPos=nInitScrPos;
        do
        {
            if (bVertical)
                aScrPos = Point( 0, nScrPos );
            else
                aScrPos = Point( nScrPos, 0 );

            SCCOLROW    nEntryNo = nCount + nPos;
            if ( nEntryNo >= nSize )                // MAXCOL/MAXROW
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

                    sal_Bool bMark = bMarkRange && nEntryNo >= nMarkStart && nEntryNo <= nMarkEnd;
                    sal_Bool bNextToMark = bMarkRange && nEntryNo + 1 >= nMarkStart && nEntryNo <= nMarkEnd;

                    switch ( nPass )
                    {
                        case SC_HDRPAINT_SEL_BOTTOM:
                        case SC_HDRPAINT_BOTTOM:
                            if ( nPass == ( bNextToMark ? SC_HDRPAINT_SEL_BOTTOM : SC_HDRPAINT_BOTTOM ) )
                            {
                                if (bVertical)
                                    aGrid.AddHorLine( aScrPos.X(), aEndPos.X(), aEndPos.Y() );
                                else
                                    aGrid.AddVerLine( aEndPos.X(), aScrPos.Y(), aEndPos.Y() );

                                //  thick bottom for hidden rows
                                //  (drawn directly, without aGrid)
                                if ( nEntryNo+1 < nSize )
                                    if ( GetEntrySize(nEntryNo+1)==0 )
                                    {
                                        if (bVertical)
                                            DrawLine( Point(aScrPos.X(),aEndPos.Y()-nLayoutSign),
                                                      Point(aEndPos.X(),aEndPos.Y()-nLayoutSign) );
                                        else
                                            DrawLine( Point(aEndPos.X()-nLayoutSign,aScrPos.Y()),
                                                      Point(aEndPos.X()-nLayoutSign,aEndPos.Y()) );
                                    }
                            }
                            break;

                        case SC_HDRPAINT_TEXT:
                            if ( nSizePix > 1 )     // minimal check for small columns/rows
                            {
                                if ( bMark != bBoldSet )
                                {
                                    if (bMark)
                                        SetFont(aBoldFont);
                                    else
                                        SetFont(aNormFont);
                                    bBoldSet = bMark;
                                }
                                aString = GetEntryText( nEntryNo );
                                aTextSize.Width() = GetTextWidth( aString );
                                aTextSize.Height() = GetTextHeight();

                                Point aTxtPos(aScrPos);
                                if (bVertical)
                                {
                                    aTxtPos.X() += (nBarSize-aTextSize.Width())/2;
                                    aTxtPos.Y() += (nSizePix*nLayoutSign-aTextSize.Height())/2;
                                    if ( bMirrored )
                                        aTxtPos.X() += 1;   // dark border is left instead of right
                                }
                                else
                                {
                                    aTxtPos.X() += (nSizePix*nLayoutSign-aTextSize.Width()+1)/2;
                                    aTxtPos.Y() += (nBarSize-aTextSize.Height())/2;
                                }
                                DrawText( aTxtPos, aString );
                            }
                            break;
                    }

                    //  bei Selektion der ganzen Zeile/Spalte:
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

//
//      Maus - Handling
//

SCCOLROW ScHeaderControl::GetMousePos( const MouseEvent& rMEvt, sal_Bool& rBorder )
{
    sal_Bool    bFound=false;
    SCCOLROW    nCount = 1;
    SCCOLROW    nPos = GetPos();
    SCCOLROW    nHitNo = nPos;
    long    nScrPos;
    long    nMousePos = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
    long    nDif;
    Size    aSize = GetOutputSizePixel();
    long    nWinSize = bVertical ? aSize.Height() : aSize.Width();

    sal_Bool bLayoutRTL = IsLayoutRTL();
    long nLayoutSign = bLayoutRTL ? -1 : 1;
    long nEndPos = bLayoutRTL ? -1 : nWinSize;

    nScrPos = GetScrPos( nPos ) - nLayoutSign;
    do
    {
        SCCOLROW nEntryNo = nCount + nPos;

        if (nEntryNo > nSize)
            nScrPos = nEndPos + nLayoutSign;
        else
            nScrPos += GetEntrySize( nEntryNo - 1 ) * nLayoutSign;      //! GetHiddenCount() ??

        nDif = nMousePos - nScrPos;
        if (nDif >= -2 && nDif <= 2 && nCount > 0)
        {
            bFound=sal_True;
            nHitNo=nEntryNo-1;
        }
        else if (nDif * nLayoutSign >= 0 && nEntryNo < nSize)
            nHitNo = nEntryNo;
        ++nCount;
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

    ScViewData* pViewData = pViewSh->GetViewData();
    sal_uInt16 nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    const ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
    bool bSelectAllowed = true;
    if ( pProtect && pProtect->isProtected() )
    {
        // This sheet is protected.  Check if a context menu is allowed on this cell.
        bool bCellsProtected = false;
        if (bVertical)
        {
            // row header
            SCROW nRPos = static_cast<SCROW>(nPos);
            bCellsProtected = pDoc->HasAttrib(0, nRPos, nTab, MAXCOL, nRPos, nTab, HASATTR_PROTECTED);
        }
        else
        {
            // column header
            SCCOL nCPos = static_cast<SCCOL>(nPos);
            bCellsProtected = pDoc->HasAttrib(nCPos, 0, nTab, nCPos, MAXROW, nTab, HASATTR_PROTECTED);
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

    sal_Bool bFound;
    SCCOLROW nHitNo = GetMousePos( rMEvt, bFound );
    if (!IsSelectionAllowed(nHitNo))
        return;

    if ( bFound && rMEvt.IsLeft() && ResizeAllowed() )
    {
        nDragNo = nHitNo;
        sal_uInt16 nClicks = rMEvt.GetClicks();
        if ( nClicks && nClicks%2==0 )
        {
            SetEntrySize( nDragNo, HDR_SIZE_OPTIMUM );
            SetPointer( Pointer( POINTER_ARROW ) );
        }
        else
        {
            if (bVertical)
                nDragStart = rMEvt.GetPosPixel().Y();
            else
                nDragStart = rMEvt.GetPosPixel().X();
            nDragPos = nDragStart;
            ShowDragHelp();
            DrawInvert( nDragPos );

            StartTracking();
            bDragging = sal_True;
            bDragMoved = false;
        }
    }
    else if (rMEvt.IsLeft())
    {
        pSelEngine->SetWindow( this );
        Point aPoint;
        Rectangle aVis( aPoint,GetOutputSizePixel() );
        if (bVertical)
            aVis.Left() = LONG_MIN, aVis.Right() = LONG_MAX;
        else
            aVis.Top() = LONG_MIN, aVis.Bottom() = LONG_MAX;
        pSelEngine->SetVisibleArea( aVis );

        SetMarking( sal_True );     //  muss vor SelMouseButtonDown sein
        pSelEngine->SelMouseButtonDown( rMEvt );

        //  In column/row headers a simple click already is a selection.
        //  -> Call SelMouseMove to ensure CreateAnchor is called (and DestroyAnchor
        //  if the next click is somewhere else with Control key).
        pSelEngine->SelMouseMove( rMEvt );

        if (IsMouseCaptured())
        {
            //  Tracking statt CaptureMouse, damit sauber abgebrochen werden kann
            //! Irgendwann sollte die SelectionEngine selber StartTracking rufen!?!
            ReleaseMouse();
            StartTracking();
        }
    }
}

void ScHeaderControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( IsDisabled() )
        return;

    SetMarking( false );
    bIgnoreMove = false;

    if ( bDragging )
    {
        DrawInvert( nDragPos );
        ReleaseMouse();
        bDragging   = false;

        long nScrPos    = GetScrPos( nDragNo );
        long nMousePos  = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
        sal_Bool bLayoutRTL = IsLayoutRTL();
        long nNewWidth  = bLayoutRTL ? ( nScrPos - nMousePos + 1 )
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
            if (nNewWidth<0) nNewWidth=0;
            if (bDragMoved)
                SetEntrySize( nDragNo, (sal_uInt16) nNewWidth );
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
        SetPointer( Pointer( POINTER_ARROW ) );
        return;
    }

    sal_Bool bFound;
    (void)GetMousePos( rMEvt, bFound );

    if ( bDragging )
    {
        long nNewPos = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
        if ( nNewPos != nDragPos )
        {
            DrawInvert( nDragPos );
            nDragPos = nNewPos;
            ShowDragHelp();
            DrawInvert( nDragPos );

            if (nDragPos <= nDragStart-SC_DRAG_MIN || nDragPos >= nDragStart+SC_DRAG_MIN)
                bDragMoved = sal_True;
        }
    }
    else
    {
        if ( bFound && rMEvt.GetButtons()==0 && ResizeAllowed() )
            SetPointer( Pointer( bVertical ? POINTER_VSIZEBAR : POINTER_HSIZEBAR ) );
        else
            SetPointer( Pointer( POINTER_ARROW ) );

        if (!bIgnoreMove)
            pSelEngine->SelMouseMove( rMEvt );
    }
}

void ScHeaderControl::Tracking( const TrackingEvent& rTEvt )
{
    //  Weil die SelectionEngine kein Tracking kennt, die Events nur auf
    //  die verschiedenen MouseHandler verteilen...

    if ( rTEvt.IsTrackingCanceled() )
        StopMarking();
    else if ( rTEvt.IsTrackingEnded() )
        MouseButtonUp( rTEvt.GetMouseEvent() );
    else
        MouseMove( rTEvt.GetMouseEvent() );
}

void ScHeaderControl::Command( const CommandEvent& rCEvt )
{
    sal_uInt16 nCmd = rCEvt.GetCommand();
    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        StopMarking();      // Selektion / Dragging beenden

        //  Popup ausfuehren

        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell,
                                            SfxViewShell::Current() );
        if ( pViewSh )
        {
            if ( rCEvt.IsMouseEvent() )
            {
                // #i18735# select the column/row under the mouse pointer
                ScViewData* pViewData = pViewSh->GetViewData();

                SelectWindow();     // also deselects drawing objects, stops draw text edit
                if ( pViewData->HasEditView( pViewData->GetActivePart() ) )
                    SC_MOD()->InputEnterHandler();  // always end edit mode

                MouseEvent aMEvt( rCEvt.GetMousePosPixel() );
                sal_Bool bBorder;
                SCCOLROW nPos = GetMousePos( aMEvt, bBorder );
                if (!IsSelectionAllowed(nPos))
                    // Selecting this cell is not allowed, neither is context menu.
                    return;

                SCTAB nTab = pViewData->GetTabNo();
                ScRange aNewRange;
                if ( bVertical )
                    aNewRange = ScRange( 0, sal::static_int_cast<SCROW>(nPos), nTab,
                                         MAXCOL, sal::static_int_cast<SCROW>(nPos), nTab );
                else
                    aNewRange = ScRange( sal::static_int_cast<SCCOL>(nPos), 0, nTab,
                                         sal::static_int_cast<SCCOL>(nPos), MAXROW, nTab );

                // see if any part of the range is already selected
                ScRangeList aRanges;
                pViewData->GetMarkData().FillRangeListWithMarks( &aRanges, false );
                bool bSelected = aRanges.Intersects(aNewRange);

                // select the range if no part of it was selected
                if ( !bSelected )
                    pViewSh->MarkRange( aNewRange );
            }

            ScResId aResId( bVertical ? RID_POPUP_ROWHEADER : RID_POPUP_COLHEADER );
            pViewSh->GetDispatcher()->ExecutePopup( aResId );
        }
    }
    else if ( nCmd == COMMAND_STARTDRAG )
    {
        pSelEngine->Command( rCEvt );
    }
}

void ScHeaderControl::StopMarking()
{
    if ( bDragging )
    {
        DrawInvert( nDragPos );
        bDragging = false;
    }

    SetMarking( false );
    bIgnoreMove = sal_True;

    //  don't call pSelEngine->Reset, so selection across the parts of
    //  a split/frozen view is possible

    ReleaseMouse();
}

void ScHeaderControl::ShowDragHelp()
{
    if (Help::IsQuickHelpEnabled())
    {
        long nScrPos    = GetScrPos( nDragNo );
        sal_Bool bLayoutRTL = IsLayoutRTL();
        long nVal = bLayoutRTL ? ( nScrPos - nDragPos + 1 )
                               : ( nDragPos + 2 - nScrPos );

        String aHelpStr = GetDragHelp( nVal );
        Point aPos = OutputToScreenPixel( Point(0,0) );
        Size aSize = GetSizePixel();

        Point aMousePos = OutputToScreenPixel(GetPointerPosPixel());

        Rectangle aRect;
        sal_uInt16 nAlign;
        if (!bVertical)
        {
            //  oberhalb
            aRect.Left() = aMousePos.X();
            aRect.Top()  = aPos.Y() - 4;
            nAlign       = QUICKHELP_BOTTOM|QUICKHELP_CENTER;
        }
        else
        {
            //  rechts oben
            aRect.Left() = aPos.X() + aSize.Width() + 8;
            aRect.Top()  = aMousePos.Y() - 2;
            nAlign       = QUICKHELP_LEFT|QUICKHELP_BOTTOM;
        }

        aRect.Right()   = aRect.Left();
        aRect.Bottom()  = aRect.Top();

        Help::ShowQuickHelp(this, aRect, aHelpStr, nAlign);
    }
}

void ScHeaderControl::RequestHelp( const HelpEvent& rHEvt )
{
    //  Wenn eigene QuickHelp angezeigt wird, nicht durch RequestHelp
    //  wieder wegnehmen lassen

    sal_Bool bOwn = bDragging && Help::IsQuickHelpEnabled();
    if (!bOwn)
        Window::RequestHelp(rHEvt);
}

// -----------------------------------------------------------------------
//                  Dummys fuer virtuelle Methoden
// -----------------------------------------------------------------------

SCCOLROW ScHeaderControl::GetHiddenCount( SCCOLROW nEntryNo )
{
    SCCOLROW nHidden = 0;
    while ( nEntryNo < nSize && GetEntrySize( nEntryNo ) == 0 )
    {
        ++nEntryNo;
        ++nHidden;
    }
    return nHidden;
}

sal_Bool ScHeaderControl::IsLayoutRTL()
{
    return false;
}

sal_Bool ScHeaderControl::IsMirrored()
{
    return false;
}

sal_Bool ScHeaderControl::IsDisabled()
{
    return false;
}

sal_Bool ScHeaderControl::ResizeAllowed()
{
    return sal_True;
}

void ScHeaderControl::SelectWindow()
{
}

void ScHeaderControl::DrawInvert( long /* nDragPos */ )
{
}

String ScHeaderControl::GetDragHelp( long /* nVal */ )
{
    return EMPTY_STRING;
}

void ScHeaderControl::SetMarking( sal_Bool /* bSet */ )
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
