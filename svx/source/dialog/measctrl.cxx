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

#include <sfx2/dialoghelper.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdmodel.hxx>
#include <svx/measctrl.hxx>
#include <svx/dlgutil.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <memory>

SvxXMeasurePreview::SvxXMeasurePreview()
    : m_aMapMode(MapUnit::Map100thMM)
{
    // Scale: 1:2
    m_aMapMode.SetScaleX(1.0 / 2);
    m_aMapMode.SetScaleY(1.0 / 2);
}

void SvxXMeasurePreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(getPreviewStripSize(pDrawingArea->get_ref_device()));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    m_pModel.reset(new SdrModel(nullptr, nullptr, true));
    m_pMeasureObj = new SdrMeasureObj(*m_pModel, Point(), Point());

    ResizeImpl(aSize);
    Invalidate();
}

void SvxXMeasurePreview::ResizeImpl(const Size& rSize)
{
    OutputDevice& rRefDevice = GetDrawingArea()->get_ref_device();
    auto popIt = rRefDevice.ScopedPush(vcl::PushFlags::MAPMODE);

    rRefDevice.SetMapMode(m_aMapMode);

    Size aSize = rRefDevice.PixelToLogic(rSize);
    Point aPt1(aSize.Width() / 5, static_cast<tools::Long>(aSize.Height() / 2));
    m_pMeasureObj->SetPoint(aPt1, 0);
    Point aPt2(aSize.Width() * 4 / 5, static_cast<tools::Long>(aSize.Height() / 2));
    m_pMeasureObj->SetPoint(aPt2, 1);
}

void SvxXMeasurePreview::Resize()
{
    CustomWidgetController::Resize();
    ResizeImpl(GetOutputSizePixel());
    Invalidate();
}

SvxXMeasurePreview::~SvxXMeasurePreview() {}

void SvxXMeasurePreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetBackground(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    rRenderContext.Erase();

    auto popIt = rRenderContext.ScopedPush(vcl::PushFlags::MAPMODE);
    rRenderContext.SetMapMode(m_aMapMode);

    bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
    rRenderContext.SetDrawMode(bHighContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);
    m_pMeasureObj->SingleObjectPainter(rRenderContext);
}

void SvxXMeasurePreview::SetAttributes(const SfxItemSet& rInAttrs)
{
    m_pMeasureObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}

bool SvxXMeasurePreview::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bZoomIn = rMEvt.IsLeft() && !rMEvt.IsShift();
    bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    bool bCtrl = rMEvt.IsMod1();

    if (bZoomIn || bZoomOut)
    {
        double fXFrac = m_aMapMode.GetScaleX();
        double fYFrac = m_aMapMode.GetScaleY();
        double fMultFrac;

        if (bZoomIn)
        {
            if (bCtrl)
                fMultFrac = 3.0 / 2;
            else
                fMultFrac = 11.0 / 10;
        }
        else
        {
            if (bCtrl)
                fMultFrac = 2.0 / 3;
            else
                fMultFrac = 10.0 / 11;
        }

        fXFrac *= fMultFrac;
        fYFrac *= fMultFrac;

        if (fXFrac > 0.001 && fXFrac < 1000.0 && fYFrac > 0.001 && fYFrac < 1000.0)
        {
            m_aMapMode.SetScaleX(fXFrac);
            m_aMapMode.SetScaleY(fYFrac);

            OutputDevice& rRefDevice = GetDrawingArea()->get_ref_device();
            rRefDevice.Push(vcl::PushFlags::MAPMODE);
            rRefDevice.SetMapMode(m_aMapMode);
            Size aOutSize(rRefDevice.PixelToLogic(GetOutputSizePixel()));
            rRefDevice.Pop();

            Point aPt(m_aMapMode.GetOrigin());
            tools::Long nX = tools::Long(
                (double(aOutSize.Width()) - (double(aOutSize.Width()) * fMultFrac)) / 2.0 + 0.5);
            tools::Long nY = tools::Long(
                (double(aOutSize.Height()) - (double(aOutSize.Height()) * fMultFrac)) / 2.0 + 0.5);
            aPt.AdjustX(nX);
            aPt.AdjustY(nY);

            m_aMapMode.SetOrigin(aPt);

            Invalidate();
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
