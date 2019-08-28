/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_GRADIENTDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_GRADIENTDRAWABLE_HXX

#include <tools/gen.hxx>
#include <tools/poly.hxx>

#include <vcl/gradient.hxx>
#include <vcl/drawables/Drawable.hxx>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC GradientDrawable : public Drawable
{
public:
    GradientDrawable(tools::Rectangle aRect, Gradient aGradient)
        : Drawable(false)
        , maRect(aRect)
        , maGradient(aGradient)
        , mbUsesRect(true)
        , mbUsesPolyPolygon(false)
        , mbAddToMetaFile(false)
    {
    }

    GradientDrawable(tools::PolyPolygon aPolyPolygon, Gradient aGradient)
        : Drawable(false)
        , maPolyPolygon(aPolyPolygon)
        , maGradient(aGradient)
        , mbUsesRect(false)
        , mbUsesPolyPolygon(true)
        , mbAddToMetaFile(false)
    {
    }

    GradientDrawable(tools::Rectangle aRect, Gradient aGradient, GDIMetaFile* pMtf)
        : Drawable(false)
        , maRect(aRect)
        , maGradient(aGradient)
        , mbUsesRect(true)
        , mbUsesPolyPolygon(false)
        , mbAddToMetaFile(true)
    {
        mpGradientMtf = pMtf;
    }

protected:
    bool CanDraw(OutputDevice* const) const override;
    bool DrawAlphaVirtDev(OutputDevice* const pRenderContext) const override;
    void AddAction(OutputDevice* const pRenderContext) const override;
    void InitLineColor(OutputDevice* const pRenderContext) const override;

    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    static bool Draw(OutputDevice* pRenderContext, tools::Rectangle const& rRect,
                     Gradient const& rGradient);
    bool Draw(OutputDevice* pRenderContext, tools::PolyPolygon const& rRect,
              Gradient const& rGradient) const;

    static bool AddGradientActions(OutputDevice* pRenderContext, tools::Rectangle const& rRect,
                                   Gradient const& rGradient, GDIMetaFile* pMetaFile);

    SAL_DLLPRIVATE void DrawLinearGradient(OutputDevice* pRenderContext,
                                           const tools::Rectangle& rRect, const Gradient& rGradient,
                                           const tools::PolyPolygon* pClipPolyPoly) const;

    SAL_DLLPRIVATE void DrawComplexGradient(OutputDevice* pRenderContext,
                                            const tools::Rectangle& rRect,
                                            const Gradient& rGradient,
                                            const tools::PolyPolygon* pClipPolyPoly) const;

    SAL_DLLPRIVATE void DrawGradientToMetafile(OutputDevice* pRenderContext,
                                               const tools::PolyPolygon& rPolyPoly,
                                               const Gradient& rGradient) const;

    SAL_DLLPRIVATE static void DrawLinearGradientToMetafile(OutputDevice* pRenderContext,
                                                            const tools::Rectangle& rRect,
                                                            const Gradient& rGradient);

    SAL_DLLPRIVATE static void DrawComplexGradientToMetafile(OutputDevice* pRenderContext,
                                                             const tools::Rectangle& rRect,
                                                             const Gradient& rGradient);

    SAL_DLLPRIVATE static void SetGrayscaleColors(OutputDevice* pRenderContext,
                                                  Gradient const& rGradient);
    SAL_DLLPRIVATE static long GetGradientSteps(OutputDevice* pRenderContext,
                                                const Gradient& rGradient,
                                                const tools::Rectangle& rRect, bool bMtf,
                                                bool bComplex = false);

    SAL_DLLPRIVATE static Color GetSingleColorGradientFill(OutputDevice* pRenderContext);
    SAL_DLLPRIVATE static void SetGrayscaleColors(OutputDevice* pRenderContext,
                                                  Gradient& rGradient);

    tools::Rectangle maRect;
    tools::PolyPolygon maPolyPolygon;
    Gradient maGradient;
    bool mbUsesRect;
    bool mbUsesPolyPolygon;
    bool mbAddToMetaFile;

    mutable GDIMetaFile* mpGradientMtf;
};

void VCL_DLLPUBLIC AddGradientActions(OutputDevice* pRenderContext, tools::Rectangle const& rRect,
                                      Gradient const& rGradient, GDIMetaFile* pMetaFile);

} // namespace vcl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
