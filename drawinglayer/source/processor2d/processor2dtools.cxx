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
#include <officecfg/Office/Common.hxx>
#endif

using namespace com::sun::star;

namespace drawinglayer::processor2d
{
std::unique_ptr<BaseProcessor2D> createPixelProcessor2DFromScratch(
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D,
    sal_uInt32 nPixelWidth,
    sal_uInt32 nPixelHeight,
    bool bUseRGBA)
{
    if (0 == nPixelWidth || 0 == nPixelHeight)
        // error: no size given
        return nullptr;

#if USE_HEADLESS_CODE
    // Linux/Cairo: now globally activated in master. Leave a
    // possibility to deactivate for easy test/request testing
    static bool bUsePrimitiveRenderer(nullptr == std::getenv("DISABLE_SYSTEM_DEPENDENT_PRIMITIVE_RENDERER"));

    if (bUsePrimitiveRenderer)
    {
        // create CairoPixelProcessor2D with given size
        std::unique_ptr<CairoPixelProcessor2D> aRetval(
            std::make_unique<CairoPixelProcessor2D>(
                rViewInformation2D,
                nPixelWidth,
                nPixelHeight,
                bUseRGBA));

        if (aRetval->valid())
            return aRetval;
    }
#endif

    // avoid unused parameter errors
    (void)rViewInformation2D;
    (void)nPixelWidth;
    (void)nPixelHeight;
    (void)bUseRGBA;

    // error: no result when no SDPR supported
    return nullptr;
}

std::unique_ptr<BaseProcessor2D> createPixelProcessor2DFromOutputDevice(
    OutputDevice& rTargetOutDev,
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D)
{
#if defined(_WIN32)
    // Windows: make dependent on TEST_SYSTEM_PRIMITIVE_RENDERER
    static bool bUsePrimitiveRenderer(nullptr != std::getenv("TEST_SYSTEM_PRIMITIVE_RENDERER"));

    if (bUsePrimitiveRenderer)
    {
        drawinglayer::geometry::ViewInformation2D aViewInformation2D(rViewInformation2D);

        // if mnOutOffX/mnOutOffY is set (a 'hack' to get a cheap additional offset), apply it additionally
        // NOTE: This will also need to take extended size of target device into
        //       consideration, using D2DPixelProcessor2D *will* have to clip
        //       against that. Thus for now this is *not* sufficient (see tdf#163125)
        if(0 != rTargetOutDev.GetOutOffXPixel() || 0 != rTargetOutDev.GetOutOffYPixel())
        {
            basegfx::B2DHomMatrix aTransform(aViewInformation2D.getViewTransformation());
            aTransform.translate(rTargetOutDev.GetOutOffXPixel(), rTargetOutDev.GetOutOffYPixel());
            aViewInformation2D.setViewTransformation(aTransform);
        }

        SystemGraphicsData aData(rTargetOutDev.GetSystemGfxData());
        std::unique_ptr<D2DPixelProcessor2D> aRetval(
            std::make_unique<D2DPixelProcessor2D>(aViewInformation2D, aData.hDC));
        if (aRetval->valid())
            return aRetval;
    }
#elif USE_HEADLESS_CODE
    // Linux/Cairo: now globally activated in master. Leave a
    // possibility to deactivate for easy test/request testing
    static bool bUsePrimitiveRenderer(nullptr == std::getenv("DISABLE_SYSTEM_DEPENDENT_PRIMITIVE_RENDERER"));

    if (bUsePrimitiveRenderer)
    {
        SystemGraphicsData aData(rTargetOutDev.GetSystemGfxData());

        // create CairoPixelProcessor2D, make use of the possibility to
        // add an initial clip relative to the real pixel dimensions of
        // the target surface. This is e.g. needed here due to the
        // existence of 'virtual' target surfaces that internally use an
        // offset and limited pixel size, mainly used for UI elements.
        // let the CairoPixelProcessor2D do this, it has internal,
        // system-specific possibilities to do that in an elegant and
        // efficient way (using cairo_surface_create_for_rectangle).
        std::unique_ptr<CairoPixelProcessor2D> aRetval(
            std::make_unique<CairoPixelProcessor2D>(
                rViewInformation2D, static_cast<cairo_surface_t*>(aData.pSurface),
                rTargetOutDev.GetOutOffXPixel(), rTargetOutDev.GetOutOffYPixel(),
                rTargetOutDev.GetOutputWidthPixel(), rTargetOutDev.GetOutputHeightPixel()));

        if (aRetval->valid())
        {
            // if we construct a CairoPixelProcessor2D from OutputDevice,
            // additionally set the XGraphics that can be obtained from
            // there. It may be used e.g. to render FormControls directly
            aRetval->setXGraphics(rTargetOutDev.CreateUnoGraphics());

            return aRetval;
        }
    }
#endif

    // default: create VclPixelProcessor2D
    // NOTE: Since this uses VCL OutputDevice in the VclPixelProcessor2D
    //       taking care of virtual devices is not needed, OutputDevice
    //       and VclPixelProcessor2D will traditionally take care of it
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

BitmapEx extractBitmapExFromBaseProcessor2D(const std::unique_ptr<BaseProcessor2D>& rProcessor)
{
    BitmapEx aRetval;

#if USE_HEADLESS_CODE
    // currently only defined for cairo
    CairoPixelProcessor2D* pSource(dynamic_cast<CairoPixelProcessor2D*>(rProcessor.get()));

    if (nullptr != pSource)
        aRetval = pSource->extractBitmapEx();
#endif

    // avoid unused parameter errors
    (void)rProcessor;

    // default: return empty BitmapEx
    return aRetval;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
