/*************************************************************************
 *
 *  $RCSfile: toolbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-14 13:06:53 $
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
        if ( pThis->mnWinHeight-2 > pThis->mnItemHeight )
            aSize.Height() = nCalcLines * pThis->mnWinHeight-2;
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
    ImplToolItem*   pItem;
    pItem = pThis->mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mbVisible )
        {
            if ( pItem->mpWindow )
            {
                long nTempSize = pItem->mpWindow->GetSizePixel().Width();
                if ( nTempSize > nCalcSize )
                    nCalcSize = nTempSize;
            }
            else if ( pItem->mnNonStdSize )
            {
                if ( pItem->mnNonStdSize > nCalcSize )
                    nCalcSize = pItem->mnNonStdSize;
            }
        }

        pItem = pThis->mpItemList->Next();
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
        aSize.Width() += TB_SPIN_SIZE-TB_SPIN_OFFSET;
    return aSize;
}

// -----------------------------------------------------------------------

static USHORT ImplCalcLines( ToolBox* pThis, long nToolSize )
{
    long nLineHeight;

    if ( pThis->mbHorz )
    {
        if ( pThis->mnWinHeight-2 > pThis->mnItemHeight )
            nLineHeight = pThis->mnWinHeight-2;
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
    ImplToolItem* pItem = pBox->mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mbVisible )
        {
            if ( nLast || !pItem->maRect.IsEmpty() )
            {
                if ( pBox->mbHorz )
                {
                    if ( nLast &&
                         ((nLast < pItem->maRect.Top()) || pItem->maRect.IsEmpty()) )
                        return nPos;

                    if ( aPos.Y() <= pItem->maRect.Bottom() )
                    {
                        if ( aPos.X() < pItem->maRect.Left() )
                            return nPos;
                        else if ( aPos.X() < pItem->maRect.Right() )
                            return nPos+1;
                        else if ( !nLast )
                            nLast = pItem->maRect.Bottom();
                    }
                }
                else
                {
                    if ( nLast &&
                         ((nLast < pItem->maRect.Left()) || pItem->maRect.IsEmpty()) )
                        return nPos;

                    if ( aPos.X() <= pItem->maRect.Right() )
                    {
                        if ( aPos.Y() < pItem->maRect.Top() )
                            return nPos;
                        else if ( aPos.Y() < pItem->maRect.Bottom() )
                            return nPos+1;
                        else if ( !nLast )
                            nLast = pItem->maRect.Right();
                    }
                }
            }
        }

        nPos++;
        pItem = pBox->mpItemList->Next();
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

    pBox = mpBoxList->First();
    while ( pBox )
    {
        if ( pBox->IsReallyVisible() )
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
    mpBtnDev          = NULL;
    mpFloatSizeAry    = NULL;
    mpItemList        = new ImplToolItemList;
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
    meButtonType      = BUTTON_SYMBOL;
    meAlign           = WINDOWALIGN_TOP;
    meLastStyle       = POINTER_ARROW;
    mnWinStyle        = nStyle;
    maTimer.SetTimeoutHdl( LINK( this, ToolBox, ImplUpdateHdl ) );

    DockingWindow::ImplInit( pParent, nStyle & ~(WB_BORDER) );

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
    // Falls noch ein Floating-Window connected ist, dann den
    // PopupModus beenden
    if ( mpFloatWin )
        mpFloatWin->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );

    // Items aus der Liste loeschen
    ImplToolItem* pItem = mpItemList->First();
    while ( pItem )
    {
        delete pItem;
        pItem = mpItemList->Next();
    }

    // Itemlist loeschen
    delete mpItemList;

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
    ImplToolItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mnId == nItemId )
            return pItem;

        pItem = mpItemList->Next();
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

    ImplToolItem*   pItem;
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

    if ( mpItemList->Count() )
    {
        nMaxWidth  = DEF_MIN_WIDTH;
        nMaxHeight = DEF_MIN_HEIGHT;
        mnWinHeight = 0;

        pItem = mpItemList->First();
        while ( pItem )
        {
            if ( pItem->meType == TOOLBOXITEM_BUTTON )
            {
                if ( !(pItem->maImage) )
                    bImage = FALSE;
                else
                    bImage = TRUE;
                if ( !pItem->maText.Len() )
                    bText = FALSE;
                else
                    bText = TRUE;

                if ( bImage || bText )
                {
                    pItem->mbEmptyBtn = FALSE;

                    if ( meButtonType == BUTTON_SYMBOL )
                    {
                        if ( bImage || !bText )
                        {
                            aItemSize = pItem->maImage.GetSizePixel();
                            pItem->mnNonStdSize = 0;
                        }
                        else
                        {
                            aItemSize.Width() = 0;
                            aItemSize.Height() = GetTextHeight();
                            pItem->mnNonStdSize = GetCtrlTextWidth( pItem->maText )+TB_TEXTOFFSET;
                        }
                    }
                    else if ( meButtonType == BUTTON_TEXT )
                    {
                        if ( bText || !bImage )
                        {
                            aItemSize.Width() = GetCtrlTextWidth( pItem->maText )+TB_TEXTOFFSET;
                            aItemSize.Height() = GetTextHeight();
                            pItem->mnNonStdSize = 0;
                        }
                        else
                        {
                            Size aImageSize = pItem->maImage.GetSizePixel();
                            if ( mbHorz )
                            {
                                aItemSize.Width()  = 0;
                                aItemSize.Height() = aImageSize.Height();
                                pItem->mnNonStdSize = aImageSize.Width();
                            }
                            else
                            {
                                aItemSize.Width()  = aImageSize.Width();
                                aItemSize.Height() = 0;
                                pItem->mnNonStdSize = aImageSize.Height();
                            }
                        }
                    }
                    else
                    {
                        aItemSize.Width() = GetCtrlTextWidth( pItem->maText )+TB_TEXTOFFSET;
                        aItemSize.Height() = GetTextHeight();
                        Size aImageSize = pItem->maImage.GetSizePixel();
                        if ( pItem->mnBits & TIB_LEFT )
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
                        pItem->mnNonStdSize = 0;
                    }

                    if ( !pItem->mnNonStdSize && (pItem->mnBits & TIB_AUTOSIZE) )
                    {
                        pItem->mnNonStdSize = aItemSize.Width();
                        aItemSize.Width() = 0;
                    }
                }
                else
                {
                    if ( pItem->mnBits & TIB_LEFT )
                    {
                        aItemSize.Width()  = nDefLeftWidth;
                        aItemSize.Height() = nDefLeftHeight;
                    }
                    else
                    {
                        aItemSize.Width()  = nDefWidth;
                        aItemSize.Height() = nDefHeight;
                    }
                    pItem->mbEmptyBtn = TRUE;
                }

                if ( aItemSize.Width() > nMaxWidth )
                    nMaxWidth = aItemSize.Width();
                if ( aItemSize.Height() > nMaxHeight )
                    nMaxHeight = aItemSize.Height();

                if ( pItem->mnNonStdSize )
                {
                    if ( mbHorz )
                        pItem->mnNonStdSize += SMALLBUTTON_HSIZE;
                    else
                        pItem->mnNonStdSize += SMALLBUTTON_VSIZE;
                }

                // Gegebenenfalls die Fensterhoehe mit beruecksichtigen
                if ( pItem->mpWindow )
                {
                    nHeight = pItem->mpWindow->GetSizePixel().Height();
                    if ( nHeight > mnWinHeight )
                        mnWinHeight = nHeight;
                }
            }

            pItem = mpItemList->Next();
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
    ImplToolItem*   pItem;
    ULONG           nLineStart = 0;
    ULONG           nGroupStart = 0;
    long            nLineWidth = 0;
    long            nCurWidth;
    long            nLastGroupLineWidth = 0;
    long            nMaxLineWidth = 0;
    USHORT          nLines = 1;
    BOOL            bWindow;
    BOOL            bBreak = FALSE;

    pItem = mpItemList->First();
    while ( pItem )
    {
        pItem->mbBreak = bBreak;
        bBreak = FALSE;

        if ( pItem->mbVisible )
        {
            bWindow     = FALSE;
            bBreak      = FALSE;
            nCurWidth   = 0;

            if ( pItem->meType == TOOLBOXITEM_BUTTON )
            {
                if ( pItem->mnNonStdSize )
                    nCurWidth = pItem->mnNonStdSize;
                else
                {
                    if ( bCalcHorz )
                        nCurWidth = mnItemWidth;
                    else
                        nCurWidth = mnItemHeight;
                }

                if ( pItem->mpWindow && bCalcHorz )
                {
                    long nWinItemWidth = pItem->mpWindow->GetSizePixel().Width();
                    if ( !mbScroll || (nWinItemWidth <= nWidth) )
                    {
                        nCurWidth = nWinItemWidth;
                        bWindow   = TRUE;
                    }
                    else
                    {
                        if ( pItem->mbEmptyBtn )
                        {
                            nCurWidth = 0;
                        }
                    }
                }

                if ( (nLineWidth+nCurWidth > nWidth) && mbScroll )
                    bBreak = TRUE;
            }
            else if ( pItem->meType == TOOLBOXITEM_SPACE )
                nCurWidth = mnItemWidth;
            else if ( pItem->meType == TOOLBOXITEM_SEPARATOR )
                nCurWidth = pItem->mnSepSize;
            else if ( pItem->meType == TOOLBOXITEM_BREAK )
                bBreak = TRUE;

            if ( bBreak )
            {
                nLines++;

                // Gruppe auseinanderbrechen oder ganze Gruppe umbrechen?
                if ( (pItem->meType == TOOLBOXITEM_BREAK) ||
                     (nLineStart == nGroupStart) )
                {
                    if ( nLineWidth > nMaxLineWidth )
                        nMaxLineWidth = nLineWidth;

                    nLineWidth = 0;
                    nLineStart = mpItemList->GetCurPos();
                    nGroupStart = nLineStart;
                    pItem->mbBreak = TRUE;
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
                    pItem = mpItemList->Seek( nGroupStart );
                    continue;
                }
            }
            else
            {
                if ( (pItem->meType != TOOLBOXITEM_BUTTON) || bWindow )
                {
                    nLastGroupLineWidth = nLineWidth;
                    nGroupStart = mpItemList->GetCurPos();
                    if ( !bWindow )
                        nGroupStart++;
                }
            }

            nLineWidth += nCurWidth;
        }

        pItem = mpItemList->Next();
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

    // Positionen/Groessen berechnen
    Rectangle       aEmptyRect;
    ImplToolItem*   pItem;
    ImplToolItem*   pTempItem;
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

    // FloatSizeAry gegebenenfalls loeschen
    if ( mpFloatSizeAry )
    {
        delete mpFloatSizeAry;
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

        if ( mnWinHeight-2 > mnItemHeight )
            nLineSize = mnWinHeight-2;

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

        pItem = mpItemList->First();
        while ( pItem )
        {
            pItem->maRect = aEmptyRect;
            pItem = mpItemList->Next();
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
            nMax -= TB_SPIN_SIZE-TB_SPIN_OFFSET;
            if ( mbHorz )
            {
                maNextToolRect.Left()    = nLeft+nMax+TB_SPIN_OFFSET;
                maNextToolRect.Right()   = maNextToolRect.Left()+TB_SPIN_SIZE-1;
                maNextToolRect.Top()     = nTop;
                maNextToolRect.Bottom()  = mnDY-mnBottomBorder-mnBorderY-TB_BORDER_OFFSET2-1;
            }
            else
            {
                maNextToolRect.Top()     = nTop+nMax+TB_SPIN_OFFSET;;
                maNextToolRect.Bottom()  = maNextToolRect.Top()+TB_SPIN_SIZE-1;
                maNextToolRect.Left()    = nLeft;
                maNextToolRect.Right()   = mnDX-mnRightBorder-mnBorderX-TB_BORDER_OFFSET2-1;
            }
        }

        // Haben wir ueberhaupt Items
        if ( mpItemList->GetObject( 0 ) )
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

            pItem = mpItemList->First();
            while ( pItem )
            {
                // Doppelte Separatoren hiden
                if ( mbCustomize )
                {
                    if ( pItem->meType == TOOLBOXITEM_SEPARATOR )
                    {
                        pItem->mbVisible = FALSE;
                        if ( !bLastSep )
                        {
                            // Feststellen ob dahinter ueberhaupt noch
                            // ein Item sichtbar ist
                            ULONG nTempPos = mpItemList->GetCurPos()+1;
                            ULONG nCount = mpItemList->Count();
                            while ( nTempPos < nCount )
                            {
                                pTempItem = mpItemList->GetObject( nTempPos );
                                if ( (pTempItem->meType == TOOLBOXITEM_SEPARATOR) ||
                                     ((pTempItem->meType == TOOLBOXITEM_BUTTON) &&
                                      pTempItem->mbVisible) )
                                {
                                    pItem->mbVisible = TRUE;
                                    break;
                                }
                                nTempPos++;
                            }
                        }
                        bLastSep = TRUE;
                    }
                    else if ( pItem->mbVisible )
                        bLastSep = FALSE;
                }

                pItem->mbShowWindow = FALSE;

                if ( pItem->mbBreak )
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

                if ( !pItem->mbVisible || (nFormatLine < mnCurLine) ||
                     (nFormatLine > mnCurLine+mnVisLines-1) )
                    pItem->maCalcRect = aEmptyRect;
                else
                {
                    if ( (pItem->meType == TOOLBOXITEM_BUTTON) ||
                         (pItem->meType == TOOLBOXITEM_SPACE) )
                    {
                        if ( pItem->mnNonStdSize )
                        {
                            if ( mbHorz )
                            {
                                nCurWidth   = pItem->mnNonStdSize;
                                nCurHeight  = mnItemHeight;
                            }
                            else
                            {
                                nCurWidth   = mnItemWidth;
                                nCurHeight  = pItem->mnNonStdSize;
                            }
                        }
                        else
                        {
                            nCurWidth   = mnItemWidth;
                            nCurHeight  = mnItemHeight;
                        }

                        if ( pItem->mpWindow && mbHorz )
                        {
                            Size aWinSize = pItem->mpWindow->GetSizePixel();
                            if ( !mbScroll || (aWinSize.Width() <= nMax) )
                            {
                                nCurWidth   = aWinSize.Width();
                                nCurHeight  = aWinSize.Height();
                                pItem->mbShowWindow = TRUE;
                            }
                            else
                            {
                                if ( pItem->mbEmptyBtn )
                                {
                                    nCurWidth   = 0;
                                    nCurHeight  = 0;
                                }
                            }
                        }
                    }
                    else if ( pItem->meType == TOOLBOXITEM_SEPARATOR )
                    {
                        if ( mbHorz )
                        {
                            nCurWidth   = pItem->mnSepSize;
                            nCurHeight  = mnItemHeight;
                        }
                        else
                        {
                            nCurWidth   = mnItemWidth;
                            nCurHeight  = pItem->mnSepSize;
                        }
                    }
                    else if ( pItem->meType == TOOLBOXITEM_BREAK )
                    {
                        nCurWidth   = 0;
                        nCurHeight  = 0;
                    }

                    if ( mbHorz )
                    {
                        pItem->maCalcRect.Left()     = nX;
                        pItem->maCalcRect.Top()      = nY+(nLineSize-nCurHeight)/2;
                        pItem->maCalcRect.Right()    = nX+nCurWidth-1;
                        pItem->maCalcRect.Bottom()   = pItem->maCalcRect.Top()+nCurHeight-1;
                        nX += nCurWidth;
                    }
                    else
                    {
                        pItem->maCalcRect.Left()     = nX+(nLineSize-nCurWidth)/2;;
                        pItem->maCalcRect.Top()      = nY;
                        pItem->maCalcRect.Right()    = pItem->maCalcRect.Left()+nCurWidth-1;
                        pItem->maCalcRect.Bottom()   = nY+nCurHeight-1;
                        nY += nCurHeight;
                    }
                }

                if ( pItem->mpWindow )
                {
                    if ( pItem->mbShowWindow )
                    {
                        Point aPos( pItem->maCalcRect.Left(), pItem->maCalcRect.Top() );
                        pItem->mpWindow->SetPosPixel( aPos );
                        if ( !mbCustomizeMode )
                            pItem->mpWindow->Show();
                    }
                    else
                        pItem->mpWindow->Hide();
                }

                pItem = mpItemList->Next();
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

                pItem = mpItemList->First();
                while ( pItem )
                {
                    if ( pItem->maRect != pItem->maCalcRect )
                    {
                        maPaintRect.Union( pItem->maRect );
                        maPaintRect.Union( pItem->maCalcRect );
                    }
                    pItem = mpItemList->Next();
                }
            }

            Invalidate( maPaintRect );
        }

        // Neu berechnete Rectangles uebertragen
        maPaintRect = aEmptyRect;
        pItem = mpItemList->First();
        while ( pItem )
        {
            pItem->maRect = pItem->maCalcRect;
            pItem = mpItemList->Next();
        }
    }

    // Es wurde die Leiste neu durchformatiert
    maTimer.Stop();
    mbFormat = FALSE;
}

// -----------------------------------------------------------------------

IMPL_LINK( ToolBox, ImplUpdateHdl, void*, EMPTYARG )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbFormat )
        ImplFormat();

    return 0;
}

// -----------------------------------------------------------------------

static void ImplDrawToolArrow( ToolBox* pBox, long nX, long nY, BOOL bBlack,
                               BOOL bLeft = FALSE, BOOL bTop = FALSE )
{
    Color           aOldFillColor = pBox->GetFillColor();
    WindowAlign     eAlign = pBox->meAlign;
    if ( bLeft )
        eAlign = WINDOWALIGN_RIGHT;
    else if ( bTop )
        eAlign = WINDOWALIGN_BOTTOM;

    switch ( eAlign )
    {
        case WINDOWALIGN_LEFT:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+0, nY+0, nX+0, nY+6 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+1, nX+1, nY+5 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+2, nX+2, nY+4 ) );
            pBox->DrawRect( Rectangle( nX+3, nY+3, nX+3, nY+3 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+1, nY+2, nX+1, nY+4 ) );
                pBox->DrawRect( Rectangle( nX+2, nY+3, nX+2, nY+3 ) );
            }
            break;
        case WINDOWALIGN_TOP:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+0, nY+0, nX+6, nY+0 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+1, nX+5, nY+1 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+2, nX+4, nY+2 ) );
            pBox->DrawRect( Rectangle( nX+3, nY+3, nX+3, nY+3 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+2, nY+1, nX+4, nY+1 ) );
                pBox->DrawRect( Rectangle( nX+3, nY+2, nX+3, nY+2 ) );
            }
            break;
        case WINDOWALIGN_RIGHT:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+3, nY+0, nX+3, nY+6 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+1, nX+2, nY+5 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+2, nX+1, nY+4 ) );
            pBox->DrawRect( Rectangle( nX+0, nY+3, nX+0, nY+3 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+2, nY+2, nX+2, nY+4 ) );
                pBox->DrawRect( Rectangle( nX+1, nY+3, nX+1, nY+3 ) );
            }
            break;
        case WINDOWALIGN_BOTTOM:
            if ( bBlack )
                pBox->SetFillColor( Color( COL_BLACK ) );
            pBox->DrawRect( Rectangle( nX+0, nY+3, nX+6, nY+3 ) );
            pBox->DrawRect( Rectangle( nX+1, nY+2, nX+5, nY+2 ) );
            pBox->DrawRect( Rectangle( nX+2, nY+1, nX+4, nY+1 ) );
            pBox->DrawRect( Rectangle( nX+3, nY+0, nX+3, nY+0 ) );
            if ( bBlack )
            {
                pBox->SetFillColor( aOldFillColor );
                pBox->DrawRect( Rectangle( nX+2, nY+2, nX+4, nY+2 ) );
                pBox->DrawRect( Rectangle( nX+3, nY+1, nX+3, nY+1 ) );
            }
            break;
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
        nX++;
        nY += (maNextToolRect.GetHeight()-12)/2;
    }
    else
    {
        bTop = TRUE;
        nY++;
        nX += (maNextToolRect.GetWidth()-12)/2;
    }

    nX += maNextToolRect.Left();
    nY += maNextToolRect.Top();
    SetLineColor();
    SetFillColor( COL_LIGHTBLUE );
    ImplDrawToolArrow( this, nX, nY, TRUE, bLeft, bTop );
}

// -----------------------------------------------------------------------

void ToolBox::ImplDrawItem( USHORT nPos, BOOL bHighlight, BOOL bPaint )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplToolItem* pItem = mpItemList->GetObject( nPos );

    // Falls Rechteck ausserhalb des sichbaren Bereichs liegt
    if ( pItem->maRect.IsEmpty() )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // Im flachen Style werden auch Separatoren gezeichnet
    if ( (mnOutStyle & TOOLBOX_STYLE_FLAT) &&
         (pItem->meType == TOOLBOXITEM_SEPARATOR) )
    {
        // Strich wird nicht gemalt, wenn vor oder hinter Fenstern
        // oder bei einem Umbruch
        ImplToolItem* pTempItem = mpItemList->GetObject( nPos );
        pTempItem = mpItemList->GetObject( nPos-1 );
        if ( pTempItem && !pTempItem->mbShowWindow )
        {
            pTempItem = mpItemList->GetObject( nPos+1 );
            if ( pTempItem && !pTempItem->mbShowWindow && !pTempItem->mbBreak )
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

        SetFont( rStyleSettings.GetAppFont() );
        SetLineColor( Color( COL_BLACK ) );
        SetFillColor( rStyleSettings.GetFieldColor() );
        SetTextColor( rStyleSettings.GetFieldTextColor() );
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
        DrawCtrlText( aPos, pItem->maText );
        if ( bClip )
            SetClipRegion();
        SetFont( aOldFont );
        SetTextColor( aOldTextColor );

        // Gegebenenfalls noch Config-Frame zeichnen
        if ( pMgr )
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
        nOffX = SMALLBUTTON_OFF_CHECKED_X;
        nOffY = SMALLBUTTON_OFF_CHECKED_Y;
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
        nOffX = SMALLBUTTON_OFF_PRESSED_X;
        nOffY = SMALLBUTTON_OFF_PRESSED_Y;
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

    if ( bImage )
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

        USHORT nTextStyle = 0;
        if ( !pItem->mbEnabled )
            nTextStyle |= TEXT_DRAW_DISABLE;
        DrawCtrlText( Point( nTempOffX, nTempOffY ), pItem->maText,
                      0, STRING_LEN, nTextStyle );

        if ( bRotate )
            SetFont( aOldFont );
    }

    // Evt. noch Pfeil rechts/oben in der Ecke zeichnen
    if ( pItem->mnBits & TIB_DROPDOWN )
    {
        Point aArrowPos( nOffX, nOffY );
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

        Erase( aClearRect );
        BOOL bBlack = FALSE;

        if ( !pItem->mbEnabled || !IsEnabled() )
            SetFillColor( rStyleSettings.GetShadowColor() );
        else
        {
            SetFillColor( COL_LIGHTGREEN );
            bBlack = TRUE;
        }
        ImplDrawToolArrow( this, aArrowPos.X(), aArrowPos.Y(), bBlack );
        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
    }

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

    // Gegebenenfalls noch Config-Frame zeichnen
    if ( pMgr )
        pMgr->UpdateDragRect();
}

// -----------------------------------------------------------------------

void ToolBox::ImplStartCustomizeMode()
{
    mbCustomizeMode = TRUE;

    ImplToolItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mbShowWindow )
        {
            pItem->mpWindow->Hide();

            if ( !(pItem->maRect.IsEmpty()) )
                Invalidate( pItem->maRect );
        }

        pItem = mpItemList->Next();
    }
}

// -----------------------------------------------------------------------

void ToolBox::ImplEndCustomizeMode()
{
    mbCustomizeMode = FALSE;

    ImplToolItem* pItem = mpItemList->First();
    while ( pItem )
    {
        if ( pItem->mbShowWindow )
        {
            if ( !(pItem->maRect.IsEmpty()) )
                Invalidate( pItem->maRect );

            pItem->mpWindow->Show();
        }

        pItem = mpItemList->Next();
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
    if ( mbDrag )
    {
        // Befindet sich Maus ueber dem Item
        ImplToolItem* pItem = mpItemList->GetObject( mnCurPos );
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
        ImplToolItem* pItem = mpItemList->GetObject( mnCurPos );
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
    if ( ImplHandleMouseMove( rMEvt ) )
        return;

    Point aMousePos = rMEvt.GetPosPixel();

    if ( mbSelection )
    {
        USHORT  i = 0;
        USHORT  nNewPos = TOOLBOX_ITEM_NOTFOUND;

        // Item suchen, das geklickt wurde
        ImplToolItem* pItem = mpItemList->First();
        while ( pItem )
        {
            // Wenn Mausposition in diesem Item vorhanden, kann die
            // Suche abgebrochen werden
            if ( pItem->maRect.IsInside( aMousePos ) )
            {
                // Wenn es ein Button ist, dann wird er selektiert
                if ( pItem->meType == TOOLBOXITEM_BUTTON )
                {
                    // Wenn er disablet ist, findet keine Aenderung
                    // statt
                    if ( !pItem->mbEnabled || pItem->mbShowWindow )
                        nNewPos = mnCurPos;
                    else
                        nNewPos = i;
                }

                break;
            }

            i++;
            pItem = mpItemList->Next();
        }

        // Wurde ein neuer Eintrag selektiert
        if ( nNewPos != mnCurPos )
        {
            if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
                ImplDrawItem( mnCurPos );

            mnCurPos = nNewPos;
            if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            {
                mnCurItemId = pItem->mnId;
                ImplDrawItem( mnCurPos, TRUE );
            }
            else
                mnCurItemId = 0;

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
        ImplToolItem* pItem = mpItemList->First();
        while ( pItem )
        {
            // Wenn es ein Customize-Window ist, gegebenenfalls den
            // Resize-Pointer anzeigen
            if ( pItem->mbShowWindow )
            {
                if ( pItem->maRect.IsInside( aMousePos ) )
                {
                    if ( pItem->maRect.Right()-TB_RESIZE_OFFSET <= aMousePos.X() )
                        eStyle = POINTER_HSIZEBAR;
                    break;
                }
            }

            pItem = mpItemList->Next();
        }
    }

    if ( ((eStyle == POINTER_ARROW) && (mnOutStyle & TOOLBOX_STYLE_HANDPOINTER)) ||
         (mnOutStyle & TOOLBOX_STYLE_FLAT) )
    {
        BOOL bClearHigh = TRUE;
        if ( !rMEvt.IsLeaveWindow() && (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
        {
            ImplToolItem* pItem = mpItemList->First();
            while ( pItem )
            {
                if ( pItem->maRect.IsInside( aMousePos ) )
                {
                    if ( (pItem->meType == TOOLBOXITEM_BUTTON) && pItem->mbEnabled )
                    {
                        if ( mnOutStyle & TOOLBOX_STYLE_FLAT )
                        {
                            bClearHigh = FALSE;
                            if ( mnHighItemId != pItem->mnId )
                            {
                                USHORT nTempPos = (USHORT)mpItemList->GetCurPos();
                                if ( mnHighItemId )
                                    ImplDrawItem( GetItemPos( mnHighItemId ) );
                                mnHighItemId = pItem->mnId;
                                ImplDrawItem( nTempPos, 2 );
                            }
                        }
                        if ( mnOutStyle & TOOLBOX_STYLE_HANDPOINTER )
                            eStyle = POINTER_REFHAND;
                    }
                    break;
                }

                pItem = mpItemList->Next();
            }
        }

        if ( bClearHigh && mnHighItemId )
        {
            USHORT nClearPos = GetItemPos( mnHighItemId );
            if ( nClearPos != TOOLBOX_ITEM_NOTFOUND )
                ImplDrawItem( nClearPos, (nClearPos == mnCurPos) ? TRUE : FALSE );
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
        ImplToolItem* pItem = mpItemList->First();
        while ( pItem )
        {
            // Ist es dieses Item
            if ( pItem->maRect.IsInside( aMousePos ) )
            {
                // Ist es ein Separator oder ist das Item disabled,
                // dann mache nichts
                if ( (pItem->meType == TOOLBOXITEM_BUTTON) &&
                     (!pItem->mbShowWindow || mbCustomizeMode) )
                    nNewPos = i;

                break;
            }

            i++;
            pItem = mpItemList->Next();
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
                    Rectangle aItemRect = GetItemRect( pItem->mnId );
                    mnConfigItem = pItem->mnId;

                    BOOL bResizeItem;
                    if ( mbCustomizeMode && pItem->mbShowWindow &&
                         (pItem->maRect.Right()-TB_RESIZE_OFFSET <= aMousePos.X()) )
                        bResizeItem = TRUE;
                    else
                        bResizeItem = FALSE;
                    pMgr->StartDragging( this, aMousePos, aItemRect, 0, bResizeItem );
                    return;
                }
            }

            if ( !pItem->mbEnabled )
            {
                Sound::Beep( SOUND_DISABLE, this );
                Deactivate();
                return;
            }

            // Aktuelle Daten setzen
            USHORT nTrackFlags = 0;
            mnCurPos         = i;
            mnCurItemId      = pItem->mnId;
            mnDownItemId     = mnCurItemId;
            mnMouseClicks    = rMEvt.GetClicks();
            mnMouseModifier  = rMEvt.GetModifier();
            if ( pItem->mnBits & TIB_REPEAT )
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
    USHORT nCount = (USHORT)mpItemList->Count();
    for( USHORT i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = mpItemList->GetObject( i );

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
        {
            mbFormat = TRUE;
            if ( IsReallyVisible() )
                ImplFormat( TRUE );
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
    USHORT nItemId = GetItemId( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

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
                Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aTempRect, aStr );
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
                    pHelp->Start( nHelpId );
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
            ImplToolItem*   pItem = mpItemList->First();
            while ( pItem )
            {
                // Ist es dieses Item
                if ( pItem->maRect.IsInside( aMousePos ) )
                {
                    // Ist es ein Separator oder ist das Item disabled,
                    // dann mache nichts
                    if ( (pItem->meType == TOOLBOXITEM_BUTTON) &&
                         !pItem->mbShowWindow )
                        mbCommandDrag = TRUE;
                    break;
                }

                pItem = mpItemList->Next();
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
    if ( maOutDockRect.IsInside( rPos ) && !IsDockingPrevented() )
    {
        Rectangle   aInRect = maInDockRect;
        Size        aDockSize;
        aDockSize.Width()  = ImplCalcSize( this, mnLines, TB_CALCMODE_VERT ).Width();
        aDockSize.Height() = ImplCalcSize( this, mnLines, TB_CALCMODE_HORZ ).Height();
        aInRect.Left()   += aDockSize.Width()/2;
        aInRect.Top()    += aDockSize.Height()/2;
        aInRect.Right()  -= aDockSize.Width()/2;
        aInRect.Bottom() -= aDockSize.Height()/2;

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
        if ( aInRect.IsInside( rPos ) )
            bFloatMode = TRUE;
        else
        {
            // Wird befinden uns im Dock-Bereich. Jetzt muessen wir
            // feststellen, an welcher Kante angedockt werden soll
            Point           aInPos( rPos.X()-aInRect.Left(), rPos.Y()-aInRect.Top() );
            Size            aInSize  = aInRect.GetSize();
            Size            aOutSize = maOutDockRect.GetSize();
            USHORT          nQuadrant = 0;

            if ( aInPos.Y() > aInSize.Height()/2 )
                nQuadrant += 2;
            if ( aInPos.X() > aInSize.Width()/2 )
                nQuadrant++;

            if ( nQuadrant == 0 )
            {
                if ( aInPos.X() >= aInPos.Y() )
                    meDockAlign = WINDOWALIGN_TOP;
                else
                    meDockAlign = WINDOWALIGN_LEFT;
            }
            else if ( nQuadrant == 1 )
            {
                if ( aInSize.Width()-aInPos.X() >= aInPos.Y() )
                    meDockAlign = WINDOWALIGN_TOP;
                else
                    meDockAlign = WINDOWALIGN_RIGHT;
            }
            else if ( nQuadrant == 2 )
            {
                if ( aInPos.X() <= aInSize.Height()-aInPos.Y() )
                    meDockAlign = WINDOWALIGN_LEFT;
                else
                    meDockAlign = WINDOWALIGN_BOTTOM;
            }
            else
            {
                if ( (rPos.X() >= aInRect.Right()) &&
                     (rPos.Y() >= aInRect.Bottom()) )
                {
                    if ( aInSize.Height()-aInPos.Y() <
                         aInSize.Width()-aInPos.X() )
                        meDockAlign = WINDOWALIGN_RIGHT;
                    else
                        meDockAlign = WINDOWALIGN_BOTTOM;
                }
                else
                {
                    if ( rPos.X() >= aInRect.Right() )
                        meDockAlign = WINDOWALIGN_RIGHT;
                    else
                        meDockAlign = WINDOWALIGN_BOTTOM;
                }
            }

            // Wenn sich Dock-Align geaendert hat, muessen wir die
            // neue Dock-Groesse setzen
            if ( (meDockAlign == WINDOWALIGN_TOP) || (meDockAlign == WINDOWALIGN_BOTTOM) )
                aDockSize.Width() = aOutSize.Width();
            else
                aDockSize.Height() = aOutSize.Height();
            rRect.SetSize( aDockSize );
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
            rRect.SetSize( ImplCalcFloatSize( this, nTemp ) );
        }
    }

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
