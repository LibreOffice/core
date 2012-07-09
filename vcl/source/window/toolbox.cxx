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


#include <rtl/logfile.hxx>

#include <tools/debug.hxx>
#include <tools/rc.h>
#include <tools/poly.hxx>

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/accel.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/sound.hxx>
#include <vcl/virdev.hxx>
#include <vcl/spin.h>
#include <vcl/toolbox.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/gradient.hxx>
#include <vcl/menu.hxx>

#include <svdata.hxx>
#include <window.h>
#include <toolbox.h>
#include <salframe.hxx>
#if defined WNT
#include <svsys.h>
#endif

#include <string.h>
#include <vector>
#include <math.h>

// =======================================================================

DBG_NAMEEX( Window )

// =======================================================================

#define SMALLBUTTON_HSIZE           7
#define SMALLBUTTON_VSIZE           7

#define SMALLBUTTON_OFF_NORMAL_X    3
#define SMALLBUTTON_OFF_NORMAL_Y    3
#define SMALLBUTTON_OFF_PRESSED_X   5
#define SMALLBUTTON_OFF_PRESSED_Y   5

#define OUTBUTTON_SIZE              6
#define OUTBUTTON_BORDER            4
#define OUTBUTTON_OFF_NORMAL_X      1
#define OUTBUTTON_OFF_NORMAL_Y      1

// -----------------------------------------------------------------------

#define TB_TEXTOFFSET           2
#define TB_IMAGETEXTOFFSET      3
#define TB_LINESPACING          3
#define TB_SPIN_SIZE            14
#define TB_SPIN_OFFSET          2
#define TB_NEXT_SIZE            22
#define TB_NEXT_OFFSET          2
#define TB_BORDER_OFFSET1       4
#define TB_BORDER_OFFSET2       2
#define TB_CUSTOMIZE_OFFSET     2
#define TB_RESIZE_OFFSET        3
#define TB_MAXLINES             5
#define TB_MAXNOSCROLL          32765

#define TB_MIN_WIN_WIDTH        20

#define TB_CALCMODE_HORZ        1
#define TB_CALCMODE_VERT        2
#define TB_CALCMODE_FLOAT       3

#define TB_WBLINESIZING         (WB_SIZEABLE | WB_DOCKABLE | WB_SCROLL)

#define DOCK_LINEHSIZE          ((sal_uInt16)0x0001)
#define DOCK_LINEVSIZE          ((sal_uInt16)0x0002)
#define DOCK_LINERIGHT          ((sal_uInt16)0x1000)
#define DOCK_LINEBOTTOM         ((sal_uInt16)0x2000)
#define DOCK_LINELEFT           ((sal_uInt16)0x4000)
#define DOCK_LINETOP            ((sal_uInt16)0x8000)
#define DOCK_LINEOFFSET         3


// -----------------------------------------------------------------------
static void ImplDrawButton( ToolBox* pThis, const Rectangle &rRect, sal_uInt16 highlight, sal_Bool bChecked, sal_Bool bEnabled, sal_Bool bIsWindow );
// -----------------------------------------------------------------------

struct ImplToolSize
{
    long            mnWidth;
    long            mnHeight;
    sal_uInt16          mnLines;
};

struct ImplToolSizeArray
{
    long          mnLength;
    long          mnLastEntry;
    ImplToolSize* mpSize;

    ImplToolSizeArray() { mpSize = NULL; mnLength = 0; mnLastEntry = 0; }
    ~ImplToolSizeArray() { if( mpSize ) delete [] mpSize; mnLength = 0; }
};

// -----------------------------------------------------------------------

typedef ::std::vector< ToolBox* > ImplTBList;

class ImplTBDragMgr
{
private:
    ImplTBList*     mpBoxList;
    ToolBox*        mpDragBox;
    Point           maMouseOff;
    Rectangle       maRect;
    Rectangle       maStartRect;
    Accelerator     maAccel;
    long            mnMinWidth;
    long            mnMaxWidth;
    sal_uInt16          mnLineMode;
    sal_uInt16          mnStartLines;
    void*           mpCustomizeData;
    sal_Bool            mbCustomizeMode;
    sal_Bool            mbResizeMode;
    sal_Bool            mbShowDragRect;

public:
                    ImplTBDragMgr();
                    ~ImplTBDragMgr();

    void            push_back( ToolBox* pBox )
                        { mpBoxList->push_back( pBox ); }
    void            erase( ToolBox* pBox )
                    {
                        for ( ImplTBList::iterator it = mpBoxList->begin(); it < mpBoxList->end(); ++it ) {
                            if ( *it == pBox ) {
                                mpBoxList->erase( it );
                                break;
                            }
                        }
                    }
    size_t          size() const
                    { return mpBoxList->size(); }

    ToolBox*        FindToolBox( const Rectangle& rRect );

    void            StartDragging( ToolBox* pDragBox,
                                   const Point& rPos, const Rectangle& rRect,
                                   sal_uInt16 nLineMode, sal_Bool bResizeItem,
                                   void* pData = NULL );
    void            Dragging( const Point& rPos );
    void            EndDragging( sal_Bool bOK = sal_True );
    void            HideDragRect() { if ( mbShowDragRect ) mpDragBox->HideTracking(); }
    void            UpdateDragRect();
                    DECL_LINK( SelectHdl, Accelerator* );

    sal_Bool            IsCustomizeMode() { return mbCustomizeMode; }
    sal_Bool            IsResizeMode() { return mbResizeMode; }
};

// -----------------------------------------------------------------------

static ImplTBDragMgr* ImplGetTBDragMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maCtrlData.mpTBDragMgr )
        pSVData->maCtrlData.mpTBDragMgr = new ImplTBDragMgr;
    return pSVData->maCtrlData.mpTBDragMgr;
}

// -----------------------------------------------------------------------

int ToolBox::ImplGetDragWidth( ToolBox* pThis )
{
    #define TB_DRAGWIDTH 8  // the default width of the grip

    int width = TB_DRAGWIDTH;
    if( pThis->IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL ) )
    {

        ImplControlValue aControlValue;
        Point aPoint;
        Rectangle aContent, aBound;
        Rectangle aArea( aPoint, pThis->GetOutputSizePixel() );

        if ( pThis->GetNativeControlRegion(CTRL_TOOLBAR, pThis->mbHorz ? PART_THUMB_VERT : PART_THUMB_HORZ,
                aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            width = pThis->mbHorz ? aContent.GetWidth() : aContent.GetHeight();
        }
    }
    return width;
}

ButtonType determineButtonType( ImplToolItem* pItem, ButtonType defaultType )
{
    ButtonType tmpButtonType = defaultType;
    ToolBoxItemBits nBits( pItem->mnBits & 0x300 );
    if ( nBits & TIB_TEXTICON ) // item has custom setting
    {
        tmpButtonType = BUTTON_SYMBOLTEXT;
        if ( nBits == TIB_TEXT_ONLY )
            tmpButtonType = BUTTON_TEXT;
        else if ( nBits == TIB_ICON_ONLY )
            tmpButtonType = BUTTON_SYMBOL;
    }
    return tmpButtonType;
}

// -----------------------------------------------------------------------

void ToolBox::ImplUpdateDragArea( ToolBox *pThis )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );
    if( pWrapper )
    {
        if ( pThis->ImplIsFloatingMode() || pWrapper->IsLocked() )
            pWrapper->SetDragArea( Rectangle() );
        else
        {
            if( pThis->meAlign == WINDOWALIGN_TOP || pThis->meAlign == WINDOWALIGN_BOTTOM )
                pWrapper->SetDragArea( Rectangle( 0, 0, ImplGetDragWidth( pThis ), pThis->GetOutputSizePixel().Height() ) );
            else
                pWrapper->SetDragArea( Rectangle( 0, 0, pThis->GetOutputSizePixel().Width(), ImplGetDragWidth( pThis ) ) );
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
                              long& rRight, long& rBottom, const ToolBox *pThis )
{
    if( pThis->ImplIsFloatingMode() || !(pThis->mnWinStyle & WB_BORDER) )
    {
        // no border in floating mode
        rLeft = rTop = rRight = rBottom = 0;
        return;
    }

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );

    // reserve dragarea only for dockable toolbars
    int    dragwidth = ( pWrapper && !pWrapper->IsLocked() ) ? ImplGetDragWidth( (ToolBox*)pThis ) : 0;

    // no shadow border for dockable toolbars
    int    borderwidth = pWrapper ? 0: 2;

    if ( eAlign == WINDOWALIGN_TOP )
    {
        rLeft   = borderwidth+dragwidth;
        rTop    = borderwidth;
        rRight  = borderwidth;
        rBottom = 0;
    }
    else if ( eAlign == WINDOWALIGN_LEFT )
    {
        rLeft   = borderwidth;
        rTop    = borderwidth+dragwidth;
        rRight  = 0;
        rBottom = borderwidth;
    }
    else if ( eAlign == WINDOWALIGN_BOTTOM )
    {
        rLeft   = borderwidth+dragwidth;
        rTop    = 0;
        rRight  = borderwidth;
        rBottom = borderwidth;
    }
    else
    {
        rLeft   = 0;
        rTop    = borderwidth+dragwidth;
        rRight  = borderwidth;
        rBottom = borderwidth;
    }
}

// -----------------------------------------------------------------------

static void ImplCheckUpdate( ToolBox *pThis )
{
    // remove any pending invalidates to avoid
    // have them triggered when paint is locked (see mpData->mbIsPaintLocked)
    // which would result in erasing the background only and not painting any items
    // this must not be done when we're already in Paint()

    // this is only required for transparent toolbars (see ImplDrawTransparentBackground() )
    if( !pThis->IsBackground() && pThis->HasPaintEvent() && !pThis->IsInPaint() )
        pThis->Update();
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawGrip( ToolBox* pThis )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );
    if( pWrapper && !pWrapper->GetDragArea().IsEmpty() )
    {
        // execute pending paint requests
        ImplCheckUpdate( pThis );

        sal_Bool bNativeOk = sal_False;
        if( pThis->IsNativeControlSupported( CTRL_TOOLBAR, pThis->mbHorz ? PART_THUMB_HORZ : PART_THUMB_VERT ) )
        {
            ToolbarValue        aToolbarValue;
            aToolbarValue.maGripRect = pWrapper->GetDragArea();
            Point aPt;
            Rectangle           aCtrlRegion( aPt, pThis->GetOutputSizePixel() );
            ControlState        nState = CTRL_STATE_ENABLED;

            bNativeOk = pThis->DrawNativeControl( CTRL_TOOLBAR, pThis->mbHorz ? PART_THUMB_VERT : PART_THUMB_HORZ,
                                            aCtrlRegion, nState, aToolbarValue, rtl::OUString() );
        }

        if( bNativeOk )
            return;

        const StyleSettings&    rStyleSettings = pThis->GetSettings().GetStyleSettings();
        pThis->SetLineColor( rStyleSettings.GetShadowColor() );

        Size aSz ( pThis->GetOutputSizePixel() );

        if ( pThis->meAlign == WINDOWALIGN_TOP || pThis->meAlign == WINDOWALIGN_BOTTOM )
        {
            int height = (int) (0.6 * aSz.Height() + 0.5);
            int i = (aSz.Height() - height) / 2;
            height += i;
            while( i <= height )
            {
                int x = ImplGetDragWidth( pThis ) / 2;

                pThis->DrawPixel( Point(x, i), rStyleSettings.GetDarkShadowColor() );
                pThis->DrawPixel( Point(x+1, i), rStyleSettings.GetShadowColor() );

                pThis->DrawPixel( Point(x, i+1), rStyleSettings.GetShadowColor() );
                pThis->DrawPixel( Point(x+1, i+1), rStyleSettings.GetFaceColor() );
                pThis->DrawPixel( Point(x+2, i+1), Color(COL_WHITE) );

                pThis->DrawPixel( Point(x+1, i+2), Color(COL_WHITE) );
                pThis->DrawPixel( Point(x+2, i+2), Color(COL_WHITE) );
                i+=4;
            }
        }
        else
        {
            int width = (int) (0.6 * aSz.Width() + 0.5);
            int i = (aSz.Width() - width) / 2;
            width += i;
            while( i <= width )
            {
                int y = ImplGetDragWidth(pThis) / 2;

                pThis->DrawPixel( Point(i, y), rStyleSettings.GetDarkShadowColor() );
                pThis->DrawPixel( Point(i+1, y), rStyleSettings.GetShadowColor() );

                pThis->DrawPixel( Point(i, y+1), rStyleSettings.GetShadowColor() );
                pThis->DrawPixel( Point(i+1, y+1), rStyleSettings.GetFaceColor() );
                pThis->DrawPixel( Point(i+2, y+1), Color(COL_WHITE) );

                pThis->DrawPixel( Point(i+1, y+2), Color(COL_WHITE) );
                pThis->DrawPixel( Point(i+2, y+2), Color(COL_WHITE) );
                i+=4;
            }
        }
    }
}

void ToolBox::ImplDrawGradientBackground( ToolBox* pThis, ImplDockingWindowWrapper * )
{
    // draw a nice gradient

    Color startCol, endCol;
    startCol = pThis->GetSettings().GetStyleSettings().GetFaceGradientColor();
    endCol = pThis->GetSettings().GetStyleSettings().GetFaceColor();
    if( pThis->GetSettings().GetStyleSettings().GetHighContrastMode() )
        // no 'extreme' gradient when high contrast
        startCol = endCol;

    Gradient g;
    g.SetAngle( pThis->mbHorz ? 0 : 900 );
    g.SetStyle( GRADIENT_LINEAR );

    g.SetStartColor( startCol );
    g.SetEndColor( endCol );

    sal_Bool bLineColor = pThis->IsLineColor();
    Color aOldCol = pThis->GetLineColor();
    pThis->SetLineColor( pThis->GetSettings().GetStyleSettings().GetShadowColor() );

    Size aFullSz( pThis->GetOutputSizePixel() );
    Size aLineSz( aFullSz );

    // use the linesize only when floating
    // full window height is used when docked (single line)
    if( pThis->ImplIsFloatingMode() )
    {
        long nLineSize;
        if( pThis->mbHorz )
        {
            nLineSize = pThis->mnMaxItemHeight;
            if ( pThis->mnWinHeight > pThis->mnMaxItemHeight )
                nLineSize = pThis->mnWinHeight;

            aLineSz.Height() = nLineSize;
        }
        else
        {
            nLineSize = pThis->mnMaxItemWidth;
            aLineSz.Width() = nLineSize;
        }
    }

    long nLeft, nTop, nRight, nBottom;
    ImplCalcBorder( pThis->meAlign, nLeft, nTop, nRight, nBottom, pThis );

    Size aTopLineSz( aLineSz );
    Size aBottomLineSz( aLineSz );

    if ( pThis->mnWinStyle & WB_BORDER )
    {
        if( pThis->mbHorz )
        {
            aTopLineSz.Height() += TB_BORDER_OFFSET2 + nTop;
            aBottomLineSz.Height() += TB_BORDER_OFFSET2 + nBottom;

            if( pThis->mnCurLines == 1 )
                aTopLineSz.Height() += TB_BORDER_OFFSET2 + nBottom;
        }
        else
        {
            aTopLineSz.Width() += TB_BORDER_OFFSET1 + nLeft;
            aBottomLineSz.Width() += TB_BORDER_OFFSET1 + nRight;

            if( pThis->mnCurLines == 1 )
                aTopLineSz.Width() += TB_BORDER_OFFSET1 + nLeft;
        }
    }

    if( pThis->mbHorz )
    {
        aTopLineSz.Height() += pThis->mnBorderY;
        if( pThis->mnCurLines == 1 )
            aTopLineSz.Height() += pThis->mnBorderY;

        aBottomLineSz.Height() += pThis->mnBorderY;
    }
    else
    {
        aTopLineSz.Width() += pThis->mnBorderX;
        if( pThis->mnCurLines == 1 )
            aTopLineSz.Width() += pThis->mnBorderX;

        aBottomLineSz.Width() += pThis->mnBorderX;
    }


    if ( pThis->mnWinStyle & WB_LINESPACING )
    {
        if( pThis->mbHorz )
        {
            aLineSz.Height() += TB_LINESPACING;
            if( pThis->mnCurLines > 1 )
                aTopLineSz.Height() += TB_LINESPACING;
        }
        else
        {
            aLineSz.Width() += TB_LINESPACING;
            if( pThis->mnCurLines > 1 )
                aTopLineSz.Width() += TB_LINESPACING;
        }
    }

    if( pThis->mbHorz )
    {
        long y = 0;
        sal_Bool bDrawSep = sal_False; // pThis->ImplIsFloatingMode() && ( pThis->mnWinStyle & WB_LINESPACING );

        pThis->DrawGradient( Rectangle( 0, y, aTopLineSz.Width(), y+aTopLineSz.Height()), g );
        y += aTopLineSz.Height();

        if ( bDrawSep )
            pThis->DrawLine( Point(0, y-2), Point(aTopLineSz.Width(), y-2) );

        while( y < (pThis->mnDY - aBottomLineSz.Height()) )
        {
            pThis->DrawGradient( Rectangle( 0, y, aLineSz.Width(), y+aLineSz.Height()), g);
            y += aLineSz.Height();

            if ( bDrawSep )
                pThis->DrawLine( Point(0, y-2), Point(aLineSz.Width(), y-2) );
        }

        pThis->DrawGradient( Rectangle( 0, y, aBottomLineSz.Width(), y+aBottomLineSz.Height()), g );
        if ( bDrawSep )
            pThis->DrawLine( Point(0, y-2), Point(aBottomLineSz.Width(), y-2) );
    }
    else
    {
        long x = 0;

        pThis->DrawGradient( Rectangle( x, 0, x+aTopLineSz.Width(), aTopLineSz.Height()), g );
        x += aTopLineSz.Width();

        while( x < (pThis->mnDX - aBottomLineSz.Width()) )
        {
            pThis->DrawGradient( Rectangle( x, 0, x+aLineSz.Width(), aLineSz.Height()), g);
            x += aLineSz.Width();
        }

        pThis->DrawGradient( Rectangle( x, 0, x+aBottomLineSz.Width(), aBottomLineSz.Height()), g );
    }

    if( bLineColor )
        pThis->SetLineColor( aOldCol );

}

sal_Bool ToolBox::ImplDrawNativeBackground( ToolBox* pThis, const Region & )
{
    // use NWF
    Point aPt;
    Rectangle aCtrlRegion( aPt, pThis->GetOutputSizePixel() );
    ControlState  nState = CTRL_STATE_ENABLED;

    return pThis->DrawNativeControl( CTRL_TOOLBAR, pThis->mbHorz ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT,
                                    aCtrlRegion, nState, ImplControlValue(), rtl::OUString() );
}

void ToolBox::ImplDrawTransparentBackground( ToolBox* pThis, const Region &rRegion )
{
    // just invalidate to trigger paint of the parent

    const bool        bOldPaintLock = pThis->mpData->mbIsPaintLocked;
    pThis->mpData->mbIsPaintLocked = true;

    // send an invalidate to the first opaque parent and invalidate the whole hierarchy from there (noclipchildren)
    pThis->Invalidate( rRegion, INVALIDATE_UPDATE|INVALIDATE_NOCLIPCHILDREN );

    pThis->mpData->mbIsPaintLocked = bOldPaintLock;
}

void ToolBox::ImplDrawConstantBackground( ToolBox* pThis, const Region &rRegion, sal_Bool bIsInPopupMode )
{
    // draw a constant color
    if( !bIsInPopupMode )
        // default background
        pThis->Erase( rRegion.GetBoundRect() );
    else
    {
        // use different color in popupmode
        pThis->DrawWallpaper( rRegion.GetBoundRect(),
            Wallpaper( pThis->GetSettings().GetStyleSettings().GetFaceGradientColor() ) );
    }
}


void ToolBox::ImplDrawBackground( ToolBox* pThis, const Rectangle &rRect )
{
    // execute pending paint requests
    ImplCheckUpdate( pThis );

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );
    sal_Bool bIsInPopupMode = pThis->ImplIsInPopupMode();

    Region aPaintRegion( rRect );

    // make sure we do not invalidate/erase too much
    if( pThis->IsInPaint() )
        aPaintRegion.Intersect( pThis->GetActiveClipRegion() );

    pThis->Push( PUSH_CLIPREGION );
    pThis->IntersectClipRegion( aPaintRegion );


    if( !pWrapper /*|| bIsInPopupMode*/ )
    {
        // no gradient for ordinary toolbars (not dockable)
        if( !pThis->IsBackground() && !pThis->IsInPaint() )
            ImplDrawTransparentBackground( pThis, aPaintRegion );
        else
            ImplDrawConstantBackground( pThis, aPaintRegion, bIsInPopupMode );
    }
    else
    {
        // toolbars known to the dockingmanager will be drawn using NWF or a gradient
        // docked toolbars are transparent and NWF is already used in the docking area which is their common background
        // so NWF is used here for floating toolbars only
        sal_Bool bNativeOk = sal_False;
        if( pThis->ImplIsFloatingMode() && pThis->IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL) )
            bNativeOk = ImplDrawNativeBackground( pThis, aPaintRegion );

        if( !bNativeOk )
        {
            if( !pThis->IsBackground() )
            {
                if( !pThis->IsInPaint() )
                    ImplDrawTransparentBackground( pThis, aPaintRegion );
            }
            else
                ImplDrawGradientBackground( pThis, pWrapper );
        }
    }

    // restore clip region
    pThis->Pop();
}

void ToolBox::ImplErase( ToolBox* pThis, const Rectangle &rRect, sal_Bool bHighlight, sal_Bool bHasOpenPopup )
{
    // the background of non NWF buttons is painted in a constant color
    // to have the same highlight color (transparency in DrawSelectionBackground())
    // items with open popups will also painted using a constant color
    if( !pThis->mpData->mbNativeButtons &&
        (bHighlight || ! (((Window*) pThis)->GetStyle() & WB_3DLOOK ) ) )
    {
        if( (((Window*) pThis)->GetStyle() & WB_3DLOOK ) )
        {
            pThis->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pThis->SetLineColor();
            if( bHasOpenPopup )
                // choose the same color as the popup will use
                pThis->SetFillColor( pThis->GetSettings().GetStyleSettings().GetFaceGradientColor() );
            else
                pThis->SetFillColor( Color( COL_WHITE ) );

            pThis->DrawRect( rRect );
            pThis->Pop();
        }
        else
            ImplDrawBackground( pThis, rRect );
    }
    else
        ImplDrawBackground( pThis, rRect );
}

void ToolBox::ImplDrawBorder( ToolBox* pWin )
{
    const StyleSettings&    rStyleSettings = pWin->GetSettings().GetStyleSettings();
    long                    nDX = pWin->mnDX;
    long                    nDY = pWin->mnDY;

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pWin );

    // draw borders for ordinary toolbars only (not dockable)
    if( pWrapper )
        return;

    if ( pWin->meAlign == WINDOWALIGN_BOTTOM )
    {
        // draw bottom border
        pWin->SetLineColor( rStyleSettings.GetShadowColor() );
        pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
        pWin->SetLineColor( rStyleSettings.GetLightColor() );
        pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
    }
    else
    {
        // draw top border
        pWin->SetLineColor( rStyleSettings.GetShadowColor() );
        pWin->DrawLine( Point( 0, 0 ), Point( nDX-1, 0 ) );
        pWin->SetLineColor( rStyleSettings.GetLightColor() );
        pWin->DrawLine( Point( 0, 1 ), Point( nDX-1, 1 ) );

        if ( (pWin->meAlign == WINDOWALIGN_LEFT) || (pWin->meAlign == WINDOWALIGN_RIGHT) )
        {
            if ( pWin->meAlign == WINDOWALIGN_LEFT )
            {
                // draw left-bottom border
                pWin->SetLineColor( rStyleSettings.GetShadowColor() );
                pWin->DrawLine( Point( 0, 0 ), Point( 0, nDY-1 ) );
                pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
                pWin->SetLineColor( rStyleSettings.GetLightColor() );
                pWin->DrawLine( Point( 1, 1 ), Point( 1, nDY-3 ) );
                pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            }
            else
            {
                // draw right-bottom border
                pWin->SetLineColor( rStyleSettings.GetShadowColor() );
                pWin->DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-3 ) );
                pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-2, nDY-2 ) );
                pWin->SetLineColor( rStyleSettings.GetLightColor() );
                pWin->DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
                pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            }
        }
    }


    if ( pWin->meAlign == WINDOWALIGN_BOTTOM || pWin->meAlign == WINDOWALIGN_TOP )
    {
        // draw right border
        pWin->SetLineColor( rStyleSettings.GetShadowColor() );
        pWin->DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-1 ) );
        pWin->SetLineColor( rStyleSettings.GetLightColor() );
        pWin->DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
    }
}

// -----------------------------------------------------------------------

static bool ImplIsFixedControl( const ImplToolItem *pItem )
{
    return ( pItem->mpWindow &&
            (pItem->mpWindow->GetType() == WINDOW_FIXEDTEXT ||
             pItem->mpWindow->GetType() == WINDOW_FIXEDLINE ||
             pItem->mpWindow->GetType() == WINDOW_GROUPBOX) );
}

// -----------------------------------------------------------------------

const ImplToolItem *ToolBox::ImplGetFirstClippedItem( const ToolBox* pThis )
{
    std::vector< ImplToolItem >::const_iterator it;
    it = pThis->mpData->m_aItems.begin();
    while ( it != pThis->mpData->m_aItems.end() )
    {
        if( it->IsClipped() )
            return &(*it);
        ++it;
    }
    return NULL;
}

// -----------------------------------------------------------------------

Size ToolBox::ImplCalcSize( const ToolBox* pThis, sal_uInt16 nCalcLines, sal_uInt16 nCalcMode )
{
    long            nMax;
    long            nLeft = 0;
    long            nTop = 0;
    long            nRight = 0;
    long            nBottom = 0;
    Size            aSize;
    WindowAlign     eOldAlign = pThis->meAlign;
    sal_Bool            bOldHorz = pThis->mbHorz;
    sal_Bool            bOldAssumeDocked = pThis->mpData->mbAssumeDocked;
    sal_Bool            bOldAssumeFloating = pThis->mpData->mbAssumeFloating;

    if ( nCalcMode )
    {
        sal_Bool bOldFloatingMode = pThis->ImplIsFloatingMode();

        pThis->mpData->mbAssumeDocked = sal_False;
        pThis->mpData->mbAssumeFloating = sal_False;

        if ( nCalcMode == TB_CALCMODE_HORZ )
        {
            pThis->mpData->mbAssumeDocked = sal_True;   // force non-floating mode during calculation
            ImplCalcBorder( WINDOWALIGN_TOP, nLeft, nTop, nRight, nBottom, pThis );
            ((ToolBox*)pThis)->mbHorz = sal_True;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_TOP;
        }
        else if ( nCalcMode == TB_CALCMODE_VERT )
        {
            pThis->mpData->mbAssumeDocked = sal_True;   // force non-floating mode during calculation
            ImplCalcBorder( WINDOWALIGN_LEFT, nLeft, nTop, nRight, nBottom, pThis );
            ((ToolBox*)pThis)->mbHorz = sal_False;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_LEFT;
        }
        else if ( nCalcMode == TB_CALCMODE_FLOAT )
        {
            pThis->mpData->mbAssumeFloating = sal_True;   // force non-floating mode during calculation
            nLeft = nTop = nRight = nBottom = 0;
            ((ToolBox*)pThis)->mbHorz = sal_True;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_TOP;
        }

        if ( (pThis->meAlign != eOldAlign) || (pThis->mbHorz != bOldHorz) ||
             (pThis->ImplIsFloatingMode() != bOldFloatingMode ) )
            ((ToolBox*)pThis)->mbCalc = sal_True;
    }
    else
        ImplCalcBorder( pThis->meAlign, nLeft, nTop, nRight, nBottom, pThis );

    ((ToolBox*)pThis)->ImplCalcItem();

    if( !nCalcMode && pThis->ImplIsFloatingMode() )
    {
        aSize = ImplCalcFloatSize( ((ToolBox*)pThis), nCalcLines );
    }
    else
    {
        if ( pThis->mbHorz )
        {
            if ( pThis->mnWinHeight > pThis->mnMaxItemHeight )
                aSize.Height() = nCalcLines * pThis->mnWinHeight;
            else
                aSize.Height() = nCalcLines * pThis->mnMaxItemHeight;

            if ( pThis->mnWinStyle & WB_LINESPACING )
                aSize.Height() += (nCalcLines-1)*TB_LINESPACING;

            if ( pThis->mnWinStyle & WB_BORDER )
                aSize.Height() += (TB_BORDER_OFFSET2*2) + nTop + nBottom;

            nMax = 0;
            ((ToolBox*)pThis)->ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, pThis->mbHorz );
            if ( nMax )
                aSize.Width() += nMax;

            if ( pThis->mnWinStyle & WB_BORDER )
                aSize.Width() += (TB_BORDER_OFFSET1*2) + nLeft + nRight;
        }
        else
        {
            aSize.Width() = nCalcLines * pThis->mnMaxItemWidth;

            if ( pThis->mnWinStyle & WB_LINESPACING )
                aSize.Width() += (nCalcLines-1)*TB_LINESPACING;

            if ( pThis->mnWinStyle & WB_BORDER )
                aSize.Width() += (TB_BORDER_OFFSET2*2) + nLeft + nRight;

            nMax = 0;
            ((ToolBox*)pThis)->ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, pThis->mbHorz );
            if ( nMax )
                aSize.Height() += nMax;

            if ( pThis->mnWinStyle & WB_BORDER )
                aSize.Height() += (TB_BORDER_OFFSET1*2) + nTop + nBottom;
        }
    }
    // restore previous values
    if ( nCalcMode )
    {
        pThis->mpData->mbAssumeDocked = bOldAssumeDocked;
        pThis->mpData->mbAssumeFloating = bOldAssumeFloating;
        if ( (pThis->meAlign != eOldAlign) || (pThis->mbHorz != bOldHorz) )
        {
            ((ToolBox*)pThis)->meAlign  = eOldAlign;
            ((ToolBox*)pThis)->mbHorz   = bOldHorz;
            ((ToolBox*)pThis)->mbCalc   = sal_True;
        }
    }

    if ( aSize.Width() )
        aSize.Width() += pThis->mnBorderX*2;
    if ( aSize.Height() )
        aSize.Height() += pThis->mnBorderY*2;

    return aSize;
}

// -----------------------------------------------------------------------

void ToolBox::ImplCalcFloatSizes( ToolBox* pThis )
{
    if ( pThis->mpFloatSizeAry )
        return;

    // calculate the minimal size, i.e. where the biggest item just fits
    long            nCalcSize = 0;

    std::vector< ImplToolItem >::const_iterator it;
    it = pThis->mpData->m_aItems.begin();
    while ( it != pThis->mpData->m_aItems.end() )
    {
        if ( it->mbVisible )
        {
            if ( it->mpWindow )
            {
                long nTempSize = it->mpWindow->GetSizePixel().Width();
                if ( nTempSize > nCalcSize )
                    nCalcSize = nTempSize;
            }
            else
            {
                if( it->maItemSize.Width() > nCalcSize )
                    nCalcSize = it->maItemSize.Width();
            }
        }
        ++it;
    }

    // calc an upper bound for ImplCalcBreaks below
    long upperBoundWidth = nCalcSize * pThis->mpData->m_aItems.size();

    sal_uInt16  i;
    sal_uInt16  nLines;
    sal_uInt16  nCalcLines;
    sal_uInt16  nTempLines;
    long    nHeight;
    long    nMaxLineWidth;
    nCalcLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, sal_True );

    pThis->mpFloatSizeAry = new ImplToolSizeArray;
    pThis->mpFloatSizeAry->mpSize = new ImplToolSize[nCalcLines];
    pThis->mpFloatSizeAry->mnLength = nCalcLines;

    memset( pThis->mpFloatSizeAry->mpSize, 0, sizeof( ImplToolSize )*nCalcLines );
    i = 0;
    nTempLines = nLines = nCalcLines;
    while ( nLines )
    {
        nHeight = ImplCalcSize( pThis, nTempLines, TB_CALCMODE_FLOAT ).Height();
        pThis->mpFloatSizeAry->mnLastEntry = i;
        pThis->mpFloatSizeAry->mpSize[i].mnHeight = nHeight;
        pThis->mpFloatSizeAry->mpSize[i].mnLines  = nTempLines;
        pThis->mpFloatSizeAry->mpSize[i].mnWidth = nMaxLineWidth+(TB_BORDER_OFFSET1*2);
        nLines--;
        if ( nLines )
        {
            do
            {
                nCalcSize += pThis->mnMaxItemWidth;
                nTempLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, sal_True );
            }
            while ( (nCalcSize < upperBoundWidth) && (nLines < nTempLines) && (nTempLines != 1) );
            if ( nTempLines < nLines )
                nLines = nTempLines;
        }
        i++;
    }
}

// -----------------------------------------------------------------------

Size ToolBox::ImplCalcFloatSize( ToolBox* pThis, sal_uInt16& rLines )
{
    ImplCalcFloatSizes( pThis );

    if ( !rLines )
    {
        rLines = pThis->mnFloatLines;
        if ( !rLines )
            rLines = pThis->mnLines;
    }

    sal_uInt16 i = 0;
    while ( i < pThis->mpFloatSizeAry->mnLastEntry &&
        rLines < pThis->mpFloatSizeAry->mpSize[i].mnLines )
        i++;

    Size aSize( pThis->mpFloatSizeAry->mpSize[i].mnWidth,
                pThis->mpFloatSizeAry->mpSize[i].mnHeight );
    rLines = pThis->mpFloatSizeAry->mpSize[i].mnLines;
    if ( pThis->maNextToolBoxStr.Len() && pThis->mbScroll )
        aSize.Width() += TB_NEXT_SIZE-TB_NEXT_OFFSET;
    return aSize;
}

// -----------------------------------------------------------------------

void ToolBox::ImplCalcMinMaxFloatSize( ToolBox* pThis, Size& rMinSize, Size& rMaxSize )
{
    ImplCalcFloatSizes( pThis );

    sal_uInt16 i = 0;
    rMinSize = Size( pThis->mpFloatSizeAry->mpSize[i].mnWidth, pThis->mpFloatSizeAry->mpSize[i].mnHeight );
    rMaxSize = Size( pThis->mpFloatSizeAry->mpSize[i].mnWidth, pThis->mpFloatSizeAry->mpSize[i].mnHeight );
    while ( ++i <= pThis->mpFloatSizeAry->mnLastEntry )
    {
        if( pThis->mpFloatSizeAry->mpSize[i].mnWidth < rMinSize.Width() )
            rMinSize.Width() = pThis->mpFloatSizeAry->mpSize[i].mnWidth;
        if( pThis->mpFloatSizeAry->mpSize[i].mnHeight < rMinSize.Height() )
            rMinSize.Height() = pThis->mpFloatSizeAry->mpSize[i].mnHeight;

        if( pThis->mpFloatSizeAry->mpSize[i].mnWidth > rMaxSize.Width() )
            rMaxSize.Width() = pThis->mpFloatSizeAry->mpSize[i].mnWidth;
        if( pThis->mpFloatSizeAry->mpSize[i].mnHeight > rMaxSize.Height() )
            rMaxSize.Height() = pThis->mpFloatSizeAry->mpSize[i].mnHeight;
    }
}

void ToolBox::ImplSetMinMaxFloatSize( ToolBox *pThis )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );
    Size aMinSize, aMaxSize;
    ImplCalcMinMaxFloatSize( pThis, aMinSize, aMaxSize );
    if( pWrapper )
    {
        pWrapper->SetMinOutputSizePixel( aMinSize );
        pWrapper->SetMaxOutputSizePixel( aMaxSize );
        pWrapper->ShowTitleButton( TITLE_BUTTON_MENU, ( pThis->GetMenuType() & TOOLBOX_MENUTYPE_CUSTOMIZE) ? sal_True : sal_False );
    }
    else
    {
        // TODO: change SetMinOutputSizePixel to be not inline
        pThis->SetMinOutputSizePixel( aMinSize );
        pThis->SetMaxOutputSizePixel( aMaxSize );
    }
}

// -----------------------------------------------------------------------


sal_uInt16 ToolBox::ImplCalcLines( ToolBox* pThis, long nToolSize )
{
    long nLineHeight;

    if ( pThis->mbHorz )
    {
        if ( pThis->mnWinHeight > pThis->mnMaxItemHeight )
            nLineHeight = pThis->mnWinHeight;
        else
            nLineHeight = pThis->mnMaxItemHeight;
    }
    else
        nLineHeight = pThis->mnMaxItemWidth;

    if ( pThis->mnWinStyle & WB_BORDER )
        nToolSize -= TB_BORDER_OFFSET2*2;

    if ( pThis->mnWinStyle & WB_LINESPACING )
    {
        nLineHeight += TB_LINESPACING;
        nToolSize += TB_LINESPACING;
    }

    // #i91917# always report at least one line
    long nLines = nToolSize/nLineHeight;
    if( nLines < 1 )
        nLines = 1;

    return static_cast<sal_uInt16>(nLines);
}

// -----------------------------------------------------------------------

sal_uInt16 ToolBox::ImplTestLineSize( ToolBox* pThis, const Point& rPos )
{
    if ( !pThis->ImplIsFloatingMode() &&
         (!pThis->mbScroll || (pThis->mnLines > 1) || (pThis->mnCurLines > pThis->mnVisLines)) )
    {
        WindowAlign eAlign = pThis->GetAlign();

        if ( eAlign == WINDOWALIGN_LEFT )
        {
            if ( rPos.X() > pThis->mnDX-DOCK_LINEOFFSET )
                return DOCK_LINEHSIZE | DOCK_LINERIGHT;
        }
        else if ( eAlign == WINDOWALIGN_TOP )
        {
            if ( rPos.Y() > pThis->mnDY-DOCK_LINEOFFSET )
                return DOCK_LINEVSIZE | DOCK_LINEBOTTOM;
        }
        else if ( eAlign == WINDOWALIGN_RIGHT )
        {
            if ( rPos.X() < DOCK_LINEOFFSET )
                return DOCK_LINEHSIZE | DOCK_LINELEFT;
        }
        else if ( eAlign == WINDOWALIGN_BOTTOM )
        {
            if ( rPos.Y() < DOCK_LINEOFFSET )
                return DOCK_LINEVSIZE | DOCK_LINETOP;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void ToolBox::ImplLineSizing( ToolBox* pThis, const Point& rPos, Rectangle& rRect,
                     sal_uInt16 nLineMode )
{
    sal_Bool    mbHorz;
    long    nOneLineSize;
    long    nCurSize;
    long    nMaxSize;
    long    nSize;
    Size    aSize;

    if ( nLineMode & DOCK_LINERIGHT )
    {
        nCurSize = rPos.X() - rRect.Left();
        mbHorz = sal_False;
    }
    else if ( nLineMode & DOCK_LINEBOTTOM )
    {
        nCurSize = rPos.Y() - rRect.Top();
        mbHorz = sal_True;
    }
    else if ( nLineMode & DOCK_LINELEFT )
    {
        nCurSize = rRect.Right() - rPos.X();
        mbHorz = sal_False;
    }
    else if ( nLineMode & DOCK_LINETOP )
    {
        nCurSize = rRect.Bottom() - rPos.Y();
        mbHorz = sal_True;
    }
    else {
        OSL_FAIL( "ImplLineSizing: Trailing else" );
        nCurSize = 0;
        mbHorz = sal_False;
    }

    Size    aWinSize = pThis->GetSizePixel();
    sal_uInt16  nMaxLines = (pThis->mnLines > pThis->mnCurLines) ? pThis->mnLines : pThis->mnCurLines;
    if ( nMaxLines > TB_MAXLINES )
        nMaxLines = TB_MAXLINES;
    if ( mbHorz )
    {
        nOneLineSize = ImplCalcSize( pThis, 1 ).Height();
        nMaxSize = pThis->maOutDockRect.GetHeight() - 20;
        if ( nMaxSize < aWinSize.Height() )
            nMaxSize = aWinSize.Height();
    }
    else
    {
        nOneLineSize = ImplCalcSize( pThis, 1 ).Width();
        nMaxSize = pThis->maOutDockRect.GetWidth() - 20;
        if ( nMaxSize < aWinSize.Width() )
            nMaxSize = aWinSize.Width();
    }

    sal_uInt16 i = 1;
    if ( nCurSize <= nOneLineSize )
        nSize = nOneLineSize;
    else
    {
        nSize = 0;
        while ( (nSize < nCurSize) && (i < nMaxLines) )
        {
            i++;
            aSize = ImplCalcSize( pThis, i );
            if ( mbHorz )
                nSize = aSize.Height();
            else
                nSize = aSize.Width();
            if ( nSize > nMaxSize )
            {
                i--;
                aSize = ImplCalcSize( pThis, i );
                if ( mbHorz )
                    nSize = aSize.Height();
                else
                    nSize = aSize.Width();
                break;
            }
        }
    }

    if ( nLineMode & DOCK_LINERIGHT )
        rRect.Right() = rRect.Left()+nSize-1;
    else if ( nLineMode & DOCK_LINEBOTTOM )
        rRect.Bottom() = rRect.Top()+nSize-1;
    else if ( nLineMode & DOCK_LINELEFT )
        rRect.Left() = rRect.Right()-nSize;
    else
        rRect.Top() = rRect.Bottom()-nSize;

    pThis->mnDockLines = i;
}

// -----------------------------------------------------------------------

sal_uInt16 ToolBox::ImplFindItemPos( ToolBox* pBox, const Point& rPos )
{
    sal_uInt16  nPos = 0;
    long    nLast = 0;
    Point   aPos = rPos;
    Size    aSize( pBox->mnDX, pBox->mnDY );

    if ( aPos.X() > aSize.Width()-TB_BORDER_OFFSET1 )
        aPos.X() = aSize.Width()-TB_BORDER_OFFSET1;
    if ( aPos.Y() > aSize.Height()-TB_BORDER_OFFSET1 )
        aPos.Y() = aSize.Height()-TB_BORDER_OFFSET1;

    // Item suchen, das geklickt wurde
    std::vector< ImplToolItem >::const_iterator it = pBox->mpData->m_aItems.begin();
    while ( it != pBox->mpData->m_aItems.end() )
    {
        if ( it->mbVisible )
        {
            if ( nLast || !it->maRect.IsEmpty() )
            {
                if ( pBox->mbHorz )
                {
                    if ( nLast &&
                         ((nLast < it->maRect.Top()) || it->maRect.IsEmpty()) )
                        return nPos;

                    if ( aPos.Y() <= it->maRect.Bottom() )
                    {
                        if ( aPos.X() < it->maRect.Left() )
                            return nPos;
                        else if ( aPos.X() < it->maRect.Right() )
                            return nPos+1;
                        else if ( !nLast )
                            nLast = it->maRect.Bottom();
                    }
                }
                else
                {
                    if ( nLast &&
                         ((nLast < it->maRect.Left()) || it->maRect.IsEmpty()) )
                        return nPos;

                    if ( aPos.X() <= it->maRect.Right() )
                    {
                        if ( aPos.Y() < it->maRect.Top() )
                            return nPos;
                        else if ( aPos.Y() < it->maRect.Bottom() )
                            return nPos+1;
                        else if ( !nLast )
                            nLast = it->maRect.Right();
                    }
                }
            }
        }

        nPos++;
        ++it;
    }

    return nPos;
}

// -----------------------------------------------------------------------

ImplTBDragMgr::ImplTBDragMgr()
{
    mpBoxList       = new ImplTBList();
    mnLineMode      = 0;
    mnStartLines    = 0;
    mbCustomizeMode = sal_False;
    mbResizeMode    = sal_False;
    mbShowDragRect  = sal_False;
    mpDragBox       = NULL;

    maAccel.InsertItem( KEY_RETURN, KeyCode( KEY_RETURN ) );
    maAccel.InsertItem( KEY_ESCAPE, KeyCode( KEY_ESCAPE ) );
    maAccel.SetSelectHdl( LINK( this, ImplTBDragMgr, SelectHdl ) );
}

// -----------------------------------------------------------------------

ImplTBDragMgr::~ImplTBDragMgr()
{
    delete mpBoxList;
}

// -----------------------------------------------------------------------

ToolBox* ImplTBDragMgr::FindToolBox( const Rectangle& rRect )
{
    for ( size_t i = 0, n = mpBoxList->size(); i < n; ++i )
    {
        ToolBox* pBox = (*mpBoxList)[ i ];
        /*
         *  FIXME: since we can have multiple frames now we cannot
         *  find the drag target by its position alone.
         *  As long as the toolbar config dialogue is not a system window
         *  this works in one frame only anyway. If the dialogue
         *  changes to a system window, we need a new implementation here
         */
        if (  pBox->IsReallyVisible()
           && pBox->ImplGetWindowImpl()->mpFrame == mpDragBox->ImplGetWindowImpl()->mpFrame
        ) {
            if ( !pBox->ImplIsFloatingMode() )
            {
                Point aPos = pBox->GetPosPixel();
                aPos = pBox->GetParent()->OutputToScreenPixel( aPos );
                Rectangle aTempRect( aPos, pBox->GetSizePixel() );
                if ( aTempRect.IsOver( rRect ) )
                    return pBox;
            }
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::StartDragging( ToolBox* pToolBox,
                                   const Point& rPos, const Rectangle& rRect,
                                   sal_uInt16 nDragLineMode, sal_Bool bResizeItem,
                                   void* pData )
{
    mpDragBox = pToolBox;
    pToolBox->CaptureMouse();
    pToolBox->mbDragging = sal_True;
    Application::InsertAccel( &maAccel );

    if ( nDragLineMode )
    {
        mnLineMode = nDragLineMode;
        mnStartLines = pToolBox->mnDockLines;
    }
    else
    {
        mpCustomizeData = pData;
        mbResizeMode = bResizeItem;
        pToolBox->Activate();
        pToolBox->mnCurItemId = pToolBox->mnConfigItem;
        pToolBox->Highlight();
        pToolBox->mnCurItemId = 0;
        if ( mbResizeMode )
        {
            if ( rRect.GetWidth() < TB_MIN_WIN_WIDTH )
                mnMinWidth = rRect.GetWidth();
            else
                mnMinWidth = TB_MIN_WIN_WIDTH;
            mnMaxWidth = pToolBox->GetSizePixel().Width()-rRect.Left()-
                         TB_SPIN_SIZE-TB_BORDER_OFFSET1-(TB_SPIN_OFFSET*2);
        }
    }

    // MouseOffset berechnen
    maMouseOff.X() = rRect.Left() - rPos.X();
    maMouseOff.Y() = rRect.Top() - rPos.Y();
    maRect = rRect;
    maStartRect = rRect;
    mbShowDragRect = sal_True;
    pToolBox->ShowTracking( maRect );
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::Dragging( const Point& rPos )
{
    if ( mnLineMode )
    {
        ToolBox::ImplLineSizing( mpDragBox, rPos, maRect, mnLineMode );
        Point aOff = mpDragBox->OutputToScreenPixel( Point() );
        maRect.Move( aOff.X(), aOff.Y() );
        mpDragBox->Docking( rPos, maRect );
        maRect.Move( -aOff.X(), -aOff.Y() );
        mpDragBox->ShowTracking( maRect );
    }
    else
    {
        if ( mbResizeMode )
        {
            long nXOff = rPos.X()-maStartRect.Left();
            nXOff += maMouseOff.X()+(maStartRect.Right()-maStartRect.Left());
            if ( nXOff < mnMinWidth )
                nXOff = mnMinWidth;
            if ( nXOff > mnMaxWidth )
                nXOff = mnMaxWidth;
            maRect.Right() = maStartRect.Left()+nXOff;
        }
        else
        {
            maRect.SetPos( rPos );
            maRect.Move( maMouseOff.X(), maMouseOff.Y() );
        }
        mpDragBox->ShowTracking( maRect );
    }
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::EndDragging( sal_Bool bOK )
{
    mpDragBox->HideTracking();
    mpDragBox->ReleaseMouse();
    mpDragBox->mbDragging = sal_False;
    mbShowDragRect = sal_False;
    Application::RemoveAccel( &maAccel );

    if ( mnLineMode )
    {
        if ( !bOK )
        {
            mpDragBox->mnDockLines = mnStartLines;
            mpDragBox->EndDocking( maStartRect, sal_False );
        }
        else
            mpDragBox->EndDocking( maRect, sal_False );
        mnLineMode = 0;
        mnStartLines = 0;
    }
    else
    {
        sal_uInt16 nTempItem = mpDragBox->mnConfigItem;
        if ( nTempItem )
        {
            mpDragBox->mnConfigItem = 0;
            if ( !mbResizeMode )
                mpDragBox->Invalidate( mpDragBox->GetItemRect( nTempItem ) );
        }

        if ( bOK && (maRect != maStartRect) )
        {
            if ( mbResizeMode )
            {
                ImplToolItem* pItem = mpDragBox->ImplGetItem( nTempItem );
                Size aSize = pItem->mpWindow->GetSizePixel();
                aSize.Width() = maRect.GetWidth();
                pItem->mpWindow->SetSizePixel( aSize );

                // ToolBox neu brechnen und neu ausgeben
                mpDragBox->ImplInvalidate( sal_True );
                mpDragBox->Customize( ToolBoxCustomizeEvent( mpDragBox, nTempItem,
                                                             TOOLBOX_CUSTOMIZE_RESIZE,
                                                             mpCustomizeData ) );
            }
            else
            {
                Point aOff = mpDragBox->OutputToScreenPixel( Point() );
                Rectangle aScreenRect( maRect );
                aScreenRect.Move( aOff.X(), aOff.Y() );
                ToolBox* pDropBox = FindToolBox( aScreenRect );
                if ( pDropBox )
                {
                    // Such-Position bestimmen
                    Point aPos;
                    if ( pDropBox->mbHorz )
                    {
                        aPos.X() = aScreenRect.Left()-TB_CUSTOMIZE_OFFSET;
                        aPos.Y() = aScreenRect.Center().Y();
                    }
                    else
                    {
                        aPos.X() = aScreenRect.Center().X();
                        aPos.Y() = aScreenRect.Top()-TB_CUSTOMIZE_OFFSET;
                    }

                    aPos = pDropBox->ScreenToOutputPixel( aPos );
                    sal_uInt16 nPos = ToolBox::ImplFindItemPos( pDropBox, aPos );
                    mpDragBox->Customize( ToolBoxCustomizeEvent( pDropBox, nTempItem,
                                                                 nPos, mpCustomizeData ) );
                }
                else
                {
                    mpDragBox->Customize( ToolBoxCustomizeEvent( NULL, nTempItem,
                                                                 0, mpCustomizeData ) );
                }
            }
        }
        mpCustomizeData = NULL;
        mbResizeMode = sal_False;
        mpDragBox->Deactivate();
    }

    mpDragBox = NULL;
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::UpdateDragRect()
{
    // Nur Updaten, wenn wir schon im Dragging sind
    if ( !mbShowDragRect )
        return;

    mpDragBox->ShowTracking( maRect );
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplTBDragMgr, SelectHdl, Accelerator*, pAccel )
{
    if ( pAccel->GetCurItemId() == KEY_ESCAPE )
        EndDragging( sal_False );
    else
        EndDragging( sal_True );

    return sal_True;
}

// -----------------------------------------------------------------------

static void ImplDrawOutButton( OutputDevice* pOutDev, const Rectangle& rRect,
                               sal_uInt16 nStyle )
{
    const StyleSettings&    rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
    Color                   aShadowColor = rStyleSettings.GetShadowColor();
    Point                   aPos( rRect.TopLeft() );
    Size                    aSize( rRect.GetSize() );
    long                    nOffset = 0;

    if ( pOutDev->GetBackground().GetColor() == aShadowColor )
        aShadowColor = rStyleSettings.GetDarkShadowColor();

    if ( nStyle & BUTTON_DRAW_PRESSED )
    {
        aPos.X()++;
        aPos.Y()++;
        nOffset++;
    }

    // Hintergrund loeschen
    pOutDev->Erase( rRect );

    // Button zeichnen
    pOutDev->SetLineColor( rStyleSettings.GetLightColor() );
    pOutDev->DrawLine( aPos,
                       Point( aPos.X()+aSize.Width()-OUTBUTTON_BORDER, aPos.Y() ) );
    pOutDev->DrawLine( aPos,
                       Point( aPos.X(), aPos.Y()+aSize.Height()-OUTBUTTON_BORDER ) );
    pOutDev->SetLineColor( aShadowColor );
    pOutDev->DrawLine( Point( aPos.X()+aSize.Width()-OUTBUTTON_BORDER, aPos.Y() ),
                       Point( aPos.X()+aSize.Width()-OUTBUTTON_BORDER, aPos.Y()+aSize.Height()-OUTBUTTON_BORDER ) );
    pOutDev->DrawLine( Point( aPos.X(), aPos.Y()+aSize.Height()-OUTBUTTON_BORDER ),
                       Point( aPos.X()+aSize.Width()-OUTBUTTON_BORDER, aPos.Y()+aSize.Height()-OUTBUTTON_BORDER ) );
    for ( long i = 0; i < OUTBUTTON_BORDER-1-nOffset; i++ )
    {
        pOutDev->DrawLine( Point( aPos.X()+aSize.Width()-(OUTBUTTON_BORDER-i-1), aPos.Y()+OUTBUTTON_BORDER ),
                           Point( aPos.X()+aSize.Width()-(OUTBUTTON_BORDER-i-1), aPos.Y()+aSize.Height()-1 ) );
        pOutDev->DrawLine( Point( aPos.X()+OUTBUTTON_BORDER, aPos.Y()+aSize.Height()-(OUTBUTTON_BORDER-i-1) ),
                           Point( aPos.X()+aSize.Width()-1, aPos.Y()+aSize.Height()-(OUTBUTTON_BORDER-i-1) )  );
    }
}


// -----------------------------------------------------------------------

void ToolBox::ImplInit( Window* pParent, WinBits nStyle )
{

    // Variablen initialisieren
    ImplGetWindowImpl()->mbToolBox         = sal_True;
    mpBtnDev          = NULL;
    mpFloatSizeAry    = NULL;
    mpData                = new ImplToolBoxPrivateData;
    mpFloatWin        = NULL;
    mnDX              = 0;
    mnDY              = 0;
    mnMaxItemWidth       = 0;
    mnMaxItemHeight      = 0;
    mnWinHeight       = 0;
    mnBorderX         = 0;
    mnBorderY         = 0;
    mnLeftBorder      = 0;
    mnTopBorder       = 0;
    mnRightBorder     = 0;
    mnBottomBorder    = 0;
    mnLastResizeDY    = 0;
    mnOutStyle        = TOOLBOX_STYLE_FLAT; // force flat buttons since NWF
    mnHighItemId      = 0;
    mnCurItemId       = 0;
    mnDownItemId      = 0;
    mnCurPos          = TOOLBOX_ITEM_NOTFOUND;
    mnFocusPos        = TOOLBOX_ITEM_NOTFOUND;    // current position during keyboard access
    mnLines           = 1;
    mnCurLine         = 1;
    mnCurLines        = 1;
    mnVisLines        = 1;
    mnFloatLines      = 0;
    mnConfigItem      = 0;
    mnMouseClicks     = 0;
    mnMouseModifier   = 0;
    mbDrag            = sal_False;
    mbSelection       = sal_False;
    mbCommandDrag     = sal_False;
    mbUpper           = sal_False;
    mbLower           = sal_False;
    mbNextTool        = sal_False;
    mbIn              = sal_False;
    mbCalc            = sal_True;
    mbFormat          = sal_False;
    mbFullPaint       = sal_False;
    mbHorz            = sal_True;
    mbScroll          = (nStyle & WB_SCROLL) != 0;
    mbCustomize       = sal_False;
    mbCustomizeMode   = sal_False;
    mbDragging        = sal_False;
    mbMenuStrings     = sal_False;
    mbIsShift          = sal_False;
    mbIsKeyEvent = sal_False;
    mbChangingHighlight = sal_False;
    meButtonType      = BUTTON_SYMBOL;
    meAlign           = WINDOWALIGN_TOP;
    meLastStyle       = POINTER_ARROW;
    mnWinStyle        = nStyle;
    meLayoutMode      = TBX_LAYOUT_NORMAL;
    mnLastFocusItemId          = 0;
    mnKeyModifier      = 0;
    mnActivateCount   = 0;

    maTimer.SetTimeout( 50 );
    maTimer.SetTimeoutHdl( LINK( this, ToolBox, ImplUpdateHdl ) );

    // set timeout and handler for dropdown items
    mpData->maDropdownTimer.SetTimeout( 250 );
    mpData->maDropdownTimer.SetTimeoutHdl( LINK( this, ToolBox, ImplDropdownLongClickHdl ) );

    DockingWindow::ImplInit( pParent, nStyle & ~(WB_BORDER) );


    // always set WB_TABSTOP for ToolBars !!!  if( mnWinStyle & WB_TABSTOP )
    {
        // dockingwindow's ImplInit removes some bits, so restore them here
        // to allow keyboard handling for toolbars
        ImplGetWindowImpl()->mnStyle |= WB_TABSTOP|WB_NODIALOGCONTROL;
        ImplGetWindowImpl()->mnStyle &= ~WB_DIALOGCONTROL;
    }

    ImplInitSettings( sal_True, sal_True, sal_True );
}

// -----------------------------------------------------------------------

void ToolBox::ImplInitSettings( sal_Bool bFont,
                                sal_Bool bForeground, sal_Bool bBackground )
{
    mpData->mbNativeButtons = IsNativeControlSupported( CTRL_TOOLBAR, PART_BUTTON );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetToolFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else if ( Window::GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetButtonTextColor();
        else
            aColor = rStyleSettings.GetWindowTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
        {
            aColor = GetControlBackground();
            SetBackground( aColor );
            SetPaintTransparent( sal_False );
            SetParentClipMode( 0 );
        }
        else
        {
            if( IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL ) )
            {
                SetBackground();
                SetPaintTransparent( sal_True );
                SetParentClipMode( PARENTCLIPMODE_NOCLIP );
                mpData->maDisplayBackground = Wallpaper( rStyleSettings.GetFaceColor() );
            }
            else
            {
                if ( Window::GetStyle() & WB_3DLOOK )
                    aColor = rStyleSettings.GetFaceColor();
                else
                    aColor = rStyleSettings.GetWindowColor();

                SetBackground( aColor );
                SetPaintTransparent( sal_False );
                SetParentClipMode( 0 );

                ImplUpdateImageList();
            }
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplLoadRes( const ResId& rResId )
{
    ResMgr* pMgr = rResId.GetResMgr();
    if( ! pMgr )
        return;

    DockingWindow::ImplLoadRes( rResId );

    sal_uLong              nObjMask;

    nObjMask = ReadLongRes();

    if ( nObjMask & RSC_TOOLBOX_BUTTONTYPE )
        SetButtonType( (ButtonType)ReadLongRes() );

    if ( nObjMask & RSC_TOOLBOX_ALIGN )
        SetAlign( (WindowAlign)ReadLongRes() );

    if ( nObjMask & RSC_TOOLBOX_LINECOUNT )
        SetLineCount( sal::static_int_cast<sal_uInt16>(ReadLongRes()) );

    if ( nObjMask & RSC_TOOLBOX_CUSTOMIZE )
    {
        sal_Bool bCust = (sal_Bool)ReadShortRes();
        EnableCustomize( bCust );
    }

    if ( nObjMask & RSC_TOOLBOX_MENUSTRINGS )
    {
        sal_Bool bCust = (sal_Bool)ReadShortRes();
        EnableMenuStrings( bCust );
    }

    if ( nObjMask & RSC_TOOLBOX_FLOATLINES )
        SetFloatingLines( ReadShortRes() );

    if ( nObjMask & RSC_TOOLBOX_ITEMIMAGELIST )
    {
        maImageList = ImageList( ResId( (RSHEADER_TYPE*)GetClassRes(), *pMgr ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }

    if ( nObjMask & RSC_TOOLBOX_ITEMLIST )
    {
        sal_uLong nEle = ReadLongRes();

        // Item hinzufuegen
        for ( sal_uLong i = 0; i < nEle; i++ )
        {
            InsertItem( ResId( (RSHEADER_TYPE *)GetClassRes(), *pMgr ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
    }
}

// -----------------------------------------------------------------------

ToolBox::ToolBox( Window* pParent, WinBits nStyle ) :
    DockingWindow( WINDOW_TOOLBOX )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ToolBox::ToolBox( Window* pParent, const ResId& rResId ) :
    DockingWindow( WINDOW_TOOLBOX )
{
    RTL_LOGFILE_CONTEXT( aLog, "vcl: ToolBox::ToolBox( Window* pParent, const ResId& rResId )" );

    rResId.SetRT( RSC_TOOLBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    // Groesse des FloatingWindows berechnen und umschalten, wenn die
    // ToolBox initial im FloatingModus ist
    if ( ImplIsFloatingMode() )
        mbHorz = sal_True;
    else
        Resize();

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

ToolBox::~ToolBox()
{
    // custom menu event still running?
    if( mpData->mnEventId )
        Application::RemoveUserEvent( mpData->mnEventId );

    // #103005# make sure our activate/deactivate balance is right
    while( mnActivateCount > 0 )
        Deactivate();

    // Falls noch ein Floating-Window connected ist, dann den
    // PopupModus beenden
    if ( mpFloatWin )
        mpFloatWin->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );

    // delete private data
    delete mpData;

    // FloatSizeAry gegebenenfalls loeschen
    delete mpFloatSizeAry;

    // Wenn keine ToolBox-Referenzen mehr auf die Listen bestehen, dann
    // Listen mit wegloeschen
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maCtrlData.mpTBDragMgr )
    {
        // Wenn im TBDrag-Manager, dann wieder rausnehmen
        if ( mbCustomize )
            pSVData->maCtrlData.mpTBDragMgr->erase( this );

        if ( !pSVData->maCtrlData.mpTBDragMgr->size() )
        {
            delete pSVData->maCtrlData.mpTBDragMgr;
            pSVData->maCtrlData.mpTBDragMgr = NULL;
        }
    }
}

// -----------------------------------------------------------------------

ImplToolItem* ToolBox::ImplGetItem( sal_uInt16 nItemId ) const
{
    std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        if ( it->mnId == nItemId )
            return &(*it);
        ++it;
    }

    return NULL;
}
// -----------------------------------------------------------------------

static void ImplAddButtonBorder( long &rWidth, long& rHeight, sal_uInt16 aOutStyle, sal_Bool bNativeButtons )
{
    if ( aOutStyle & TOOLBOX_STYLE_OUTBUTTON )
    {
        rWidth += OUTBUTTON_SIZE;
        rHeight += OUTBUTTON_SIZE;
    }
    else
    {
        rWidth += SMALLBUTTON_HSIZE;
        rHeight += SMALLBUTTON_VSIZE;
    }

    if( bNativeButtons )
    {
        // give more border space for rounded buttons
        rWidth += 2;
        rHeight += 4;
    }
}

// -----------------------------------------------------------------------

sal_Bool ToolBox::ImplCalcItem()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // recalc required ?
    if ( !mbCalc )
        return sal_False;

    ImplDisableFlatButtons();

    long            nDefWidth;
    long            nDefHeight;
    long            nMaxWidth = 0;
    long            nMaxHeight = 0;
    long            nMinWidth   = 6;
    long            nMinHeight  = 6;
    long            nDropDownArrowWidth = TB_DROPDOWNARROWWIDTH;

    // set defaults if image or text is needed but empty
    nDefWidth       = GetDefaultImageSize().Width();
    nDefHeight      = GetDefaultImageSize().Height();

    mnWinHeight = 0;
    // determine minimum size necessary in NWF
    {
        Rectangle aRect( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        Rectangle aReg( aRect );
        ImplControlValue aVal;
        Rectangle aNativeBounds, aNativeContent;
        if( IsNativeControlSupported( CTRL_TOOLBAR, PART_BUTTON ) )
        {
            if( GetNativeControlRegion( CTRL_TOOLBAR, PART_BUTTON,
                                        aReg,
                                        CTRL_STATE_ENABLED | CTRL_STATE_ROLLOVER,
                                        aVal, OUString(),
                                        aNativeBounds, aNativeContent ) )
            {
                aRect = aNativeBounds;
                if( aRect.GetWidth() > nMinWidth )
                    nMinWidth = aRect.GetWidth();
                if( aRect.GetHeight() > nMinHeight )
                    nMinHeight = aRect.GetHeight();
                if( nDropDownArrowWidth < nMinWidth )
                    nDropDownArrowWidth = nMinWidth;
                if( nMinWidth > mpData->mnMenuButtonWidth )
                    mpData->mnMenuButtonWidth = nMinWidth;
                else if( nMinWidth < TB_MENUBUTTON_SIZE )
                    mpData->mnMenuButtonWidth = TB_MENUBUTTON_SIZE;
            }
        }

        // also calculate the area for comboboxes, drop down list boxes and spinfields
        // as these are often inserted into toolboxes; set mnWinHeight to the
        // greater of those values to prevent toolbar flickering (#i103385#)
        aRect = Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( CTRL_COMBOBOX, PART_ENTIRE_CONTROL,
                                    aReg,
                                    CTRL_STATE_ENABLED | CTRL_STATE_ROLLOVER,
                                    aVal, OUString(),
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
        aRect = Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( CTRL_LISTBOX, PART_ENTIRE_CONTROL,
                                    aReg,
                                    CTRL_STATE_ENABLED | CTRL_STATE_ROLLOVER,
                                    aVal, OUString(),
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
        aRect = Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( CTRL_SPINBOX, PART_ENTIRE_CONTROL,
                                    aReg,
                                    CTRL_STATE_ENABLED | CTRL_STATE_ROLLOVER,
                                    aVal, OUString(),
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
    }

    if ( ! mpData->m_aItems.empty() )
    {
        std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            sal_Bool bImage;
            sal_Bool bText;

            it->mbVisibleText = sal_False;  // indicates if text will definitely be drawn, influences dropdown pos

            if ( it->meType == TOOLBOXITEM_BUTTON )
            {
                // check if image and/or text exists
                if ( !(it->maImage) )
                    bImage = sal_False;
                else
                    bImage = sal_True;
                if ( !it->maText.Len() )
                    bText = sal_False;
                else
                    bText = sal_True;
                ButtonType tmpButtonType = determineButtonType( &(*it), meButtonType ); // default to toolbox setting
                if ( bImage || bText )
                {

                    it->mbEmptyBtn = sal_False;

                    if ( tmpButtonType == BUTTON_SYMBOL )
                    {
                        // we're drawing images only
                        if ( bImage || !bText )
                        {
                            it->maItemSize = it->maImage.GetSizePixel();
                        }
                        else
                        {
                            it->maItemSize = Size( GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET,
                                                   GetTextHeight() );
                            it->mbVisibleText = sal_True;
                        }
                    }
                    else if ( tmpButtonType == BUTTON_TEXT )
                    {
                        // we're drawing text only
                        if ( bText || !bImage )
                        {
                            it->maItemSize = Size( GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET,
                                                   GetTextHeight() );
                            it->mbVisibleText = sal_True;
                        }
                        else
                        {
                            it->maItemSize = it->maImage.GetSizePixel();
                        }
                    }
                    else
                    {
                        // we're drawing images and text
                        it->maItemSize.Width() = bText ? GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET : 0;
                        it->maItemSize.Height() = bText ? GetTextHeight() : 0;

                        // leave space between image and text
                        if( bText )
                            it->maItemSize.Width() += TB_IMAGETEXTOFFSET;

                        // image and text side by side
                        it->maItemSize.Width() += it->maImage.GetSizePixel().Width();
                        if ( it->maImage.GetSizePixel().Height() > it->maItemSize.Height() )
                            it->maItemSize.Height() = it->maImage.GetSizePixel().Height();

                        it->mbVisibleText = bText;
                    }
                }
                else
                {   // no image and no text
                    it->maItemSize = Size( nDefWidth, nDefHeight );
                    it->mbEmptyBtn = sal_True;
                }

                // Gegebenenfalls die Fensterhoehe mit beruecksichtigen
                if ( it->mpWindow )
                {
                    long nHeight = it->mpWindow->GetSizePixel().Height();
                    if ( nHeight > mnWinHeight )
                        mnWinHeight = nHeight;
                }

                // add in drop down arrow
                if( it->mnBits & TIB_DROPDOWN )
                {
                    it->maItemSize.Width() += nDropDownArrowWidth;
                    it->mnDropDownArrowWidth = nDropDownArrowWidth;
                }

                // text items will be rotated in vertical mode
                // -> swap width and height
                if( it->mbVisibleText && !mbHorz )
                {
                    long tmp = it->maItemSize.Width();
                    it->maItemSize.Width() = it->maItemSize.Height();
                    it->maItemSize.Height() = tmp;
                }
            }
            else if ( it->meType == TOOLBOXITEM_SPACE )
            {
                it->maItemSize = Size( nDefWidth, nDefHeight );
            }

            if ( it->meType == TOOLBOXITEM_BUTTON || it->meType == TOOLBOXITEM_SPACE )
            {
                // add borders
                ImplAddButtonBorder( it->maItemSize.Width(), it->maItemSize.Height(), mnOutStyle, mpData->mbNativeButtons );

                if( it->meType == TOOLBOXITEM_BUTTON )
                {
                    if( it->maItemSize.Width() < nMinWidth )
                        it->maItemSize.Width() = nMinWidth;
                    if( it->maItemSize.Height() < nMinHeight )
                        it->maItemSize.Height() = nMinHeight;
                }

                // keep track of max item size
                if ( it->maItemSize.Width() > nMaxWidth )
                    nMaxWidth = it->maItemSize.Width();
                if ( it->maItemSize.Height() > nMaxHeight )
                    nMaxHeight = it->maItemSize.Height();
            }

            ++it;
        }
    }
    else
    {
        nMaxWidth  = nDefWidth;
        nMaxHeight = nDefHeight;

        ImplAddButtonBorder( nMaxWidth, nMaxHeight, mnOutStyle, mpData->mbNativeButtons );
    }

    if( !ImplIsFloatingMode() && GetToolboxButtonSize() != TOOLBOX_BUTTONSIZE_DONTCARE )
    {
        // make sure all vertical toolbars have the same width and horizontal have the same height
        // this depends on the used button sizes
        // as this is used for alignement of multiple toolbars
        // it is only required for docked toolbars

        long nFixedWidth = nDefWidth+nDropDownArrowWidth;
        long nFixedHeight = nDefHeight;
        ImplAddButtonBorder( nFixedWidth, nFixedHeight, mnOutStyle, mpData->mbNativeButtons );

        if( mbHorz )
            nMaxHeight = nFixedHeight;
        else
            nMaxWidth = nFixedWidth;
    }

    mbCalc = sal_False;
    mbFormat = sal_True;

    // do we have to recalc the sizes ?
    if ( (nMaxWidth != mnMaxItemWidth) || (nMaxHeight != mnMaxItemHeight) )
    {
        mnMaxItemWidth  = nMaxWidth;
        mnMaxItemHeight = nMaxHeight;

        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_uInt16 ToolBox::ImplCalcBreaks( long nWidth, long* pMaxLineWidth, sal_Bool bCalcHorz )
{
    sal_uLong           nLineStart = 0;
    sal_uLong           nGroupStart = 0;
    long            nLineWidth = 0;
    long            nCurWidth;
    long            nLastGroupLineWidth = 0;
    long            nMaxLineWidth = 0;
    sal_uInt16          nLines = 1;
    sal_Bool            bWindow;
    sal_Bool            bBreak = sal_False;
    long            nWidthTotal = nWidth;
    long nMenuWidth = 0;

    // when docked the menubutton will be in the first line
    if( IsMenuEnabled() && !ImplIsFloatingMode() )
        nMenuWidth = mpData->maMenubuttonItem.maItemSize.Width();

    // we need to know which item is the last visible one to be able to add
    // the menu width in case we are unable to show all the items
    std::vector< ImplToolItem >::iterator it, lastVisible;
    for ( it = mpData->m_aItems.begin(); it != mpData->m_aItems.end(); ++it )
    {
        if ( it->mbVisible )
            lastVisible = it;
    }

    it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        it->mbBreak = bBreak;
        bBreak = sal_False;

        if ( it->mbVisible )
        {
            bWindow     = sal_False;
            bBreak      = sal_False;
            nCurWidth   = 0;

            if ( it->meType == TOOLBOXITEM_BUTTON || it->meType == TOOLBOXITEM_SPACE )
            {
                if ( bCalcHorz )
                    nCurWidth = it->maItemSize.Width();
                else
                    nCurWidth = it->maItemSize.Height();

                if ( it->mpWindow && bCalcHorz )
                {
                    long nWinItemWidth = it->mpWindow->GetSizePixel().Width();
                    if ( !mbScroll || (nWinItemWidth <= nWidthTotal) )
                    {
                        nCurWidth = nWinItemWidth;
                        bWindow   = sal_True;
                    }
                    else
                    {
                        if ( it->mbEmptyBtn )
                        {
                            nCurWidth = 0;
                        }
                    }
                }

                // in case we are able to show all the items, we do not want
                // to show the toolbar's menu; otherwise yes
                if ( ( ( it == lastVisible ) && (nLineWidth+nCurWidth > nWidthTotal) && mbScroll ) ||
                     ( ( it != lastVisible ) && (nLineWidth+nCurWidth+nMenuWidth > nWidthTotal) && mbScroll ) )
                    bBreak = sal_True;
            }
            else if ( it->meType == TOOLBOXITEM_SEPARATOR )
            {
                nCurWidth = it->mnSepSize;
                if ( !ImplIsFloatingMode() && ( it != lastVisible ) && (nLineWidth+nCurWidth+nMenuWidth > nWidthTotal) )
                    bBreak = sal_True;
            }
            // treat breaks as separators, except when using old style toolbars (ie. no menu button)
            else if ( (it->meType == TOOLBOXITEM_BREAK) && !IsMenuEnabled() )
                bBreak = sal_True;

            if ( bBreak )
            {
                nLines++;

                // Gruppe auseinanderbrechen oder ganze Gruppe umbrechen?
                if ( (it->meType == TOOLBOXITEM_BREAK) ||
                     (nLineStart == nGroupStart) )
                {
                    if ( nLineWidth > nMaxLineWidth )
                        nMaxLineWidth = nLineWidth;

                    nLineWidth = 0;
                    nLineStart = it - mpData->m_aItems.begin();
                    nGroupStart = nLineStart;
                    it->mbBreak = sal_True;
                    bBreak = sal_False;
                }
                else
                {
                    if ( nLastGroupLineWidth > nMaxLineWidth )
                        nMaxLineWidth = nLastGroupLineWidth;

                    // Wenn ganze Gruppe umgebrochen wird, diese auf
                    // Zeilenanfang setzen und wieder neu berechnen
                    nLineWidth = 0;
                    nLineStart = nGroupStart;
                    it = mpData->m_aItems.begin() + nGroupStart;
                    continue;
                }
            }
            else
            {
                if( ImplIsFloatingMode() || !IsMenuEnabled() ) // no group breaking when being docked single-line
                {
                    if ( (it->meType != TOOLBOXITEM_BUTTON) || bWindow )
                    {
                        // found separator or break
                        nLastGroupLineWidth = nLineWidth;
                        nGroupStart = it - mpData->m_aItems.begin();
                        if ( !bWindow )
                            nGroupStart++;
                    }
                }
            }

            nLineWidth += nCurWidth;
        }

        ++it;
    }


    if ( pMaxLineWidth )
    {
        if ( nLineWidth > nMaxLineWidth )
            nMaxLineWidth = nLineWidth;

        if( ImplIsFloatingMode() && !ImplIsInPopupMode() )
        {
            // leave enough space to display buttons in the decoration
            long aMinWidth = 2 * GetSettings().GetStyleSettings().GetFloatTitleHeight();
            if( nMaxLineWidth < aMinWidth )
                nMaxLineWidth = aMinWidth;
        }
        *pMaxLineWidth = nMaxLineWidth;
    }

    return nLines;
}

// -----------------------------------------------------------------------

Size ToolBox::ImplGetOptimalFloatingSize( FloatingSizeMode eMode )
{
    if( !ImplIsFloatingMode() )
        return Size();

    Size aCurrentSize( mnDX, mnDY );
    Size aSize1( aCurrentSize );
    Size aSize2( aCurrentSize );

    // try to preserve current height
    if( eMode == FSMODE_AUTO || eMode == FSMODE_FAVOURHEIGHT )
    {
        // calc number of floating lines for current window height
        sal_uInt16 nFloatLinesHeight = ImplCalcLines( this, mnDY );
        // calc window size according to this number
        aSize1 = ImplCalcFloatSize( this, nFloatLinesHeight );

        if( eMode == FSMODE_FAVOURHEIGHT || aCurrentSize == aSize1 )
            return aSize1;
    }

    if( eMode == FSMODE_AUTO || eMode == FSMODE_FAVOURWIDTH )
    {
        // try to preserve current width
        long nLineHeight = ( mnWinHeight > mnMaxItemHeight ) ? mnWinHeight : mnMaxItemHeight;
        int nBorderX = 2*TB_BORDER_OFFSET1 + mnLeftBorder + mnRightBorder + 2*mnBorderX;
        int nBorderY = 2*TB_BORDER_OFFSET2 + mnTopBorder + mnBottomBorder + 2*mnBorderY;
        Size aSz( aCurrentSize );
        long maxX;
        sal_uInt16 nLines = ImplCalcBreaks( aSz.Width()-nBorderX, &maxX, mbHorz );

        sal_uInt16 manyLines = 1000;
        Size aMinimalFloatSize = ImplCalcFloatSize( this, manyLines );

        aSz.Height() = nBorderY + nLineHeight * nLines;
        // line space when more than one line
        if ( mnWinStyle & WB_LINESPACING )
            aSz.Height() += (nLines-1)*TB_LINESPACING;

        aSz.Width() = nBorderX + maxX;

        // avoid clipping of any items
        if( aSz.Width() < aMinimalFloatSize.Width() )
            aSize2 = ImplCalcFloatSize( this, nLines );
        else
            aSize2 = aSz;

        if( eMode == FSMODE_FAVOURWIDTH || aCurrentSize == aSize2 )
            return aSize2;
        else
        {
            // set the size with the smallest delta as the current size
            long dx1 = abs( mnDX - aSize1.Width() );
            long dy1 = abs( mnDY - aSize1.Height() );

            long dx2 = abs( mnDX - aSize2.Width() );
            long dy2 = abs( mnDY - aSize2.Height() );

            if( dx1*dy1 < dx2*dy2 )
                aCurrentSize = aSize1;
            else
                aCurrentSize = aSize2;
        }
    }
    return aCurrentSize;
}

namespace
{
static void lcl_hideDoubleSeparators( std::vector< ImplToolItem >& rItems )
{
    bool bLastSep( true );
    std::vector< ImplToolItem >::iterator it;
    for ( it = rItems.begin(); it != rItems.end(); ++it )
    {
        if ( it->meType == TOOLBOXITEM_SEPARATOR )
        {
            it->mbVisible = sal_False;
            if ( !bLastSep )
            {
                // check if any visible items have to appear behind it
                std::vector< ImplToolItem >::iterator temp_it;
                for ( temp_it = it+1; temp_it != rItems.end(); ++temp_it )
                {
                    if ( ((temp_it->meType == TOOLBOXITEM_BUTTON) &&
                          temp_it->mbVisible) )
                    {
                        it->mbVisible = sal_True;
                        break;
                    }
                }
            }
            bLastSep = true;
        }
        else if ( it->mbVisible )
            bLastSep = false;
    }
}
}

void ToolBox::ImplFormat( sal_Bool bResize )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // Muss ueberhaupt neu formatiert werden
    if ( !mbFormat )
        return;

    mpData->ImplClearLayoutData();

    // Positionen/Groessen berechnen
    Rectangle       aEmptyRect;
    long            nLineSize;
    long            nLeft;
    long            nTop;
    long            nMax;   // width of layoutarea in pixels
    sal_uInt16          nFormatLine;
    sal_Bool            bMustFullPaint;

    std::vector< ImplToolItem >::iterator   it;

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    sal_Bool bIsInPopupMode = ImplIsInPopupMode();

    // FloatSizeAry gegebenenfalls loeschen
    if ( mpFloatSizeAry )
    {
        delete mpFloatSizeAry;
        mpFloatSizeAry = NULL;
    }

    // compute border sizes
    ImplCalcBorder( meAlign, mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder, this );

    // update drag area (where the 'grip' will be placed)
    Rectangle aOldDragRect;
    if( pWrapper )
        aOldDragRect = pWrapper->GetDragArea();
    ImplUpdateDragArea( this );

    if ( ImplCalcItem() )
        bMustFullPaint = sal_True;
    else
        bMustFullPaint = sal_False;


    // calculate new size during interactive resize or
    // set computed size when formatting only
    if ( ImplIsFloatingMode() )
    {
        if ( bResize )
            mnFloatLines = ImplCalcLines( this, mnDY );
        else
            SetOutputSizePixel( ImplGetOptimalFloatingSize( FSMODE_AUTO ) );
    }

    // Horizontal
    if ( mbHorz )
    {
        long nBottom;
        // nLineSize: height of a single line, will fit highest item
        nLineSize = mnMaxItemHeight;

        if ( mnWinHeight > mnMaxItemHeight )
            nLineSize = mnWinHeight;

        if ( mbScroll )
        {
            nMax        = mnDX;
            mnVisLines  = ImplCalcLines( this, mnDY );
        }
        else
        {
            // layout over all lines
            mnVisLines  = mnLines;
            nMax        = TB_MAXNOSCROLL;
        }

        // add in all border offsets
        // inner border as well as custom border (mnBorderX, mnBorderY)
        if ( mnWinStyle & WB_BORDER )
        {
            nLeft       = TB_BORDER_OFFSET1 + mnLeftBorder;
            nTop        = TB_BORDER_OFFSET2 + mnTopBorder;
            nBottom     = TB_BORDER_OFFSET1 + mnBottomBorder;
            nMax       -= nLeft + TB_BORDER_OFFSET1 + mnRightBorder;
        }
        else
        {
            nLeft       = 0;
            nTop        = 0;
            nBottom     = 0;
        }

        nLeft += mnBorderX;
        nTop  += mnBorderY;
        nBottom += mnBorderY;
        nMax  -= mnBorderX*2;

        // adjust linesize if docked in single-line mode (i.e. when using a clipped item menu)
        // we have to center all items in the window height
        if( IsMenuEnabled() && !ImplIsFloatingMode() )
        {
            long  nWinHeight = mnDY - nTop - nBottom;
            if( nWinHeight > nLineSize )
                nLineSize = nWinHeight;
        }
    }
    else
    {
        long nRight;
        nLineSize = mnMaxItemWidth;

        if ( mbScroll )
        {
            mnVisLines  = ImplCalcLines( this, mnDX );
            nMax        = mnDY;
        }
        else
        {
            mnVisLines  = mnLines;
            nMax        = TB_MAXNOSCROLL;
        }

        if ( mnWinStyle & WB_BORDER )
        {
            nTop        = TB_BORDER_OFFSET1 + mnTopBorder;
            nLeft       = TB_BORDER_OFFSET2 + mnLeftBorder;
            nRight      = TB_BORDER_OFFSET2 + mnRightBorder;
            nMax       -= nTop + TB_BORDER_OFFSET1 + mnBottomBorder;
        }
        else
        {
            nLeft       = 0;
            nTop        = 0;
            nRight      = 0;
        }

        nLeft += mnBorderX;
        nRight+= mnBorderX;
        nTop  += mnBorderY;
        nMax  -= mnBorderY*2;

        // adjust linesize if docked in single-line mode (i.e. when using a clipped item menu)
        // we have to center all items in the window height
        if( !ImplIsFloatingMode() && IsMenuEnabled() )
        {
            long  nWinWidth = mnDX - nLeft - nRight;
            if( nWinWidth > nLineSize )
                nLineSize = nWinWidth;
        }
    }

    // no calculation if the window has no size (nMax=0)
    // non scrolling toolboxes must be computed though
    if ( (nMax <= 0) && mbScroll )
    {
        mnVisLines   = 1;
        mnCurLine    = 1;
        mnCurLines   = 1;

        it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            it->maRect = aEmptyRect;
            ++it;
        }

        maLowerRect = aEmptyRect;
        maUpperRect = aEmptyRect;
        maNextToolRect = aEmptyRect;
    }
    else
    {
        // init start values
        long nX = nLeft;    // top-left offset
        long nY = nTop;
        nFormatLine = 1;

        // save old scroll rectangles and reset them
        Rectangle aOldLowerRect = maLowerRect;
        Rectangle aOldUpperRect = maUpperRect;
        Rectangle aOldNextToolRect = maNextToolRect;
        Rectangle aOldMenubuttonRect = mpData->maMenubuttonItem.maRect;
        maUpperRect = aEmptyRect;
        maLowerRect = aEmptyRect;
        maNextToolRect = aEmptyRect;
        mpData->maMenubuttonItem.maRect = aEmptyRect;

        // additional toolboxes require a toggle button (maNextToolRect)
        if ( maNextToolBoxStr.Len() && mbScroll )
        {
            nMax -= TB_NEXT_SIZE-TB_NEXT_OFFSET;
            if ( mbHorz )
            {
                maNextToolRect.Left()    = nLeft+nMax;
                maNextToolRect.Right()   = maNextToolRect.Left()+TB_NEXT_SIZE-1;
                maNextToolRect.Top()     = nTop;
                maNextToolRect.Bottom()  = mnDY-mnBottomBorder-mnBorderY-TB_BORDER_OFFSET2-1;
            }
            else
            {
                maNextToolRect.Top()     = nTop+nMax;
                maNextToolRect.Bottom()  = maNextToolRect.Top()+TB_NEXT_SIZE-1;
                maNextToolRect.Left()    = nLeft;
                maNextToolRect.Right()   = mnDX-mnRightBorder-mnBorderX-TB_BORDER_OFFSET2-1;
            }
        }

        // do we have any toolbox items at all ?
        if ( !mpData->m_aItems.empty() || IsMenuEnabled() )
        {
            lcl_hideDoubleSeparators( mpData->m_aItems );

            // compute line breaks and visible lines give the current window width (nMax)
            // the break indicators will be stored within each item (it->mbBreak)
            mnCurLines = ImplCalcBreaks( nMax, NULL, mbHorz );

            // check for scrollbar buttons or dropdown menu
            // (if a menu is enabled, this will be used to store clipped
            //  items and no scroll buttons will appear)
            if ( (!ImplIsFloatingMode() && (mnCurLines > mnVisLines) && mbScroll ) ||
                IsMenuEnabled() )
            {
                // compute linebreaks again, incorporating scrollbar buttons
                if( !IsMenuEnabled() )
                {
                    nMax -= TB_SPIN_SIZE+TB_SPIN_OFFSET;
                    mnCurLines = ImplCalcBreaks( nMax, NULL, mbHorz );
                }

                // compute scroll rectangles or menu button
                if ( mbHorz )
                {
                    if( IsMenuEnabled() && !ImplHasExternalMenubutton() && !bIsInPopupMode )
                    {
                        if( !ImplIsFloatingMode() )
                        {
                            mpData->maMenubuttonItem.maRect.Right() = mnDX - 2;
                            mpData->maMenubuttonItem.maRect.Top() = nTop;
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY-mnBottomBorder-mnBorderY-TB_BORDER_OFFSET2-1;
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.Right() = mnDX - mnRightBorder-mnBorderX-TB_BORDER_OFFSET1-1;
                            mpData->maMenubuttonItem.maRect.Top() = nTop;
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY-mnBottomBorder-mnBorderY-TB_BORDER_OFFSET2-1;
                        }
                        mpData->maMenubuttonItem.maRect.Left() = mpData->maMenubuttonItem.maRect.Right() - mpData->mnMenuButtonWidth;
                    }
                    else
                    {
                        maUpperRect.Left()   = nLeft+nMax+TB_SPIN_OFFSET;
                        maUpperRect.Right()  = maUpperRect.Left()+TB_SPIN_SIZE-1;
                        maUpperRect.Top()    = nTop;
                        maLowerRect.Bottom() = mnDY-mnBottomBorder-mnBorderY-TB_BORDER_OFFSET2-1;
                        maLowerRect.Left()   = maUpperRect.Left();
                        maLowerRect.Right()  = maUpperRect.Right();
                        maUpperRect.Bottom() = maUpperRect.Top() +
                                            (maLowerRect.Bottom()-maUpperRect.Top())/2;
                        maLowerRect.Top()    = maUpperRect.Bottom();
                    }
                }
                else
                {
                    if( IsMenuEnabled() && !ImplHasExternalMenubutton() && !bIsInPopupMode )
                    {
                        if( !ImplIsFloatingMode() )
                        {
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY - 2;
                            mpData->maMenubuttonItem.maRect.Left() = nLeft;
                            mpData->maMenubuttonItem.maRect.Right() = mnDX-mnRightBorder-mnBorderX-TB_BORDER_OFFSET2-1;
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY - mnBottomBorder-mnBorderY-TB_BORDER_OFFSET1-1;
                            mpData->maMenubuttonItem.maRect.Left() = nLeft;
                            mpData->maMenubuttonItem.maRect.Right() = mnDX-mnRightBorder-mnBorderX-TB_BORDER_OFFSET2-1;
                        }
                        mpData->maMenubuttonItem.maRect.Top() = mpData->maMenubuttonItem.maRect.Bottom() - mpData->mnMenuButtonWidth;
                    }
                    else
                    {
                        maUpperRect.Top()    = nTop+nMax+TB_SPIN_OFFSET;;
                        maUpperRect.Bottom() = maUpperRect.Top()+TB_SPIN_SIZE-1;
                        maUpperRect.Left()   = nLeft;
                        maLowerRect.Right()  = mnDX-mnRightBorder-mnBorderX-TB_BORDER_OFFSET2-1;
                        maLowerRect.Top()    = maUpperRect.Top();
                        maLowerRect.Bottom() = maUpperRect.Bottom();
                        maUpperRect.Right()  = maUpperRect.Left() +
                                            (maLowerRect.Right()-maUpperRect.Left())/2;
                        maLowerRect.Left()   = maUpperRect.Right();
                    }
                }
            }

            // no scrolling when there is a "more"-menu
            // anything will "fit" in a single line then
            if( IsMenuEnabled() )
                mnCurLines = 1;

            // determine the currently visible line
            if ( mnVisLines >= mnCurLines )
                mnCurLine = 1;
            else if ( mnCurLine+mnVisLines-1 > mnCurLines )
                mnCurLine = mnCurLines - (mnVisLines-1);

            it = mpData->m_aItems.begin();
            while ( it != mpData->m_aItems.end() )
            {
                it->mbShowWindow = sal_False;

                // check for line break and advance nX/nY accordingly
                if ( it->mbBreak )
                {
                    nFormatLine++;

                    // increment starting with the second line
                    if ( nFormatLine > mnCurLine )
                    {
                        if ( mbHorz )
                        {
                            nX = nLeft;
                            if ( mnWinStyle & WB_LINESPACING )
                                nY += nLineSize+TB_LINESPACING;
                            else
                                nY += nLineSize;
                        }
                        else
                        {
                            nY = nTop;
                            if ( mnWinStyle & WB_LINESPACING )
                                nX += nLineSize+TB_LINESPACING;
                            else
                                nX += nLineSize;
                        }
                    }
                }

                if ( !it->mbVisible || (nFormatLine < mnCurLine) ||
                     (nFormatLine > mnCurLine+mnVisLines-1) )
                     // item is not visible
                    it->maCalcRect = aEmptyRect;
                else
                {
                    // 1. determine current item width/height
                    // take window size and orientation into account, because this affects the size of item windows

                    Size aCurrentItemSize( it->GetSize( mbHorz, mbScroll, nMax, Size(mnMaxItemWidth, mnMaxItemHeight) ) );

                    // 2. position item rect and use size from step 1
                    //  items will be centered horizontally (if mbHorz) or vertically
                    //  advance nX and nY accordingly
                    if ( mbHorz )
                    {
                        it->maCalcRect.Left()     = nX;
                        // if special TBX_LAYOUT_LOCKVERT lock vertical position
                        // don't recalulate the vertical position of the item
                        if ( meLayoutMode == TBX_LAYOUT_LOCKVERT && mnLines == 1 )
                        {
                            // Somewhat of a hack here, calc deletes and re-adds
                            // the sum/assign & ok/cancel items dynamically.
                            // Because TBX_LAYOUT_LOCKVERT effectively prevents
                            // recalculation of the vertical pos of an item the
                            // it->maRect.Top() for those newly added items is
                            // 0. The hack here is that we want to effectively
                            // recalculate the vertical pos for those added
                            // items here. ( Note: assume mnMaxItemHeight is
                            // equal to the LineSize when multibar has a single
                            // line size )
                            it->maCalcRect.Top()      =  it->maRect.Top() ? it->maRect.Top() : ( nY + ( mnMaxItemHeight-aCurrentItemSize.Height())/2 );
                        }
                        else
                            it->maCalcRect.Top()      = nY+(nLineSize-aCurrentItemSize.Height())/2;
                        it->maCalcRect.Right()    = nX+aCurrentItemSize.Width()-1;
                        it->maCalcRect.Bottom()   = it->maCalcRect.Top()+aCurrentItemSize.Height()-1;
                        nX += aCurrentItemSize.Width();
                    }
                    else
                    {
                        it->maCalcRect.Left()     = nX+(nLineSize-aCurrentItemSize.Width())/2;
                        it->maCalcRect.Top()      = nY;
                        it->maCalcRect.Right()    = it->maCalcRect.Left()+aCurrentItemSize.Width()-1;
                        it->maCalcRect.Bottom()   = nY+aCurrentItemSize.Height()-1;
                        nY += aCurrentItemSize.Height();
                    }
                }

                // position window items into calculated item rect
                if ( it->mpWindow )
                {
                    if ( it->mbShowWindow )
                    {
                        Point aPos( it->maCalcRect.Left(), it->maCalcRect.Top() );
                        it->mpWindow->SetPosPixel( aPos );
                        if ( !mbCustomizeMode )
                            it->mpWindow->Show();
                    }
                    else
                        it->mpWindow->Hide();
                }

                ++it;
            } // end of loop over all items
        }
        else
            // we have no toolbox items
            mnCurLines = 1;


        if( IsMenuEnabled() && ImplIsFloatingMode() && !ImplHasExternalMenubutton() && !bIsInPopupMode )
        {
            // custom menu will be the last button in floating mode
            ImplToolItem &rIt = mpData->maMenubuttonItem;

            if ( mbHorz )
            {
                rIt.maRect.Left()     = nX+TB_MENUBUTTON_OFFSET;
                rIt.maRect.Top()      = nY;
                rIt.maRect.Right()    = rIt.maRect.Left() + mpData->mnMenuButtonWidth;
                rIt.maRect.Bottom()   = nY+nLineSize-1;
                nX += rIt.maItemSize.Width();
            }
            else
            {
                rIt.maRect.Left()     = nX;
                rIt.maRect.Top()      = nY+TB_MENUBUTTON_OFFSET;
                rIt.maRect.Right()    = nX+nLineSize-1;
                rIt.maRect.Bottom()   = rIt.maRect.Top() + mpData->mnMenuButtonWidth;
                nY += rIt.maItemSize.Height();
            }
        }


        // if toolbox visible trigger paint for changed regions
        if ( IsVisible() && !mbFullPaint )
        {
            if ( bMustFullPaint )
            {
                maPaintRect = Rectangle( mnLeftBorder, mnTopBorder,
                                         mnDX-mnRightBorder, mnDY-mnBottomBorder );
            }
            else
            {
                if ( aOldLowerRect != maLowerRect )
                {
                    maPaintRect.Union( maLowerRect );
                    maPaintRect.Union( aOldLowerRect );
                }
                if ( aOldUpperRect != maUpperRect )
                {
                    maPaintRect.Union( maUpperRect );
                    maPaintRect.Union( aOldUpperRect );
                }
                if ( aOldNextToolRect != maNextToolRect )
                {
                    maPaintRect.Union( maNextToolRect );
                    maPaintRect.Union( aOldNextToolRect );
                }
                if ( aOldMenubuttonRect != mpData->maMenubuttonItem.maRect )
                {
                    maPaintRect.Union( mpData->maMenubuttonItem.maRect );
                    maPaintRect.Union( aOldMenubuttonRect );
                }
                if ( pWrapper && aOldDragRect != pWrapper->GetDragArea() )
                {
                    maPaintRect.Union( pWrapper->GetDragArea() );
                    maPaintRect.Union( aOldDragRect );
                }

                it = mpData->m_aItems.begin();
                while ( it != mpData->m_aItems.end() )
                {
                    if ( it->maRect != it->maCalcRect )
                    {
                        maPaintRect.Union( it->maRect );
                        maPaintRect.Union( it->maCalcRect );
                    }
                    ++it;
                }
            }

            Invalidate( maPaintRect );
        }

        // store the new calculated item rects
        maPaintRect = aEmptyRect;
        Rectangle aVisibleRect(Point(0, 0), GetOutputSizePixel());
        it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            it->maRect = it->maCalcRect;
            it->maRect.IsOver(aVisibleRect);
            ++it;
        }
    }

    // indicate formatting is done
    mbFormat = sal_False;
}

// -----------------------------------------------------------------------

IMPL_LINK( ToolBox, ImplDropdownLongClickHdl, ToolBox*, EMPTYARG )
{
    if( mnCurPos != TOOLBOX_ITEM_NOTFOUND &&
        (mpData->m_aItems[ mnCurPos ].mnBits & TIB_DROPDOWN)
        )
    {
        mpData->mbDropDownByKeyboard = sal_False;
        GetDropdownClickHdl().Call( this );

        // do not reset data if the dropdown handler opened a floating window
        // see ImplFloatControl()
        if( mpFloatWin == NULL )
        {
            // no floater was opened
            Deactivate();
            ImplDrawItem( mnCurPos, 0 );

            mnCurPos         = TOOLBOX_ITEM_NOTFOUND;
            mnCurItemId      = 0;
            mnDownItemId     = 0;
            mnMouseClicks    = 0;
            mnMouseModifier  = 0;
            mnHighItemId     = 0;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ToolBox, ImplUpdateHdl, void*, EMPTYARG )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mbFormat )
        ImplFormat();

    return 0;
}

// -----------------------------------------------------------------------

static void ImplDrawMoreIndicator( ToolBox *pBox, const Rectangle& rRect, sal_Bool bSetColor, sal_Bool bRotate )
{
    Color aOldFillColor = pBox->GetFillColor();
    Color aOldLineColor = pBox->GetLineColor();
    pBox->SetLineColor();

    if ( bSetColor )
    {
        if ( pBox->GetSettings().GetStyleSettings().GetFaceColor().IsDark() )
            pBox->SetFillColor( Color( COL_WHITE ) );
        else
            pBox->SetFillColor( Color( COL_BLACK ) );
    }

    if( !bRotate )
    {
        long width = 8;
        long height = 5;
        long x = rRect.Left() + (rRect.getWidth() - width)/2 + 1;
        long y = rRect.Top() + (rRect.getHeight() - height)/2 + 1;
        while( height >= 1)
        {
            pBox->DrawRect( Rectangle( x, y, x+1, y ) );
            x+=4;
            pBox->DrawRect( Rectangle( x, y, x+1, y ) );
            x-=4;
            y++;
            if( height <= 3) x--;
            else            x++;
            height--;
        }
    }
    else
    {
        long width = 5;
        long height = 8;
        long x = rRect.Left() + (rRect.getWidth() - width)/2 + 1;
        long y = rRect.Top() + (rRect.getHeight() - height)/2 + 1;
        while( width >= 1)
        {
            pBox->DrawRect( Rectangle( x, y, x, y+1 ) );
            y+=4;
            pBox->DrawRect( Rectangle( x, y, x, y+1 ) );
            y-=4;
            x++;
            if( width <= 3) y--;
            else           y++;
            width--;
        }
    }

    pBox->SetFillColor( aOldFillColor );
    pBox->SetLineColor( aOldLineColor );
}

static void ImplDrawDropdownArrow( ToolBox *pBox, const Rectangle& rDropDownRect, sal_Bool bSetColor, sal_Bool bRotate )
{
    sal_Bool bLineColor = pBox->IsLineColor();
    sal_Bool bFillColor = pBox->IsFillColor();
    Color aOldFillColor = pBox->GetFillColor();
    Color aOldLineColor = pBox->GetLineColor();
    pBox->SetLineColor();

    if ( bSetColor )
    {
        if ( pBox->GetSettings().GetStyleSettings().GetFaceColor().IsDark() )
            pBox->SetFillColor( Color( COL_WHITE ) );
        else
            pBox->SetFillColor( Color( COL_BLACK ) );
    }

    if( !bRotate )
    {
        long width = 5;
        long height = 3;
        long x = rDropDownRect.Left() + (rDropDownRect.getWidth() - width)/2;
        long y = rDropDownRect.Top() + (rDropDownRect.getHeight() - height)/2;
        while( width >= 1)
        {
            pBox->DrawRect( Rectangle( x, y, x+width-1, y ) );
            y++; x++;
            width -= 2;
        }
    }
    else
    {
        long width = 3;
        long height = 5;
        long x = rDropDownRect.Left() + (rDropDownRect.getWidth() - width)/2;
        long y = rDropDownRect.Top() + (rDropDownRect.getHeight() - height)/2;
        while( height >= 1)
        {
            pBox->DrawRect( Rectangle( x, y, x, y+height-1 ) );
            y++; x++;
            height -= 2;
        }
    }

    if( bFillColor )
        pBox->SetFillColor( aOldFillColor );
    else
        pBox->SetFillColor();
    if( bLineColor )
        pBox->SetLineColor( aOldLineColor );
    else
        pBox->SetLineColor( );
}

void ToolBox::ImplDrawToolArrow( ToolBox* pBox, long nX, long nY, sal_Bool bBlack, sal_Bool bColTransform,
                                 sal_Bool bLeft, sal_Bool bTop, long nSize )
{
    Color           aOldFillColor = pBox->GetFillColor();
    WindowAlign     eAlign = pBox->meAlign;
    long            n = 0;
    long            nHalfSize;
    if ( bLeft )
        eAlign = WINDOWALIGN_RIGHT;
    else if ( bTop )
        eAlign = WINDOWALIGN_BOTTOM;

    nHalfSize = nSize/2;

    switch ( eAlign )
    {
        case WINDOWALIGN_LEFT:
            if ( bBlack )
                pBox->SetFillColor( Color( bColTransform ? COL_WHITE : COL_BLACK ) );
            while ( n <= nHalfSize )
            {
                pBox->DrawRect( Rectangle( nX+n, nY+n, nX+n, nY+nSize-n ) );
                n++;
            }
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                n = 1;
                while ( n < nHalfSize )
                {
                    pBox->DrawRect( Rectangle( nX+n, nY+1+n, nX+n, nY+nSize-1-n ) );
                    n++;
                }
            }
            break;
        case WINDOWALIGN_TOP:
            if ( bBlack )
                pBox->SetFillColor( Color( bColTransform ? COL_WHITE : COL_BLACK ) );
            while ( n <= nHalfSize )
            {
                pBox->DrawRect( Rectangle( nX+n, nY+n, nX+nSize-n, nY+n ) );
                n++;
            }
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                n = 1;
                while ( n < nHalfSize )
                {
                    pBox->DrawRect( Rectangle( nX+1+n, nY+n, nX+nSize-1-n, nY+n ) );
                    n++;
                }
            }
            break;
        case WINDOWALIGN_RIGHT:
            if ( bBlack )
                pBox->SetFillColor( Color( bColTransform ? COL_WHITE : COL_BLACK ) );
            while ( n <= nHalfSize )
            {
                pBox->DrawRect( Rectangle( nX+nHalfSize-n, nY+n, nX+nHalfSize-n, nY+nSize-n ) );
                n++;
            }
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                n = 1;
                while ( n < nHalfSize )
                {
                    pBox->DrawRect( Rectangle( nX+nHalfSize-n, nY+1+n, nX+nHalfSize-n, nY+nSize-1-n ) );
                    n++;
                }
            }
            break;
        case WINDOWALIGN_BOTTOM:
            if ( bBlack )
                pBox->SetFillColor( Color( bColTransform ? COL_WHITE : COL_BLACK ) );
            while ( n <= nHalfSize )
            {
                pBox->DrawRect( Rectangle( nX+n, nY+nHalfSize-n, nX+nSize-n, nY+nHalfSize-n ) );
                n++;
            }
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                n = 1;
                while ( n < nHalfSize )
                {
                    pBox->DrawRect( Rectangle( nX+1+n, nY+nHalfSize-n, nX+nSize-1-n, nY+nHalfSize-n ) );
                    n++;
                }
            }
            break;
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawMenubutton( ToolBox *pThis, sal_Bool bHighlight )
{
    if( !pThis->mpData->maMenubuttonItem.maRect.IsEmpty() )
    {
        // #i53937# paint menu button only if necessary
        if( !pThis->ImplHasClippedItems() )
            return;

        // execute pending paint requests
        ImplCheckUpdate( pThis );

        sal_Bool bFillColor = pThis->IsFillColor();
        sal_Bool bLineColor = pThis->IsLineColor();
        Color aOldFillCol = pThis->GetFillColor();
        Color aOldLineCol = pThis->GetLineColor();

        // draw the 'more' indicator / button (>>)
        ImplErase( pThis, pThis->mpData->maMenubuttonItem.maRect, bHighlight );

        if( bHighlight )
            ImplDrawButton( pThis, pThis->mpData->maMenubuttonItem.maRect, 2, sal_False, sal_True, sal_False );

        if( pThis->ImplHasClippedItems() )
            ImplDrawMoreIndicator( pThis, pThis->mpData->maMenubuttonItem.maRect, sal_True, !pThis->mbHorz );

        // store highlight state
        pThis->mpData->mbMenubuttonSelected = bHighlight;

        // restore colors
        if( bFillColor )
            pThis->SetFillColor( aOldFillCol );
        else
            pThis->SetFillColor();
        if( bLineColor )
            pThis->SetLineColor( aOldLineCol );
        else
            pThis->SetLineColor();
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawSpin( sal_Bool bUpperIn, sal_Bool bLowerIn )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    sal_Bool    bTmpUpper;
    sal_Bool    bTmpLower;

    if ( maUpperRect.IsEmpty() || maLowerRect.IsEmpty() )
        return;

    if ( mnCurLine > 1 )
        bTmpUpper = sal_True;
    else
        bTmpUpper = sal_False;

    if ( mnCurLine+mnVisLines-1 < mnCurLines )
        bTmpLower = sal_True;
    else
        bTmpLower = sal_False;

    if ( !IsEnabled() )
    {
        bTmpUpper = sal_False;
        bTmpLower = sal_False;
    }

    ImplDrawSpinButton( this, maUpperRect, maLowerRect,
                        bUpperIn, bLowerIn, bTmpUpper, bTmpLower, !mbHorz );
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawNext( sal_Bool bIn )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( maNextToolRect.IsEmpty() )
        return;

    DecorationView aDecoView( this );

    // Button malen
    long    nX      = SMALLBUTTON_OFF_NORMAL_X;
    long    nY      = SMALLBUTTON_OFF_NORMAL_Y;
    sal_uInt16  nStyle  = 0;
    if ( bIn == 1 )
    {
        nStyle |= BUTTON_DRAW_PRESSED;
        nX = SMALLBUTTON_OFF_PRESSED_X;
        nY = SMALLBUTTON_OFF_PRESSED_Y;
    }
    aDecoView.DrawButton( maNextToolRect, nStyle );

    // Inhalt ausgeben
    sal_Bool    bLeft   = sal_False;
    sal_Bool    bTop    = sal_False;
    if ( mbHorz )
    {
        bLeft = sal_True;
        nX += (maNextToolRect.GetWidth()-6)/2-4;
        nY += (maNextToolRect.GetHeight()-6)/2-6;
    }
    else
    {
        bTop = sal_True;
        nY += (maNextToolRect.GetHeight()-6)/2-4;
        nX += (maNextToolRect.GetWidth()-6)/2-6;
    }

    nX += maNextToolRect.Left();
    nY += maNextToolRect.Top();
    SetLineColor();
    SetFillColor( COL_LIGHTBLUE );
    ImplDrawToolArrow( this, nX, nY, sal_True, sal_False, bLeft, bTop, 10 );
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawSeparator( sal_uInt16 nPos, Rectangle rRect )
{
    bool bNativeOk = false;
    ImplToolItem* pItem = &mpData->m_aItems[nPos];

    if( IsHorizontal() && IsNativeControlSupported( CTRL_TOOLBAR, PART_SEPARATOR ) )
    {
        ImplControlValue    aControlValue;
        ControlState        nState = 0;
        bNativeOk = DrawNativeControl( CTRL_TOOLBAR, PART_SEPARATOR,
                                       rRect, nState, aControlValue, rtl::OUString() );
    }

    /* Draw the widget only if it can't be drawn natively. */
    if( !bNativeOk )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        ImplToolItem* pTempItem = &mpData->m_aItems[nPos-1];

        // no separator before or after windows or at breaks
        if ( pTempItem && !pTempItem->mbShowWindow && nPos < mpData->m_aItems.size()-1 )
        {
            pTempItem = &mpData->m_aItems[nPos+1];
            if ( !pTempItem->mbShowWindow && !pTempItem->mbBreak )
            {
                long nCenterPos, nSlim;
                SetLineColor( rStyleSettings.GetSeparatorColor() );
                if ( IsHorizontal() )
                {
                    nSlim = (pItem->maRect.Bottom() - pItem->maRect.Top ()) / 4;
                    nCenterPos = pItem->maRect.Center().X();
                    DrawLine( Point( nCenterPos, pItem->maRect.Top() + nSlim ),
                              Point( nCenterPos, pItem->maRect.Bottom() - nSlim ) );
                }
                else
                {
                    nSlim = (pItem->maRect.Right() - pItem->maRect.Left ()) / 4;
                    nCenterPos = pItem->maRect.Center().Y();
                    DrawLine( Point( pItem->maRect.Left() + nSlim, nCenterPos ),
                              Point( pItem->maRect.Right() - nSlim, nCenterPos ) );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

static void ImplDrawButton( ToolBox* pThis, const Rectangle &rRect, sal_uInt16 highlight, sal_Bool bChecked, sal_Bool bEnabled, sal_Bool bIsWindow )
{
    // draws toolbar button background either native or using a coloured selection
    // if bIsWindow is sal_True, the corresponding item is a control and only a selection border will be drawn

    sal_Bool bNativeOk = sal_False;
    if( !bIsWindow && pThis->IsNativeControlSupported( CTRL_TOOLBAR, PART_BUTTON ) )
    {
        ImplControlValue    aControlValue;
        ControlState        nState = 0;

        if ( highlight == 1 )   nState |= CTRL_STATE_PRESSED;
        if ( highlight == 2 )     nState |= CTRL_STATE_ROLLOVER;
        if ( bEnabled )         nState |= CTRL_STATE_ENABLED;

        aControlValue.setTristateVal( bChecked ? BUTTONVALUE_ON : BUTTONVALUE_OFF );


        bNativeOk = pThis->DrawNativeControl( CTRL_TOOLBAR, PART_BUTTON,
                                              rRect, nState, aControlValue, rtl::OUString() );
    }

    if( !bNativeOk )
        pThis->DrawSelectionBackground( rRect, bIsWindow ? 3 : highlight, bChecked, sal_True, bIsWindow, 2, NULL, NULL );
}

void ToolBox::ImplDrawItem( sal_uInt16 nPos, sal_uInt16 nHighlight, sal_Bool bPaint, sal_Bool bLayout )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( nPos >= mpData->m_aItems.size() )
        return;

    // execute pending paint requests
    ImplCheckUpdate( this );

    ImplDisableFlatButtons();

    SetFillColor();

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    MetricVector* pVector = bLayout ? &mpData->m_pLayoutData->m_aUnicodeBoundRects : NULL;
    String* pDisplayText = bLayout ? &mpData->m_pLayoutData->m_aDisplayText : NULL;

    if(!pItem->mbEnabled)
        nHighlight = 0;

    // Falls Rechteck ausserhalb des sichbaren Bereichs liegt
    if ( pItem->maRect.IsEmpty() )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // no gradient background for items that have a popup open
    sal_Bool bHasOpenPopup = (mpFloatWin != NULL) && (mnDownItemId==pItem->mnId);

    sal_Bool bHighContrastWhite = sal_False;
    // check the face color as highcontrast indicator
    // because the toolbox itself might have a gradient
    if( rStyleSettings.GetFaceColor() == Color( COL_WHITE ) )
        bHighContrastWhite = sal_True;

    // Compute buttons area.
    Size    aBtnSize    = pItem->maRect.GetSize();
    if( ImplGetSVData()->maNWFData.mbToolboxDropDownSeparate )
    {
        // separate button not for dropdown only where the whole button is painted
        if ( pItem->mnBits & TIB_DROPDOWN &&
            ((pItem->mnBits & TIB_DROPDOWNONLY) != TIB_DROPDOWNONLY) )
        {
            Rectangle aArrowRect = pItem->GetDropDownRect( mbHorz );
            if( aArrowRect.Top() == pItem->maRect.Top() ) // dropdown arrow on right side
                aBtnSize.Width() -= aArrowRect.GetWidth();
            else // dropdown arrow on bottom side
                aBtnSize.Height() -= aArrowRect.GetHeight();
        }
    }

    /* Compute the button/separator rectangle here, we'll need it for
     * both the buttons and the separators. */
    Rectangle aButtonRect( pItem->maRect.TopLeft(), aBtnSize );
    long    nOffX       = SMALLBUTTON_OFF_NORMAL_X;
    long    nOffY       = SMALLBUTTON_OFF_NORMAL_Y;
    long    nImageOffX  = 0;
    long    nImageOffY  = 0;
    sal_uInt16  nStyle      = 0;

    // draw separators in flat style only
    if ( !bLayout &&
         (mnOutStyle & TOOLBOX_STYLE_FLAT) &&
         (pItem->meType == TOOLBOXITEM_SEPARATOR) &&
         nPos > 0
         )
    {
        ImplDrawSeparator( nPos, aButtonRect );
    }

    // do nothing if item is no button or will be displayed as window
    if ( (pItem->meType != TOOLBOXITEM_BUTTON) ||
         (pItem->mbShowWindow && !mbCustomizeMode) )
        return;

    // we need a TBDragMananger to draw the configuration item
    ImplTBDragMgr* pMgr;
    if ( pItem->mnId == mnConfigItem )
    {
        pMgr = ImplGetTBDragMgr();
        pMgr->HideDragRect();
    }
    else
        pMgr = NULL;

    // during configuration mode visible windows will be drawn in a special way
    if ( mbCustomizeMode && pItem->mbShowWindow )
    {
        Font    aOldFont = GetFont();
        Color   aOldTextColor = GetTextColor();

        SetZoomedPointFont( rStyleSettings.GetAppFont() );
        SetLineColor( Color( COL_BLACK ) );
        SetFillColor( rStyleSettings.GetFieldColor() );
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        if( !bLayout )
            DrawRect( pItem->maRect );

        Size aSize( GetCtrlTextWidth( pItem->maText ), GetTextHeight() );
        Point aPos( pItem->maRect.Left()+2, pItem->maRect.Top() );
        aPos.Y() += (pItem->maRect.GetHeight()-aSize.Height())/2;
        sal_Bool bClip;
        if ( (aSize.Width() > pItem->maRect.GetWidth()-2) ||
             (aSize.Height() > pItem->maRect.GetHeight()-2) )
        {
            bClip = sal_True;
            Rectangle aTempRect( pItem->maRect.Left()+1, pItem->maRect.Top()+1,
                                 pItem->maRect.Right()-1, pItem->maRect.Bottom()-1 );
            Region aTempRegion( aTempRect );
            SetClipRegion( aTempRegion );
        }
        else
            bClip = sal_False;
        if( bLayout )
        {
            mpData->m_pLayoutData->m_aLineIndices.push_back( mpData->m_pLayoutData->m_aDisplayText.Len() );
            mpData->m_pLayoutData->m_aLineItemIds.push_back( pItem->mnId );
            mpData->m_pLayoutData->m_aLineItemPositions.push_back( nPos );
        }
        DrawCtrlText( aPos, pItem->maText, 0, STRING_LEN, TEXT_DRAW_MNEMONIC, pVector, pDisplayText );
        if ( bClip )
            SetClipRegion();
        SetFont( aOldFont );
        SetTextColor( aOldTextColor );

        // Gegebenenfalls noch Config-Frame zeichnen
        if ( pMgr && !bLayout)
            pMgr->UpdateDragRect();
        return;
    }

    if ( pItem->meState == STATE_CHECK )
    {
        nStyle |= BUTTON_DRAW_CHECKED;
    }
    else if ( pItem->meState == STATE_DONTKNOW )
    {
        nStyle |= BUTTON_DRAW_DONTKNOW;
    }
    if ( nHighlight == 1 )
    {
        nStyle |= BUTTON_DRAW_PRESSED;
    }

    if ( mnOutStyle & TOOLBOX_STYLE_OUTBUTTON )
    {
        nOffX = OUTBUTTON_OFF_NORMAL_X;
        nOffY = OUTBUTTON_OFF_NORMAL_Y;
        if ( nHighlight != 0 )
        {
            nOffX++;
            nOffY++;
        }
    }

    if( ! bLayout )
    {
        if ( mnOutStyle & TOOLBOX_STYLE_FLAT )
        {
            if ( (pItem->meState != STATE_NOCHECK) || !bPaint )
            {
                ImplErase( this, pItem->maRect, nHighlight != 0, bHasOpenPopup );
            }
        }
        else
        {
            if ( mnOutStyle & TOOLBOX_STYLE_OUTBUTTON )
                ImplDrawOutButton( this, aButtonRect, nStyle );
            else
            {
                DecorationView aDecoView( this );
                aDecoView.DrawButton( aButtonRect, nStyle );
            }
        }
    }

    nOffX += pItem->maRect.Left();
    nOffY += pItem->maRect.Top();

    // determine what has to be drawn on the button: image, text or both
    sal_Bool bImage;
    sal_Bool bText;
    ButtonType tmpButtonType = determineButtonType( pItem, meButtonType ); // default to toolbox setting
    pItem->DetermineButtonDrawStyle( tmpButtonType, bImage, bText );

    // compute output values
    long    nBtnWidth = aBtnSize.Width()-SMALLBUTTON_HSIZE;
    long    nBtnHeight = aBtnSize.Height()-SMALLBUTTON_VSIZE;
    Size    aImageSize;
    Size    aTxtSize;

    if ( bText )
    {
        aTxtSize.Width() = GetCtrlTextWidth( pItem->maText );
        aTxtSize.Height() = GetTextHeight();
    }

    if ( bImage && ! bLayout )
    {
        const Image* pImage;
        if ( (nHighlight != 0) && (!(pItem->maHighImage)) == sal_False )
            pImage = &(pItem->maHighImage);
        else
            pImage = &(pItem->maImage);

        aImageSize = pImage->GetSizePixel();

        // determine drawing flags
        sal_uInt16 nImageStyle = 0;

        if ( !pItem->mbEnabled || !IsEnabled() )
            nImageStyle |= IMAGE_DRAW_DISABLE;

        // #i35563# the dontknow state indicates different states at the same time
        // which should not be rendered disabled but normal

        // draw the image
        nImageOffX = nOffX;
        nImageOffY = nOffY;
        if ( (pItem->mnBits & (TIB_LEFT|TIB_DROPDOWN)) || bText )
        {
            // left align also to leave space for drop down arrow
            // and when drawing text+image
            // just center in y, except for vertical (ie rotated text)
            if( mbHorz || !bText )
                nImageOffY += (nBtnHeight-aImageSize.Height())/2;
        }
        else
        {
            nImageOffX += (nBtnWidth-aImageSize.Width())/2;
            nImageOffY += (nBtnHeight-aImageSize.Height())/2;
        }
        if ( nHighlight != 0 || (pItem->meState == STATE_CHECK) )
        {
            if( bHasOpenPopup )
                ImplDrawFloatwinBorder( pItem );
            else
                ImplDrawButton( this, aButtonRect, nHighlight, pItem->meState == STATE_CHECK, pItem->mbEnabled && IsEnabled(), pItem->mbShowWindow ? sal_True : sal_False );

            if( nHighlight != 0 )
            {
                if( bHighContrastWhite )
                    nImageStyle |= IMAGE_DRAW_COLORTRANSFORM;
            }
        }
        DrawImage( Point( nImageOffX, nImageOffY ), *pImage, nImageStyle );
    }

    // draw the text
    sal_Bool bRotate = sal_False;
    if ( bText )
    {
        long nTextOffX = nOffX;
        long nTextOffY = nOffY;

        // rotate text when vertically docked
        Font aOldFont = GetFont();
        if( pItem->mbVisibleText && !ImplIsFloatingMode() &&
            ((meAlign == WINDOWALIGN_LEFT) || (meAlign == WINDOWALIGN_RIGHT)) )
        {
            bRotate = sal_True;

            Font aRotateFont = aOldFont;
            aRotateFont.SetOrientation( 2700 );

            // center horizontally
            nTextOffX += aTxtSize.Height();
            nTextOffX += (nBtnWidth-aTxtSize.Height())/2;

            // add in image offset
            if( bImage )
                nTextOffY = nImageOffY + aImageSize.Height() + TB_IMAGETEXTOFFSET;

            SetFont( aRotateFont );
        }
        else
        {
            // center vertically
            nTextOffY += (nBtnHeight-aTxtSize.Height())/2;

            // add in image offset
            if( bImage )
                nTextOffX = nImageOffX + aImageSize.Width() + TB_IMAGETEXTOFFSET;
        }

        // draw selection only if not already drawn during image output (see above)
        if ( !bLayout && !bImage && (nHighlight != 0 || (pItem->meState == STATE_CHECK) ) )
        {
            if( bHasOpenPopup )
                ImplDrawFloatwinBorder( pItem );
            else
                ImplDrawButton( this, pItem->maRect, nHighlight, pItem->meState == STATE_CHECK, pItem->mbEnabled && IsEnabled(), pItem->mbShowWindow ? sal_True : sal_False );
        }

        sal_uInt16 nTextStyle = 0;
        if ( !pItem->mbEnabled )
            nTextStyle |= TEXT_DRAW_DISABLE;
        if( bLayout )
        {
            mpData->m_pLayoutData->m_aLineIndices.push_back( mpData->m_pLayoutData->m_aDisplayText.Len() );
            mpData->m_pLayoutData->m_aLineItemIds.push_back( pItem->mnId );
            mpData->m_pLayoutData->m_aLineItemPositions.push_back( nPos );
        }
        DrawCtrlText( Point( nTextOffX, nTextOffY ), pItem->maText,
                      0, STRING_LEN, nTextStyle, pVector, pDisplayText );
        if ( bRotate )
            SetFont( aOldFont );
    }

    if( bLayout )
        return;

    // paint optional drop down arrow
    if ( pItem->mnBits & TIB_DROPDOWN )
    {
        Rectangle aDropDownRect( pItem->GetDropDownRect( mbHorz ) );
        sal_Bool bSetColor = sal_True;
        if ( !pItem->mbEnabled || !IsEnabled() )
        {
            bSetColor = sal_False;
            SetFillColor( rStyleSettings.GetShadowColor() );
        }

        // dropdown only will be painted without inner border
        if( (pItem->mnBits & TIB_DROPDOWNONLY) != TIB_DROPDOWNONLY )
        {
            ImplErase( this, aDropDownRect, nHighlight != 0, bHasOpenPopup );

            if( nHighlight != 0 || (pItem->meState == STATE_CHECK) )
            {
                if( bHasOpenPopup )
                    ImplDrawFloatwinBorder( pItem );
                else
                    ImplDrawButton( this, aDropDownRect, nHighlight, pItem->meState == STATE_CHECK, pItem->mbEnabled && IsEnabled(), sal_False );
            }
        }
        ImplDrawDropdownArrow( this, aDropDownRect, bSetColor, bRotate );
    }

    // Gegebenenfalls noch Config-Frame zeichnen
    if ( pMgr )
        pMgr->UpdateDragRect();
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawFloatwinBorder( ImplToolItem* pItem )
{
    if ( !pItem->maRect.IsEmpty() )
    {
        Rectangle aRect( mpFloatWin->ImplGetItemEdgeClipRect() );
        aRect.SetPos( AbsoluteScreenToOutputPixel( aRect.TopLeft() ) );
        SetLineColor( GetSettings().GetStyleSettings().GetShadowColor() );
        Point p1, p2;

        p1 = pItem->maRect.TopLeft();
        p1.X()++;
        p2 = pItem->maRect.TopRight();
        p2.X()--;
        DrawLine( p1, p2);
        p1 = pItem->maRect.BottomLeft();
        p1.X()++;
        p2 = pItem->maRect.BottomRight();
        p2.X()--;
        DrawLine( p1, p2);

        p1 = pItem->maRect.TopLeft();
        p1.Y()++;
        p2 = pItem->maRect.BottomLeft();
        p2.Y()--;
        DrawLine( p1, p2);
        p1 = pItem->maRect.TopRight();
        p1.Y()++;
        p2 = pItem->maRect.BottomRight();
        p2.Y()--;
        DrawLine( p1, p2);

        //DrawRect( pItem->maRect );
    }
}

void ToolBox::ImplFloatControl( sal_Bool bStart, FloatingWindow* pFloatWindow )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( bStart )
    {
        mpFloatWin = pFloatWindow;

        // redraw item, to trigger drawing of a special border
        ImplDrawItem( mnCurPos, 1 );

        mbDrag = sal_False;
        EndTracking();
        ReleaseMouse();
    }
    else
    {
        mpFloatWin = NULL;

        // if focus is still in this toolbox, then the floater was opened by keyboard
        // draw current item with highlight and keep old state
        sal_Bool bWasKeyboardActivate = mpData->mbDropDownByKeyboard;


        if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            ImplDrawItem( mnCurPos, bWasKeyboardActivate ? 2 : 0 );
        Deactivate();

        if( !bWasKeyboardActivate )
        {
            mnCurPos = TOOLBOX_ITEM_NOTFOUND;
            mnCurItemId = 0;
            mnHighItemId = 0;
        }
        mnDownItemId = 0;

    }
}

// -----------------------------------------------------------------------

void ToolBox::ShowLine( sal_Bool bNext )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mbFormat = sal_True;

    if ( mpData->mbPageScroll )
    {
        sal_uInt16 delta = mnVisLines;
        if ( bNext )
        {
            mnCurLine = mnCurLine + delta;
            if ( mnCurLine+mnVisLines-1 > mnCurLines )
                mnCurLine = mnCurLines - mnVisLines+1;
        }
        else
        {
            if( mnCurLine >= delta+1 )
                mnCurLine = mnCurLine - delta;
            else
                mnCurLine = 1;
        }
    }
    else
    {
        if ( bNext )
            mnCurLine++;
        else
            mnCurLine--;
    }

    ImplFormat();
}

// -----------------------------------------------------------------------

sal_Bool ToolBox::ImplHandleMouseMove( const MouseEvent& rMEvt, sal_Bool bRepeat )
{
    Point aMousePos = rMEvt.GetPosPixel();

    // Ist ToolBox aktiv
    if ( mbDrag && mnCurPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // Befindet sich Maus ueber dem Item
        ImplToolItem* pItem = &mpData->m_aItems[mnCurPos];
        if ( pItem->maRect.IsInside( aMousePos ) )
        {
            if ( !mnCurItemId )
            {
                ImplDrawItem( mnCurPos, 1 );
                mnCurItemId = pItem->mnId;
                Highlight();
            }

            if ( (pItem->mnBits & TIB_REPEAT) && bRepeat )
                Select();
        }
        else
        {
            if ( mnCurItemId )
            {
                ImplDrawItem( mnCurPos );
                mnCurItemId = 0;
                ImplDrawItem( mnCurPos );
                Highlight();
            }
        }

        return sal_True;
    }

    if ( mbUpper )
    {
        sal_Bool bNewIn = maUpperRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            ImplDrawSpin( mbIn, sal_False );
        }
        return sal_True;
    }

    if ( mbLower )
    {
        sal_Bool bNewIn = maLowerRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            ImplDrawSpin( sal_False, mbIn );
        }
        return sal_True;
    }

    if ( mbNextTool )
    {
        sal_Bool bNewIn = maNextToolRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            ImplDrawNext( mbIn );
        }
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ToolBox::ImplHandleMouseButtonUp( const MouseEvent& rMEvt, sal_Bool bCancel )
{
    ImplDisableFlatButtons();

    // stop eventual running dropdown timer
    if( mnCurPos < mpData->m_aItems.size() &&
        (mpData->m_aItems[mnCurPos].mnBits & TIB_DROPDOWN ) )
    {
        mpData->maDropdownTimer.Stop();
    }

    if ( mbDrag || mbSelection )
    {
        // Hier die MouseDaten setzen, wenn Selection-Modus, da dann kein
        // MouseButtonDown-Handler gerufen wird
        if ( mbSelection )
        {
            mnMouseClicks    = rMEvt.GetClicks();
            mnMouseModifier  = rMEvt.GetModifier();
        }

        Deactivate();

        if ( mbDrag )
            mbDrag = sal_False;
        else
        {
            mbSelection = sal_False;
            if ( mnCurPos == TOOLBOX_ITEM_NOTFOUND )
                return sal_True;
        }

        // Wurde Maus ueber dem Item losgelassen
        if( mnCurPos < mpData->m_aItems.size() )
        {
            ImplToolItem* pItem = &mpData->m_aItems[mnCurPos];
            if ( pItem->maRect.IsInside( rMEvt.GetPosPixel() ) )
            {
                mnCurItemId = pItem->mnId;
                if ( !bCancel )
                {
                    // Gegebenenfalls ein AutoCheck durchfuehren
                    if ( pItem->mnBits & TIB_AUTOCHECK )
                    {
                        if ( pItem->mnBits & TIB_RADIOCHECK )
                        {
                            if ( pItem->meState != STATE_CHECK )
                                SetItemState( pItem->mnId, STATE_CHECK );
                        }
                        else
                        {
                            if ( pItem->meState != STATE_CHECK )
                                pItem->meState = STATE_CHECK;
                            else
                                pItem->meState = STATE_NOCHECK;
                        }
                    }

                    // Select nicht bei Repeat ausloesen, da dies schon im
                    // MouseButtonDown ausgeloest wurde
                    if ( !(pItem->mnBits & TIB_REPEAT) )
                    {
                        // Gegen zerstoeren im Select-Handler sichern
                        ImplDelData aDelData;
                        ImplAddDel( &aDelData );
                        Select();
                        if ( aDelData.IsDelete() )
                            return sal_True;
                        ImplRemoveDel( &aDelData );
                    }
                }

                {
                    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
                }

                // Items nicht geloescht, im Select-Handler
                if ( mnCurItemId )
                {
                    sal_uInt16 nHighlight;
                    if ( (mnCurItemId == mnHighItemId) && (mnOutStyle & TOOLBOX_STYLE_FLAT) )
                        nHighlight = 2;
                    else
                        nHighlight = 0;
                    // Get current pos for the case that items are inserted/removed
                    // in the toolBox
                    mnCurPos = GetItemPos( mnCurItemId );
                    if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
                    {
                        ImplDrawItem( mnCurPos, nHighlight );
                        Flush();
                    }
                }
            }
        }

        mnCurPos         = TOOLBOX_ITEM_NOTFOUND;
        mnCurItemId      = 0;
        mnDownItemId     = 0;
        mnMouseClicks    = 0;
        mnMouseModifier  = 0;
        return sal_True;
    }
    else if ( mbUpper || mbLower )
    {
        if ( mbIn )
            ShowLine( !mbUpper );
        mbUpper = sal_False;
        mbLower = sal_False;
        mbIn    = sal_False;
        ImplDrawSpin( sal_False, sal_False );
        return sal_True;
    }
    else if ( mbNextTool )
    {
        mbNextTool  = sal_False;
        mbIn        = sal_False;
        ImplDrawNext( sal_False );
        NextToolBox();
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void ToolBox::MouseMove( const MouseEvent& rMEvt )
{
    // pressing a modifier generates synthetic mouse moves
    // ignore it if keyboard selection is acive
    if( HasFocus() && ( rMEvt.GetMode() & MOUSE_MODIFIERCHANGED ) )
        return;

    if ( ImplHandleMouseMove( rMEvt ) )
        return;

    ImplDisableFlatButtons();

    Point aMousePos = rMEvt.GetPosPixel();

    // only highlight when the focus is not inside a child window of a toolbox
    // eg, in a edit control
    // and do not hilight when focus is in a different toolbox
    sal_Bool bDrawHotSpot = sal_True;
    Window *pWin = Application::GetFocusWindow();
    if( pWin && pWin->ImplGetWindowImpl()->mbToolBox && pWin != this )
        bDrawHotSpot = sal_False;

    if ( mbSelection && bDrawHotSpot )
    {
        sal_uInt16  i = 0;
        sal_uInt16  nNewPos = TOOLBOX_ITEM_NOTFOUND;

        // Item suchen, das geklickt wurde
        std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            // Wenn Mausposition in diesem Item vorhanden, kann die
            // Suche abgebrochen werden
            if ( it->maRect.IsInside( aMousePos ) )
            {
                // Wenn es ein Button ist, dann wird er selektiert
                if ( it->meType == TOOLBOXITEM_BUTTON )
                {
                    // Wenn er disablet ist, findet keine Aenderung
                    // statt
                    if ( !it->mbEnabled || it->mbShowWindow )
                        nNewPos = mnCurPos;
                    else
                        nNewPos = i;
                }

                break;
            }

            i++;
            ++it;
        }

        // was a new entery selected ?
        // don't  change selection if keyboard selection is active and
        // mouse leaves the toolbox
        if ( nNewPos != mnCurPos && !( HasFocus() && nNewPos == TOOLBOX_ITEM_NOTFOUND ) )
        {
            if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            {
                ImplDrawItem( mnCurPos );
                ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, reinterpret_cast< void* >( mnCurPos ) );
            }

            mnCurPos = nNewPos;
            if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            {
                mnCurItemId = mnHighItemId = it->mnId;
                ImplDrawItem( mnCurPos, 2 ); // always use shadow effect (2)
            }
            else
                mnCurItemId = mnHighItemId = 0;

            Highlight();
        }
        return;
    }

    if ( mbDragging )
    {
        ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
        pMgr->Dragging( aMousePos );
        return;
    }

    PointerStyle eStyle = POINTER_ARROW;

    // change mouse cursor over drag area
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && pWrapper->GetDragArea().IsInside( rMEvt.GetPosPixel() ) )
        eStyle = POINTER_MOVE;

    if ( (mnWinStyle & TB_WBLINESIZING) == TB_WBLINESIZING )
    {
        if ( rMEvt.GetMode() & MOUSE_SIMPLEMOVE )
        {
            sal_uInt16 nLinePtr = ImplTestLineSize( this, rMEvt.GetPosPixel() );
            if ( nLinePtr & DOCK_LINEHSIZE )
            {
                if ( meAlign == WINDOWALIGN_LEFT )
                    eStyle = POINTER_WINDOW_ESIZE;
                else
                    eStyle = POINTER_WINDOW_WSIZE;
            }
            else if ( nLinePtr & DOCK_LINEVSIZE )
            {
                if ( meAlign == WINDOWALIGN_TOP )
                    eStyle = POINTER_WINDOW_SSIZE;
                else
                    eStyle = POINTER_WINDOW_NSIZE;
            }
        }
    }

    if ( (eStyle == POINTER_ARROW) && mbCustomizeMode )
    {
        // Item suchen, das geklickt wurde
        std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            // Wenn es ein Customize-Window ist, gegebenenfalls den
            // Resize-Pointer anzeigen
            if ( it->mbShowWindow )
            {
                if ( it->maRect.IsInside( aMousePos ) )
                {
                    if ( it->maRect.Right()-TB_RESIZE_OFFSET <= aMousePos.X() )
                        eStyle = POINTER_HSIZEBAR;
                    break;
                }
            }

            ++it;
        }
    }

    if ( bDrawHotSpot && ( ((eStyle == POINTER_ARROW) && (mnOutStyle & TOOLBOX_STYLE_HANDPOINTER)) ||
         (mnOutStyle & TOOLBOX_STYLE_FLAT) || !mnOutStyle ) )
    {
        sal_Bool bClearHigh = sal_True;
        if ( !rMEvt.IsLeaveWindow() && (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
        {
            std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
            while ( it != mpData->m_aItems.end() )
            {
                if ( it->maRect.IsInside( aMousePos ) )
                {
                    if ( (it->meType == TOOLBOXITEM_BUTTON) && it->mbEnabled )
                    {
                        if ( !mnOutStyle || (mnOutStyle & TOOLBOX_STYLE_FLAT) )
                        {
                            bClearHigh = sal_False;
                            if ( mnHighItemId != it->mnId )
                            {
                                sal_uInt16 nTempPos = sal::static_int_cast<sal_uInt16>(it - mpData->m_aItems.begin());
                                if ( mnHighItemId )
                                {
                                    ImplHideFocus();
                                    sal_uInt16 nPos = GetItemPos( mnHighItemId );
                                    ImplDrawItem( nPos );
                                    ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, reinterpret_cast< void* >( nPos ) );
                                }
                                if ( mpData->mbMenubuttonSelected )
                                {
                                    // remove highlight from menubutton
                                    ImplDrawMenubutton( this, sal_False );
                                }
                                mnHighItemId = it->mnId;
                                ImplDrawItem( nTempPos, 2 );
                                ImplShowFocus();
                                ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHT );
                            }
                        }
                        if ( mnOutStyle & TOOLBOX_STYLE_HANDPOINTER )
                            eStyle = POINTER_REFHAND;
                    }
                    break;
                }

                ++it;
            }
        }

        // only clear highlight when focus is not in toolbar
        sal_Bool bMenuButtonHit = mpData->maMenubuttonItem.maRect.IsInside( aMousePos ) && ImplHasClippedItems();
        if ( bClearHigh || bMenuButtonHit )
        {
            if ( !bMenuButtonHit && mpData->mbMenubuttonSelected )
            {
                // remove highlight from menubutton
                ImplDrawMenubutton( this, sal_False );
            }

            if( mnHighItemId )
            {
                sal_uInt16 nClearPos = GetItemPos( mnHighItemId );
                if ( nClearPos != TOOLBOX_ITEM_NOTFOUND )
                {
                    ImplDrawItem( nClearPos, (nClearPos == mnCurPos) ? 1 : 0 );
                    if( nClearPos != mnCurPos )
                        ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, reinterpret_cast< void* >( nClearPos ) );
                }
                ImplHideFocus();
                mnHighItemId = 0;
            }

            if( bMenuButtonHit )
            {
                ImplDrawMenubutton( this, sal_True );
            }
        }
    }

    if ( meLastStyle != eStyle )
    {
        meLastStyle = eStyle;
        Pointer aPtr( eStyle );
        SetPointer( aPtr );
    }

    DockingWindow::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void ToolBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    // Nur bei linker Maustaste ToolBox ausloesen und wenn wir uns nicht
    // noch in der normalen Bearbeitung befinden
    if ( rMEvt.IsLeft() && !mbDrag && (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
    {
        // Activate schon hier rufen, da gegebenenfalls noch Items
        // ausgetauscht werden
        Activate();

        // ToolBox hier updaten, damit der Anwender weiss, was Sache ist
        if ( mbFormat )
        {
            ImplFormat();
            Update();
        }

        Point  aMousePos = rMEvt.GetPosPixel();
        sal_uInt16 i = 0;
        sal_uInt16 nNewPos = TOOLBOX_ITEM_NOTFOUND;

        // Item suchen, das geklickt wurde
        std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            // Ist es dieses Item
            if ( it->maRect.IsInside( aMousePos ) )
            {
                // Ist es ein Separator oder ist das Item disabled,
                // dann mache nichts
                if ( (it->meType == TOOLBOXITEM_BUTTON) &&
                     (!it->mbShowWindow || mbCustomizeMode) )
                    nNewPos = i;

                break;
            }

            i++;
            ++it;
        }

        // Item gefunden
        if ( nNewPos != TOOLBOX_ITEM_NOTFOUND )
        {
            if ( mbCustomize )
            {
                if ( rMEvt.IsMod2() || mbCustomizeMode )
                {
                    Deactivate();

                    ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
                    Rectangle aItemRect = GetItemRect( it->mnId );
                    mnConfigItem = it->mnId;

                    sal_Bool bResizeItem;
                    if ( mbCustomizeMode && it->mbShowWindow &&
                         (it->maRect.Right()-TB_RESIZE_OFFSET <= aMousePos.X()) )
                        bResizeItem = sal_True;
                    else
                        bResizeItem = sal_False;
                    pMgr->StartDragging( this, aMousePos, aItemRect, 0, bResizeItem );
                    return;
                }
            }

            if ( !it->mbEnabled )
            {
                Deactivate();
                return;
            }


            // Aktuelle Daten setzen
            sal_uInt16 nTrackFlags = 0;
            mnCurPos         = i;
            mnCurItemId      = it->mnId;
            mnDownItemId     = mnCurItemId;
            mnMouseClicks    = rMEvt.GetClicks();
            mnMouseModifier  = rMEvt.GetModifier();
            if ( it->mnBits & TIB_REPEAT )
                nTrackFlags |= STARTTRACK_BUTTONREPEAT;


            if ( mbSelection )
            {
                ImplDrawItem( mnCurPos, 1 );
                Highlight();
            }
            else
            {
                // Hier schon bDrag setzen, da in EndSelection ausgewertet wird
                mbDrag = sal_True;

                // Bei Doppelklick nur den Handler rufen, aber bevor der
                // Button gehiltet wird, da evt. in diesem Handler der
                // Drag-Vorgang abgebrochen wird
                if ( rMEvt.GetClicks() == 2 )
                    DoubleClick();


                if ( mbDrag )
                {
                    ImplDrawItem( mnCurPos, 1 );
                    Highlight();
                }

                // was dropdown arrow pressed
                if( (it->mnBits & TIB_DROPDOWN) )
                {
                    if( ( (it->mnBits & TIB_DROPDOWNONLY) == TIB_DROPDOWNONLY) || it->GetDropDownRect( mbHorz ).IsInside( aMousePos ))
                    {
                        // dropdownonly always triggers the dropdown handler, over the whole button area

                        // the drop down arrow should not trigger the item action
                        mpData->mbDropDownByKeyboard = sal_False;
                        GetDropdownClickHdl().Call( this );

                        // do not reset data if the dropdown handler opened a floating window
                        // see ImplFloatControl()
                        if( mpFloatWin == NULL )
                        {
                            // no floater was opened
                            Deactivate();
                            ImplDrawItem( mnCurPos, 0 );

                            mnCurPos         = TOOLBOX_ITEM_NOTFOUND;
                            mnCurItemId      = 0;
                            mnDownItemId     = 0;
                            mnMouseClicks    = 0;
                            mnMouseModifier  = 0;
                            mnHighItemId     = 0;
                        }
                        return;
                    }
                    else // activate long click timer
                        mpData->maDropdownTimer.Start();
                }


                // Click-Handler aufrufen
                if ( rMEvt.GetClicks() != 2 )
                    Click();

                // Bei Repeat auch den Select-Handler rufen
                if ( nTrackFlags & STARTTRACK_BUTTONREPEAT )
                    Select();

                // Wenn die Aktion nicht im Click-Handler abgebrochen wurde
                if ( mbDrag )
                    StartTracking( nTrackFlags );
            }

            // Wenn Maus ueber einem Item gedrueckt wurde, koennen wir
            // die Bearbeitung abbrechen
            return;
        }

        Deactivate();

        // menu button hit ?
        if( mpData->maMenubuttonItem.maRect.IsInside( aMousePos ) && ImplHasClippedItems() )
        {
            ExecuteCustomMenu();
            return;
        }


        // Gegebenenfalls noch Scroll- und Next-Buttons ueberpruefen
        if ( maUpperRect.IsInside( aMousePos ) )
        {
            if ( mnCurLine > 1 )
            {
                StartTracking();
                mbUpper = sal_True;
                mbIn    = sal_True;
                ImplDrawSpin( sal_True, sal_False );
            }
            return;
        }
        if ( maLowerRect.IsInside( aMousePos ) )
        {
            if ( mnCurLine+mnVisLines-1 < mnCurLines )
            {
                StartTracking();
                mbLower = sal_True;
                mbIn    = sal_True;
                ImplDrawSpin( sal_False, sal_True );
            }
            return;
        }
        if ( maNextToolRect.IsInside( aMousePos ) )
        {
            StartTracking();
            mbNextTool   = sal_True;
            mbIn         = sal_True;
            ImplDrawNext( sal_True );
            return;
        }

        // Linesizing testen
        if ( (mnWinStyle & TB_WBLINESIZING) == TB_WBLINESIZING )
        {
            sal_uInt16 nLineMode = ImplTestLineSize( this, aMousePos );
            if ( nLineMode )
            {
                ImplTBDragMgr* pMgr = ImplGetTBDragMgr();

                // Handler rufen, damit die Dock-Rectangles gesetzt werden
                // koenen
                StartDocking();

                Point aPos  = GetParent()->OutputToScreenPixel( GetPosPixel() );
                Size  aSize = GetSizePixel();
                aPos = ScreenToOutputPixel( aPos );

                // Dragging starten
                pMgr->StartDragging( this, aMousePos, Rectangle( aPos, aSize ),
                                     nLineMode, sal_False );
                return;
            }
        }

        // Kein Item, dann nur Click oder DoubleClick
        if ( rMEvt.GetClicks() == 2 )
            DoubleClick();
        else
            Click();
    }

    if ( !mbDrag && !mbSelection && (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
        DockingWindow::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void ToolBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( ImplHandleMouseButtonUp( rMEvt ) )
        return;

    if ( mbDragging && (rMEvt.IsLeft() || mbCommandDrag) )
    {
        ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
        pMgr->EndDragging();
        return;
    }
    mbCommandDrag = sal_False;

    DockingWindow::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void ToolBox::Tracking( const TrackingEvent& rTEvt )
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    if ( rTEvt.IsTrackingEnded() )
        ImplHandleMouseButtonUp( rTEvt.GetMouseEvent(), rTEvt.IsTrackingCanceled() );
    else
        ImplHandleMouseMove( rTEvt.GetMouseEvent(), rTEvt.IsTrackingRepeat() );

    if ( aDelData.IsDelete() )
        // toolbox was deleted
        return;
    ImplRemoveDel( &aDelData );
    DockingWindow::Tracking( rTEvt );
}

// -----------------------------------------------------------------------

void ToolBox::Paint( const Rectangle& rPaintRect )
{
    if( mpData->mbIsPaintLocked )
        return;
    if ( rPaintRect == Rectangle( 0, 0, mnDX-1, mnDY-1 ) )
        mbFullPaint = sal_True;
    ImplFormat();
    mbFullPaint = sal_False;


    ImplDrawBackground( this, rPaintRect );

    if ( (mnWinStyle & WB_BORDER) && !ImplIsFloatingMode() )
        ImplDrawBorder( this );

    if( !ImplIsFloatingMode() )
        ImplDrawGrip( this );

    ImplDrawMenubutton( this, mpData->mbMenubuttonSelected );

    // SpinButtons zeichnen
    if ( mnWinStyle & WB_SCROLL )
    {
        if ( mnCurLines > mnLines )
            ImplDrawSpin( sal_False, sal_False );
    }

    // NextButton zeichnen
    ImplDrawNext( sal_False );

    // Buttons zeichnen
    sal_uInt16 nHighPos;
    if ( mnHighItemId )
        nHighPos = GetItemPos( mnHighItemId );
    else
        nHighPos = TOOLBOX_ITEM_NOTFOUND;

    sal_uInt16 nCount = (sal_uInt16)mpData->m_aItems.size();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = &mpData->m_aItems[i];

        // Nur malen, wenn Rechteck im PaintRectangle liegt
        if ( !pItem->maRect.IsEmpty() && rPaintRect.IsOver( pItem->maRect ) )
        {
            sal_uInt16 nHighlight = 0;
            if ( i == mnCurPos )
                nHighlight = 1;
            else if ( i == nHighPos )
                nHighlight = 2;
            ImplDrawItem( i, nHighlight );
        }
    }
    ImplShowFocus();
}

// -----------------------------------------------------------------------

void ToolBox::Move()
{
    DockingWindow::Move();
}

// -----------------------------------------------------------------------

void ToolBox::Resize()
{
    Size aSize = GetOutputSizePixel();
    // #i31422# some WindowManagers send (0,0) sizes when
    // switching virtual desktops - ignore this and avoid reformatting
    if( !aSize.Width() && !aSize.Height() )
        return;

    long nOldDX = mnDX;
    long nOldDY = mnDY;
    mnDX = aSize.Width();
    mnDY = aSize.Height();

    mnLastResizeDY = 0;

    // invalidate everything to have gradient backgrounds properly drawn
    Invalidate();

    // Evt. neu formatieren oder neu painten
    if ( mbScroll )
    {
        if ( !mbFormat )
        {
            mbFormat = sal_True;
            if( IsReallyVisible() )
                ImplFormat( sal_True );
        }
    }

    // Border muss neu ausgegeben werden
    if ( mnWinStyle & WB_BORDER )
    {
        // Da wir sonst beim Paint denken, das alles neu gepaintet wird
        if ( mbFormat && IsReallyVisible() )
            Invalidate();
        else
        {
            if ( mnRightBorder )
            {
                if ( nOldDX > mnDX )
                    Invalidate( Rectangle( mnDX-mnRightBorder-1, 0, mnDX, mnDY ) );
                else
                    Invalidate( Rectangle( nOldDX-mnRightBorder-1, 0, nOldDX, nOldDY ) );
            }

            if ( mnBottomBorder )
            {
                if ( nOldDY > mnDY )
                    Invalidate( Rectangle( 0, mnDY-mnBottomBorder-1, mnDX, mnDY ) );
                else
                    Invalidate( Rectangle( 0, nOldDY-mnBottomBorder-1, nOldDX, nOldDY ) );
            }
        }
    }
}

// -----------------------------------------------------------------------
const XubString& ToolBox::ImplGetHelpText( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
    {
        if ( !pItem->maHelpText.Len() && ( pItem->maHelpId.getLength() || pItem->maCommandStr.Len() ))
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if ( pItem->maCommandStr.Len() )
                    pItem->maHelpText = pHelp->GetHelpText( pItem->maCommandStr, this );
                if ( !pItem->maHelpText.Len() && pItem->maHelpId.getLength() )
                    pItem->maHelpText = pHelp->GetHelpText( rtl::OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
            }
        }

        return pItem->maHelpText;
    }
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId;
    Point aHelpPos;

    if( !rHEvt.KeyboardActivated() )
    {
        nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );
        aHelpPos = rHEvt.GetMousePosPixel();
    }
    else
    {
        if( !mnHighItemId )
            return;
        else
            nItemId = mnHighItemId;
        Rectangle aRect( GetItemRect( nItemId ) );
        if( aRect.IsEmpty() )
            return;
        else
            aHelpPos = OutputToScreenPixel( aRect.Center() );
    }

    if ( nItemId )
    {
        if ( rHEvt.GetMode() & (HELPMODE_BALLOON | HELPMODE_QUICK) )
        {
            // Rechteck ermitteln
            Rectangle aTempRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aTempRect.TopLeft() );
            aTempRect.Left()   = aPt.X();
            aTempRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aTempRect.BottomRight() );
            aTempRect.Right()  = aPt.X();
            aTempRect.Bottom() = aPt.Y();

            // Text ermitteln und anzeigen
            XubString aStr = GetQuickHelpText( nItemId );
            const XubString& rHelpStr = GetHelpText( nItemId );
            if ( !aStr.Len() )
                aStr = MnemonicGenerator::EraseAllMnemonicChars( GetItemText( nItemId ) );
            if ( rHEvt.GetMode() & HELPMODE_BALLOON )
            {
                if ( rHelpStr.Len() )
                    aStr = rHelpStr;
                Help::ShowBalloon( this, aHelpPos, aTempRect, aStr );
            }
            else
                Help::ShowQuickHelp( this, aTempRect, aStr, rHelpStr, QUICKHELP_CTRLTEXT );
            return;
        }
        else if ( rHEvt.GetMode() & HELPMODE_EXTENDED )
        {
            String aCommand = GetItemCommand( nItemId );
            rtl::OString  aHelpId( GetHelpId( nItemId ) );

            if ( aCommand.Len() || aHelpId.getLength() )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                {
                    if ( aCommand.Len() )
                        pHelp->Start( aCommand, this );
                    else if ( aHelpId.getLength() )
                        pHelp->Start( rtl::OStringToOUString( aHelpId, RTL_TEXTENCODING_UTF8 ), this );
                }
                return;
            }
        }
    }
    else if ( maNextToolRect.IsInside( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) ) )
    {
        if ( rHEvt.GetMode() & (HELPMODE_BALLOON | HELPMODE_QUICK) )
        {
            // Rechteck ermitteln
            Rectangle aTempRect = maNextToolRect;
            Point aPt = OutputToScreenPixel( aTempRect.TopLeft() );
            aTempRect.Left()   = aPt.X();
            aTempRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aTempRect.BottomRight() );
            aTempRect.Right()  = aPt.X();
            aTempRect.Bottom() = aPt.Y();

            if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                Help::ShowBalloon( this, aTempRect.Center(), aTempRect, maNextToolBoxStr );
            else
                Help::ShowQuickHelp( this, aTempRect, maNextToolBoxStr );
            return;
        }
    }

    DockingWindow::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

long ToolBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        KeyEvent aKEvt = *rNEvt.GetKeyEvent();
        KeyCode aKeyCode = aKEvt.GetKeyCode();
        sal_uInt16  nKeyCode = aKeyCode.GetCode();
        switch( nKeyCode )
        {
            case KEY_TAB:
                {
                // internal TAB cycling only if parent is not a dialog or if we are the ony child
                // otherwise the dialog control will take over
                sal_Bool bNoTabCycling = ( ( ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL) ) == WB_DIALOGCONTROL &&
                    ImplGetParent()->GetChildCount() != 1 );

                if( bNoTabCycling &&  ! (GetStyle() & WB_FORCETABCYCLE) )
                    return DockingWindow::Notify( rNEvt );
                else if( ImplChangeHighlightUpDn( aKeyCode.IsShift() ? sal_True : sal_False , bNoTabCycling ) )
                    return sal_False;
                else
                    return DockingWindow::Notify( rNEvt );
                }
            default:
                break;
        };
    }
    else if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        if( rNEvt.GetWindow() == this )
        {
            // the toolbar itself got the focus
            if( mnLastFocusItemId != 0 )
            {
                // restore last item
                ImplChangeHighlight( ImplGetItem( mnLastFocusItemId ) );
                mnLastFocusItemId = 0;
            }
            else if( (GetGetFocusFlags() & (GETFOCUS_BACKWARD|GETFOCUS_TAB) ) == (GETFOCUS_BACKWARD|GETFOCUS_TAB))
                // Shift-TAB was pressed in the parent
                ImplChangeHighlightUpDn( sal_False );
            else
                ImplChangeHighlightUpDn( sal_True );

            mnLastFocusItemId = 0;

            return true;
        }
        else
        {
            // a child window got the focus so update current item to
            // allow for proper lose focus handling in keyboard navigation
            std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
            while( it != mpData->m_aItems.end() )
            {
                if ( it->mbVisible )
                {
                    if ( it->mpWindow && it->mpWindow->ImplIsWindowOrChild( rNEvt.GetWindow() ) )
                    {
                        mnHighItemId = it->mnId;
                        break;
                    }
                }

                ++it;
            }
            return DockingWindow::Notify( rNEvt );
        }
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        // deselect
        ImplHideFocus();
        mnHighItemId = 0;
        mnCurPos = TOOLBOX_ITEM_NOTFOUND;
    }

    return DockingWindow::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void ToolBox::Command( const CommandEvent& rCEvt )
{
    // StartDrag auf MouseButton/Left/Alt abbilden
    if ( (rCEvt.GetCommand() == COMMAND_STARTDRAG) && rCEvt.IsMouseEvent() &&
         mbCustomize && !mbDragging && !mbDrag && !mbSelection &&
         (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
    {
        // Wir erlauben nur das Draggen von Items. Deshalb muessen wir
        // testen, ob auch ein Item angeklickt wurde, ansonsten wuerden
        // wir evt. das Fenster verschieben, was nicht gewollt waere.
        // Wir machen dieses jedoch nur im Customize-Mode, da ansonsten
        // Items zuhaeufig ausversehen verschoben werden.
        if ( mbCustomizeMode )
        {
            Point           aMousePos = rCEvt.GetMousePosPixel();
            std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
            while ( it != mpData->m_aItems.end() )
            {
                // Ist es dieses Item
                if ( it->maRect.IsInside( aMousePos ) )
                {
                    // Ist es ein Separator oder ist das Item disabled,
                    // dann mache nichts
                    if ( (it->meType == TOOLBOXITEM_BUTTON) &&
                         !it->mbShowWindow )
                        mbCommandDrag = sal_True;
                    break;
                }

                ++it;
            }

            if ( mbCommandDrag )
            {
                MouseEvent aMEvt( aMousePos, 1, MOUSE_SIMPLECLICK,
                                  MOUSE_LEFT, KEY_MOD2 );
                ToolBox::MouseButtonDown( aMEvt );
                return;
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_WHEEL )
    {
        if ( (mnCurLine > 1) || (mnCurLine+mnVisLines-1 < mnCurLines) )
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if ( pData->GetMode() == COMMAND_WHEEL_SCROLL )
            {
                if ( (mnCurLine > 1) && (pData->GetDelta() > 0) )
                    ShowLine( sal_False );
                else if ( (mnCurLine+mnVisLines-1 < mnCurLines) && (pData->GetDelta() < 0) )
                    ShowLine( sal_True );
                ImplDrawSpin( sal_False, sal_False );
                return;
            }
        }
    }

    DockingWindow::Command( rCEvt );
}

// -----------------------------------------------------------------------

void ToolBox::StateChanged( StateChangedType nType )
{
    DockingWindow::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
        ImplFormat();
    else if ( nType == STATE_CHANGE_ENABLE )
        ImplUpdateItem();
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        mbCalc = sal_True;
        mbFormat = sal_True;
        ImplInitSettings( sal_True, sal_False, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_False, sal_True ); // font, foreground, background
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ToolBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    DockingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        mbCalc = sal_True;
        mbFormat = sal_True;
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

sal_Bool ToolBox::PrepareToggleFloatingMode()
{
    return DockingWindow::PrepareToggleFloatingMode();
}

// -----------------------------------------------------------------------

void ToolBox::ToggleFloatingMode()
{
    DockingWindow::ToggleFloatingMode();

    sal_Bool mbOldHorz = mbHorz;

    if ( ImplIsFloatingMode() )
    {
        mbHorz   = sal_True;
        meAlign  = WINDOWALIGN_TOP;
        mbScroll = sal_True;

        if( mbOldHorz != mbHorz )
            mbCalc = sal_True;  // orientation was changed !

        ImplSetMinMaxFloatSize( this );
        SetOutputSizePixel( ImplCalcFloatSize( this, mnFloatLines ) );
    }
    else
    {
        mbScroll = (mnWinStyle & WB_SCROLL) ? sal_True : sal_False;
        if ( (meAlign == WINDOWALIGN_TOP) || (meAlign == WINDOWALIGN_BOTTOM) )
            mbHorz = sal_True;
        else
            mbHorz = sal_False;

        // set focus back to document
        ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->GrabFocus();
    }

    if( mbOldHorz != mbHorz )
    {
        // if orientation changes, the toolbox has to be initialized again
        // to update the direction of the gradient
        mbCalc = sal_True;
        ImplInitSettings( sal_True, sal_True, sal_True );
    }

    mbFormat = sal_True;
    ImplFormat();
}

// -----------------------------------------------------------------------

void ToolBox::StartDocking()
{
    meDockAlign = meAlign;
    mnDockLines = mnLines;
    mbLastFloatMode = ImplIsFloatingMode();
    DockingWindow::StartDocking();
}

// -----------------------------------------------------------------------

sal_Bool ToolBox::Docking( const Point& rPos, Rectangle& rRect )
{
    // Wenn Dragging, dann nicht machen, da vorher schon berechnet
    if ( mbDragging )
        return sal_False;

    sal_Bool bFloatMode = sal_False;

    DockingWindow::Docking( rPos, rRect );

    // Befindet sich die Maus ausserhalb des Bereichs befindet, kann es nur ein
    // FloatWindow werden
    Rectangle aDockingRect( rRect );
    if ( !ImplIsFloatingMode() )
    {
        // don't use tracking rectangle for alignment check, because it will be too large
        // to get a floating mode as result - switch to floating size
        // so the calculation only depends on the position of the rectangle, not the current
        // docking state of the window
        sal_uInt16 nTemp = 0;
        aDockingRect.SetSize( ImplCalcFloatSize( this, nTemp ) );

        // in this mode docking is never done by keyboard, so it's OK to use the mouse position
        aDockingRect.SetPos( ImplGetFrameWindow()->GetPointerPosPixel() );
    }

    Rectangle aIntersection = maOutDockRect.GetIntersection( aDockingRect );
    if ( !aIntersection.IsEmpty() && !IsDockingPrevented() )
    {
        Rectangle   aInRect = maInDockRect;
        Size aDockSize;
        aDockSize.Width()  = ImplCalcSize( this, mnLines, TB_CALCMODE_VERT ).Width();
        aDockSize.Height() = ImplCalcSize( this, mnLines, TB_CALCMODE_HORZ ).Height();
        aInRect.Left()   += aDockSize.Width()/2;
        aInRect.Top()    += aDockSize.Height()/2;
        aInRect.Right()  -= aDockSize.Width()/2;
        aInRect.Bottom() -= aDockSize.Height()/2;

        // Wenn Fenster zu klein, wird das gesammte InDock-Rect genommen
        if ( aInRect.Left() >= aInRect.Right() )
        {
            aInRect.Left()  = maInDockRect.Left();
            aInRect.Right() = maInDockRect.Right();
        }
        if ( aInRect.Top() >= aInRect.Bottom() )
        {
            aInRect.Top()    = maInDockRect.Top();
            aInRect.Bottom() = maInDockRect.Bottom();
        }

        // Wenn Maus nicht im Dock-Bereich, dann kann es nur zum
        // FloatWindow werden
        Rectangle aIntersect = aInRect.GetIntersection( aDockingRect );
        if ( aIntersect == aDockingRect )
            bFloatMode = sal_True;
        else
        {
            // docking rectangle is in the "sensible area"
            Point aPos = aDockingRect.TopLeft();
            Point aInPosTL( aPos.X()-aInRect.Left(), aPos.Y()-aInRect.Top() );
            Point aInPosBR( aPos.X()-aInRect.Left() + aDockingRect.GetWidth(), aPos.Y()-aInRect.Top() + aDockingRect.GetHeight() );
            Size  aInSize = aInRect.GetSize();

            if ( aInPosTL.X() <= 0 )
                meDockAlign = WINDOWALIGN_LEFT;
            else if ( aInPosTL.Y() <= 0)
                meDockAlign = WINDOWALIGN_TOP;
            else if ( aInPosBR.X() >= aInSize.Width() )
                meDockAlign = WINDOWALIGN_RIGHT;
            else if ( aInPosBR.Y() >= aInSize.Height() )
                meDockAlign = WINDOWALIGN_BOTTOM;

            // Wenn sich Dock-Align geaendert hat, muessen wir die
            // neue Dock-Groesse setzen
            if ( (meDockAlign == WINDOWALIGN_TOP) || (meDockAlign == WINDOWALIGN_BOTTOM) )
                aDockSize.Width() = maInDockRect.GetWidth();
            else
                aDockSize.Height() = maInDockRect.GetHeight();

            aDockingRect.SetSize( aDockSize );

            Point aPosTL( maInDockRect.TopLeft() );
            switch ( meDockAlign )
            {
                case WINDOWALIGN_TOP :
                    aDockingRect.SetPos( aPosTL );
                    break;
                case WINDOWALIGN_LEFT :
                    aDockingRect.SetPos( aPosTL );
                    break;
                case WINDOWALIGN_BOTTOM :
                {
                    Point aPosBL( maInDockRect.BottomLeft() );
                    aPosBL.Y() -= aDockingRect.GetHeight();
                    aDockingRect.SetPos( aPosBL );
                    break;
                }
                case WINDOWALIGN_RIGHT :
                {
                    Point aPosTR( maInDockRect.TopRight() );
                    aPosTR.X() -= aDockingRect.GetWidth();
                    aDockingRect.SetPos( aPosTR );
                    break;
                }
            }
        }
    }
    else
        bFloatMode = sal_True;

    if ( bFloatMode )
    {
        meDockAlign = meAlign;
        if ( !mbLastFloatMode )
        {
            sal_uInt16 nTemp = 0;
            aDockingRect.SetSize( ImplCalcFloatSize( this, nTemp ) );
        }
    }

    rRect = aDockingRect;
    mbLastFloatMode = bFloatMode;

    return bFloatMode;
}

// -----------------------------------------------------------------------

void ToolBox::EndDocking( const Rectangle& rRect, sal_Bool bFloatMode )
{
    if ( !IsDockingCanceled() )
    {
        if ( mnLines != mnDockLines )
            SetLineCount( mnDockLines );
        if ( meAlign != meDockAlign )
            SetAlign( meDockAlign );
    }
    if ( bFloatMode || (bFloatMode != ImplIsFloatingMode()) )
        DockingWindow::EndDocking( rRect, bFloatMode );
}

// -----------------------------------------------------------------------

void ToolBox::Resizing( Size& rSize )
{
    sal_uInt16  nCalcLines;
    sal_uInt16  nTemp;

    // Alle Floatinggroessen berechnen
    ImplCalcFloatSizes( this );

    if ( !mnLastResizeDY )
        mnLastResizeDY = mnDY;

    // Ist vertikales Resizing angesagt
    if ( (mnLastResizeDY != rSize.Height()) && (mnDY != rSize.Height()) )
    {
        nCalcLines = ImplCalcLines( this, rSize.Height() );
        if ( nCalcLines < 1 )
            nCalcLines = 1;
        rSize = ImplCalcFloatSize( this, nCalcLines );
    }
    else
    {
        nCalcLines = 1;
        nTemp = nCalcLines;
        Size aTempSize = ImplCalcFloatSize( this, nTemp );
        while ( (aTempSize.Width() > rSize.Width()) &&
                (nCalcLines <= mpFloatSizeAry->mpSize[0].mnLines) )
        {
            nCalcLines++;
            nTemp = nCalcLines;
            aTempSize = ImplCalcFloatSize( this, nTemp );
        }
        rSize = aTempSize;
    }

    mnLastResizeDY = rSize.Height();
}

// -----------------------------------------------------------------------

Size ToolBox::CalcWindowSizePixel( sal_uInt16 nCalcLines ) const
{
    return ImplCalcSize( this, nCalcLines );
}

Size ToolBox::CalcWindowSizePixel( sal_uInt16 nCalcLines, WindowAlign eAlign ) const
{
    return ImplCalcSize( this, nCalcLines,
        (eAlign == WINDOWALIGN_TOP || eAlign == WINDOWALIGN_BOTTOM) ? TB_CALCMODE_HORZ : TB_CALCMODE_VERT );
}

sal_uInt16 ToolBox::ImplCountLineBreaks( const ToolBox *pThis )
{
    sal_uInt16 nLines = 0;

    std::vector< ImplToolItem >::const_iterator it = ((ToolBox*)pThis)->mpData->m_aItems.begin();
    while ( it != ((ToolBox*)pThis)->mpData->m_aItems.end() )
    {
        if( it->meType == TOOLBOXITEM_BREAK )
            ++nLines;
        ++it;
    }
    return nLines;
}

Size ToolBox::CalcPopupWindowSizePixel() const
{
    // count number of breaks and calc corresponding floating window size
    sal_uInt16 nLines = ImplCountLineBreaks( this );

    if( nLines )
        ++nLines;   // add the first line
    else
    {
        // no breaks found: use quadratic layout
        nLines = (sal_uInt16) ceil( sqrt( (double) GetItemCount() ) );
    }

    sal_Bool bPopup = mpData->mbAssumePopupMode;
    ToolBox *pThis = (ToolBox*) this;
    pThis->mpData->mbAssumePopupMode = sal_True;

    Size aSize = CalcFloatingWindowSizePixel( nLines );

    pThis->mpData->mbAssumePopupMode = bPopup;
    return aSize;
}

Size ToolBox::CalcFloatingWindowSizePixel() const
{
    sal_uInt16 nLines = ImplCountLineBreaks( this );
    ++nLines; // add the first line
    return CalcFloatingWindowSizePixel( nLines );
}

Size ToolBox::CalcFloatingWindowSizePixel( sal_uInt16 nCalcLines ) const
{
    sal_Bool bFloat = mpData->mbAssumeFloating;
    sal_Bool bDocking = mpData->mbAssumeDocked;

    // simulate floating mode and force reformat before calculating
    ToolBox *pThis = (ToolBox*) this;
    pThis->mpData->mbAssumeFloating = sal_True;
    pThis->mpData->mbAssumeDocked = sal_False;

    Size aSize = ImplCalcFloatSize( (ToolBox*) this, nCalcLines );

    pThis->mbFormat = sal_True;
    pThis->mpData->mbAssumeFloating = bFloat;
    pThis->mpData->mbAssumeDocked = bDocking;

    return aSize;
}

// -----------------------------------------------------------------------

Size ToolBox::CalcMinimumWindowSizePixel() const
{
    if( ImplIsFloatingMode() )
        return ImplCalcSize( this, mnFloatLines );
    else
    {
        // create dummy toolbox for measurements
        ToolBox *pToolBox = new ToolBox( GetParent(), GetStyle() );

        // copy until first useful item
        std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
        while( it != mpData->m_aItems.end() )
        {
            pToolBox->CopyItem( *this, it->mnId );
            if( (it->meType != TOOLBOXITEM_BUTTON) ||
                !it->mbVisible || ImplIsFixedControl( &(*it) ) )
                ++it;
            else
                break;
        }

        // add to docking manager if required to obtain a drag area
        // (which is accounted for in calcwindowsizepixel)
        if( ImplGetDockingManager()->GetDockingWindowWrapper( this ) )
            ImplGetDockingManager()->AddWindow( pToolBox );

        // account for menu
        if( IsMenuEnabled() )
            pToolBox->SetMenuType( GetMenuType() );

        pToolBox->SetAlign( GetAlign() );
        Size aSize = pToolBox->CalcWindowSizePixel( 1 );

        ImplGetDockingManager()->RemoveWindow( pToolBox );
        pToolBox->Clear();
        delete pToolBox;

        return aSize;
    }
}

// -----------------------------------------------------------------------

void ToolBox::EnableCustomize( sal_Bool bEnable )
{
    if ( bEnable != mbCustomize )
    {
        mbCustomize = bEnable;

        ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
        if ( bEnable )
            pMgr->push_back( this );
        else
            pMgr->erase( this );
    }
}

// -----------------------------------------------------------------------

void ToolBox::GetFocus()
{
    DockingWindow::GetFocus();
}

// -----------------------------------------------------------------------

void ToolBox::LoseFocus()
{
    ImplChangeHighlight( NULL, sal_True );

    DockingWindow::LoseFocus();
}

// -----------------------------------------------------------------------

// performs the action associated with an item, ie simulates clicking the item
void ToolBox::TriggerItem( sal_uInt16 nItemId, sal_Bool bShift, sal_Bool bCtrl )
{
    mnHighItemId = nItemId;
    sal_uInt16 nModifier = 0;
    if( bShift )
        nModifier |= KEY_SHIFT;
    if( bCtrl )
        nModifier |= KEY_MOD1;
    KeyCode aKeyCode( 0, nModifier );
    ImplActivateItem( aKeyCode );
}

// -----------------------------------------------------------------------

// calls the button's action handler
// returns sal_True if action was called
sal_Bool ToolBox::ImplActivateItem( KeyCode aKeyCode )
{
    sal_Bool bRet = sal_True;
    if( mnHighItemId )
    {
        ImplToolItem *pToolItem = ImplGetItem( mnHighItemId );

        // #107712#, activate can also be called for disabled entries
        if( pToolItem && !pToolItem->mbEnabled )
            return sal_True;

        if( pToolItem && pToolItem->mpWindow && HasFocus() )
        {
            ImplHideFocus();
            mbChangingHighlight = sal_True;  // avoid focus change due to loose focus
            pToolItem->mpWindow->ImplControlFocus( GETFOCUS_TAB );
            mbChangingHighlight = sal_False;
        }
        else
        {
            mnDownItemId = mnCurItemId = mnHighItemId;
            ImplToolItem* pItem = ImplGetItem( mnHighItemId );
            if ( pItem->mnBits & TIB_AUTOCHECK )
            {
                if ( pItem->mnBits & TIB_RADIOCHECK )
                {
                    if ( pItem->meState != STATE_CHECK )
                        SetItemState( pItem->mnId, STATE_CHECK );
                }
                else
                {
                    if ( pItem->meState != STATE_CHECK )
                        pItem->meState = STATE_CHECK;
                    else
                        pItem->meState = STATE_NOCHECK;
                }
            }
            mnMouseModifier = aKeyCode.GetModifier();
            mbIsKeyEvent = sal_True;
            Activate();
            Click();

            // #107776# we might be destroyed in the selecthandler
            ImplDelData aDelData;
            ImplAddDel( &aDelData );
            Select();
            if ( aDelData.IsDelete() )
                return bRet;
            ImplRemoveDel( &aDelData );

            Deactivate();
            mbIsKeyEvent = sal_False;
            mnMouseModifier = 0;
        }
    }
    else
        bRet = sal_False;
    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool ImplCloseLastPopup( Window *pParent )
{
    // close last popup toolbox (see also:
    // ImplHandleMouseFloatMode(...) in winproc.cxx )

    if( ImplGetSVData()->maWinData.mpFirstFloat )
    {
        FloatingWindow* pLastLevelFloat = ImplGetSVData()->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
        // only close the floater if it is not our direct parent, which would kill ourself
        if( pLastLevelFloat && pLastLevelFloat != pParent )
        {
            pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
            return sal_True;
        }
    }
    return sal_False;
}

// opens a drop down toolbox item
// returns sal_True if item was opened
sal_Bool ToolBox::ImplOpenItem( KeyCode aKeyCode )
{
    sal_uInt16 nCode = aKeyCode.GetCode();
    sal_Bool bRet = sal_True;

    // arrow keys should work only in the opposite direction of alignment (to not break cursor travelling)
    if ( ((nCode == KEY_LEFT || nCode == KEY_RIGHT) && IsHorizontal())
      || ((nCode == KEY_UP   || nCode == KEY_DOWN)  && !IsHorizontal()) )
        return sal_False;

    if( IsMenuEnabled() && mpData->mbMenubuttonSelected )
    {
        if( ImplCloseLastPopup( GetParent() ) )
            return bRet;

        UpdateCustomMenu();
        Application::PostUserEvent( mpData->mnEventId, LINK( this, ToolBox, ImplCallExecuteCustomMenu ) );
    }
    else if( mnHighItemId &&  ImplGetItem( mnHighItemId ) &&
        (ImplGetItem( mnHighItemId )->mnBits & TIB_DROPDOWN) )
    {
        if( ImplCloseLastPopup( GetParent() ) )
            return bRet;

        mnDownItemId = mnCurItemId = mnHighItemId;
        mnCurPos = GetItemPos( mnCurItemId );
        mnLastFocusItemId = mnCurItemId; // save item id for possible later focus restore
        mnMouseModifier = aKeyCode.GetModifier();
        mbIsShift = sal_True;
        mbIsKeyEvent = sal_True;
        Activate();

        mpData->mbDropDownByKeyboard = sal_True;
        GetDropdownClickHdl().Call( this );

        mbIsKeyEvent = sal_False;
        mbIsShift = sal_False;
        mnMouseModifier = 0;
    }
    else
        bRet = sal_False;

    return bRet;
}

// -----------------------------------------------------------------------

void ToolBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    mnKeyModifier = aKeyCode.GetModifier();
    sal_uInt16 nCode = aKeyCode.GetCode();
    sal_Bool bParentIsDialog = ( ( ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL) ) == WB_DIALOGCONTROL );
    sal_Bool bForwardKey = sal_False;
    sal_Bool bGrabFocusToDocument = sal_False;

    // #107776# we might be destroyed in the keyhandler
    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    switch ( nCode )
    {
        case KEY_UP:
        {
            // Ctrl-Cursor activates next toolbox, indicated by a blue arrow pointing to the left/up
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( !IsHorizontal() )
                ImplChangeHighlightUpDn( sal_True );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_LEFT:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( IsHorizontal() )
                ImplChangeHighlightUpDn( sal_True );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_DOWN:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( !IsHorizontal() )
                ImplChangeHighlightUpDn( sal_False );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_RIGHT:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( IsHorizontal() )
                ImplChangeHighlightUpDn( sal_False );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_PAGEUP:
            if ( mnCurLine > 1 )
            {
                if( mnCurLine > mnVisLines )
                    mnCurLine = mnCurLine - mnVisLines;
                else
                    mnCurLine = 1;
                mbFormat = sal_True;
                ImplFormat();
                ImplDrawSpin( sal_False, sal_False );
                ImplChangeHighlight( ImplGetFirstValidItem( mnCurLine ) );
            }
        break;
        case KEY_PAGEDOWN:
            if ( mnCurLine+mnVisLines-1 < mnCurLines )
            {
                if( mnCurLine + 2*mnVisLines-1 < mnCurLines )
                    mnCurLine = mnCurLine + mnVisLines;
                else
                    mnCurLine = mnCurLines;
                mbFormat = sal_True;
                ImplFormat();
                ImplDrawSpin( sal_False, sal_False );
                ImplChangeHighlight( ImplGetFirstValidItem( mnCurLine ) );
            }
        break;
        case KEY_END:
            {
                ImplChangeHighlight( NULL );
                ImplChangeHighlightUpDn( sal_False );
            }
            break;
        case KEY_HOME:
            {
                ImplChangeHighlight( NULL );
                ImplChangeHighlightUpDn( sal_True );
            }
            break;
        case KEY_ESCAPE:
        {
            if( !ImplIsFloatingMode() && bParentIsDialog )
                DockingWindow::KeyInput( rKEvt );
            else
            {
                // send focus to document pane
                Window *pWin = this;
                while( pWin )
                {
                    if( !pWin->GetParent() )
                    {
                        pWin->ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->GrabFocus();
                        break;
                    }
                    pWin = pWin->GetParent();
                }
            }
        }
        break;
        case KEY_RETURN:
        {
            // #107712#, disabled entries are selectable now
            //  leave toolbox and move focus to document
            if( mnHighItemId )
            {
                ImplToolItem *pItem = ImplGetItem( mnHighItemId );
                if( !pItem->mbEnabled )
                {
                    Sound::Beep( SOUND_DISABLE, this );
                    bGrabFocusToDocument = sal_True;
                }
            }
            if( !bGrabFocusToDocument )
                bForwardKey = !ImplActivateItem( aKeyCode );
        }
        break;
        default:
            {
            sal_uInt16 aKeyGroup = aKeyCode.GetGroup();
            ImplToolItem *pItem = NULL;
            if( mnHighItemId )
                pItem = ImplGetItem( mnHighItemId );
            // #i13931# forward alphanum keyinput into embedded control
            if( (aKeyGroup == KEYGROUP_NUM || aKeyGroup == KEYGROUP_ALPHA ) && pItem && pItem->mpWindow && pItem->mbEnabled )
            {
                Window *pFocusWindow = Application::GetFocusWindow();
                ImplHideFocus();
                mbChangingHighlight = sal_True;  // avoid focus change due to loose focus
                pItem->mpWindow->ImplControlFocus( GETFOCUS_TAB );
                mbChangingHighlight = sal_False;
                if( pFocusWindow != Application::GetFocusWindow() )
                    Application::GetFocusWindow()->KeyInput( rKEvt );
            }
            else
            {
                // do nothing to avoid key presses going into the document
                // while the toolbox has the focus
                // just forward function and special keys and combinations with Alt-key
                if( aKeyGroup == KEYGROUP_FKEYS || aKeyGroup == KEYGROUP_MISC || aKeyCode.IsMod2() )
                    bForwardKey = sal_True;
            }
        }
    }

    if ( aDelData.IsDelete() )
        return;
    ImplRemoveDel( &aDelData );

    // #107251# move focus away if this toolbox was disabled during keyinput
    if( HasFocus() && mpData->mbKeyInputDisabled && (ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL) ) == WB_DIALOGCONTROL)
    {
        sal_uInt16 n = 0;
        Window *pFocusControl = ImplGetParent()->ImplGetDlgWindow( n, DLGWINDOW_FIRST );
        if ( pFocusControl && pFocusControl != this )
            pFocusControl->ImplControlFocus( GETFOCUS_INIT );
    }

    mnKeyModifier = 0;

    // #107712#, leave toolbox
    if( bGrabFocusToDocument )
    {
        GrabFocusToDocument();
        return;
    }

    if( bForwardKey )
        DockingWindow::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

// returns the current toolbox line of the item
sal_uInt16 ToolBox::ImplGetItemLine( ImplToolItem* pCurrentItem )
{
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    sal_uInt16 nLine = 1;
    while( it != mpData->m_aItems.end() )
    {
        if ( it->mbBreak )
            ++nLine;
        if( &(*it) == pCurrentItem)
            break;
        ++it;
    }
    return nLine;
}

// returns the first displayable item in the given line
ImplToolItem* ToolBox::ImplGetFirstValidItem( sal_uInt16 nLine )
{
    if( !nLine || nLine > mnCurLines )
        return NULL;

    nLine--;

    std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        // find correct line
        if ( it->mbBreak )
            nLine--;
        if( !nLine )
        {
            // find first useful item
            while( it != mpData->m_aItems.end() && ((it->meType != TOOLBOXITEM_BUTTON) ||
                /*!it->mbEnabled ||*/ !it->mbVisible || ImplIsFixedControl( &(*it) )) )
            {
                ++it;
                if( it == mpData->m_aItems.end() || it->mbBreak )
                    return NULL;    // no valid items in this line
            }
            return &(*it);
        }
        ++it;
    }

    return (it == mpData->m_aItems.end()) ? NULL : &(*it);
}

// -----------------------------------------------------------------------

sal_uInt16 ToolBox::ImplFindItemPos( const ImplToolItem* pItem, const std::vector< ImplToolItem >& rList )
{
    if( pItem )
    {
        sal_uInt16 nPos;
        for( nPos = 0; nPos < rList.size(); ++nPos )
            if( &rList[ nPos ] == pItem )
                return nPos;
    }
    return TOOLBOX_ITEM_NOTFOUND;
}

void ToolBox::ChangeHighlight( sal_uInt16 nPos )
{
    if ( nPos < GetItemCount() ) {
        ImplGrabFocus( 0 );
        ImplChangeHighlight ( ImplGetItem ( GetItemId ( (sal_uInt16) nPos ) ), sal_False );
    }
}

void ToolBox::ImplChangeHighlight( ImplToolItem* pItem, sal_Bool bNoGrabFocus )
{
    // avoid recursion due to focus change
    if( mbChangingHighlight )
        return;

    mbChangingHighlight = sal_True;

    ImplToolItem* pOldItem = NULL;

    if ( mnHighItemId )
    {
        ImplHideFocus();
        sal_uInt16 nPos = GetItemPos( mnHighItemId );
        pOldItem = ImplGetItem( mnHighItemId );
        // #i89962# ImplDrawItem can cause Invalidate/Update
        // which will in turn ImplShowFocus again
        // set mnHighItemId to 0 already to prevent this hen/egg problem
        mnHighItemId = 0;
        ImplDrawItem( nPos, 0 );
        ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, reinterpret_cast< void* >( nPos ) );
    }

    if( !bNoGrabFocus && pItem != pOldItem && pOldItem && pOldItem->mpWindow )
    {
        // move focus into toolbox
        GrabFocus();
    }

    if( pItem )
    {
        sal_uInt16 aPos = ToolBox::ImplFindItemPos( pItem, mpData->m_aItems );
        if( aPos != TOOLBOX_ITEM_NOTFOUND)
        {
            // check for line breaks
            sal_uInt16 nLine = ImplGetItemLine( pItem );

            if( nLine >= mnCurLine + mnVisLines )
            {
                mnCurLine = nLine - mnVisLines + 1;
                mbFormat = sal_True;
            }
            else if ( nLine < mnCurLine )
            {
                mnCurLine = nLine;
                mbFormat = sal_True;
            }

            if( mbFormat )
            {
                ImplFormat();
            }

            mnHighItemId = pItem->mnId;
            ImplDrawItem( aPos, 2 );    // always use shadow effect (2)

            if( mbSelection )
                mnCurPos = aPos;
            ImplShowFocus();

            ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHT );
        }
    }
    else
    {
        ImplHideFocus();
        mnHighItemId = 0;
        mnCurPos = TOOLBOX_ITEM_NOTFOUND;
    }

    mbChangingHighlight = sal_False;
}

// -----------------------------------------------------------------------

// check for keyboard accessible items
static sal_Bool ImplIsValidItem( const ImplToolItem* pItem, sal_Bool bNotClipped )
{
    sal_Bool bValid = (pItem && pItem->meType == TOOLBOXITEM_BUTTON && pItem->mbVisible && !ImplIsFixedControl( pItem ));
    if( bValid && bNotClipped && pItem->IsClipped() )
        bValid = sal_False;
    return bValid;
}

// -----------------------------------------------------------------------

sal_Bool ToolBox::ImplChangeHighlightUpDn( sal_Bool bUp, sal_Bool bNoCycle )
{
    ImplToolItem* pToolItem = ImplGetItem( mnHighItemId );

    if( !pToolItem || !mnHighItemId )
    {
        // menubutton highlighted ?
        if( mpData->mbMenubuttonSelected )
        {
            if( bUp )
            {
                // select last valid non-clipped item
                std::vector< ImplToolItem >::iterator it = mpData->m_aItems.end();
                ImplToolItem* pItem = NULL;
                while( it != mpData->m_aItems.begin() )
                {
                    --it;
                    if ( ImplIsValidItem( &(*it), sal_True ) )
                    {
                        pItem = &(*it);
                        break;
                    }
                }
                ImplDrawMenubutton( this, sal_False );
                ImplChangeHighlight( pItem );
            }
            else
            {
                // select first valid non-clipped item
                std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
                while( it != mpData->m_aItems.end() )
                {
                    if ( ImplIsValidItem( &(*it), sal_True ) )
                        break;
                    ++it;
                }
                if( it != mpData->m_aItems.end() )
                {
                    ImplDrawMenubutton( this, sal_False );
                    ImplChangeHighlight( &(*it) );
                }
            }
            return sal_True;
        }

        if( bUp )
        {
            // Select first valid item
            std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
            while( it != mpData->m_aItems.end() )
            {
                if ( ImplIsValidItem( &(*it), sal_False ) )
                    break;
                ++it;
            }

            // select the menu button if a clipped item would be selected
            if( (it != mpData->m_aItems.end() && &(*it) == ImplGetFirstClippedItem( this )) && IsMenuEnabled() )
            {
                ImplChangeHighlight( NULL );
                ImplDrawMenubutton( this, sal_True );
            }
            else
                ImplChangeHighlight( (it != mpData->m_aItems.end()) ? &(*it) : NULL );
            return sal_True;
        }
        else
        {
            // Select last valid item

            // docked toolbars have the menubutton as last item - if this button is enabled
            if( IsMenuEnabled() && !ImplIsFloatingMode() )
            {
                ImplChangeHighlight( NULL );
                ImplDrawMenubutton( this, sal_True );
            }
            else
            {
                std::vector< ImplToolItem >::iterator it = mpData->m_aItems.end();
                ImplToolItem* pItem = NULL;
                while( it != mpData->m_aItems.begin() )
                {
                    --it;
                    if ( ImplIsValidItem( &(*it), sal_False ) )
                    {
                        pItem = &(*it);
                        break;
                    }
                }
                ImplChangeHighlight( pItem );
            }
            return sal_True;
        }
    }

    if( pToolItem )
    {
        sal_uLong pos = ToolBox::ImplFindItemPos( pToolItem, mpData->m_aItems );
        sal_uLong nCount = mpData->m_aItems.size();

        sal_uLong i=0;
        do
        {
            if( bUp )
            {
                if( !pos-- )
                {
                    if( bNoCycle )
                        return sal_False;

                    // highlight the menu button if it is the last item
                    if( IsMenuEnabled() && !ImplIsFloatingMode() )
                    {
                        ImplChangeHighlight( NULL );
                        ImplDrawMenubutton( this, sal_True );
                        return sal_True;
                    }
                    else
                        pos = nCount-1;
                }
            }
            else
            {
                if( ++pos >= nCount )
                {
                    if( bNoCycle )
                        return sal_False;

                    // highlight the menu button if it is the last item
                    if( IsMenuEnabled() && !ImplIsFloatingMode() )
                    {
                        ImplChangeHighlight( NULL );
                        ImplDrawMenubutton( this, sal_True );
                        return sal_True;
                    }
                    else
                        pos = 0;
                }
            }

            pToolItem = &mpData->m_aItems[pos];

            if ( ImplIsValidItem( pToolItem, sal_False ) )
                break;

        } while( ++i < nCount);

        if( pToolItem->IsClipped() && IsMenuEnabled() )
        {
            // select the menu button if a clipped item would be selected
            ImplChangeHighlight( NULL );
            ImplDrawMenubutton( this, sal_True );
        }
        else if( i != nCount )
            ImplChangeHighlight( pToolItem );
        else
            return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

void ToolBox::ImplShowFocus()
{
    if( mnHighItemId && HasFocus() )
    {
        ImplToolItem* pItem = ImplGetItem( mnHighItemId );
        if( pItem->mpWindow )
        {
            Window *pWin = pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow ? pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow : pItem->mpWindow;
            pWin->ImplGetWindowImpl()->mbDrawSelectionBackground = sal_True;
            pWin->Invalidate( 0 );
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplHideFocus()
{
    if( mnHighItemId )
    {
        ImplToolItem* pItem = ImplGetItem( mnHighItemId );
        if( pItem->mpWindow )
        {
            Window *pWin = pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow ? pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow : pItem->mpWindow;
            pWin->ImplGetWindowImpl()->mbDrawSelectionBackground = sal_False;
            pWin->Invalidate( 0 );
        }
    }

    if ( mpData->mbMenubuttonSelected )
    {
        // remove highlight from menubutton
        ImplDrawMenubutton( this, sal_False );
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplDisableFlatButtons()
{
#ifdef WNT        // Check in the Windows registry if an AT tool wants no flat toolboxes
    static bool bInit = false, bValue = false;
    if( ! bInit )
    {
        bInit = true;
        HKEY hkey;

        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
            "Software\\OpenOffice.org\\Accessibility\\AtToolSupport",
            &hkey) )
        {
            DWORD dwType = 0;
            sal_uInt8 Data[6]; // possible values: "true", "false", "1", "0", DWORD
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueEx(hkey, "DisableFlatToolboxButtons",
                NULL, &dwType, Data, &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        bValue = ((0 == stricmp((const char *) Data, "1")) || (0 == stricmp((const char *) Data, "true")));
                        break;
                    case REG_DWORD:
                        bValue = (bool)(((DWORD *) Data)[0]);
                        break;
                }
            }
            RegCloseKey(hkey);
        }
    }
    if( bValue )
        mnOutStyle &= ~TOOLBOX_STYLE_FLAT;
#endif
}

ToolBoxLayoutMode ToolBox::GetToolbarLayoutMode()
{
    return meLayoutMode;
}

void ToolBox::SetToolbarLayoutMode( ToolBoxLayoutMode eLayout )
{
    if ( meLayoutMode != eLayout )
       meLayoutMode  = eLayout;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
