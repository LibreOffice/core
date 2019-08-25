/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_B2DPOLYLINEDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_B2DPOLYLINEDRAWABLE_HXX

#include <tools/gen.hxx>

#include <vcl/metaact.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/drawables/Drawable.hxx>

#include <memory>

class OutputDevice;

namespace vcl
{
/** Helper for line geometry paint with support for graphic expansion (pattern and fat_to_area)
 */
class VCL_DLLPUBLIC B2DPolyLineDrawable : public Drawable
{
public:
    B2DPolyLineDrawable(basegfx::B2DPolygon aPolyLine, LineInfo const aLineInfo,
                        double fMiterMinimumAngle, bool bUsesScaffolding = true)
        : Drawable(bUsesScaffolding)
        , maPolyLine(aPolyLine)
        , maLineInfo(aLineInfo)
        , mfMiterMinimumAngle(fMiterMinimumAngle)
    {
        LineInfo aTmpLineInfo(aLineInfo);
        if (aTmpLineInfo.GetWidth() != 0.0)
            aTmpLineInfo.SetWidth(static_cast<long>(aLineInfo.GetWidth() + 0.5));

        const tools::Polygon aToolsPolygon(aPolyLine);
        mpMetaAction = new MetaPolyLineAction(aToolsPolygon, aTmpLineInfo);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool UseAlphaVirtDev() const override { return false; }

    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    bool Draw(OutputDevice* pRenderContext, basegfx::B2DPolygon const& rB2DPolygon,
              LineInfo const& rLineInfo, double fMiterMinimumAngle) const;

    basegfx::B2DPolygon maPolyLine;
    LineInfo maLineInfo;
    double mfMiterMinimumAngle;
};

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
