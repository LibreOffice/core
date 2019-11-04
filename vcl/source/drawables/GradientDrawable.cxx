/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/color.hxx>

#include <vcl/drawables/PolygonDrawable.hxx>
#include <vcl/drawables/PolyPolygonDrawable.hxx>
#include <vcl/drawables/GradientDrawable.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>
#include <drawables/GradientDrawableHelper.hxx>

#include <cassert>

#define GRADIENT_DEFAULT_STEPCOUNT 0

namespace vcl
{
bool GradientDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    if (mbAddToMetaFile)
        return GradientDrawableHelper::AddGradientActions(pRenderContext, maRect, maGradient,
                                                          mpGradientMtf);

    if (mbUsesRect)
        return Draw(pRenderContext, maRect, maGradient);

    if (mbUsesPolyPolygon)
        return Draw(pRenderContext, maPolyPolygon, maGradient);

    return false;
}

bool GradientDrawable::Draw(OutputDevice* pRenderContext, tools::Rectangle const& rRect,
                            Gradient const& rGradient)
{
    tools::Polygon aPolygon(rRect);
    tools::PolyPolygon aPolyPoly(aPolygon);

    pRenderContext->Draw(GradientDrawable(aPolyPoly, rGradient));
    return true;
}

bool GradientDrawable::Draw(OutputDevice* pRenderContext, tools::PolyPolygon const& rPolyPolygon,
                            Gradient const& rGradient) const
{
    if (!(rPolyPolygon.Count() && rPolyPolygon[0].GetSize()))
        return false;

    if (pRenderContext->GetDrawMode()
        & (DrawModeFlags::BlackGradient | DrawModeFlags::WhiteGradient
           | DrawModeFlags::SettingsGradient))
    {
        Color aColor = GradientDrawableHelper::GetSingleColorGradientFill(pRenderContext);

        pRenderContext->Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
        pRenderContext->SetLineColor(aColor);
        pRenderContext->SetFillColor(aColor);
        pRenderContext->DrawPolyPolygon(maPolyPolygon);
        pRenderContext->Pop();
        return true;
    }

    Gradient aGradient(rGradient);

    if (pRenderContext->GetDrawMode() & DrawModeFlags::GrayGradient)
        GradientDrawableHelper::SetGrayscaleColors(pRenderContext, aGradient);

    AddAction(pRenderContext);

    if (!CanDraw(pRenderContext))
        return false;

    // Clip and then draw the gradient
    if (!tools::Rectangle(pRenderContext->PixelToLogic(Point()), pRenderContext->GetOutputSize())
             .IsEmpty())
    {
        const tools::Rectangle aBoundRect(rPolyPolygon.GetBoundRect());

        // convert rectangle to pixels
        tools::Rectangle aRect(pRenderContext->ImplLogicToDevicePixel(aBoundRect));
        aRect.Justify();

        // do nothing if the rectangle is empty
        if (!aRect.IsEmpty())
        {
            tools::PolyPolygon aClixPolyPoly(pRenderContext->ImplLogicToDevicePixel(rPolyPolygon));
            bool bDrawn = false;

            if (!InitClipRegion(pRenderContext))
                return false;

            // secure clip region
            pRenderContext->Push(PushFlags::CLIPREGION);
            pRenderContext->IntersectClipRegion(aBoundRect);

            // try to draw gradient natively
            bDrawn = mpGraphics->DrawGradient(aClixPolyPoly, aGradient);

            if (!bDrawn)
            {
                // draw gradients without border
                InitLineColor(pRenderContext);

                pRenderContext->FlagFillColorInitialized();

                // calculate step count if necessary
                if (!aGradient.GetSteps())
                    aGradient.SetSteps(GRADIENT_DEFAULT_STEPCOUNT);

                if (rPolyPolygon.IsRect())
                {
                    // because we draw with no border line, we have to expand gradient
                    // rect to avoid missing lines on the right and bottom edge
                    aRect = GradientDrawableHelper::ExpandGradientOverBorder(aRect);
                }

                // if the clipping polypolygon is a rectangle, then it's the same size as the bounding of the
                // polypolygon, so pass in a NULL for the clipping parameter
                if (aGradient.GetStyle() == GradientStyle::Linear
                    || rGradient.GetStyle() == GradientStyle::Axial)
                {
                    GradientDrawableHelper::DrawLinearGradient(
                        pRenderContext, aRect, aGradient,
                        aClixPolyPoly.IsRect() ? nullptr : &aClixPolyPoly);
                }
                else
                {
                    GradientDrawableHelper::DrawComplexGradient(
                        pRenderContext, aRect, aGradient,
                        aClixPolyPoly.IsRect() ? nullptr : &aClixPolyPoly);
                }
            }

            pRenderContext->Pop();
        }
    }

    DrawAlphaVirtDev(pRenderContext);

    return true;
}

void GradientDrawable::InitLineColor(OutputDevice* const pRenderContext) const
{
    if (pRenderContext->IsLineColor() || pRenderContext->IsLineColorInitialized())
        pRenderContext->InitLineColor();

    pRenderContext->FlagLineColorInitialized();
}

void GradientDrawable::AddAction(OutputDevice* const pRenderContext) const
{
    GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();

    if (!pMetaFile)
        return;

    if (maPolyPolygon.Count() && maPolyPolygon[0].GetSize())
    {
        Gradient aGradient(maGradient);

        if (pRenderContext->GetDrawMode() & DrawModeFlags::GrayGradient)
            GradientDrawableHelper::SetGrayscaleColors(pRenderContext, aGradient);

        const tools::Rectangle aBoundRect(maPolyPolygon.GetBoundRect());

        if (maPolyPolygon.IsRect())
        {
            pMetaFile->AddAction(new MetaGradientAction(aBoundRect, aGradient));
        }
        else
        {
            pMetaFile->AddAction(new MetaCommentAction("XGRAD_SEQ_BEGIN"));
            pMetaFile->AddAction(new MetaGradientExAction(maPolyPolygon, maGradient));

            pRenderContext->ClipAndDrawGradientMetafile(maGradient, maPolyPolygon);

            pMetaFile->AddAction(new MetaCommentAction("XGRAD_SEQ_END"));
        }

        if (!CanDraw(pRenderContext))
            return;

        // Clip and then draw the gradient
        if (!tools::Rectangle(pRenderContext->PixelToLogic(Point()),
                              pRenderContext->GetOutputSize())
                 .IsEmpty())
        {
            // convert rectangle to pixels
            tools::Rectangle aRect(pRenderContext->ImplLogicToDevicePixel(aBoundRect));
            aRect.Justify();

            // do nothing if the rectangle is empty
            if (!aRect.IsEmpty())
            {
                if (!pRenderContext->IsOutputClipped())
                {
                    // calculate step count if necessary
                    if (!aGradient.GetSteps())
                        aGradient.SetSteps(GRADIENT_DEFAULT_STEPCOUNT);

                    if (maPolyPolygon.IsRect())
                    {
                        // because we draw with no border line, we have to expand gradient
                        // rect to avoid missing lines on the right and bottom edge
                        aRect = GradientDrawableHelper::ExpandGradientOverBorder(aRect);
                    }

                    // if the clipping polypolygon is a rectangle, then it's the same size as the bounding of the
                    // polypolygon, so pass in a NULL for the clipping parameter
                    if (aGradient.GetStyle() == GradientStyle::Linear
                        || maGradient.GetStyle() == GradientStyle::Axial)
                    {
                        GradientDrawableHelper::DrawLinearGradientToMetafile(pRenderContext, aRect,
                                                                             aGradient);
                    }
                    else
                    {
                        GradientDrawableHelper::DrawComplexGradientToMetafile(pRenderContext, aRect,
                                                                              aGradient);
                    }
                }
            }
        }
    }
}

bool GradientDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary() || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

bool GradientDrawable::DrawAlphaVirtDev(OutputDevice* const pRenderContext) const
{
    VirtualDevice* pAlphaVDev = pRenderContext->GetAlphaVirtDev();

    if (pAlphaVDev)
    {
        pAlphaVDev->DrawPolyPolygon(maPolyPolygon);
        return true;
    }

    return false;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
