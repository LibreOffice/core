/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLE_POLYHAIRLINEDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLE_POLYHAIRLINEDRAWABLE_HXX

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <vcl/lineinfo.hxx>
#include <vcl/drawables/Drawable.hxx>

#include <memory>

class OutputDevice;

namespace vcl
{
/** Render the given polygon as a hairline stroke

    The given polygon is stroked with the current LineColor, start
    and end point are not automatically connected. The line is
    rendered according to the specified LineInfo, e.g. supplying a
    dash pattern, or a line thickness.
*/
class VCL_DLLPUBLIC PolyHairlineDrawable : public Drawable
{
public:
    PolyHairlineDrawable(basegfx::B2DHomMatrix const& rObjectTransform,
                         basegfx::B2DPolygon aPolygon, LineInfo aLineInfo,
                         double fTransparency = 0.0,
                         double fMiterMinimumAngle = basegfx::deg2rad(15.0))
        : maObjectTransform(rObjectTransform)
        , maB2DPolygon(aPolygon)
        , maLineInfo(aLineInfo)
        , mfTransparency(fTransparency)
        , mfMiterMinimumAngle(fMiterMinimumAngle)
    {
        LineInfo aTmpLineInfo;
        double fLineWidth = maLineInfo.GetWidth();
        if (fLineWidth != 0.0)
            aTmpLineInfo.SetWidth(static_cast<long>(fLineWidth + 0.5));

        // Transport known information, might be needed
        aTmpLineInfo.SetLineJoin(aLineInfo.GetLineJoin());
        aTmpLineInfo.SetLineCap(aLineInfo.GetLineCap());

        // MiterMinimumAngle does not exist yet in LineInfo
        const tools::Polygon aToolsPolygon(maB2DPolygon);
        mpMetaAction = new MetaPolyLineAction(aToolsPolygon, aTmpLineInfo);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool ShouldAddAction() const override { return false; }
    bool ShouldInitFillColor() const override { return false; }
    bool UseAlphaVirtDev() const override { return false; }

    /** Tries to use SalGDI's DrawPolyLine directly and returns its bool. See #i101491#.
    */
    virtual bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    basegfx::B2DHomMatrix maObjectTransform;
    basegfx::B2DPolygon maB2DPolygon;
    LineInfo maLineInfo;
    double mfTransparency;
    double mfMiterMinimumAngle;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
