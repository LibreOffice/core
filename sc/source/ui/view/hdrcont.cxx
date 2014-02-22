/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sfx2/dispatch.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <tools/poly.hxx>
#include <svtools/colorcfg.hxx>

#include "scresid.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "hdrcont.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"
#include "gridmerg.hxx"
#include "document.hxx"
#include "markdata.hxx"

#define SC_DRAG_MIN     2





#define SC_HDRPAINT_SEL_BOTTOM  4
#define SC_HDRPAINT_BOTTOM      5
#define SC_HDRPAINT_TEXT        6
#define SC_HDRPAINT_COUNT       7

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
            nDragNo     ( 0 ),
            nDragStart  ( 0 ),
            nDragPos    ( 0 ),
            bDragMoved  ( false ),
            bIgnoreMove ( false )
{
    
    
    
    
    
    EnableRTL( false );

    aNormFont = GetFont();
    aNormFont.SetTransparent( true );       
    aBoldFont = aNormFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );

    SetFont(aBoldFont);
    bBoldSet = true;

    Size aSize = LogicToPixel( Size(
        GetTextWidth(OUString("8888")),
        GetTextHeight() ) );
    aSize.Width()  += 4;    
    aSize.Height() += 3;
    SetSizePixel( aSize );

    nWidth = nSmallWidth = aSize.Width();
    nBigWidth = LogicToPixel( Size( GetTextWidth(OUString("8888888")), 0 ) ).Width() + 5;

    SetBackground();    
}

void ScHeaderControl::SetWidth( long nNew )
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
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    Rectangle aRect( Point(0,0), GetOutputSizePixel() );
    if ( bVertical )
    {
        aRect.Top() = GetScrPos( nStart )-nLayoutSign;      
        aRect.Bottom() = GetScrPos( nEnd+1 )-nLayoutSign;
    }
    else
    {
        aRect.Left() = GetScrPos( nStart )-nLayoutSign;     
        aRect.Right() = GetScrPos( nEnd+1 )-nLayoutSign;
    }
    Invalidate(aRect);
}

void ScHeaderControl::SetMark( bool bNewSet, SCCOLROW nNewStart, SCCOLROW nNewEnd )
{
    bool bEnabled = SC_MOD()->GetInputOptions().GetMarkHeader();    
    if (!bEnabled)
        bNewSet = false;

    bool bOldSet       = bMarkRange;
    SCCOLROW nOldStart = nMarkStart;
    SCCOLROW nOldEnd   = nMarkEnd;
    PutInOrder( nNewStart, nNewEnd );
    bMarkRange = bNewSet;
    nMarkStart = nNewStart;
    nMarkEnd   = nNewEnd;

    

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
                
                DoPaint( nOldStart, nOldEnd );
                DoPaint( nNewStart, nNewEnd );
            }
            else 
                DoPaint( std::min( nNewStart, nOldStart ), std::max( nNewEnd, nOldEnd ) );
        }
        else
            DoPaint( nNewStart, nNewEnd );      
    }
    else if ( bOldSet )
        DoPaint( nOldStart, nOldEnd );          
}

long ScHeaderControl::GetScrPos( SCCOLROW nEntryNo ) const
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



void ScHeaderControl::DrawShadedRect( long nStart, long nEnd, const Color& rBaseColor )
{
    Color aWhite( COL_WHITE );

    Color aInner( rBaseColor );             
    Color aCenter( rBaseColor );
    aCenter.Merge( aWhite, 0xd0 );          
    Color aOuter( rBaseColor );
    aOuter.Merge( aWhite, 0xa0 );           

    if ( IsMirrored() )
        std::swap( aInner, aOuter );        

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

void ScHeaderControl::Paint( const Rectangle& rRect )
{
    
    

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    bool bHighContrast = rStyleSettings.GetHighContrastMode();
    bool bDark = rStyleSettings.GetFaceColor().IsDark();
    

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
    aSelLineColor.Merge( aBlack, 0xe0 );        

    bool bLayoutRTL = IsLayoutRTL();
    long nLayoutSign = bLayoutRTL ? -1 : 1;
    bool bMirrored = IsMirrored();

    OUString            aString;
    sal_uInt16          nBarSize;
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
        long nTemp = nPStart;       
        nPStart = nPEnd;
        nPEnd = nTemp;
        nTemp = nTransStart;        
        nTransStart = nTransEnd;
        nTransEnd = nTemp;
        if ( bVertical )            
            nInitScrPos = GetSizePixel().Height() - 1;
        else
            nInitScrPos = GetSizePixel().Width() - 1;
    }

    
    

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

    

    Rectangle aFillRect;
    SetLineColor();

    if ( nLineEnd * nLayoutSign >= nInitScrPos * nLayoutSign )
    {
        if ( bHighContrast )
        {
            
            SetFillColor( rStyleSettings.GetFaceColor() );
            if ( bVertical )
                aFillRect = Rectangle( 0, nInitScrPos, nBarSize-1, nLineEnd );
            else
                aFillRect = Rectangle( nInitScrPos, 0, nLineEnd, nBarSize-1 );
            DrawRect( aFillRect );
        }
        else
        {
            
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
    
    
    //

    ScGridMerger aGrid( this, 1, 1 );

    
    
    

    for (sal_uInt16 nPass = SC_HDRPAINT_SEL_BOTTOM; nPass < SC_HDRPAINT_COUNT; nPass++)
    {
        
        switch ( nPass )
        {
            case SC_HDRPAINT_SEL_BOTTOM:
                
                SetLineColor( bHighContrast ? rStyleSettings.GetDarkShadowColor() : aSelLineColor );
                break;
            case SC_HDRPAINT_BOTTOM:
                SetLineColor( rStyleSettings.GetDarkShadowColor() );
                break;
            case SC_HDRPAINT_TEXT:
                
                if ( nTransEnd * nLayoutSign >= nTransStart * nLayoutSign && bHighContrast && !bDark )
                {
                    
                    
                    
                    
                    
                    
                    

                    Rectangle aTransRect;
                    if (bVertical)
                        aTransRect = Rectangle( 0, nTransStart, nBarSize-1, nTransEnd );
                    else
                        aTransRect = Rectangle( nTransStart, 0, nTransEnd, nBarSize-1 );
                    SetBackground( Color( rStyleSettings.GetFaceColor() ) );
                    DrawSelectionBackground( aTransRect, 0, true, false, false );
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
            if ( nEntryNo >= nSize )                
                nScrPos = nPEnd + nLayoutSign;      
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
                                    aGrid.AddHorLine( aScrPos.X(), aEndPos.X(), aEndPos.Y() );
                                else
                                    aGrid.AddVerLine( aEndPos.X(), aScrPos.Y(), aEndPos.Y() );

                                
                                
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
                            if ( nSizePix > 1 )     
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
                                        aTxtPos.X() += 1;   
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

                    
                    
                }
                nScrPos += nSizePix * nLayoutSign;      
            }
            ++nCount;
        }
        while ( nScrPos * nLayoutSign <= nPEnd * nLayoutSign );

        aGrid.Flush();
    }
}

SCCOLROW ScHeaderControl::GetMousePos( const MouseEvent& rMEvt, bool& rBorder ) const
{
    bool        bFound = false;
    SCCOLROW    nPos = GetPos();
    SCCOLROW    nHitNo = nPos;
    SCCOLROW    nEntryNo = 1 + nPos;
    long    nScrPos;
    long    nMousePos = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
    long    nDif;
    Size    aSize = GetOutputSizePixel();
    long    nWinSize = bVertical ? aSize.Height() : aSize.Width();

    bool bLayoutRTL = IsLayoutRTL();
    long nLayoutSign = bLayoutRTL ? -1 : 1;
    long nEndPos = bLayoutRTL ? -1 : nWinSize;

    nScrPos = GetScrPos( nPos ) - nLayoutSign;
    do
    {
        if (nEntryNo > nSize)
            nScrPos = nEndPos + nLayoutSign;
        else
            nScrPos += GetEntrySize( nEntryNo - 1 ) * nLayoutSign;      

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

    ScViewData* pViewData = pViewSh->GetViewData();
    sal_uInt16 nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();
    const ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
    bool bSelectAllowed = true;
    if ( pProtect && pProtect->isProtected() )
    {
        
        bool bCellsProtected = false;
        if (bVertical)
        {
            
            SCROW nRPos = static_cast<SCROW>(nPos);
            bCellsProtected = pDoc->HasAttrib(0, nRPos, nTab, MAXCOL, nRPos, nTab, HASATTR_PROTECTED);
        }
        else
        {
            
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

    bool bIsBorder;
    SCCOLROW nHitNo = GetMousePos( rMEvt, bIsBorder );
    if (!IsSelectionAllowed(nHitNo))
        return;
    if ( ! rMEvt.IsLeft() )
        return;
    if ( bIsBorder && ResizeAllowed() )
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
            bDragging = true;
            bDragMoved = false;
        }
    }
    else
    {
        pSelEngine->SetWindow( this );
        Point aPoint;
        Rectangle aVis( aPoint,GetOutputSizePixel() );
        if (bVertical)
            aVis.Left() = LONG_MIN, aVis.Right() = LONG_MAX;
        else
            aVis.Top() = LONG_MIN, aVis.Bottom() = LONG_MAX;
        pSelEngine->SetVisibleArea( aVis );

        SetMarking( true );     
        pSelEngine->SelMouseButtonDown( rMEvt );

        
        
        
        pSelEngine->SelMouseMove( rMEvt );

        if (IsMouseCaptured())
        {
            
            
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
        bDragging = false;

        long nScrPos    = GetScrPos( nDragNo );
        long nMousePos  = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
        bool bLayoutRTL = IsLayoutRTL();
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
                    nNewWidth += GetEntrySize( nDragNo );   
                }
                else
                    nNewWidth = 0;
            }
            HideEntries( nStart, nEnd );
        }
        else
        {
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
                bDragMoved = true;
        }
    }
    else
    {
        bool bIsBorder;
        (void)GetMousePos( rMEvt, bIsBorder );

        if ( bIsBorder && rMEvt.GetButtons()==0 && ResizeAllowed() )
            SetPointer( Pointer( bVertical ? POINTER_VSIZEBAR : POINTER_HSIZEBAR ) );
        else
            SetPointer( Pointer( POINTER_ARROW ) );

        if (!bIgnoreMove)
            pSelEngine->SelMouseMove( rMEvt );
    }
}

void ScHeaderControl::Tracking( const TrackingEvent& rTEvt )
{
    
    

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
        StopMarking();      

        

        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell,
                                            SfxViewShell::Current() );
        if ( pViewSh )
        {
            if ( rCEvt.IsMouseEvent() )
            {
                
                ScViewData* pViewData = pViewSh->GetViewData();

                SelectWindow();     
                if ( pViewData->HasEditView( pViewData->GetActivePart() ) )
                    SC_MOD()->InputEnterHandler();  

                MouseEvent aMEvt( rCEvt.GetMousePosPixel() );
                bool bBorder;
                SCCOLROW nPos = GetMousePos( aMEvt, bBorder );
                if (!IsSelectionAllowed(nPos))
                    
                    return;

                SCTAB nTab = pViewData->GetTabNo();
                ScRange aNewRange;
                if ( bVertical )
                    aNewRange = ScRange( 0, sal::static_int_cast<SCROW>(nPos), nTab,
                                         MAXCOL, sal::static_int_cast<SCROW>(nPos), nTab );
                else
                    aNewRange = ScRange( sal::static_int_cast<SCCOL>(nPos), 0, nTab,
                                         sal::static_int_cast<SCCOL>(nPos), MAXROW, nTab );

                
                ScRangeList aRanges;
                pViewData->GetMarkData().FillRangeListWithMarks( &aRanges, false );
                bool bSelected = aRanges.Intersects(aNewRange);

                
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
    bIgnoreMove = true;

    
    

    ReleaseMouse();
}

void ScHeaderControl::ShowDragHelp()
{
    if (Help::IsQuickHelpEnabled())
    {
        long nScrPos    = GetScrPos( nDragNo );
        bool bLayoutRTL = IsLayoutRTL();
        long nVal = bLayoutRTL ? ( nScrPos - nDragPos + 1 )
                               : ( nDragPos + 2 - nScrPos );

        OUString aHelpStr = GetDragHelp( nVal );
        Point aPos = OutputToScreenPixel( Point(0,0) );
        Size aSize = GetSizePixel();

        Point aMousePos = OutputToScreenPixel(GetPointerPosPixel());

        Rectangle aRect;
        sal_uInt16 nAlign;
        if (!bVertical)
        {
            
            aRect.Left() = aMousePos.X();
            aRect.Top()  = aPos.Y() - 4;
            nAlign       = QUICKHELP_BOTTOM|QUICKHELP_CENTER;
        }
        else
        {
            
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
    

    bool bOwn = bDragging && Help::IsQuickHelpEnabled();
    if (!bOwn)
        Window::RequestHelp(rHEvt);
}





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

void ScHeaderControl::DrawInvert( long /* nDragPos */ )
{
}

OUString ScHeaderControl::GetDragHelp( long /* nVal */ )
{
    return EMPTY_OUSTRING;
}

void ScHeaderControl::SetMarking( bool /* bSet */ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
