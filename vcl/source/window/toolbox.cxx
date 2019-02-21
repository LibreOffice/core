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
#include <vcl/ptrstyle.hxx>

#include <tools/poly.hxx>
#include <svl/imageitm.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <svdata.hxx>
#include <window.h>
#include <toolbox.h>
#include <salframe.hxx>
#include <spin.hxx>
#if defined(_WIN32)
#include <svsys.h>
#endif

#include <cstdlib>
#include <limits>
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
#define TB_MAXLINES             5
#define TB_MAXNOSCROLL          32765

#define TB_DRAGWIDTH            8  // the default width of the drag grip

#define TB_CALCMODE_HORZ        1
#define TB_CALCMODE_VERT        2
#define TB_CALCMODE_FLOAT       3

#define TB_WBLINESIZING         (WB_SIZEABLE | WB_DOCKABLE | WB_SCROLL)

#define DOCK_LINEHSIZE          (sal_uInt16(0x0001))
#define DOCK_LINEVSIZE          (sal_uInt16(0x0002))
#define DOCK_LINERIGHT          (sal_uInt16(0x1000))
#define DOCK_LINEBOTTOM         (sal_uInt16(0x2000))
#define DOCK_LINELEFT           (sal_uInt16(0x4000))
#define DOCK_LINETOP            (sal_uInt16(0x8000))
#define DOCK_LINEOFFSET         3

class ImplTBDragMgr
{
private:
    VclPtr<ToolBox> mpDragBox;
    Point           maMouseOff;
    tools::Rectangle       maRect;
    tools::Rectangle       maStartRect;
    Accelerator     maAccel;
    sal_uInt16      mnLineMode;
    ToolBox::ImplToolItems::size_type mnStartLines;

    ImplTBDragMgr(const ImplTBDragMgr&) = delete;
    ImplTBDragMgr& operator=(const ImplTBDragMgr&) = delete;

public:
                    ImplTBDragMgr();

    void            StartDragging( ToolBox* pDragBox, const Point& rPos, const tools::Rectangle& rRect, sal_uInt16 nLineMode );
    void            Dragging( const Point& rPos );
    void            EndDragging( bool bOK = true );
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
        tools::Rectangle aContent, aBound;
        tools::Rectangle aArea( Point(), rRenderContext.GetOutputSizePixel() );

        if ( rRenderContext.GetNativeControlRegion(ControlType::Toolbar,
                bHorz ? ControlPart::ThumbVert : ControlPart::ThumbHorz,
                aArea, ControlState::NONE, aControlValue, aBound, aContent) )
        {
            nWidth = bHorz ? aContent.GetWidth() : aContent.GetHeight();
        }
    }

    // increase the hit area of the drag handle according to DPI scale factor
    nWidth *= rRenderContext.GetDPIScaleFactor();

    return nWidth;
}

int ToolBox::ImplGetDragWidth() const
{
    return ToolBox::ImplGetDragWidth( *this, mbHorz );
}

static ButtonType determineButtonType( ImplToolItem const * pItem, ButtonType defaultType )
{
    ButtonType tmpButtonType = defaultType;
    ToolBoxItemBits nBits = pItem->mnBits & ( ToolBoxItemBits::TEXT_ONLY | ToolBoxItemBits::ICON_ONLY );
    if ( nBits != ToolBoxItemBits::NONE ) // item has custom setting
    {
        tmpButtonType = ButtonType::SYMBOLTEXT;
        if ( nBits == ToolBoxItemBits::TEXT_ONLY )
            tmpButtonType = ButtonType::TEXT;
        else if ( nBits == ToolBoxItemBits::ICON_ONLY )
            tmpButtonType = ButtonType::SYMBOLONLY;
    }
    return tmpButtonType;
}

void ToolBox::ImplUpdateDragArea() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        if ( ImplIsFloatingMode() || pWrapper->IsLocked() )
            pWrapper->SetDragArea( tools::Rectangle() );
        else
        {
            if( meAlign == WindowAlign::Top || meAlign == WindowAlign::Bottom )
                pWrapper->SetDragArea( tools::Rectangle( 0, 0, ImplGetDragWidth(), GetOutputSizePixel().Height() ) );
            else
                pWrapper->SetDragArea( tools::Rectangle( 0, 0, GetOutputSizePixel().Width(), ImplGetDragWidth() ) );
        }
    }
}

void ToolBox::ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
                              long& rRight, long& rBottom ) const
{
    if( ImplIsFloatingMode() || !(mnWinStyle & WB_BORDER) )
    {
        // no border in floating mode
        rLeft = rTop = rRight = rBottom = 0;
        return;
    }

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );

    // reserve DragArea only for dockable toolbars
    int    dragwidth = ( pWrapper && !pWrapper->IsLocked() ) ? ImplGetDragWidth() : 0;

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

void ToolBox::ImplCheckUpdate()
{
    // remove any pending invalidates to avoid
    // have them triggered when paint is locked (see mpData->mbIsPaintLocked)
    // which would result in erasing the background only and not painting any items
    // this must not be done when we're already in Paint()

    // this is only required for transparent toolbars (see ImplDrawTransparentBackground() )
    if( !IsBackground() && HasPaintEvent() && !IsInPaint() )
        Update();
}

void ToolBox::ImplDrawGrip(vcl::RenderContext& rRenderContext,
        const tools::Rectangle &aDragArea, int nDragWidth, WindowAlign eAlign, bool bHorz)
{
    bool bNativeOk = false;
    const ControlPart ePart = bHorz ? ControlPart::ThumbVert : ControlPart::ThumbHorz;
    const Size aSz( rRenderContext.GetOutputSizePixel() );
    if (rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ePart))
    {
        ToolbarValue aToolbarValue;
        aToolbarValue.maGripRect = aDragArea;

        tools::Rectangle aCtrlRegion(Point(), aSz);

        bNativeOk = rRenderContext.DrawNativeControl( ControlType::Toolbar, ePart,
                                        aCtrlRegion, ControlState::ENABLED, aToolbarValue, OUString() );
    }

    if( bNativeOk )
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());

    float fScaleFactor = rRenderContext.GetDPIScaleFactor();

    if (eAlign == WindowAlign::Top || eAlign == WindowAlign::Bottom)
    {
        int height = static_cast<int>(0.6 * aSz.Height() + 0.5);
        int i = (aSz.Height() - height) / 2;
        height += i;
        while (i <= height)
        {
            int x = nDragWidth / 2;
            rRenderContext.DrawEllipse(tools::Rectangle(Point(x, i), Size(2 * fScaleFactor, 2 * fScaleFactor)));
            i += 4 * fScaleFactor;
        }
    }
    else
    {
        int width = static_cast<int>(0.6 * aSz.Width() + 0.5);
        int i = (aSz.Width() - width) / 2;
        width += i;
        while (i <= width)
        {
            int y = nDragWidth / 2;
            rRenderContext.DrawEllipse(tools::Rectangle(Point(i, y), Size(2 * fScaleFactor, 2 * fScaleFactor)));
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
        ImplCheckUpdate();
        ImplDrawGrip( rRenderContext, pWrapper->GetDragArea(),
                      ImplGetDragWidth(), meAlign, mbHorz );
    }
}

void ToolBox::ImplDrawGradientBackground(vcl::RenderContext& rRenderContext)
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

            aLineSz.setHeight( nLineSize );
        }
        else
        {
            nLineSize = mnMaxItemWidth;
            aLineSz.setWidth( nLineSize );
        }
    }

    long nLeft, nTop, nRight, nBottom;
    ImplCalcBorder(meAlign, nLeft, nTop, nRight, nBottom);

    Size aTopLineSz(aLineSz);
    Size aBottomLineSz(aLineSz);

    if (mnWinStyle & WB_BORDER)
    {
        if (mbHorz)
        {
            aTopLineSz.AdjustHeight(TB_BORDER_OFFSET2 + nTop );
            aBottomLineSz.AdjustHeight(TB_BORDER_OFFSET2 + nBottom );

            if (mnCurLines == 1)
                aTopLineSz.AdjustHeight(TB_BORDER_OFFSET2 + nBottom );
        }
        else
        {
            aTopLineSz.AdjustWidth(TB_BORDER_OFFSET1 + nLeft );
            aBottomLineSz.AdjustWidth(TB_BORDER_OFFSET1 + nRight );

            if (mnCurLines == 1)
                aTopLineSz.AdjustWidth(TB_BORDER_OFFSET1 + nLeft );
        }
    }

    if (mbLineSpacing)
    {
        if (mbHorz)
        {
            aLineSz.AdjustHeight(TB_LINESPACING );
            if (mnCurLines > 1)
                aTopLineSz.AdjustHeight(TB_LINESPACING );
        }
        else
        {
            aLineSz.AdjustWidth(TB_LINESPACING );
            if (mnCurLines > 1)
                aTopLineSz.AdjustWidth(TB_LINESPACING );
        }
    }

    if (mbHorz)
    {
        long y = 0;

        rRenderContext.DrawGradient(tools::Rectangle(0, y, aTopLineSz.Width(), y + aTopLineSz.Height()), g);
        y += aTopLineSz.Height();

        while (y < (mnDY - aBottomLineSz.Height()))
        {
            rRenderContext.DrawGradient(tools::Rectangle(0, y, aLineSz.Width(), y + aLineSz.Height()), g);
            y += aLineSz.Height();
        }

        rRenderContext.DrawGradient(tools::Rectangle(0, y, aBottomLineSz.Width(), y + aBottomLineSz.Height()), g);
    }
    else
    {
        long x = 0;

        rRenderContext.DrawGradient(tools::Rectangle(x, 0, x + aTopLineSz.Width(), aTopLineSz.Height()), g);
        x += aTopLineSz.Width();

        while (x < (mnDX - aBottomLineSz.Width()))
        {
            rRenderContext.DrawGradient(tools::Rectangle(x, 0, x + aLineSz.Width(), aLineSz.Height()), g);
            x += aLineSz.Width();
        }

        rRenderContext.DrawGradient(tools::Rectangle( x, 0, x + aBottomLineSz.Width(), aBottomLineSz.Height()), g);
    }

    if( bLineColor )
        rRenderContext.SetLineColor( aOldCol );

}

bool ToolBox::ImplDrawNativeBackground(vcl::RenderContext& rRenderContext)
{
    // use NWF
    tools::Rectangle aCtrlRegion(Point(), GetOutputSizePixel());

    return rRenderContext.DrawNativeControl( ControlType::Toolbar, mbHorz ? ControlPart::DrawBackgroundHorz : ControlPart::DrawBackgroundVert,
                                    aCtrlRegion, ControlState::ENABLED, ImplControlValue(), OUString() );
}

void ToolBox::ImplDrawTransparentBackground(const vcl::Region &rRegion)
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

void ToolBox::ImplDrawBackground(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    // execute pending paint requests
    ImplCheckUpdate();

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
            ImplDrawTransparentBackground(aPaintRegion);
        else
            ImplDrawConstantBackground(rRenderContext, aPaintRegion, bIsInPopupMode);
    }
    else
    {
        // toolbars known to the dockingmanager will be drawn using NWF or a gradient
        // docked toolbars are transparent and NWF is already used in the docking area which is their common background
        // so NWF is used here for floating toolbars only
        bool bNativeOk = false;
        if( ImplIsFloatingMode() && rRenderContext.IsNativeControlSupported( ControlType::Toolbar, ControlPart::Entire) )
            bNativeOk = ImplDrawNativeBackground(rRenderContext);
        if (!bNativeOk)
        {
            const StyleSettings rSetting = Application::GetSettings().GetStyleSettings();
            const bool isHeader = GetAlign() == WindowAlign::Top && !rSetting.GetPersonaHeader().IsEmpty();
            const bool isFooter = GetAlign() == WindowAlign::Bottom && !rSetting.GetPersonaFooter().IsEmpty();
            if (!IsBackground() || isHeader || isFooter)
            {
                if (!IsInPaint())
                    ImplDrawTransparentBackground(aPaintRegion);
            }
            else
                ImplDrawGradientBackground(rRenderContext);
        }
    }

    // restore clip region
    rRenderContext.Pop();
}

void ToolBox::ImplErase(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect, bool bHighlight, bool bHasOpenPopup)
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
                rRenderContext.SetFillColor(COL_WHITE);

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
            (pItem->mpWindow->GetType() == WindowType::FIXEDTEXT ||
             pItem->mpWindow->GetType() == WindowType::FIXEDLINE ||
             pItem->mpWindow->GetType() == WindowType::GROUPBOX) );
}

const ImplToolItem *ToolBox::ImplGetFirstClippedItem() const
{
    for (auto & item : mpData->m_aItems)
    {
        if( item.IsClipped() )
            return &item;
    }
    return nullptr;
}

Size ToolBox::ImplCalcSize( ImplToolItems::size_type nCalcLines, sal_uInt16 nCalcMode )
{
    long            nMax;
    long            nLeft = 0;
    long            nTop = 0;
    long            nRight = 0;
    long            nBottom = 0;
    Size            aSize;
    WindowAlign     eOldAlign = meAlign;
    bool            bOldHorz = mbHorz;
    bool            bOldAssumeDocked = mpData->mbAssumeDocked;
    bool            bOldAssumeFloating = mpData->mbAssumeFloating;

    if ( nCalcMode )
    {
        bool bOldFloatingMode = ImplIsFloatingMode();

        mpData->mbAssumeDocked = false;
        mpData->mbAssumeFloating = false;

        if ( nCalcMode == TB_CALCMODE_HORZ )
        {
            mpData->mbAssumeDocked = true;   // force non-floating mode during calculation
            ImplCalcBorder( WindowAlign::Top, nLeft, nTop, nRight, nBottom );
            mbHorz = true;
            if ( mbHorz != bOldHorz )
                meAlign = WindowAlign::Top;
        }
        else if ( nCalcMode == TB_CALCMODE_VERT )
        {
            mpData->mbAssumeDocked = true;   // force non-floating mode during calculation
            ImplCalcBorder( WindowAlign::Left, nLeft, nTop, nRight, nBottom );
            mbHorz = false;
            if ( mbHorz != bOldHorz )
                meAlign = WindowAlign::Left;
        }
        else if ( nCalcMode == TB_CALCMODE_FLOAT )
        {
            mpData->mbAssumeFloating = true;   // force non-floating mode during calculation
            nLeft = nTop = nRight = nBottom = 0;
            mbHorz = true;
            if ( mbHorz != bOldHorz )
                meAlign = WindowAlign::Top;
        }

        if ( (meAlign != eOldAlign) || (mbHorz != bOldHorz) ||
             (ImplIsFloatingMode() != bOldFloatingMode ) )
            mbCalc = true;
    }
    else
        ImplCalcBorder( meAlign, nLeft, nTop, nRight, nBottom );

    ImplCalcItem();

    if( !nCalcMode && ImplIsFloatingMode() )
    {
        aSize = ImplCalcFloatSize( nCalcLines );
    }
    else
    {
        if ( mbHorz )
        {
            if ( mnWinHeight > mnMaxItemHeight )
                aSize.setHeight( nCalcLines * mnWinHeight );
            else
                aSize.setHeight( nCalcLines * mnMaxItemHeight );

            if ( mbLineSpacing )
                aSize.AdjustHeight((nCalcLines-1)*TB_LINESPACING );

            if ( mnWinStyle & WB_BORDER )
                aSize.AdjustHeight((TB_BORDER_OFFSET2*2) + nTop + nBottom );

            nMax = 0;
            ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, mbHorz );
            if ( nMax )
                aSize.AdjustWidth(nMax );

            if ( mnWinStyle & WB_BORDER )
                aSize.AdjustWidth((TB_BORDER_OFFSET1*2) + nLeft + nRight );
        }
        else
        {
            aSize.setWidth( nCalcLines * mnMaxItemWidth );

            if ( mbLineSpacing )
                aSize.AdjustWidth((nCalcLines-1)*TB_LINESPACING );

            if ( mnWinStyle & WB_BORDER )
                aSize.AdjustWidth((TB_BORDER_OFFSET2*2) + nLeft + nRight );

            nMax = 0;
            ImplCalcBreaks( TB_MAXNOSCROLL, &nMax, mbHorz );
            if ( nMax )
                aSize.AdjustHeight(nMax );

            if ( mnWinStyle & WB_BORDER )
                aSize.AdjustHeight((TB_BORDER_OFFSET1*2) + nTop + nBottom );
        }
    }
    // restore previous values
    if ( nCalcMode )
    {
        mpData->mbAssumeDocked = bOldAssumeDocked;
        mpData->mbAssumeFloating = bOldAssumeFloating;
        if ( (meAlign != eOldAlign) || (mbHorz != bOldHorz) )
        {
            meAlign  = eOldAlign;
            mbHorz   = bOldHorz;
            mbCalc   = true;
        }
    }

    return aSize;
}

void ToolBox::ImplCalcFloatSizes()
{
    if ( !maFloatSizes.empty() )
        return;

    // calculate the minimal size, i.e. where the biggest item just fits
    long            nCalcSize = 0;

    for (auto const& item : mpData->m_aItems)
    {
        if ( item.mbVisible )
        {
            if ( item.mpWindow )
            {
                long nTempSize = item.mpWindow->GetSizePixel().Width();
                if ( nTempSize > nCalcSize )
                    nCalcSize = nTempSize;
            }
            else
            {
                if( item.maItemSize.Width() > nCalcSize )
                    nCalcSize = item.maItemSize.Width();
            }
        }
    }

    // calc an upper bound for ImplCalcBreaks below
    long upperBoundWidth = nCalcSize * mpData->m_aItems.size();

    ImplToolItems::size_type nLines;
    ImplToolItems::size_type nCalcLines;
    ImplToolItems::size_type nTempLines;
    long    nMaxLineWidth;
    nCalcLines = ImplCalcBreaks( nCalcSize, &nMaxLineWidth, true );

    maFloatSizes.reserve( nCalcLines );

    nTempLines = nLines = nCalcLines;
    while ( nLines )
    {
        long nHeight = ImplCalcSize( nTempLines, TB_CALCMODE_FLOAT ).Height();

        ImplToolSize aSize;
        aSize.mnWidth  = nMaxLineWidth+(TB_BORDER_OFFSET1*2);
        aSize.mnHeight = nHeight;
        aSize.mnLines  = nTempLines;
        maFloatSizes.push_back( aSize );
        nLines--;
        if ( nLines )
        {
            do
            {
                nCalcSize += mnMaxItemWidth;
                nTempLines = ImplCalcBreaks( nCalcSize, &nMaxLineWidth, true );
            }
            while ((nCalcSize < upperBoundWidth) && (nLines < nTempLines)); // implies nTempLines>1
            if ( nTempLines < nLines )
                nLines = nTempLines;
        }
    }
}

Size ToolBox::ImplCalcFloatSize( ImplToolItems::size_type& rLines )
{
    ImplCalcFloatSizes();

    if ( !rLines )
    {
        rLines = mnFloatLines;
        if ( !rLines )
            rLines = mnLines;
    }

    sal_uInt16 i = 0;
    while ( i + 1u < maFloatSizes.size() && rLines < maFloatSizes[i].mnLines )
    {
        i++;
    }

    Size aSize( maFloatSizes[i].mnWidth, maFloatSizes[i].mnHeight );
    rLines = maFloatSizes[i].mnLines;

    return aSize;
}

void ToolBox::ImplCalcMinMaxFloatSize( Size& rMinSize, Size& rMaxSize )
{
    ImplCalcFloatSizes();

    sal_uInt16 i = 0;
    rMinSize = Size( maFloatSizes[i].mnWidth, maFloatSizes[i].mnHeight );
    rMaxSize = Size( maFloatSizes[i].mnWidth, maFloatSizes[i].mnHeight );
    while ( ++i < maFloatSizes.size() )
    {
        if( maFloatSizes[i].mnWidth < rMinSize.Width() )
            rMinSize.setWidth( maFloatSizes[i].mnWidth );
        if( maFloatSizes[i].mnHeight < rMinSize.Height() )
            rMinSize.setHeight( maFloatSizes[i].mnHeight );

        if( maFloatSizes[i].mnWidth > rMaxSize.Width() )
            rMaxSize.setWidth( maFloatSizes[i].mnWidth );
        if( maFloatSizes[i].mnHeight > rMaxSize.Height() )
            rMaxSize.setHeight( maFloatSizes[i].mnHeight );
    }
}

void ToolBox::ImplSetMinMaxFloatSize()
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    Size aMinSize, aMaxSize;
    ImplCalcMinMaxFloatSize( aMinSize, aMaxSize );
    if( pWrapper )
    {
        pWrapper->SetMinOutputSizePixel( aMinSize );
        pWrapper->SetMaxOutputSizePixel( aMaxSize );
        pWrapper->ShowTitleButton( TitleButton::Menu, bool( GetMenuType() & ToolBoxMenuType::Customize) );
    }
    else
    {
        // TODO: change SetMinOutputSizePixel to be not inline
        SetMinOutputSizePixel( aMinSize );
        SetMaxOutputSizePixel( aMaxSize );
    }
}

ToolBox::ImplToolItems::size_type ToolBox::ImplCalcLines( long nToolSize ) const
{
    long nLineHeight;

    if ( mbHorz )
    {
        if ( mnWinHeight > mnMaxItemHeight )
            nLineHeight = mnWinHeight;
        else
            nLineHeight = mnMaxItemHeight;
    }
    else
        nLineHeight = mnMaxItemWidth;

    if ( mnWinStyle & WB_BORDER )
        nToolSize -= TB_BORDER_OFFSET2*2;

    if ( mbLineSpacing )
    {
        nLineHeight += TB_LINESPACING;
        nToolSize += TB_LINESPACING;
    }

    // #i91917# always report at least one line
    long nLines = nToolSize/nLineHeight;
    if( nLines < 1 )
        nLines = 1;

    return nLines;
}

sal_uInt16 ToolBox::ImplTestLineSize( const Point& rPos ) const
{
    if ( !ImplIsFloatingMode() &&
         (!mbScroll || (mnLines > 1) || (mnCurLines > mnVisLines)) )
    {
        WindowAlign eAlign = GetAlign();

        if ( eAlign == WindowAlign::Left )
        {
            if ( rPos.X() > mnDX-DOCK_LINEOFFSET )
                return DOCK_LINEHSIZE | DOCK_LINERIGHT;
        }
        else if ( eAlign == WindowAlign::Top )
        {
            if ( rPos.Y() > mnDY-DOCK_LINEOFFSET )
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

void ToolBox::ImplLineSizing( const Point& rPos, tools::Rectangle& rRect, sal_uInt16 nLineMode )
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

    Size    aWinSize = GetSizePixel();
    ImplToolItems::size_type nMaxLines = std::max(mnLines, mnCurLines);
    if ( nMaxLines > TB_MAXLINES )
        nMaxLines = TB_MAXLINES;
    if ( bHorz )
    {
        nOneLineSize = ImplCalcSize( 1 ).Height();
        nMaxSize = - 20;
        if ( nMaxSize < aWinSize.Height() )
            nMaxSize = aWinSize.Height();
    }
    else
    {
        nOneLineSize = ImplCalcSize( 1 ).Width();
        nMaxSize = - 20;
        if ( nMaxSize < aWinSize.Width() )
            nMaxSize = aWinSize.Width();
    }

    ImplToolItems::size_type i = 1;
    if ( nCurSize <= nOneLineSize )
        nSize = nOneLineSize;
    else
    {
        nSize = 0;
        while ( (nSize < nCurSize) && (i < nMaxLines) )
        {
            i++;
            aSize = ImplCalcSize( i );
            if ( bHorz )
                nSize = aSize.Height();
            else
                nSize = aSize.Width();
            if ( nSize > nMaxSize )
            {
                i--;
                aSize = ImplCalcSize( i );
                if ( bHorz )
                    nSize = aSize.Height();
                else
                    nSize = aSize.Width();
                break;
            }
        }
    }

    if ( nLineMode & DOCK_LINERIGHT )
        rRect.SetRight( rRect.Left()+nSize-1 );
    else if ( nLineMode & DOCK_LINEBOTTOM )
        rRect.SetBottom( rRect.Top()+nSize-1 );
    else if ( nLineMode & DOCK_LINELEFT )
        rRect.SetLeft( rRect.Right()-nSize );
    else
        rRect.SetTop( rRect.Bottom()-nSize );

    mnDockLines = i;
}

ImplTBDragMgr::ImplTBDragMgr()
    : mpDragBox(nullptr)
    , mnLineMode(0)
    , mnStartLines(0)
{
    maAccel.InsertItem( KEY_RETURN, vcl::KeyCode( KEY_RETURN ) );
    maAccel.InsertItem( KEY_ESCAPE, vcl::KeyCode( KEY_ESCAPE ) );
    maAccel.SetSelectHdl( LINK( this, ImplTBDragMgr, SelectHdl ) );
}

void ImplTBDragMgr::StartDragging( ToolBox* pToolBox,
                                   const Point& rPos, const tools::Rectangle& rRect,
                                   sal_uInt16 nDragLineMode )
{
    mpDragBox = pToolBox;
    pToolBox->CaptureMouse();
    pToolBox->mbDragging = true;
    Application::InsertAccel( &maAccel );

    mnLineMode = nDragLineMode;
    mnStartLines = pToolBox->mnDockLines;

    // calculate MouseOffset
    maMouseOff.setX( rRect.Left() - rPos.X() );
    maMouseOff.setY( rRect.Top() - rPos.Y() );
    maRect = rRect;
    maStartRect = rRect;
    pToolBox->ShowTracking( maRect );
}

void ImplTBDragMgr::Dragging( const Point& rPos )
{
    mpDragBox->ImplLineSizing( rPos, maRect, mnLineMode );
    Point aOff = mpDragBox->OutputToScreenPixel( Point() );
    maRect.Move( aOff.X(), aOff.Y() );
    mpDragBox->Docking( rPos, maRect );
    maRect.Move( -aOff.X(), -aOff.Y() );
    mpDragBox->ShowTracking( maRect );
}

void ImplTBDragMgr::EndDragging( bool bOK )
{
    mpDragBox->HideTracking();
    if (mpDragBox->IsMouseCaptured())
        mpDragBox->ReleaseMouse();
    mpDragBox->mbDragging = false;
    Application::RemoveAccel( &maAccel );

    if ( !bOK )
    {
        mpDragBox->mnDockLines = mnStartLines;
        mpDragBox->EndDocking( maStartRect, false );
    }
    else
        mpDragBox->EndDocking( maRect, false );
    mnStartLines = 0;

    mpDragBox = nullptr;
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
    mpData.reset(new ImplToolBoxPrivateData);
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
    mnCurPos          = ITEM_NOTFOUND;
    mnLines           = 1;
    mnCurLine         = 1;
    mnCurLines        = 1;
    mnVisLines        = 1;
    mnFloatLines      = 0;
    mnDockLines       = 0;
    mnMouseModifier   = 0;
    mbDrag            = false;
    mbSelection       = false;
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
    mbDragging        = false;
    mbIsKeyEvent = false;
    mbChangingHighlight = false;
    mbImagesMirrored  = false;
    mbLineSpacing     = false;
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
    mpStatusListener->startListening();

    mpIdle.reset(new Idle("vcl::ToolBox maIdle update"));
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

    DockingWindow::ImplInit( pParent, nStyle & ~WB_BORDER );

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

        EnableChildTransparentMode(IsPaintTransparent());
    }
}

void ToolBox::doDeferredInit(WinBits nBits)
{
    VclPtr<vcl::Window> pParent = mpDialogParent;
    mpDialogParent = nullptr;
    ImplInit(pParent, nBits);
    mbIsDeferredInit = false;
}

void ToolBox::queue_resize(StateChangedType eReason)
{
    Window::queue_resize(eReason);
}

ToolBox::ToolBox( vcl::Window* pParent, WinBits nStyle ) :
    DockingWindow( WindowType::TOOLBOX )
{
    ImplInitToolBoxData();
    ImplInit( pParent, nStyle );
}

ToolBox::ToolBox(vcl::Window* pParent, const OString& rID,
    const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : DockingWindow(WindowType::TOOLBOX)
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
    // #103005# make sure our activate/deactivate balance is right
    while( mnActivateCount > 0 )
        Deactivate();

    // terminate popupmode if the floating window is
    // still connected
    if ( mpFloatWin )
        mpFloatWin->EndPopupMode( FloatWinPopupEndFlags::Cancel );
    mpFloatWin = nullptr;

    // delete private data
    mpData.reset();

    ImplSVData* pSVData = ImplGetSVData();
    delete pSVData->maCtrlData.mpTBDragMgr;
    pSVData->maCtrlData.mpTBDragMgr = nullptr;

    if (mpStatusListener.is())
        mpStatusListener->dispose();

    mpFloatWin.clear();

    mpIdle.reset();

    DockingWindow::dispose();
}

ImplToolItem* ToolBox::ImplGetItem( sal_uInt16 nItemId ) const
{
    if (!mpData)
        return nullptr;

    for (auto & item : mpData->m_aItems)
    {
        if ( item.mnId == nItemId )
            return &item;
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
        tools::Rectangle aRect( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        tools::Rectangle aReg( aRect );
        ImplControlValue aVal;
        tools::Rectangle aNativeBounds, aNativeContent;
        if( IsNativeControlSupported( ControlType::Toolbar, ControlPart::Button ) )
        {
            if( GetNativeControlRegion( ControlType::Toolbar, ControlPart::Button,
                                        aReg,
                                        ControlState::ENABLED | ControlState::ROLLOVER,
                                        aVal,
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
        aRect = tools::Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( ControlType::Combobox, ControlPart::Entire,
                                    aReg,
                                    ControlState::ENABLED | ControlState::ROLLOVER,
                                    aVal,
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
        aRect = tools::Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( ControlType::Listbox, ControlPart::Entire,
                                    aReg,
                                    ControlState::ENABLED | ControlState::ROLLOVER,
                                    aVal,
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
        aRect = tools::Rectangle( Point( 0, 0 ), Size( nMinWidth, nMinHeight ) );
        aReg = aRect;
        if( GetNativeControlRegion( ControlType::Spinbox, ControlPart::Entire,
                                    aReg,
                                    ControlState::ENABLED | ControlState::ROLLOVER,
                                    aVal,
                                    aNativeBounds, aNativeContent ) )
        {
            aRect = aNativeBounds;
            if( aRect.GetHeight() > mnWinHeight )
                mnWinHeight = aRect.GetHeight();
        }
    }

    if ( ! mpData->m_aItems.empty() )
    {
        for (auto & item : mpData->m_aItems)
        {
            item.mbVisibleText = false;  // indicates if text will definitely be drawn, influences dropdown pos

            if ( item.meType == ToolBoxItemType::BUTTON )
            {
                bool bImage;
                bool bText;

                // check if image and/or text exists
                bImage = !!item.maImage;
                bText = !item.maText.isEmpty();
                ButtonType tmpButtonType = determineButtonType( &item, meButtonType ); // default to toolbox setting
                if ( bImage || bText )
                {

                    item.mbEmptyBtn = false;

                    if ( tmpButtonType == ButtonType::SYMBOLONLY )
                    {
                        // we're drawing images only
                        if ( bImage || !bText )
                        {
                            item.maItemSize = item.maImage.GetSizePixel();
                        }
                        else
                        {
                            item.maItemSize = Size( GetCtrlTextWidth( item.maText )+TB_TEXTOFFSET,
                                                   GetTextHeight() );
                            item.mbVisibleText = true;
                        }
                    }
                    else if ( tmpButtonType == ButtonType::TEXT )
                    {
                        // we're drawing text only
                        if ( bText || !bImage )
                        {
                            item.maItemSize = Size( GetCtrlTextWidth( item.maText )+TB_TEXTOFFSET,
                                                   GetTextHeight() );
                            item.mbVisibleText = true;
                        }
                        else
                        {
                            item.maItemSize = item.maImage.GetSizePixel();
                        }
                    }
                    else
                    {
                        // we're drawing images and text
                        item.maItemSize.setWidth( bText ? GetCtrlTextWidth( item.maText )+TB_TEXTOFFSET : 0 );
                        item.maItemSize.setHeight( bText ? GetTextHeight() : 0 );

                        if ( meTextPosition == ToolBoxTextPosition::Right )
                        {
                            // leave space between image and text
                            if( bText )
                                item.maItemSize.AdjustWidth(TB_IMAGETEXTOFFSET );

                            // image and text side by side
                            item.maItemSize.AdjustWidth(item.maImage.GetSizePixel().Width() );
                            if ( item.maImage.GetSizePixel().Height() > item.maItemSize.Height() )
                                item.maItemSize.setHeight( item.maImage.GetSizePixel().Height() );
                        }
                        else
                        {
                            // leave space between image and text
                            if( bText )
                                item.maItemSize.AdjustHeight(TB_IMAGETEXTOFFSET );

                            // text below image
                            item.maItemSize.AdjustHeight(item.maImage.GetSizePixel().Height() );
                            if ( item.maImage.GetSizePixel().Width() > item.maItemSize.Width() )
                                item.maItemSize.setWidth( item.maImage.GetSizePixel().Width() );
                        }

                        item.mbVisibleText = bText;
                    }
                }
                else
                {   // no image and no text
                    item.maItemSize = Size( nDefWidth, nDefHeight );
                    item.mbEmptyBtn = true;
                }

                // save the content size
                item.maContentSize = item.maItemSize;

                // if required, take window height into consideration
                if ( item.mpWindow )
                {
                    long nHeight = item.mpWindow->GetSizePixel().Height();
                    if ( nHeight > mnWinHeight )
                        mnWinHeight = nHeight;
                }

                // add in drop down arrow
                if( item.mnBits & ToolBoxItemBits::DROPDOWN )
                {
                    item.maItemSize.AdjustWidth(nDropDownArrowWidth );
                    item.mnDropDownArrowWidth = nDropDownArrowWidth;
                }

                // text items will be rotated in vertical mode
                // -> swap width and height
                if( item.mbVisibleText && !mbHorz )
                {
                    long tmp = item.maItemSize.Width();
                    item.maItemSize.setWidth( item.maItemSize.Height() );
                    item.maItemSize.setHeight( tmp );

                    tmp = item.maContentSize.Width();
                    item.maContentSize.setWidth( item.maContentSize.Height() );
                    item.maContentSize.setHeight( tmp );
                }
            }
            else if ( item.meType == ToolBoxItemType::SPACE )
            {
                item.maItemSize = Size( nDefWidth, nDefHeight );
                item.maContentSize = item.maItemSize;
            }

            if ( item.meType == ToolBoxItemType::BUTTON || item.meType == ToolBoxItemType::SPACE )
            {
                // add borders
                long w = item.maItemSize.Width();
                long h = item.maItemSize.Height();
                ImplAddButtonBorder( w, h, mpData->mbNativeButtons );
                item.maItemSize.setWidth(w);
                item.maItemSize.setHeight(h);

                if( item.meType == ToolBoxItemType::BUTTON )
                {
                    long nMinW = std::max(nMinWidth, item.maMinimalItemSize.Width());
                    long nMinH = std::max(nMinHeight, item.maMinimalItemSize.Height());

                    long nGrowContentWidth = 0;
                    long nGrowContentHeight = 0;

                    if( item.maItemSize.Width() < nMinW )
                    {
                        nGrowContentWidth = nMinW - item.maItemSize.Width();
                        item.maItemSize.setWidth( nMinW );
                    }
                    if( item.maItemSize.Height() < nMinH )
                    {
                        nGrowContentHeight = nMinH - item.maItemSize.Height();
                        item.maItemSize.setHeight( nMinH );
                    }

                    // grow the content size by the additional available space
                    item.maContentSize.AdjustWidth(nGrowContentWidth );
                    item.maContentSize.AdjustHeight(nGrowContentHeight );
                }

                // keep track of max item size
                if ( item.maItemSize.Width() > nMaxWidth )
                    nMaxWidth = item.maItemSize.Width();
                if ( item.maItemSize.Height() > nMaxHeight )
                    nMaxHeight = item.maItemSize.Height();
            }
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
        // as this is used for alignment of multiple toolbars
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

ToolBox::ImplToolItems::size_type ToolBox::ImplCalcBreaks( long nWidth, long* pMaxLineWidth, bool bCalcHorz ) const
{
    sal_uLong           nLineStart = 0;
    sal_uLong           nGroupStart = 0;
    long            nLineWidth = 0;
    long            nCurWidth;
    long            nLastGroupLineWidth = 0;
    long            nMaxLineWidth = 0;
    ImplToolItems::size_type nLines = 1;
    bool            bWindow;
    bool            bBreak = false;
    long            nWidthTotal = nWidth;
    long nMenuWidth = 0;

    // when docked the menubutton will be in the first line
    if( IsMenuEnabled() && !ImplIsFloatingMode() )
        nMenuWidth = mpData->maMenubuttonItem.maItemSize.Width();

    // we need to know which item is the last visible one to be able to add
    // the menu width in case we are unable to show all the items
    ImplToolItems::iterator it, lastVisible;
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
    ImplToolItems::size_type nFloatLinesHeight = ImplCalcLines( mnDY );
    // calc window size according to this number
    aSize1 = ImplCalcFloatSize( nFloatLinesHeight );

    if( aCurrentSize == aSize1 )
        return aSize1;

    // try to preserve current width

    long nLineHeight = std::max( mnWinHeight, mnMaxItemHeight );
    int nBorderX = 2*TB_BORDER_OFFSET1 + mnLeftBorder + mnRightBorder;
    int nBorderY = 2*TB_BORDER_OFFSET2 + mnTopBorder + mnBottomBorder;
    Size aSz( aCurrentSize );
    long maxX;
    ImplToolItems::size_type nLines = ImplCalcBreaks( aSz.Width()-nBorderX, &maxX, mbHorz );

    ImplToolItems::size_type manyLines = 1000;
    Size aMinimalFloatSize = ImplCalcFloatSize( manyLines );

    aSz.setHeight( nBorderY + nLineHeight * nLines );
    // line space when more than one line
    if ( mbLineSpacing )
        aSz.AdjustHeight((nLines-1)*TB_LINESPACING );

    aSz.setWidth( nBorderX + maxX );

    // avoid clipping of any items
    if( aSz.Width() < aMinimalFloatSize.Width() )
        aSize2 = ImplCalcFloatSize( nLines );
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
void lcl_hideDoubleSeparators( ToolBox::ImplToolItems& rItems )
{
    bool bLastSep( true );
    ToolBox::ImplToolItems::iterator it;
    for ( it = rItems.begin(); it != rItems.end(); ++it )
    {
        if ( it->meType == ToolBoxItemType::SEPARATOR )
        {
            it->mbVisible = false;
            if ( !bLastSep )
            {
                // check if any visible items have to appear behind it
                if (std::any_of(it + 1, rItems.end(), [](const ImplToolItem& rItem) {
                        return (rItem.meType == ToolBoxItemType::BUTTON) && rItem.mbVisible; }))
                    it->mbVisible = true;
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
    tools::Rectangle       aEmptyRect;
    long            nLineSize;
    long            nLeft;
    long            nTop;
    long            nMax;   // width of layoutarea in pixels
    ImplToolItems::size_type nFormatLine;
    bool            bMustFullPaint;

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    bool bIsInPopupMode = ImplIsInPopupMode();

    maFloatSizes.clear();

    // compute border sizes
    ImplCalcBorder( meAlign, mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );

    // update drag area (where the 'grip' will be placed)
    tools::Rectangle aOldDragRect;
    if( pWrapper )
        aOldDragRect = pWrapper->GetDragArea();
    ImplUpdateDragArea();

    bMustFullPaint = ImplCalcItem();

    // calculate new size during interactive resize or
    // set computed size when formatting only
    if ( ImplIsFloatingMode() )
    {
        if ( bResize )
            mnFloatLines = ImplCalcLines( mnDY );
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
            mnVisLines  = ImplCalcLines( mnDY );
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
            mnVisLines  = ImplCalcLines( mnDX );
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

        for (auto & item : mpData->m_aItems)
        {
            item.maRect = aEmptyRect;
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
        tools::Rectangle aOldLowerRect = maLowerRect;
        tools::Rectangle aOldUpperRect = maUpperRect;
        tools::Rectangle aOldMenubuttonRect = mpData->maMenubuttonItem.maRect;
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
                            mpData->maMenubuttonItem.maRect.SetRight( mnDX - 2 );
                            mpData->maMenubuttonItem.maRect.SetTop( nTop );
                            mpData->maMenubuttonItem.maRect.SetBottom( mnDY-mnBottomBorder-TB_BORDER_OFFSET2-1 );
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.SetRight( mnDX - mnRightBorder-TB_BORDER_OFFSET1-1 );
                            mpData->maMenubuttonItem.maRect.SetTop( nTop );
                            mpData->maMenubuttonItem.maRect.SetBottom( mnDY-mnBottomBorder-TB_BORDER_OFFSET2-1 );
                        }
                        mpData->maMenubuttonItem.maRect.SetLeft( mpData->maMenubuttonItem.maRect.Right() - mpData->mnMenuButtonWidth );
                    }
                    else
                    {
                        maUpperRect.SetLeft( nLeft+nMax+TB_SPIN_OFFSET );
                        maUpperRect.SetRight( maUpperRect.Left()+TB_SPIN_SIZE-1 );
                        maUpperRect.SetTop( nTop );
                        maLowerRect.SetBottom( mnDY-mnBottomBorder-TB_BORDER_OFFSET2-1 );
                        maLowerRect.SetLeft( maUpperRect.Left() );
                        maLowerRect.SetRight( maUpperRect.Right() );
                        maUpperRect.SetBottom( maUpperRect.Top() +
                                            (maLowerRect.Bottom()-maUpperRect.Top())/2 );
                        maLowerRect.SetTop( maUpperRect.Bottom() );
                    }
                }
                else
                {
                    if( IsMenuEnabled() && !ImplHasExternalMenubutton() && !bIsInPopupMode )
                    {
                        if( !ImplIsFloatingMode() )
                        {
                            mpData->maMenubuttonItem.maRect.SetBottom( mnDY - 2 );
                            mpData->maMenubuttonItem.maRect.SetLeft( nLeft );
                            mpData->maMenubuttonItem.maRect.SetRight( mnDX-mnRightBorder-TB_BORDER_OFFSET2-1 );
                        }
                        else
                        {
                            mpData->maMenubuttonItem.maRect.SetBottom( mnDY - mnBottomBorder-TB_BORDER_OFFSET1-1 );
                            mpData->maMenubuttonItem.maRect.SetLeft( nLeft );
                            mpData->maMenubuttonItem.maRect.SetRight( mnDX-mnRightBorder-TB_BORDER_OFFSET2-1 );
                        }
                        mpData->maMenubuttonItem.maRect.SetTop( mpData->maMenubuttonItem.maRect.Bottom() - mpData->mnMenuButtonWidth );
                    }
                    else
                    {
                        maUpperRect.SetTop( nTop+nMax+TB_SPIN_OFFSET );
                        maUpperRect.SetBottom( maUpperRect.Top()+TB_SPIN_SIZE-1 );
                        maUpperRect.SetLeft( nLeft );
                        maLowerRect.SetRight( mnDX-mnRightBorder-TB_BORDER_OFFSET2-1 );
                        maLowerRect.SetTop( maUpperRect.Top() );
                        maLowerRect.SetBottom( maUpperRect.Bottom() );
                        maUpperRect.SetRight( maUpperRect.Left() +
                                            (maLowerRect.Right()-maUpperRect.Left())/2 );
                        maLowerRect.SetLeft( maUpperRect.Right() );
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

            for (auto & item : mpData->m_aItems)
            {
                item.mbShowWindow = false;

                // check for line break and advance nX/nY accordingly
                if ( item.mbBreak )
                {
                    nFormatLine++;

                    // increment starting with the second line
                    if ( nFormatLine > mnCurLine )
                    {
                        if ( mbHorz )
                        {
                            nX = nLeft;
                            if ( mbLineSpacing )
                                nY += nLineSize+TB_LINESPACING;
                            else
                                nY += nLineSize;
                        }
                        else
                        {
                            nY = nTop;
                            if ( mbLineSpacing )
                                nX += nLineSize+TB_LINESPACING;
                            else
                                nX += nLineSize;
                        }
                    }
                }

                if ( !item.mbVisible || (nFormatLine < mnCurLine) ||
                     (nFormatLine > mnCurLine+mnVisLines-1) )
                     // item is not visible
                    item.maCalcRect = aEmptyRect;
                else
                {
                    // 1. determine current item width/height
                    // take window size and orientation into account, because this affects the size of item windows

                    Size aCurrentItemSize( item.GetSize( mbHorz, mbScroll, nMax, Size(mnMaxItemWidth, mnMaxItemHeight) ) );

                    // 2. position item rect and use size from step 1
                    //  items will be centered horizontally (if mbHorz) or vertically
                    //  advance nX and nY accordingly
                    if ( mbHorz )
                    {
                        item.maCalcRect.SetLeft( nX );
                        // if special ToolBoxLayoutMode::LockVert lock vertical position
                        // don't recalculate the vertical position of the item
                        if ( meLayoutMode == ToolBoxLayoutMode::LockVert && mnLines == 1 )
                        {
                            // Somewhat of a hack here, calc deletes and re-adds
                            // the sum/assign & ok/cancel items dynamically.
                            // Because ToolBoxLayoutMode::LockVert effectively prevents
                            // recalculation of the vertical pos of an item the
                            // item.maRect.Top() for those newly added items is
                            // 0. The hack here is that we want to effectively
                            // recalculate the vertical pos for those added
                            // items here. ( Note: assume mnMaxItemHeight is
                            // equal to the LineSize when multibar has a single
                            // line size )
                            if ( item.maRect.Top() ||
                                 (item.mpWindow && item.mpWindow->GetType() == WindowType::CALCINPUTLINE) ) // tdf#83099
                            {
                                item.maCalcRect.SetTop( item.maRect.Top() );
                            }
                            else
                            {
                                item.maCalcRect.SetTop( nY+(mnMaxItemHeight-aCurrentItemSize.Height())/2 );
                            }
                        }
                        else
                            item.maCalcRect.SetTop( nY+(nLineSize-aCurrentItemSize.Height())/2 );
                        item.maCalcRect.SetRight( nX+aCurrentItemSize.Width()-1 );
                        item.maCalcRect.SetBottom( item.maCalcRect.Top()+aCurrentItemSize.Height()-1 );
                        nX += aCurrentItemSize.Width();
                    }
                    else
                    {
                        item.maCalcRect.SetLeft( nX+(nLineSize-aCurrentItemSize.Width())/2 );
                        item.maCalcRect.SetTop( nY );
                        item.maCalcRect.SetRight( item.maCalcRect.Left()+aCurrentItemSize.Width()-1 );
                        item.maCalcRect.SetBottom( nY+aCurrentItemSize.Height()-1 );
                        nY += aCurrentItemSize.Height();
                    }
                }

                // position window items into calculated item rect
                if ( item.mpWindow )
                {
                    if ( item.mbShowWindow )
                    {
                        Point aPos( item.maCalcRect.Left(), item.maCalcRect.Top() );

                        assert( item.maCalcRect.Top() >= 0 );

                        item.mpWindow->SetPosPixel( aPos );
                        item.mpWindow->Show();
                    }
                    else
                        item.mpWindow->Hide();
                }
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
                rIt.maRect.SetLeft( nX+TB_MENUBUTTON_OFFSET );
                rIt.maRect.SetTop( nY );
                rIt.maRect.SetRight( rIt.maRect.Left() + mpData->mnMenuButtonWidth );
                rIt.maRect.SetBottom( nY+nLineSize-1 );
                nX += rIt.maItemSize.Width();
            }
            else
            {
                rIt.maRect.SetLeft( nX );
                rIt.maRect.SetTop( nY+TB_MENUBUTTON_OFFSET );
                rIt.maRect.SetRight( nX+nLineSize-1 );
                rIt.maRect.SetBottom( rIt.maRect.Top() + mpData->mnMenuButtonWidth );
                nY += rIt.maItemSize.Height();
            }
        }

        // if toolbox visible trigger paint for changed regions
        if ( IsVisible() && !mbFullPaint )
        {
            if ( bMustFullPaint )
            {
                maPaintRect = tools::Rectangle( mnLeftBorder, mnTopBorder,
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

                for (auto const& item : mpData->m_aItems)
                {
                    if ( item.maRect != item.maCalcRect )
                    {
                        maPaintRect.Union( item.maRect );
                        maPaintRect.Union( item.maCalcRect );
                    }
                }
            }

            Invalidate( maPaintRect );
        }

        // store the new calculated item rects
        maPaintRect = aEmptyRect;
        for (auto & item : mpData->m_aItems)
            item.maRect = item.maCalcRect;
    }

    // indicate formatting is done
    mbFormat = false;
}

IMPL_LINK_NOARG(ToolBox, ImplDropdownLongClickHdl, Timer *, void)
{
    if (mnCurPos != ITEM_NOTFOUND &&
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

            mnCurPos         = ITEM_NOTFOUND;
            mnCurItemId      = 0;
            mnDownItemId     = 0;
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

static void ImplDrawMoreIndicator(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, bool bRotate )
{
    rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
    rRenderContext.SetLineColor();

    if (rRenderContext.GetSettings().GetStyleSettings().GetFaceColor().IsDark())
        rRenderContext.SetFillColor(COL_WHITE);
    else
        rRenderContext.SetFillColor(COL_BLACK);
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
            rRenderContext.DrawRect( tools::Rectangle( x, y, x + linewidth, y ) );
            x += space;
            rRenderContext.DrawRect( tools::Rectangle( x, y, x + linewidth, y ) );
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
            rRenderContext.DrawRect( tools::Rectangle( x, y, x, y + linewidth ) );
            y += space;
            rRenderContext.DrawRect( tools::Rectangle( x, y, x, y + linewidth ) );
            y -= space;
            x++;
            if( width <= widthOrig / 2 + 1) y--;
            else           y++;
            width--;
        }
    }

    rRenderContext.Pop();
}

static void ImplDrawDropdownArrow(vcl::RenderContext& rRenderContext, const tools::Rectangle& rDropDownRect, bool bSetColor, bool bRotate )
{
    bool bLineColor = rRenderContext.IsLineColor();
    bool bFillColor = rRenderContext.IsFillColor();
    Color aOldFillColor = rRenderContext.GetFillColor();
    Color aOldLineColor = rRenderContext.GetLineColor();
    rRenderContext.SetLineColor();

    if ( bSetColor )
    {
        if (rRenderContext.GetSettings().GetStyleSettings().GetFaceColor().IsDark())
            rRenderContext.SetFillColor(COL_WHITE);
        else
            rRenderContext.SetFillColor(COL_BLACK);
    }

    float fScaleFactor = rRenderContext.GetDPIScaleFactor();

    tools::Polygon aPoly(4);

    long width = round(rDropDownRect.getHeight()/5.5) * fScaleFactor; // scale triangle depending on theme/toolbar height with 7 for gtk, 5 for gen
    long height = round(rDropDownRect.getHeight()/9.5) * fScaleFactor; // 4 for gtk, 3 for gen
    if (width < 4) width = 4;
    if (height < 3) height = 3;

    long x = rDropDownRect.Left() + (rDropDownRect.getWidth() - width)/2;
    long y = rDropDownRect.Top() + (rDropDownRect.getHeight() - height)/2;

    long halfwidth = (width+1)>>1;
    aPoly.SetPoint(Point(x, y), 0);
    aPoly.SetPoint(Point(x + halfwidth, y + height), 1);
    aPoly.SetPoint(Point(x + halfwidth*2, y), 2);
    aPoly.SetPoint(Point(x, y), 3);

    if (bRotate) // TESTME: harder ...
        aPoly.Rotate(Point(x,y+height/2),2700);

    auto aaflags = rRenderContext.GetAntialiasing();
    rRenderContext.SetAntialiasing(AntialiasingFlags::EnableB2dDraw);
    rRenderContext.DrawPolygon( aPoly );
    rRenderContext.SetAntialiasing(aaflags);

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
        ImplCheckUpdate();

        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);

        // draw the 'more' indicator / button (>>)
        ImplErase(rRenderContext, mpData->maMenubuttonItem.maRect, bHighlight);

        if (bHighlight)
            ImplDrawButton(rRenderContext, mpData->maMenubuttonItem.maRect, 2, false, true, false );

        if (ImplHasClippedItems())
            ImplDrawMoreIndicator(rRenderContext, mpData->maMenubuttonItem.maRect, !mbHorz);

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

    bTmpUpper = mnCurLine > 1;

    bTmpLower = mnCurLine+mnVisLines-1 < mnCurLines;

    if ( !IsEnabled() )
    {
        bTmpUpper = false;
        bTmpLower = false;
    }

    ImplDrawUpDownButtons(rRenderContext, maUpperRect, maLowerRect,
                          false/*bUpperIn*/, false/*bLowerIn*/, bTmpUpper, bTmpLower, !mbHorz);
}

void ToolBox::ImplDrawSeparator(vcl::RenderContext& rRenderContext, ImplToolItems::size_type nPos, const tools::Rectangle& rRect)
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
        bNativeOk = rRenderContext.DrawNativeControl(ControlType::Toolbar, nPart, rRect, ControlState::NONE, aControlValue, OUString());
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

void ToolBox::ImplDrawButton(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect, sal_uInt16 highlight,
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

void ToolBox::ImplDrawItem(vcl::RenderContext& rRenderContext, ImplToolItems::size_type nPos, sal_uInt16 nHighlight)
{
    if (nPos >= mpData->m_aItems.size())
        return;

    // execute pending paint requests
    ImplCheckUpdate();

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
    if (rStyleSettings.GetFaceColor() == COL_WHITE)
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
            tools::Rectangle aArrowRect = pItem->GetDropDownRect( mbHorz );
            if( aArrowRect.Top() == pItem->maRect.Top() ) // dropdown arrow on right side
                aBtnSize.AdjustWidth( -(aArrowRect.GetWidth()) );
            else if ( !( (meTextPosition == ToolBoxTextPosition::Bottom)
                        && ((pItem->mnBits & ToolBoxItemBits::DROPDOWNONLY) == ToolBoxItemBits::DROPDOWNONLY) ) )
                aBtnSize.AdjustHeight( -(aArrowRect.GetHeight()) ); // dropdown arrow on bottom side
        }
    }

    /* Compute the button/separator rectangle here, we'll need it for
     * both the buttons and the separators. */
    tools::Rectangle aButtonRect( pItem->maRect.TopLeft(), aBtnSize );
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
    if ( (pItem->meType != ToolBoxItemType::BUTTON) || pItem->mbShowWindow )
        return;

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
        aTxtSize.setWidth( GetCtrlTextWidth( pItem->maText ) );
        aTxtSize.setHeight( GetTextHeight() );
    }

    if ( bImage )
    {
        const Image* pImage = &(pItem->maImage);
        const Image& rOverlayImage = pItem->maOverlayImage;
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
        if (!!rOverlayImage)
            rRenderContext.DrawImage(Point( nImageOffX, nImageOffY ), rOverlayImage, nImageStyle);
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
                // center horizontally
                nTextOffX += (nBtnWidth-aTxtSize.Width() - TB_IMAGETEXTOFFSET)/2;
                // set vertical position
                nTextOffY += nBtnHeight - aTxtSize.Height();
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
        tools::Rectangle aDropDownRect( pItem->GetDropDownRect( mbHorz ) );
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
}

void ToolBox::ImplDrawFloatwinBorder(vcl::RenderContext& rRenderContext, ImplToolItem const * pItem)
{
    if ( pItem->maRect.IsEmpty() )
        return;

    tools::Rectangle aRect( mpFloatWin->ImplGetItemEdgeClipRect() );
    aRect.SetPos( AbsoluteScreenToOutputPixel( aRect.TopLeft() ) );
    rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetShadowColor());
    Point p1, p2;

    p1 = pItem->maRect.TopLeft();
    p1.AdjustX( 1 );
    p2 = pItem->maRect.TopRight();
    p2.AdjustX( -1 );
    rRenderContext.DrawLine( p1, p2);
    p1 = pItem->maRect.BottomLeft();
    p1.AdjustX( 1 );
    p2 = pItem->maRect.BottomRight();
    p2.AdjustX( -1 );
    rRenderContext.DrawLine( p1, p2);

    p1 = pItem->maRect.TopLeft();
    p1.AdjustY( 1 );
    p2 = pItem->maRect.BottomLeft();
    p2.AdjustY( -1 );
    rRenderContext.DrawLine( p1, p2);
    p1 = pItem->maRect.TopRight();
    p1.AdjustY( 1 );
    p2 = pItem->maRect.BottomRight();
    p2.AdjustY( -1 );
    rRenderContext.DrawLine( p1, p2);

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

        if ( mnCurPos != ITEM_NOTFOUND )
            InvalidateItem(mnCurPos);
        Deactivate();

        if( !bWasKeyboardActivate )
        {
            mnCurPos = ITEM_NOTFOUND;
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
        ImplToolItems::size_type delta = mnVisLines;
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
    if ( mbDrag && mnCurPos != ITEM_NOTFOUND )
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
            mnMouseModifier  = rMEvt.GetModifier();

        Deactivate();

        if ( mbDrag )
            mbDrag = false;
        else
        {
            mbSelection = false;
            if ( mnCurPos == ITEM_NOTFOUND )
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
                    if ( mnCurPos != ITEM_NOTFOUND )
                    {
                        InvalidateItem(mnCurPos);
                        Flush();
                    }
                }
            }
        }

        mnCurPos         = ITEM_NOTFOUND;
        mnCurItemId      = 0;
        mnDownItemId     = 0;
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
        ImplToolItems::size_type i = 0;
        ImplToolItems::size_type nNewPos = ITEM_NOTFOUND;

        // search the item that has been clicked
        for (auto const& item : mpData->m_aItems)
        {
            // if the mouse position is in this item,
            // we can stop the search
            if ( item.maRect.IsInside( aMousePos ) )
            {
                // select it if it is a button
                if ( item.meType == ToolBoxItemType::BUTTON )
                {
                    // if button is disabled, do not
                    // change it
                    if ( !item.mbEnabled || item.mbShowWindow )
                        nNewPos = mnCurPos;
                    else
                        nNewPos = i;
                }
                break;
            }
            ++i;
        }

        // was a new entry selected?
        // don't change selection if keyboard selection is active and
        // mouse leaves the toolbox
        if ( nNewPos != mnCurPos && !( HasFocus() && nNewPos == ITEM_NOTFOUND ) )
        {
            if ( mnCurPos != ITEM_NOTFOUND )
            {
                InvalidateItem(mnCurPos);
                CallEventListeners( VclEventId::ToolboxHighlightOff, reinterpret_cast< void* >( mnCurPos ) );
            }

            mnCurPos = nNewPos;
            if ( mnCurPos != ITEM_NOTFOUND )
            {
                mnCurItemId = mnHighItemId = mpData->m_aItems[mnCurPos].mnId;
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
            sal_uInt16 nLinePtr = ImplTestLineSize( rMEvt.GetPosPixel() );
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

    if ( bDrawHotSpot && ( (mnOutStyle & TOOLBOX_STYLE_FLAT) || !mnOutStyle ) )
    {
        bool bClearHigh = true;
        if ( !rMEvt.IsLeaveWindow() && (mnCurPos == ITEM_NOTFOUND) )
        {
            ImplToolItems::size_type nTempPos = 0;
            for (auto const& item : mpData->m_aItems)
            {
                if ( item.maRect.IsInside( aMousePos ) )
                {
                    if ( (item.meType == ToolBoxItemType::BUTTON) && item.mbEnabled )
                    {
                        if ( !mnOutStyle || (mnOutStyle & TOOLBOX_STYLE_FLAT) )
                        {
                            bClearHigh = false;
                            if ( mnHighItemId != item.mnId )
                            {
                                if ( mnHighItemId )
                                {
                                    ImplHideFocus();
                                    ImplToolItems::size_type nPos = GetItemPos( mnHighItemId );
                                    InvalidateItem(nPos);
                                    CallEventListeners( VclEventId::ToolboxHighlightOff, reinterpret_cast< void* >( nPos ) );
                                }
                                if ( mpData->mbMenubuttonSelected )
                                {
                                    // remove highlight from menubutton
                                    InvalidateMenuButton();
                                }
                                mnHighItemId = item.mnId;
                                InvalidateItem(nTempPos);
                                ImplShowFocus();
                                CallEventListeners( VclEventId::ToolboxHighlight );
                            }
                        }
                    }
                    break;
                }
                ++nTempPos;
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
                ImplToolItems::size_type nClearPos = GetItemPos( mnHighItemId );
                if ( nClearPos != ITEM_NOTFOUND )
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
        SetPointer( eStyle );
    }

    DockingWindow::MouseMove( rMEvt );
}

void ToolBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    // only trigger toolbox for left mouse button and when
    // we're not in normal operation
    if ( rMEvt.IsLeft() && !mbDrag && (mnCurPos == ITEM_NOTFOUND) )
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
        ImplToolItems::size_type i = 0;
        ImplToolItems::size_type nNewPos = ITEM_NOTFOUND;

        // search for item that was clicked
        for (auto const& item : mpData->m_aItems)
        {
            // is this the item?
            if ( item.maRect.IsInside( aMousePos ) )
            {
                // do nothing if it is a separator or
                // if the item has been disabled
                if ( (item.meType == ToolBoxItemType::BUTTON) &&
                     !item.mbShowWindow )
                    nNewPos = i;

                break;
            }

            i++;
        }

        // item found
        if ( nNewPos != ITEM_NOTFOUND )
        {
            if ( !mpData->m_aItems[nNewPos].mbEnabled )
            {
                Deactivate();
                return;
            }

            // update actual data
            StartTrackingFlags nTrackFlags = StartTrackingFlags::NONE;
            mnCurPos         = i;
            mnCurItemId      = mpData->m_aItems[nNewPos].mnId;
            mnDownItemId     = mnCurItemId;
            mnMouseModifier  = rMEvt.GetModifier();
            if ( mpData->m_aItems[nNewPos].mnBits & ToolBoxItemBits::REPEAT )
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
                if( mpData->m_aItems[nNewPos].mnBits & ToolBoxItemBits::DROPDOWN )
                {
                    if( ( (mpData->m_aItems[nNewPos].mnBits & ToolBoxItemBits::DROPDOWNONLY) == ToolBoxItemBits::DROPDOWNONLY)
                        || mpData->m_aItems[nNewPos].GetDropDownRect( mbHorz ).IsInside( aMousePos ))
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

                            mnCurPos         = ITEM_NOTFOUND;
                            mnCurItemId      = 0;
                            mnDownItemId     = 0;
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
            if ( maMenuButtonHdl.IsSet() )
                maMenuButtonHdl.Call( this );
            else
                ExecuteCustomMenu( mpData->maMenubuttonItem.maRect );
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
            sal_uInt16 nLineMode = ImplTestLineSize( aMousePos );
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
                pMgr->StartDragging( this, aMousePos, tools::Rectangle( aPos, aSize ),
                                     nLineMode );
                return;
            }
        }

        // no item, then only click or double click
        if ( rMEvt.GetClicks() == 2 )
            DoubleClick();
        else
            Click();
    }

    if ( !mbDrag && !mbSelection && (mnCurPos == ITEM_NOTFOUND) )
        DockingWindow::MouseButtonDown( rMEvt );
}

void ToolBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( ImplHandleMouseButtonUp( rMEvt ) )
        return;

    if ( mbDragging && rMEvt.IsLeft() )
    {
        ImplTBDragMgr* pMgr = ImplGetTBDragMgr();
        pMgr->EndDragging();
        return;
    }

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

void ToolBox::InvalidateItem(ImplToolItems::size_type nPosition)
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

void ToolBox::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rPaintRect)
{
    if( mpData->mbIsPaintLocked )
        return;

    if (rPaintRect == tools::Rectangle(0, 0, mnDX-1, mnDY-1))
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
    ImplToolItems::size_type nHighPos;
    if ( mnHighItemId )
        nHighPos = GetItemPos( mnHighItemId );
    else
        nHighPos = ITEM_NOTFOUND;

    ImplToolItems::size_type nCount = mpData->m_aItems.size();
    for( ImplToolItems::size_type i = 0; i < nCount; i++ )
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
            aWinSize.setWidth( aPrefSize.Width() );
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
                    tools::Rectangle aBounds;
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
                        aWinSize.setWidth( aPrefSize.Width() + nDiff );
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
                    Invalidate( tools::Rectangle( mnDX-mnRightBorder-1, 0, mnDX, mnDY ) );
                else
                    Invalidate( tools::Rectangle( nOldDX-mnRightBorder-1, 0, nOldDX, nOldDY ) );
            }

            if ( mnBottomBorder )
            {
                if ( nOldDY > mnDY )
                    Invalidate( tools::Rectangle( 0, mnDY-mnBottomBorder-1, mnDX, mnDY ) );
                else
                    Invalidate( tools::Rectangle( 0, nOldDY-mnBottomBorder-1, nOldDX, nOldDY ) );
            }
        }
    }
}

namespace
{
    bool DispatchableCommand(const OUString& rName)
    {
        return rName.startsWith(".uno")  ||
               rName.startsWith("slot:")  ||
               rName.startsWith("macro:")  ||
               rName.startsWith("vnd.sun.star.script");
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
            if (DispatchableCommand(pItem->maCommandStr))
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
        tools::Rectangle aRect( GetItemRect( nItemId ) );
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
            tools::Rectangle aTempRect = GetItemRect( nItemId );
            Point aPt = OutputToScreenPixel( aTempRect.TopLeft() );
            aTempRect.SetLeft( aPt.X() );
            aTempRect.SetTop( aPt.Y() );
            aPt = OutputToScreenPixel( aTempRect.BottomRight() );
            aTempRect.SetRight( aPt.X() );
            aTempRect.SetBottom( aPt.Y() );

            // get text and display it
            OUString aStr = GetQuickHelpText( nItemId );
            if (aStr.isEmpty())
                aStr = MnemonicGenerator::EraseAllMnemonicChars( GetItemText( nItemId ) );
            if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
            {
                const OUString& rHelpStr = GetHelpText( nItemId );
                if (!rHelpStr.isEmpty())
                    aStr = rHelpStr;
                Help::ShowBalloon( this, aHelpPos, aTempRect, aStr );
            }
            else
                Help::ShowQuickHelp( this, aTempRect, aStr, QuickHelpFlags::CtrlText );
            return;
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

                if( bNoTabCycling )
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
            for (auto const& item : mpData->m_aItems)
            {
                if ( item.mbVisible )
                {
                    if ( item.mpWindow && item.mpWindow->ImplIsWindowOrChild( rNEvt.GetWindow() ) )
                    {
                        mnHighItemId = item.mnId;
                        break;
                    }
                }
            }
            return DockingWindow::EventNotify( rNEvt );
        }
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        // deselect
        ImplHideFocus();
        mnHighItemId = 0;
        mnCurPos = ITEM_NOTFOUND;
    }

    return DockingWindow::EventNotify( rNEvt );
}

void ToolBox::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::Wheel )
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
    else if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        ExecuteCustomMenu( tools::Rectangle( rCEvt.GetMousePosPixel(), rCEvt.GetMousePosPixel() ) );
        return;
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
        OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(mpStatusListener->getFrame()));
        for (auto const& item : mpData->m_aItems)
        {
            if (vcl::CommandInfoProvider::IsMirrored(item.maCommandStr, aModuleName))
                SetItemImageMirrorMode(item.mnId, mbImagesMirrored);
            if (vcl::CommandInfoProvider::IsRotated(item.maCommandStr, aModuleName))
                SetItemImageAngle(item.mnId, mnImagesRotationAngle);
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

        ImplSetMinMaxFloatSize();
        SetOutputSizePixel( ImplCalcFloatSize( mnFloatLines ) );
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

bool ToolBox::Docking( const Point& rPos, tools::Rectangle& rRect )
{
    // do nothing during dragging, it was calculated before
    if ( mbDragging )
        return false;

    bool bFloatMode = false;

    DockingWindow::Docking( rPos, rRect );

    // if the mouse is outside the area, it can only become a floating window
    tools::Rectangle aDockingRect( rRect );
    if ( !ImplIsFloatingMode() )
    {
        // don't use tracking rectangle for alignment check, because it will be too large
        // to get a floating mode as result - switch to floating size
        // so the calculation only depends on the position of the rectangle, not the current
        // docking state of the window
        ImplToolItems::size_type nTemp = 0;
        aDockingRect.SetSize( ImplCalcFloatSize( nTemp ) );

        // in this mode docking is never done by keyboard, so it's OK to use the mouse position
        aDockingRect.SetPos( ImplGetFrameWindow()->GetPointerPosPixel() );
    }

    bFloatMode = true;

    meDockAlign = meAlign;
    if ( !mbLastFloatMode )
    {
        ImplToolItems::size_type nTemp = 0;
        aDockingRect.SetSize( ImplCalcFloatSize( nTemp ) );
    }

    rRect = aDockingRect;
    mbLastFloatMode = bFloatMode;

    return bFloatMode;
}

void ToolBox::EndDocking( const tools::Rectangle& rRect, bool bFloatMode )
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

void ToolBox::Resizing( Size& rSize )
{
    ImplToolItems::size_type nCalcLines;
    ImplToolItems::size_type nTemp;

    // calculate all floating sizes
    ImplCalcFloatSizes();

    if ( !mnLastResizeDY )
        mnLastResizeDY = mnDY;

    // is vertical resizing needed
    if ( (mnLastResizeDY != rSize.Height()) && (mnDY != rSize.Height()) )
    {
        nCalcLines = ImplCalcLines( rSize.Height() );
        if ( nCalcLines < 1 )
            nCalcLines = 1;
        rSize = ImplCalcFloatSize( nCalcLines );
    }
    else
    {
        nCalcLines = 1;
        nTemp = nCalcLines;
        Size aTempSize = ImplCalcFloatSize( nTemp );
        while ( (aTempSize.Width() > rSize.Width()) &&
                (nCalcLines <= maFloatSizes[0].mnLines) )
        {
            nCalcLines++;
            nTemp = nCalcLines;
            aTempSize = ImplCalcFloatSize( nTemp );
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
            aWinSize.setWidth( aPrefSize.Width() );
            pWindow->SetSizePixel(aWinSize);
        }
    }

    Size aSize(const_cast<ToolBox *>(this)->ImplCalcSize( mnLines ));

    for (auto const& expandable : aExpandables)
    {
        vcl::Window *pWindow = expandable.first;
        Size aWinSize = expandable.second;
        pWindow->SetSizePixel(aWinSize);
    }

    return aSize;
}

Size ToolBox::CalcWindowSizePixel( ImplToolItems::size_type nCalcLines )
{
    return ImplCalcSize( nCalcLines );
}

Size ToolBox::CalcWindowSizePixel( ImplToolItems::size_type nCalcLines, WindowAlign eAlign )
{
    return ImplCalcSize( nCalcLines,
        (eAlign == WindowAlign::Top || eAlign == WindowAlign::Bottom) ? TB_CALCMODE_HORZ : TB_CALCMODE_VERT );
}

ToolBox::ImplToolItems::size_type ToolBox::ImplCountLineBreaks() const
{
    ImplToolItems::size_type nLines = 0;

    for (auto const& item : mpData->m_aItems)
    {
        if( item.meType == ToolBoxItemType::BREAK )
            ++nLines;
    }
    return nLines;
}

Size ToolBox::CalcPopupWindowSizePixel()
{
    // count number of breaks and calc corresponding floating window size
    ImplToolItems::size_type nLines = ImplCountLineBreaks();

    if( nLines )
        ++nLines;   // add the first line
    else
    {
        // no breaks found: use quadratic layout
        nLines = static_cast<ImplToolItems::size_type>(ceil( sqrt( static_cast<double>(GetItemCount()) ) ));
    }

    bool bPopup = mpData->mbAssumePopupMode;
    mpData->mbAssumePopupMode = true;

    Size aSize = CalcFloatingWindowSizePixel( nLines );

    mpData->mbAssumePopupMode = bPopup;
    return aSize;
}

Size ToolBox::CalcFloatingWindowSizePixel()
{
    ImplToolItems::size_type nLines = ImplCountLineBreaks();
    ++nLines; // add the first line
    return CalcFloatingWindowSizePixel( nLines );
}

Size ToolBox::CalcFloatingWindowSizePixel( ImplToolItems::size_type nCalcLines )
{
    bool bFloat = mpData->mbAssumeFloating;
    bool bDocking = mpData->mbAssumeDocked;

    // simulate floating mode and force reformat before calculating
    mpData->mbAssumeFloating = true;
    mpData->mbAssumeDocked = false;

    Size aSize = ImplCalcFloatSize( nCalcLines );

    mbFormat = true;
    mpData->mbAssumeFloating = bFloat;
    mpData->mbAssumeDocked = bDocking;

    return aSize;
}

Size ToolBox::CalcMinimumWindowSizePixel()
{
    if( ImplIsFloatingMode() )
        return ImplCalcSize( mnFloatLines );
    else
    {
        // create dummy toolbox for measurements
        VclPtrInstance< ToolBox > pToolBox( GetParent(), GetStyle() );

        // copy until first useful item
        for (auto const& item : mpData->m_aItems)
        {
            pToolBox->CopyItem( *this, item.mnId );
            if( (item.meType == ToolBoxItemType::BUTTON) &&
                item.mbVisible && !ImplIsFixedControl( &item ) )
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
    mbCustomize = bEnable;
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
    vcl::KeyCode aKeyCode( 0, 0 );
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

static bool ImplCloseLastPopup( vcl::Window const *pParent )
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

    if( mpData->mbMenubuttonSelected )
    {
        if( ImplCloseLastPopup( GetParent() ) )
            return bRet;

        if ( maMenuButtonHdl.IsSet() )
            maMenuButtonHdl.Call( this );
        else
            ExecuteCustomMenu( mpData->maMenubuttonItem.maRect );
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
        mbIsKeyEvent = true;
        Activate();

        mpData->mbDropDownByKeyboard = true;
        mpData->maDropdownClickHdl.Call( this );

        mbIsKeyEvent = false;
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
                ImplToolItem *pItem = ImplGetItem(mnHighItemId);
                if (!pItem || !pItem->mbEnabled)
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
        vcl::Window *pFocusControl = pParent->ImplGetDlgWindow( 0, GetDlgWindowType::First );
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
ToolBox::ImplToolItems::size_type ToolBox::ImplGetItemLine( ImplToolItem const * pCurrentItem )
{
    ImplToolItems::size_type nLine = 1;
    for (auto const& item : mpData->m_aItems)
    {
        if ( item.mbBreak )
            ++nLine;
        if( &item == pCurrentItem)
            break;
    }
    return nLine;
}

// returns the first displayable item in the given line
ImplToolItem* ToolBox::ImplGetFirstValidItem( ImplToolItems::size_type nLine )
{
    if( !nLine || nLine > mnCurLines )
        return nullptr;

    nLine--;

    ImplToolItems::iterator it = mpData->m_aItems.begin();
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

ToolBox::ImplToolItems::size_type ToolBox::ImplFindItemPos( const ImplToolItem* pItem, const ImplToolItems& rList )
{
    if( pItem )
    {
        for( ImplToolItems::size_type nPos = 0; nPos < rList.size(); ++nPos )
            if( &rList[ nPos ] == pItem )
                return nPos;
    }
    return ITEM_NOTFOUND;
}

void ToolBox::ChangeHighlight( ImplToolItems::size_type nPos )
{
    if ( nPos < GetItemCount() ) {
        ImplGrabFocus( GetFocusFlags::NONE );
        ImplChangeHighlight ( ImplGetItem ( GetItemId ( nPos ) ) );
    }
}

void ToolBox::ImplChangeHighlight( ImplToolItem const * pItem, bool bNoGrabFocus )
{
    // avoid recursion due to focus change
    if( mbChangingHighlight )
        return;

    mbChangingHighlight = true;

    ImplToolItem* pOldItem = nullptr;

    if ( mnHighItemId )
    {
        ImplHideFocus();
        ImplToolItems::size_type nPos = GetItemPos( mnHighItemId );
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
        ImplToolItems::size_type aPos = ToolBox::ImplFindItemPos( pItem, mpData->m_aItems );
        if( aPos != ITEM_NOTFOUND)
        {
            // check for line breaks
            ImplToolItems::size_type nLine = ImplGetItemLine( pItem );

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
        mnCurPos = ITEM_NOTFOUND;
    }

    mbChangingHighlight = false;
}

// check for keyboard accessible items
static bool ImplIsValidItem( const ImplToolItem* pItem, bool bNotClipped )
{
    bool bValid = (pItem && pItem->meType == ToolBoxItemType::BUTTON && pItem->mbVisible && !ImplIsFixedControl( pItem )
                   && pItem->mbEnabled);
    if( bValid && bNotClipped && pItem->IsClipped() )
        bValid = false;
    return bValid;
}

bool ToolBox::ChangeHighlightUpDn( bool bUp )
{
    return ImplChangeHighlightUpDn(bUp, /*bNoCycle*/ false);
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
                ImplToolItem* pItem = nullptr;
                auto it = std::find_if(mpData->m_aItems.rbegin(), mpData->m_aItems.rend(),
                    [](const ImplToolItem& rItem) { return ImplIsValidItem( &rItem, true ); });
                if( it != mpData->m_aItems.rend() )
                    pItem = &(*it);

                InvalidateMenuButton();
                ImplChangeHighlight( pItem );
            }
            else
            {
                // select first valid non-clipped item
                ImplToolItems::iterator it = std::find_if(mpData->m_aItems.begin(), mpData->m_aItems.end(),
                    [](const ImplToolItem& rItem) { return ImplIsValidItem( &rItem, true ); });
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
            ImplToolItems::iterator it = std::find_if(mpData->m_aItems.begin(), mpData->m_aItems.end(),
                [](const ImplToolItem& rItem) { return ImplIsValidItem( &rItem, false ); });

            // select the menu button if a clipped item would be selected
            if( (it != mpData->m_aItems.end() && &(*it) == ImplGetFirstClippedItem()) && IsMenuEnabled() )
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
                ImplToolItem* pItem = nullptr;
                auto it = std::find_if(mpData->m_aItems.rbegin(), mpData->m_aItems.rend(),
                    [](const ImplToolItem& rItem) { return ImplIsValidItem( &rItem, false ); });
                if( it != mpData->m_aItems.rend() )
                    pItem = &(*it);

                ImplChangeHighlight( pItem );
            }
            return true;
        }
    }

    assert(pToolItem);

    ImplToolItems::size_type pos = ToolBox::ImplFindItemPos( pToolItem, mpData->m_aItems );
    ImplToolItems::size_type nCount = mpData->m_aItems.size();

    ImplToolItems::size_type i=0;
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

    return true;
}

void ToolBox::ImplShowFocus()
{
    if( mnHighItemId && HasFocus() )
    {
        ImplToolItem* pItem = ImplGetItem( mnHighItemId );
        if (pItem && pItem->mpWindow && !pItem->mpWindow->IsDisposed())
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

        if( ERROR_SUCCESS == RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\LibreOffice\\Accessibility\\AtToolSupport", &hkey) )
        {
            DWORD dwType = 0;
            wchar_t Data[6]; // possible values: "true", "false", "1", "0", DWORD
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueExW(hkey, L"DisableFlatToolboxButtons",
                nullptr, &dwType, reinterpret_cast<LPBYTE>(Data), &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        bValue = ((0 == wcsicmp(Data, L"1")) || (0 == wcsicmp(Data, L"true")));
                        break;
                    case REG_DWORD:
                        bValue = static_cast<bool>(reinterpret_cast<DWORD *>(Data)[0]);
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
