/*************************************************************************
 *
 *  $RCSfile: hdrcont.cxx,v $
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

#include <sfx2/dispatch.hxx>
#include <vcl/help.hxx>

#include "scresid.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "hdrcont.hxx"
#include "scmod.hxx"        // Optionen
#include "inputopt.hxx"     // Optionen
#include "gridmerg.hxx"

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
                                    USHORT nNewSize, USHORT nNewFlags ) :
            Window      ( pParent ),
            pSelEngine  ( pSelectionEngine ),
            nSize       ( nNewSize ),
            nFlags      ( nNewFlags ),
            bVertical   ( (nNewFlags & HDR_VERTICAL) != 0 ),
            bDragging   ( FALSE ),
            bIgnoreMove ( FALSE ),
            nMarkStart  ( 0 ),
            nMarkEnd    ( 0 ),
            bMarkRange  ( FALSE )
{
    aNormFont = GetFont();
    aNormFont.SetTransparent( TRUE );       //! WEIGHT_NORMAL hart setzen ???
    aBoldFont = aNormFont;
    aBoldFont.SetWeight( WEIGHT_BOLD );

    SetFont(aBoldFont);
    bBoldSet = TRUE;

    Size aSize = LogicToPixel( Size(
        GetTextWidth( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("8888")) ),
        GetTextHeight() ) );
    aSize.Width()  += 4;    // Platz fuer hervorgehobene Umrandung
    aSize.Height() += 3;
    SetSizePixel( aSize );

    nWidth = nSmallWidth = aSize.Width();
    nBigWidth = LogicToPixel( Size( GetTextWidth(
        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("88888")) ), 0 ) ).Width() + 4;

    SetBackground();    // sonst Probleme auf OS/2 !?!?!
}

void ScHeaderControl::SetWidth( long nNew )
{
    DBG_ASSERT( bVertical, "SetDigits nur fuer Zeilenkoepfe erlaubt" );
    if ( nNew != nWidth )
    {
        Size aSize( nNew, GetSizePixel().Height() );    // Hoehe nicht aendern
        SetSizePixel( aSize );

        nWidth = nNew;

        Invalidate();       // neu zentrieren
    }
}

__EXPORT ScHeaderControl::~ScHeaderControl()
{
}

void ScHeaderControl::DoPaint( USHORT nStart, USHORT nEnd )
{
    Rectangle aRect( Point(0,0), GetOutputSizePixel() );
    if ( bVertical )
    {
        aRect.Top() = GetScrPos( nStart );
        aRect.Bottom() = GetScrPos( nEnd+1 )-1;
    }
    else
    {
        aRect.Left() = GetScrPos( nStart );
        aRect.Right() = GetScrPos( nEnd+1 )-1;
    }
    Invalidate(aRect);
}

void ScHeaderControl::SetMark( BOOL bNewSet, USHORT nNewStart, USHORT nNewEnd )
{
    BOOL bEnabled = SC_MOD()->GetInputOptions().GetMarkHeader();    //! cachen?
    if (!bEnabled)
        bNewSet = FALSE;

    //  Variablen setzen

    BOOL bOldSet     = bMarkRange;
    USHORT nOldStart = nMarkStart;
    USHORT nOldEnd   = nMarkEnd;
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

long ScHeaderControl::GetScrPos( USHORT nEntryNo )
{
    long nScrPos;

    long nMax = ( bVertical ? GetOutputSizePixel().Height() : GetOutputSizePixel().Width() ) + 1;
    if (nEntryNo >= nSize)
        nScrPos = nMax;
    else
    {
        nScrPos = 0;
        for (USHORT i=GetPos(); i<nEntryNo && nScrPos<nMax; i++)
        {
            USHORT nAdd = GetEntrySize(i);
            if (nAdd)
                nScrPos += nAdd;
            else
            {
                USHORT nHidden = GetHiddenCount(i);
                if (nHidden)
                    i += nHidden - 1;
            }
        }
    }

    return nScrPos;
}

//
//      Paint
//

void __EXPORT ScHeaderControl::Paint( const Rectangle& rRect )
{
    //  fuer VCL ist es wichtig, wenig Aufrufe zu haben, darum werden die aeusseren
    //  Linien zusammengefasst

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    SetTextColor( rStyleSettings.GetButtonTextColor() );
    SetFillColor( rStyleSettings.GetFaceColor() );
    SetLineColor();
    DrawRect( rRect );

    const FunctionSet*  pFuncSet = pSelEngine->GetFunctionSet();
    String              aString;
    USHORT              nBarSize;
    Point               aScrPos;
    Point               aEndPos;
    Size                aTextSize;
    Size                aSize = GetOutputSizePixel();

    if (bVertical)
        nBarSize = (USHORT) GetSizePixel().Width();
    else
        nBarSize = (USHORT) GetSizePixel().Height();

    USHORT  nPos = GetPos();

    long nPStart = bVertical ? rRect.Top() : rRect.Left();
    long nPEnd = bVertical ? rRect.Bottom() : rRect.Right();

    //  aeussere Linien komplett durchzeichnen
    //  Zuerst Ende der letzten Zelle finden

    long nLineEnd = -1;
    for (USHORT i=nPos; i<nSize; i++)
    {
        USHORT nSizePix = GetEntrySize( i );
        if (nSizePix)
        {
            nLineEnd += nSizePix;
            if ( nLineEnd > nPEnd )
            {
                nLineEnd = nPEnd;
                break;
            }
        }
        else
        {
            USHORT nHidden = GetHiddenCount(i);
            if (nHidden)
                i += nHidden - 1;
        }
    }

    if ( nLineEnd >= nPStart )
    {
        SetLineColor( rStyleSettings.GetLightColor() );
        if (bVertical)
            DrawLine( Point( 0, nPStart ), Point( 0, nLineEnd ) );
        else
            DrawLine( Point( nPStart, 0 ), Point( nLineEnd, 0 ) );
        SetLineColor( rStyleSettings.GetDarkShadowColor() );
        if (bVertical)
            DrawLine( Point( nBarSize-1, nPStart ), Point( nBarSize-1, nLineEnd ) );
        else
            DrawLine( Point( nPStart, nBarSize-1 ), Point( nLineEnd, nBarSize-1 ) );
    }

    //
    //  loop through entries several times to avoid changing the line color too often
    //  and to allow merging of lines
    //

    ScGridMerger aGrid( this, 1, 1 );
    for (USHORT nPass = 0; nPass < SC_HDRPAINT_COUNT; nPass++)
    {
        //  set line color etc. before entry loop
        switch ( nPass )
        {
            case SC_HDRPAINT_SEL_RIGHT:
                SetLineColor( rStyleSettings.GetShadowColor() );
                break;
            case SC_HDRPAINT_SEL_LEFT:
                SetLineColor( rStyleSettings.GetLightColor() );
                break;
            case SC_HDRPAINT_TOP:
                SetLineColor( rStyleSettings.GetLightColor() );
                break;
            case SC_HDRPAINT_SEL_TOP:
                SetLineColor( rStyleSettings.GetLightColor() );
                break;
            case SC_HDRPAINT_SEL_BOTTOM:
                SetLineColor( rStyleSettings.GetShadowColor() );
                break;
            case SC_HDRPAINT_BOTTOM:
                SetLineColor( rStyleSettings.GetDarkShadowColor() );
                break;
            //case SC_HDRPAINT_TEXT:
            //  break;
        }

        USHORT  nCount=0;
        long    nScrPos=0;
        do
        {
            if (bVertical)
                aScrPos = Point( 0, nScrPos );
            else
                aScrPos = Point( nScrPos, 0 );

            USHORT  nEntryNo = nCount + nPos;
            if ( nEntryNo >= nSize )                // MAXCOL/MAXROW
                nScrPos = nPEnd + 1;                //  ausserhalb -> Ende
            else
            {
                USHORT nSizePix = GetEntrySize( nEntryNo );

                if (nSizePix == 0)
                {
                    USHORT nHidden = GetHiddenCount(nEntryNo);
                    if (nHidden)
                        nCount += nHidden - 1;
                }
                else if (nScrPos+nSizePix >= nPStart)
                {
                    Point aEndPos(aScrPos);
                    if (bVertical)
                        aEndPos = Point( aScrPos.X()+nBarSize-1, aScrPos.Y()+nSizePix-1 );
                    else
                        aEndPos = Point( aScrPos.X()+nSizePix-1, aScrPos.Y()+nBarSize-1 );

                    BOOL bMark = bMarkRange && nEntryNo >= nMarkStart && nEntryNo <= nMarkEnd;

                    switch ( nPass )
                    {
                        case SC_HDRPAINT_SEL_LEFT:
                            if (bMark)
                            {
                                // continuous line - partly overwritten later
                                if (bVertical)
                                    aGrid.AddVerLine( aScrPos.X()+1, aScrPos.Y(), aEndPos.Y() );
                                else
                                    aGrid.AddHorLine( aScrPos.X(), aEndPos.X(), aScrPos.Y()+1 );
                            }
                            break;

                        case SC_HDRPAINT_SEL_TOP:
                            if (bMark)
                            {
                                if (bVertical)
                                    aGrid.AddHorLine( aScrPos.X()+1, aEndPos.X()-2, aScrPos.Y()+1 );
                                else
                                    aGrid.AddVerLine( aScrPos.X()+1, aScrPos.Y()+1, aEndPos.Y()-2 );
                            }
                            break;

                        case SC_HDRPAINT_SEL_RIGHT:
                            if (bMark)
                            {
                                // continuous line - partly overwritten later
                                if (bVertical)
                                    aGrid.AddVerLine( aEndPos.X()-1, aScrPos.Y(), aEndPos.Y() );
                                else
                                    aGrid.AddHorLine( aScrPos.X(), aEndPos.X(), aEndPos.Y()-1 );
                            }
                            break;

                        case SC_HDRPAINT_SEL_BOTTOM:
                            if (bMark)
                            {
                                if (bVertical)
                                    aGrid.AddHorLine( aScrPos.X()+1, aEndPos.X()-1, aEndPos.Y()-1 );
                                else
                                    aGrid.AddVerLine( aEndPos.X()-1, aScrPos.Y()+1, aEndPos.Y()-1 );
                            }
                            break;

                        case SC_HDRPAINT_TOP:
                            if (bVertical)
                                aGrid.AddHorLine( aScrPos.X(), aEndPos.X()-1, aScrPos.Y() );
                            else
                                aGrid.AddVerLine( aScrPos.X(), aScrPos.Y(), aEndPos.Y()-1 );
                            break;

                        case SC_HDRPAINT_BOTTOM:
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
                                        DrawLine( Point(aScrPos.X(),aEndPos.Y()-1),
                                                  Point(aEndPos.X(),aEndPos.Y()-1) );
                                    else
                                        DrawLine( Point(aEndPos.X()-1,aScrPos.Y()),
                                                  Point(aEndPos.X()-1,aEndPos.Y()) );
                                }
                            break;

                        case SC_HDRPAINT_TEXT:
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
                                    aTxtPos.Y() += (nSizePix-aTextSize.Height())/2;
                                }
                                else
                                {
                                    aTxtPos.X() += (nSizePix-aTextSize.Width()+1)/2;
                                    aTxtPos.Y() += (nBarSize-aTextSize.Height()+1)/2;
                                }
                                DrawText( aTxtPos, aString );
                            }
                            break;
                    }

                    //  bei Selektion der ganzen Zeile/Spalte:
                    //  InvertRect( Rectangle( aScrPos, aEndPos ) );
                }
                nScrPos += nSizePix;    // auch wenn noch oberhalb
            }
            ++nCount;
        }
        while ( nScrPos <= nPEnd );

        aGrid.Flush();
    }
}

//
//      Maus - Handling
//

USHORT ScHeaderControl::GetMousePos( const MouseEvent& rMEvt, BOOL& rBorder )
{
    BOOL    bFound=FALSE;
    USHORT  nCount = 1;
    USHORT  nPos = GetPos();
    USHORT  nHitNo = nPos;
    long    nScrPos;
    long    nMousePos = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
    long    nDif;
    Size    aSize = GetOutputSizePixel();

    nScrPos = GetScrPos( nPos ) - 1;
    do
    {
        USHORT nEntryNo = nCount + nPos;

//      nScrPos = GetScrPos( nEntryNo ) - 1;

        if (nEntryNo >= nSize)
            nScrPos = ( bVertical ? GetOutputSizePixel().Height() : GetOutputSizePixel().Width() ) + 1;
        else
            nScrPos += GetEntrySize( nEntryNo - 1 );        //! GetHiddenCount() ??

        nDif = nMousePos - nScrPos;
        if (nDif >= -2 && nDif <= 2 && nCount > 0)
        {
            bFound=TRUE;
            nHitNo=nEntryNo-1;
        }
        else if (nDif >= 0)
            nHitNo = nEntryNo;
        ++nCount;
    }
    while ( nScrPos < ( bVertical ? aSize.Height() : aSize.Width() ) && nDif > 0 );

    rBorder = bFound;
    return nHitNo;
}

void __EXPORT ScHeaderControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (IsDisabled())
        return;

    bIgnoreMove = FALSE;
    SelectWindow();

    BOOL bFound;
    USHORT nHitNo = GetMousePos( rMEvt, bFound );

    if ( bFound && rMEvt.IsLeft() && ResizeAllowed() )
    {
        nDragNo = nHitNo;
        USHORT nClicks = rMEvt.GetClicks();
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

            // CaptureMouse();
            StartTracking();
            bDragging = TRUE;
            bDragMoved = FALSE;
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

        SetMarking( TRUE );     //  muss vor SelMouseButtonDown sein
        pSelEngine->SelMouseButtonDown( rMEvt );

        if (IsMouseCaptured())
        {
            //  Tracking statt CaptureMouse, damit sauber abgebrochen werden kann
            //! Irgendwann sollte die SelectionEngine selber StartTracking rufen!?!
            ReleaseMouse();
            StartTracking();
        }
    }
}

void __EXPORT ScHeaderControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( IsDisabled() )
        return;

    SetMarking( FALSE );
    bIgnoreMove = FALSE;
    BOOL bFound;
    USHORT nHitNo = GetMousePos( rMEvt, bFound );

    if ( bDragging )
    {
        DrawInvert( nDragPos );
        ReleaseMouse();
        bDragging   = FALSE;

        long nScrPos    = GetScrPos( nDragNo );
        long nMousePos  = bVertical ? rMEvt.GetPosPixel().Y() : rMEvt.GetPosPixel().X();
        long nNewWidth  = nMousePos + 2 - nScrPos;

        if ( nNewWidth < 0 /* && !IsSelected(nDragNo) */ )
        {
            USHORT nStart;
            USHORT nEnd = nDragNo;
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
                SetEntrySize( nDragNo, (USHORT) nNewWidth );
        }
    }
    else
    {
        pSelEngine->SelMouseButtonUp( rMEvt );
        ReleaseMouse();
    }
}

void __EXPORT ScHeaderControl::MouseMove( const MouseEvent& rMEvt )
{
    if ( IsDisabled() )
    {
        SetPointer( Pointer( POINTER_ARROW ) );
        return;
    }

    BOOL bFound;
    USHORT nHitNo = GetMousePos( rMEvt, bFound );

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
                bDragMoved = TRUE;
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

void __EXPORT ScHeaderControl::Command( const CommandEvent& rCEvt )
{
    USHORT nCmd = rCEvt.GetCommand();
    if ( nCmd == COMMAND_CONTEXTMENU )
    {
        StopMarking();      // Selektion / Dragging beenden

        //  Popup ausfuehren

        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell,
                                            SfxViewShell::Current() );
        if ( pViewSh )
        {
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
        bDragging = FALSE;
    }

    SetMarking( FALSE );
    bIgnoreMove = TRUE;
    pSelEngine->Reset();
    ReleaseMouse();
}

void ScHeaderControl::ShowDragHelp()
{
    if (Help::IsQuickHelpEnabled())
    {
        long nVal = nDragPos + 2 - GetScrPos( nDragNo );

        String aHelpStr = GetDragHelp( nVal );
        Point aPos = OutputToScreenPixel( Point(0,0) );
        Size aSize = GetSizePixel();

#ifdef VCL
        Point aMousePos = OutputToScreenPixel(GetPointerPosPixel());
#else
        Point aMousePos = Pointer::GetPosPixel();
#endif

        Rectangle aRect;
        USHORT nAlign;
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

void __EXPORT ScHeaderControl::RequestHelp( const HelpEvent& rHEvt )
{
    //  Wenn eigene QuickHelp angezeigt wird, nicht durch RequestHelp
    //  wieder wegnehmen lassen

    BOOL bOwn = bDragging && Help::IsQuickHelpEnabled();
    if (!bOwn)
        Window::RequestHelp(rHEvt);
}

// -----------------------------------------------------------------------
//                  Dummys fuer virtuelle Methoden
// -----------------------------------------------------------------------

USHORT ScHeaderControl::GetHiddenCount( USHORT nEntryNo )
{
    USHORT nHidden = 0;
    while ( nEntryNo < nSize && GetEntrySize( nEntryNo ) == 0 )
    {
        ++nEntryNo;
        ++nHidden;
    }
    return nHidden;
}

BOOL ScHeaderControl::IsDisabled()
{
    return FALSE;
}

BOOL ScHeaderControl::ResizeAllowed()
{
    return TRUE;
}

void ScHeaderControl::SelectWindow()
{
}

void ScHeaderControl::DrawInvert( long nDragPos )
{
}

String ScHeaderControl::GetDragHelp( long nVal )
{
    return EMPTY_STRING;
}

void ScHeaderControl::SetMarking( BOOL bSet )
{
}



