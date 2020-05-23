/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <edtwin.hxx>
#include <OutlineContentVisibilityWin.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <vcl/canvastools.hxx>
#include <memory>

#include <IDocumentOutlineNodes.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <node.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <strings.hrc>
#include <svx/svdview.hxx>

#define BUTTON_WIDTH 18
#define BUTTON_HEIGHT 20

using namespace basegfx;
using namespace basegfx::utils;
using namespace drawinglayer::attribute;

namespace
{
basegfx::BColor lcl_GetFillColor(const basegfx::BColor& rLineColor)
{
    basegfx::BColor aHslLine = basegfx::utils::rgb2hsl(rLineColor);
    double nLuminance = aHslLine.getZ() * 2.5;
    if (nLuminance == 0)
        nLuminance = 0.5;
    else if (nLuminance >= 1.0)
        nLuminance = aHslLine.getZ() * 0.4;
    aHslLine.setZ(nLuminance);
    return basegfx::utils::hsl2rgb(aHslLine);
}

basegfx::BColor lcl_GetLighterGradientColor(const basegfx::BColor& rDarkColor)
{
    basegfx::BColor aHslDark = basegfx::utils::rgb2hsl(rDarkColor);
    double nLuminance = aHslDark.getZ() * 255 + 20;
    aHslDark.setZ(nLuminance / 255.0);
    return basegfx::utils::hsl2rgb(aHslDark);
}
}

SwOutlineContentVisibilityWin::SwOutlineContentVisibilityWin(SwEditWin* pEditWin,
                                                             const SwFrame* pFrame)
    : PushButton(pEditWin, 0)
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
    , m_bIsAppearing(false)
    , m_nDelayAppearing(0)
    , m_bDestroyed(false)
{
    SetSizePixel(Size(BUTTON_WIDTH, BUTTON_HEIGHT));

    m_aDelayTimer.SetTimeout(50);
    m_aDelayTimer.SetInvokeHandler(LINK(this, SwOutlineContentVisibilityWin, DelayHandler));
}

void SwOutlineContentVisibilityWin::dispose()
{
    m_bDestroyed = true;
    m_aDelayTimer.Stop();

    m_pEditWin.clear();
    m_pFrame = nullptr;

    PushButton::dispose();
}

void SwOutlineContentVisibilityWin::Paint(vcl::RenderContext& rRenderContext,
                                          const ::tools::Rectangle&)
{
    SetMapMode(MapMode(MapUnit::MapPixel));
    drawinglayer::primitive2d::Primitive2DContainer aSeq;
    const ::tools::Rectangle aRect(
        ::tools::Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));

    aSeq.clear();

    const double nRadius = 3;
    const double nKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);

    B2DPolygon aPolygon;

    {
        B2DPoint aCorner(aRect.Left(), aRect.Top());
        B2DPoint aStart(aRect.Left() + nRadius, aRect.Top());
        B2DPoint aEnd(aRect.Left(), aRect.Top() + nRadius);
        aPolygon.append(aStart);
        aPolygon.appendBezierSegment(interpolate(aStart, aCorner, nKappa),
                                     interpolate(aEnd, aCorner, nKappa), aEnd);
    }

    {
        B2DPoint aCorner(aRect.Left(), aRect.Bottom());
        B2DPoint aStart(aRect.Left(), aRect.Bottom() - nRadius);
        B2DPoint aEnd(aRect.Left() + nRadius, aRect.Bottom());
        aPolygon.append(aStart);
        aPolygon.appendBezierSegment(interpolate(aStart, aCorner, nKappa),
                                     interpolate(aEnd, aCorner, nKappa), aEnd);
    }

    {
        B2DPoint aCorner(aRect.Right(), aRect.Bottom());
        B2DPoint aStart(aRect.Right() - nRadius, aRect.Bottom());
        B2DPoint aEnd(aRect.Right(), aRect.Bottom() - nRadius);
        aPolygon.append(aStart);
        aPolygon.appendBezierSegment(interpolate(aStart, aCorner, nKappa),
                                     interpolate(aEnd, aCorner, nKappa), aEnd);
    }

    {
        B2DPoint aCorner(aRect.Right(), aRect.Top());
        B2DPoint aStart(aRect.Right(), aRect.Top() + nRadius);
        B2DPoint aEnd(aRect.Right() - nRadius, aRect.Top());
        aPolygon.append(aStart);
        aPolygon.appendBezierSegment(interpolate(aStart, aCorner, nKappa),
                                     interpolate(aEnd, aCorner, nKappa), aEnd);
    }

    aPolygon.setClosed(true);

    // Colors
    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor()
                                     .getBColor(); // todo: SwViewOption::GetOutlineButtonColor()
    basegfx::BColor aFillColor = lcl_GetFillColor(aLineColor);
    basegfx::BColor aLighterColor = lcl_GetLighterGradientColor(aFillColor);

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if (rSettings.GetHighContrastMode())
    {
        aFillColor = rSettings.GetDialogColor().getBColor();
        aLineColor = rSettings.GetDialogTextColor().getBColor();
        aSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
            new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon),
                                                                       aFillColor)));
    }
    else
    {
        B2DRectangle aGradientRect = vcl::unotools::b2DRectangleFromRectangle(aRect);
        FillGradientAttribute aFillAttrs(drawinglayer::attribute::GradientStyle::Linear, 0.0, 0.0,
                                         0.0, 0.0, aLighterColor, aFillColor, 10);
        aSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
            new drawinglayer::primitive2d::FillGradientPrimitive2D(aGradientRect, aFillAttrs)));
    }

    // Create the border lines primitive
    aSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aPolygon, aLineColor)));

    // Create the '+' or '-' sign primitive
    B2DRectangle aSignArea(
        B2DPoint(aRect.Left() + (aRect.Right() - BUTTON_WIDTH) / 2.0, 1.0),
        B2DSize(aRect.Right() + 1.0 - (aRect.Right() - BUTTON_WIDTH) / 2.0, aRect.getHeight()));

    B2DPolygon aSign;

    if (GetEditWin()->GetView().GetWrtShell().IsOutlineContentFolded(m_nOutlinePos))
    {
        // Create the + polygon
        double nLeft = aSignArea.getMinX() + 4.0;
        double nRight = aSignArea.getMaxX() - 4.0;
        double nHalfW = (nRight - nLeft) / 2.0;

        double nTop = aSignArea.getCenterY() - nHalfW;
        double nBottom = aSignArea.getCenterY() + nHalfW;

        aSign.append(B2DPoint(nLeft, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, nTop));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, nTop));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(nRight, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(nRight, aSignArea.getCenterY() + 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, aSignArea.getCenterY() + 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, nBottom));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, nBottom));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, aSignArea.getCenterY() + 1.0));
        aSign.append(B2DPoint(nLeft, aSignArea.getCenterY() + 1.0));
    }
    else
    {
        // Create the - polygon
        double nLeft = aSignArea.getMinX() + 4.0;
        double nRight = aSignArea.getMaxX() - 4.0;

        aSign.append(B2DPoint(nLeft, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(nRight, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(nRight, aSignArea.getCenterY() + 1.0));
        aSign.append(B2DPoint(nLeft, aSignArea.getCenterY() + 1.0));
    }
    aSign.setClosed(true);

    BColor aSignColor = COL_BLACK.getBColor();
    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        aSignColor = COL_WHITE.getBColor();

    aSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(B2DPolyPolygon(aSign),
                                                                   aSignColor)));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext,
                                                                         aNewViewInfos));

    pProcessor->process(aSeq);
}

void SwOutlineContentVisibilityWin::Set()
{
    const SwTextFrame* pTextFrame = static_cast<const SwTextFrame*>(GetFrame());
    const SwTextNode* pTextNode = pTextFrame->GetTextNodeFirst();
    SwWrtShell& rSh = GetEditWin()->GetView().GetWrtShell();
    rSh.GetNodes().GetOutLineNds().Seek_Entry(
        static_cast<SwNode*>(const_cast<SwTextNode*>(pTextNode)), &m_nOutlinePos);
    assert(m_nOutlinePos != SwOutlineNodes::npos);

    // check for subs
    SwOutlineNodes::size_type nOutlineNodesCount
        = rSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
    int nLevel = rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(m_nOutlinePos);
    OUString sQuickHelp(SwResId(STR_OUTLINE_CONTENT_TOGGLE_VISIBILITY));
    if (m_nOutlinePos + 1 < nOutlineNodesCount
        && rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(m_nOutlinePos + 1) > nLevel)
        sQuickHelp += " (" + SwResId(STR_OUTLINE_CONTENT_TOGGLE_VISIBILITY_EXT) + ")";
    SetQuickHelpText(sQuickHelp);

    // Set the position of the window
    SwRect aSwRect = GetFrame()->getFrameArea(); // not far in margin
    //SwRect aSwRect = GetFrame()->GetPaintArea(); // far in margin
    Point aPxPt(GetEditWin()->GetOutDev()->LogicToPixel(aSwRect.BottomLeft()));
    aPxPt.AdjustX(-GetSizePixel().getWidth() - 2);
    aPxPt.AdjustY(-GetSizePixel().getHeight());
    SetPosPixel(aPxPt);
}

void SwOutlineContentVisibilityWin::ShowAll(bool bShow)
{
    m_bIsAppearing = bShow;
    if (bShow)
        m_nDelayAppearing = 0;

    if (!m_bDestroyed && m_aDelayTimer.IsActive())
        m_aDelayTimer.Stop();
    if (!m_bDestroyed)
        m_aDelayTimer.Start();

    if (!bShow)
        GrabFocusToDocument();
}

bool SwOutlineContentVisibilityWin::Contains(const Point& rDocPt) const
{
    ::tools::Rectangle aRect(GetPosPixel(), GetSizePixel());
    if (aRect.IsInside(rDocPt))
        return true;
    return false;
}

void SwOutlineContentVisibilityWin::ToggleOutlineContentVisibility(const bool bSubs)
{
    SwWrtShell& rSh = GetEditWin()->GetView().GetWrtShell();

    // get outline frame to check this is still next to it after toggle
    SwContentAtPos aSwContentAtPos(IsAttrAtPos::Outline);
    assert(GetEditWin()->GetView().GetWrtShell().GetContentAtPos(
        GetEditWin()->PixelToLogic(GetEditWin()->GetPointerPosPixel()), aSwContentAtPos));
    assert(aSwContentAtPos.aFnd.pNode && aSwContentAtPos.aFnd.pNode->IsTextNode());
    SwContentFrame* pContentFrame
        = aSwContentAtPos.aFnd.pNode->GetTextNode()->getLayoutFrame(nullptr);

    rSh.LockView(true);
    if (GetEditWin()->GetView().GetDrawView()->IsTextEdit())
        rSh.EndTextEdit();
    if (GetEditWin()->GetView().IsDrawMode())
        GetEditWin()->GetView().LeaveDrawCreate();
    rSh.EnterStdMode();
    if (bSubs)
    {
        // toggle including sub levels
        SwOutlineNodes::size_type nPos = m_nOutlinePos;
        SwOutlineNodes::size_type nOutlineNodesCount
            = rSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
        int nLevel = rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos);
        bool bFold = rSh.IsOutlineContentFolded(nPos);
        do
        {
            if (rSh.IsOutlineContentFolded(nPos) == bFold)
                rSh.ToggleOutlineContentVisibility(nPos);
        } while (++nPos < nOutlineNodesCount
                 && rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos) > nLevel);
    }
    else
        rSh.ToggleOutlineContentVisibility(m_nOutlinePos);
    rSh.GotoOutline(m_nOutlinePos);
    rSh.LockView(false);

    // hide if no longer next to its outline frame
    if (!(rSh.GetContentAtPos(GetEditWin()->PixelToLogic(GetEditWin()->GetPointerPosPixel()),
                              aSwContentAtPos)
          && aSwContentAtPos.aFnd.pNode && aSwContentAtPos.aFnd.pNode->IsTextNode()
          && pContentFrame == aSwContentAtPos.aFnd.pNode->GetTextNode()->getLayoutFrame(nullptr)))
    {
        Hide();
        GrabFocusToDocument();
    }
}

void SwOutlineContentVisibilityWin::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    if (!aKeyCode.GetModifier()
        && (aKeyCode.GetCode() == KEY_RETURN || aKeyCode.GetCode() == KEY_SPACE))
    {
        ToggleOutlineContentVisibility(aKeyCode.GetCode() == KEY_RETURN);
    }
    else if (aKeyCode.GetCode() == KEY_ESCAPE)
    {
        Hide();
        GrabFocusToDocument();
    }
}

void SwOutlineContentVisibilityWin::MouseButtonDown(const MouseEvent& rMEvt)
{
    ToggleOutlineContentVisibility(rMEvt.IsRight() || rMEvt.IsMod1());
}

IMPL_LINK_NOARG(SwOutlineContentVisibilityWin, DelayHandler, Timer*, void)
{
    const int TICKS_BEFORE_WE_APPEAR = 10;
    if (m_bIsAppearing && m_nDelayAppearing < TICKS_BEFORE_WE_APPEAR)
    {
        ++m_nDelayAppearing;
        m_aDelayTimer.Start();
        return;
    }
    if (m_bIsAppearing)
    {
        Show();
        GrabFocus();
    }
    else
        Hide();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
