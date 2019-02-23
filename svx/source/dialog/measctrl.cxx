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
    m_aMapMode.SetScaleX(Fraction(1, 2));
    m_aMapMode.SetScaleY(Fraction(1, 2));
}

void SvxXMeasurePreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(getPreviewStripSize(pDrawingArea->get_ref_device()));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    pModel.reset(new SdrModel(nullptr, nullptr, true));
    pMeasureObj.reset(new SdrMeasureObj(*pModel, Point(), Point()));

    ResizeImpl(aSize);
    Invalidate();
}

void SvxXMeasurePreview::ResizeImpl(const Size& rSize)
{
    OutputDevice& rRefDevice = GetDrawingArea()->get_ref_device();
    rRefDevice.Push(PushFlags::MAPMODE);

    rRefDevice.SetMapMode(m_aMapMode);

    Size aSize = rRefDevice.PixelToLogic(rSize);
    Point aPt1 = Point(aSize.Width() / 5, static_cast<long>(aSize.Height() / 2));
    pMeasureObj->SetPoint(aPt1, 0);
    Point aPt2 = Point(aSize.Width() * 4 / 5, static_cast<long>(aSize.Height() / 2));
    pMeasureObj->SetPoint(aPt2, 1);

    rRefDevice.Pop();
}

void SvxXMeasurePreview::Resize()
{
    CustomWidgetController::Resize();
    ResizeImpl(GetOutputSizePixel());
    Invalidate();
}

SvxXMeasurePreview::~SvxXMeasurePreview()
{
}

void SvxXMeasurePreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.Push(PushFlags::MAPMODE);
    rRenderContext.SetMapMode(m_aMapMode);

    bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();
    rRenderContext.SetDrawMode(bHighContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);
    pMeasureObj->SingleObjectPainter(rRenderContext);

    rRenderContext.Pop();
}

void SvxXMeasurePreview::SetAttributes(const SfxItemSet& rInAttrs)
{
    pMeasureObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}

bool SvxXMeasurePreview::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    bool bCtrl    = rMEvt.IsMod1();

    if (bZoomIn || bZoomOut)
    {
        Fraction aXFrac = m_aMapMode.GetScaleX();
        Fraction aYFrac = m_aMapMode.GetScaleY();
        std::unique_ptr<Fraction> pMultFrac;

        if (bZoomIn)
        {
            if (bCtrl)
                pMultFrac.reset(new Fraction(3, 2));
            else
                pMultFrac.reset(new Fraction(11, 10));
        }
        else
        {
            if (bCtrl)
                pMultFrac.reset(new Fraction(2, 3));
            else
                pMultFrac.reset(new Fraction(10, 11));
        }

        aXFrac *= *pMultFrac;
        aYFrac *= *pMultFrac;

        if (double(aXFrac) > 0.001 && double(aXFrac) < 1000.0 &&
            double(aYFrac) > 0.001 && double(aYFrac) < 1000.0)
        {
            m_aMapMode.SetScaleX(aXFrac);
            m_aMapMode.SetScaleY(aYFrac);

            OutputDevice& rRefDevice = GetDrawingArea()->get_ref_device();
            rRefDevice.Push(PushFlags::MAPMODE);
            rRefDevice.SetMapMode(m_aMapMode);
            Size aOutSize(rRefDevice.PixelToLogic(GetOutputSizePixel()));
            rRefDevice.Pop();

            Point aPt(m_aMapMode.GetOrigin());
            long nX = long((double(aOutSize.Width()) - (double(aOutSize.Width()) * double(*pMultFrac))) / 2.0 + 0.5);
            long nY = long((double(aOutSize.Height()) - (double(aOutSize.Height()) * double(*pMultFrac))) / 2.0 + 0.5);
            aPt.AdjustX(nX );
            aPt.AdjustY(nY );

            m_aMapMode.SetOrigin(aPt);

            Invalidate();
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
