/*************************************************************************
 *
 *  $RCSfile: toolbox.cxx,v $
 *
 *  $Revision: 1.72 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:01:55 $
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

#include <string.h>
#include <vector>
#include <math.h>

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_ACCEL_HXX
#include <accel.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_SPIN_H
#include <spin.h>
#endif
#define private public
#ifndef _SV_TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#undef private
#ifndef _SV_TOOLBOX_H
#include <toolbox.h>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <mnemonic.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif


// =======================================================================

DBG_NAMEEX( Window );

// =======================================================================

#define SMALLBUTTON_HSIZE           7
#define SMALLBUTTON_VSIZE           7

#define SMALLBUTTON_OFF_NORMAL_X    3
#define SMALLBUTTON_OFF_NORMAL_Y    3
#define SMALLBUTTON_OFF_CHECKED_X   4
#define SMALLBUTTON_OFF_CHECKED_Y   4
#define SMALLBUTTON_OFF_PRESSED_X   5
#define SMALLBUTTON_OFF_PRESSED_Y   5

#define OUTBUTTON_SIZE              6
#define OUTBUTTON_BORDER            4
#define OUTBUTTON_OFF_NORMAL_X      1
#define OUTBUTTON_OFF_NORMAL_Y      1

// -----------------------------------------------------------------------

#define DEF_MIN_WIDTH           8
#define DEF_MIN_HEIGHT          8
#define DEF_TEXT_WIDTH          40

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

#define TB_MAX_GROUPS           100

#define DOCK_LINEHSIZE          ((USHORT)0x0001)
#define DOCK_LINEVSIZE          ((USHORT)0x0002)
#define DOCK_LINERIGHT          ((USHORT)0x1000)
#define DOCK_LINEBOTTOM         ((USHORT)0x2000)
#define DOCK_LINELEFT           ((USHORT)0x4000)
#define DOCK_LINETOP            ((USHORT)0x8000)
#define DOCK_LINEOFFSET         3


// -----------------------------------------------------------------------
static Size ImplCalcFloatSize( ToolBox* pThis, USHORT& rLines );
// -----------------------------------------------------------------------

// Hue, 360 degree
// Saturation, 100 %
// Brightness, 100 %

void ImplRGBtoHSB( const Color& rColor, USHORT& nHue, USHORT& nSat, USHORT& nBri )
{
    UINT8 c[3];
    UINT8 cMax, cMin;

    c[0] = rColor.GetRed();
    c[1] = rColor.GetGreen();
    c[2] = rColor.GetBlue();

    cMax = c[0];
    if( c[1] > cMax )
        cMax = c[1];
    if( c[2] > cMax )
        cMax = c[2];

    // Brightness = max(R, G, B);
    nBri = cMax * 100 / 255;

    cMin = c[0];
    if( c[1] < cMin )
        cMin = c[1];
    if( c[2] < cMin )
        cMin = c[2];

    UINT8 cDelta = cMax - cMin;

    // Saturation = max - min / max
    if( nBri > 0 )
        nSat = cDelta * 100 / cMax;
    else
        nSat = 0;

    if( nSat == 0 )
        nHue = 0; // Default = undefined
    else
    {
        double dHue = 0.0;

        if( c[0] == cMax )
        {
            dHue = (double)( c[1] - c[2] ) / (double)cDelta;
        }
        else if( c[1] == cMax )
        {
            dHue = 2.0 + (double)( c[2] - c[0] ) / (double)cDelta;
        }
        else if ( c[2] == cMax )
        {
            dHue = 4.0 + (double)( c[0] - c[1] ) / (double)cDelta;
        }
        dHue *= 60.0;

        if( dHue < 0.0 )
            dHue += 360.0;

        nHue = (UINT16) dHue;
    }
}


Color ImplHSBtoRGB( USHORT nHue, USHORT nSat, USHORT nBri )
{
    UINT8 cR=0,cG=0,cB=0;
    UINT8 nB = (UINT8) ( nBri * 255 / 100 );

    if( nSat == 0 )
    {
        cR = nB;
        cG = nB;
        cB = nB;
    }
    else
    {
        double dH = nHue;
        double f;
        UINT16 n;
        if( dH == 360.0 )
            dH = 0.0;

        dH /= 60.0;
        n = (UINT16) dH;
        f = dH - n;

        UINT8 a = (UINT8) ( nB * ( 100 - nSat ) / 100 );
        UINT8 b = (UINT8) ( nB * ( 100 - ( (double)nSat * f + 0.5 ) ) / 100 );
        UINT8 c = (UINT8) ( nB * ( 100 - ( (double)nSat * ( 1.0 - f ) + 0.5 ) ) / 100 );

        switch( n )
        {
            case 0: cR = nB;    cG = c;     cB = a;     break;
            case 1: cR = b;     cG = nB;    cB = a;     break;
            case 2: cR = a;     cG = nB;    cB = c;     break;
            case 3: cR = a;     cG = b;     cB = nB;    break;
            case 4: cR = c;     cG = a;     cB = nB;    break;
            case 5: cR = nB;    cG = a;     cB = b;     break;
        }
    }

    return( Color( cR, cG, cB ) );
}

// -----------------------------------------------------------------------

struct ImplToolSize
{
    long            mnWidth;
    long            mnHeight;
    USHORT          mnLines;
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

DECLARE_LIST( ImplTBList, ToolBox* );

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
    USHORT          mnLineMode;
    USHORT          mnStartLines;
    void*           mpCustomizeData;
    BOOL            mbCustomizeMode;
    BOOL            mbResizeMode;
    BOOL            mbShowDragRect;

public:
                    ImplTBDragMgr();
                    ~ImplTBDragMgr();

    void            Insert( ToolBox* pBox )
                        { mpBoxList->Insert( pBox ); }
    void            Remove( ToolBox* pBox )
                        { mpBoxList->Remove( pBox ); }
    ULONG           Count() const
                        { return mpBoxList->Count(); }

    ToolBox*        FindToolBox( const Rectangle& rRect );

    void            StartDragging( ToolBox* pDragBox,
                                   const Point& rPos, const Rectangle& rRect,
                                   USHORT nLineMode, BOOL bResizeItem,
                                   void* pData = NULL );
    void            Dragging( const Point& rPos );
    void            EndDragging( BOOL bOK = TRUE );
    void            HideDragRect() { if ( mbShowDragRect ) mpDragBox->HideTracking(); }
    void            UpdateDragRect();
                    DECL_LINK( SelectHdl, Accelerator* );

    void            StartCustomizeMode();
    void            EndCustomizeMode();
    BOOL            IsCustomizeMode() { return mbCustomizeMode; }
    BOOL            IsResizeMode() { return mbResizeMode; }
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

static void ImplDrawConfigFrame( ToolBox* pThis, const Rectangle& rRect )
{
}

// -----------------------------------------------------------------------

static void ImplUpdateDragArea( ToolBox *pThis )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );
    if( pWrapper )
    {
        if ( pThis->ImplIsFloatingMode() || pWrapper->IsLocked() )
            pWrapper->SetDragArea( Rectangle() );
        else
        {
            if( pThis->meAlign == WINDOWALIGN_TOP || pThis->meAlign == WINDOWALIGN_BOTTOM )
                pWrapper->SetDragArea( Rectangle( 0, 0, TB_DRAGWIDTH, pThis->GetOutputSizePixel().Height() ) );
            else
                pWrapper->SetDragArea( Rectangle( 0, 0, pThis->GetOutputSizePixel().Width(), TB_DRAGWIDTH ) );
        }
    }
}

// -----------------------------------------------------------------------

static void ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
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
    int    dragwidth = ( pWrapper && !pWrapper->IsLocked() ) ? TB_DRAGWIDTH : 0;

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

static void ImplDrawGrip( ToolBox* pThis )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );
    if( pWrapper && !pWrapper->GetDragArea().IsEmpty() )
    {
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
                pThis->DrawLine( Point(4, i), Point( 4+TB_DRAGWIDTH-6, i) );
                i += 2;
            }
        }
        else
        {
            int width = (int) (0.6 * aSz.Width() + 0.5);
            int i = (aSz.Width() - width) / 2;
            width += i;
            while( i <= width )
            {
                pThis->DrawLine( Point(i, 4), Point( i, 4+TB_DRAGWIDTH-6) );
                i += 2;
            }
        }
    }
}

static void ImplDrawBorderEffects( ToolBox* pThis, ImplDockingWindowWrapper *pWrapper )
{
    // make 'round' corners and provide a special border
    if( pWrapper && !pThis->ImplIsFloatingMode() )
    {
        Color startCol, endCol;
        startCol = Color( COL_WHITE );
        endCol = pThis->GetSettings().GetStyleSettings().GetFaceColor();
        if( endCol.IsDark() )
            // no 'extreme' gradient when high contrast
            startCol = endCol;

        Point aPt;
        Rectangle aRect( aPt, pThis->GetOutputSizePixel() );

        long nTmp = pThis->mbHorz ? aRect.nRight : aRect.nBottom;
        // draw lines up to the menu button only
        if( !pThis->mpData->maMenubuttonItem.maRect.IsEmpty() )
            if( pThis->mbHorz )
                aRect.nRight = pThis->mpData->maMenubuttonItem.maRect.nLeft-1;
            else
                aRect.nBottom = pThis->mpData->maMenubuttonItem.maRect.nTop-1;

        pThis->SetLineColor( startCol );    // typically white

        if( pThis->mbHorz )
            pThis->DrawLine( aRect.TopLeft(),aRect.TopRight() );
        else
            pThis->DrawLine( aRect.TopRight(),aRect.BottomRight() );

        pThis->SetLineColor( endCol );
        if( pThis->mbHorz )
        {
            pThis->DrawLine( aRect.TopLeft(),aRect.BottomLeft() );
            pThis->DrawLine( aRect.BottomLeft(),aRect.BottomRight() );
        }
        else
        {
            pThis->DrawLine( aRect.TopRight(),aRect.TopLeft() );
            pThis->DrawLine( aRect.TopLeft(),aRect.BottomLeft() );
        }

        // this line is placed beyond the menu button
        if( pThis->mbHorz )
            aRect.nRight = nTmp;
        else
            aRect.nBottom = nTmp;

        //pThis->SetLineColor( pThis->GetSettings().GetStyleSettings().GetShadowColor() );
        pThis->SetLineColor( endCol );
        if( pThis->mbHorz )
            pThis->DrawLine( aRect.TopRight(),aRect.BottomRight() );
        else
            pThis->DrawLine( aRect.BottomRight(),aRect.BottomLeft() );

        // make 'round' corners
        if( pThis->mbHorz )
            { aRect.nLeft++; aRect.nRight--; }
        else
            { aRect.nTop++; aRect.nBottom--; }
        pThis->SetLineColor( pThis->GetSettings().GetStyleSettings().GetFaceColor() );
        pThis->DrawLine( aRect.TopLeft(),aRect.TopLeft() );
        pThis->DrawLine( aRect.TopRight(),aRect.TopRight() );
        pThis->DrawLine( aRect.BottomLeft(),aRect.BottomLeft() );
        pThis->DrawLine( aRect.BottomRight(),aRect.BottomRight() );
    }
}

static void ImplDrawGradient( ToolBox* pThis, const Rectangle &rRect )
{
    if( !(((Window*)pThis)->GetStyle() & WB_3DLOOK) )
        return;

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );

    Color aColor = pThis->GetSettings().GetStyleSettings().GetFaceColor();

    if( !pWrapper )
    {
        // no gradient for ordinary toolbars (not dockable)
        BOOL bFillColor = pThis->IsFillColor();
        BOOL bLineColor = pThis->IsLineColor();
        Color aOldFillCol = pThis->GetFillColor();
        Color aOldLineCol = pThis->GetLineColor();
        pThis->SetFillColor( aColor );
        pThis->SetLineColor();
        pThis->DrawRect( rRect );
        if( bFillColor )
            pThis->SetFillColor( aOldFillCol );
        if( bLineColor )
            pThis->SetLineColor( aOldLineCol );
        return;
    }

    Color startCol, endCol;
    startCol = Color( COL_WHITE );
    endCol = pThis->GetSettings().GetStyleSettings().GetFaceColor();
    if( endCol.IsDark() )
        // no 'extreme' gradient when high contrast
        startCol = endCol;

    Gradient g;
    g.SetAngle( pThis->mbHorz ? 0 : 900 );
    g.SetStyle( GRADIENT_LINEAR );

    g.SetStartColor( startCol );
    g.SetEndColor( endCol );

    BOOL bLineColor = pThis->IsLineColor();
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
        BOOL bDrawSep = FALSE; // pThis->ImplIsFloatingMode() && ( pThis->mnWinStyle & WB_LINESPACING );

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

    // round corners etc.
    ImplDrawBorderEffects( pThis, pWrapper );

    if( bLineColor )
        pThis->SetLineColor( aOldCol );
}

static void ImplErase( ToolBox* pThis, const Rectangle &rRect, BOOL bHighlight = FALSE )
{
    // no gradient when highlighting items, just use the (white) background
    if( bHighlight || ! (((Window*) pThis)->GetStyle() & WB_3DLOOK ) )
    {
        if( (((Window*) pThis)->GetStyle() & WB_3DLOOK ) )
        {
            pThis->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pThis->SetLineColor();
            pThis->SetFillColor( Color( COL_WHITE ) );
            pThis->DrawRect( rRect );
            pThis->Pop();
        }
        else
            pThis->Erase( rRect );
    }
    else
    {
        BOOL bClipRegion = pThis->IsClipRegion();
        Region aOldRgn = pThis->GetClipRegion();
        pThis->SetClipRegion( Region( rRect ) );
        ImplDrawGradient( pThis, rRect );
        if( bClipRegion )
            pThis->SetClipRegion( aOldRgn );
        else
            pThis->SetClipRegion();
    }
}

static void ImplDrawBorder( ToolBox* pWin )
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

static bool ImplIsFixedControl( ImplToolItem *pItem )
{
    return ( pItem->mpWindow &&
            (pItem->mpWindow->GetType() == WINDOW_FIXEDTEXT ||
             pItem->mpWindow->GetType() == WINDOW_FIXEDLINE ||
             pItem->mpWindow->GetType() == WINDOW_GROUPBOX) );
}

// -----------------------------------------------------------------------

static Size ImplCalcSize( const ToolBox* pThis,
                          USHORT nCalcLines, USHORT nCalcMode = 0 )
{
    long            nMax;
    long            nLeft;
    long            nTop;
    long            nRight;
    long            nBottom;
    Size            aSize;
    WindowAlign     eOldAlign = pThis->meAlign;
    BOOL            bOldHorz = pThis->mbHorz;
    BOOL            bOldAssumeDocked = pThis->mpData->mbAssumeDocked;
    BOOL            bOldAssumeFloating = pThis->mpData->mbAssumeFloating;

    if ( nCalcMode )
    {
        BOOL bOldFloatingMode = pThis->ImplIsFloatingMode();

        pThis->mpData->mbAssumeDocked = FALSE;
        pThis->mpData->mbAssumeFloating = FALSE;

        if ( nCalcMode == TB_CALCMODE_HORZ )
        {
            pThis->mpData->mbAssumeDocked = TRUE;   // force non-floating mode during calculation
            ImplCalcBorder( WINDOWALIGN_TOP, nLeft, nTop, nRight, nBottom, pThis );
            ((ToolBox*)pThis)->mbHorz = TRUE;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_TOP;
        }
        else if ( nCalcMode == TB_CALCMODE_VERT )
        {
            pThis->mpData->mbAssumeDocked = TRUE;   // force non-floating mode during calculation
            ImplCalcBorder( WINDOWALIGN_LEFT, nLeft, nTop, nRight, nBottom, pThis );
            ((ToolBox*)pThis)->mbHorz = FALSE;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_LEFT;
        }
        else if ( nCalcMode == TB_CALCMODE_FLOAT )
        {
            pThis->mpData->mbAssumeFloating = TRUE;   // force non-floating mode during calculation
            nLeft = nTop = nRight = nBottom = 0;
            ((ToolBox*)pThis)->mbHorz = TRUE;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_TOP;
        }

        if ( (pThis->meAlign != eOldAlign) || (pThis->mbHorz != bOldHorz) ||
             (pThis->ImplIsFloatingMode() != bOldFloatingMode ) )
            ((ToolBox*)pThis)->mbCalc = TRUE;
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
            ((ToolBox*)pThis)->mbCalc   = TRUE;
        }
    }

    if ( aSize.Width() )
        aSize.Width() += pThis->mnBorderX*2;
    if ( aSize.Height() )
        aSize.Height() += pThis->mnBorderY*2;

    return aSize;
}

// -----------------------------------------------------------------------

static void ImplCalcFloatSizes( ToolBox* pThis )
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

    USHORT  i;
    USHORT  nLines;
    USHORT  nCalcLines;
    USHORT  nTempLines;
    long    nHeight;
    long    nMaxLineWidth;
    nCalcLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, TRUE );

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
                nTempLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, TRUE );
            }
            while ( (nCalcSize < upperBoundWidth) && (nLines < nTempLines) && (nTempLines != 1) );
            if ( nTempLines < nLines )
                nLines = nTempLines;
        }
        i++;
    }
}

// -----------------------------------------------------------------------

static Size ImplCalcFloatSize( ToolBox* pThis, USHORT& rLines )
{
    ImplCalcFloatSizes( pThis );

    if ( !rLines )
    {
        rLines = pThis->mnFloatLines;
        if ( !rLines )
            rLines = pThis->mnLines;
    }

    USHORT i = 0;
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

static Size ImplCalcMinFloatSize( ToolBox* pThis )
{
    ImplCalcFloatSizes( pThis );

    USHORT i = 0;
    Size aMinSize( pThis->mpFloatSizeAry->mpSize[i].mnWidth, pThis->mpFloatSizeAry->mpSize[i].mnHeight );
    Size aMaxSize( pThis->mpFloatSizeAry->mpSize[i].mnWidth, pThis->mpFloatSizeAry->mpSize[i].mnHeight );
    while ( ++i <= pThis->mpFloatSizeAry->mnLastEntry )
    {
        if( pThis->mpFloatSizeAry->mpSize[i].mnWidth < aMinSize.Width() )
            aMinSize.Width() = pThis->mpFloatSizeAry->mpSize[i].mnWidth;
        if( pThis->mpFloatSizeAry->mpSize[i].mnHeight < aMinSize.Height() )
            aMinSize.Height() = pThis->mpFloatSizeAry->mpSize[i].mnHeight;

        if( pThis->mpFloatSizeAry->mpSize[i].mnWidth > aMaxSize.Width() )
            aMaxSize.Width() = pThis->mpFloatSizeAry->mpSize[i].mnWidth;
        if( pThis->mpFloatSizeAry->mpSize[i].mnHeight > aMaxSize.Height() )
            aMaxSize.Height() = pThis->mpFloatSizeAry->mpSize[i].mnHeight;
    }
    return aMinSize;
}

// -----------------------------------------------------------------------


static USHORT ImplCalcLines( ToolBox* pThis, long nToolSize )
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

    return (USHORT)(nToolSize/nLineHeight);
}

// -----------------------------------------------------------------------

static USHORT ImplTestLineSize( ToolBox* pThis, const Point& rPos )
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

static void ImplLineSizing( ToolBox* pThis, const Point& rPos, Rectangle& rRect,
                            USHORT nLineMode )
{
    BOOL    mbHorz;
    long    nOneLineSize;
    long    nCurSize;
    long    nMaxSize;
    long    nSize;
    Size    aSize;

    if ( nLineMode & DOCK_LINERIGHT )
    {
        nCurSize = rPos.X() - rRect.Left();
        mbHorz = FALSE;
    }
    else if ( nLineMode & DOCK_LINEBOTTOM )
    {
        nCurSize = rPos.Y() - rRect.Top();
        mbHorz = TRUE;
    }
    else if ( nLineMode & DOCK_LINELEFT )
    {
        nCurSize = rRect.Right() - rPos.X();
        mbHorz = FALSE;
    }
    else //if ( nLineMode & DOCK_LINETOP )
    {
        nCurSize = rRect.Bottom() - rPos.Y();
        mbHorz = TRUE;
    }

    Size    aWinSize = pThis->GetSizePixel();
    USHORT  nMaxLines = (pThis->mnLines > pThis->mnCurLines) ? pThis->mnLines : pThis->mnCurLines;
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

    USHORT i = 1;
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
    else //if ( nLineMode & DOCK_LINETOP )
        rRect.Top() = rRect.Bottom()-nSize;

    pThis->mnDockLines = i;
}

// -----------------------------------------------------------------------

static USHORT ImplFindItemPos( ToolBox* pBox, const Point& rPos )
{
    USHORT  nPos = 0;
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
    mpBoxList       = new ImplTBList( 4, 4 );
    mnLineMode      = 0;
    mnStartLines    = 0;
    mbCustomizeMode = FALSE;
    mbResizeMode    = FALSE;
    mbShowDragRect  = FALSE;
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
    ToolBox* pBox = mpBoxList->First();
    while ( pBox )
    {
        /*
         *  FIXME: since we can have multiple frames now we cannot
         *  find the drag target by its position alone.
         *  As long as the toolbar config dialogue is not a system window
         *  this works in one frame only anyway. If the dialogue
         *  changes to a system window, we need a new implementation here
         */
        if ( pBox->IsReallyVisible() && pBox->mpFrame == mpDragBox->mpFrame )
        {
            if ( !pBox->ImplIsFloatingMode() )
            {
                Point aPos = pBox->GetPosPixel();
                aPos = pBox->GetParent()->OutputToScreenPixel( aPos );
                Rectangle aTempRect( aPos, pBox->GetSizePixel() );
                if ( aTempRect.IsOver( rRect ) )
                    return pBox;
            }
        }

        pBox = mpBoxList->Next();
    }

    return pBox;
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::StartDragging( ToolBox* pToolBox,
                                   const Point& rPos, const Rectangle& rRect,
                                   USHORT nDragLineMode, BOOL bResizeItem,
                                   void* pData )
{
    mpDragBox = pToolBox;
    pToolBox->CaptureMouse();
    pToolBox->mbDragging = TRUE;
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
        if ( !mbResizeMode )
            ImplDrawConfigFrame( pToolBox, rRect );
        else
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
    mbShowDragRect = TRUE;
    pToolBox->ShowTracking( maRect );
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::Dragging( const Point& rPos )
{
    if ( mnLineMode )
    {
        ImplLineSizing( mpDragBox, rPos, maRect, mnLineMode );
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

void ImplTBDragMgr::EndDragging( BOOL bOK )
{
    mpDragBox->HideTracking();
    mpDragBox->ReleaseMouse();
    mpDragBox->mbDragging = FALSE;
    mbShowDragRect = FALSE;
    Application::RemoveAccel( &maAccel );

    if ( mnLineMode )
    {
        if ( !bOK )
        {
            mpDragBox->mnDockLines = mnStartLines;
            mpDragBox->EndDocking( maStartRect, FALSE );
        }
        else
            mpDragBox->EndDocking( maRect, FALSE );
        mnLineMode = 0;
        mnStartLines = 0;
    }
    else
    {
        USHORT nTempItem = mpDragBox->mnConfigItem;
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
                mpDragBox->ImplInvalidate( TRUE );
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
                    USHORT nPos = ImplFindItemPos( pDropBox, aPos );
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
        mbResizeMode = FALSE;
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

    if ( !mbResizeMode )
        ImplDrawConfigFrame( mpDragBox, maStartRect );

    mpDragBox->ShowTracking( maRect );
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplTBDragMgr, SelectHdl, Accelerator*, pAccel )
{
    if ( pAccel->GetCurItemId() == KEY_ESCAPE )
        EndDragging( FALSE );
    else
        EndDragging( TRUE );

    return TRUE;
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::StartCustomizeMode()
{
    mbCustomizeMode = TRUE;

    ToolBox* pBox = mpBoxList->First();
    while ( pBox )
    {
        pBox->ImplStartCustomizeMode();
        pBox = mpBoxList->Next();
    }
}

// -----------------------------------------------------------------------

void ImplTBDragMgr::EndCustomizeMode()
{
    mbCustomizeMode = FALSE;

    ToolBox* pBox = mpBoxList->First();
    while ( pBox )
    {
        pBox->ImplEndCustomizeMode();
        pBox = mpBoxList->Next();
    }
}

// -----------------------------------------------------------------------


static void ImplDrawOutButton( OutputDevice* pOutDev, const Rectangle& rRect,
                               USHORT nStyle )
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
    mbToolBox         = TRUE;
    mpBtnDev          = NULL;
    mpFloatSizeAry    = NULL;
    mpData              = new ImplToolBoxPrivateData;
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
    mnFocusPos        = TOOLBOX_ITEM_NOTFOUND;  // current position during keyboard access
    mnLines           = 1;
    mnCurLine         = 1;
    mnCurLines        = 1;
    mnVisLines        = 1;
    mnFloatLines      = 0;
    mnConfigItem      = 0;
    mnMouseClicks     = 0;
    mnMouseModifier   = 0;
    mbDrag            = FALSE;
    mbSelection       = FALSE;
    mbCommandDrag     = FALSE;
    mbUpper           = FALSE;
    mbLower           = FALSE;
    mbNextTool        = FALSE;
    mbIn              = FALSE;
    mbCalc            = TRUE;
    mbFormat          = FALSE;
    mbFullPaint       = FALSE;
    mbHorz            = TRUE;
    mbScroll          = (nStyle & WB_SCROLL) != 0;
    mbCustomize       = FALSE;
    mbCustomizeMode   = FALSE;
    mbDragging        = FALSE;
    mbHideStatusText  = FALSE;
    mbMenuStrings     = FALSE;
    mbIsShift         = FALSE;
    mbIsKeyEvent = FALSE;
    mbChangingHighlight = FALSE;
    meButtonType      = BUTTON_SYMBOL;
    meAlign           = WINDOWALIGN_TOP;
    meLastStyle       = POINTER_ARROW;
    mnWinStyle        = nStyle;
    mnLastFocusItemId          = 0;
    mnKeyModifier     = 0;
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
        mnStyle |= WB_TABSTOP|WB_NODIALOGCONTROL;
        mnStyle &= ~WB_DIALOGCONTROL;
    }

    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

void ToolBox::ImplInitSettings( BOOL bFont,
                                BOOL bForeground, BOOL bBackground )
{
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
        }
        else
        {
            aColor = rStyleSettings.GetWindowColor();
            SetBackground( aColor );
        }

    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplLoadRes( const ResId& rResId )
{
    DockingWindow::ImplLoadRes( rResId );

    USHORT              nObjMask;

    nObjMask = ReadShortRes();

    if ( nObjMask & RSC_TOOLBOX_BUTTONTYPE )
        SetButtonType( (ButtonType)ReadShortRes() );

    if ( nObjMask & RSC_TOOLBOX_ALIGN )
        SetAlign( (WindowAlign)ReadShortRes() );

    if ( nObjMask & RSC_TOOLBOX_LINECOUNT )
        SetLineCount( ReadShortRes() );

    if ( nObjMask & RSC_TOOLBOX_CUSTOMIZE )
    {
        BOOL bCust = (BOOL)ReadShortRes();
        EnableCustomize( bCust );
    }

    if ( nObjMask & RSC_TOOLBOX_MENUSTRINGS )
    {
        BOOL bCust = (BOOL)ReadShortRes();
        EnableMenuStrings( bCust );
    }

    if ( nObjMask & RSC_TOOLBOX_FLOATLINES )
        SetFloatingLines( ReadShortRes() );

    if ( nObjMask & RSC_TOOLBOX_ITEMIMAGELIST )
    {
        maImageList = ImageList( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }

    if ( nObjMask & RSC_TOOLBOX_ITEMLIST )
    {
        USHORT nEle = ReadShortRes();

        // Item hinzufuegen
        for ( USHORT i = 0; i < nEle; i++ )
        {
            InsertItem( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
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
    rResId.SetRT( RSC_TOOLBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    // Groesse des FloatingWindows berechnen und umschalten, wenn die
    // ToolBox initial im FloatingModus ist
    if ( ImplIsFloatingMode() )
        mbHorz = TRUE;
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
    if ( mpFloatSizeAry )
        delete mpFloatSizeAry;

    // Wenn keine ToolBox-Referenzen mehr auf die Listen bestehen, dann
    // Listen mit wegloeschen
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maCtrlData.mpTBDragMgr )
    {
        // Wenn im TBDrag-Manager, dann wieder rausnehmen
        if ( mbCustomize )
            pSVData->maCtrlData.mpTBDragMgr->Remove( this );

        if ( !pSVData->maCtrlData.mpTBDragMgr->Count() )
        {
            delete pSVData->maCtrlData.mpTBDragMgr;
            pSVData->maCtrlData.mpTBDragMgr = NULL;
        }
    }
}

// -----------------------------------------------------------------------

ImplToolItem* ToolBox::ImplGetItem( USHORT nItemId ) const
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

static void ImplAddButtonBorder( long &rWidth, long& rHeight, USHORT aOutStyle )
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
}

// -----------------------------------------------------------------------

BOOL ToolBox::ImplCalcItem()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // recalc required ?
    if ( !mbCalc )
        return FALSE;

    ImplDisableFlatButtons();

    long            nDefWidth;
    long            nDefHeight;
    long            nMaxWidth = 0;
    long            nMaxHeight = 0;
    long            nHeight;

    // set defaults if image or text is needed but empty
    nDefWidth       = ImplGetDefaultImageSize().Width();
    nDefHeight      = ImplGetDefaultImageSize().Height();

    if ( ! mpData->m_aItems.empty() )
    {
        mnWinHeight = 0;

        std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            BOOL bImage;
            BOOL bText;

            it->mbVisibleText = FALSE;  // indicates if text will definitely be drawn, influences dropdown pos

            if ( it->meType == TOOLBOXITEM_BUTTON )
            {
                // check if image and/or text exists
                if ( !(it->maImage) )
                    bImage = FALSE;
                else
                    bImage = TRUE;
                if ( !it->maText.Len() )
                    bText = FALSE;
                else
                    bText = TRUE;

                if ( bImage || bText )
                {
                    it->mbEmptyBtn = FALSE;

                    if ( meButtonType == BUTTON_SYMBOL )
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
                            it->mbVisibleText = TRUE;
                        }
                    }
                    else if ( meButtonType == BUTTON_TEXT )
                    {
                        // we're drawing text only
                        if ( bText || !bImage )
                        {
                            it->maItemSize = Size( GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET,
                                                   GetTextHeight() );
                            it->mbVisibleText = TRUE;
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
                    it->mbEmptyBtn = TRUE;
                }

                // Gegebenenfalls die Fensterhoehe mit beruecksichtigen
                if ( it->mpWindow )
                {
                    nHeight = it->mpWindow->GetSizePixel().Height();
                    if ( nHeight > mnWinHeight )
                        mnWinHeight = nHeight;
                }

                // add in drop down arrow
                if( it->mnBits & TIB_DROPDOWN )
                    it->maItemSize.Width() += TB_DROPDOWNARROWWIDTH;

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
                ImplAddButtonBorder( it->maItemSize.Width(), it->maItemSize.Height(), mnOutStyle );

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

        ImplAddButtonBorder( nMaxWidth, nMaxHeight, mnOutStyle );
    }

    if( !ImplIsFloatingMode() && GetToolboxButtonSize() != TOOLBOX_BUTTONSIZE_DONTCARE )
    {
        // make sure all vertical toolbars have the same width and horizontal have the same height
        // this depends on the used button sizes
        // as this is used for alignement of multiple toolbars
        // it is only required for docked toolbars

        long nFixedWidth = nDefWidth+TB_DROPDOWNARROWWIDTH;
        long nFixedHeight = nDefHeight;
        ImplAddButtonBorder( nFixedWidth, nFixedHeight, mnOutStyle );

        if( mbHorz )
            nMaxHeight = nFixedHeight;
        else
            nMaxWidth = nFixedWidth;
    }

    mbCalc = FALSE;
    mbFormat = TRUE;

    // do we have to recalc the sizes ?
    if ( (nMaxWidth != mnMaxItemWidth) || (nMaxHeight != mnMaxItemHeight) )
    {
        mnMaxItemWidth  = nMaxWidth;
        mnMaxItemHeight = nMaxHeight;

        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

USHORT ToolBox::ImplCalcBreaks( long nWidth, long* pMaxLineWidth, BOOL bCalcHorz )
{
    ULONG           nLineStart = 0;
    ULONG           nGroupStart = 0;
    long            nLineWidth = 0;
    long            nCurWidth;
    long            nLastGroupLineWidth = 0;
    long            nMaxLineWidth = 0;
    USHORT          nLines = 1;
    BOOL            bWindow;
    BOOL            bBreak = FALSE;
    long            nWidthTotal = nWidth;


    // when docked the menubutton will be in the first line
    // ->initialize first linewidth with button
    if( IsMenuEnabled() && !ImplIsFloatingMode() )
        nLineWidth = mpData->maMenubuttonItem.maItemSize.Width();

    std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
    while ( 1 )
    {
        if( it == mpData->m_aItems.end() )
        {
            if( ImplIsFloatingMode() && IsMenuEnabled() )
                // the menu button will be the last item in floating mode
                it = &mpData->maMenubuttonItem;
            else
                // done
                break;
        }

        it->mbBreak = bBreak;
        bBreak = FALSE;

        if ( it->mbVisible )
        {
            bWindow     = FALSE;
            bBreak      = FALSE;
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
                        bWindow   = TRUE;
                    }
                    else
                    {
                        if ( it->mbEmptyBtn )
                        {
                            nCurWidth = 0;
                        }
                    }
                }

                // check for line break but never break before the menubutton
                if ( (nLineWidth+nCurWidth > nWidthTotal) && mbScroll && (it != &mpData->maMenubuttonItem) )
                    bBreak = TRUE;
            }
            else if ( it->meType == TOOLBOXITEM_SEPARATOR )
                nCurWidth = it->mnSepSize;
            else if ( (it->meType == TOOLBOXITEM_BREAK) &&
                    (ImplIsFloatingMode() || !IsMenuEnabled() ) )    // ignore breaks completely when being docked single-line
                bBreak = TRUE;

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
                    it->mbBreak = TRUE;
                    bBreak = FALSE;
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
                        nLastGroupLineWidth = nLineWidth;
                        nGroupStart = it - mpData->m_aItems.begin();
                        if ( !bWindow )
                            nGroupStart++;
                    }
                }
            }

            nLineWidth += nCurWidth;
        }

        if( it == &mpData->maMenubuttonItem )
            // this was definitely the last item
            break;
        else
            ++it;
    }


    if ( pMaxLineWidth )
    {
        if ( nLineWidth > nMaxLineWidth )
            nMaxLineWidth = nLineWidth;

        // Wegen Separatoren kann MaxLineWidth > Width werden, hat aber
        // auf die Umbrueche keine Auswirkung
        //if ( nMaxLineWidth > nWidth )
        //    nMaxLineWidth = nWidth;

        *pMaxLineWidth = nMaxLineWidth;
    }

    return nLines;
}

// -----------------------------------------------------------------------
namespace
{
    BOOL ImplFollowedByVisibleButton( std::vector< ImplToolItem >::iterator _aSeparator, std::vector< ImplToolItem >::iterator _aEnd )
    {
        std::vector< ImplToolItem >::iterator aLookup = _aSeparator;
        while ( ++aLookup != _aEnd )
        {
            if ( aLookup->meType == TOOLBOXITEM_SEPARATOR )
                return ImplFollowedByVisibleButton( aLookup, _aEnd );

            if ( ( aLookup->meType == TOOLBOXITEM_BUTTON ) && aLookup->mbVisible )
                return TRUE;
        }
        return FALSE;
    }
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
        USHORT nFloatLinesHeight = ImplCalcLines( this, mnDY );
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
        USHORT nLines = ImplCalcBreaks( aSz.Width()-nBorderX, &maxX, mbHorz );

        USHORT manyLines = 1000;
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


void ToolBox::ImplFormat( BOOL bResize )
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
    long            nRight;
    long            nTop;
    long            nBottom;
    long            nMax;   // width of layoutarea in pixels
    long            nX;
    long            nY;
    USHORT          nFormatLine;
    BOOL            bMustFullPaint;
    BOOL            bLastSep;

    std::vector< ImplToolItem >::iterator   it;
    std::vector< ImplToolItem >::iterator   temp_it;

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );

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
        bMustFullPaint = TRUE;
    else
        bMustFullPaint = FALSE;

    // during interactive resizes, check in which direction the window was resized
    //  and try to keep the other dimension unchanged
    // but do this only if the delta is smaller than the whole window
    //  which means it was just showed the first time and the delta is useless
    if ( ImplIsFloatingMode() && (mpData->m_nDeltaSizeY != mnDY || mpData->m_nDeltaSizeX != mnDX) )
    {
        if ( bResize )
        {
            if( mpData->m_nDeltaSizeY != mnDY || mpData->m_nDeltaSizeX != mnDX )
            {
                if( mpData->m_nDeltaSizeY > mpData->m_nDeltaSizeX )
                {
                    // favour window height
                    mnFloatLines = ImplCalcLines( this, mpData->maFloatingSize.Height() );
                }
                else
                {
                    // favour window width
                    int nBorderX = 2*TB_BORDER_OFFSET1 + mnLeftBorder + mnRightBorder + 2*mnBorderX;
                    mnFloatLines = ImplCalcBreaks( mpData->maFloatingSize.Width()-nBorderX, NULL, mbHorz );
                }
            }
        }
        else
        {
            if( mpData->m_nDeltaSizeY > mpData->m_nDeltaSizeX )
                SetOutputSizePixel( ImplGetOptimalFloatingSize( FSMODE_FAVOURHEIGHT ) );
            else
                SetOutputSizePixel( ImplGetOptimalFloatingSize( FSMODE_FAVOURWIDTH ) );
        }
    }

    // Horizontal
    if ( mbHorz )
    {
        // nLineSize: height of a single line, will fit highest item
        nLineSize = mnMaxItemHeight;

        if ( mnWinHeight > mnMaxItemHeight )
            nLineSize = mnWinHeight;

        if ( mbScroll )
        {
            nMax        = mnDX;

            // limit layout to current window size (mnDX,mnDY)
            // or number of lines while resizing
            if( ImplIsFloatingMode() && bResize )
            {
                mnVisLines = mnFloatLines;
                if( mpData->m_nDeltaSizeY > mpData->m_nDeltaSizeX )
                    // set window width to optimal value for this height
                    // this will trigger proper repaint during resizing of the height
                    nMax = ImplCalcFloatSize( this, mnFloatLines ).Width();
            }
            else
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

            // For items not visible, release resources only needed during
            // painting the items (on Win98, for example, these are system-wide
            // resources that are easily exhausted, so be nice):

            /* !!!
                it->maImage.ClearCaches();
                it->maHighImage.ClearCaches();
            */

            ++it;
        }

        maLowerRect = aEmptyRect;
        maUpperRect = aEmptyRect;
        maNextToolRect = aEmptyRect;
    }
    else
    {
        // init start values
        nX          = nLeft;    // top-left offset
        nY          = nTop;
        nFormatLine = 1;
        bLastSep    = TRUE;

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
        if ( !mpData->m_aItems.empty() )
        {
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
                    if( IsMenuEnabled() )
                    {
                        if( !ImplIsFloatingMode() )
                        {
                            // when docked paint menu button up to the borders
                            mpData->maMenubuttonItem.maRect.Right() = mnDX - 2;
                            mpData->maMenubuttonItem.maRect.Top() = 0;
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY-1;
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.Right() = mnDX - mnRightBorder-mnBorderX-TB_BORDER_OFFSET1-1;
                            mpData->maMenubuttonItem.maRect.Top() = nTop;
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY-mnBottomBorder-mnBorderY-TB_BORDER_OFFSET2-1;
                        }
                        mpData->maMenubuttonItem.maRect.Left() = mpData->maMenubuttonItem.maRect.Right() - TB_MENUBUTTON_SIZE;
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
                    if( IsMenuEnabled() )
                    {
                        if( !ImplIsFloatingMode() )
                        {
                            // when docked paint menu button up to the borders
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY - 2;
                            mpData->maMenubuttonItem.maRect.Left() = 0;
                            mpData->maMenubuttonItem.maRect.Right() = mnDX-1;
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY - mnBottomBorder-mnBorderY-TB_BORDER_OFFSET1-1;
                            mpData->maMenubuttonItem.maRect.Left() = nLeft;
                            mpData->maMenubuttonItem.maRect.Right() = mnDX-mnRightBorder-mnBorderX-TB_BORDER_OFFSET2-1;
                        }
                        mpData->maMenubuttonItem.maRect.Top() = mpData->maMenubuttonItem.maRect.Bottom() - TB_MENUBUTTON_SIZE;
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
                // hide double separators
                if ( it->meType == TOOLBOXITEM_SEPARATOR )
                {
                    it->mbVisible = FALSE;
                    if ( !bLastSep )
                    {
                        // check if any visible items have to appear behind it
                        temp_it = it+1;
                        while ( temp_it != mpData->m_aItems.end() )
                        {
                            if ( (temp_it->meType == TOOLBOXITEM_SEPARATOR) ||
                                 ((temp_it->meType == TOOLBOXITEM_BUTTON) &&
                                  temp_it->mbVisible) )
                            {
                                it->mbVisible = TRUE;
                                break;
                            }
                            ++temp_it;
                        }
                    }
                    bLastSep = TRUE;
                }
                else if ( it->mbVisible )
                    bLastSep = FALSE;

                it->mbShowWindow = FALSE;

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


        if( IsMenuEnabled() && ImplIsFloatingMode() )
        {
            // custom menu will be the last button in floating mode
            it = &mpData->maMenubuttonItem;

            if ( mbHorz )
            {
                it->maRect.Left()     = nX+TB_MENUBUTTON_OFFSET;
                it->maRect.Top()      = nY;
                it->maRect.Right()    = it->maRect.Left() + TB_MENUBUTTON_SIZE;
                it->maRect.Bottom()   = nY+nLineSize-1;
                nX += it->maItemSize.Width();
            }
            else
            {
                it->maRect.Left()     = nX;
                it->maRect.Top()      = nY+TB_MENUBUTTON_OFFSET;
                it->maRect.Right()    = nX+nLineSize-1;
                it->maRect.Bottom()   = it->maRect.Top() + TB_MENUBUTTON_SIZE;
                nY += it->maItemSize.Height();
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
            if (!it->maRect.IsOver(aVisibleRect))
            {
                // For items not visible, release resources only needed during
                // painting the items (on Win98, for example, these are system-
                // wide resources that are easily exhausted, so be nice):

                /* !!!
                it->maImage.ClearCaches();
                it->maHighImage.ClearCaches();
                */
            }
            ++it;
        }
    }

    // indicate formatting is done
    mbFormat = FALSE;

}

// -----------------------------------------------------------------------

IMPL_LINK( ToolBox, ImplDropdownLongClickHdl, ToolBox*, pThis )
{
    if( mnCurPos != TOOLBOX_ITEM_NOTFOUND &&
        (mpData->m_aItems[ mnCurPos ].mnBits & TIB_DROPDOWN)
        )
    {
        GetDropdownClickHdl().Call( this );

        Deactivate();
        ImplDrawItem( mnCurPos, FALSE );

        mnCurPos         = TOOLBOX_ITEM_NOTFOUND;
        mnCurItemId      = 0;
        mnDownItemId     = 0;
        mnMouseClicks    = 0;
        mnMouseModifier  = 0;
        mnHighItemId     = 0;
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ToolBox, ImplUpdateHdl, void*, EMPTYARG )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    PointerState aState = GetPointerState();
    if( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) )
    {
        mbFormat = TRUE;
        ImplFormat( TRUE );
        maTimer.Start();
        mpData->m_nUpdateAutoSizeTries = 0;
    }
    /*  #i30930# auto sizing will on some desktops result in a too small
        window size in the window manager's opinion. This will result
        in being resized back to a larger size again by the WM ...
        which will in turn trigger autosize -> endless resizing loop.
        The problem is that it is indistinguishable whether the WM sized
        our window or the user did. So the only thing we can do is limit
        the loop.
     */
    else if( mpData->m_nUpdateAutoSizeTries < 3 )
    {
        mbFormat = TRUE;
        if ( ImplIsFloatingMode() && mpData->m_nDeltaSizeY == mnDY && mpData->m_nDeltaSizeX == mnDX )
        {
            // an initial and perhaps wrong floating size was set per API
            // adjust it to an optimal size to make sure no items will be clipped
            SetSizePixel( ImplGetOptimalFloatingSize( FSMODE_AUTO ) );
        }
        ImplFormat();
        mpData->m_nUpdateAutoSizeTries++;
    }
    else
        mpData->m_nUpdateAutoSizeTries = 0;

    return 0;
}

// -----------------------------------------------------------------------

static void ImplDrawMoreIndicator( ToolBox *pBox, const Rectangle& rRect, BOOL bSetColor, BOOL bRotate )
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

static void ImplDrawDropdownArrow( ToolBox *pBox, const Rectangle& rDropDownRect, BOOL bSetColor, BOOL bRotate )
{
    BOOL bLineColor = pBox->IsLineColor();
    BOOL bFillColor = pBox->IsFillColor();
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
        long x = rDropDownRect.Left() + (rDropDownRect.getWidth() - width)/2 + 1;
        long y = rDropDownRect.Top() + (rDropDownRect.getHeight() - height)/2 + 1;
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
        long x = rDropDownRect.Left() + (rDropDownRect.getWidth() - width)/2 + 1;
        long y = rDropDownRect.Top() + (rDropDownRect.getHeight() - height)/2 + 1;
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

static void ImplDrawToolArrow( ToolBox* pBox, long nX, long nY, BOOL bBlack, BOOL bColTransform,
                               BOOL bLeft = FALSE, BOOL bTop = FALSE,
                               long nSize = 6 )
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

static void SetToolArrowClipregion( ToolBox* pBox, long nX, long nY,
                               BOOL bLeft = FALSE, BOOL bTop = FALSE,
                               long nSize = 6 )
{
    WindowAlign     eAlign = pBox->meAlign;
    long            nHalfSize;
    if ( bLeft )
        eAlign = WINDOWALIGN_RIGHT;
    else if ( bTop )
        eAlign = WINDOWALIGN_BOTTOM;

    nHalfSize = nSize/2;

    Point p[6];

    switch ( eAlign )
    {
        case WINDOWALIGN_LEFT:
            p[0].X() = nX-1; p[0].Y() = nY-1;
            p[1].X() = nX-1; p[1].Y() = nY+nSize+1;
            p[2].X() = nX+1; p[2].Y() = nY+nSize+1;
            p[3].X() = nX+nHalfSize+1; p[3].Y() = nY+nHalfSize+1;
            p[4].X() = nX+nHalfSize+1; p[4].Y() = nY+nHalfSize-1;
            p[5].X() = nX+1; p[5].Y() = nY-1;
            break;
        case WINDOWALIGN_TOP:
            p[0].X() = nX-1; p[0].Y() = nY-1;
            p[1].X() = nX-1; p[1].Y() = nY+1;
            p[2].X() = nX+nHalfSize-1; p[2].Y() = nY+nHalfSize+1;
            p[3].X() = nX+nHalfSize+1; p[3].Y() = nY+nHalfSize+1;
            p[4].X() = nX+nSize+1; p[4].Y() = nY+1;
            p[5].X() = nX+nSize+1; p[5].Y() = nY-1;
            break;
        case WINDOWALIGN_RIGHT:
            p[0].X() = nX+nHalfSize-1; p[0].Y() = nY-1;
            p[1].X() = nX-1; p[1].Y() = nY+nHalfSize-1;
            p[2].X() = nX-1; p[2].Y() = nY+nHalfSize+1;
            p[3].X() = nX+nHalfSize-1; p[3].Y() = nY+nSize+1;
            p[4].X() = nX+nHalfSize+1; p[4].Y() = nY+nSize+1;
            p[5].X() = nX+nHalfSize+1; p[5].Y() = nY-1;
            break;
        case WINDOWALIGN_BOTTOM:
            p[0].X() = nX-1; p[0].Y() = nY+nHalfSize-1;
            p[1].X() = nX-1; p[1].Y() = nY+nHalfSize+1;
            p[2].X() = nX+nSize+1; p[2].Y() = nY+nHalfSize+1;
            p[3].X() = nX+nSize+1; p[3].Y() = nY+nHalfSize-1;
            p[4].X() = nX+nHalfSize+1; p[4].Y() = nY-1;
            p[5].X() = nX+nHalfSize-1; p[5].Y() = nY-1;
            break;
    }
    Polygon aPoly(6,p);
    Region aRgn( aPoly );
    pBox->SetClipRegion( aRgn );
}

// -----------------------------------------------------------------------

static void ImplDrawMenubutton( ToolBox *pThis, BOOL bHighlight )
{
    if( !pThis->mpData->maMenubuttonItem.maRect.IsEmpty() )
    {
        BOOL bFillColor = pThis->IsFillColor();
        BOOL bLineColor = pThis->IsLineColor();
        Color aOldFillCol = pThis->GetFillColor();
        Color aOldLineCol = pThis->GetLineColor();

        ImplErase( pThis, pThis->mpData->maMenubuttonItem.maRect, bHighlight );

        if( bHighlight )
            pThis->DrawSelectionBackground( pThis->mpData->maMenubuttonItem.maRect, 2, FALSE, FALSE, FALSE );
        else
        {
            // improve visibility by using a dark gradient
            Gradient g;
            g.SetAngle( pThis->mbHorz ? 0 : 900 );
            g.SetStyle( GRADIENT_LINEAR );

            g.SetStartColor( pThis->GetSettings().GetStyleSettings().GetFaceColor() );
            g.SetEndColor( pThis->GetSettings().GetStyleSettings().GetShadowColor() );

            pThis->DrawGradient( pThis->mpData->maMenubuttonItem.maRect, g );
        }

        Rectangle aRect( pThis->mpData->maMenubuttonItem.maRect );
        if( pThis->mbHorz )
            aRect.Top() = aRect.Bottom() - aRect.getHeight()/3;
        else
            aRect.Left() = aRect.Right() - aRect.getWidth()/3;

        BOOL bSetColor = TRUE;
        if( !(pThis->mpData->maMenuType & TOOLBOX_MENUTYPE_CUSTOMIZE) )
        {
            // customization disabled
            bSetColor = FALSE;
            pThis->SetFillColor( pThis->GetSettings().GetStyleSettings().GetShadowColor() );
        }
        ImplDrawDropdownArrow( pThis, aRect, bSetColor, !pThis->mbHorz );

        if( pThis->ImplHasClippedItems() )
        {
            aRect = pThis->mpData->maMenubuttonItem.maRect;
            if( pThis->mbHorz )
                aRect.Bottom() = aRect.Top() + aRect.getHeight()/3;
            else
                aRect.Right() = aRect.Left() + aRect.getWidth()/3;

            ImplDrawMoreIndicator( pThis, aRect, TRUE, !pThis->mbHorz );
        }

        // the menubutton may have overwritten the 'round' corners, so restore them
        ImplDrawBorderEffects( pThis, ImplGetDockingManager()->GetDockingWindowWrapper( pThis ) );

        // ugly: store highlight state
        pThis->mpData->maMenubuttonItem.mpUserData = bHighlight ? (void*)1 : NULL;

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

void ToolBox::ImplDrawSpin( BOOL bUpperIn, BOOL bLowerIn )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    BOOL    bTmpUpper;
    BOOL    bTmpLower;

    if ( maUpperRect.IsEmpty() || maLowerRect.IsEmpty() )
        return;

    if ( mnCurLine > 1 )
        bTmpUpper = TRUE;
    else
        bTmpUpper = FALSE;

    if ( mnCurLine+mnVisLines-1 < mnCurLines )
        bTmpLower = TRUE;
    else
        bTmpLower = FALSE;

    if ( !IsEnabled() )
    {
        bTmpUpper = FALSE;
        bTmpLower = FALSE;
    }

    ImplDrawSpinButton( this, maUpperRect, maLowerRect,
                        bUpperIn, bLowerIn, bTmpUpper, bTmpLower, !mbHorz );
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawNext( BOOL bIn )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( maNextToolRect.IsEmpty() )
        return;

    DecorationView aDecoView( this );

    // Button malen
    long    nX      = SMALLBUTTON_OFF_NORMAL_X;
    long    nY      = SMALLBUTTON_OFF_NORMAL_Y;
    USHORT  nStyle  = 0;
    if ( bIn == 1 )
    {
        nStyle |= BUTTON_DRAW_PRESSED;
        nX = SMALLBUTTON_OFF_PRESSED_X;
        nY = SMALLBUTTON_OFF_PRESSED_Y;
    }
    aDecoView.DrawButton( maNextToolRect, nStyle );

    // Inhalt ausgeben
    BOOL    bLeft   = FALSE;
    BOOL    bTop    = FALSE;
    if ( mbHorz )
    {
        bLeft = TRUE;
        nX += (maNextToolRect.GetWidth()-6)/2-4;
        nY += (maNextToolRect.GetHeight()-6)/2-6;
    }
    else
    {
        bTop = TRUE;
        nY += (maNextToolRect.GetHeight()-6)/2-4;
        nX += (maNextToolRect.GetWidth()-6)/2-6;
    }

    nX += maNextToolRect.Left();
    nY += maNextToolRect.Top();
    SetLineColor();
    SetFillColor( COL_LIGHTBLUE );
    ImplDrawToolArrow( this, nX, nY, TRUE, FALSE, bLeft, bTop, 10 );
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawItem( USHORT nPos, BOOL bHighlight, BOOL bPaint, BOOL bLayout )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( nPos >= mpData->m_aItems.size() )
        return;

    ImplDisableFlatButtons();

    SetFillColor();

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    MetricVector* pVector = bLayout ? &mpData->m_pLayoutData->m_aUnicodeBoundRects : NULL;
    String* pDisplayText = bLayout ? &mpData->m_pLayoutData->m_aDisplayText : NULL;

    // Falls Rechteck ausserhalb des sichbaren Bereichs liegt
    if ( pItem->maRect.IsEmpty() )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    BOOL bHighContrastWhite = FALSE;
    // check the face color as highcontrast indicator
    // because the toolbox itself might have a gradient
    if( rStyleSettings.GetFaceColor().IsBright() )
        bHighContrastWhite = TRUE;

    // draw separators in flat style only
    if ( !bLayout &&
         (mnOutStyle & TOOLBOX_STYLE_FLAT) &&
         (pItem->meType == TOOLBOXITEM_SEPARATOR) &&
         nPos > 0
         )
    {
        // no separator before or after windows or at breaks
        ImplToolItem* pTempItem = &mpData->m_aItems[nPos-1];
        if ( pTempItem && !pTempItem->mbShowWindow && nPos < mpData->m_aItems.size()-1 )
        {
            pTempItem = &mpData->m_aItems[nPos+1];
            if ( !pTempItem->mbShowWindow && !pTempItem->mbBreak )
            {
                long nCenterPos;
                SetLineColor( rStyleSettings.GetShadowColor() );
                if ( IsHorizontal() )
                {
                    nCenterPos = pItem->maRect.Center().X()-1;
                    DrawLine( Point( nCenterPos, pItem->maRect.Top() ),
                              Point( nCenterPos, pItem->maRect.Bottom() ) );
                    nCenterPos++;
                    SetLineColor( rStyleSettings.GetLightColor() );
                    DrawLine( Point( nCenterPos, pItem->maRect.Top() ),
                              Point( nCenterPos, pItem->maRect.Bottom() ) );
                }
                else
                {
                    nCenterPos = pItem->maRect.Center().Y()-1;
                    DrawLine( Point( pItem->maRect.Left(), nCenterPos ),
                              Point( pItem->maRect.Right(), nCenterPos ) );
                    nCenterPos++;
                    SetLineColor( rStyleSettings.GetLightColor() );
                    DrawLine( Point( pItem->maRect.Left(), nCenterPos ),
                              Point( pItem->maRect.Right(), nCenterPos ) );
                }
            }
        }
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
        BOOL bClip;
        if ( (aSize.Width() > pItem->maRect.GetWidth()-2) ||
             (aSize.Height() > pItem->maRect.GetHeight()-2) )
        {
            bClip = TRUE;
            Rectangle aTempRect( pItem->maRect.Left()+1, pItem->maRect.Top()+1,
                                 pItem->maRect.Right()-1, pItem->maRect.Bottom()-1 );
            Region aTempRegion( aTempRect );
            SetClipRegion( aTempRegion );
        }
        else
            bClip = FALSE;
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

    // draw button
    Size    aBtnSize    = pItem->maRect.GetSize();
    long    nOffX       = SMALLBUTTON_OFF_NORMAL_X;
    long    nOffY       = SMALLBUTTON_OFF_NORMAL_Y;
    long    nImageOffX=0;
    long    nImageOffY=0;
    long    nTextOffX=0;
    long    nTextOffY=0;
    USHORT  nStyle      = 0;

    if ( pItem->meState == STATE_CHECK )
    {
        nStyle |= BUTTON_DRAW_CHECKED;
    }
    else if ( pItem->meState == STATE_DONTKNOW )
    {
        nStyle |= BUTTON_DRAW_DONTKNOW;
    }
    if ( bHighlight == 1 )
    {
        nStyle |= BUTTON_DRAW_PRESSED;
    }

    if ( mnOutStyle & TOOLBOX_STYLE_OUTBUTTON )
    {
        nOffX = OUTBUTTON_OFF_NORMAL_X;
        nOffY = OUTBUTTON_OFF_NORMAL_Y;
        if ( bHighlight )
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
                ImplErase( this, pItem->maRect, bHighlight );
            }
        }
        else
        {
            if ( mnOutStyle & TOOLBOX_STYLE_OUTBUTTON )
                ImplDrawOutButton( this, pItem->maRect, nStyle );
            else
            {
                DecorationView aDecoView( this );
                aDecoView.DrawButton( pItem->maRect, nStyle );
            }
        }
    }

    nOffX += pItem->maRect.Left();
    nOffY += pItem->maRect.Top();

    // determine what has to be drawn on the button: image, text or both
    BOOL bImage;
    BOOL bText;
    pItem->DetermineButtonDrawStyle( meButtonType, bImage, bText );

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
        if ( bHighlight && (!(pItem->maHighImage)) == FALSE )
            pImage = &(pItem->maHighImage);
        else
            pImage = &(pItem->maImage);

        aImageSize = pImage->GetSizePixel();

        // determine drawing flags
        USHORT nImageStyle = 0;

        if ( !pItem->mbEnabled || !IsEnabled() )
            nImageStyle |= IMAGE_DRAW_DISABLE;

        if ( pItem->meState == STATE_DONTKNOW )
            nImageStyle |= IMAGE_DRAW_DISABLE;

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
        if ( bHighlight || (pItem->meState == STATE_CHECK) )
        {
            DrawSelectionBackground( pItem->maRect, bHighlight, pItem->meState == STATE_CHECK, TRUE, pItem->mbShowWindow ? TRUE : FALSE );

            // draw shadows
            if( bHighlight == 2 && pItem->meState != STATE_CHECK )
            {
                nImageOffX++;
                nImageOffY++;

                /* !!!
                if( pImage->HasMaskBitmap() )
                {
                    Color aMaskCol = GetpApp()->GetSettings().GetStyleSettings().GetHighlightColor();
                    USHORT h,s,b;
                    ImplRGBtoHSB( aMaskCol, h, s, b );
                    if( s > 20 ) s=20;
                    if( b > 5 ) b-=5;
                    aMaskCol = ImplHSBtoRGB( h, s, b );
                    DrawMask( Point( nImageOffX, nImageOffY ), pImage->GetMaskBitmap(), aMaskCol );
                }
                */

                nImageOffX-=2;
                nImageOffY-=2;
            }

            if( bHighlight )
            {
                if( bHighContrastWhite )
                    nImageStyle |= IMAGE_DRAW_COLORTRANSFORM;
            }
            DrawImage( Point( nImageOffX, nImageOffY ), *pImage, nImageStyle );
        }
        else
            DrawImage( Point( nImageOffX, nImageOffY ), *pImage, nImageStyle );
    }

    // draw the text
    BOOL bRotate = FALSE;
    if ( bText )
    {
        nTextOffX = nOffX;
        nTextOffY = nOffY;

        // rotate text when vertically docked
        Font aOldFont = GetFont();
        if( pItem->mbVisibleText && !ImplIsFloatingMode() &&
            ((meAlign == WINDOWALIGN_LEFT) || (meAlign == WINDOWALIGN_RIGHT)) )
        {
            bRotate = TRUE;

            Font aRotateFont = aOldFont;
            /*
            if ( meAlign == WINDOWALIGN_LEFT )
            {
                aRotateFont.SetOrientation( 900 );
                nTextOffX += (nBtnWidth-aTxtSize.Height())/2;
                nTextOffY += aTxtSize.Width();
                nTextOffY += (nBtnHeight-aTxtSize.Width())/2;
            }
            else*/
            {
                aRotateFont.SetOrientation( 2700 );

                // center horizontally
                nTextOffX += aTxtSize.Height();
                nTextOffX += (nBtnWidth-aTxtSize.Height())/2;

                // add in image offset
                if( bImage )
                    nTextOffY = nImageOffY + aImageSize.Height() + TB_IMAGETEXTOFFSET;
            }

            SetFont( aRotateFont );
        }
        else
        {
            // center vertically
            nTextOffY += (nBtnHeight-aTxtSize.Height())/2;

            // add in image offset
            if( bImage )
                nTextOffX = nImageOffX + aImageSize.Width() + TB_IMAGETEXTOFFSET;
            //nTextOffX += TB_TEXTOFFSET/2;
        }

        // draw selection only if not already drawn during image output (see above)
        if ( !bLayout && !bImage && (bHighlight || (pItem->meState == STATE_CHECK) ) )
            DrawSelectionBackground( pItem->maRect, bHighlight, pItem->meState == STATE_CHECK, TRUE, pItem->mbShowWindow ? TRUE : FALSE );

        USHORT nTextStyle = 0;
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
        BOOL bSetColor = TRUE;

        Rectangle aDropDownRect( pItem->GetDropDownRect( mbHorz ) );
        ImplErase( this, aDropDownRect, bHighlight );

        if ( !pItem->mbEnabled || !IsEnabled() )
        {
            bSetColor = FALSE;
            SetFillColor( rStyleSettings.GetShadowColor() );
        }
        if( bHighlight || (pItem->meState == STATE_CHECK) )
        {
            DrawSelectionBackground( aDropDownRect, bHighlight, pItem->meState == STATE_CHECK, TRUE, FALSE );
        }

        ImplDrawDropdownArrow( this, aDropDownRect, bSetColor, bRotate );
    }

    // Gegebenenfalls noch Config-Frame zeichnen
    if ( pMgr )
        pMgr->UpdateDragRect();
}

// -----------------------------------------------------------------------

void ToolBox::ImplStartCustomizeMode()
{
    mbCustomizeMode = TRUE;

    mpData->ImplClearLayoutData();

    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        if ( it->mbShowWindow )
        {
            it->mpWindow->Hide();

            if ( !(it->maRect.IsEmpty()) )
                Invalidate( it->maRect );
        }

        ++it;
    }
}

void ToolBox::SetCustomizeMode( BOOL bSet )
{
    if ( bSet )
        ImplStartCustomizeMode();
    else
        ImplEndCustomizeMode();
}

// -----------------------------------------------------------------------

void ToolBox::ImplEndCustomizeMode()
{
    mbCustomizeMode = FALSE;

    mpData->ImplClearLayoutData();

    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        if ( it->mbShowWindow )
        {
            if ( !(it->maRect.IsEmpty()) )
                Invalidate( it->maRect );

            it->mpWindow->Show();
        }

        ++it;
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplFloatControl( BOOL bStart, FloatingWindow* pFloatWindow )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( bStart )
    {
        mpFloatWin = pFloatWindow;

        // Wenn Button nicht gedrueckt dargestellt wird, dann den
        // Button neu malen
        if ( mnCurItemId != mnDownItemId )
            ImplDrawItem( mnCurPos, TRUE );
        mbDrag = FALSE;
        EndTracking();
        ReleaseMouse();
    }
    else
    {
        mpFloatWin = NULL;

        if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            ImplDrawItem( mnCurPos );
        Deactivate();
        mnCurPos = TOOLBOX_ITEM_NOTFOUND;
        mnCurItemId = 0;
        mnDownItemId = 0;
    }
}

// -----------------------------------------------------------------------

void ToolBox::ShowLine( BOOL bNext )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    mbFormat = TRUE;
    if ( bNext )
        mnCurLine++;
    else
        mnCurLine--;
    ImplFormat();
}

// -----------------------------------------------------------------------

BOOL ToolBox::ImplHandleMouseMove( const MouseEvent& rMEvt, BOOL bRepeat )
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
                ImplDrawItem( mnCurPos, TRUE );
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

        return TRUE;
    }

    if ( mbUpper )
    {
        BOOL bNewIn = maUpperRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            ImplDrawSpin( mbIn, FALSE );
        }
        return TRUE;
    }

    if ( mbLower )
    {
        BOOL bNewIn = maLowerRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            ImplDrawSpin( FALSE, mbIn );
        }
        return TRUE;
    }

    if ( mbNextTool )
    {
        BOOL bNewIn = maNextToolRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            ImplDrawNext( mbIn );
        }
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ToolBox::ImplHandleMouseButtonUp( const MouseEvent& rMEvt, BOOL bCancel )
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
            mbDrag = FALSE;
        else
        {
            mbSelection = FALSE;
            if ( mnCurPos == TOOLBOX_ITEM_NOTFOUND )
                return TRUE;
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
                            return TRUE;
                        ImplRemoveDel( &aDelData );
                    }
                }

                {
                    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
                }

                // Items nicht geloescht, im Select-Handler
                if ( mnCurItemId )
                {
                    BOOL bHighlight;
                    if ( (mnCurItemId == mnHighItemId) && (mnOutStyle & TOOLBOX_STYLE_FLAT) )
                        bHighlight = 2;
                    else
                        bHighlight = FALSE;
                    // Get current pos for the case that items are inserted/removed
                    // in the toolBox
                    mnCurPos = GetItemPos( mnCurItemId );
                    if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
                        ImplDrawItem( mnCurPos, bHighlight );
                }
            }
        }

        mnCurPos         = TOOLBOX_ITEM_NOTFOUND;
        mnCurItemId      = 0;
        mnDownItemId     = 0;
        mnMouseClicks    = 0;
        mnMouseModifier  = 0;
        return TRUE;
    }
    else if ( mbUpper || mbLower )
    {
        if ( mbIn )
            ShowLine( !mbUpper );
        mbUpper = FALSE;
        mbLower = FALSE;
        mbIn    = FALSE;
        ImplDrawSpin( FALSE, FALSE );
        return TRUE;
    }
    else if ( mbNextTool )
    {
        mbNextTool  = FALSE;
        mbIn        = FALSE;
        ImplDrawNext( FALSE );
        NextToolBox();
        return TRUE;
    }

    return FALSE;
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
    BOOL bDrawHotSpot = TRUE;
    Window *pWin = Application::GetFocusWindow();
    if( pWin && pWin->mbToolBox && pWin != this )
        bDrawHotSpot = FALSE;
    else if( !HasFocus() && HasChildPathFocus() )   // focus is in our childwindow: no highlight
        bDrawHotSpot = FALSE;
    /*
    else
        if( pWin && !pWin->mbToolBox )
            while( pWin )
            {
                pWin = pWin->GetParent();
                if( pWin && pWin->mbToolBox )
                {
                    bDrawHotSpot = FALSE;
                    break;
                }
            }
            */

    if ( mbSelection && bDrawHotSpot )
    {
        USHORT  i = 0;
        USHORT  nNewPos = TOOLBOX_ITEM_NOTFOUND;

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
                ImplDrawItem( mnCurPos, 2 /*TRUE*/ ); // always use shadow effect (2)
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
            USHORT nLinePtr = ImplTestLineSize( this, rMEvt.GetPosPixel() );
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
        BOOL bClearHigh = TRUE;
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
                            bClearHigh = FALSE;
                            if ( mnHighItemId != it->mnId )
                            {
                                USHORT nTempPos = it - mpData->m_aItems.begin();
                                if ( mnHighItemId )
                                {
                                    ImplHideFocus();
                                    USHORT nPos = GetItemPos( mnHighItemId );
                                    ImplDrawItem( nPos );
                                    ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, reinterpret_cast< void* >( nPos ) );
                                }
                                if ( mpData->maMenubuttonItem.mpUserData )
                                {
                                    // remove highlight from menubutton
                                    ImplDrawMenubutton( this, FALSE );
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
        BOOL bMenuButtonHit = mpData->maMenubuttonItem.maRect.IsInside( aMousePos );
        if ( bClearHigh && !HasChildPathFocus() || bMenuButtonHit )
        {
            if ( !bMenuButtonHit && mpData->maMenubuttonItem.mpUserData )
            {
                // remove highlight from menubutton
                ImplDrawMenubutton( this, FALSE );
            }

            if( mnHighItemId )
            {
                USHORT nClearPos = GetItemPos( mnHighItemId );
                if ( nClearPos != TOOLBOX_ITEM_NOTFOUND )
                {
                    ImplDrawItem( nClearPos, (nClearPos == mnCurPos) ? TRUE : FALSE );
                    if( nClearPos != mnCurPos )
                        ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, reinterpret_cast< void* >( nClearPos ) );
                }
                ImplHideFocus();
                mnHighItemId = 0;
            }

            if( bMenuButtonHit )
            {
                ImplDrawMenubutton( this, TRUE );
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
        USHORT i = 0;
        USHORT nNewPos = TOOLBOX_ITEM_NOTFOUND;

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

                    BOOL bResizeItem;
                    if ( mbCustomizeMode && it->mbShowWindow &&
                         (it->maRect.Right()-TB_RESIZE_OFFSET <= aMousePos.X()) )
                        bResizeItem = TRUE;
                    else
                        bResizeItem = FALSE;
                    pMgr->StartDragging( this, aMousePos, aItemRect, 0, bResizeItem );
                    return;
                }
            }

            if ( !it->mbEnabled )
            {
                Sound::Beep( SOUND_DISABLE, this );
                Deactivate();
                return;
            }


            // Aktuelle Daten setzen
            USHORT nTrackFlags = 0;
            mnCurPos         = i;
            mnCurItemId      = it->mnId;
            mnDownItemId     = mnCurItemId;
            mnMouseClicks    = rMEvt.GetClicks();
            mnMouseModifier  = rMEvt.GetModifier();
            if ( it->mnBits & TIB_REPEAT )
                nTrackFlags |= STARTTRACK_BUTTONREPEAT;


            if ( mbSelection )
            {
                ImplDrawItem( mnCurPos, TRUE );
                Highlight();
            }
            else
            {
                // was dropdown arrow pressed
                if( (it->mnBits & TIB_DROPDOWN) )
                {
                    if(it->GetDropDownRect( mbHorz ).IsInside( aMousePos ) )
                    {
                        // the drop down arrow should not trigger the item action
                        GetDropdownClickHdl().Call( this );

                        Deactivate();
                        ImplDrawItem( mnCurPos, FALSE );

                        mnCurPos         = TOOLBOX_ITEM_NOTFOUND;
                        mnCurItemId      = 0;
                        mnDownItemId     = 0;
                        mnMouseClicks    = 0;
                        mnMouseModifier  = 0;
                        mnHighItemId     = 0;
                        return;
                    }
                    else // activate long click timer
                        mpData->maDropdownTimer.Start();
                }

                // Hier schon bDrag setzen, da in EndSelection ausgewertet wird
                mbDrag = TRUE;

                // Bei Doppelklick nur den Handler rufen, aber bevor der
                // Button gehiltet wird, da evt. in diesem Handler der
                // Drag-Vorgang abgebrochen wird
                if ( rMEvt.GetClicks() == 2 )
                    DoubleClick();


                if ( mbDrag )
                {
                    ImplDrawItem( mnCurPos, TRUE );
                    Highlight();
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
        if( mpData->maMenubuttonItem.maRect.IsInside( aMousePos ) )
        {
            // handle custom menu asynchronously
            // to avoid problems if the toolbox is closed during menu execute
            ImplUpdateCustomMenu();
            Application::PostUserEvent( mpData->mnEventId, LINK( this, ToolBox, ImplCallExecuteCustomMenu ) );
            return;
        }


        // Gegebenenfalls noch Scroll- und Next-Buttons ueberpruefen
        if ( maUpperRect.IsInside( aMousePos ) )
        {
            if ( mnCurLine > 1 )
            {
                StartTracking();
                mbUpper = TRUE;
                mbIn    = TRUE;
                ImplDrawSpin( TRUE, FALSE );
            }
            return;
        }
        if ( maLowerRect.IsInside( aMousePos ) )
        {
            if ( mnCurLine+mnVisLines-1 < mnCurLines )
            {
                StartTracking();
                mbLower = TRUE;
                mbIn    = TRUE;
                ImplDrawSpin( FALSE, TRUE );
            }
            return;
        }
        if ( maNextToolRect.IsInside( aMousePos ) )
        {
            StartTracking();
            mbNextTool   = TRUE;
            mbIn         = TRUE;
            ImplDrawNext( TRUE );
            return;
        }

        // Linesizing testen
        if ( (mnWinStyle & TB_WBLINESIZING) == TB_WBLINESIZING )
        {
            USHORT nLineMode = ImplTestLineSize( this, aMousePos );
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
                                     nLineMode, FALSE );
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
    mbCommandDrag = FALSE;

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
    // check if locking has changed and recalc accordingly
    if( !ImplIsFloatingMode() )
    {
        ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
        if( pWrapper && mpData->mbIsLocked != pWrapper->IsLocked() )
        {
            mpData->mbIsLocked = pWrapper->IsLocked();
            mbCalc = TRUE;
            SetSizePixel( CalcWindowSizePixel(1) );
            return;
        }
    }

    if ( rPaintRect == Rectangle( 0, 0, mnDX-1, mnDY-1 ) )
        mbFullPaint = TRUE;
    ImplFormat();
    mbFullPaint = FALSE;

    ImplDrawGradient( this, rPaintRect );

    if ( (mnWinStyle & WB_BORDER) && !ImplIsFloatingMode() )
        ImplDrawBorder( this );

    if( !ImplIsFloatingMode() )
        ImplDrawGrip( this );

    ImplDrawMenubutton( this, FALSE );

    // SpinButtons zeichnen
    if ( mnWinStyle & WB_SCROLL )
    {
        if ( mnCurLines > mnLines )
            ImplDrawSpin( FALSE, FALSE );
    }

    // NextButton zeichnen
    ImplDrawNext( FALSE );

    // Buttons zeichnen
    USHORT nHighPos;
    if ( mnHighItemId )
        nHighPos = GetItemPos( mnHighItemId );
    else
        nHighPos = TOOLBOX_ITEM_NOTFOUND;
    USHORT nCount = (USHORT)mpData->m_aItems.size();
    for( USHORT i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = &mpData->m_aItems[i];

        // Nur malen, wenn Rechteck im PaintRectangle liegt
        if ( !pItem->maRect.IsEmpty() && rPaintRect.IsOver( pItem->maRect ) )
        {
            BOOL bHighlight = FALSE;
            if ( i == mnCurPos )
                bHighlight = 1;
            else if ( i == nHighPos )
                bHighlight = 2;
            ImplDrawItem( i, bHighlight );
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
    long nOldDX = mnDX;
    long nOldDY = mnDY;
    Size aSize = GetOutputSizePixel();
    mnDX = aSize.Width();
    mnDY = aSize.Height();
    if( ImplIsFloatingMode() )
    {
        mpData->m_nDeltaSizeX = abs( mpData->maFloatingSize.Width() - mnDX );
        mpData->m_nDeltaSizeY = abs( mpData->maFloatingSize.Height() - mnDY );
        mpData->maFloatingSize = aSize;
    }
    mnLastResizeDY = 0;

    // Evt. neu formatieren oder neu painten
    if ( mbScroll )
    {
        if ( !mbFormat )
        {
            if( ImplIsFloatingMode() )
            {
                // force reformatting once as the user might have
                // released the mouse button before the timer elapsed
                // which would result in no reformatting at all
                mbFormat = TRUE;
                ImplFormat( TRUE ); // indicate resize
            }
            maTimer.Start();
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

void ToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nItemId;
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
            ULONG nHelpId = GetHelpId( nItemId );
            if ( nHelpId )
            {
                // Wenn eine Hilfe existiert, dann ausloesen
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                    pHelp->Start( nHelpId, this );
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
        USHORT  nKeyCode = aKeyCode.GetCode();
        switch( nKeyCode )
        {
            case KEY_TAB:
                {
                // internal TAB cycling only if parent is not a dialog or if we are the ony child
                // otherwise the dialog control will take over
                BOOL bNoTabCycling = ( ( ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL) ) == WB_DIALOGCONTROL &&
                    ImplGetParent()->GetChildCount() != 1 );

                if( bNoTabCycling )
                    return DockingWindow::Notify( rNEvt );
                else if( ImplChangeHighlightUpDn( aKeyCode.IsShift() ? TRUE : FALSE , bNoTabCycling ) )
                    return FALSE;
                else
                    return DockingWindow::Notify( rNEvt );
                }
                break;
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
                ImplChangeHighlightUpDn( FALSE );
            else
                ImplChangeHighlightUpDn( TRUE );

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
                        mbCommandDrag = TRUE;
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
                    ShowLine( FALSE );
                else if ( (mnCurLine+mnVisLines-1 < mnCurLines) && (pData->GetDelta() < 0) )
                    ShowLine( TRUE );
                ImplDrawSpin( FALSE, FALSE );
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
        mbCalc = TRUE;
        mbFormat = TRUE;
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
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
        mbCalc = TRUE;
        mbFormat = TRUE;
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::PrepareToggleFloatingMode()
{
    return DockingWindow::PrepareToggleFloatingMode();
}

// -----------------------------------------------------------------------

void ToolBox::ToggleFloatingMode()
{
    DockingWindow::ToggleFloatingMode();

    BOOL mbOldHorz = mbHorz;

    if ( ImplIsFloatingMode() )
    {
        mbHorz   = TRUE;
        meAlign  = WINDOWALIGN_TOP;
        mbScroll = TRUE;

        if( mbOldHorz != mbHorz )
            mbCalc = TRUE;  // orientation was changed !

        ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
        if( pWrapper )
            pWrapper->SetMinOutputSizePixel( ImplCalcMinFloatSize( this ) );
        else
            // TODO: change SetMinOutputSizePixel to be not inline
            SetMinOutputSizePixel( ImplCalcMinFloatSize( this ) );

        SetOutputSizePixel( ImplCalcFloatSize( this, mnFloatLines ) );
    }
    else
    {
        mbScroll = (mnWinStyle & WB_SCROLL) ? TRUE : FALSE;
        if ( (meAlign == WINDOWALIGN_TOP) || (meAlign == WINDOWALIGN_BOTTOM) )
            mbHorz = TRUE;
        else
            mbHorz = FALSE;

        // set focus back to document
        ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->GrabFocus();
    }

    if( mbOldHorz != mbHorz )
    {
        // if orientation changes, the toolbox has to be initialized again
        // to update the direction of the gradient
        mbCalc = TRUE;
        ImplInitSettings( TRUE, TRUE, TRUE );
    }

    mbFormat = TRUE;
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

BOOL ToolBox::Docking( const Point& rPos, Rectangle& rRect )
{
    // Wenn Dragging, dann nicht machen, da vorher schon berechnet
    if ( mbDragging )
        return FALSE;

    BOOL bFloatMode = FALSE;

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
        USHORT nTemp = 0;
        aDockingRect.SetSize( ImplCalcFloatSize( this, nTemp ) );

        // in this mode docking is never done by keyboard, so it's OK to use the mouse position
        aDockingRect.SetPos( ImplGetFrameWindow()->GetPointerPosPixel() );
    }

    Rectangle aIntersect = maOutDockRect.GetIntersection( aDockingRect );
    if ( !aIntersect.IsEmpty() && !IsDockingPrevented() )
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
            bFloatMode = TRUE;
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
        bFloatMode = TRUE;

    if ( bFloatMode )
    {
        meDockAlign = meAlign;
        if ( !mbLastFloatMode )
        {
            USHORT nTemp = 0;
            aDockingRect.SetSize( ImplCalcFloatSize( this, nTemp ) );
        }
    }

    rRect = aDockingRect;
    mbLastFloatMode = bFloatMode;

    return bFloatMode;
}

// -----------------------------------------------------------------------

void ToolBox::EndDocking( const Rectangle& rRect, BOOL bFloatMode )
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
    USHORT  nCalcLines;
    USHORT  nTemp;

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

Size ToolBox::CalcWindowSizePixel( USHORT nCalcLines ) const
{
    return ImplCalcSize( this, nCalcLines );
}

Size ToolBox::CalcWindowSizePixel( USHORT nCalcLines, WindowAlign eAlign ) const
{
    return ImplCalcSize( this, nCalcLines,
        (eAlign == WINDOWALIGN_TOP || eAlign == WINDOWALIGN_BOTTOM) ? TB_CALCMODE_HORZ : TB_CALCMODE_VERT );
}

Size ToolBox::CalcPopupWindowSizePixel() const
{
    // count number of breaks and calc corresponding floating window size
    USHORT nLines = 0;
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        if( it->meType == TOOLBOXITEM_BREAK )
            nLines++;
        it++;
    }

    if( nLines )
        nLines++;   // add the first line
    else
    {
        // no breaks found: use quadratic layout
        nLines = (USHORT) ceil( sqrt( (double) GetItemCount() ) );
    }

    return CalcFloatingWindowSizePixel( nLines );
}

Size ToolBox::CalcFloatingWindowSizePixel( USHORT nCalcLines ) const
{
    BOOL bFloat = mpData->mbAssumeFloating;
    BOOL bDocking = mpData->mbAssumeDocked;

    // simulate floating mode and force reformat before calculating
    ToolBox *pThis = (ToolBox*) this;
    pThis->mpData->mbAssumeFloating = TRUE;
    pThis->mpData->mbAssumeDocked = FALSE;

    Size aSize = ImplCalcFloatSize( (ToolBox*) this, nCalcLines );

    pThis->mbFormat = TRUE;
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
                it++;
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

void ToolBox::EnableCustomize( BOOL bEnable )
{
    if ( bEnable != mbCustomize )
    {
        mbCustomize = bEnable;

        ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
        if ( bEnable )
            pMgr->Insert( this );
        else
            pMgr->Remove( this );
    }
}

// -----------------------------------------------------------------------

void ToolBox::StartCustomize( const Rectangle& rRect, void* pData )
{
    DBG_ASSERT( mbCustomize,
                "ToolBox::StartCustomize(): ToolBox must be customized" );

    ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
    Point aMousePos = GetPointerPosPixel();
    Point aPos = ScreenToOutputPixel( rRect.TopLeft() );
    Rectangle aRect( aPos.X(), aPos.Y(),
                     aPos.X()+rRect.GetWidth()+SMALLBUTTON_HSIZE,
                     aPos.Y()+rRect.GetHeight()+SMALLBUTTON_VSIZE );
    aMousePos = ScreenToOutputPixel( aPos );
    Pointer aPtr;
    SetPointer( aPtr );
    pMgr->StartDragging( this, aMousePos, aRect, 0, FALSE, pData );
}

// -----------------------------------------------------------------------

void ToolBox::StartCustomizeMode()
{
    ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
    pMgr->StartCustomizeMode();
}

// -----------------------------------------------------------------------

void ToolBox::EndCustomizeMode()
{
    ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
    pMgr->EndCustomizeMode();
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsCustomizeMode()
{
    ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
    return pMgr->IsCustomizeMode();
}

// -----------------------------------------------------------------------

void ToolBox::GetFocus()
{
    DockingWindow::GetFocus();
}

// -----------------------------------------------------------------------

void ToolBox::LoseFocus()
{
    ImplChangeHighlight( NULL, TRUE );

    DockingWindow::LoseFocus();
}

// -----------------------------------------------------------------------

// performs the action associated with an item, ie simulates clicking the item
void ToolBox::TriggerItem( USHORT nItemId, BOOL bShift, BOOL bCtrl )
{
    mnHighItemId = nItemId;
    USHORT nModifier = 0;
    if( bShift )
        nModifier |= KEY_SHIFT;
    if( bCtrl )
        nModifier |= KEY_MOD1;
    KeyCode aKeyCode( 0, nModifier );
    ImplActivateItem( aKeyCode );
}

// -----------------------------------------------------------------------

// calls the button's action handler
// returns TRUE if action was called
BOOL ToolBox::ImplActivateItem( KeyCode aKeyCode )
{
    BOOL bRet = TRUE;
    if( mnHighItemId )
    {
        ImplToolItem *pItem = ImplGetItem( mnHighItemId );

        // #107712#, activate can also be called for disabled entries
        if( !pItem->mbEnabled )
            return TRUE;

        if( pItem && pItem->mpWindow && HasFocus() )
        {
            ImplHideFocus();
            mbChangingHighlight = TRUE;  // avoid focus change due to loose focus
            pItem->mpWindow->ImplControlFocus( GETFOCUS_TAB );
            mbChangingHighlight = FALSE;
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
            mbIsKeyEvent = TRUE;
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
            mbIsKeyEvent = FALSE;
            mnMouseModifier = 0;
        }
    }
    else
        bRet = FALSE;
    return bRet;
}

// -----------------------------------------------------------------------

// opens a drop down toolbox item
// returns TRUE if item was opened
BOOL ToolBox::ImplOpenItem( KeyCode aKeyCode )
{
    USHORT nCode = aKeyCode.GetCode();
    BOOL bRet = TRUE;

    // arrow keys should work only in the opposite direction of alignment (to not break cursor travelling)
    if ( ((nCode == KEY_LEFT || nCode == KEY_RIGHT) && IsHorizontal())
      || ((nCode == KEY_UP   || nCode == KEY_DOWN)  && !IsHorizontal()) )
        return FALSE;

    if( mnHighItemId &&  ImplGetItem( mnHighItemId ) &&
        (ImplGetItem( mnHighItemId )->mnBits & TIB_DROPDOWN) )
    {
        // close last popup toolbox (see also:
        // ImplHandleMouseFloatMode(...) in winproc.cxx )

        if( ImplGetSVData()->maWinData.mpFirstFloat )
        {
            FloatingWindow* pLastLevelFloat = ImplGetSVData()->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
            // only close the floater if it is not our direct parent, which would kill ourself
            if( pLastLevelFloat && pLastLevelFloat != GetParent() )
            {
                pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
                return bRet;
            }
        }

        mnDownItemId = mnCurItemId = mnHighItemId;
        mnLastFocusItemId = mnCurItemId; // save item id for possible later focus restore
        mnMouseModifier = aKeyCode.GetModifier();
        mbIsShift = TRUE;
        mbIsKeyEvent = TRUE;
        Activate();

        GetDropdownClickHdl().Call( this );

        mbIsKeyEvent = FALSE;
        mbIsShift = FALSE;
        mnMouseModifier = 0;
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

void ToolBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    mnKeyModifier = aKeyCode.GetModifier();
    USHORT nCode = aKeyCode.GetCode();
    BOOL bParentIsDialog = ( ( ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL) ) == WB_DIALOGCONTROL );
    BOOL bForwardKey = FALSE;
    BOOL bGrabFocusToDocument = FALSE;

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
                ImplChangeHighlightUpDn( TRUE );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_LEFT:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( IsHorizontal() )
                ImplChangeHighlightUpDn( TRUE );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_DOWN:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( !IsHorizontal() )
                ImplChangeHighlightUpDn( FALSE );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_RIGHT:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( IsHorizontal() )
                ImplChangeHighlightUpDn( FALSE );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_PAGEUP:
            if ( mnCurLine > 1 )
            {
                if( mnCurLine > mnVisLines )
                    mnCurLine -= mnVisLines;
                else
                    mnCurLine = 1;
                mbFormat = TRUE;
                ImplFormat();
                ImplDrawSpin( FALSE, FALSE );
                ImplChangeHighlight( ImplGetFirstValidItem( mnCurLine ) );
            }
        break;
        case KEY_PAGEDOWN:
            if ( mnCurLine+mnVisLines-1 < mnCurLines )
            {
                if( mnCurLine + 2*mnVisLines-1 < mnCurLines )
                    mnCurLine += mnVisLines;
                else
                    mnCurLine = mnCurLines;
                mbFormat = TRUE;
                ImplFormat();
                ImplDrawSpin( FALSE, FALSE );
                ImplChangeHighlight( ImplGetFirstValidItem( mnCurLine ) );
            }
        break;
        case KEY_END:
            {
                ImplChangeHighlight( NULL );
                ImplChangeHighlightUpDn( FALSE );
            }
            break;
        case KEY_HOME:
            {
                ImplChangeHighlight( NULL );
                ImplChangeHighlightUpDn( TRUE );
            }
            break;
        case KEY_ESCAPE:
        {
            if( bParentIsDialog )
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
                mpFrameWindow->GrabFocus();
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
                    bGrabFocusToDocument = TRUE;
                }
            }
            if( !bGrabFocusToDocument )
                bForwardKey = !ImplActivateItem( aKeyCode );
        }
        break;
        default:
            {
            USHORT aKeyGroup = aKeyCode.GetGroup();
            ImplToolItem *pItem = NULL;
            if( mnHighItemId )
                pItem = ImplGetItem( mnHighItemId );
            // #i13931# forward alphanum keyinput into embedded control
            if( (aKeyGroup == KEYGROUP_NUM || aKeyGroup == KEYGROUP_ALPHA ) && pItem && pItem->mpWindow && pItem->mbEnabled )
            {
                Window *pFocusWindow = Application::GetFocusWindow();
                ImplHideFocus();
                mbChangingHighlight = TRUE;  // avoid focus change due to loose focus
                pItem->mpWindow->ImplControlFocus( GETFOCUS_TAB );
                mbChangingHighlight = FALSE;
                if( pFocusWindow != Application::GetFocusWindow() )
                    Application::GetFocusWindow()->KeyInput( rKEvt );
            }
            else
            {
                // do nothing to avoid key presses going into the document
                // while the toolbox has the focus
                // just forward function and special keys and combinations with Alt-key
                if( aKeyGroup == KEYGROUP_FKEYS || aKeyGroup == KEYGROUP_MISC || aKeyCode.IsMod2() )
                    bForwardKey = TRUE;
            }
        }
    }

    if ( aDelData.IsDelete() )
        return;
    ImplRemoveDel( &aDelData );

    // #107251# move focus away if this toolbox was disabled during keyinput
    if( HasFocus() && mpData->mbKeyInputDisabled && (ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL) ) == WB_DIALOGCONTROL)
    {
        USHORT n = 0;
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
USHORT ToolBox::ImplGetItemLine( ImplToolItem* pCurrentItem )
{
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    USHORT nLine = 1;
    while( it != mpData->m_aItems.end() )
    {
        if ( it->meType == TOOLBOXITEM_BREAK || it->mbBreak )
            nLine++;
        if( &(*it) == pCurrentItem)
            break;
        ++it;
    }
    return nLine;
}

// returns the first displayable item in the given line
ImplToolItem* ToolBox::ImplGetFirstValidItem( USHORT nLine )
{
    if( !nLine || nLine > mnCurLines )
        return NULL;

    nLine--;

    std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        // find correct line
        if ( it->meType == TOOLBOXITEM_BREAK || it->mbBreak )
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

// returns the last displayable item in the given line
ImplToolItem* ToolBox::ImplGetLastValidItem( USHORT nLine )
{
    if( !nLine || nLine > mnCurLines )
        return NULL;

    nLine--;
    ImplToolItem *pFound = NULL;
    std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        // find correct line
        if ( it->meType == TOOLBOXITEM_BREAK || it->mbBreak )
            nLine--;
        if( !nLine )
        {
            // find last useful item
            while( it != mpData->m_aItems.end() && ((it->meType == TOOLBOXITEM_BUTTON) &&
                /*it->mbEnabled &&*/ it->mbVisible && !ImplIsFixedControl( &(*it) )) )
            {
                pFound = &(*it);
                ++it;
                if( it == mpData->m_aItems.end() || it->mbBreak )
                    return pFound;    // end of line: return last useful item
            }
            return pFound;
        }
        ++it;
    }

    return pFound;
}

// -----------------------------------------------------------------------

static USHORT ImplFindItemPos( const ImplToolItem* pItem, const std::vector< ImplToolItem >& rList )
{
    USHORT nPos;
    for( nPos = 0; nPos < rList.size(); nPos++ )
        if( &rList[ nPos ] == pItem )
            return nPos;
    return TOOLBOX_ITEM_NOTFOUND;
}

void ToolBox::ImplChangeHighlight( ImplToolItem* pItem, BOOL bNoGrabFocus )
{
    // avoid recursion due to focus change
    if( mbChangingHighlight )
        return;

    mbChangingHighlight = TRUE;

    ImplToolItem* pOldItem = NULL;
    USHORT        oldPos = 0;

    if ( mnHighItemId )
    {
        ImplHideFocus();
        USHORT nPos = GetItemPos( mnHighItemId );
        ImplDrawItem( nPos, FALSE );
        ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, reinterpret_cast< void* >( nPos ) );
        pOldItem = ImplGetItem( mnHighItemId );
        oldPos = ImplFindItemPos( pOldItem, mpData->m_aItems );
    }

    if( !bNoGrabFocus && pItem != pOldItem && pOldItem && pOldItem->mpWindow )
    {
        // move focus into toolbox
        GrabFocus();
    }

    if( pItem )
    {
        USHORT aPos = ImplFindItemPos( pItem, mpData->m_aItems );
        if( aPos != TOOLBOX_ITEM_NOTFOUND)
        {
            // check for line breaks
            USHORT nLine = ImplGetItemLine( pItem );

            if( nLine >= mnCurLine + mnVisLines )
            {
                mnCurLine = nLine - mnVisLines + 1;
                mbFormat = TRUE;
            }
            else if ( nLine < mnCurLine )
            {
                mnCurLine = nLine;
                mbFormat = TRUE;
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

    mbChangingHighlight = FALSE;
}

// -----------------------------------------------------------------------

BOOL ToolBox::ImplChangeHighlightUpDn( BOOL bUp, BOOL bNoCycle )
{
    ImplToolItem* pItem = ImplGetItem( mnHighItemId );

    if( !pItem || !mnHighItemId )
    {
        if( bUp )
        {
            // Select first valid item
            std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
            while( it != mpData->m_aItems.end() )
            {
                if ( (it->meType == TOOLBOXITEM_BUTTON) &&
                    /*it->mbEnabled &&*/ it->mbVisible && !ImplIsFixedControl( &(*it) ))
                    break;
                ++it;
            }

            ImplChangeHighlight( (it != mpData->m_aItems.end()) ? &(*it) : NULL );
            return TRUE;
        }
        else
        {
            // Select last valid item
            std::vector< ImplToolItem >::iterator it = mpData->m_aItems.end();
            ImplToolItem* pItem = NULL;
            while( it != mpData->m_aItems.begin() )
            {
                --it;
                if ( (it->meType == TOOLBOXITEM_BUTTON) &&
                    /*it->mbEnabled &&*/ it->mbVisible && !ImplIsFixedControl( &(*it) ) )
                {
                    pItem = &(*it);
                    break;
                }
            }

            ImplChangeHighlight( pItem );
            return TRUE;
        }
    }

    if( pItem )
    {
        ULONG pos = ImplFindItemPos( pItem, mpData->m_aItems );
        ULONG nCount = mpData->m_aItems.size();
        ULONG i=0;
        do
        {
            if( bUp )
            {
                if( !pos-- )
                {
                    if( bNoCycle )
                        return FALSE;
                    pos = nCount-1;
                }
            }
            else
            {
                if( ++pos >= nCount )
                {
                    if( bNoCycle )
                        return FALSE;
                    pos = 0;
                }
            }

            pItem = &mpData->m_aItems[pos];
            if ( (pItem->meType == TOOLBOXITEM_BUTTON) &&
                /*pItem->mbEnabled &&*/ pItem->mbVisible && !ImplIsFixedControl( pItem ) )  // #107712# make disabled entries selectable
                break;
        } while( ++i < nCount);

        if( i != nCount )
        {
            ImplChangeHighlight( pItem );
        }
    }
    return TRUE;
}

// -----------------------------------------------------------------------

void ToolBox::ImplShowFocus()
{
    if( mnHighItemId && HasFocus() )
    {
        ImplToolItem* pItem = ImplGetItem( mnHighItemId );
        if( pItem->mpWindow )
        {
            Window *pWin = pItem->mpWindow->mpBorderWindow ? pItem->mpWindow->mpBorderWindow : pItem->mpWindow;
            pWin->mbDrawSelectionBackground = TRUE;
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
            Window *pWin = pItem->mpWindow->mpBorderWindow ? pItem->mpWindow->mpBorderWindow : pItem->mpWindow;
            pWin->mbDrawSelectionBackground = FALSE;
            pWin->Invalidate( 0 );
        }
    }

    if ( mpData->maMenubuttonItem.mpUserData )
    {
        // remove highlight from menubutton
        ImplDrawMenubutton( this, FALSE );
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
            WIN_BYTE Data[6]; // possible values: "true", "false", "1", "0", DWORD
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
