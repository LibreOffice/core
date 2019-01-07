/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// FIX fdo#38246 https://bugs.libreoffice.org/show_bug.cgi?id=38246
// Design proposal: https://wiki.documentfoundation.org/Design/Whiteboards/Comments_Ruler_Control
// TODO Alpha blend border when it doesn't fit in window

#include <swruler.hxx>

#include <viewsh.hxx>
#include <edtwin.hxx>
#include <PostItMgr.hxx>
#include <viewopt.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <sfx2/request.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <strings.hrc>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <boost/property_tree/json_parser.hpp>

#define CONTROL_BORDER_WIDTH    1
#define CONTROL_LEFT_OFFSET     6
#define CONTROL_RIGHT_OFFSET    3
#define CONTROL_TOP_OFFSET      4

#define CONTROL_TRIANGLE_WIDTH  4
#define CONTROL_TRIANGLE_PAD    3

namespace {

/**
 * Draw a little horizontal arrow tip on VirtualDevice.
 * \param nX left coordinate of arrow
 * \param nY top coordinate of arrow
 * \param Color arrow color
 * \param bPointRight if arrow should point to right. Otherwise, it will point left.
 */
void ImplDrawArrow(vcl::RenderContext& rRenderContext, long nX, long nY, const Color& rColor, bool bPointRight)
{
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(rColor);
    if (bPointRight)
    {
        rRenderContext.DrawRect(tools::Rectangle(nX + 0, nY + 0, nX + 0, nY + 6) );
        rRenderContext.DrawRect(tools::Rectangle(nX + 1, nY + 1, nX + 1, nY + 5) );
        rRenderContext.DrawRect(tools::Rectangle(nX + 2, nY + 2, nX + 2, nY + 4) );
        rRenderContext.DrawRect(tools::Rectangle(nX + 3, nY + 3, nX + 3, nY + 3) );
    }
    else
    {
        rRenderContext.DrawRect(tools::Rectangle(nX + 0, nY + 3, nX + 0, nY + 3));
        rRenderContext.DrawRect(tools::Rectangle(nX + 1, nY + 2, nX + 1, nY + 4));
        rRenderContext.DrawRect(tools::Rectangle(nX + 2, nY + 1, nX + 2, nY + 5));
        rRenderContext.DrawRect(tools::Rectangle(nX + 3, nY + 0, nX + 3, nY + 6));
    }
}

}

// Constructor
SwCommentRuler::SwCommentRuler( SwViewShell* pViewSh, vcl::Window* pParent, SwEditWin* pWin, SvxRulerSupportFlags nRulerFlags,  SfxBindings& rBindings, WinBits nWinStyle)
: SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle | WB_HSCROLL)
, mpViewShell(pViewSh)
, mpSwWin(pWin)
, mbIsHighlighted(false)
, mnFadeRate(0)
, maVirDev( VclPtr<VirtualDevice>::Create(*this) )
{
    // Set fading timeout: 5 x 40ms = 200ms
    maFadeTimer.SetTimeout(40);
    maFadeTimer.SetInvokeHandler( LINK( this, SwCommentRuler, FadeHandler ) );
    maFadeTimer.SetDebugName( "sw::SwCommentRuler maFadeTimer" );
}

// Destructor
SwCommentRuler::~SwCommentRuler()
{
    disposeOnce();
}

void SwCommentRuler::dispose()
{
    mpSwWin.clear();
    SvxRuler::dispose();
}

void SwCommentRuler::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (comphelper::LibreOfficeKit::isActive())
        return; // no need to waste time on startup

    SvxRuler::Paint(rRenderContext, rRect);

    // Don't draw if there is not any note
    if (mpViewShell->GetPostItMgr() && mpViewShell->GetPostItMgr()->HasNotes())
        DrawCommentControl(rRenderContext);
}

void SwCommentRuler::DrawCommentControl(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    bool bIsCollapsed = ! mpViewShell->GetPostItMgr()->ShowNotes();

    tools::Rectangle aControlRect = GetCommentControlRegion();
    maVirDev->SetOutputSizePixel(aControlRect.GetSize());

    // Paint comment control background
    // TODO Check if these are best colors to be used
    Color aBgColor = GetFadedColor( rStyleSettings.GetDarkShadowColor(), rStyleSettings.GetWorkspaceColor() );
    maVirDev->SetFillColor( aBgColor );

    if ( mbIsHighlighted || !bIsCollapsed )
    {
        // Draw borders
        maVirDev->SetLineColor( rStyleSettings.GetShadowColor() );
    }
    else
    {
        // No borders
        maVirDev->SetLineColor();
    }

    maVirDev->DrawRect( tools::Rectangle( Point(), aControlRect.GetSize() ) );

    // Label and arrow tip
    OUString aLabel( SwResId ( STR_COMMENTS_LABEL ) );
    // Get label and arrow coordinates
    Point aLabelPos;
    Point aArrowPos;
    bool  bArrowToRight;
    // TODO Discover why it should be 0 instead of CONTROL_BORDER_WIDTH + CONTROL_TOP_OFFSET
    aLabelPos.setY( 0 );
    aArrowPos.setY( CONTROL_BORDER_WIDTH + CONTROL_TOP_OFFSET );
    if ( !AllSettings::GetLayoutRTL() )
    {
        // LTR
        if ( bIsCollapsed )
        {
            // It should draw something like | > Comments  |
            aLabelPos.setX( CONTROL_LEFT_OFFSET + CONTROL_TRIANGLE_WIDTH + CONTROL_TRIANGLE_PAD );
            aArrowPos.setX( CONTROL_LEFT_OFFSET );
        }
        else
        {
            // It should draw something like | Comments  < |
            aLabelPos.setX( CONTROL_LEFT_OFFSET );
            aArrowPos.setX( aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - CONTROL_TRIANGLE_WIDTH );
        }
        bArrowToRight = bIsCollapsed;
    }
    else
    {
        // RTL
        long nLabelWidth = GetTextWidth( aLabel );
        if ( bIsCollapsed )
        {
            // It should draw something like |  Comments < |
            aArrowPos.setX( aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - CONTROL_TRIANGLE_WIDTH );
            aLabelPos.setX( aArrowPos.X() - CONTROL_TRIANGLE_PAD - nLabelWidth );
        }
        else
        {
            // It should draw something like | >  Comments |
            aLabelPos.setX( aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - nLabelWidth );
            aArrowPos.setX( CONTROL_LEFT_OFFSET );
        }
        bArrowToRight = !bIsCollapsed;
    }

    // Draw label
    Color aTextColor = GetFadedColor( rStyleSettings.GetButtonTextColor(), rStyleSettings.GetDarkShadowColor() );
    maVirDev->SetTextColor( aTextColor );
    // FIXME Expected font size?
    maVirDev->DrawText( aLabelPos, aLabel );

    // Draw arrow
    // FIXME consistence of button colors. https://opengrok.libreoffice.org/xref/core/vcl/source/control/button.cxx#785
    Color aArrowColor = GetFadedColor(COL_BLACK, rStyleSettings.GetShadowColor());
    ImplDrawArrow(*maVirDev, aArrowPos.X(), aArrowPos.Y(), aArrowColor, bArrowToRight);

    // Blit comment control
    rRenderContext.DrawOutDev(aControlRect.TopLeft(), aControlRect.GetSize(), Point(), aControlRect.GetSize(), *maVirDev);
}

// Just accept double-click outside comment control
void SwCommentRuler::Command( const CommandEvent& rCEvt )
{
    Point aMousePos = rCEvt.GetMousePosPixel();
    // Ignore command request if it is inside Comment Control
    if ( !mpViewShell->GetPostItMgr()
          || !mpViewShell->GetPostItMgr()->HasNotes()
          || !GetCommentControlRegion().IsInside( aMousePos ) )
        SvxRuler::Command( rCEvt );
}

void SwCommentRuler::MouseMove(const MouseEvent& rMEvt)
{
    SvxRuler::MouseMove(rMEvt);
    if ( ! mpViewShell->GetPostItMgr() || ! mpViewShell->GetPostItMgr()->HasNotes() )
        return;

    Point aMousePos = rMEvt.GetPosPixel();
    bool  bWasHighlighted = mbIsHighlighted;
    mbIsHighlighted = GetCommentControlRegion().IsInside( aMousePos );
    if ( mbIsHighlighted != bWasHighlighted )
    {
        // Set proper help text
        if ( mbIsHighlighted )
        {
            // Mouse over comment control
            UpdateCommentHelpText();
        }
        else
        {
            // Mouse out of comment control
            // FIXME Should remember previous tooltip text?
            SetQuickHelpText( OUString() );
        }
        // Do start fading
        maFadeTimer.Start();
    }
}

void SwCommentRuler::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point   aMousePos = rMEvt.GetPosPixel();
    if ( !rMEvt.IsLeft() || IsTracking() || !GetCommentControlRegion().IsInside( aMousePos ) )
    {
        SvxRuler::MouseButtonDown(rMEvt);
        return;
    }

    // Toggle notes visibility
    SwView &rView = mpSwWin->GetView();
    SfxRequest aRequest( rView.GetViewFrame(), SID_TOGGLE_NOTES );
    rView.ExecViewOptions( aRequest );

    // It is inside comment control, so update help text
    UpdateCommentHelpText();

    Invalidate();
}

const std::string SwCommentRuler::CreateJsonNotification()
{
    boost::property_tree::ptree jsonNotif;

    jsonNotif.put("margin1", convertTwipToMm100(GetMargin1()));
    jsonNotif.put("margin2", convertTwipToMm100(GetMargin2()));
    jsonNotif.put("leftOffset", convertTwipToMm100(GetNullOffset()));
    jsonNotif.put("pageOffset", convertTwipToMm100(GetPageOffset()));
    jsonNotif.put("pageWidth", convertTwipToMm100(GetPageWidth()));

    RulerUnitData aUnitData = GetCurrentRulerUnit();
    jsonNotif.put("unit", aUnitData.aUnitStr);

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, jsonNotif);
    std::string aPayload = aStream.str();
    return aPayload;
}

void SwCommentRuler::NotifyKit()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    const std::string test = CreateJsonNotification();
    mpViewShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_RULER_UPDATE, test.c_str());
}

void SwCommentRuler::Update()
{
    tools::Rectangle aPreviousControlRect = GetCommentControlRegion();
    SvxRuler::Update();
    if (aPreviousControlRect != GetCommentControlRegion())
        Invalidate();
    NotifyKit();
}

void SwCommentRuler::UpdateCommentHelpText()
{
    const char* pTooltipResId;
    if ( mpViewShell->GetPostItMgr()->ShowNotes() )
        pTooltipResId = STR_HIDE_COMMENTS;
    else
        pTooltipResId = STR_SHOW_COMMENTS;
    SetQuickHelpText(SwResId(pTooltipResId));
}

// TODO Make Ruler return its central rectangle instead of margins.
tools::Rectangle SwCommentRuler::GetCommentControlRegion()
{
    long nLeft = 0;
    SwPostItMgr *pPostItMgr = mpViewShell->GetPostItMgr();

    //rhbz#1006850 When the SwPostItMgr ctor is called from SwView::SwView it
    //triggers an update of the uiview, but the result of the ctor hasn't been
    //set into the mpViewShell yet, so GetPostItMgr is temporarily still NULL
    if (!pPostItMgr)
        return tools::Rectangle();

    unsigned long nSidebarWidth = pPostItMgr->GetSidebarWidth(true);
    //FIXME When the page width is larger then screen, the ruler is misplaced by one pixel
    if (GetTextRTL())
       nLeft = GetPageOffset() - nSidebarWidth + GetBorderOffset();
    else
       nLeft = GetWinOffset() + GetPageOffset() + mpSwWin->LogicToPixel(Size(GetPageWidth(), 0)).Width();
    long nTop    = 0 + 4; // Ruler::ImplDraw uses RULER_OFF (value: 3px) as offset, and Ruler::ImplFormat adds one extra pixel
    // Somehow pPostItMgr->GetSidebarBorderWidth() returns border width already doubled
    long nRight  = nLeft + nSidebarWidth + pPostItMgr->GetSidebarBorderWidth(true);
    long nBottom = nTop + GetRulerVirHeight() - 3;

    tools::Rectangle aRect(nLeft, nTop, nRight, nBottom);
    return aRect;
}

Color SwCommentRuler::GetFadedColor(const Color &rHighColor, const Color &rLowColor)
{
    if (!maFadeTimer.IsActive())
        return mbIsHighlighted ? rHighColor : rLowColor;

    Color aColor = rHighColor;
    aColor.Merge(rLowColor, mnFadeRate * 255 / 100.0f);
    return aColor;
}

IMPL_LINK_NOARG(SwCommentRuler, FadeHandler, Timer *, void)
{
    const int nStep = 25;
    if ( mbIsHighlighted && mnFadeRate < 100 )
        mnFadeRate += nStep;
    else if ( !mbIsHighlighted && mnFadeRate > 0 )
        mnFadeRate -= nStep;
    else
        return;

    Invalidate();

    if ( mnFadeRate != 0 && mnFadeRate != 100)
        maFadeTimer.Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
