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

#include <vcl/toolbox.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/accel.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/gradient.hxx>
#include <vcl/layout.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/vclstatuslistener.hxx>

#include <tools/debug.hxx>
#include <tools/rc.h>
#include <tools/poly.hxx>
#include <svl/imageitm.hxx>

#include <svdata.hxx>
#include <window.h>
#include <toolbox.h>
#include <salframe.hxx>
#include <spin.hxx>
#if defined(_WIN32)
#include <svsys.h>
#endif

#include <cstdlib>
#include <string.h>
#include <vector>
#include <math.h>


#define SMALLBUTTON_HSIZE           7
#define SMALLBUTTON_VSIZE           7

#define SMALLBUTTON_OFF_NORMAL_X    3
#define SMALLBUTTON_OFF_NORMAL_Y    3

#define TB_TEXTOFFSET           2
#define TB_IMAGETEXTOFFSET      3
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
#define TB_DRAGWIDTH            8  // the default width of the drag grip

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

typedef ::std::vector< VclPtr<ToolBox> > ImplTBList;


class ImplTBDragMgr
{
private:
    std::unique_ptr<ImplTBList>
                    mpBoxList;
    VclPtr<ToolBox> mpDragBox;
    Point           maMouseOff;
    Rectangle       maRect;
    Rectangle       maStartRect;
    Accelerator     maAccel;
    long            mnMinWidth;
    long            mnMaxWidth;
    sal_uInt16      mnLineMode;
    sal_uInt16      mnStartLines;
    void*           mpCustomizeData;
    bool            mbResizeMode;
    bool            mbShowDragRect;

    ImplTBDragMgr(const ImplTBDragMgr&) = delete;
    ImplTBDragMgr& operator=(const ImplTBDragMgr&) = delete;

public:
                    ImplTBDragMgr();

    void            push_back( ToolBox* pBox )
                        { mpBoxList->push_back( pBox ); }
    void            erase( ToolBox* pBox )
                    {
                        for ( ImplTBList::iterator it = mpBoxList->begin(); it != mpBoxList->end(); ++it ) {
                            if ( *it == pBox ) {
                                mpBoxList->erase( it );
                                break;
                            }
                        }
                    }
    size_t          size() const
                    { return mpBoxList->size(); }

    ToolBox*        FindToolBox( const Rectangle& rRect );

    void            StartDragging( ToolBox* pDragBox, const Point& rPos, const Rectangle& rRect, sal_uInt16 nLineMode,
                                   bool bResizeItem );
    void            Dragging( const Point& rPos );
    void            EndDragging( bool bOK = true );
    void            HideDragRect() { if ( mbShowDragRect ) mpDragBox->HideTracking(); }
    void            UpdateDragRect();
    DECL_LINK( SelectHdl, Accelerator&, void );
};


static ImplTBDragMgr* ImplGetTBDragMgr()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maCtrlData.mpTBDragMgr )
        pSVData->maCtrlData.mpTBDragMgr = new ImplTBDragMgr;
    return pSVData->maCtrlData.mpTBDragMgr;
}

int ToolBox::ImplGetDragWidth( const vcl::RenderContext& rRenderContext, bool bHorz )
{
    int nWidth = TB_DRAGWIDTH;
    if( rRenderContext.IsNativeControlSupported( ControlType::Toolbar, ControlPart::Entire ) )
    {

        ImplControlValue aControlValue;
        Point aPoint;
        Rectangle aContent, aBound;
        Rectangle aArea( aPoint, rRenderContext.GetOutputSizePixel() );

        if ( rRenderContext.GetNativeControlRegion(ControlType::Toolbar,
                bHorz ? ControlPart::ThumbVert : ControlPart::ThumbHorz,
                aArea, ControlState::NONE, aControlValue, OUString(), aBound, aContent) )
        {
            nWidth = bHorz ? aContent.GetWidth() : aContent.GetHeight();
        }
    }

    // increase the hit area of the drag handle according to DPI scale factor
    nWidth *= rRenderContext.GetDPIScaleFactor();

    return nWidth;
}

int ToolBox::ImplGetDragWidth( ToolBox* pThis )
{
    return ToolBox::ImplGetDragWidth( *pThis, pThis->mbHorz );
}

ButtonType determineButtonType( ImplToolItem* pItem, ButtonType defaultType )
{
    ButtonType tmpButtonType = defaultType;
    if ( pItem->mnBits & (ToolBoxItemBits::TEXT_ONLY | ToolBoxItemBits::ICON_ONLY) ) // item has custom setting
    {
        tmpButtonType = ButtonType::SYMBOLTEXT;
        if ( pItem->mnBits & ToolBoxItemBits::TEXT_ONLY )
            tmpButtonType = ButtonType::TEXT;
        else if ( pItem->mnBits & ToolBoxItemBits::ICON_ONLY )
            tmpButtonType = ButtonType::SYMBOLONLY;
    }
    return tmpButtonType;
}

void ToolBox::ImplUpdateDragArea( ToolBox *pThis )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( pThis );
    if( pWrapper )
    {
        if ( pThis->ImplIsFloatingMode() || pWrapper->IsLocked() )
            pWrapper->SetDragArea( Rectangle() );
        else
        {
            if( pThis->meAlign == WindowAlign::Top || pThis->meAlign == WindowAlign::Bottom )
                pWrapper->SetDragArea( Rectangle( 0, 0, ImplGetDragWidth( pThis ), pThis->GetOutputSizePixel().Height() ) );
            else
                pWrapper->SetDragArea( Rectangle( 0, 0, pThis->GetOutputSizePixel().Width(), ImplGetDragWidth( pThis ) ) );
        }
    }
}

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

    // reserve DragArea only for dockable toolbars
    int    dragwidth = ( pWrapper && !pWrapper->IsLocked() ) ? ImplGetDragWidth( const_cast<ToolBox*>(pThis) ) : 0;

    // no shadow border for dockable toolbars
    int    borderwidth = pWrapper ? 0: 2;

    if ( eAlign == WindowAlign::Top )
    {
        rLeft   = borderwidth+dragwidth;
        rTop    = borderwidth;
        rRight  = borderwidth;
        rBottom = 0;
    }
    else if ( eAlign == WindowAlign::Left )
    {
        rLeft   = borderwidth;
        rTop    = borderwidth+dragwidth;
        rRight  = 0;
        rBottom = borderwidth;
    }
    else if ( eAlign == WindowAlign::Bottom )
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

static void ImplCheckUpdate(ToolBox* pThis)
{
    // remove any pending invalidates to avoid
    // have them triggered when paint is locked (see mpData->mbIsPaintLocked)
    // which would result in erasing the background only and not painting any items
    // this must not be done when we're already in Paint()

    // this is only required for transparent toolbars (see ImplDrawTransparentBackground() )
    if( !pThis->IsBackground() && pThis->HasPaintEvent() && !pThis->IsInPaint() )
        pThis->Update();
}

void ToolBox::ImplDrawGrip(vcl::RenderContext& rRenderContext,
        const Rectangle &aDragArea, int nDragWidth, WindowAlign eAlign, bool bHorz)
{
    bool bNativeOk = false;
    const ControlPart ePart = bHorz ? ControlPart::ThumbVert : ControlPart::ThumbHorz;
    const Size aSz( rRenderContext.GetOutputSizePixel() );
    if (rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ePart))
    {
        ToolbarValue aToolbarValue;
        aToolbarValue.maGripRect = aDragArea;

        Point aPt;
        Rectangle aCtrlRegion(aPt, aSz);
        ControlState nState = ControlState::ENABLED;

        bNativeOk = rRenderContext.DrawNativeControl( ControlType::Toolbar, ePart,
                                        aCtrlRegion, nState, aToolbarValue, OUString() );
    }

    if( bNativeOk )
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());

    float fScaleFactor = rRenderContext.GetDPIScaleFactor();

    if (eAlign == WindowAlign::Top || eAlign == WindowAlign::Bottom)
    {
        int height = (int) (0.6 * aSz.Height() + 0.5);
        int i = (aSz.Height() - height) / 2;
        height += i;
        while (i <= height)
        {
            int x = nDragWidth / 2;
            rRenderContext.DrawEllipse(Rectangle(Point(x, i), Size(2 * fScaleFactor, 2 * fScaleFactor)));
            i += 4 * fScaleFactor;
        }
    }
    else
    {
        int width = (int) (0.6 * aSz.Width() + 0.5);
        int i = (aSz.Width() - width) / 2;
        width += i;
        while (i <= width)
        {
            int y = nDragWidth / 2;
            rRenderContext.DrawEllipse(Rectangle(Point(i, y), Size(2 * fScaleFactor, 2 * fScaleFactor)));
            i += 4 * fScaleFactor;
        }
    }
}

void ToolBox::ImplDrawGrip(vcl::RenderContext& rRenderContext)
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper(this);
    if( pWrapper && !pWrapper->GetDragArea().IsEmpty() )
    {
        // execute pending paint requests
        ImplCheckUpdate(this);
        ImplDrawGrip( rRenderContext, pWrapper->GetDragArea(),
                      ImplGetDragWidth(this), meAlign, mbHorz );
    }
}

void ToolBox::ImplDrawGradientBackground(vcl::RenderContext& rRenderContext, ImplDockingWindowWrapper*)
{
    // draw a nice gradient

    Color startCol, endCol;
    const StyleSettings rSettings = rRenderContext.GetSettings().GetStyleSettings();

    startCol = rSettings.GetFaceGradientColor();
    endCol = rSettings.GetFaceColor();
    if (rSettings.GetHighContrastMode())
        // no 'extreme' gradient when high contrast
        startCol = endCol;

    Gradient g;
    g.SetAngle(mbHorz ? 0 : 900);
    g.SetStyle(GradientStyle::Linear);

    g.SetStartColor(startCol);
    g.SetEndColor(endCol);

    bool bLineColor = rRenderContext.IsLineColor();
    Color aOldCol = rRenderContext.GetLineColor();
    rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetShadowColor());

    Size aFullSz(GetOutputSizePixel());
    Size aLineSz(aFullSz);

    // use the linesize only when floating
    // full window height is used when docked (single line)
    if (ImplIsFloatingMode())
    {
        long nLineSize;
        if (mbHorz)
        {
            nLineSize = mnMaxItemHeight;
            if (mnWinHeight > mnMaxItemHeight)
                nLineSize = mnWinHeight;

            aLineSz.Height() = nLineSize;
        }
        else
        {
            nLineSize = mnMaxItemWidth;
            aLineSz.Width() = nLineSize;
        }
    }

    long nLeft, nTop, nRight, nBottom;
    ImplCalcBorder(meAlign, nLeft, nTop, nRight, nBottom, this);

    Size aTopLineSz(aLineSz);
    Size aBottomLineSz(aLineSz);

    if (mnWinStyle & WB_BORDER)
    {
        if (mbHorz)
        {
            aTopLineSz.Height() += TB_BORDER_OFFSET2 + nTop;
            aBottomLineSz.Height() += TB_BORDER_OFFSET2 + nBottom;

            if (mnCurLines == 1)
                aTopLineSz.Height() += TB_BORDER_OFFSET2 + nBottom;
        }
        else
        {
            aTopLineSz.Width() += TB_BORDER_OFFSET1 + nLeft;
            aBottomLineSz.Width() += TB_BORDER_OFFSET1 + nRight;

            if (mnCurLines == 1)
                aTopLineSz.Width() += TB_BORDER_OFFSET1 + nLeft;
        }
    }

    if (mnWinStyle & WB_LINESPACING)
    {
        if (mbHorz)
        {
            aLineSz.Height() += TB_LINESPACING;
            if (mnCurLines > 1)
                aTopLineSz.Height() += TB_LINESPACING;
        }
        else
        {
            aLineSz.Width() += TB_LINESPACING;
            if (mnCurLines > 1)
                aTopLineSz.Width() += TB_LINESPACING;
        }
    }

    if (mbHorz)
    {
        long y = 0;

        rRenderContext.DrawGradient(Rectangle(0, y, aTopLineSz.Width(), y + aTopLineSz.Height()), g);
        y += aTopLineSz.Height();

        while (y < (mnDY - aBottomLineSz.Height()))
        {
            rRenderContext.DrawGradient(Rectangle(0, y, aLineSz.Width(), y + aLineSz.Height()), g);
            y += aLineSz.Height();
        }

        rRenderContext.DrawGradient(Rectangle(0, y, aBottomLineSz.Width(), y + aBottomLineSz.Height()), g);
    }
    else
    {
        long x = 0;

        rRenderContext.DrawGradient(Rectangle(x, 0, x + aTopLineSz.Width(), aTopLineSz.Height()), g);
        x += aTopLineSz.Width();

        while (x < (mnDX - aBottomLineSz.Width()))
        {
            rRenderContext.DrawGradient(Rectangle(x, 0, x + aLineSz.Width(), aLineSz.Height()), g);
            x += aLineSz.Width();
        }

        rRenderContext.DrawGradient(Rectangle( x, 0, x + aBottomLineSz.Width(), aBottomLineSz.Height()), g);
    }

    if( bLineColor )
        rRenderContext.SetLineColor( aOldCol );

}

bool ToolBox::ImplDrawNativeBackground(vcl::RenderContext& rRenderContext, const vcl::Region& /*rRegion*/)
{
    // use NWF
    Point aPt;
    Rectangle aCtrlRegion(aPt, GetOutputSizePixel());
    ControlState  nState = ControlState::ENABLED;

    return rRenderContext.DrawNativeControl( ControlType::Toolbar, mbHorz ? ControlPart::DrawBackgroundHorz : ControlPart::DrawBackgroundVert,
                                    aCtrlRegion, nState, ImplControlValue(), OUString() );
}

void ToolBox::ImplDrawTransparentBackground(vcl::RenderContext& /*rRenderContext*/, const vcl::Region &rRegion)
{
    // just invalidate to trigger paint of the parent
    const bool bOldPaintLock = mpData->mbIsPaintLocked;
    mpData->mbIsPaintLocked = true;

    // send an invalidate to the first opaque parent and invalidate the whole hierarchy from there (noclipchildren)
    Invalidate(rRegion, InvalidateFlags::Update | InvalidateFlags::NoClipChildren);

    mpData->mbIsPaintLocked = bOldPaintLock;
}

void ToolBox::ImplDrawConstantBackground(vcl::RenderContext& rRenderContext, const vcl::Region &rRegion, bool bIsInPopupMode)
{
    // draw a constant color
    if (!bIsInPopupMode)
    {
        // default background
        rRenderContext.Erase(rRegion.GetBoundRect());
    }
    else
    {
        // use different color in popupmode
        const StyleSettings rSettings = rRenderContext.GetSettings().GetStyleSettings();
        Wallpaper aWallpaper(rSettings.GetFaceGradientColor());
        rRenderContext.DrawWallpaper(rRegion.GetBoundRect(), aWallpaper);
    }
}

void ToolBox::ImplDrawBackground(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    // execute pending paint requests
    ImplCheckUpdate(this);

    ImplDockingWindowWrapper* pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper(this);
    bool bIsInPopupMode = ImplIsInPopupMode();

    vcl::Region aPaintRegion(rRect);

    // make sure we do not invalidate/erase too much
    if (IsInPaint())
        aPaintRegion.Intersect(GetActiveClipRegion());

    rRenderContext.Push(PushFlags::CLIPREGION);
    rRenderContext.IntersectClipRegion( aPaintRegion );

    if (!pWrapper)
    {
        // no gradient for ordinary toolbars (not dockable)
        if( !IsBackground() && !IsInPaint() )
            ImplDrawTransparentBackground(rRenderContext, aPaintRegion);
        else
            ImplDrawConstantBackground(rRenderContext,  aPaintRegion, bIsInPopupMode);
    }
    else
    {
        // toolbars known to the dockingmanager will be drawn using NWF or a gradient
        // docked toolbars are transparent and NWF is already used in the docking area which is their common background
        // so NWF is used here for floating toolbars only
        bool bNativeOk = false;
        if( ImplIsFloatingMode() && rRenderContext.IsNativeControlSupported( ControlType::Toolbar, ControlPart::Entire) )
            bNativeOk = ImplDrawNativeBackground(rRenderContext, aPaintRegion);
        if (!bNativeOk)
        {
            const StyleSettings rSetting = Application::GetSettings().GetStyleSettings();
            const bool isHeader = GetAlign() == WindowAlign::Top && !rSetting.GetPersonaHeader().IsEmpty();
            const bool isFooter = GetAlign() == WindowAlign::Bottom && !rSetting.GetPersonaFooter().IsEmpty();
            if (!IsBackground() || isHeader || isFooter)
            {
                if (!IsInPaint())
                    ImplDrawTransparentBackground(rRenderContext, aPaintRegion);
            }
            else
                ImplDrawGradientBackground(rRenderContext, pWrapper);
        }
    }

    // restore clip region
    rRenderContext.Pop();
}

void ToolBox::ImplErase(vcl::RenderContext& rRenderContext, const Rectangle &rRect, bool bHighlight, bool bHasOpenPopup)
{
    // the background of non NWF buttons is painted in a constant color
    // to have the same highlight color (transparency in DrawSelectionBackground())
    // items with open popups will also painted using a constant color
    if (!mpData->mbNativeButtons &&
        (bHighlight || !(GetStyle() & WB_3DLOOK)))
    {
        if (GetStyle() & WB_3DLOOK)
        {
            rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
            rRenderContext.SetLineColor();
            if (bHasOpenPopup)
                // choose the same color as the popup will use
                rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetFaceGradientColor());
            else
                rRenderContext.SetFillColor(Color(COL_WHITE));

            rRenderContext.DrawRect(rRect);
            rRenderContext.Pop();
        }
        else
            ImplDrawBackground(rRenderContext, rRect);
    }
    else
        ImplDrawBackground(rRenderContext, rRect);
}

void ToolBox::ImplDrawBorder(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    long nDX = mnDX;
    long nDY = mnDY;

    ImplDockingWindowWrapper* pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper(this);

    // draw borders for ordinary toolbars only (not dockable)
    if( pWrapper )
        return;

    if (meAlign == WindowAlign::Bottom)
    {
        // draw bottom border
        rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
        rRenderContext.DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
        rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
        rRenderContext.DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
    }
    else
    {
        // draw top border
        rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
        rRenderContext.DrawLine( Point( 0, 0 ), Point( nDX-1, 0 ) );
        rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
        rRenderContext.DrawLine( Point( 0, 1 ), Point( nDX-1, 1 ) );

        if (meAlign == WindowAlign::Left || meAlign == WindowAlign::Right)
        {
            if (meAlign == WindowAlign::Left)
            {
                // draw left-bottom border
                rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
                rRenderContext.DrawLine( Point( 0, 0 ), Point( 0, nDY-1 ) );
                rRenderContext.DrawLine( Point( 0, nDY-2 ), Point( nDX-1, nDY-2 ) );
                rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
                rRenderContext.DrawLine( Point( 1, 1 ), Point( 1, nDY-3 ) );
                rRenderContext.DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            }
            else
            {
                // draw right-bottom border
                rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
                rRenderContext.DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-3 ) );
                rRenderContext.DrawLine( Point( 0, nDY-2 ), Point( nDX-2, nDY-2 ) );
                rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
                rRenderContext.DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
                rRenderContext.DrawLine( Point( 0, nDY-1 ), Point( nDX-1, nDY-1 ) );
            }
        }
    }

    if ( meAlign == WindowAlign::Bottom || meAlign == WindowAlign::Top )
    {
        // draw right border
        rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
        rRenderContext.DrawLine( Point( nDX-2, 0 ), Point( nDX-2, nDY-1 ) );
        rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
        rRenderContext.DrawLine( Point( nDX-1, 0 ), Point( nDX-1, nDY-1 ) );
    }
}

static bool ImplIsFixedControl( const ImplToolItem *pItem )
{
    return ( pItem->mpWindow &&
            (pItem->mpWindow->GetType() == WINDOW_FIXEDTEXT ||
             pItem->mpWindow->GetType() == WINDOW_FIXEDLINE ||
             pItem->mpWindow->GetType() == WINDOW_GROUPBOX) );
}

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
    return nullptr;
}

Size ToolBox::ImplCalcSize( const ToolBox* pThis, sal_uInt16 nCalcLines, sal_uInt16 nCalcMode )
{
    long            nMax;
    long            nLeft = 0;
    long            nTop = 0;
    long            nRight = 0;
    long            nBottom = 0;
    Size            aSize;
    WindowAlign     eOldAlign = pThis->meAlign;
    bool            bOldHorz = pThis->mbHorz;
    bool            bOldAssumeDocked = pThis->mpData->mbAssumeDocked;
    bool            bOldAssumeFloating = pThis->mpData->mbAssumeFloating;

    if ( nCalcMode )
    {
        bool bOldFloatingMode = pThis->ImplIsFloatingMode();

        pThis->mpData->mbAssumeDocked = false;
        pThis->mpData->mbAssumeFloating = false;

        if ( nCalcMode == TB_CALCMODE_HORZ )
        {
            pThis->mpData->mbAssumeDocked = true;   // force non-floating mode during calculation
            ImplCalcBorder( WindowAlign::Top, nLeft, nTop, nRight, nBottom, pThis );
            const_cast<ToolBox*>(pThis)->mbHorz = true;
            if ( pThis->mbHorz != bOldHorz )
                const_cast<ToolBox*>(pThis)->meAlign = WindowAlign::Top;
        }
        else if ( nCalcMode == TB_CALCMODE_VERT )
        {
            pThis->mpData->mbAssumeDocked = true;   // force non-floating mode during calculation
            ImplCalcBorder( WindowAlign::Left, nLeft, nTop, nRight, nBottom, pThis );
            const_cast<ToolBox*>(pThis)->mbHorz = false;
            if ( pThis->mbHorz != bOldHorz )
                const_cast<ToolBox*>(pThis)->meAlign = WindowAlign::Left;
        }
        else if ( nCalcMode == TB_CALCMODE_FLOAT )
        {
            pThis->mpData->mbAssumeFloating = true;   // force non-floating mode during calculation
            nLeft = nTop = nRight = nBottom = 0;
            const_cast<ToolBox*>(pThis)->mbHorz = true;
            if ( pThis->mbHorz != bOldHorz )
                const_cast<ToolBox*>(pThis)->meAlign = WindowAlign::Top;
        }

        if ( (pThis->meAlign != eOldAlign) || (pThis->mbHorz != bOldHorz) ||
             (pThis->ImplIsFloatingMode() != bOldFloatingMode ) )
            const_cast<ToolBox*>(pThis)->mbCalc = true;
    }
    else
        ImplCalcBorder( pThis->meAlign, nLeft, nTop, nRight, nBottom, pThis );

    const_cast<ToolBox*>(pThis)->ImplCalcItem();

    if( !nCalcMode && pThis->ImplIsFloatingMode() )
    {
        aSize = ImplCalcFloatSize( const_cast<ToolBox*>(pThis), nCalcLines );
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
            const_cast<ToolBox*>(pThis)->ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, pThis->mbHorz );
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
            const_cast<ToolBox*>(pThis)->ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, pThis->mbHorz );
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
            const_cast<ToolBox*>(pThis)->meAlign  = eOldAlign;
            const_cast<ToolBox*>(pThis)->mbHorz   = bOldHorz;
            const_cast<ToolBox*>(pThis)->mbCalc   = true;
        }
    }

    return aSize;
}

void ToolBox::ImplCalcFloatSizes( ToolBox* pThis )
{
    if ( !pThis->maFloatSizes.empty() )
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

    sal_uInt16  nLines;
    sal_uInt16  nCalcLines;
    sal_uInt16  nTempLines;
    long    nMaxLineWidth;
    nCalcLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, true );

    pThis->maFloatSizes.reserve( nCalcLines );

    nTempLines = nLines = nCalcLines;
    while ( nLines )
    {
        long nHeight = ImplCalcSize( pThis, nTempLines, TB_CALCMODE_FLOAT ).Height();

        ImplToolSize aSize;
        aSize.mnWidth  = nMaxLineWidth+(TB_BORDER_OFFSET1*2);
        aSize.mnHeight = nHeight;
        aSize.mnLines  = nTempLines;
        pThis->maFloatSizes.push_back( aSize );
        nLines--;
        if ( nLines )
        {
            do
            {
                nCalcSize += pThis->mnMaxItemWidth;
                nTempLines = pThis->ImplCalcBreaks( nCalcSize, &nMaxLineWidth, true );
            }
            while ( (nCalcSize < upperBoundWidth) && (nLines < nTempLines) && (nTempLines != 1) );
            if ( nTempLines < nLines )
                nLines = nTempLines;
        }
    }
}

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
    while ( i + 1u < pThis->maFloatSizes.size() && rLines < pThis->maFloatSizes[i].mnLines )
    {
        i++;
    }

    Size aSize( pThis->maFloatSizes[i].mnWidth, pThis->maFloatSizes[i].mnHeight );
    rLines = pThis->maFloatSizes[i].mnLines;

    return aSize;
}

void ToolBox::ImplCalcMinMaxFloatSize( ToolBox* pThis, Size& rMinSize, Size& rMaxSize )
{
    ImplCalcFloatSizes( pThis );

    sal_uInt16 i = 0;
    rMinSize = Size( pThis->maFloatSizes[i].mnWidth, pThis->maFloatSizes[i].mnHeight );
    rMaxSize = Size( pThis->maFloatSizes[i].mnWidth, pThis->maFloatSizes[i].mnHeight );
    while ( ++i < pThis->maFloatSizes.size() )
    {
        if( pThis->maFloatSizes[i].mnWidth < rMinSize.Width() )
            rMinSize.Width() = pThis->maFloatSizes[i].mnWidth;
        if( pThis->maFloatSizes[i].mnHeight < rMinSize.Height() )
            rMinSize.Height() = pThis->maFloatSizes[i].mnHeight;

        if( pThis->maFloatSizes[i].mnWidth > rMaxSize.Width() )
            rMaxSize.Width() = pThis->maFloatSizes[i].mnWidth;
        if( pThis->maFloatSizes[i].mnHeight > rMaxSize.Height() )
            rMaxSize.Height() = pThis->maFloatSizes[i].mnHeight;
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
        pWrapper->ShowTitleButton( TitleButton::Menu, bool( pThis->GetMenuType() & ToolBoxMenuType::Customize) );
    }
    else
    {
        // TODO: change SetMinOutputSizePixel to be not inline
        pThis->SetMinOutputSizePixel( aMinSize );
        pThis->SetMaxOutputSizePixel( aMaxSize );
    }
}

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

sal_uInt16 ToolBox::ImplTestLineSize( ToolBox* pThis, const Point& rPos )
{
    if ( !pThis->ImplIsFloatingMode() &&
         (!pThis->mbScroll || (pThis->mnLines > 1) || (pThis->mnCurLines > pThis->mnVisLines)) )
    {
        WindowAlign eAlign = pThis->GetAlign();

        if ( eAlign == WindowAlign::Left )
        {
            if ( rPos.X() > pThis->mnDX-DOCK_LINEOFFSET )
                return DOCK_LINEHSIZE | DOCK_LINERIGHT;
        }
        else if ( eAlign == WindowAlign::Top )
        {
            if ( rPos.Y() > pThis->mnDY-DOCK_LINEOFFSET )
                return DOCK_LINEVSIZE | DOCK_LINEBOTTOM;
        }
        else if ( eAlign == WindowAlign::Right )
        {
            if ( rPos.X() < DOCK_LINEOFFSET )
                return DOCK_LINEHSIZE | DOCK_LINELEFT;
        }
        else if ( eAlign == WindowAlign::Bottom )
        {
            if ( rPos.Y() < DOCK_LINEOFFSET )
                return DOCK_LINEVSIZE | DOCK_LINETOP;
        }
    }

    return 0;
}

void ToolBox::ImplLineSizing( ToolBox* pThis, const Point& rPos, Rectangle& rRect, sal_uInt16 nLineMode )
{
    bool    bHorz;
    long    nOneLineSize;
    long    nCurSize;
    long    nMaxSize;
    long    nSize;
    Size    aSize;

    if ( nLineMode & DOCK_LINERIGHT )
    {
        nCurSize = rPos.X() - rRect.Left();
        bHorz = false;
    }
    else if ( nLineMode & DOCK_LINEBOTTOM )
    {
        nCurSize = rPos.Y() - rRect.Top();
        bHorz = true;
    }
    else if ( nLineMode & DOCK_LINELEFT )
    {
        nCurSize = rRect.Right() - rPos.X();
        bHorz = false;
    }
    else if ( nLineMode & DOCK_LINETOP )
    {
        nCurSize = rRect.Bottom() - rPos.Y();
        bHorz = true;
    }
    else {
        OSL_FAIL( "ImplLineSizing: Trailing else" );
        nCurSize = 0;
        bHorz = false;
    }

    Size    aWinSize = pThis->GetSizePixel();
    sal_uInt16  nMaxLines = (pThis->mnLines > pThis->mnCurLines) ? pThis->mnLines : pThis->mnCurLines;
    if ( nMaxLines > TB_MAXLINES )
        nMaxLines = TB_MAXLINES;
    if ( bHorz )
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
            if ( bHorz )
                nSize = aSize.Height();
            else
                nSize = aSize.Width();
            if ( nSize > nMaxSize )
            {
                i--;
                aSize = ImplCalcSize( pThis, i );
                if ( bHorz )
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

ImplTBDragMgr::ImplTBDragMgr()
    : mpBoxList(new ImplTBList())
    , mpDragBox(nullptr)
    , mnMinWidth(0)
    , mnMaxWidth(0)
    , mnLineMode(0)
    , mnStartLines(0)
    , mpCustomizeData(nullptr)
    , mbResizeMode(false)
    , mbShowDragRect(false)
{
    maAccel.InsertItem( KEY_RETURN, vcl::KeyCode( KEY_RETURN ) );
    maAccel.InsertItem( KEY_ESCAPE, vcl::KeyCode( KEY_ESCAPE ) );
    maAccel.SetSelectHdl( LINK( this, ImplTBDragMgr, SelectHdl ) );
}

ToolBox* ImplTBDragMgr::FindToolBox( const Rectangle& rRect )
{
    for (VclPtr<ToolBox> & i : *mpBoxList)
    {
        ToolBox* pBox = i;
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

    return nullptr;
}

void ImplTBDragMgr::StartDragging( ToolBox* pToolBox,
                                   const Point& rPos, const Rectangle& rRect,
                                   sal_uInt16 nDragLineMode, bool bResizeItem )
{
    mpDragBox = pToolBox;
    pToolBox->CaptureMouse();
    pToolBox->mbDragging = true;
    Application::InsertAccel( &maAccel );

    if ( nDragLineMode )
    {
        mnLineMode = nDragLineMode;
        mnStartLines = pToolBox->mnDockLines;
    }
    else
    {
        mpCustomizeData = nullptr;
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
    mbShowDragRect = true;
    pToolBox->ShowTracking( maRect );
}

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

void ImplTBDragMgr::EndDragging( bool bOK )
{
    mpDragBox->HideTracking();
    if (mpDragBox->IsMouseCaptured())
        mpDragBox->ReleaseMouse();
    mpDragBox->mbDragging = false;
    mbShowDragRect = false;
    Application::RemoveAccel( &maAccel );

    if ( mnLineMode )
    {
        if ( !bOK )
        {
            mpDragBox->mnDockLines = mnStartLines;
            mpDragBox->EndDocking( maStartRect, false );
        }
        else
            mpDragBox->EndDocking( maRect, false );
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

                // re-calculate and show ToolBox
                mpDragBox->ImplInvalidate( true );
            }
            else
            {
                Point aOff = mpDragBox->OutputToScreenPixel( Point() );
                Rectangle aScreenRect( maRect );
                aScreenRect.Move( aOff.X(), aOff.Y() );
                ToolBox* pDropBox = FindToolBox( aScreenRect );
                if ( pDropBox )
                {
                    // Determine search position
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
                    ToolBox::ImplFindItemPos( pDropBox, aPos );
                }
            }
        }
        mpCustomizeData = nullptr;
        mbResizeMode = false;
        mpDragBox->Deactivate();
    }

    mpDragBox = nullptr;
}

void ImplTBDragMgr::UpdateDragRect()
{
    // Only update if we're already dragging
    if ( !mbShowDragRect )
        return;

    mpDragBox->ShowTracking( maRect );
}

IMPL_LINK( ImplTBDragMgr, SelectHdl, Accelerator&, rAccel, void )
{
    if ( rAccel.GetCurItemId() == KEY_ESCAPE )
        EndDragging( false );
    else
        EndDragging();
}

void ToolBox::ImplInitToolBoxData()
{
    // initialize variables
    ImplGetWindowImpl()->mbToolBox  = true;
    mpData = new ImplToolBoxPrivateData;
    mpFloatWin        = nullptr;
    mnDX              = 0;
    mnDY              = 0;
    mnMaxItemWidth    = 0;
    mnMaxItemHeight   = 0;
    mnWinHeight       = 0;
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
    mnDockLines       = 0;
    mnConfigItem      = 0;
    mnMouseClicks     = 0;
    mnMouseModifier   = 0;
    mbDrag            = false;
    mbSelection       = false;
    mbCommandDrag     = false;
    mbUpper           = false;
    mbLower           = false;
    mbIn              = false;
    mbCalc            = true;
    mbFormat          = false;
    mbFullPaint       = false;
    mbHorz            = true;
    mbScroll          = false;
    mbLastFloatMode   = false;
    mbCustomize       = false;
    mbCustomizeMode   = false;
    mbDragging        = false;
    mbIsShift         = false;
    mbIsKeyEvent = false;
    mbChangingHighlight = false;
    mbImagesMirrored  = false;
    meButtonType      = ButtonType::SYMBOLONLY;
    meAlign           = WindowAlign::Top;
    meDockAlign       = WindowAlign::Top;
    meLastStyle       = PointerStyle::Arrow;
    mnWinStyle        = 0;
    meLayoutMode      = ToolBoxLayoutMode::Normal;
    meTextPosition    = ToolBoxTextPosition::Right;
    mnLastFocusItemId = 0;
    mnKeyModifier     = 0;
    mnActivateCount   = 0;
    mnImagesRotationAngle = 0;
    mpStatusListener  = new VclStatusListener<ToolBox>(this, ".uno:ImageOrientation");

    mpIdle = new Idle("vcl::ToolBox maIdle update");
    mpIdle->SetPriority( TaskPriority::RESIZE );
    mpIdle->SetInvokeHandler( LINK( this, ToolBox, ImplUpdateHdl ) );

    // set timeout and handler for dropdown items
    mpData->maDropdownTimer.SetTimeout( 250 );
    mpData->maDropdownTimer.SetInvokeHandler( LINK( this, ToolBox, ImplDropdownLongClickHdl ) );
    mpData->maDropdownTimer.SetDebugName( "vcl::ToolBox mpData->maDropdownTimer" );
}

void ToolBox::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    // initialize variables
    mbScroll          = (nStyle & WB_SCROLL) != 0;
    mnWinStyle        = nStyle;

    DockingWindow::ImplInit( pParent, nStyle & ~(WB_BORDER) );

    // dockingwindow's ImplInit removes some bits, so restore them here to allow keyboard handling for toolbars
    ImplGetWindowImpl()->mnStyle |= WB_TABSTOP|WB_NODIALOGCONTROL; // always set WB_TABSTOP for ToolBars
    ImplGetWindowImpl()->mnStyle &= ~WB_DIALOGCONTROL;

    ImplInitSettings(true, true, true);
}

void ToolBox::ApplySettings(vcl::RenderContext& rRenderContext)
{
    mpData->mbNativeButtons = rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ControlPart::Button);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    // Font
    vcl::Font aFont = rStyleSettings.GetToolFont();
    if (IsControlFont())
        aFont.Merge(GetControlFont());
    SetZoomedPointFont(rRenderContext, aFont);

    // ControlForeground
    Color aColor;
    if (IsControlForeground())
        aColor = GetControlForeground();
    else if (Window::GetStyle() & WB_3DLOOK)
        aColor = rStyleSettings.GetButtonTextColor();
    else
        aColor = rStyleSettings.GetWindowTextColor();
    rRenderContext.SetTextColor(aColor);
    rRenderContext.SetTextFillColor();

    if (IsControlBackground())
    {
        aColor = GetControlBackground();
        SetBackground( aColor );
        SetPaintTransparent(false);
        SetParentClipMode();
    }
    else
    {
        if (rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ControlPart::Entire)
            || (GetAlign() == WindowAlign::Top && !Application::GetSettings().GetStyleSettings().GetPersonaHeader().IsEmpty())
            || (GetAlign() == WindowAlign::Bottom && !Application::GetSettings().GetStyleSettings().GetPersonaFooter().IsEmpty()))
        {
            rRenderContext.SetBackground();
            rRenderContext.SetTextColor(rStyleSettings.GetToolTextColor());
            SetPaintTransparent(true);
            SetParentClipMode(ParentClipMode::NoClip);
            mpData->maDisplayBackground = Wallpaper(rStyleSettings.GetFaceColor());
        }
        else
        {
            if (Window::GetStyle() & WB_3DLOOK)
                aColor = rStyleSettings.GetFaceColor();
            else
                aColor = rStyleSettings.GetWindowColor();

            rRenderContext.SetBackground(aColor);
            SetPaintTransparent(false);
            SetParentClipMode();
        }
    }
}

void ToolBox::ImplInitSettings(bool bFont, bool bForeground, bool bBackground)
{
    mpData->mbNativeButtons = IsNativeControlSupported( ControlType::Toolbar, ControlPart::Button );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if (bFont)
    {
        vcl::Font aFont = rStyleSettings.GetToolFont();
        if (IsControlFont())
            aFont.Merge(GetControlFont());
        SetZoomedPointFont(*this, aFont);
    }

    if (bForeground || bFont)
    {
        Color aColor;
        if (IsControlForeground())
            aColor = GetControlForeground();
        else if (Window::GetStyle() & WB_3DLOOK)
            aColor = rStyleSettings.GetButtonTextColor();
        else
            aColor = rStyleSettings.GetWindowTextColor();
        SetTextColor(aColor);
        SetTextFillColor();
    }

    if (bBackground)
    {
        Color aColor;
        if (IsControlBackground())
        {
            aColor = GetControlBackground();
            SetBackground( aColor );
            SetPaintTransparent(false);
            SetParentClipMode();
        }
        else
        {
            if (IsNativeControlSupported(ControlType::Toolbar, ControlPart::Entire)
                || (GetAlign() == WindowAlign::Top && !Application::GetSettings().GetStyleSettings().GetPersonaHeader().IsEmpty())
                || (GetAlign() == WindowAlign::Bottom && !Application::GetSettings().GetStyleSettings().GetPersonaFooter().IsEmpty()))
            {
                SetBackground();
                SetTextColor(rStyleSettings.GetMenuBarTextColor());
                SetPaintTransparent( true );
                SetParentClipMode( ParentClipMode::NoClip );
                mpData->maDisplayBackground = Wallpaper( rStyleSettings.GetFaceColor() );
            }
            else
            {
                if (Window::GetStyle() & WB_3DLOOK)
                    aColor = rStyleSettings.GetFaceColor();
                else
                    aColor = rStyleSettings.GetWindowColor();

                SetBackground(aColor);
                SetPaintTransparent(false);
                SetParentClipMode();
            }
        }
    }
}

void ToolBox::doDeferredInit(WinBits nBits)
{
    VclPtr<vcl::Window> pParent = mpDialogParent;
    mpDialogParent = nullptr;
    ImplInit(pParent, nBits);
    mbIsDefferedInit = false;
}

ToolBox::ToolBox( vcl::Window* pParent, WinBits nStyle ) :
    DockingWindow( WINDOW_TOOLBOX )
{
    ImplInitToolBoxData();
    ImplInit( pParent, nStyle );
}

ToolBox::ToolBox(vcl::Window* pParent, const OString& rID,
    const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : DockingWindow(WINDOW_TOOLBOX)
{
    ImplInitToolBoxData();

    loadUI(pParent, rID, rUIXMLDescription, rFrame);

    // calculate size of floating windows and switch if the
    // toolbox is initially in floating mode
    if ( ImplIsFloatingMode() )
        mbHorz = true;
    else
        Resize();

    if (!(GetStyle() & WB_HIDE))
        Show();
}

ToolBox::~ToolBox()
{
    disposeOnce();
}

void ToolBox::dispose()
{
    // custom menu event still running?
    if( mpData && mpData->mnEventId )
        Application::RemoveUserEvent( mpData->mnEventId );

    // #103005# make sure our activate/deactivate balance is right
    while( mnActivateCount > 0 )
        Deactivate();

    // terminate popupmode if the floating window is
    // still connected
    if ( mpFloatWin )
        mpFloatWin->EndPopupMode( FloatWinPopupEndFlags::Cancel );
    mpFloatWin = nullptr;

    // delete private data
    delete mpData;
    mpData = nullptr;

    // remove the lists when there are no more toolbox references to
    // the lists
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maCtrlData.mpTBDragMgr )
    {
        // remove if in TBDrag-Manager
        if ( mbCustomize )
            pSVData->maCtrlData.mpTBDragMgr->erase( this );

        if ( !pSVData->maCtrlData.mpTBDragMgr->size() )
        {
            delete pSVData->maCtrlData.mpTBDragMgr;
            pSVData->maCtrlData.mpTBDragMgr = nullptr;
        }
    }

    if (mpStatusListener.is())
        mpStatusListener->dispose();

    mpFloatWin.clear();

    delete mpIdle;
    mpIdle = nullptr;

    DockingWindow::dispose();
}

ImplToolItem* ToolBox::ImplGetItem( sal_uInt16 nItemId ) const
{
    if (!mpData)
        return nullptr;

    std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        if ( it->mnId == nItemId )
            return &(*it);
        ++it;
    }

    return nullptr;
}

static void ImplAddButtonBorder( long &rWidth, long& rHeight, bool bNativeButtons )
{
    rWidth += SMALLBUTTON_HSIZE;
    rHeight += SMALLBUTTON_VSIZE;

    if( bNativeButtons )
    {
        // give more border space for rounded buttons
        rWidth += 2;
        rHeight += 4;
    }
}

bool ToolBox::ImplCalcItem()
{

    // recalc required ?
    if ( !mbCalc )
        return false;

    ImplDisableFlatButtons();

    long            nDefWidth;
    long            nDefHeight;
    long            nMaxWidth = 0;
    long            nMaxHeight = 0;
    long            nMinWidth   = 6;
    long            nMinHeight  = 6;
    long            nDropDownArrowWidth = TB_DROPDOWNARROWWIDTH;

    // set defaults if image or text is needed but empty
    nDefWidth  = GetDefaultImageSize().Width();
    nDefHeight = GetDefaultImageSize().Height();

    mnWinHeight = 0;
    // determine minimum size necessary in NWF
    {
        Rectangle aRect( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        Rectangle aReg( aRect );
        ImplControlValue aVal;
        Rectangle aNativeBounds, aNativeContent;
        if( IsNativeControlSupported( ControlType::Toolbar, ControlPart::Button ) )
        {
            if( GetNativeControlRegion( ControlType::Toolbar, ControlPart::Button,
                                        aReg,
                                        ControlState::ENABLED | ControlState::ROLLOVER,
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
        if( GetNativeControlRegion( ControlType::Combobox, ControlPart::Entire,
                                    aReg,
                                    ControlState::ENABLED | ControlState::ROLLOVER,
                                    aVal, OUString(),
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
        aRect = Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( ControlType::Listbox, ControlPart::Entire,
                                    aReg,
                                    ControlState::ENABLED | ControlState::ROLLOVER,
                                    aVal, OUString(),
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
        aRect = Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( ControlType::Spinbox, ControlPart::Entire,
                                    aReg,
                                    ControlState::ENABLED | ControlState::ROLLOVER,
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
            it->mbVisibleText = false;  // indicates if text will definitely be drawn, influences dropdown pos

            if ( it->meType == ToolBoxItemType::BUTTON )
            {
                bool bImage;
                bool bText;

                // check if image and/or text exists
                if ( !(it->maImage) )
                    bImage = false;
                else
                    bImage = true;
                if ( it->maText.isEmpty() )
                    bText = false;
                else
                    bText = true;
                ButtonType tmpButtonType = determineButtonType( &(*it), meButtonType ); // default to toolbox setting
                if ( bImage || bText )
                {

                    it->mbEmptyBtn = false;

                    if ( tmpButtonType == ButtonType::SYMBOLONLY )
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
                            it->mbVisibleText = true;
                        }
                    }
                    else if ( tmpButtonType == ButtonType::TEXT )
                    {
                        // we're drawing text only
                        if ( bText || !bImage )
                        {
                            it->maItemSize = Size( GetCtrlTextWidth( it->maText )+TB_TEXTOFFSET,
                                                   GetTextHeight() );
                            it->mbVisibleText = true;
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

                        if ( meTextPosition == ToolBoxTextPosition::Right )
                        {
                            // leave space between image and text
                            if( bText )
                                it->maItemSize.Width() += TB_IMAGETEXTOFFSET;

                            // image and text side by side
                            it->maItemSize.Width() += it->maImage.GetSizePixel().Width();
                            if ( it->maImage.GetSizePixel().Height() > it->maItemSize.Height() )
                                it->maItemSize.Height() = it->maImage.GetSizePixel().Height();
                        }
                        else
                        {
                            // leave space between image and text
                            if( bText )
                                it->maItemSize.Height() += TB_IMAGETEXTOFFSET;

                            // text below image
                            it->maItemSize.Height() += it->maImage.GetSizePixel().Height();
                            if ( it->maImage.GetSizePixel().Width() > it->maItemSize.Width() )
                                it->maItemSize.Width() = it->maImage.GetSizePixel().Width();
                        }

                        it->mbVisibleText = bText;
                    }
                }
                else
                {   // no image and no text
                    it->maItemSize = Size( nDefWidth, nDefHeight );
                    it->mbEmptyBtn = true;
                }

                // save the content size
                it->maContentSize = it->maItemSize;

                // if required, take window height into consideration
                if ( it->mpWindow )
                {
                    long nHeight = it->mpWindow->GetSizePixel().Height();
                    if ( nHeight > mnWinHeight )
                        mnWinHeight = nHeight;
                }

                // add in drop down arrow
                if( it->mnBits & ToolBoxItemBits::DROPDOWN )
                {
                    if ( meTextPosition == ToolBoxTextPosition::Right )
                    {
                        it->maItemSize.Width() += nDropDownArrowWidth;
                        it->mnDropDownArrowWidth = nDropDownArrowWidth;
                    }
                    else
                    {
                        it->maItemSize.Height() += nDropDownArrowWidth;
                        it->mnDropDownArrowWidth = nDropDownArrowWidth;
                    }
                }

                // text items will be rotated in vertical mode
                // -> swap width and height
                if( it->mbVisibleText && !mbHorz )
                {
                    long tmp = it->maItemSize.Width();
                    it->maItemSize.Width() = it->maItemSize.Height();
                    it->maItemSize.Height() = tmp;

                    tmp = it->maContentSize.Width();
                    it->maContentSize.Width() = it->maContentSize.Height();
                    it->maContentSize.Height() = tmp;
                }
            }
            else if ( it->meType == ToolBoxItemType::SPACE )
            {
                it->maItemSize = Size( nDefWidth, nDefHeight );
                it->maContentSize = it->maItemSize;
            }

            if ( it->meType == ToolBoxItemType::BUTTON || it->meType == ToolBoxItemType::SPACE )
            {
                // add borders
                ImplAddButtonBorder( it->maItemSize.Width(), it->maItemSize.Height(), mpData->mbNativeButtons );

                if( it->meType == ToolBoxItemType::BUTTON )
                {
                    long nMinW = std::max(nMinWidth, it->maMinimalItemSize.Width());
                    long nMinH = std::max(nMinHeight, it->maMinimalItemSize.Height());

                    long nGrowContentWidth = 0;
                    long nGrowContentHeight = 0;

                    if( it->maItemSize.Width() < nMinW )
                    {
                        nGrowContentWidth = nMinW - it->maItemSize.Width();
                        it->maItemSize.Width() = nMinW;
                    }
                    if( it->maItemSize.Height() < nMinH )
                    {
                        nGrowContentHeight = nMinH - it->maItemSize.Height();
                        it->maItemSize.Height() = nMinH;
                    }

                    // grow the content size by the additional available space
                    it->maContentSize.Width() += nGrowContentWidth;
                    it->maContentSize.Height() += nGrowContentHeight;
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

        ImplAddButtonBorder( nMaxWidth, nMaxHeight, mpData->mbNativeButtons );
    }

    if( !ImplIsFloatingMode() && GetToolboxButtonSize() != ToolBoxButtonSize::DontCare
        && ( meTextPosition == ToolBoxTextPosition::Right ) )
    {
        // make sure all vertical toolbars have the same width and horizontal have the same height
        // this depends on the used button sizes
        // as this is used for alignement of multiple toolbars
        // it is only required for docked toolbars

        long nFixedWidth = nDefWidth+nDropDownArrowWidth;
        long nFixedHeight = nDefHeight;
        ImplAddButtonBorder( nFixedWidth, nFixedHeight, mpData->mbNativeButtons );

        if( mbHorz )
            nMaxHeight = nFixedHeight;
        else
            nMaxWidth = nFixedWidth;
    }

    mbCalc = false;
    mbFormat = true;

    // do we have to recalc the sizes ?
    if ( (nMaxWidth != mnMaxItemWidth) || (nMaxHeight != mnMaxItemHeight) )
    {
        mnMaxItemWidth  = nMaxWidth;
        mnMaxItemHeight = nMaxHeight;

        return true;
    }
    else
        return false;
}

sal_uInt16 ToolBox::ImplCalcBreaks( long nWidth, long* pMaxLineWidth, bool bCalcHorz )
{
    sal_uLong           nLineStart = 0;
    sal_uLong           nGroupStart = 0;
    long            nLineWidth = 0;
    long            nCurWidth;
    long            nLastGroupLineWidth = 0;
    long            nMaxLineWidth = 0;
    sal_uInt16          nLines = 1;
    bool            bWindow;
    bool            bBreak = false;
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
        bBreak = false;

        if ( it->mbVisible )
        {
            bWindow     = false;
            bBreak      = false;
            nCurWidth   = 0;

            if ( it->meType == ToolBoxItemType::BUTTON || it->meType == ToolBoxItemType::SPACE )
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
                        bWindow   = true;
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
                    bBreak = true;
            }
            else if ( it->meType == ToolBoxItemType::SEPARATOR )
            {
                nCurWidth = it->mnSepSize;
                if ( !ImplIsFloatingMode() && ( it != lastVisible ) && (nLineWidth+nCurWidth+nMenuWidth > nWidthTotal) )
                    bBreak = true;
            }
            // treat breaks as separators, except when using old style toolbars (ie. no menu button)
            else if ( (it->meType == ToolBoxItemType::BREAK) && !IsMenuEnabled() )
                bBreak = true;

            if ( bBreak )
            {
                nLines++;

                // Add break before the entire group or take group apart?
                if ( (it->meType == ToolBoxItemType::BREAK) ||
                     (nLineStart == nGroupStart) )
                {
                    if ( nLineWidth > nMaxLineWidth )
                        nMaxLineWidth = nLineWidth;

                    nLineWidth = 0;
                    nLineStart = it - mpData->m_aItems.begin();
                    nGroupStart = nLineStart;
                    it->mbBreak = true;
                    bBreak = false;
                }
                else
                {
                    if ( nLastGroupLineWidth > nMaxLineWidth )
                        nMaxLineWidth = nLastGroupLineWidth;

                    // if the break is added before the group, set it to
                    // beginning of line and re-calculate
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
                    if ( (it->meType != ToolBoxItemType::BUTTON) || bWindow )
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

Size ToolBox::ImplGetOptimalFloatingSize()
{
    if( !ImplIsFloatingMode() )
        return Size();

    Size aCurrentSize( mnDX, mnDY );
    Size aSize1( aCurrentSize );
    Size aSize2( aCurrentSize );

    // try to preserve current height

    // calc number of floating lines for current window height
    sal_uInt16 nFloatLinesHeight = ImplCalcLines( this, mnDY );
    // calc window size according to this number
    aSize1 = ImplCalcFloatSize( this, nFloatLinesHeight );

    if( aCurrentSize == aSize1 )
        return aSize1;

    // try to preserve current width

    long nLineHeight = ( mnWinHeight > mnMaxItemHeight ) ? mnWinHeight : mnMaxItemHeight;
    int nBorderX = 2*TB_BORDER_OFFSET1 + mnLeftBorder + mnRightBorder;
    int nBorderY = 2*TB_BORDER_OFFSET2 + mnTopBorder + mnBottomBorder;
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

    if( aCurrentSize == aSize2 )
        return aSize2;

    // set the size with the smallest delta as the current size
    long dx1 = std::abs( mnDX - aSize1.Width() );
    long dy1 = std::abs( mnDY - aSize1.Height() );

    long dx2 = std::abs( mnDX - aSize2.Width() );
    long dy2 = std::abs( mnDY - aSize2.Height() );

    if( dx1*dy1 < dx2*dy2 )
        aCurrentSize = aSize1;
    else
        aCurrentSize = aSize2;

    return aCurrentSize;
}

namespace
{
void lcl_hideDoubleSeparators( std::vector< ImplToolItem >& rItems )
{
    bool bLastSep( true );
    std::vector< ImplToolItem >::iterator it;
    for ( it = rItems.begin(); it != rItems.end(); ++it )
    {
        if ( it->meType == ToolBoxItemType::SEPARATOR )
        {
            it->mbVisible = false;
            if ( !bLastSep )
            {
                // check if any visible items have to appear behind it
                std::vector< ImplToolItem >::iterator temp_it;
                for ( temp_it = it+1; temp_it != rItems.end(); ++temp_it )
                {
                    if ( ((temp_it->meType == ToolBoxItemType::BUTTON) &&
                          temp_it->mbVisible) )
                    {
                        it->mbVisible = true;
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

void ToolBox::ImplFormat( bool bResize )
{
    // Has to re-formatted
    if ( !mbFormat )
        return;

    mpData->ImplClearLayoutData();

    // recalculate positions and sizes
    Rectangle       aEmptyRect;
    long            nLineSize;
    long            nLeft;
    long            nTop;
    long            nMax;   // width of layoutarea in pixels
    sal_uInt16          nFormatLine;
    bool            bMustFullPaint;

    std::vector< ImplToolItem >::iterator   it;

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    bool bIsInPopupMode = ImplIsInPopupMode();

    maFloatSizes.clear();

    // compute border sizes
    ImplCalcBorder( meAlign, mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder, this );

    // update drag area (where the 'grip' will be placed)
    Rectangle aOldDragRect;
    if( pWrapper )
        aOldDragRect = pWrapper->GetDragArea();
    ImplUpdateDragArea( this );

    if ( ImplCalcItem() )
        bMustFullPaint = true;
    else
        bMustFullPaint = false;

    // calculate new size during interactive resize or
    // set computed size when formatting only
    if ( ImplIsFloatingMode() )
    {
        if ( bResize )
            mnFloatLines = ImplCalcLines( this, mnDY );
        else
            SetOutputSizePixel( ImplGetOptimalFloatingSize() );
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
        Rectangle aOldMenubuttonRect = mpData->maMenubuttonItem.maRect;
        maUpperRect = aEmptyRect;
        maLowerRect = aEmptyRect;
        mpData->maMenubuttonItem.maRect = aEmptyRect;

        // do we have any toolbox items at all ?
        if ( !mpData->m_aItems.empty() || IsMenuEnabled() )
        {
            lcl_hideDoubleSeparators( mpData->m_aItems );

            // compute line breaks and visible lines give the current window width (nMax)
            // the break indicators will be stored within each item (it->mbBreak)
            mnCurLines = ImplCalcBreaks( nMax, nullptr, mbHorz );

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
                    mnCurLines = ImplCalcBreaks( nMax, nullptr, mbHorz );
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
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY-mnBottomBorder-TB_BORDER_OFFSET2-1;
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.Right() = mnDX - mnRightBorder-TB_BORDER_OFFSET1-1;
                            mpData->maMenubuttonItem.maRect.Top() = nTop;
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY-mnBottomBorder-TB_BORDER_OFFSET2-1;
                        }
                        mpData->maMenubuttonItem.maRect.Left() = mpData->maMenubuttonItem.maRect.Right() - mpData->mnMenuButtonWidth;
                    }
                    else
                    {
                        maUpperRect.Left()   = nLeft+nMax+TB_SPIN_OFFSET;
                        maUpperRect.Right()  = maUpperRect.Left()+TB_SPIN_SIZE-1;
                        maUpperRect.Top()    = nTop;
                        maLowerRect.Bottom() = mnDY-mnBottomBorder-TB_BORDER_OFFSET2-1;
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
                            mpData->maMenubuttonItem.maRect.Right() = mnDX-mnRightBorder-TB_BORDER_OFFSET2-1;
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.Bottom() = mnDY - mnBottomBorder-TB_BORDER_OFFSET1-1;
                            mpData->maMenubuttonItem.maRect.Left() = nLeft;
                            mpData->maMenubuttonItem.maRect.Right() = mnDX-mnRightBorder-TB_BORDER_OFFSET2-1;
                        }
                        mpData->maMenubuttonItem.maRect.Top() = mpData->maMenubuttonItem.maRect.Bottom() - mpData->mnMenuButtonWidth;
                    }
                    else
                    {
                        maUpperRect.Top()    = nTop+nMax+TB_SPIN_OFFSET;
                        maUpperRect.Bottom() = maUpperRect.Top()+TB_SPIN_SIZE-1;
                        maUpperRect.Left()   = nLeft;
                        maLowerRect.Right()  = mnDX-mnRightBorder-TB_BORDER_OFFSET2-1;
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
                it->mbShowWindow = false;

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
                        // if special ToolBoxLayoutMode::LockVert lock vertical position
                        // don't recalculate the vertical position of the item
                        if ( meLayoutMode == ToolBoxLayoutMode::LockVert && mnLines == 1 )
                        {
                            // Somewhat of a hack here, calc deletes and re-adds
                            // the sum/assign & ok/cancel items dynamically.
                            // Because ToolBoxLayoutMode::LockVert effectively prevents
                            // recalculation of the vertical pos of an item the
                            // it->maRect.Top() for those newly added items is
                            // 0. The hack here is that we want to effectively
                            // recalculate the vertical pos for those added
                            // items here. ( Note: assume mnMaxItemHeight is
                            // equal to the LineSize when multibar has a single
                            // line size )
                            if ( it->maRect.Top() ||
                                 (it->mpWindow && it->mpWindow->GetType() == WINDOW_CALCINPUTLINE) ) // tdf#83099
                            {
                                it->maCalcRect.Top()  = it->maRect.Top();
                            }
                            else
                            {
                                it->maCalcRect.Top()  = nY+(mnMaxItemHeight-aCurrentItemSize.Height())/2;
                            }
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

                        assert( it->maCalcRect.Top() >= 0 );

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
        it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            it->maRect = it->maCalcRect;
            ++it;
        }
    }

    // indicate formatting is done
    mbFormat = false;
}

IMPL_LINK_NOARG(ToolBox, ImplDropdownLongClickHdl, Timer *, void)
{
    if (mnCurPos != TOOLBOX_ITEM_NOTFOUND &&
        (mpData->m_aItems[ mnCurPos ].mnBits & ToolBoxItemBits::DROPDOWN))
    {
        mpData->mbDropDownByKeyboard = false;
        mpData->maDropdownClickHdl.Call( this );

        // do not reset data if the dropdown handler opened a floating window
        // see ImplFloatControl()
        if( !mpFloatWin )
        {
            // no floater was opened
            Deactivate();
            InvalidateItem(mnCurPos);

            mnCurPos         = TOOLBOX_ITEM_NOTFOUND;
            mnCurItemId      = 0;
            mnDownItemId     = 0;
            mnMouseClicks    = 0;
            mnMouseModifier  = 0;
            mnHighItemId     = 0;
        }
    }
}

IMPL_LINK_NOARG(ToolBox, ImplUpdateHdl, Timer *, void)
{

    if( mbFormat && mpData )
        ImplFormat();
}

static void ImplDrawMoreIndicator(vcl::RenderContext& rRenderContext, const Rectangle& rRect, bool bSetColor, bool bRotate )
{
    rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
    rRenderContext.SetLineColor();

    if (bSetColor)
    {
        if (rRenderContext.GetSettings().GetStyleSettings().GetFaceColor().IsDark())
            rRenderContext.SetFillColor(Color(COL_WHITE));
        else
            rRenderContext.SetFillColor(Color(COL_BLACK));
    }
    float fScaleFactor = rRenderContext.GetDPIScaleFactor();

    int linewidth = 1 * fScaleFactor;
    int space = 4 * fScaleFactor;

    if( !bRotate )
    {
        long width = 8 * fScaleFactor;
        long height = 5 * fScaleFactor;

        //Keep odd b/c drawing code works better
        if ( height % 2 == 0 )
            height--;

        long heightOrig = height;

        long x = rRect.Left() + (rRect.getWidth() - width)/2 + 1;
        long y = rRect.Top() + (rRect.getHeight() - height)/2 + 1;
        while( height >= 1)
        {
            rRenderContext.DrawRect( Rectangle( x, y, x + linewidth, y ) );
            x += space;
            rRenderContext.DrawRect( Rectangle( x, y, x + linewidth, y ) );
            x -= space;
            y++;
            if( height <= heightOrig / 2 + 1) x--;
            else            x++;
            height--;
        }
    }
    else
    {
        long width = 5 * fScaleFactor;
        long height = 8 * fScaleFactor;

        //Keep odd b/c drawing code works better
        if (width % 2 == 0)
            width--;

        long widthOrig = width;

        long x = rRect.Left() + (rRect.getWidth() - width)/2 + 1;
        long y = rRect.Top() + (rRect.getHeight() - height)/2 + 1;
        while( width >= 1)
        {
            rRenderContext.DrawRect( Rectangle( x, y, x, y + linewidth ) );
            y += space;
            rRenderContext.DrawRect( Rectangle( x, y, x, y + linewidth ) );
            y -= space;
            x++;
            if( width <= widthOrig / 2 + 1) y--;
            else           y++;
            width--;
        }
    }

    rRenderContext.Pop();
}

static void ImplDrawDropdownArrow(vcl::RenderContext& rRenderContext, const Rectangle& rDropDownRect, bool bSetColor, bool bRotate )
{
    bool bLineColor = rRenderContext.IsLineColor();
    bool bFillColor = rRenderContext.IsFillColor();
    Color aOldFillColor = rRenderContext.GetFillColor();
    Color aOldLineColor = rRenderContext.GetLineColor();
    rRenderContext.SetLineColor();

    if ( bSetColor )
    {
        if (rRenderContext.GetSettings().GetStyleSettings().GetFaceColor().IsDark())
            rRenderContext.SetFillColor(Color(COL_WHITE));
        else
            rRenderContext.SetFillColor(Color(COL_BLACK));
    }

    float fScaleFactor = rRenderContext.GetDPIScaleFactor();

    if( !bRotate )
    {
        long width = 5 * fScaleFactor;
        long height = 3 * fScaleFactor;

        long x = rDropDownRect.Left() + (rDropDownRect.getWidth() - width)/2;
        long y = rDropDownRect.Top() + (rDropDownRect.getHeight() - height)/2;
        while( width >= 1)
        {
            rRenderContext.DrawRect( Rectangle( x, y, x+width-1, y ) );
            y++;
            x++;
            width -= 2;
        }
    }
    else
    {
        long width = 3 * fScaleFactor;
        long height = 5 * fScaleFactor;

        long x = rDropDownRect.Left() + (rDropDownRect.getWidth() - width)/2;
        long y = rDropDownRect.Top() + (rDropDownRect.getHeight() - height)/2;
        while( height >= 1)
        {
            rRenderContext.DrawRect( Rectangle( x, y, x, y+height-1 ) );
            y++;
            x++;
            height -= 2;
        }
    }

    if( bFillColor )
        rRenderContext.SetFillColor(aOldFillColor);
    else
        rRenderContext.SetFillColor();
    if( bLineColor )
        rRenderContext.SetLineColor(aOldLineColor);
    else
        rRenderContext.SetLineColor();
}

void ToolBox::ImplDrawMenuButton(vcl::RenderContext& rRenderContext, bool bHighlight)
{
    if (!mpData->maMenubuttonItem.maRect.IsEmpty())
    {
        // #i53937# paint menu button only if necessary
        if (!ImplHasClippedItems())
            return;

        // execute pending paint requests
        ImplCheckUpdate(this);

        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);

        // draw the 'more' indicator / button (>>)
        ImplErase(rRenderContext, mpData->maMenubuttonItem.maRect, bHighlight);

        if (bHighlight)
            ImplDrawButton(rRenderContext, mpData->maMenubuttonItem.maRect, 2, false, true, false );

        if (ImplHasClippedItems())
            ImplDrawMoreIndicator(rRenderContext, mpData->maMenubuttonItem.maRect, true, !mbHorz);

        // store highlight state
        mpData->mbMenubuttonSelected = bHighlight;

        // restore colors
        rRenderContext.Pop();
    }
}

void ToolBox::ImplDrawSpin(vcl::RenderContext& rRenderContext)
{
    bool    bTmpUpper;
    bool    bTmpLower;

    if ( maUpperRect.IsEmpty() || maLowerRect.IsEmpty() )
        return;

    if ( mnCurLine > 1 )
        bTmpUpper = true;
    else
        bTmpUpper = false;

    if ( mnCurLine+mnVisLines-1 < mnCurLines )
        bTmpLower = true;
    else
        bTmpLower = false;

    if ( !IsEnabled() )
    {
        bTmpUpper = false;
        bTmpLower = false;
    }

    ImplDrawUpDownButtons(rRenderContext, maUpperRect, maLowerRect,
                          false/*bUpperIn*/, false/*bLowerIn*/, bTmpUpper, bTmpLower, !mbHorz);
}

void ToolBox::ImplDrawSeparator(vcl::RenderContext& rRenderContext, sal_uInt16 nPos, const Rectangle& rRect)
{
    if ( nPos >= mpData->m_aItems.size() - 1 )
        // no separator if it's the last item
        return;

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    ImplToolItem* pPreviousItem = &mpData->m_aItems[nPos-1];
    ImplToolItem* pNextItem = &mpData->m_aItems[nPos+1];

    if ( ( pPreviousItem->mbShowWindow && pNextItem->mbShowWindow ) || pNextItem->mbBreak )
        // no separator between two windows or before a break
        return;

    bool bNativeOk = false;
    ControlPart nPart = IsHorizontal() ? ControlPart::SeparatorVert : ControlPart::SeparatorHorz;
    if (rRenderContext.IsNativeControlSupported(ControlType::Toolbar, nPart))
    {
        ImplControlValue aControlValue;
        ControlState     nState = ControlState::NONE;
        bNativeOk = rRenderContext.DrawNativeControl(ControlType::Toolbar, nPart, rRect, nState, aControlValue, OUString());
    }

    /* Draw the widget only if it can't be drawn natively. */
    if (!bNativeOk)
    {
        long nCenterPos, nSlim;
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        rRenderContext.SetLineColor(rStyleSettings.GetSeparatorColor());
        if (IsHorizontal())
        {
            nSlim = (pItem->maRect.Bottom() - pItem->maRect.Top ()) / 4;
            nCenterPos = pItem->maRect.Center().X();
            rRenderContext.DrawLine(Point(nCenterPos, pItem->maRect.Top() + nSlim),
                                    Point(nCenterPos, pItem->maRect.Bottom() - nSlim));
        }
        else
        {
            nSlim = (pItem->maRect.Right() - pItem->maRect.Left ()) / 4;
            nCenterPos = pItem->maRect.Center().Y();
            rRenderContext.DrawLine(Point(pItem->maRect.Left() + nSlim, nCenterPos),
                                    Point(pItem->maRect.Right() - nSlim, nCenterPos));
        }
    }
}

void ToolBox::ImplDrawButton(vcl::RenderContext& rRenderContext, const Rectangle &rRect, sal_uInt16 highlight,
                             bool bChecked, bool bEnabled, bool bIsWindow )
{
    // draws toolbar button background either native or using a coloured selection
    // if bIsWindow is true, the corresponding item is a control and only a selection border will be drawn

    bool bNativeOk = false;
    if( !bIsWindow && rRenderContext.IsNativeControlSupported( ControlType::Toolbar, ControlPart::Button ) )
    {
        ImplControlValue    aControlValue;
        ControlState        nState = ControlState::NONE;

        if ( highlight == 1 )   nState |= ControlState::PRESSED;
        if ( highlight == 2 )     nState |= ControlState::ROLLOVER;
        if ( bEnabled )         nState |= ControlState::ENABLED;

        aControlValue.setTristateVal( bChecked ? ButtonValue::On : ButtonValue::Off );

        bNativeOk = rRenderContext.DrawNativeControl( ControlType::Toolbar, ControlPart::Button,
                                              rRect, nState, aControlValue, OUString() );
    }

    if (!bNativeOk)
        vcl::RenderTools::DrawSelectionBackground(rRenderContext, *this, rRect, bIsWindow ? 3 : highlight,
                                                  bChecked, true, bIsWindow, nullptr, 2);
}

void ToolBox::ImplDrawItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos, sal_uInt16 nHighlight)
{
    if (nPos >= mpData->m_aItems.size())
        return;

    // execute pending paint requests
    ImplCheckUpdate(this);

    ImplDisableFlatButtons();

    rRenderContext.SetFillColor();

    ImplToolItem* pItem = &mpData->m_aItems[nPos];

    if (!pItem->mbEnabled)
        nHighlight = 0;

    // if the rectangle is outside visible area
    if (pItem->maRect.IsEmpty())
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    // no gradient background for items that have a popup open
    bool bHasOpenPopup = mpFloatWin && (mnDownItemId==pItem->mnId);

    bool bHighContrastWhite = false;
    // check the face color as highcontrast indicator
    // because the toolbox itself might have a gradient
    if (rStyleSettings.GetFaceColor() == Color(COL_WHITE))
        bHighContrastWhite = true;

    // Compute buttons area.
    Size    aBtnSize    = pItem->maRect.GetSize();
    if( ImplGetSVData()->maNWFData.mbToolboxDropDownSeparate )
    {
        // separate button not for dropdown only where the whole button is painted
        // exception: when text position is set to bottom then we want to calculate rect for dropdown only button
        if ( ( pItem->mnBits & ToolBoxItemBits::DROPDOWN &&
            ((pItem->mnBits & ToolBoxItemBits::DROPDOWNONLY) != ToolBoxItemBits::DROPDOWNONLY) )
            || ( ( pItem->mnBits & ToolBoxItemBits::DROPDOWN) && ( meTextPosition == ToolBoxTextPosition::Bottom ) ) )
        {
            Rectangle aArrowRect = pItem->GetDropDownRect( mbHorz && ( meTextPosition == ToolBoxTextPosition::Right ) );
            if( aArrowRect.Top() == pItem->maRect.Top() ) // dropdown arrow on right side
                aBtnSize.Width() -= aArrowRect.GetWidth();
            else if ( !( (meTextPosition == ToolBoxTextPosition::Bottom)
                        && ((pItem->mnBits & ToolBoxItemBits::DROPDOWNONLY) == ToolBoxItemBits::DROPDOWNONLY) ) )
                aBtnSize.Height() -= aArrowRect.GetHeight(); // dropdown arrow on bottom side
        }
    }

    /* Compute the button/separator rectangle here, we'll need it for
     * both the buttons and the separators. */
    Rectangle aButtonRect( pItem->maRect.TopLeft(), aBtnSize );
    long    nOffX       = SMALLBUTTON_OFF_NORMAL_X;
    long    nOffY       = SMALLBUTTON_OFF_NORMAL_Y;
    long    nImageOffX  = 0;
    long    nImageOffY  = 0;
    DrawButtonFlags nStyle      = DrawButtonFlags::NONE;

    // draw separators in flat style only
    if ( (mnOutStyle & TOOLBOX_STYLE_FLAT) &&
         (pItem->meType == ToolBoxItemType::SEPARATOR) &&
         nPos > 0
         )
    {
        ImplDrawSeparator(rRenderContext, nPos, aButtonRect);
    }

    // do nothing if item is no button or will be displayed as window
    if ( (pItem->meType != ToolBoxItemType::BUTTON) ||
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
        pMgr = nullptr;

    // during configuration mode visible windows will be drawn in a special way
    if ( mbCustomizeMode && pItem->mbShowWindow )
    {
        vcl::Font aOldFont = rRenderContext.GetFont();
        Color     aOldTextColor = rRenderContext.GetTextColor();

        SetZoomedPointFont(rRenderContext, rStyleSettings.GetAppFont());
        rRenderContext.SetLineColor(Color(COL_BLACK));
        rRenderContext.SetFillColor(rStyleSettings.GetFieldColor());
        rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
        rRenderContext.DrawRect(pItem->maRect);

        Size aSize( GetCtrlTextWidth( pItem->maText ), GetTextHeight() );
        Point aPos( pItem->maRect.Left()+2, pItem->maRect.Top() );
        aPos.Y() += (pItem->maRect.GetHeight()-aSize.Height())/2;
        bool bClip;
        if ( (aSize.Width() > pItem->maRect.GetWidth()-2) ||
             (aSize.Height() > pItem->maRect.GetHeight()-2) )
        {
            bClip = true;
            Rectangle aTempRect(pItem->maRect.Left() + 1, pItem->maRect.Top() + 1,
                                pItem->maRect.Right() - 1, pItem->maRect.Bottom() - 1);
            vcl::Region aTempRegion(aTempRect);
            rRenderContext.SetClipRegion(aTempRegion);
        }
        else
            bClip = false;
        rRenderContext.DrawCtrlText( aPos, pItem->maText, 0, pItem->maText.getLength() );
        if (bClip)
            rRenderContext.SetClipRegion();
        rRenderContext.SetFont(aOldFont);
        rRenderContext.SetTextColor(aOldTextColor);

        // draw Config-Frame if required
        if (pMgr)
            pMgr->UpdateDragRect();
        return;
    }

    if ( pItem->meState == TRISTATE_TRUE )
    {
        nStyle |= DrawButtonFlags::Checked;
    }
    else if ( pItem->meState == TRISTATE_INDET )
    {
        nStyle |= DrawButtonFlags::DontKnow;
    }
    if ( nHighlight == 1 )
    {
        nStyle |= DrawButtonFlags::Pressed;
    }

    if ( mnOutStyle & TOOLBOX_STYLE_FLAT )
    {
        ImplErase(rRenderContext, pItem->maRect, nHighlight != 0, bHasOpenPopup );
    }
    else
    {
        DecorationView aDecoView(&rRenderContext);
        aDecoView.DrawButton(aButtonRect, nStyle);
    }

    nOffX += pItem->maRect.Left();
    nOffY += pItem->maRect.Top();

    // determine what has to be drawn on the button: image, text or both
    bool bImage;
    bool bText;
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

    if ( bImage )
    {
        const Image* pImage = &(pItem->maImage);
        aImageSize = pImage->GetSizePixel();

        // determine drawing flags
        DrawImageFlags nImageStyle = DrawImageFlags::NONE;

        if ( !pItem->mbEnabled || !IsEnabled() )
            nImageStyle |= DrawImageFlags::Disable;

        // #i35563# the dontknow state indicates different states at the same time
        // which should not be rendered disabled but normal

        // draw the image
        nImageOffX = nOffX;
        nImageOffY = nOffY;
        if ( ( (pItem->mnBits & (ToolBoxItemBits::LEFT|ToolBoxItemBits::DROPDOWN)) || bText )
            && ( meTextPosition == ToolBoxTextPosition::Right ) )
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
            if ( meTextPosition == ToolBoxTextPosition::Right )
                nImageOffY += (nBtnHeight-aImageSize.Height())/2;
        }
        if ( nHighlight != 0 || (pItem->meState == TRISTATE_TRUE) )
        {
            if( bHasOpenPopup )
                ImplDrawFloatwinBorder(rRenderContext, pItem);
            else
                ImplDrawButton(rRenderContext, aButtonRect, nHighlight, pItem->meState == TRISTATE_TRUE,
                               pItem->mbEnabled && IsEnabled(), pItem->mbShowWindow);

            if( nHighlight != 0 )
            {
                if( bHighContrastWhite )
                    nImageStyle |= DrawImageFlags::ColorTransform;
            }
        }
        rRenderContext.DrawImage(Point( nImageOffX, nImageOffY ), *pImage, nImageStyle);
    }

    // draw the text
    bool bRotate = false;
    if ( bText )
    {
        long nTextOffX = nOffX;
        long nTextOffY = nOffY;

        // rotate text when vertically docked
        vcl::Font aOldFont = rRenderContext.GetFont();
        if( pItem->mbVisibleText && !ImplIsFloatingMode() &&
            ((meAlign == WindowAlign::Left) || (meAlign == WindowAlign::Right)) )
        {
            bRotate = true;

            vcl::Font aRotateFont = aOldFont;
            aRotateFont.SetOrientation( 2700 );

            // center horizontally
            nTextOffX += aTxtSize.Height();
            nTextOffX += (nBtnWidth-aTxtSize.Height())/2;

            // add in image offset
            if( bImage )
                nTextOffY = nImageOffY + aImageSize.Height() + TB_IMAGETEXTOFFSET;

            rRenderContext.SetFont(aRotateFont);
        }
        else
        {
            if ( meTextPosition == ToolBoxTextPosition::Right )
            {
                // center vertically
                nTextOffY += (nBtnHeight-aTxtSize.Height())/2;

                // add in image offset
                if( bImage )
                    nTextOffX = nImageOffX + aImageSize.Width() + TB_IMAGETEXTOFFSET;
            }
            else
            {
                long nArrowHeight = ( pItem->mnBits & ToolBoxItemBits::DROPDOWN )
                                        ? TB_DROPDOWNARROWWIDTH : 0;

                // only if button is a "dropdown only" type then is painted as a single button
                // and we need to move text above the arrow
                if ( ImplGetSVData()->maNWFData.mbToolboxDropDownSeparate
                    && (pItem->mnBits & ToolBoxItemBits::DROPDOWNONLY) != ToolBoxItemBits::DROPDOWNONLY )
                    nArrowHeight = 0;

                nTextOffY += nBtnHeight - aTxtSize.Height() - nArrowHeight;
            }
        }

        // draw selection only if not already drawn during image output (see above)
        if ( !bImage && (nHighlight != 0 || (pItem->meState == TRISTATE_TRUE) ) )
        {
            if( bHasOpenPopup )
                ImplDrawFloatwinBorder(rRenderContext, pItem);
            else
                ImplDrawButton(rRenderContext, pItem->maRect, nHighlight, pItem->meState == TRISTATE_TRUE,
                               pItem->mbEnabled && IsEnabled(), pItem->mbShowWindow );
        }

        DrawTextFlags nTextStyle = DrawTextFlags::NONE;
        if ( !pItem->mbEnabled )
            nTextStyle |= DrawTextFlags::Disable;
        rRenderContext.DrawCtrlText( Point( nTextOffX, nTextOffY ), pItem->maText,
                      0, pItem->maText.getLength(), nTextStyle );
        if ( bRotate )
            SetFont( aOldFont );
    }

    // paint optional drop down arrow
    if ( pItem->mnBits & ToolBoxItemBits::DROPDOWN )
    {
        Rectangle aDropDownRect( pItem->GetDropDownRect( mbHorz && ( meTextPosition == ToolBoxTextPosition::Right ) ) );
        bool bSetColor = true;
        if ( !pItem->mbEnabled || !IsEnabled() )
        {
            bSetColor = false;
            rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
        }

        // dropdown only will be painted without inner border
        if( (pItem->mnBits & ToolBoxItemBits::DROPDOWNONLY) != ToolBoxItemBits::DROPDOWNONLY )
        {
            ImplErase(rRenderContext, aDropDownRect, nHighlight != 0, bHasOpenPopup);

            if( nHighlight != 0 || (pItem->meState == TRISTATE_TRUE) )
            {
                if( bHasOpenPopup )
                    ImplDrawFloatwinBorder(rRenderContext, pItem);
                else
                    ImplDrawButton(rRenderContext, aDropDownRect, nHighlight, pItem->meState == TRISTATE_TRUE,
                                   pItem->mbEnabled && IsEnabled(), false);
            }
        }
        ImplDrawDropdownArrow(rRenderContext, aDropDownRect, bSetColor, bRotate);
    }

    // draw config-frame if required
    if (pMgr)
        pMgr->UpdateDragRect();
}

void ToolBox::ImplDrawFloatwinBorder(vcl::RenderContext& rRenderContext, ImplToolItem* pItem)
{
    if ( !pItem->maRect.IsEmpty() )
    {
        Rectangle aRect( mpFloatWin->ImplGetItemEdgeClipRect() );
        aRect.SetPos( AbsoluteScreenToOutputPixel( aRect.TopLeft() ) );
        rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetShadowColor());
        Point p1, p2;

        p1 = pItem->maRect.TopLeft();
        p1.X()++;
        p2 = pItem->maRect.TopRight();
        p2.X()--;
        rRenderContext.DrawLine( p1, p2);
        p1 = pItem->maRect.BottomLeft();
        p1.X()++;
        p2 = pItem->maRect.BottomRight();
        p2.X()--;
        rRenderContext.DrawLine( p1, p2);

        p1 = pItem->maRect.TopLeft();
        p1.Y()++;
        p2 = pItem->maRect.BottomLeft();
        p2.Y()--;
        rRenderContext.DrawLine( p1, p2);
        p1 = pItem->maRect.TopRight();
        p1.Y()++;
        p2 = pItem->maRect.BottomRight();
        p2.Y()--;
        rRenderContext.DrawLine( p1, p2);
    }
}

void ToolBox::ImplFloatControl( bool bStart, FloatingWindow* pFloatWindow )
{

    if ( bStart )
    {
        mpFloatWin = pFloatWindow;

        // redraw item, to trigger drawing of a special border
        InvalidateItem(mnCurPos);

        mbDrag = false;
        EndTracking();
        if (IsMouseCaptured())
            ReleaseMouse();
    }
    else
    {
        mpFloatWin = nullptr;

        // if focus is still in this toolbox, then the floater was opened by keyboard
        // draw current item with highlight and keep old state
        bool bWasKeyboardActivate = mpData->mbDropDownByKeyboard;

        if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            InvalidateItem(mnCurPos);
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

void ToolBox::ShowLine( bool bNext )
{

    mbFormat = true;

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

bool ToolBox::ImplHandleMouseMove( const MouseEvent& rMEvt, bool bRepeat )
{
    Point aMousePos = rMEvt.GetPosPixel();

    if ( !mpData )
        return false;

    // ToolBox active?
    if ( mbDrag && mnCurPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // is the cursor over the item?
        ImplToolItem* pItem = &mpData->m_aItems[mnCurPos];
        if ( pItem->maRect.IsInside( aMousePos ) )
        {
            if ( !mnCurItemId )
            {
                InvalidateItem(mnCurPos);
                mnCurItemId = pItem->mnId;
                Highlight();
            }

            if ( (pItem->mnBits & ToolBoxItemBits::REPEAT) && bRepeat )
                Select();
        }
        else
        {
            if ( mnCurItemId )
            {
                InvalidateItem(mnCurPos);
                mnCurItemId = 0;
                InvalidateItem(mnCurPos);
                Highlight();
            }
        }

        return true;
    }

    if ( mbUpper )
    {
        bool bNewIn = maUpperRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            InvalidateSpin(true, false);
        }
        return true;
    }

    if ( mbLower )
    {
        bool bNewIn = maLowerRect.IsInside( aMousePos );
        if ( bNewIn != mbIn )
        {
            mbIn = bNewIn;
            InvalidateSpin(false);
        }
        return true;
    }

    return false;
}

bool ToolBox::ImplHandleMouseButtonUp( const MouseEvent& rMEvt, bool bCancel )
{
    ImplDisableFlatButtons();

    if ( !mpData )
        return false;

    // stop eventual running dropdown timer
    if( mnCurPos < mpData->m_aItems.size() &&
        (mpData->m_aItems[mnCurPos].mnBits & ToolBoxItemBits::DROPDOWN ) )
    {
        mpData->maDropdownTimer.Stop();
    }

    if ( mbDrag || mbSelection )
    {
        // set mouse data if in selection mode, as then
        // the MouseButtonDown handler cannot be called
        if ( mbSelection )
        {
            mnMouseClicks    = rMEvt.GetClicks();
            mnMouseModifier  = rMEvt.GetModifier();
        }

        Deactivate();

        if ( mbDrag )
            mbDrag = false;
        else
        {
            mbSelection = false;
            if ( mnCurPos == TOOLBOX_ITEM_NOTFOUND )
                return true;
        }

        // has mouse been released on top of item?
        if( mnCurPos < mpData->m_aItems.size() )
        {
            ImplToolItem* pItem = &mpData->m_aItems[mnCurPos];
            if ( pItem->maRect.IsInside( rMEvt.GetPosPixel() ) )
            {
                mnCurItemId = pItem->mnId;
                if ( !bCancel )
                {
                    // execute AutoCheck if required
                    if ( pItem->mnBits & ToolBoxItemBits::AUTOCHECK )
                    {
                        if ( pItem->mnBits & ToolBoxItemBits::RADIOCHECK )
                        {
                            if ( pItem->meState != TRISTATE_TRUE )
                                SetItemState( pItem->mnId, TRISTATE_TRUE );
                        }
                        else
                        {
                            if ( pItem->meState != TRISTATE_TRUE )
                                pItem->meState = TRISTATE_TRUE;
                            else
                                pItem->meState = TRISTATE_FALSE;
                        }
                    }

                    // do not call Select when Repeat is active, as in this
                    // case that was triggered already in MouseButtonDown
                    if ( !(pItem->mnBits & ToolBoxItemBits::REPEAT) )
                    {
                        // prevent from being destroyed in the select handler
                        VclPtr<vcl::Window> xWindow = this;
                        Select();
                        if ( xWindow->IsDisposed() )
                            return true;
                    }
                }

                {
                }

                // Items not destroyed, in Select handler
                if ( mnCurItemId )
                {
                    // Get current pos for the case that items are inserted/removed
                    // in the toolBox
                    mnCurPos = GetItemPos( mnCurItemId );
                    if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
                    {
                        InvalidateItem(mnCurPos);
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
        return true;
    }
    else if ( mbUpper || mbLower )
    {
        if ( mbIn )
            ShowLine( !mbUpper );
        mbUpper = false;
        mbLower = false;
        mbIn    = false;
        InvalidateSpin();
        return true;
    }

    return false;
}

void ToolBox::MouseMove( const MouseEvent& rMEvt )
{
    // pressing a modifier generates synthetic mouse moves
    // ignore it if keyboard selection is active
    if( HasFocus() && ( rMEvt.GetMode() & MouseEventModifiers::MODIFIERCHANGED ) )
        return;

    if ( ImplHandleMouseMove( rMEvt ) )
        return;

    ImplDisableFlatButtons();

    Point aMousePos = rMEvt.GetPosPixel();

    // only highlight when the focus is not inside a child window of a toolbox
    // eg, in a edit control
    // and do not highlight when focus is in a different toolbox
    bool bDrawHotSpot = true;
    vcl::Window *pWin = Application::GetFocusWindow();
    if( pWin && pWin->ImplGetWindowImpl()->mbToolBox && pWin != this )
        bDrawHotSpot = false;

    if ( mbSelection && bDrawHotSpot )
    {
        sal_uInt16  i = 0;
        sal_uInt16  nNewPos = TOOLBOX_ITEM_NOTFOUND;

        // search the item that has been clicked
        std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            // if the mouse position is in this item,
            // we can stop the search
            if ( it->maRect.IsInside( aMousePos ) )
            {
                // select it if it is a button
                if ( it->meType == ToolBoxItemType::BUTTON )
                {
                    // if button is disabled, do not
                    // change it
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

        // was a new entry selected?
        // don't change selection if keyboard selection is active and
        // mouse leaves the toolbox
        if ( nNewPos != mnCurPos && !( HasFocus() && nNewPos == TOOLBOX_ITEM_NOTFOUND ) )
        {
            if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            {
                InvalidateItem(mnCurPos);
                CallEventListeners( VclEventId::ToolboxHighlightOff, reinterpret_cast< void* >( mnCurPos ) );
            }

            mnCurPos = nNewPos;
            if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            {
                mnCurItemId = mnHighItemId = it->mnId;
                InvalidateItem(mnCurPos);
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

    PointerStyle eStyle = PointerStyle::Arrow;

    // change mouse cursor over drag area
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && pWrapper->GetDragArea().IsInside( rMEvt.GetPosPixel() ) )
        eStyle = PointerStyle::Move;

    if ( (mnWinStyle & TB_WBLINESIZING) == TB_WBLINESIZING )
    {
        if ( rMEvt.GetMode() & MouseEventModifiers::SIMPLEMOVE )
        {
            sal_uInt16 nLinePtr = ImplTestLineSize( this, rMEvt.GetPosPixel() );
            if ( nLinePtr & DOCK_LINEHSIZE )
            {
                if ( meAlign == WindowAlign::Left )
                    eStyle = PointerStyle::WindowESize;
                else
                    eStyle = PointerStyle::WindowWSize;
            }
            else if ( nLinePtr & DOCK_LINEVSIZE )
            {
                if ( meAlign == WindowAlign::Top )
                    eStyle = PointerStyle::WindowSSize;
                else
                    eStyle = PointerStyle::WindowNSize;
            }
        }
    }

    if ( (eStyle == PointerStyle::Arrow) && mbCustomizeMode )
    {
        // search the item which was clicked
        std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            // show resize pointer if it is a customize window
            if ( it->mbShowWindow )
            {
                if ( it->maRect.IsInside( aMousePos ) )
                {
                    if ( it->maRect.Right()-TB_RESIZE_OFFSET <= aMousePos.X() )
                        eStyle = PointerStyle::HSizeBar;
                    break;
                }
            }

            ++it;
        }
    }

    if ( bDrawHotSpot && ( (mnOutStyle & TOOLBOX_STYLE_FLAT) || !mnOutStyle ) )
    {
        bool bClearHigh = true;
        if ( !rMEvt.IsLeaveWindow() && (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
        {
            std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
            while ( it != mpData->m_aItems.end() )
            {
                if ( it->maRect.IsInside( aMousePos ) )
                {
                    if ( (it->meType == ToolBoxItemType::BUTTON) && it->mbEnabled )
                    {
                        if ( !mnOutStyle || (mnOutStyle & TOOLBOX_STYLE_FLAT) )
                        {
                            bClearHigh = false;
                            if ( mnHighItemId != it->mnId )
                            {
                                sal_uInt16 nTempPos = sal::static_int_cast<sal_uInt16>(it - mpData->m_aItems.begin());
                                if ( mnHighItemId )
                                {
                                    ImplHideFocus();
                                    sal_uInt16 nPos = GetItemPos( mnHighItemId );
                                    InvalidateItem(nPos);
                                    CallEventListeners( VclEventId::ToolboxHighlightOff, reinterpret_cast< void* >( nPos ) );
                                }
                                if ( mpData->mbMenubuttonSelected )
                                {
                                    // remove highlight from menubutton
                                    InvalidateMenuButton();
                                }
                                mnHighItemId = it->mnId;
                                InvalidateItem(nTempPos);
                                ImplShowFocus();
                                CallEventListeners( VclEventId::ToolboxHighlight );
                            }
                        }
                    }
                    break;
                }

                ++it;
            }
        }

        // only clear highlight when focus is not in toolbar
        bool bMenuButtonHit = mpData->maMenubuttonItem.maRect.IsInside( aMousePos ) && ImplHasClippedItems();
        if ( bClearHigh || bMenuButtonHit )
        {
            if ( !bMenuButtonHit && mpData->mbMenubuttonSelected )
            {
                // remove highlight from menubutton
                InvalidateMenuButton();
            }

            if( mnHighItemId )
            {
                sal_uInt16 nClearPos = GetItemPos( mnHighItemId );
                if ( nClearPos != TOOLBOX_ITEM_NOTFOUND )
                {
                    InvalidateItem(nClearPos);
                    if( nClearPos != mnCurPos )
                        CallEventListeners( VclEventId::ToolboxHighlightOff, reinterpret_cast< void* >( nClearPos ) );
                }
                ImplHideFocus();
                mnHighItemId = 0;
            }

            if( bMenuButtonHit )
            {
                InvalidateMenuButton();
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

void ToolBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    // only trigger toolbox for left mouse button and when
    // we're not in normal operation
    if ( rMEvt.IsLeft() && !mbDrag && (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
    {
        // call activate already here, as items could
        // be exchanged
        Activate();

        // update ToolBox here, such that user knows it
        if ( mbFormat )
        {
            ImplFormat();
            Update();
        }

        Point  aMousePos = rMEvt.GetPosPixel();
        sal_uInt16 i = 0;
        sal_uInt16 nNewPos = TOOLBOX_ITEM_NOTFOUND;

        // search for item that was clicked
        std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
        while ( it != mpData->m_aItems.end() )
        {
            // is this the item?
            if ( it->maRect.IsInside( aMousePos ) )
            {
                // do nothing if it is a separator or
                // if the item has been disabled
                if ( (it->meType == ToolBoxItemType::BUTTON) &&
                     (!it->mbShowWindow || mbCustomizeMode) )
                    nNewPos = i;

                break;
            }

            i++;
            ++it;
        }

        // item found
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

                    bool bResizeItem;
                    if ( mbCustomizeMode && it->mbShowWindow &&
                         (it->maRect.Right()-TB_RESIZE_OFFSET <= aMousePos.X()) )
                        bResizeItem = true;
                    else
                        bResizeItem = false;
                    pMgr->StartDragging( this, aMousePos, aItemRect, 0, bResizeItem );
                    return;
                }
            }

            if ( !it->mbEnabled )
            {
                Deactivate();
                return;
            }

            // update actual data
            StartTrackingFlags nTrackFlags = StartTrackingFlags::NONE;
            mnCurPos         = i;
            mnCurItemId      = it->mnId;
            mnDownItemId     = mnCurItemId;
            mnMouseClicks    = rMEvt.GetClicks();
            mnMouseModifier  = rMEvt.GetModifier();
            if ( it->mnBits & ToolBoxItemBits::REPEAT )
                nTrackFlags |= StartTrackingFlags::ButtonRepeat;

            if ( mbSelection )
            {
                InvalidateItem(mnCurPos);
                Highlight();
            }
            else
            {
                // update bDrag here, as it is evaluated in the EndSelection
                mbDrag = true;

                // on double-click: only call the handler, but do so before the button
                // is hit, as in the handler dragging
                // can be terminated
                if ( rMEvt.GetClicks() == 2 )
                    DoubleClick();

                if ( mbDrag )
                {
                    InvalidateItem(mnCurPos);
                    Highlight();
                }

                // was dropdown arrow pressed
                if( (it->mnBits & ToolBoxItemBits::DROPDOWN) )
                {
                    if( ( (it->mnBits & ToolBoxItemBits::DROPDOWNONLY) == ToolBoxItemBits::DROPDOWNONLY)
                        || it->GetDropDownRect( mbHorz && ( meTextPosition == ToolBoxTextPosition::Right ) ).IsInside( aMousePos ))
                    {
                        // dropdownonly always triggers the dropdown handler, over the whole button area

                        // the drop down arrow should not trigger the item action
                        mpData->mbDropDownByKeyboard = false;
                        mpData->maDropdownClickHdl.Call( this );

                        // do not reset data if the dropdown handler opened a floating window
                        // see ImplFloatControl()
                        if( !mpFloatWin )
                        {
                            // no floater was opened
                            Deactivate();
                            InvalidateItem(mnCurPos);

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

                // call Click handler
                if ( rMEvt.GetClicks() != 2 )
                    Click();

                // also call Select handler at repeat
                if ( nTrackFlags & StartTrackingFlags::ButtonRepeat )
                    Select();

                // if the actions was not aborted in Click handler
                if ( mbDrag )
                    StartTracking( nTrackFlags );
            }

            // if mouse was clicked over an item we
            // can abort here
            return;
        }

        Deactivate();

        // menu button hit ?
        if( mpData->maMenubuttonItem.maRect.IsInside( aMousePos ) && ImplHasClippedItems() )
        {
            ExecuteCustomMenu();
            return;
        }

        // check scroll- and next-buttons here
        if ( maUpperRect.IsInside( aMousePos ) )
        {
            if ( mnCurLine > 1 )
            {
                StartTracking();
                mbUpper = true;
                mbIn    = true;
                InvalidateSpin(true, false);
            }
            return;
        }
        if ( maLowerRect.IsInside( aMousePos ) )
        {
            if ( mnCurLine+mnVisLines-1 < mnCurLines )
            {
                StartTracking();
                mbLower = true;
                mbIn    = true;
                InvalidateSpin(false);
            }
            return;
        }

        // Linesizing testen
        if ( (mnWinStyle & TB_WBLINESIZING) == TB_WBLINESIZING )
        {
            sal_uInt16 nLineMode = ImplTestLineSize( this, aMousePos );
            if ( nLineMode )
            {
                ImplTBDragMgr* pMgr = ImplGetTBDragMgr();

                // call handler, such that we can set the
                // dock rectangles
                StartDocking();

                Point aPos  = GetParent()->OutputToScreenPixel( GetPosPixel() );
                Size  aSize = GetSizePixel();
                aPos = ScreenToOutputPixel( aPos );

                // start dragging
                pMgr->StartDragging( this, aMousePos, Rectangle( aPos, aSize ),
                                     nLineMode, false );
                return;
            }
        }

        // no item, then only click or double click
        if ( rMEvt.GetClicks() == 2 )
            DoubleClick();
        else
            Click();
    }

    if ( !mbDrag && !mbSelection && (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
        DockingWindow::MouseButtonDown( rMEvt );
}

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
    mbCommandDrag = false;

    DockingWindow::MouseButtonUp( rMEvt );
}

void ToolBox::Tracking( const TrackingEvent& rTEvt )
{
    VclPtr<vcl::Window> xWindow = this;

    if ( rTEvt.IsTrackingEnded() )
        ImplHandleMouseButtonUp( rTEvt.GetMouseEvent(), rTEvt.IsTrackingCanceled() );
    else
        ImplHandleMouseMove( rTEvt.GetMouseEvent(), rTEvt.IsTrackingRepeat() );

    if ( xWindow->IsDisposed() )
        // toolbox was deleted
        return;
    DockingWindow::Tracking( rTEvt );
}

void ToolBox::InvalidateItem(sal_uInt16 nPosition)
{
    if (mpData && nPosition < mpData->m_aItems.size())
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPosition];
        Invalidate(pItem->maRect);
    }
}

void ToolBox::InvalidateMenuButton()
{
    if (!mpData->maMenubuttonItem.maRect.IsEmpty())
        Invalidate(mpData->maMenubuttonItem.maRect);
}

void ToolBox::InvalidateSpin(bool bUpperIn, bool bLowerIn)
{
    if (bUpperIn && !maUpperRect.IsEmpty())
        Invalidate(maUpperRect);

    if (bLowerIn && !maLowerRect.IsEmpty())
        Invalidate(maLowerRect);
}

void ToolBox::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rPaintRect)
{
    if( mpData->mbIsPaintLocked )
        return;

    if (rPaintRect == Rectangle(0, 0, mnDX-1, mnDY-1))
        mbFullPaint = true;
    ImplFormat();
    mbFullPaint = false;

    ImplDrawBackground(rRenderContext, rPaintRect);

    if ( (mnWinStyle & WB_BORDER) && !ImplIsFloatingMode() )
        ImplDrawBorder(rRenderContext);

    if( !ImplIsFloatingMode() )
        ImplDrawGrip(rRenderContext);

    ImplDrawMenuButton(rRenderContext, mpData->mbMenubuttonSelected);

    // draw SpinButtons
    if (mnWinStyle & WB_SCROLL)
    {
        if (mnCurLines > mnLines)
            ImplDrawSpin(rRenderContext);
    }

    // draw buttons
    sal_uInt16 nHighPos;
    if ( mnHighItemId )
        nHighPos = GetItemPos( mnHighItemId );
    else
        nHighPos = TOOLBOX_ITEM_NOTFOUND;

    sal_uInt16 nCount = (sal_uInt16)mpData->m_aItems.size();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = &mpData->m_aItems[i];

        // only draw when the rectangle is in the draw rectangle
        if ( !pItem->maRect.IsEmpty() && rPaintRect.IsOver( pItem->maRect ) )
        {
            sal_uInt16 nHighlight = 0;
            if ( i == mnCurPos )
                nHighlight = 1;
            else if ( i == nHighPos )
                nHighlight = 2;
            ImplDrawItem(rRenderContext, i, nHighlight);
        }
    }
    ImplShowFocus();
}

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

    // If we have any expandable entries, then force a reformat first using
    // their optimal sizes, then share out the excess space evenly across those
    // expandables and reformat again
    std::vector<size_t> aExpandables;
    for (size_t i = 0; i < mpData->m_aItems.size(); ++i)
    {
        if (mpData->m_aItems[i].mbExpand)
        {
            vcl::Window *pWindow = mpData->m_aItems[i].mpWindow;
            SAL_INFO_IF(!pWindow, "vcl.layout", "only tabitems with window supported at the moment");
            if (!pWindow)
                continue;
            Size aWinSize(pWindow->GetSizePixel());
            Size aPrefSize(pWindow->get_preferred_size());
            aWinSize.Width() = aPrefSize.Width();
            pWindow->SetSizePixel(aWinSize);
            aExpandables.push_back(i);
        }
    }

    // re-format or re-draw
    if ( mbScroll || !aExpandables.empty() )
    {
        if ( !mbFormat || !aExpandables.empty() )
        {
            mbFormat = true;
            if( IsReallyVisible() || !aExpandables.empty() )
            {
                ImplFormat(true);

                if (!aExpandables.empty())
                {
                    //Get how big the optimal size is
                    Rectangle aBounds;
                    for (const ImplToolItem & rItem : mpData->m_aItems)
                    {
                        aBounds.Union( rItem.maRect );
                    }

                    long nOptimalWidth = aBounds.GetWidth();
                    long nDiff = aSize.Width() - nOptimalWidth;
                    nDiff /= aExpandables.size();

                    //share out the diff from optimal to real across
                    //expandable entries
                    for (size_t nIndex : aExpandables)
                    {
                        vcl::Window *pWindow = mpData->m_aItems[nIndex].mpWindow;
                        Size aWinSize(pWindow->GetSizePixel());
                        Size aPrefSize(pWindow->get_preferred_size());
                        aWinSize.Width() = aPrefSize.Width() + nDiff;
                        pWindow->SetSizePixel(aWinSize);
                    }

                    //now reformat with final sizes
                    mbFormat = true;
                    ImplFormat(true);
                }
            }
        }
    }

    // redraw border
    if ( mnWinStyle & WB_BORDER )
    {
        // as otherwise, when painting we might think we have to re-draw everything
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

const OUString& ToolBox::ImplGetHelpText( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    assert( pItem );

    if ( pItem->maHelpText.isEmpty() && ( !pItem->maHelpId.isEmpty() || pItem->maCommandStr.getLength() ))
    {
        Help* pHelp = Application::GetHelp();
        if ( pHelp )
        {
            if ( pItem->maCommandStr.getLength() )
                pItem->maHelpText = pHelp->GetHelpText( pItem->maCommandStr, this );
            if ( pItem->maHelpText.isEmpty() && !pItem->maHelpId.isEmpty() )
                pItem->maHelpText = pHelp->GetHelpText( OStringToOUString( pItem->maHelpId, RTL_TEXTENCODING_UTF8 ), this );
        }
    }

    return pItem->maHelpText;
}

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
        if ( rHEvt.GetMode() & (HelpEventMode::BALLOON | HelpEventMode::QUICK) )
        {
            // get rectangle
            Rectangle aTempRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aTempRect.TopLeft() );
            aTempRect.Left()   = aPt.X();
            aTempRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aTempRect.BottomRight() );
            aTempRect.Right()  = aPt.X();
            aTempRect.Bottom() = aPt.Y();

            // get text and display it
            OUString aStr = GetQuickHelpText( nItemId );
            const OUString& rHelpStr = GetHelpText( nItemId );
            if (aStr.isEmpty())
                aStr = MnemonicGenerator::EraseAllMnemonicChars( GetItemText( nItemId ) );
            if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
            {
                if (!rHelpStr.isEmpty())
                    aStr = rHelpStr;
                Help::ShowBalloon( this, aHelpPos, aTempRect, aStr );
            }
            else
                Help::ShowQuickHelp( this, aTempRect, aStr, rHelpStr, QuickHelpFlags::CtrlText );
            return;
        }
        else if ( rHEvt.GetMode() & HelpEventMode::EXTENDED )
        {
            OUString aCommand = GetItemCommand( nItemId );
            OString  aHelpId( GetHelpId( nItemId ) );

            if ( !aCommand.isEmpty() || !aHelpId.isEmpty() )
            {
                // If help is available then trigger it
                Help* pHelp = Application::GetHelp();
                if ( pHelp )
                {
                    if ( !aCommand.isEmpty() )
                        pHelp->Start( aCommand, this );
                    else if ( !aHelpId.isEmpty() )
                        pHelp->Start( OStringToOUString( aHelpId, RTL_TEXTENCODING_UTF8 ), this );
                }
                return;
            }
        }
    }

    DockingWindow::RequestHelp( rHEvt );
}

bool ToolBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        KeyEvent aKEvt = *rNEvt.GetKeyEvent();
        vcl::KeyCode aKeyCode = aKEvt.GetKeyCode();
        sal_uInt16  nKeyCode = aKeyCode.GetCode();
        switch( nKeyCode )
        {
            case KEY_TAB:
            {
                // internal TAB cycling only if parent is not a dialog or if we are the only child
                // otherwise the dialog control will take over
                vcl::Window *pParent = ImplGetParent();
                bool bOldSchoolContainer =
                    ((pParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL &&
                    pParent->GetChildCount() != 1);
                bool bNoTabCycling = bOldSchoolContainer || isContainerWindow(pParent);

                if( bNoTabCycling &&  ! (GetStyle() & WB_FORCETABCYCLE) )
                    return DockingWindow::EventNotify( rNEvt );
                else if( ImplChangeHighlightUpDn( aKeyCode.IsShift() , bNoTabCycling ) )
                    return false;
                else
                    return DockingWindow::EventNotify( rNEvt );
            }
            default:
                break;
        };
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
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
            else if( (GetGetFocusFlags() & (GetFocusFlags::Backward|GetFocusFlags::Tab) ) == (GetFocusFlags::Backward|GetFocusFlags::Tab))
                // Shift-TAB was pressed in the parent
                ImplChangeHighlightUpDn( false );
            else
                ImplChangeHighlightUpDn( true );

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
            return DockingWindow::EventNotify( rNEvt );
        }
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        // deselect
        ImplHideFocus();
        mnHighItemId = 0;
        mnCurPos = TOOLBOX_ITEM_NOTFOUND;
    }

    return DockingWindow::EventNotify( rNEvt );
}

void ToolBox::Command( const CommandEvent& rCEvt )
{
    maCommandHandler.Call( &rCEvt );

    // depict StartDrag on MouseButton/Left/Alt
    if ( (rCEvt.GetCommand() == CommandEventId::StartDrag) && rCEvt.IsMouseEvent() &&
         mbCustomize && !mbDragging && !mbDrag && !mbSelection &&
         (mnCurPos == TOOLBOX_ITEM_NOTFOUND) )
    {
        // We only allow dragging of items. Therefore, we have to check
        // if an item was clicked, otherwise we could move the window, and
        // this is unwanted.
        // We only do this in customize mode, as otherwise
        // items could be moved accidentally
        if ( mbCustomizeMode )
        {
            Point           aMousePos = rCEvt.GetMousePosPixel();
            std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
            while ( it != mpData->m_aItems.end() )
            {
                // is this the item?
                if ( it->maRect.IsInside( aMousePos ) )
                {
                    // do nothing if it is a separator or
                    // the item has been disabled
                    if ( (it->meType == ToolBoxItemType::BUTTON) &&
                         !it->mbShowWindow )
                        mbCommandDrag = true;
                    break;
                }

                ++it;
            }

            if ( mbCommandDrag )
            {
                MouseEvent aMEvt( aMousePos, 1, MouseEventModifiers::SIMPLEMOVE,
                                  MOUSE_LEFT, KEY_MOD2 );
                ToolBox::MouseButtonDown( aMEvt );
                return;
            }
        }
    }
    else if ( rCEvt.GetCommand() == CommandEventId::Wheel )
    {
        if ( (mnCurLine > 1) || (mnCurLine+mnVisLines-1 < mnCurLines) )
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if ( pData->GetMode() == CommandWheelMode::SCROLL )
            {
                if ( (mnCurLine > 1) && (pData->GetDelta() > 0) )
                    ShowLine( false );
                else if ( (mnCurLine+mnVisLines-1 < mnCurLines) && (pData->GetDelta() < 0) )
                    ShowLine( true );
                InvalidateSpin();
                return;
            }
        }
    }

    DockingWindow::Command( rCEvt );
}

void ToolBox::StateChanged( StateChangedType nType )
{
    DockingWindow::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
        ImplFormat();
    else if ( nType == StateChangedType::Enable )
        ImplUpdateItem();
    else if ( nType == StateChangedType::UpdateMode )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        mbCalc = true;
        mbFormat = true;
        ImplInitSettings( true, false, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false, true, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( false, false, true ); // font, foreground, background
        Invalidate();
    }

    maStateChangedHandler.Call( &nType );
}

void ToolBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    DockingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        mbCalc = true;
        mbFormat = true;
        ImplInitSettings( true, true, true );
        Invalidate();
    }

    maDataChangedHandler.Call( &rDCEvt );
}

void ToolBox::statusChanged( const css::frame::FeatureStateEvent& Event )
{
    // Update image mirroring/rotation
    if ( Event.FeatureURL.Complete == ".uno:ImageOrientation" )
    {
        SfxImageItem aItem( 1 );
        aItem.PutValue( Event.State, 0 );

        mbImagesMirrored = aItem.IsMirrored();
        mnImagesRotationAngle = aItem.GetRotation();

        // update image orientation
        for (std::vector<ImplToolItem>::const_iterator it = mpData->m_aItems.begin(); it != mpData->m_aItems.end(); ++it)
        {
            if (vcl::CommandInfoProvider::IsMirrored(it->maCommandStr, mpStatusListener->getFrame()))
                SetItemImageMirrorMode(it->mnId, mbImagesMirrored);
            if (vcl::CommandInfoProvider::IsRotated(it->maCommandStr, mpStatusListener->getFrame()))
                SetItemImageAngle(it->mnId, mnImagesRotationAngle);
        }
    }
}

void ToolBox::SetStyle(WinBits nNewStyle)
{
    mnWinStyle = nNewStyle;
    if (!ImplIsFloatingMode())
    {
        bool bOldScroll = mbScroll;
        mbScroll = (mnWinStyle & WB_SCROLL) != 0;
        if (mbScroll != bOldScroll)
        {
            mbFormat = true;
            ImplFormat();
        }
    }
}

void ToolBox::ToggleFloatingMode()
{
    DockingWindow::ToggleFloatingMode();

    if (!mpData)
        return;

    bool bOldHorz = mbHorz;

    if ( ImplIsFloatingMode() )
    {
        mbHorz   = true;
        meAlign  = WindowAlign::Top;
        mbScroll = true;

        if( bOldHorz != mbHorz )
            mbCalc = true;  // orientation was changed !

        ImplSetMinMaxFloatSize( this );
        SetOutputSizePixel( ImplCalcFloatSize( this, mnFloatLines ) );
    }
    else
    {
        mbScroll = (mnWinStyle & WB_SCROLL) != 0;
        if ( (meAlign == WindowAlign::Top) || (meAlign == WindowAlign::Bottom) )
            mbHorz = true;
        else
            mbHorz = false;

        // set focus back to document
        ImplGetFrameWindow()->GetWindow( GetWindowType::Client )->GrabFocus();
    }

    if( bOldHorz != mbHorz )
    {
        // if orientation changes, the toolbox has to be initialized again
        // to update the direction of the gradient
        mbCalc = true;
        ImplInitSettings( true, true, true );
    }

    mbFormat = true;
    ImplFormat();
}

void ToolBox::StartDocking()
{
    meDockAlign = meAlign;
    mnDockLines = mnLines;
    mbLastFloatMode = ImplIsFloatingMode();
    DockingWindow::StartDocking();
}

bool ToolBox::Docking( const Point& rPos, Rectangle& rRect )
{
    // do nothing during dragging, it was calculated before
    if ( mbDragging )
        return false;

    bool bFloatMode = false;

    DockingWindow::Docking( rPos, rRect );

    // if the mouse is outside the area, it can only become a floating window
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
    if ( !aIntersection.IsEmpty() )
    {
        Rectangle   aInRect = maInDockRect;
        Size aDockSize;
        aDockSize.Width()  = ImplCalcSize( this, mnLines, TB_CALCMODE_VERT ).Width();
        aDockSize.Height() = ImplCalcSize( this, mnLines, TB_CALCMODE_HORZ ).Height();
        aInRect.Left()   += aDockSize.Width()/2;
        aInRect.Top()    += aDockSize.Height()/2;
        aInRect.Right()  -= aDockSize.Width()/2;
        aInRect.Bottom() -= aDockSize.Height()/2;

        // if the window is too small, use the complete InDock-Rect
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

        // if the mouse is outside the Dock area, it can only
        // become a floating window
        Rectangle aIntersect = aInRect.GetIntersection( aDockingRect );
        if ( aIntersect == aDockingRect )
            bFloatMode = true;
        else
        {
            // docking rectangle is in the "sensible area"
            Point aPos = aDockingRect.TopLeft();
            Point aInPosTL( aPos.X()-aInRect.Left(), aPos.Y()-aInRect.Top() );
            Point aInPosBR( aPos.X()-aInRect.Left() + aDockingRect.GetWidth(), aPos.Y()-aInRect.Top() + aDockingRect.GetHeight() );
            Size  aInSize = aInRect.GetSize();

            if ( aInPosTL.X() <= 0 )
                meDockAlign = WindowAlign::Left;
            else if ( aInPosTL.Y() <= 0)
                meDockAlign = WindowAlign::Top;
            else if ( aInPosBR.X() >= aInSize.Width() )
                meDockAlign = WindowAlign::Right;
            else if ( aInPosBR.Y() >= aInSize.Height() )
                meDockAlign = WindowAlign::Bottom;

            // update the Dock size if Dock-Align was changed
            if ( (meDockAlign == WindowAlign::Top) || (meDockAlign == WindowAlign::Bottom) )
                aDockSize.Width() = maInDockRect.GetWidth();
            else
                aDockSize.Height() = maInDockRect.GetHeight();

            aDockingRect.SetSize( aDockSize );

            Point aPosTL( maInDockRect.TopLeft() );
            switch ( meDockAlign )
            {
                case WindowAlign::Top :
                    aDockingRect.SetPos( aPosTL );
                    break;
                case WindowAlign::Left :
                    aDockingRect.SetPos( aPosTL );
                    break;
                case WindowAlign::Bottom :
                {
                    Point aPosBL( maInDockRect.BottomLeft() );
                    aPosBL.Y() -= aDockingRect.GetHeight();
                    aDockingRect.SetPos( aPosBL );
                    break;
                }
                case WindowAlign::Right :
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
        bFloatMode = true;

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

void ToolBox::EndDocking( const Rectangle& rRect, bool bFloatMode )
{
    if ( !IsDockingCanceled() )
    {
        if ( mnLines != mnDockLines )
            SetLineCount( mnDockLines );
        if ( meAlign != meDockAlign )
            SetAlign( meDockAlign );
    }
    if ( bFloatMode || (bool(bFloatMode) != ImplIsFloatingMode()) )
        DockingWindow::EndDocking( rRect, bFloatMode );
}

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
                (nCalcLines <= maFloatSizes[0].mnLines) )
        {
            nCalcLines++;
            nTemp = nCalcLines;
            aTempSize = ImplCalcFloatSize( this, nTemp );
        }
        rSize = aTempSize;
    }

    mnLastResizeDY = rSize.Height();
}

Size ToolBox::GetOptimalSize() const
{
    // If we have any expandable entries, then force them to their
    // optimal sizes, then reset them afterwards
    std::map<vcl::Window*, Size> aExpandables;
    for (ImplToolItem & rItem : mpData->m_aItems)
    {
        if (rItem.mbExpand)
        {
            vcl::Window *pWindow = rItem.mpWindow;
            SAL_INFO_IF(!pWindow, "vcl.layout", "only tabitems with window supported at the moment");
            if (!pWindow)
                continue;
            Size aWinSize(pWindow->GetSizePixel());
            aExpandables[pWindow] = aWinSize;
            Size aPrefSize(pWindow->get_preferred_size());
            aWinSize.Width() = aPrefSize.Width();
            pWindow->SetSizePixel(aWinSize);
        }
    }

    Size aSize(ImplCalcSize( this, mnLines ));

    for (std::map<vcl::Window*, Size>::iterator aI = aExpandables.begin(); aI != aExpandables.end(); ++aI)
    {
        vcl::Window *pWindow = aI->first;
        Size aWinSize = aI->second;
        pWindow->SetSizePixel(aWinSize);
    }

    return aSize;
}

Size ToolBox::CalcWindowSizePixel( sal_uInt16 nCalcLines ) const
{
    return ImplCalcSize( this, nCalcLines );
}

Size ToolBox::CalcWindowSizePixel( sal_uInt16 nCalcLines, WindowAlign eAlign ) const
{
    return ImplCalcSize( this, nCalcLines,
        (eAlign == WindowAlign::Top || eAlign == WindowAlign::Bottom) ? TB_CALCMODE_HORZ : TB_CALCMODE_VERT );
}

sal_uInt16 ToolBox::ImplCountLineBreaks( const ToolBox *pThis )
{
    sal_uInt16 nLines = 0;

    std::vector< ImplToolItem >::const_iterator it = const_cast<ToolBox*>(pThis)->mpData->m_aItems.begin();
    while ( it != const_cast<ToolBox*>(pThis)->mpData->m_aItems.end() )
    {
        if( it->meType == ToolBoxItemType::BREAK )
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

    bool bPopup = mpData->mbAssumePopupMode;
    ToolBox *pThis = const_cast<ToolBox*>(this);
    pThis->mpData->mbAssumePopupMode = true;

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
    bool bFloat = mpData->mbAssumeFloating;
    bool bDocking = mpData->mbAssumeDocked;

    // simulate floating mode and force reformat before calculating
    ToolBox *pThis = const_cast<ToolBox*>(this);
    pThis->mpData->mbAssumeFloating = true;
    pThis->mpData->mbAssumeDocked = false;

    Size aSize = ImplCalcFloatSize( const_cast<ToolBox*>(this), nCalcLines );

    pThis->mbFormat = true;
    pThis->mpData->mbAssumeFloating = bFloat;
    pThis->mpData->mbAssumeDocked = bDocking;

    return aSize;
}

Size ToolBox::CalcMinimumWindowSizePixel() const
{
    if( ImplIsFloatingMode() )
        return ImplCalcSize( this, mnFloatLines );
    else
    {
        // create dummy toolbox for measurements
        VclPtrInstance< ToolBox > pToolBox( GetParent(), GetStyle() );

        // copy until first useful item
        std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
        while( it != mpData->m_aItems.end() )
        {
            pToolBox->CopyItem( *this, it->mnId );
            if( (it->meType != ToolBoxItemType::BUTTON) ||
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

        pToolBox.disposeAndClear();

        return aSize;
    }
}

void ToolBox::EnableCustomize( bool bEnable )
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

void ToolBox::LoseFocus()
{
    ImplChangeHighlight( nullptr, true );

    DockingWindow::LoseFocus();
}

// performs the action associated with an item, ie simulates clicking the item
void ToolBox::TriggerItem( sal_uInt16 nItemId )
{
    mnHighItemId = nItemId;
    sal_uInt16 nModifier = 0;
    vcl::KeyCode aKeyCode( 0, nModifier );
    ImplActivateItem( aKeyCode );
}

// calls the button's action handler
// returns true if action was called
bool ToolBox::ImplActivateItem( vcl::KeyCode aKeyCode )
{
    bool bRet = true;
    if( mnHighItemId )
    {
        ImplToolItem *pToolItem = ImplGetItem( mnHighItemId );

        // #107712#, activate can also be called for disabled entries
        if( pToolItem && !pToolItem->mbEnabled )
            return true;

        if( pToolItem && pToolItem->mpWindow && HasFocus() )
        {
            ImplHideFocus();
            mbChangingHighlight = true;  // avoid focus change due to loss of focus
            pToolItem->mpWindow->ImplControlFocus( GetFocusFlags::Tab );
            mbChangingHighlight = false;
        }
        else
        {
            mnDownItemId = mnCurItemId = mnHighItemId;
            if (pToolItem && (pToolItem->mnBits & ToolBoxItemBits::AUTOCHECK))
            {
                if ( pToolItem->mnBits & ToolBoxItemBits::RADIOCHECK )
                {
                    if ( pToolItem->meState != TRISTATE_TRUE )
                        SetItemState( pToolItem->mnId, TRISTATE_TRUE );
                }
                else
                {
                    if ( pToolItem->meState != TRISTATE_TRUE )
                        pToolItem->meState = TRISTATE_TRUE;
                    else
                        pToolItem->meState = TRISTATE_FALSE;
                }
            }
            mnMouseModifier = aKeyCode.GetModifier();
            mbIsKeyEvent = true;
            Activate();
            Click();

            // #107776# we might be destroyed in the selecthandler
            VclPtr<vcl::Window> xWindow = this;
            Select();
            if ( xWindow->IsDisposed() )
                return bRet;

            Deactivate();
            mbIsKeyEvent = false;
            mnMouseModifier = 0;
        }
    }
    else
        bRet = false;
    return bRet;
}

bool ImplCloseLastPopup( vcl::Window *pParent )
{
    // close last popup toolbox (see also:
    // ImplHandleMouseFloatMode(...) in winproc.cxx )

    if( ImplGetSVData()->maWinData.mpFirstFloat )
    {
        FloatingWindow* pLastLevelFloat = ImplGetSVData()->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
        // only close the floater if it is not our direct parent, which would kill ourself
        if( pLastLevelFloat && pLastLevelFloat != pParent )
        {
            pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
            return true;
        }
    }
    return false;
}

// opens a drop down toolbox item
// returns true if item was opened
bool ToolBox::ImplOpenItem( vcl::KeyCode aKeyCode )
{
    sal_uInt16 nCode = aKeyCode.GetCode();
    bool bRet = true;

    // arrow keys should work only in the opposite direction of alignment (to not break cursor travelling)
    if ( ((nCode == KEY_LEFT || nCode == KEY_RIGHT) && IsHorizontal())
      || ((nCode == KEY_UP   || nCode == KEY_DOWN)  && !IsHorizontal()) )
        return false;

    if( IsMenuEnabled() && mpData->mbMenubuttonSelected )
    {
        if( ImplCloseLastPopup( GetParent() ) )
            return bRet;

        UpdateCustomMenu();
        mpData->mnEventId = Application::PostUserEvent( LINK( this, ToolBox, ImplCallExecuteCustomMenu ), nullptr, true );
    }
    else if( mnHighItemId &&  ImplGetItem( mnHighItemId ) &&
        (ImplGetItem( mnHighItemId )->mnBits & ToolBoxItemBits::DROPDOWN) )
    {
        if( ImplCloseLastPopup( GetParent() ) )
            return bRet;

        mnDownItemId = mnCurItemId = mnHighItemId;
        mnCurPos = GetItemPos( mnCurItemId );
        mnLastFocusItemId = mnCurItemId; // save item id for possible later focus restore
        mnMouseModifier = aKeyCode.GetModifier();
        mbIsShift = true;
        mbIsKeyEvent = true;
        Activate();

        mpData->mbDropDownByKeyboard = true;
        mpData->maDropdownClickHdl.Call( this );

        mbIsKeyEvent = false;
        mbIsShift = false;
        mnMouseModifier = 0;
    }
    else
        bRet = false;

    return bRet;
}

void ToolBox::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    mnKeyModifier = aKeyCode.GetModifier();
    sal_uInt16 nCode = aKeyCode.GetCode();

    vcl::Window *pParent = ImplGetParent();
    bool bOldSchoolContainer = ((pParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL);
    bool bParentIsContainer = bOldSchoolContainer || isContainerWindow(pParent);

    bool bForwardKey = false;
    bool bGrabFocusToDocument = false;

    // #107776# we might be destroyed in the keyhandler
    VclPtr<vcl::Window> xWindow = this;

    switch ( nCode )
    {
        case KEY_UP:
        {
            // Ctrl-Cursor activates next toolbox, indicated by a blue arrow pointing to the left/up
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( !IsHorizontal() )
                ImplChangeHighlightUpDn( true );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_LEFT:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( IsHorizontal() )
                ImplChangeHighlightUpDn( true );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_DOWN:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( !IsHorizontal() )
                ImplChangeHighlightUpDn( false );
            else
                ImplOpenItem( aKeyCode );
        }
        break;
        case KEY_RIGHT:
        {
            if( aKeyCode.GetModifier() )    // allow only pure cursor keys
                break;
            if( IsHorizontal() )
                ImplChangeHighlightUpDn( false );
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
                mbFormat = true;
                ImplFormat();
                InvalidateSpin();
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
                mbFormat = true;
                ImplFormat();
                InvalidateSpin();
                ImplChangeHighlight( ImplGetFirstValidItem( mnCurLine ) );
            }
        break;
        case KEY_END:
            {
                ImplChangeHighlight( nullptr );
                ImplChangeHighlightUpDn( false );
            }
            break;
        case KEY_HOME:
            {
                ImplChangeHighlight( nullptr );
                ImplChangeHighlightUpDn( true );
            }
            break;
        case KEY_ESCAPE:
        {
            if( !ImplIsFloatingMode() && bParentIsContainer )
                DockingWindow::KeyInput( rKEvt );
            else
            {
                // send focus to document pane
                vcl::Window *pWin = this;
                while( pWin )
                {
                    if( !pWin->GetParent() )
                    {
                        pWin->ImplGetFrameWindow()->GetWindow( GetWindowType::Client )->GrabFocus();
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
                    bGrabFocusToDocument = true;
                }
            }
            if( !bGrabFocusToDocument )
                bForwardKey = !ImplActivateItem( aKeyCode );
        }
        break;
        case KEY_SPACE:
        {
            ImplOpenItem( aKeyCode );
        }
        break;
        default:
            {
            sal_uInt16 aKeyGroup = aKeyCode.GetGroup();
            ImplToolItem *pItem = nullptr;
            if( mnHighItemId )
                pItem = ImplGetItem( mnHighItemId );
            // #i13931# forward alphanum keyinput into embedded control
            if( (aKeyGroup == KEYGROUP_NUM || aKeyGroup == KEYGROUP_ALPHA ) && pItem && pItem->mpWindow && pItem->mbEnabled )
            {
                vcl::Window *pFocusWindow = Application::GetFocusWindow();
                ImplHideFocus();
                mbChangingHighlight = true;  // avoid focus change due to loss of focus
                pItem->mpWindow->ImplControlFocus( GetFocusFlags::Tab );
                mbChangingHighlight = false;
                if( pFocusWindow != Application::GetFocusWindow() )
                    Application::GetFocusWindow()->KeyInput( rKEvt );
            }
            else
            {
                // do nothing to avoid key presses going into the document
                // while the toolbox has the focus
                // just forward function and special keys and combinations with Alt-key
                if( aKeyGroup == KEYGROUP_FKEYS || aKeyGroup == KEYGROUP_MISC || aKeyCode.IsMod2() )
                    bForwardKey = true;
            }
        }
    }

    if ( xWindow->IsDisposed() )
        return;

    // #107251# move focus away if this toolbox was disabled during keyinput
    if (HasFocus() && mpData->mbKeyInputDisabled && bParentIsContainer)
    {
        sal_uInt16 n = 0;
        vcl::Window *pFocusControl = pParent->ImplGetDlgWindow( n, GetDlgWindowType::First );
        if ( pFocusControl && pFocusControl != this )
            pFocusControl->ImplControlFocus( GetFocusFlags::Init );
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
        return nullptr;

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
            while( it != mpData->m_aItems.end() && ((it->meType != ToolBoxItemType::BUTTON) ||
                /*!it->mbEnabled ||*/ !it->mbVisible || ImplIsFixedControl( &(*it) )) )
            {
                ++it;
                if( it == mpData->m_aItems.end() || it->mbBreak )
                    return nullptr;    // no valid items in this line
            }
            return &(*it);
        }
        ++it;
    }

    return (it == mpData->m_aItems.end()) ? nullptr : &(*it);
}

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
        ImplGrabFocus( GetFocusFlags::NONE );
        ImplChangeHighlight ( ImplGetItem ( GetItemId ( (sal_uInt16) nPos ) ) );
    }
}

void ToolBox::ImplChangeHighlight( ImplToolItem* pItem, bool bNoGrabFocus )
{
    // avoid recursion due to focus change
    if( mbChangingHighlight )
        return;

    mbChangingHighlight = true;

    ImplToolItem* pOldItem = nullptr;

    if ( mnHighItemId )
    {
        ImplHideFocus();
        sal_uInt16 nPos = GetItemPos( mnHighItemId );
        pOldItem = ImplGetItem( mnHighItemId );
        // #i89962# ImplDrawItem can cause Invalidate/Update
        // which will in turn ImplShowFocus again
        // set mnHighItemId to 0 already to prevent this hen/egg problem
        mnHighItemId = 0;
        InvalidateItem(nPos);
        CallEventListeners( VclEventId::ToolboxHighlightOff, reinterpret_cast< void* >( nPos ) );
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
                mbFormat = true;
            }
            else if ( nLine < mnCurLine )
            {
                mnCurLine = nLine;
                mbFormat = true;
            }

            if( mbFormat )
            {
                ImplFormat();
            }

            mnHighItemId = pItem->mnId;
            InvalidateItem(aPos);

            if( mbSelection )
                mnCurPos = aPos;
            ImplShowFocus();

            if( pItem->mpWindow )
                pItem->mpWindow->GrabFocus();
            if( pItem != pOldItem )
                CallEventListeners( VclEventId::ToolboxHighlight );
        }
    }
    else
    {
        ImplHideFocus();
        mnHighItemId = 0;
        mnCurPos = TOOLBOX_ITEM_NOTFOUND;
    }

    mbChangingHighlight = false;
}

// check for keyboard accessible items
static bool ImplIsValidItem( const ImplToolItem* pItem, bool bNotClipped )
{
    bool bValid = (pItem && pItem->meType == ToolBoxItemType::BUTTON && pItem->mbVisible && !ImplIsFixedControl( pItem ));
    if( bValid && bNotClipped && pItem->IsClipped() )
        bValid = false;
    return bValid;
}

bool ToolBox::ImplChangeHighlightUpDn( bool bUp, bool bNoCycle )
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
                ImplToolItem* pItem = nullptr;
                while( it != mpData->m_aItems.begin() )
                {
                    --it;
                    if ( ImplIsValidItem( &(*it), true ) )
                    {
                        pItem = &(*it);
                        break;
                    }
                }
                InvalidateMenuButton();
                ImplChangeHighlight( pItem );
            }
            else
            {
                // select first valid non-clipped item
                std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
                while( it != mpData->m_aItems.end() )
                {
                    if ( ImplIsValidItem( &(*it), true ) )
                        break;
                    ++it;
                }
                if( it != mpData->m_aItems.end() )
                {
                    InvalidateMenuButton();
                    ImplChangeHighlight( &(*it) );
                }
            }
            return true;
        }

        if( bUp )
        {
            // Select first valid item
            std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
            while( it != mpData->m_aItems.end() )
            {
                if ( ImplIsValidItem( &(*it), false ) )
                    break;
                ++it;
            }

            // select the menu button if a clipped item would be selected
            if( (it != mpData->m_aItems.end() && &(*it) == ImplGetFirstClippedItem( this )) && IsMenuEnabled() )
            {
                ImplChangeHighlight( nullptr );
                InvalidateMenuButton();
            }
            else
                ImplChangeHighlight( (it != mpData->m_aItems.end()) ? &(*it) : nullptr );
            return true;
        }
        else
        {
            // Select last valid item

            // docked toolbars have the menubutton as last item - if this button is enabled
            if( IsMenuEnabled() && !ImplIsFloatingMode() )
            {
                ImplChangeHighlight( nullptr );
                InvalidateMenuButton();
            }
            else
            {
                std::vector< ImplToolItem >::iterator it = mpData->m_aItems.end();
                ImplToolItem* pItem = nullptr;
                while( it != mpData->m_aItems.begin() )
                {
                    --it;
                    if ( ImplIsValidItem( &(*it), false ) )
                    {
                        pItem = &(*it);
                        break;
                    }
                }
                ImplChangeHighlight( pItem );
            }
            return true;
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
                        return false;

                    // highlight the menu button if it is the last item
                    if( IsMenuEnabled() && !ImplIsFloatingMode() )
                    {
                        ImplChangeHighlight( nullptr );
                        InvalidateMenuButton();
                        return true;
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
                        return false;

                    // highlight the menu button if it is the last item
                    if( IsMenuEnabled() && !ImplIsFloatingMode() )
                    {
                        ImplChangeHighlight( nullptr );
                        InvalidateMenuButton();
                        return true;
                    }
                    else
                        pos = 0;
                }
            }

            pToolItem = &mpData->m_aItems[pos];

            if ( ImplIsValidItem( pToolItem, false ) )
                break;

        } while( ++i < nCount);

        if( pToolItem->IsClipped() && IsMenuEnabled() )
        {
            // select the menu button if a clipped item would be selected
            ImplChangeHighlight( nullptr );
            InvalidateMenuButton();
        }
        else if( i != nCount )
            ImplChangeHighlight( pToolItem );
        else
            return false;
    }
    return true;
}

void ToolBox::ImplShowFocus()
{
    if( mnHighItemId && HasFocus() )
    {
        ImplToolItem* pItem = ImplGetItem( mnHighItemId );
        if( pItem->mpWindow && !pItem->mpWindow->IsDisposed() )
        {
            vcl::Window *pWin = pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow ? pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow.get() : pItem->mpWindow.get();
            pWin->ImplGetWindowImpl()->mbDrawSelectionBackground = true;
            pWin->Invalidate();
        }
    }
}

void ToolBox::ImplHideFocus()
{
    if( mnHighItemId )
    {
        ImplToolItem* pItem = ImplGetItem( mnHighItemId );
        if( pItem && pItem->mpWindow )
        {
            vcl::Window *pWin = pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow ? pItem->mpWindow->ImplGetWindowImpl()->mpBorderWindow.get() : pItem->mpWindow.get();
            pWin->ImplGetWindowImpl()->mbDrawSelectionBackground = false;
            pWin->Invalidate();
        }
    }

    if ( mpData && mpData->mbMenubuttonSelected )
    {
        // remove highlight from menubutton
        InvalidateMenuButton();
    }
}

void ToolBox::ImplDisableFlatButtons()
{
#ifdef _WIN32        // Check in the Windows registry if an AT tool wants no flat toolboxes
    static bool bInit = false, bValue = false;
    if( ! bInit )
    {
        bInit = true;
        HKEY hkey;

        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, "Software\\LibreOffice\\Accessibility\\AtToolSupport", &hkey) )
        {
            DWORD dwType = 0;
            sal_uInt8 Data[6]; // possible values: "true", "false", "1", "0", DWORD
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueEx(hkey, "DisableFlatToolboxButtons",
                nullptr, &dwType, Data, &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        bValue = ((0 == stricmp(reinterpret_cast<const char *>(Data), "1")) || (0 == stricmp(reinterpret_cast<const char *>(Data), "true")));
                        break;
                    case REG_DWORD:
                        bValue = (bool)(reinterpret_cast<DWORD *>(Data)[0]);
                        break;
                }
            }
            RegCloseKey(hkey);
        }
    }
    if( bValue )
        mnOutStyle &= ~TOOLBOX_STYLE_FLAT;
#else
    (void) this; // loplugin:staticmethods
#endif
}

void ToolBox::SetToolbarLayoutMode( ToolBoxLayoutMode eLayout )
{
    if ( meLayoutMode != eLayout )
       meLayoutMode  = eLayout;
}

void ToolBox::SetToolBoxTextPosition( ToolBoxTextPosition ePosition )
{
    meTextPosition = ePosition;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
