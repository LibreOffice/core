/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/Drawable.hxx>

#include <sal/log.hxx>

bool vcl::Drawable::execute(OutputDevice* const pRenderContext) const
{
    assert(!pRenderContext->is_double_buffered_window());

    if (mbUsesScaffolding)
    {
        mpGraphics = pRenderContext->GetGraphics();
        if (!mpGraphics)
            return false;

        if (ShouldAddAction())
            AddAction(pRenderContext);

        if (!CanDraw(pRenderContext))
            return false;

        if (ShouldInitClipRegion())
        {
            if (!InitClipRegion(pRenderContext))
                return false;
        }

        if (ShouldInitLineColor())
            InitLineColor(pRenderContext);

        if (ShouldInitFillColor())
            InitFillColor(pRenderContext);

        if (!DrawCommand(pRenderContext))
            return false;

        if (UseAlphaVirtDev())
            DrawAlphaVirtDev(pRenderContext);
    }
    else
    {
        mpGraphics = pRenderContext->GetGraphics();
        if (!mpGraphics)
            return false;

        if (!DrawCommand(pRenderContext))
            return false;
    }

    return true;
}

bool vcl::Drawable::ShouldAddAction() const
{
    if (mpMetaAction)
        return true;
    else
        return false;
}

void vcl::Drawable::AddAction(OutputDevice* const pRenderContext) const
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();
    if (pMetaFile)
        pMetaFile->AddAction(mpMetaAction);
}

bool vcl::Drawable::InitClipRegion(OutputDevice* const pRenderContext) const
{
    if (pRenderContext->IsClipRegionInitialized())
        pRenderContext->InitClipRegion();

    if (pRenderContext->IsOutputClipped())
        return false;

    return true;
}

void vcl::Drawable::InitLineColor(OutputDevice* const pRenderContext) const
{
    if (pRenderContext->IsLineColorInitialized())
        pRenderContext->InitLineColor();
}

void vcl::Drawable::InitFillColor(OutputDevice* const pRenderContext) const
{
    if (pRenderContext->IsFillColorInitialized())
        pRenderContext->InitFillColor();
}

bool vcl::Drawable::DrawAlphaVirtDev(OutputDevice* const pRenderContext) const
{
    OutputDevice* pAlphaVDev = pRenderContext->GetAlphaVirtDev();

    if (pAlphaVDev)
    {
        this->execute(pAlphaVDev);
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
