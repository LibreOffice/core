/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Design proposal: https://wiki.documentfoundation.org/Design/Whiteboards/Comments_Ruler_Control

#include <swruler.hxx>

#include <viewsh.hxx>
#include <edtwin.hxx>
#include <PostItMgr.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <sfx2/request.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <tools/json_writer.hxx>
#include <strings.hrc>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <boost/property_tree/json_parser.hpp>

#define CONTROL_BORDER_WIDTH 1

namespace
{
/**
 * Draw a little arrow / triangle with different directions
 *
 * \param nX left coordinate of arrow square
 * \param nY top coordinate of arrow square
 * \param nSize size of the long triangle side / arrow square
 * \param Color arrow color
 * \param bCollapsed if the arrow should display the collapsed state
 */
void ImplDrawArrow(vcl::RenderContext& rRenderContext, long nX, long nY, long nSize,
                   const Color& rColor, bool bCollapsed)
{
    tools::Polygon aTrianglePolygon(4);

    if (bCollapsed)
    {
        if (AllSettings::GetLayoutRTL()) // <
        {
            aTrianglePolygon.SetPoint({ nX + nSize / 2, nY }, 0);
            aTrianglePolygon.SetPoint({ nX + nSize / 2, nY + nSize }, 1);
            aTrianglePolygon.SetPoint({ nX, nY + nSize / 2 }, 2);
            aTrianglePolygon.SetPoint({ nX + nSize / 2, nY }, 3);
        }
        else // >
        {
            aTrianglePolygon.SetPoint({ nX, nY }, 0);
            aTrianglePolygon.SetPoint({ nX + nSize / 2, nY + nSize / 2 }, 1);
            aTrianglePolygon.SetPoint({ nX, nY + nSize }, 2);
            aTrianglePolygon.SetPoint({ nX, nY }, 3);
        }
    }
    else // v
    {
        aTrianglePolygon.SetPoint({ nX, nY + nSize / 2 }, 0);
        aTrianglePolygon.SetPoint({ nX + nSize, nY + nSize / 2 }, 1);
        aTrianglePolygon.SetPoint({ nX + nSize / 2, nY + nSize }, 2);
        aTrianglePolygon.SetPoint({ nX, nY + nSize / 2 }, 3);
    }

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(rColor);
    rRenderContext.DrawPolygon(aTrianglePolygon);
}
}

// Constructor
SwCommentRuler::SwCommentRuler(SwViewShell* pViewSh, vcl::Window* pParent, SwEditWin* pWin,
                               SvxRulerSupportFlags nRulerFlags, SfxBindings& rBindings,
                               WinBits nWinStyle)
    : SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle | WB_HSCROLL)
    , mpViewShell(pViewSh)
    , mpSwWin(pWin)
    , mbIsHighlighted(false)
    , mnFadeRate(0)
    , maVirDev(VclPtr<VirtualDevice>::Create(*this))
{
    // Set fading timeout: 5 x 40ms = 200ms
    maFadeTimer.SetTimeout(40);
    maFadeTimer.SetInvokeHandler(LINK(this, SwCommentRuler, FadeHandler));
    maFadeTimer.SetDebugName("sw::SwCommentRuler maFadeTimer");

    // we have a little bit more space, as we don't draw ruler ticks
    vcl::Font aFont(maVirDev->GetFont());
    aFont.SetFontHeight(aFont.GetFontHeight() + 1);
    maVirDev->SetFont(aFont);
}

SwCommentRuler::~SwCommentRuler() { disposeOnce(); }

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
    const bool bIsCollapsed = !mpViewShell->GetPostItMgr()->ShowNotes();
    const tools::Rectangle aControlRect = GetCommentControlRegion();

    maVirDev->SetOutputSizePixel(aControlRect.GetSize());

    // set colors
    if (!bIsCollapsed)
    {
        if (mbIsHighlighted)
            maVirDev->SetFillColor(
                GetFadedColor(rStyleSettings.GetHighlightColor(), rStyleSettings.GetDialogColor()));
        else
            maVirDev->SetFillColor(rStyleSettings.GetDialogColor());
        maVirDev->SetLineColor(rStyleSettings.GetShadowColor());
    }
    else
    {
        if (mbIsHighlighted)
            maVirDev->SetFillColor(GetFadedColor(rStyleSettings.GetHighlightColor(),
                                                 rStyleSettings.GetWorkspaceColor()));
        else
            maVirDev->SetFillColor(rStyleSettings.GetWorkspaceColor());
        maVirDev->SetLineColor();
    }
    Color aTextColor = GetFadedColor(rStyleSettings.GetHighlightTextColor(),
                                     rStyleSettings.GetButtonTextColor());
    maVirDev->SetTextColor(aTextColor);

    // calculate label and arrow positions
    const OUString aLabel = SwResId(STR_COMMENTS_LABEL);
    const long nTriangleSize = maVirDev->GetTextHeight() / 2 + 1;
    const long nTrianglePad = maVirDev->GetTextHeight() / 4;

    Point aLabelPos(0, (aControlRect.GetHeight() - maVirDev->GetTextHeight()) / 2);
    Point aArrowPos(0, (aControlRect.GetHeight() - nTriangleSize) / 2);

    if (!AllSettings::GetLayoutRTL()) // | > Comments |
    {
        aArrowPos.setX(nTrianglePad);
        aLabelPos.setX(aArrowPos.X() + nTriangleSize + nTrianglePad);
    }
    else // RTL => | Comments < |
    {
        const long nLabelWidth = maVirDev->GetTextWidth(aLabel);
        if (!bIsCollapsed)
        {
            aArrowPos.setX(aControlRect.GetWidth() - 1 - nTrianglePad - CONTROL_BORDER_WIDTH
                           - nTriangleSize);
            aLabelPos.setX(aArrowPos.X() - nTrianglePad - nLabelWidth);
        }
        else
        {
            // if comments are collapsed, left align the text, because otherwise it's very likely to be invisible
            aArrowPos.setX(nLabelWidth + nTrianglePad + nTriangleSize);
            aLabelPos.setX(aArrowPos.X() - nTrianglePad - nLabelWidth);
        }
    }

    // draw control
    maVirDev->DrawRect(tools::Rectangle(Point(), aControlRect.GetSize()));
    maVirDev->DrawText(aLabelPos, aLabel);
    ImplDrawArrow(*maVirDev, aArrowPos.X(), aArrowPos.Y(), nTriangleSize, aTextColor, bIsCollapsed);
    rRenderContext.DrawOutDev(aControlRect.TopLeft(), aControlRect.GetSize(), Point(),
                              aControlRect.GetSize(), *maVirDev);
}

// Just accept double-click outside comment control
void SwCommentRuler::Command(const CommandEvent& rCEvt)
{
    Point aMousePos = rCEvt.GetMousePosPixel();
    // Ignore command request if it is inside Comment Control
    if (!mpViewShell->GetPostItMgr() || !mpViewShell->GetPostItMgr()->HasNotes()
        || !GetCommentControlRegion().IsInside(aMousePos))
        SvxRuler::Command(rCEvt);
}

void SwCommentRuler::MouseMove(const MouseEvent& rMEvt)
{
    SvxRuler::MouseMove(rMEvt);
    if (!mpViewShell->GetPostItMgr() || !mpViewShell->GetPostItMgr()->HasNotes())
        return;

    UpdateCommentHelpText();

    Point aMousePos = rMEvt.GetPosPixel();
    bool bWasHighlighted = mbIsHighlighted;
    mbIsHighlighted = GetCommentControlRegion().IsInside(aMousePos);
    if (mbIsHighlighted != bWasHighlighted)
        // Do start fading
        maFadeTimer.Start();
}

void SwCommentRuler::MouseButtonDown(const MouseEvent& rMEvt)
{
    Point aMousePos = rMEvt.GetPosPixel();
    if (!rMEvt.IsLeft() || IsTracking() || !GetCommentControlRegion().IsInside(aMousePos))
    {
        SvxRuler::MouseButtonDown(rMEvt);
        return;
    }

    // Toggle notes visibility
    SwView& rView = mpSwWin->GetView();
    SfxRequest aRequest(rView.GetViewFrame(), SID_TOGGLE_NOTES);
    rView.ExecViewOptions(aRequest);

    // It is inside comment control, so update help text
    UpdateCommentHelpText();

    Invalidate();
}

void SwCommentRuler::CreateJsonNotification(tools::JsonWriter& rJsonWriter)
{
    // Note that GetMargin1(), GetMargin2(), GetNullOffset(), and GetPageOffset() return values in
    // pixels. Not twips. So "converting" the returned values with convertTwipToMm100() is quite
    // wrong. (Also, even if the return values actually were in twips, it is questionable why we
    // would want to pass them in mm100, as all other length values in the LOKit protocol apparently
    // are in twips.)

    // Anyway, as the consuming code in Online mostly seems to work anyway, it is likely that it
    // would work as well even if the values in pixels were passed without a bogus "conversion" to
    // mm100. But let's keep this as is for now.

    // Also note that in desktop LibreOffice, these pixel values for the ruler of course change as
    // one changes the zoom level. (Can be seen if one temporarily modifies the NotifyKit() function
    // below to call this CreateJsonNotification() function and print its result in all cases even
    // without LibreOfficeKit::isActive().) But in both web-based Online and in the iOS app, the
    // zoom level from the point of view of this code here apparently does not change even if one
    // zooms from the Online code's point of view.
    rJsonWriter.put("margin1", convertTwipToMm100(GetMargin1()));
    rJsonWriter.put("margin2", convertTwipToMm100(GetMargin2()));
    rJsonWriter.put("leftOffset", convertTwipToMm100(GetNullOffset()));
    rJsonWriter.put("pageOffset", convertTwipToMm100(GetPageOffset()));

    // GetPageWidth() on the other hand does return a value in twips.
    // So here convertTwipToMm100() really does produce actual mm100. Fun.
    rJsonWriter.put("pageWidth", convertTwipToMm100(GetPageWidth()));

    {
        auto tabsNode = rJsonWriter.startNode("tabs");

        // The RulerTab array elements that GetTabs() returns have their nPos field in twips. So these
        // too are actual mm100.
        for (auto const& tab : GetTabs())
        {
            auto tabNode = rJsonWriter.startNode("");
            rJsonWriter.put("position", convertTwipToMm100(tab.nPos));
            rJsonWriter.put("style", tab.nStyle);
        }
    }

    RulerUnitData aUnitData = GetCurrentRulerUnit();
    rJsonWriter.put("unit", aUnitData.aUnitStr);
}

void SwCommentRuler::NotifyKit()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    tools::JsonWriter aJsonWriter;
    CreateJsonNotification(aJsonWriter);
    mpViewShell->GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_RULER_UPDATE,
                                                               aJsonWriter.extractData());
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
    if (mpViewShell->GetPostItMgr()->ShowNotes())
        pTooltipResId = STR_HIDE_COMMENTS;
    else
        pTooltipResId = STR_SHOW_COMMENTS;
    SetQuickHelpText(SwResId(pTooltipResId));
}

// TODO Make Ruler return its central rectangle instead of margins.
tools::Rectangle SwCommentRuler::GetCommentControlRegion()
{
    SwPostItMgr* pPostItMgr = mpViewShell->GetPostItMgr();

    //rhbz#1006850 When the SwPostItMgr ctor is called from SwView::SwView it
    //triggers an update of the uiview, but the result of the ctor hasn't been
    //set into the mpViewShell yet, so GetPostItMgr is temporarily still NULL
    if (!pPostItMgr)
        return tools::Rectangle();

    const unsigned long nSidebarWidth = pPostItMgr->GetSidebarWidth(true);

    //FIXME When the page width is larger then screen, the ruler is misplaced by one pixel
    long nLeft = GetPageOffset();
    if (GetTextRTL())
        nLeft += GetBorderOffset() - nSidebarWidth;
    else
        nLeft += GetWinOffset() + mpSwWin->LogicToPixel(Size(GetPageWidth(), 0)).Width();

    // Ruler::ImplDraw uses RULER_OFF (value: 3px) as offset, and Ruler::ImplFormat adds one extra pixel
    long nTop = 4;
    // Somehow pPostItMgr->GetSidebarBorderWidth() returns border width already doubled
    long nRight = nLeft + nSidebarWidth + pPostItMgr->GetSidebarBorderWidth(true);
    long nBottom = nTop + GetRulerVirHeight() - 3;

    tools::Rectangle aRect(nLeft, nTop, nRight, nBottom);
    return aRect;
}

Color SwCommentRuler::GetFadedColor(const Color& rHighColor, const Color& rLowColor)
{
    if (!maFadeTimer.IsActive())
        return mbIsHighlighted ? rHighColor : rLowColor;

    Color aColor = rHighColor;
    aColor.Merge(rLowColor, mnFadeRate * 255 / 100.0f);
    return aColor;
}

IMPL_LINK_NOARG(SwCommentRuler, FadeHandler, Timer*, void)
{
    const int nStep = 25;
    if (mbIsHighlighted && mnFadeRate < 100)
        mnFadeRate += nStep;
    else if (!mbIsHighlighted && mnFadeRate > 0)
        mnFadeRate -= nStep;
    else
        return;

    Invalidate();

    if (mnFadeRate != 0 && mnFadeRate != 100)
        maFadeTimer.Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
