/*************************************************************************
 *
 *  $RCSfile: toolbox.cxx,v $
 *
 *  $Revision: 1.60 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 15:19:46 $
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

#define _SV_TOOLBOX_CXX

#include <string.h>

#include <vector>

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
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
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
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
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
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
#define DEF_IMAGE_WIDTH         16
#define DEF_IMAGE_HEIGHT        15
#define DEF_TEXT_WIDTH          40

#define TB_TEXTOFFSET           2
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
        double dHue;

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
    UINT8 cR,cG,cB;
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

// -----------------------------------------------------------------------

struct ImplButtonData
{
    VirtualDevice*  mpBtnDev;
    long            mnWidth;
    long            mnHeight;
    USHORT          mnRefCount;
};

DECLARE_LIST( ImplButtonList, ImplButtonData* );

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
/*
    Color   aBlackColor( COL_BLACK );
    Pen     aOldPen;
    Brush   aOldBrush;
    Pen     aNullPen( PEN_NULL );

    aOldPen = pThis->GetPen();
    pThis->SetPen( aNullPen );
    if ( pThis->IsSVLook() )
    {
        Color aFaceColor( COL_3DFACE );
        Brush aBrush( aFaceColor, aBlackColor, BRUSH_50 );
        aOldBrush = pThis->GetFillInBrush();
        pThis->SetFillInBrush( aBrush );
    }
    else
    {
        Color aWhiteColor( COL_WHITE );
        Brush aBrush( aBlackColor, aWhiteColor, BRUSH_50 );
        aOldBrush = pThis->GetFillInBrush();
        pThis->SetFillInBrush( aBrush );
    }
    pThis->DrawRect( Rectangle( rRect.Left(), rRect.Top(),
                                rRect.Right(), rRect.Top()+2 ) );
    pThis->DrawRect( Rectangle( rRect.Left(), rRect.Top(),
                                rRect.Left()+2, rRect.Bottom() ) );
    pThis->DrawRect( Rectangle( rRect.Left(), rRect.Bottom()-2,
                                rRect.Right(), rRect.Bottom() ) );
    pThis->DrawRect( Rectangle( rRect.Right()-2, rRect.Top(),
                                rRect.Right(), rRect.Bottom() ) );

    pThis->SetPen( aOldPen );
    pThis->SetFillInBrush( aOldBrush );
*/
}

// -----------------------------------------------------------------------

static void ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
                            long& rRight, long& rBottom )
{
    if ( eAlign == WINDOWALIGN_TOP )
    {
        rLeft   = 0;
        rTop    = 2;
        rRight  = 0;
        rBottom = 0;
    }
    else if ( eAlign == WINDOWALIGN_LEFT )
    {
        rLeft   = 2;
        rTop    = 2;
        rRight  = 0;
        rBottom = 2;
    }
    else if ( eAlign == WINDOWALIGN_BOTTOM )
    {
        rLeft   = 0;
        rTop    = 0;
        rRight  = 0;
        rBottom = 2;
    }
    else
    {
        rLeft   = 0;
        rTop    = 2;
        rRight  = 2;
        rBottom = 2;
    }
}

// -----------------------------------------------------------------------

static void ImplDrawBorder( ToolBox* pWin )
{
    const StyleSettings&    rStyleSettings = pWin->GetSettings().GetStyleSettings();
    long                    nDX = pWin->mnDX;
    long                    nDY = pWin->mnDY;

    if ( pWin->meAlign == WINDOWALIGN_BOTTOM )
    {
        pWin->SetLineColor( rStyleSettings.GetShadowColor() );
        pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
        pWin->SetLineColor( rStyleSettings.GetLightColor() );
        pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
    }
    else
    {
        pWin->SetLineColor( rStyleSettings.GetShadowColor() );
        pWin->DrawLine( Point( 0, 0 ), Point( nDX-1, 0 ) );
        pWin->SetLineColor( rStyleSettings.GetLightColor() );
        pWin->DrawLine( Point( 0, 1 ), Point( nDX-1, 1 ) );
        if ( (pWin->meAlign == WINDOWALIGN_LEFT) || (pWin->meAlign == WINDOWALIGN_RIGHT) )
        {
            if ( pWin->meAlign == WINDOWALIGN_LEFT )
            {
                pWin->SetLineColor( rStyleSettings.GetShadowColor() );
                pWin->DrawLine( Point( 0, 0 ), Point( 0, nDY-1 ) );
                pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
                pWin->SetLineColor( rStyleSettings.GetLightColor() );
                pWin->DrawLine( Point( 1, 1 ), Point( 1, nDY-3 ) );
                pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            }
            else
            {
                pWin->SetLineColor( rStyleSettings.GetShadowColor() );
                pWin->DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-3 ) );
                pWin->DrawLine( Point( 0, nDY-2 ), Point( nDX-2, nDY-2 ) );
                pWin->SetLineColor( rStyleSettings.GetLightColor() );
                pWin->DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
                pWin->DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            }
        }
    }
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
    WindowAlign     eOldAlign;
    BOOL            bOldHorz;

    // Gegebenenfalls neu durchrechnen
    if ( nCalcMode )
    {
        eOldAlign   = pThis->meAlign;
        bOldHorz    = pThis->mbHorz;

        if ( nCalcMode == TB_CALCMODE_HORZ )
        {
            ImplCalcBorder( WINDOWALIGN_TOP, nLeft, nTop, nRight, nBottom );
            ((ToolBox*)pThis)->mbHorz = TRUE;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_TOP;
        }
        else if ( nCalcMode == TB_CALCMODE_FLOAT )
        {
            nLeft = nTop = nRight = nBottom = 0;
            ((ToolBox*)pThis)->mbHorz = TRUE;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_TOP;
        }
        else
        {
            ImplCalcBorder( WINDOWALIGN_LEFT, nLeft, nTop, nRight, nBottom );
            ((ToolBox*)pThis)->mbHorz = FALSE;
            if ( pThis->mbHorz != bOldHorz )
                ((ToolBox*)pThis)->meAlign = WINDOWALIGN_LEFT;
        }

        if ( (pThis->meAlign != eOldAlign) || (pThis->mbHorz != bOldHorz) )
            ((ToolBox*)pThis)->mbCalc = TRUE;
    }
    else
        ImplCalcBorder( pThis->meAlign, nLeft, nTop, nRight, nBottom );

    ((ToolBox*)pThis)->ImplCalcItem();

    if ( pThis->mbHorz )
    {
        if ( pThis->mnWinHeight > pThis->mnItemHeight )
            aSize.Height() = nCalcLines * pThis->mnWinHeight;
        else
            aSize.Height() = nCalcLines * pThis->mnItemHeight;

        if ( pThis->mnWinStyle & WB_LINESPACING )
            aSize.Height() += (nCalcLines-1)*TB_LINESPACING;

        if ( pThis->mnWinStyle & WB_BORDER )
            aSize.Height() += (TB_BORDER_OFFSET2*2) + nTop + nBottom;

        if ( !(pThis->mnWinStyle & WB_SCROLL) )
        {
            nMax = 0;
            ((ToolBox*)pThis)->ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, pThis->mbHorz );
            if ( nMax )
                aSize.Width() += nMax;

            if ( pThis->mnWinStyle & WB_BORDER )
                aSize.Width() += (TB_BORDER_OFFSET1*2) + nLeft + nRight;
        }
    }
    else
    {
        aSize.Width() = nCalcLines * pThis->mnItemWidth;

        if ( pThis->mnWinStyle & WB_LINESPACING )
            aSize.Width() += (nCalcLines-1)*TB_LINESPACING;

        if ( pThis->mnWinStyle & WB_BORDER )
            aSize.Width() += (TB_BORDER_OFFSET2*2) + nLeft + nRight;

        if ( !(pThis->mnWinStyle & WB_SCROLL) )
        {
            nMax = 0;
            ((ToolBox*)pThis)->ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, pThis->mbHorz );
            if ( nMax )
                aSize.Height() += nMax;

            if ( pThis->mnWinStyle & WB_BORDER )
                aSize.Height() += (TB_BORDER_OFFSET1*2) + nTop + nBottom;
        }
    }

    // Gegebenenfalls wieder alte Werte herstellen
    if ( nCalcMode )
    {
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

    // min. Groesse berechnen
    long            nCalcSize = pThis->mnItemWidth;

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
            else if ( it->mnNonStdSize )
            {
                if ( it->mnNonStdSize > nCalcSize )
                    nCalcSize = it->mnNonStdSize;
            }
        }
        ++it;
    }

    USHORT  i;
    USHORT  nLines;
    USHORT  nCalcLines;
    USHORT  nTempLines;
    long    nHeight;
    long    nMaxLineWidth;
    long    nDesktopWidth = pThis->GetDesktopRectPixel().GetWidth()-10;
    nCalcLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, TRUE );
    pThis->mpFloatSizeAry = new ImplToolSize[nCalcLines];
    memset( pThis->mpFloatSizeAry, 0, sizeof( ImplToolSize )*nCalcLines );
    i = 0;
    nLines = nCalcLines;
    while ( nLines )
    {
        nHeight = ImplCalcSize( pThis, nLines, TB_CALCMODE_FLOAT ).Height();
        pThis->mpFloatSizeAry[i].mnHeight = nHeight;
        pThis->mpFloatSizeAry[i].mnLines  = nLines;
        if ( nCalcSize >= nDesktopWidth )
        {
            pThis->mpFloatSizeAry[i].mnWidth = nDesktopWidth;
            nLines--;
        }
        else
        {
            pThis->mpFloatSizeAry[i].mnWidth = nMaxLineWidth+(TB_BORDER_OFFSET1*2);
            nLines--;
            if ( nLines )
            {
                do
                {
                    nCalcSize += pThis->mnItemWidth;
                    nTempLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, TRUE );
                }
                while ( (nLines < nTempLines) && (nTempLines != 1) && (nCalcSize < nDesktopWidth) );
                if ( nTempLines < nLines )
                    nLines = nTempLines;
            }
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
    while ( rLines < pThis->mpFloatSizeAry[i].mnLines )
        i++;

    Size aSize( pThis->mpFloatSizeAry[i].mnWidth,
                pThis->mpFloatSizeAry[i].mnHeight );
    rLines = pThis->mpFloatSizeAry[i].mnLines;
    if ( pThis->maNextToolBoxStr.Len() && pThis->mbScroll )
        aSize.Width() += TB_NEXT_SIZE-TB_NEXT_OFFSET;
    return aSize;
}

// -----------------------------------------------------------------------

static USHORT ImplCalcLines( ToolBox* pThis, long nToolSize )
{
    long nLineHeight;

    if ( pThis->mbHorz )
    {
        if ( pThis->mnWinHeight > pThis->mnItemHeight )
            nLineHeight = pThis->mnWinHeight;
        else
            nLineHeight = pThis->mnItemHeight;
    }
    else
        nLineHeight = pThis->mnItemWidth;

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
    if ( !pThis->IsFloatingMode() &&
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
    else if ( nLineMode & DOCK_LINETOP )
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
    else if ( nLineMode & DOCK_LINETOP )
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
#if 0
    // ToolBox suchen
    Point aPos = rRect.Center();
    ToolBox* pBox = mpBoxList->First();
    while ( pBox )
    {
        if ( pBox->IsReallyVisible() )
        {
            Window* pWindow = pBox->ImplGetFrameWindow()->FindWindow( aPos );
            if ( pWindow && pBox->IsWindowOrChild( pWindow ) )
                return pBox;
        }
        pBox = mpBoxList->Next();
    }

    // Falls so nicht gefunden wurde, suchen wir die ToolBox ueber das Rechteck
    pBox = mpBoxList->First();
    while ( pBox )
    {
        if ( pBox->IsReallyVisible() )
        {
            if ( pBox->IsFloatingMode() )
            {
                Rectangle aTempRect( pBox->GetPosPixel(), pBox->GetSizePixel() );
                if ( aTempRect.IsOver( rRect ) )
                    return pBox;
            }
        }

        pBox = mpBoxList->Next();
    }
#endif
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
            if ( !pBox->IsFloatingMode() )
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
        Point aPos = mpDragBox->OutputToScreenPixel( rPos );
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

static ImplButtonList* ImplGetButtonList()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maCtrlData.mpButtonList )
        pSVData->maCtrlData.mpButtonList = new ImplButtonList;
    return pSVData->maCtrlData.mpButtonList;
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

static void ImplInitButtonVirDev( const Window* pBox, VirtualDevice* pVirDev )
{
    // Farben/Settings vom Window am virtuellen Device setzen
    pVirDev->SetSettings( pBox->GetSettings() );
    const Wallpaper& rWallpaper = pBox->GetBackground();
    if ( rWallpaper.GetStyle() == WALLPAPER_NULL )
        pVirDev->SetBackground( Wallpaper( pBox->GetSettings().GetStyleSettings().GetFaceColor() ) );
    else
        pVirDev->SetBackground( rWallpaper );
}

// -----------------------------------------------------------------------

static void ImplDrawButtons( VirtualDevice* pVirDev, long nWidth, long nHeight,
                             USHORT nType )
{
    // Button Frames ausgeben
    long nY = 0;
    if ( nType & TOOLBOX_STYLE_OUTBUTTON )
    {
        for ( USHORT i = 0; i < 6; i++ )
        {
            USHORT nStyle = 0;
            if ( i & 0x01 )
                nStyle = BUTTON_DRAW_PRESSED;
            ImplDrawOutButton( pVirDev, Rectangle( 0, nY, nWidth-1, nY+nHeight-1 ), nStyle );
            nY += nHeight;
        }
    }
    else
    {
        DecorationView aDecoView( pVirDev );

        aDecoView.DrawButton( Rectangle( 0, 0, nWidth-1, nHeight-1 ), 0 );
        nY += nHeight;
        aDecoView.DrawButton( Rectangle( 0, nY, nWidth-1, nY+nHeight-1 ),
                              BUTTON_DRAW_PRESSED );
        nY += nHeight;
        aDecoView.DrawButton( Rectangle( 0, nY, nWidth-1, nY+nHeight-1 ),
                              BUTTON_DRAW_CHECKED );
        nY += nHeight;
        aDecoView.DrawButton( Rectangle( 0, nY, nWidth-1, nY+nHeight-1 ),
                              BUTTON_DRAW_CHECKED | BUTTON_DRAW_PRESSED );
        nY += nHeight;
        aDecoView.DrawButton( Rectangle( 0, nY, nWidth-1, nY+nHeight-1 ),
                              BUTTON_DRAW_DONTKNOW );
        nY += nHeight;
        aDecoView.DrawButton( Rectangle( 0, nY, nWidth-1, nY+nHeight-1 ),
                              BUTTON_DRAW_DONTKNOW | BUTTON_DRAW_PRESSED );
    }
}

// -----------------------------------------------------------------------

static void ImplButtonSysChange( ToolBox* pBox, VirtualDevice* pVirDev, USHORT nType )
{
    ImplButtonList* pBtnList = ImplGetButtonList();
    ImplButtonData* pBtnData;

    pBtnData = pBtnList->First();
    while ( pBtnData )
    {
        if ( pBtnData->mpBtnDev == pVirDev )
        {
            ImplInitButtonVirDev( pBox, pVirDev );
            ImplDrawButtons( pVirDev, pBtnData->mnWidth, pBtnData->mnHeight, nType );
            break;
        }

        pBtnData = pBtnList->Next();
    }
}

// -----------------------------------------------------------------------

static VirtualDevice* ImplGetButtonDevice( ToolBox* pBox,
                                           long nWidth, long nHeight, USHORT nType )
{
    ImplButtonList* pBtnList = ImplGetButtonList();
    ImplButtonData* pBtnData;

    pBtnData = pBtnList->First();
    while ( pBtnData )
    {
        if ( (pBtnData->mnWidth == nWidth) &&
             (pBtnData->mnHeight == nHeight) )
        {
            pBtnData->mnRefCount++;
            return pBtnData->mpBtnDev;
        }

        pBtnData = pBtnList->Next();
    }

    VirtualDevice* pVirDev = new VirtualDevice( *pBox );

    // Neue Groesse vom virtuellen Device setzen
    pVirDev->SetOutputSizePixel( Size( nWidth, nHeight*6 ), TRUE );
    ImplInitButtonVirDev( pBox, pVirDev );
    ImplDrawButtons( pVirDev, nWidth, nHeight, nType );

    pBtnData = new ImplButtonData;
    pBtnData->mpBtnDev   = pVirDev;
    pBtnData->mnWidth    = nWidth;
    pBtnData->mnHeight   = nHeight;
    pBtnData->mnRefCount = 1;
    pBtnList->Insert( pBtnData, LIST_APPEND );

    return pVirDev;
}

// -----------------------------------------------------------------------

static void ImplFreeButtonDevice( VirtualDevice* pVirDev )
{
    ImplButtonList* pBtnList = ImplGetButtonList();
    ImplButtonData* pBtnData;

    // Virtuelles Device suchen und loeschen
    pBtnData = pBtnList->First();
    while ( pBtnData )
    {
        if ( pBtnData->mpBtnDev == pVirDev )
        {
            pBtnData->mnRefCount--;
            if ( !pBtnData->mnRefCount )
            {
                delete pBtnData->mpBtnDev;
                delete pBtnData;
                pBtnList->Remove();
            }

            return;
        }

        pBtnData = pBtnList->Next();
    }

    DBG_ERRORFILE( "ImplFreeButtonDevice(): Button-Device not in list" );
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
    mnItemWidth       = 0;
    mnItemHeight      = 0;
    mnWinHeight       = 0;
    mnBorderX         = 0;
    mnBorderY         = 0;
    mnLeftBorder      = 0;
    mnTopBorder       = 0;
    mnRightBorder     = 0;
    mnBottomBorder    = 0;
    mnLastResizeDY    = 0;
    mnOutStyle        = 0;
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
    mbDummy1_Shift    = FALSE;
    mbDummy2_KeyEvt   = FALSE;
    mbDummy3_ChangingHighlight = FALSE;
    meButtonType      = BUTTON_SYMBOL;
    meAlign           = WINDOWALIGN_TOP;
    meLastStyle       = POINTER_ARROW;
    mnWinStyle        = nStyle;
    mnLastFocusItemId          = 0;
    mnKeyModifier     = 0;
    mnActivateCount   = 0;

    maTimer.SetTimeout( 50 );
    maTimer.SetTimeoutHdl( LINK( this, ToolBox, ImplUpdateHdl ) );

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
            aColor = GetControlBackground();
        else if ( Window::GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetFaceColor();
        else
            aColor = rStyleSettings.GetWindowColor();
        SetBackground( aColor );
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
    if ( IsFloatingMode() )
        mbHorz = TRUE;
    else
        Resize();

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

ToolBox::~ToolBox()
{
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
        delete[] mpFloatSizeAry;

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

    // Button-Device freigeben
    if ( mpBtnDev )
        ImplFreeButtonDevice( mpBtnDev );

    if ( pSVData->maCtrlData.mpButtonList )
    {
        if ( !pSVData->maCtrlData.mpButtonList->Count() )
        {
            delete pSVData->maCtrlData.mpButtonList;
            pSVData->maCtrlData.mpButtonList = NULL;
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

BOOL ToolBox::ImplCalcItem()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    // Muss Itemgroesse ueberhaupt neu berechnet werden
    if ( !mbCalc )
        return FALSE;

    ImplDisableFlatButtons();

    long            nDefWidth;
    long            nDefHeight;
    long            nDefLeftWidth;
    long            nDefLeftHeight;
    long            nMaxWidth;
    long            nMaxHeight;
    long            nHeight;
    BOOL            bImage;
    BOOL            bText;
    Size            aItemSize;

    if ( meButtonType == BUTTON_SYMBOL )
    {
        nDefWidth       = DEF_IMAGE_WIDTH;
        nDefHeight      = DEF_IMAGE_HEIGHT;
        nDefLeftWidth   = nDefWidth;
        nDefLeftHeight  = nDefHeight;
    }
    else if ( meButtonType == BUTTON_TEXT )
    {
        nDefWidth       = DEF_TEXT_WIDTH;
        nDefHeight      = GetTextHeight();
        nDefLeftWidth   = nDefWidth;
        nDefLeftHeight  = nDefHeight;
    }
    else
    {
        nDefWidth       = DEF_TEXT_WIDTH;
        nDefHeight      = DEF_IMAGE_HEIGHT + GetTextHeight();
        nDefLeftWidth   = nDefWidth;
        nDefLeftHeight  = nDefHeight-DEF_IMAGE_HEIGHT;
    }

    if ( ! mpData->m_aItems.empty() )
    {
        nMaxWidth  = DEF_MIN_WIDTH;
        nMaxHeight = DEF_MIN_HEIGHT;
        mnWinHeight = 0;

        std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            if ( it->meType == TOOLBOXITEM_BUTTON )
            {
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
                        if ( bImage || !bText )
                        {
                            aItemSize = it->maImage.GetSizePixel();
                            it->mnNonStdSize = 0;
                        }
                        else
                        {
                            aItemSize.Width() = 0;
                            aItemSize.Height() = GetTextHeight();
                            it->mnNonStdSize = GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET;
                        }
                    }
                    else if ( meButtonType == BUTTON_TEXT )
                    {
                        if ( bText || !bImage )
                        {
                            aItemSize.Width() = GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET;
                            aItemSize.Height() = GetTextHeight();
                            it->mnNonStdSize = 0;
                        }
                        else
                        {
                            Size aImageSize = it->maImage.GetSizePixel();
                            if ( mbHorz )
                            {
                                aItemSize.Width()  = 0;
                                aItemSize.Height() = aImageSize.Height();
                                it->mnNonStdSize = aImageSize.Width();
                            }
                            else
                            {
                                aItemSize.Width()  = aImageSize.Width();
                                aItemSize.Height() = 0;
                                it->mnNonStdSize = aImageSize.Height();
                            }
                        }
                    }
                    else
                    {
                        aItemSize.Width() = GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET;
                        aItemSize.Height() = GetTextHeight();
                        Size aImageSize = it->maImage.GetSizePixel();
                        if ( it->mnBits & TIB_LEFT )
                        {
                            aItemSize.Width() += aImageSize.Width();
                            if ( aImageSize.Height() > aItemSize.Height() )
                                aItemSize.Height() = aImageSize.Height();
                        }
                        else
                        {
                            aItemSize.Height() += aImageSize.Height();
                            if ( aImageSize.Width() > aItemSize.Width() )
                                aItemSize.Width() = aImageSize.Width();
                        }
                        it->mnNonStdSize = 0;
                    }

                    if ( !it->mnNonStdSize && (it->mnBits & TIB_AUTOSIZE) )
                    {
                        it->mnNonStdSize = aItemSize.Width();
                        aItemSize.Width() = 0;
                    }
                }
                else
                {
                    if ( it->mnBits & TIB_LEFT )
                    {
                        aItemSize.Width()  = nDefLeftWidth;
                        aItemSize.Height() = nDefLeftHeight;
                    }
                    else
                    {
                        aItemSize.Width()  = nDefWidth;
                        aItemSize.Height() = nDefHeight;
                    }
                    it->mbEmptyBtn = TRUE;
                }

                if ( aItemSize.Width() > nMaxWidth )
                    nMaxWidth = aItemSize.Width();
                if ( aItemSize.Height() > nMaxHeight )
                    nMaxHeight = aItemSize.Height();

                if ( it->mnNonStdSize )
                {
                    if ( mbHorz )
                        it->mnNonStdSize += SMALLBUTTON_HSIZE;
                    else
                        it->mnNonStdSize += SMALLBUTTON_VSIZE;
                }

                // Gegebenenfalls die Fensterhoehe mit beruecksichtigen
                if ( it->mpWindow )
                {
                    nHeight = it->mpWindow->GetSizePixel().Height();
                    if ( nHeight > mnWinHeight )
                        mnWinHeight = nHeight;
                }
            }

            ++it;
        }
    }
    else
    {
        nMaxWidth  = nDefWidth;
        nMaxHeight = nDefHeight;
    }

    mbCalc = FALSE;
    mbFormat = TRUE;

    // Button-Umrandung dazurechnen
    if ( mnOutStyle & TOOLBOX_STYLE_OUTBUTTON )
    {
        nMaxWidth  += OUTBUTTON_SIZE;
        nMaxHeight += OUTBUTTON_SIZE;
    }
    else
    {
        nMaxWidth  += SMALLBUTTON_HSIZE;
        nMaxHeight += SMALLBUTTON_VSIZE;
    }

    // Muessen die groessen neu berechnet werden
    if ( (nMaxWidth != mnItemWidth) || (nMaxHeight != mnItemHeight) )
    {
        // Neue Werte zuweisen
        mnItemWidth  = nMaxWidth;
        mnItemHeight = nMaxHeight;

        // Button-Device freigeben
        if ( mpBtnDev )
        {
            ImplFreeButtonDevice( mpBtnDev );
            mpBtnDev = NULL;
        }
        if ( !(mnOutStyle & TOOLBOX_STYLE_FLAT) )
            mpBtnDev = ImplGetButtonDevice( this, mnItemWidth, mnItemHeight, mnOutStyle );
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

    std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        it->mbBreak = bBreak;
        bBreak = FALSE;

        if ( it->mbVisible )
        {
            bWindow     = FALSE;
            bBreak      = FALSE;
            nCurWidth   = 0;

            if ( it->meType == TOOLBOXITEM_BUTTON )
            {
                if ( it->mnNonStdSize )
                    nCurWidth = it->mnNonStdSize;
                else
                {
                    if ( bCalcHorz )
                        nCurWidth = mnItemWidth;
                    else
                        nCurWidth = mnItemHeight;
                }

                if ( it->mpWindow && bCalcHorz )
                {
                    long nWinItemWidth = it->mpWindow->GetSizePixel().Width();
                    if ( !mbScroll || (nWinItemWidth <= nWidth) )
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

                if ( (nLineWidth+nCurWidth > nWidth) && mbScroll )
                    bBreak = TRUE;
            }
            else if ( it->meType == TOOLBOXITEM_SPACE )
                nCurWidth = mnItemWidth;
            else if ( it->meType == TOOLBOXITEM_SEPARATOR )
                nCurWidth = it->mnSepSize;
            else if ( it->meType == TOOLBOXITEM_BREAK )
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
                if ( (it->meType != TOOLBOXITEM_BUTTON) || bWindow )
                {
                    nLastGroupLineWidth = nLineWidth;
                    nGroupStart = it - mpData->m_aItems.begin();
                    if ( !bWindow )
                        nGroupStart++;
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
        // Wegen Separatoren kann MaxLineWidth > Width werden, hat aber
        // auf die Umbrueche keine Auswirkung
        if ( nMaxLineWidth > nWidth )
            nMaxLineWidth = nWidth;
        *pMaxLineWidth = nMaxLineWidth;
    }

    return nLines;
}

// -----------------------------------------------------------------------

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
    long            nTop;
    long            nMax;
    long            nX;
    long            nY;
    long            nCurWidth;
    long            nCurHeight;
    USHORT          nFormatLine;
    BOOL            bMustFullPaint;
    BOOL            bLastSep;

    std::vector< ImplToolItem >::iterator   it;
    std::vector< ImplToolItem >::iterator   temp_it;

    // FloatSizeAry gegebenenfalls loeschen
    if ( mpFloatSizeAry )
    {
        delete[] mpFloatSizeAry;
        mpFloatSizeAry = NULL;
    }

    // Borderbreite berechnen
    if ( IsFloatingMode() || !(mnWinStyle & WB_BORDER) )
    {
        mnLeftBorder     = 0;
        mnTopBorder      = 0;
        mnRightBorder    = 0;
        mnBottomBorder   = 0;
    }
    else
        ImplCalcBorder( meAlign, mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );

    // Itemgroesse gegebenenfalls neu berechnet werden
    if ( ImplCalcItem() )
        bMustFullPaint = TRUE;
    else
        bMustFullPaint = FALSE;

    // Im FloatingMode die Fenstergroesse immer neu setzen oder im Resize
    // die neue Anzahl der FloatingLines berechnen
    if ( IsFloatingMode() )
    {
        if ( bResize )
            mnFloatLines = ImplCalcLines( this, mnDY );
        else
            SetOutputSizePixel( ImplCalcFloatSize( this, mnFloatLines ) );
    }

    // Horizontal
    if ( mbHorz )
    {
        nLineSize = mnItemHeight;

        if ( mnWinHeight > mnItemHeight )
            nLineSize = mnWinHeight;

        if ( mbScroll )
        {
            mnVisLines  = ImplCalcLines( this, mnDY );
            nMax        = mnDX;
        }
        else
        {
            mnVisLines  = mnLines;
            nMax        = TB_MAXNOSCROLL;
        }

        if ( mnWinStyle & WB_BORDER )
        {
            nLeft       = TB_BORDER_OFFSET1 + mnLeftBorder;
            nTop        = TB_BORDER_OFFSET2 + mnTopBorder;
            nMax       -= nLeft + TB_BORDER_OFFSET1 + mnRightBorder;
        }
        else
        {
            nLeft       = 0;
            nTop        = 0;
        }

        nLeft += mnBorderX;
        nTop  += mnBorderY;
        nMax  -= mnBorderX*2;
    }
    else
    {
        nLineSize = mnItemWidth;

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
            nMax       -= nTop + TB_BORDER_OFFSET1 + mnBottomBorder;
        }
        else
        {
            nLeft       = 0;
            nTop        = 0;
        }

        nLeft += mnBorderX;
        nTop  += mnBorderY;
        nMax  -= mnBorderY*2;
    }

    // Wenn Fenster keine Groesse hat, dann nichts berechnen. Fuer alle
    // ToolBoxen ohne Scroll muss hier schon die Groesse berechnet werden.
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
            it->maImage.ClearCaches();
            it->maHighImage.ClearCaches();

            ++it;
        }

        maLowerRect = aEmptyRect;
        maUpperRect = aEmptyRect;
        maNextToolRect = aEmptyRect;
    }
    else
    {
        // Anfangswerte setzen
        nX          = nLeft;
        nY          = nTop;
        nFormatLine = 1;
        bLastSep    = TRUE;

        // Scroll-Rectangles merken und zuruecksetzen
        Rectangle aOldLowerRect = maLowerRect;
        Rectangle aOldUpperRect = maUpperRect;
        Rectangle aOldNextToolRect = maNextToolRect;
        maUpperRect = aEmptyRect;
        maLowerRect = aEmptyRect;
        maNextToolRect = aEmptyRect;

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

        // Haben wir ueberhaupt Items
        if ( !mpData->m_aItems.empty() )
        {
            // Umbrueche und sichtbare Zeilen berechnen
            mnCurLines = ImplCalcBreaks( nMax, NULL, mbHorz );
            if ( (mnCurLines > mnVisLines) && mbScroll )
            {
                nMax -= TB_SPIN_SIZE+TB_SPIN_OFFSET;
                mnCurLines = ImplCalcBreaks( nMax, NULL, mbHorz );
                // Wenn wir umbrechen muessen, dann Scroll-Rectangles neu setzen
                if ( mbHorz )
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
            if ( mnVisLines >= mnCurLines )
                mnCurLine = 1;
            else if ( mnCurLine+mnVisLines-1 > mnCurLines )
                mnCurLine = mnCurLines - (mnVisLines-1);

            it = mpData->m_aItems.begin();
            while ( it != mpData->m_aItems.end() )
            {
                // Doppelte Separatoren hiden
                if ( it->meType == TOOLBOXITEM_SEPARATOR )
                {
                    it->mbVisible = FALSE;
                    if ( !bLastSep )
                    {
                        // Feststellen ob dahinter ueberhaupt noch
                        // ein Item sichtbar ist
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

                if ( it->mbBreak )
                {
                    nFormatLine++;

                    // Ab der zweiten Zeile erhoehen
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
                    it->maCalcRect = aEmptyRect;
                else
                {
                    if ( (it->meType == TOOLBOXITEM_BUTTON) ||
                         (it->meType == TOOLBOXITEM_SPACE) )
                    {
                        if ( it->mnNonStdSize )
                        {
                            if ( mbHorz )
                            {
                                nCurWidth   = it->mnNonStdSize;
                                nCurHeight  = mnItemHeight;
                            }
                            else
                            {
                                nCurWidth   = mnItemWidth;
                                nCurHeight  = it->mnNonStdSize;
                            }
                        }
                        else
                        {
                            nCurWidth   = mnItemWidth;
                            nCurHeight  = mnItemHeight;
                        }

                        if ( it->mpWindow && mbHorz )
                        {
                            Size aWinSize = it->mpWindow->GetSizePixel();
                            if ( !mbScroll || (aWinSize.Width() <= nMax) )
                            {
                                nCurWidth   = aWinSize.Width();
                                nCurHeight  = aWinSize.Height();
                                it->mbShowWindow = TRUE;
                            }
                            else
                            {
                                if ( it->mbEmptyBtn )
                                {
                                    nCurWidth   = 0;
                                    nCurHeight  = 0;
                                }
                            }
                        }
                    }
                    else if ( it->meType == TOOLBOXITEM_SEPARATOR )
                    {
                        if ( mbHorz )
                        {
                            nCurWidth   = it->mnSepSize;
                            nCurHeight  = mnItemHeight;
                        }
                        else
                        {
                            nCurWidth   = mnItemWidth;
                            nCurHeight  = it->mnSepSize;
                        }
                    }
                    else if ( it->meType == TOOLBOXITEM_BREAK )
                    {
                        nCurWidth   = 0;
                        nCurHeight  = 0;
                    }

                    if ( mbHorz )
                    {
                        it->maCalcRect.Left()     = nX;
                        it->maCalcRect.Top()      = nY+(nLineSize-nCurHeight)/2;
                        it->maCalcRect.Right()    = nX+nCurWidth-1;
                        it->maCalcRect.Bottom()   = it->maCalcRect.Top()+nCurHeight-1;
                        nX += nCurWidth;
                    }
                    else
                    {
                        it->maCalcRect.Left()     = nX+(nLineSize-nCurWidth)/2;;
                        it->maCalcRect.Top()      = nY;
                        it->maCalcRect.Right()    = it->maCalcRect.Left()+nCurWidth-1;
                        it->maCalcRect.Bottom()   = nY+nCurHeight-1;
                        nY += nCurHeight;
                    }
                }

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
            }
        }
        else
            mnCurLines = 1;

        // Wenn ToolBox sichtbar, Paint fuer geaenderte Bereiche ausloesen
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

        // Neu berechnete Rectangles uebertragen
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
                it->maImage.ClearCaches();
                it->maHighImage.ClearCaches();
            }
            ++it;
        }
    }

    // Es wurde die Leiste neu durchformatiert
    mbFormat = FALSE;
}

// -----------------------------------------------------------------------

IMPL_LINK( ToolBox, ImplUpdateHdl, void*, EMPTYARG )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

//#ifndef REMOTE_APPSERVER
    if( GetCurrentModButtons() & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) )
    {
        mbFormat = TRUE;
        ImplFormat( TRUE );
        maTimer.Start();
    }
    else
//#endif
    {
        mbFormat = TRUE;
        ImplFormat();
    }

    return 0;
}

// -----------------------------------------------------------------------

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
    long            n = 0;
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

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    MetricVector* pVector = bLayout ? &mpData->m_pLayoutData->m_aUnicodeBoundRects : NULL;
    String* pDisplayText = bLayout ? &mpData->m_pLayoutData->m_aDisplayText : NULL;

    // Falls Rechteck ausserhalb des sichbaren Bereichs liegt
    if ( pItem->maRect.IsEmpty() )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    BOOL bHighContrastWhite = FALSE;
    Wallpaper aWall = GetDisplayBackground();
    if( aWall.GetColor().IsBright() )
        bHighContrastWhite = TRUE;

    // Im flachen Style werden auch Separatoren gezeichnet
    if ( !bLayout &&
         (mnOutStyle & TOOLBOX_STYLE_FLAT) &&
         (pItem->meType == TOOLBOXITEM_SEPARATOR) &&
         nPos > 0
         )
    {
        // Strich wird nicht gemalt, wenn vor oder hinter Fenstern
        // oder bei einem Umbruch
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

    // Ist es kein Button oder wird er als Fenster dargestellt,
    // dann mache nichts
    if ( (pItem->meType != TOOLBOXITEM_BUTTON) ||
         (pItem->mbShowWindow && !mbCustomizeMode) )
        return;

    // Wenn wir das Configurations-Item zeichen, brauchen wir dazu ein
    // TBDragMananger
    ImplTBDragMgr* pMgr;
    if ( pItem->mnId == mnConfigItem )
    {
        pMgr = ImplGetTBDragMgr();
        pMgr->HideDragRect();
    }
    else
        pMgr = NULL;

    // Im Konfigurationsmodus werden sichtbare Fenster durch eine andere
    // Darstellung ersetzt
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

    // Button malen
    Point   aBtnPos;
    Size    aBtnSize    = pItem->maRect.GetSize();
    long    nOffX       = SMALLBUTTON_OFF_NORMAL_X;
    long    nOffY       = SMALLBUTTON_OFF_NORMAL_Y;
    long    nTempOffX;
    long    nTempOffY;
    USHORT  nStyle      = 0;

    if ( pItem->meState == STATE_CHECK )
    {
        aBtnPos.Y() = aBtnSize.Height() * 2;
        nStyle |= BUTTON_DRAW_CHECKED;
        //nOffX = SMALLBUTTON_OFF_CHECKED_X;
        //nOffY = SMALLBUTTON_OFF_CHECKED_Y;
    }
    else if ( pItem->meState == STATE_DONTKNOW )
    {
        aBtnPos.Y() = aBtnSize.Height() * 4;
        nStyle |= BUTTON_DRAW_DONTKNOW;
    }
    if ( bHighlight == 1 )
    {
        aBtnPos.Y() += aBtnSize.Height();
        nStyle |= BUTTON_DRAW_PRESSED;
        //nOffX = SMALLBUTTON_OFF_PRESSED_X;
        //nOffY = SMALLBUTTON_OFF_PRESSED_Y;
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
                if ( pItem->meState != STATE_NOCHECK )
                {
                    SetLineColor();
                    SetFillColor( rStyleSettings.GetCheckedColor() );
                    DrawRect( pItem->maRect );
                }
                else
                    Erase( pItem->maRect );
            }
        }
        else
        {
            if ( !pItem->mnNonStdSize )
                DrawOutDev( pItem->maRect.TopLeft(), aBtnSize, aBtnPos, aBtnSize, *mpBtnDev );
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
    }

    nOffX += pItem->maRect.Left();
    nOffY += pItem->maRect.Top();

    // Feststellen, was gemalt werden soll
    BOOL bImage;
    BOOL bText;
    if ( meButtonType == BUTTON_SYMBOL )
    {
        if ( pItem->mnNonStdSize )
        {
            bImage = FALSE;
            bText  = TRUE;
        }
        else
        {
            bImage = TRUE;
            bText  = FALSE;
        }
    }
    else if ( meButtonType == BUTTON_TEXT )
    {
        if ( pItem->mnNonStdSize )
        {
            bImage = TRUE;
            bText  = FALSE;
        }
        else
        {
            bImage = FALSE;
            bText  = TRUE;
        }
    }
    else
    {
        bImage = TRUE;
        bText  = TRUE;
    }

    // Werte fuer die Ausgabe bestimmen
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

        // Ausgabeflags bestimmen
        USHORT nImageStyle = 0;

        if ( !pItem->mbEnabled || !IsEnabled() )
            nImageStyle |= IMAGE_DRAW_DISABLE;

        if ( pItem->meState == STATE_DONTKNOW )
            nImageStyle |= IMAGE_DRAW_DISABLE;


        // Image ausgeben
        nTempOffX = nOffX;
        nTempOffY = nOffY;
        if ( pItem->mnBits & TIB_LEFT )
            nTempOffY += (nBtnHeight-aImageSize.Height())/2;
        else
        {
            nTempOffX += (nBtnWidth-aImageSize.Width())/2;
            if ( bText )
                nTempOffY += (nBtnHeight-aTxtSize.Height()-aImageSize.Height())/2;
            else
                nTempOffY += (nBtnHeight-aImageSize.Height())/2;
        }
        if ( bHighlight || (pItem->meState == STATE_CHECK) )
        {
            DrawSelectionBackground( pItem->maRect, bHighlight, pItem->meState == STATE_CHECK, TRUE, pItem->mpWindow ? TRUE : FALSE );

            // draw shadows
            if( bHighlight == 2 && pItem->meState != STATE_CHECK )
            {
                nTempOffX++;
                nTempOffY++;
                if( pImage->HasMaskBitmap() )
                {
                    Color aMaskCol = GetpApp()->GetSettings().GetStyleSettings().GetHighlightColor();
                    USHORT h,s,b;
                    ImplRGBtoHSB( aMaskCol, h, s, b );
                    if( s > 20 ) s=20;
                    if( b > 5 ) b-=5;
                    aMaskCol = ImplHSBtoRGB( h, s, b );
                    DrawMask( Point( nTempOffX, nTempOffY ), pImage->GetMaskBitmap(), aMaskCol );
                }
                nTempOffX-=2;
                nTempOffY-=2;
            }

            if( bHighlight )
            {
                if( bHighContrastWhite )
                    nImageStyle |= IMAGE_DRAW_COLORTRANSFORM;
            }
            DrawImage( Point( nTempOffX, nTempOffY ), *pImage, nImageStyle );

        }
        else
            DrawImage( Point( nTempOffX, nTempOffY ), *pImage, nImageStyle );
    }

    // Text ausgeben
    if ( bText )
    {
        nTempOffX = nOffX;
        nTempOffY = nOffY;

        // Muss Text gegebenenfalls gedreht werden
        Font aOldFont = GetFont();
        BOOL bRotate = FALSE;
        if ( pItem->mnNonStdSize && !bImage && !IsFloatingMode() &&
             ((meAlign == WINDOWALIGN_LEFT) || (meAlign == WINDOWALIGN_RIGHT)) )
        {
            bRotate = TRUE;

            Font aRotateFont = aOldFont;
            if ( meAlign == WINDOWALIGN_LEFT )
            {
                aRotateFont.SetOrientation( 900 );
                nTempOffX += (nBtnWidth-aTxtSize.Height())/2;
                nTempOffY += aTxtSize.Width();
                nTempOffY += (nBtnHeight-aTxtSize.Width())/2;
            }
            else
            {
                aRotateFont.SetOrientation( 2700 );
                nTempOffX += aTxtSize.Height();
                nTempOffX += (nBtnWidth-aTxtSize.Height())/2;
                nTempOffY += (nBtnHeight-aTxtSize.Width())/2;
            }

            SetFont( aRotateFont );
        }
        else
        {
            if ( pItem->mnBits & TIB_LEFT )
            {
                nTempOffX += aImageSize.Width();
                nTempOffY += (nBtnHeight-aTxtSize.Height())/2;
            }
            else
            {
                nTempOffX += (nBtnWidth-aTxtSize.Width())/2;
                if ( bImage )
                    nTempOffY += nBtnHeight-aTxtSize.Height();
                else
                    nTempOffY += (nBtnHeight-aTxtSize.Height())/2;
            }
        }

        // draw selection only if not already draw during imgae output (see above)
        if ( !bLayout && !bImage && (bHighlight || (pItem->meState == STATE_CHECK) ) )
            DrawSelectionBackground( pItem->maRect, bHighlight, pItem->meState == STATE_CHECK, TRUE, pItem->mpWindow ? TRUE : FALSE );

        USHORT nTextStyle = 0;
        if ( !pItem->mbEnabled )
            nTextStyle |= TEXT_DRAW_DISABLE;
        if( bLayout )
        {
            mpData->m_pLayoutData->m_aLineIndices.push_back( mpData->m_pLayoutData->m_aDisplayText.Len() );
            mpData->m_pLayoutData->m_aLineItemIds.push_back( pItem->mnId );
            mpData->m_pLayoutData->m_aLineItemPositions.push_back( nPos );
        }
        DrawCtrlText( Point( nTempOffX, nTempOffY ), pItem->maText,
                      0, STRING_LEN, nTextStyle, pVector, pDisplayText );
        if ( bRotate )
            SetFont( aOldFont );
    }

    if( bLayout )
        return;

    // Evt. noch Pfeil rechts/oben in der Ecke zeichnen
    if ( pItem->mnBits & TIB_DROPDOWN )
    {

        Point aArrowPos( nOffX, nOffY );
        // shadows
        if( bHighlight == 2 )
        {
            aArrowPos.X() -= 2;
            aArrowPos.Y() -= 2;
        }

        aArrowPos.X() += nBtnWidth-6;

        Color       aOldLineColor = GetLineColor();
        Color       aOldFillColor = GetFillColor();
        Rectangle   aClearRect( aArrowPos.X()-1, aArrowPos.Y(),
                                aArrowPos.X()+3, aArrowPos.Y()+4 );
        SetLineColor();

        if ( (meAlign == WINDOWALIGN_LEFT) || (meAlign == WINDOWALIGN_RIGHT) )
        {
            aArrowPos.X()       += 2;
            aClearRect.Left()   += 2;
            aClearRect.Right()  += 2;
            aClearRect.Bottom() += 2;
        }
        else
            aClearRect.Right()  += 2;

        Region aOldRegion = GetClipRegion();

        // set expanded arrow as clipping region
        SetToolArrowClipregion( this, aArrowPos.X(), aArrowPos.Y() );

        aClearRect.Right()+=2;
        aClearRect.Bottom()+=2;

        Erase( aClearRect );

        BOOL bColTransform = FALSE;
        if( bHighlight || (pItem->meState == STATE_CHECK) )
        {
            DrawSelectionBackground( aClearRect, bHighlight, pItem->meState == STATE_CHECK, FALSE, FALSE );
            if( bHighContrastWhite )
                bColTransform = TRUE;
        }

        BOOL bBlack = FALSE;

        if ( !pItem->mbEnabled || !IsEnabled() )
            SetFillColor( rStyleSettings.GetShadowColor() );
        else
        {
            SetFillColor( COL_LIGHTGREEN );
            bBlack = TRUE;
        }

        SetClipRegion( aOldRegion );

        ImplDrawToolArrow( this, aArrowPos.X(), aArrowPos.Y(), bBlack, bColTransform );
        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
    }

    /*
    if ( mnOutStyle & TOOLBOX_STYLE_FLAT )
    {
        if ( bHighlight || (pItem->meState == STATE_CHECK) )
        {
            Point   aPos( pItem->maRect.TopLeft() );
            Size    aSize( pItem->maRect.GetSize() );

            if ( bHighlight == 2 )
                SetLineColor( rStyleSettings.GetLightColor() );
            else
                SetLineColor( rStyleSettings.GetShadowColor() );
            DrawLine( aPos, Point( aPos.X()+aSize.Width()-1, aPos.Y() ) );
            DrawLine( aPos, Point( aPos.X(), aPos.Y()+aSize.Height()-1 ) );
            if ( bHighlight == 2 )
                SetLineColor( rStyleSettings.GetShadowColor() );
            else
                SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( Point( aPos.X()+aSize.Width()-1, aPos.Y() ),
                      Point( aPos.X()+aSize.Width()-1, aPos.Y()+aSize.Height()-1 ) );
            DrawLine( Point( aPos.X(), aPos.Y()+aSize.Height()-1 ),
                      Point( aPos.X()+aSize.Width()-1, aPos.Y()+aSize.Height()-1 ) );
        }
    }
    */

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
                ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, (void*) mnCurPos );
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
                                    ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, (void*) nPos );
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
        if ( bClearHigh && mnHighItemId && !HasChildPathFocus() )
        {
            USHORT nClearPos = GetItemPos( mnHighItemId );
            if ( nClearPos != TOOLBOX_ITEM_NOTFOUND )
            {
                ImplDrawItem( nClearPos, (nClearPos == mnCurPos) ? TRUE : FALSE );
                if( nClearPos != mnCurPos )
                    ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, (void*) nClearPos );
            }
            ImplHideFocus();
            mnHighItemId = 0;
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

    if ( !mbDrag && !mbSelection )
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
    if ( rTEvt.IsTrackingEnded() )
        ImplHandleMouseButtonUp( rTEvt.GetMouseEvent(), rTEvt.IsTrackingCanceled() );
    else
        ImplHandleMouseMove( rTEvt.GetMouseEvent(), rTEvt.IsTrackingRepeat() );

    DockingWindow::Tracking( rTEvt );
}

// -----------------------------------------------------------------------

void ToolBox::Paint( const Rectangle& rPaintRect )
{
    if ( rPaintRect == Rectangle( 0, 0, mnDX-1, mnDY-1 ) )
        mbFullPaint = TRUE;
    ImplFormat();
    mbFullPaint = FALSE;

    if ( (mnWinStyle & WB_BORDER) && !IsFloatingMode() )
        ImplDrawBorder( this );

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
    mnLastResizeDY = 0;

    // Evt. neu formatieren oder neu painten
    if ( mbScroll )
    {
        if ( !mbFormat )
            maTimer.Start();
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
                aStr = GetItemText( nItemId );
            if ( rHEvt.GetMode() & HELPMODE_BALLOON )
            {
                if ( rHelpStr.Len() )
                    aStr = rHelpStr;
                else
                    aStr.EraseAllChars( '~' );
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
        if ( mpBtnDev )
            ImplButtonSysChange( this, mpBtnDev, mnOutStyle );
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

    if ( IsFloatingMode() )
    {
        mbHorz   = TRUE;
        meAlign  = WINDOWALIGN_TOP;
        mbScroll = TRUE;
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

    mbFormat = TRUE;
    ImplFormat();
}

// -----------------------------------------------------------------------

void ToolBox::StartDocking()
{
    meDockAlign = meAlign;
    mnDockLines = mnLines;
    mbLastFloatMode = IsFloatingMode();
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
    if ( !IsFloatingMode() )
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
/*
        // Auf den Dockbereich eine halbe Breite der ToolBox dazurechnen
        if ( !IsFloatingMode() )
        {
            if ( meAlign == WINDOWALIGN_LEFT )
                aInRect.Left() -= aDockSize.Width()/2;
            else  if ( meAlign == WINDOWALIGN_TOP )
                aInRect.Top() -= aDockSize.Height()/2;
            else  if ( meAlign == WINDOWALIGN_RIGHT )
                aInRect.Right() += aDockSize.Width()/2;
            else
                aInRect.Bottom() += aDockSize.Height()/2;
        }
*/
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
/*
    // Ist Pointer nicht mehr im Rechteck
    if ( !rRect.IsInside( rPos ) )
    {
        Point aMouseOff;
        aMouseOff.X() = rRect.Left() - rPos.X();
        aMouseOff.Y() = rRect.Top() - rPos.Y();

        if ( (rPos.X() < rRect.Left()) || (rPos.X() > rRect.Right()) )
        {
            rRect.SetPos( rPos );
            rRect.Move( -5, aMouseOff.Y() );
        }
        if ( (rPos.Y() < rRect.Top()) || (rPos.Y() > rRect.Bottom()) )
        {
            rRect.SetPos( rPos );
            rRect.Move( aMouseOff.X(), -5 );
        }
    }
*/
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
    if ( bFloatMode || (bFloatMode != IsFloatingMode()) )
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
                (nCalcLines <= mpFloatSizeAry[0].mnLines) )
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

void ToolBox::GetAccessObject( AccessObjectRef& rAcc ) const
{
    rAcc = new AccessObject( (void*) this, ACCESS_TYPE_TOOLBOX );
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
            mbDummy3_ChangingHighlight = TRUE;  // avoid focus change due to loose focus
            pItem->mpWindow->ImplControlFocus( 0 );
            mbDummy3_ChangingHighlight = FALSE;
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
            mbDummy2_KeyEvt = TRUE;
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
            mbDummy2_KeyEvt = FALSE;
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

    // arrow keys should work as the direction of the green arrow suggests
    if ( ( nCode == KEY_LEFT && meAlign != WINDOWALIGN_RIGHT )
        || ( nCode == KEY_RIGHT && meAlign != WINDOWALIGN_LEFT )
        || ( nCode == KEY_UP && meAlign != WINDOWALIGN_BOTTOM )
        || ( nCode == KEY_DOWN && meAlign != WINDOWALIGN_TOP ) )
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
        ImplToolItem* pItem = ImplGetItem( mnHighItemId );

        mnMouseModifier = aKeyCode.GetModifier();
        mbDummy1_Shift = TRUE;
        mbDummy2_KeyEvt = TRUE;
        Activate();
        Click();
        if (pItem->mnBits & TIB_REPEAT)
            Select();
        mbDummy2_KeyEvt = FALSE;
        mbDummy1_Shift = FALSE;
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
            if( !IsHorizontal() && (aKeyCode.IsMod1() || aKeyCode.IsMod2()) && !maNextToolRect.IsEmpty() )
            {
                ImplDrawNext( TRUE );
                ImplDrawNext( FALSE );
                NextToolBox();
            }
            else
            {
                if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                    break;
                if( !IsHorizontal() )
                    ImplChangeHighlightUpDn( TRUE );
                else
                    ImplOpenItem( aKeyCode );
            }
        }
        break;
        case KEY_LEFT:
        {
            // Ctrl-Cursor activates next toolbox, indicated by a blue arrow pointing to the left/up
            if( IsHorizontal() && (aKeyCode.IsMod1() || aKeyCode.IsMod2()) && !maNextToolRect.IsEmpty() )
            {
                ImplDrawNext( TRUE );
                ImplDrawNext( FALSE );
                NextToolBox();
            }
            else
            {
                if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                    break;
                if( IsHorizontal() )
                    ImplChangeHighlightUpDn( TRUE );
                else
                    ImplOpenItem( aKeyCode );
            }
        }
        break;
        case KEY_DOWN:
        {
            // Ctrl-Cursor activates next toolbox, indicated by a blue arrow pointing to the left/up
            if( !IsHorizontal() && (aKeyCode.IsMod1() || aKeyCode.IsMod2()) && !maNextToolRect.IsEmpty() )
            {
                ImplDrawNext( TRUE );
                ImplDrawNext( FALSE );
                NextToolBox();
            }
            else
            {
                if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                    break;
                if( !IsHorizontal() )
                    ImplChangeHighlightUpDn( FALSE );
                else
                    ImplOpenItem( aKeyCode );
            }
        }
        break;
        case KEY_RIGHT:
        {
            // Ctrl-Cursor activates next toolbox, indicated by a blue arrow pointing to the left/up
            if( IsHorizontal() && (aKeyCode.IsMod1() || aKeyCode.IsMod2()) && !maNextToolRect.IsEmpty() )
            {
                ImplDrawNext( TRUE );
                ImplDrawNext( FALSE );
                NextToolBox();
            }
            else
            {
                if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                    break;
                if( IsHorizontal() )
                    ImplChangeHighlightUpDn( FALSE );
                else
                    ImplOpenItem( aKeyCode );
            }
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
            // do nothing to avoid key presses going into the document
            // while the toolbox has the focus
            // just forward function and special keys and combinations with Alt-key
            USHORT aKeyGroup = aKeyCode.GetGroup();
            if( aKeyGroup == KEYGROUP_FKEYS || aKeyGroup == KEYGROUP_MISC || aKeyCode.IsMod2() )
                bForwardKey = TRUE;
        }
    }

    if ( aDelData.IsDelete() )
        return;
    ImplRemoveDel( &aDelData );

    // #107251# move focus away if this toolbox was disabled during keyinput
    if( HasFocus() && mbInputDisabled && (ImplGetParent()->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL) ) == WB_DIALOGCONTROL)
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

static bool ImplIsFixedControl( ImplToolItem *pItem )
{
    return ( pItem->mpWindow &&
            (pItem->mpWindow->GetType() == WINDOW_FIXEDTEXT ||
             pItem->mpWindow->GetType() == WINDOW_FIXEDLINE ||
             pItem->mpWindow->GetType() == WINDOW_GROUPBOX) );
}

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
    if( mbDummy3_ChangingHighlight )
        return;

    mbDummy3_ChangingHighlight = TRUE;

    ImplToolItem* pOldItem = NULL;
    USHORT        oldPos = 0;

    if ( mnHighItemId )
    {
        ImplHideFocus();
        USHORT nPos = GetItemPos( mnHighItemId );
        ImplDrawItem( nPos, FALSE );
        ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHTOFF, (void*) nPos );
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

    mbDummy3_ChangingHighlight = FALSE;
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

    ImplToolItem* pOldItem = pItem;
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
