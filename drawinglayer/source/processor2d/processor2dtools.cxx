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
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/sysdata.hxx>
#include "vclpixelprocessor2d.hxx"
#include "vclmetafileprocessor2d.hxx"
#include <config_vclplug.h>

#if defined(_WIN32)
#include <drawinglayer/processor2d/d2dpixelprocessor2d.hxx>
#elif USE_HEADLESS_CODE
#include <drawinglayer/processor2d/cairopixelprocessor2d.hxx>
#endif

namespace drawinglayer::processor2d
{
std::unique_ptr<BaseProcessor2D> createPixelProcessor2DFromOutputDevice(
    OutputDevice& rTargetOutDev,
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D)
{
    static const bool bTestSystemPrimitiveRenderer(nullptr != std::getenv("TEST_SYSTEM_PRIMITIVE_RENDERER"));
    if(bTestSystemPrimitiveRenderer)
    {
        drawinglayer::geometry::ViewInformation2D aViewInformation2D(rViewInformation2D);
        // if mnOutOffX/mnOutOffY is set (a 'hack' to get a cheap additional offset), apply it additionally
        if(0 != rTargetOutDev.GetOutOffXPixel() || 0 != rTargetOutDev.GetOutOffYPixel())
        {
            basegfx::B2DHomMatrix aTransform(aViewInformation2D.getViewTransformation());
            aTransform.translate(rTargetOutDev.GetOutOffXPixel(), rTargetOutDev.GetOutOffYPixel());
            aViewInformation2D.setViewTransformation(aTransform);
        }
#if defined(_WIN32)
        SystemGraphicsData aData(rTargetOutDev.GetSystemGfxData());
        std::unique_ptr<D2DPixelProcessor2D> aRetval(
            std::make_unique<D2DPixelProcessor2D>(aViewInformation2D, aData.hDC));
        if (aRetval->valid())
            return aRetval;
#elif USE_HEADLESS_CODE
        SystemGraphicsData aData(rTargetOutDev.GetSystemGfxData());
        std::unique_ptr<CairoPixelProcessor2D> aRetval(
            std::make_unique<CairoPixelProcessor2D>(aViewInformation2D, static_cast<cairo_surface_t*>(aData.pSurface)));
        if (aRetval->valid())
            return aRetval;
#endif
    }

    // create Pixel Vcl-Processor
    return std::make_unique<VclPixelProcessor2D>(rViewInformation2D, rTargetOutDev);
}

std::unique_ptr<BaseProcessor2D> createProcessor2DFromOutputDevice(
    OutputDevice& rTargetOutDev,
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D)
{
    const GDIMetaFile* pMetaFile = rTargetOutDev.GetConnectMetaFile();
    const bool bOutputToRecordingMetaFile(pMetaFile && pMetaFile->IsRecord()
                                          && !pMetaFile->IsPause());

    if (bOutputToRecordingMetaFile)
    {
        // create MetaFile Vcl-Processor and process
        return std::make_unique<VclMetafileProcessor2D>(rViewInformation2D, rTargetOutDev);
    }
    else
    {
        // create Pixel Vcl-Processor
        return createPixelProcessor2DFromOutputDevice(rTargetOutDev, rViewInformation2D);
    }
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
