/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLE_POLYLINEDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLE_POLYLINEDRAWABLE_HXX

#include <tools/poly.hxx>

#include <vcl/metaact.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/drawables/Drawable.hxx>

#include <memory>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC PolyLineDrawable : public Drawable
{
public:
    PolyLineDrawable(tools::Polygon aPolygon, LineInfo aLineInfo = LineInfo())
        : maPolygon(aPolygon)
        , maLineInfo(aLineInfo)
    {
        mpMetaAction = new MetaPolyLineAction(aPolygon, aLineInfo);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool ShouldInitFillColor() const override { return false; }

    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    bool Draw(OutputDevice* pRenderContext, tools::Polygon const& rPoly,
              LineInfo const& rLineInfo) const;

    tools::Polygon maPolygon;
    LineInfo maLineInfo;
};
} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
