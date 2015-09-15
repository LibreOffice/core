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
#include "svx/measctrl.hxx"
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>
#include <memory>

SvxXMeasurePreview::SvxXMeasurePreview(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle)
{
    SetMapMode(MAP_100TH_MM);

    // Scale: 1:2
    MapMode aMapMode = GetMapMode();
    aMapMode.SetScaleX(Fraction(1, 2));
    aMapMode.SetScaleY(Fraction(1, 2));
    SetMapMode(aMapMode);

    Size aSize = GetOutputSize();
    Point aPt1 = Point(aSize.Width() / 5, (long) (aSize.Height() / 2));
    Point aPt2 = Point(aSize.Width() * 4 / 5, (long) (aSize.Height() / 2));

    pMeasureObj = new SdrMeasureObj(aPt1, aPt2);
    pModel = new SdrModel();
    pMeasureObj->SetModel(pModel);

    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SetDrawMode(bHighContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);

    Invalidate();
}

void SvxXMeasurePreview::Resize()
{
    Control::Resize();

    Size aSize = GetOutputSize();
    Point aPt1 = Point(aSize.Width() / 5, (long) (aSize.Height() / 2));
    pMeasureObj->SetPoint(aPt1, 0);
    Point aPt2 = Point(aSize.Width() * 4 / 5, (long) (aSize.Height() / 2));
    pMeasureObj->SetPoint(aPt2, 1);
}

VCL_BUILDER_DECL_FACTORY(SvxXMeasurePreview)
{
    WinBits nWinStyle = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SvxXMeasurePreview>::Create(pParent, nWinStyle);
}

Size SvxXMeasurePreview::GetOptimalSize() const
{
    return getPreviewStripSize(this);
}

SvxXMeasurePreview::~SvxXMeasurePreview()
{
    disposeOnce();
}

void SvxXMeasurePreview::dispose()
{
    // No one is deleting the MeasureObj? This is not only an error but also
    // a memory leak (!). Main problem is that this object is still listening to
    // a StyleSheet of the model which was set. Thus, if You want to keep the obnject,
    // set the model to 0L, if object is not needed (seems to be the case here),
    // delete it.
    delete pMeasureObj;

    delete pModel;
    Control::dispose();
}

void SvxXMeasurePreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    pMeasureObj->SingleObjectPainter(rRenderContext);
}

void SvxXMeasurePreview::SetAttributes(const SfxItemSet& rInAttrs)
{
    pMeasureObj->SetMergedItemSetAndBroadcast(rInAttrs);

    Invalidate();
}

void SvxXMeasurePreview::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bZoomIn  = rMEvt.IsLeft() && !rMEvt.IsShift();
    bool bZoomOut = rMEvt.IsRight() || rMEvt.IsShift();
    bool bCtrl    = rMEvt.IsMod1();

    if (bZoomIn || bZoomOut)
    {
        MapMode aMapMode = GetMapMode();
        Fraction aXFrac = aMapMode.GetScaleX();
        Fraction aYFrac = aMapMode.GetScaleY();
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
            aMapMode.SetScaleX(aXFrac);
            aMapMode.SetScaleY(aYFrac);
            SetMapMode(aMapMode);

            Size aOutSize(GetOutputSize());

            Point aPt(aMapMode.GetOrigin());
            long nX = long((double(aOutSize.Width()) - (double(aOutSize.Width()) * double(*pMultFrac))) / 2.0 + 0.5);
            long nY = long((double(aOutSize.Height()) - (double(aOutSize.Height()) * double(*pMultFrac))) / 2.0 + 0.5);
            aPt.X() += nX;
            aPt.Y() += nY;

            aMapMode.SetOrigin(aPt);
            SetMapMode(aMapMode);

            Invalidate();
        }
    }
}

void SvxXMeasurePreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
        SetDrawMode(bHighContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
