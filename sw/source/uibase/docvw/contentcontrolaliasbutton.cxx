/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <contentcontrolaliasbutton.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <vcl/metric.hxx>

#include <HeaderFooterWin.hxx>
#include <edtwin.hxx>
#include <formatcontentcontrol.hxx>
#include <swabstdlg.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>

#define TEXT_PADDING 3
#define BOX_DISTANCE 3
#define BUTTON_WIDTH 12

SwContentControlAliasButton::SwContentControlAliasButton(SwEditWin* pEditWin,
                                                         SwContentControl* pContentControl)
    : SwFrameMenuButtonBase(pEditWin, nullptr,
                            u"modules/swriter/ui/contentcontrolaliasbutton.ui"_ustr,
                            u"ContentControlAliasButton"_ustr)
    , m_xPushButton(m_xBuilder->weld_button(u"button"_ustr))
    , m_sLabel(pContentControl->GetAlias())
{
    m_xPushButton->set_accessible_name(m_sLabel);
    m_xPushButton->connect_clicked(LINK(this, SwContentControlAliasButton, ClickHdl));
    m_xVirDev = m_xPushButton->create_virtual_device();
    SetVirDevFont();
}

SwContentControlAliasButton::~SwContentControlAliasButton() { disposeOnce(); }

void SwContentControlAliasButton::dispose()
{
    m_xPushButton.reset();
    m_xVirDev.disposeAndClear();
    SwFrameMenuButtonBase::dispose();
}

void SwContentControlAliasButton::SetOffset(Point aTopLeftPixel)
{
    // Compute the text size and get the box position & size from it.
    tools::Rectangle aTextRect;
    m_xVirDev->GetTextBoundRect(aTextRect, m_sLabel);
    tools::Rectangle aTextPxRect = m_xVirDev->LogicToPixel(aTextRect);
    FontMetric aFontMetric = m_xVirDev->GetFontMetric(m_xVirDev->GetFont());
    Size aBoxSize(aTextPxRect.GetWidth() + BUTTON_WIDTH + TEXT_PADDING * 2,
                  aFontMetric.GetLineHeight() + TEXT_PADDING * 2);
    Point aBoxPos(aTopLeftPixel.X() + BOX_DISTANCE, aTopLeftPixel.Y() - aBoxSize.Height());

    // Set the position & size of the window.
    SetPosSizePixel(aBoxPos, aBoxSize);
    m_xVirDev->SetOutputSizePixel(aBoxSize);

    PaintButton();
}

IMPL_LINK_NOARG(SwContentControlAliasButton, ClickHdl, weld::Button&, void)
{
    if (m_bReadOnly)
    {
        return;
    }

    SwView& rView = GetEditWin()->GetView();
    SwWrtShell& rWrtSh = rView.GetWrtShell();
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<VclAbstractDialog> pDlg(
        pFact->CreateSwContentControlDlg(GetEditWin()->GetFrameWeld(), rWrtSh));
    VclAbstractDialog::AsyncContext aContext;
    aContext.maEndDialogFn = [](sal_Int32) {};
    pDlg->StartExecuteAsync(aContext);
}

void SwContentControlAliasButton::PaintButton()
{
    if (!m_xVirDev)
    {
        return;
    }

    m_xVirDev->SetMapMode(MapMode(MapUnit::MapPixel));
    drawinglayer::primitive2d::Primitive2DContainer aSeq;
    tools::Rectangle aRect(Point(0, 0), m_xVirDev->PixelToLogic(GetSizePixel()));

    // Create button
    SwFrameButtonPainter::PaintButton(aSeq, aRect, /*bOnTop=*/false);

    // Create the text primitive
    const SwViewOption* pVOpt = GetEditWin()->GetView().GetWrtShell().GetViewOptions();
    basegfx::BColor aLineColor = pVOpt->GetHeaderFooterMarkColor().getBColor();
    basegfx::B2DVector aFontSize;
    drawinglayer::attribute::FontAttribute aFontAttr
        = drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, m_xVirDev->GetFont(),
                                                                 false, false);

    FontMetric aFontMetric = m_xVirDev->GetFontMetric(m_xVirDev->GetFont());
    double nTextOffsetY = aFontMetric.GetAscent() + TEXT_PADDING;
    double nTextOffsetX = std::abs(aRect.GetWidth() - m_xVirDev->GetTextWidth(m_sLabel)) / 2.0;
    Point aTextPos(nTextOffsetX, nTextOffsetY);

    basegfx::B2DHomMatrix aTextMatrix = basegfx::utils::createScaleTranslateB2DHomMatrix(
        aFontSize.getX(), aFontSize.getY(), static_cast<double>(aTextPos.X()),
        static_cast<double>(aTextPos.Y()));

    aSeq.push_back(new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
        aTextMatrix, m_sLabel, 0, m_sLabel.getLength(), std::vector<double>(), {},
        std::move(aFontAttr), css::lang::Locale(), aLineColor));

    // Create the processor and process the primitives
    drawinglayer::geometry::ViewInformation2D aViewInfo;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor
        = drawinglayer::processor2d::createProcessor2DFromOutputDevice(*m_xVirDev, aViewInfo);

    pProcessor->process(aSeq);

    m_xPushButton->set_custom_button(m_xVirDev.get());
}

void SwContentControlAliasButton::ShowAll(bool bShow) { Show(bShow); }

bool SwContentControlAliasButton::Contains(const Point& rDocPt) const
{
    tools::Rectangle aRect(GetPosPixel(), GetSizePixel());
    return aRect.Contains(rDocPt);
}

void SwContentControlAliasButton::SetReadonly(bool bReadonly) { m_bReadOnly = bReadonly; }

void SwContentControlAliasButton::SetContentControl(SwContentControl* pContentControl)
{
    m_sLabel = pContentControl->GetAlias();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
