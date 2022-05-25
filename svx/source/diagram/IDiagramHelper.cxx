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

#include <svx/diagram/IDiagramHelper.hxx>
#include <svx/svdogrp.hxx>

#include <svx/svdhdl.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

namespace {

class DiagramFrameHdl final : public SdrHdl
{
    basegfx::B2DHomMatrix maTransformation;

    // create marker for this kind
    virtual void CreateB2dIAObject() override;

public:
    DiagramFrameHdl(const basegfx::B2DHomMatrix& rTransformation);
    // virtual ~DiagramFrameHdl() override;
};

void DiagramFrameHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    SdrMarkView* pView = pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                OutputDevice& rOutDev(rPageWindow.GetPaintWindow().GetOutputDevice());
                const basegfx::B2DVector aDiscreteInLogic(rOutDev.GetInverseViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                const double fDiscretePixelSize(aDiscreteInLogic.getLength());
                const double fOuterDistance(7.0);
                const double fInnerDistance(5.0);

                basegfx::B2DRange aRange(0.0, 0.0, 1.0, 1.0);
                aRange.transform(maTransformation);

                basegfx::B2DRange aOuterRange(aRange);
                aOuterRange.grow(fDiscretePixelSize * fOuterDistance);

                basegfx::B2DRange aInnerRange(aRange);
                aInnerRange.grow(fDiscretePixelSize * fInnerDistance);

                basegfx::B2DPolyPolygon aPolyPolygon;
                aPolyPolygon.append(basegfx::utils::createPolygonFromRect(aOuterRange));
                aPolyPolygon.append(basegfx::utils::createPolygonFromRect(aInnerRange));

                const StyleSettings& rStyles(rOutDev.GetSettings().GetStyleSettings());
                Color aFillColor(rStyles.GetHighlightColor());
                Color aLineColor(aFillColor);
                aFillColor.IncreaseLuminance(10);
                aLineColor.DecreaseLuminance(30);

                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(
                    new sdr::overlay::OverlayPolyPolygon(
                            aPolyPolygon,
                            aLineColor,  // Line
                            fDiscretePixelSize * 2.0,
                            aFillColor)); // Fill

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

DiagramFrameHdl::DiagramFrameHdl(const basegfx::B2DHomMatrix& rTransformation)
: SdrHdl(Point(), SdrHdlKind::Move)
, maTransformation(rTransformation)
{
}

// DiagramFrameHdl::~DiagramFrameHdl()
// {
// }

} // end of anonymous namespace

namespace svx { namespace diagram {

IDiagramHelper::IDiagramHelper()
: mbUseDiagramThemeData(false)
, mbUseDiagramModelData(true)
, mbForceThemePtrRecreation(false)
{
}

IDiagramHelper::~IDiagramHelper() {}

void IDiagramHelper::anchorToSdrObjGroup(SdrObjGroup& rTarget)
{
    rTarget.mp_DiagramHelper.reset(this);
}

void IDiagramHelper::AddAdditionalVisualization(const SdrObjGroup& rTarget, SdrHdlList& rHdlList)
{
    // create an extra frame visualization here
    basegfx::B2DHomMatrix aTransformation;
    basegfx::B2DPolyPolygon aPolyPolygon;
    rTarget.TRGetBaseGeometry(aTransformation, aPolyPolygon);

    std::unique_ptr<SdrHdl> pHdl(new DiagramFrameHdl(aTransformation));
    rHdlList.AddHdl(std::move(pHdl));
}

}} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
