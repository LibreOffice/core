/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_POLYGONDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_POLYGONDRAWABLE_HXX

#include <tools/gen.hxx>

#include <vcl/drawables/Drawable.hxx>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC PolygonDrawable : public Drawable
{
public:
    PolygonDrawable(tools::Polygon aPolygon)
        : maPolygon(aPolygon)
        , mbClipped(false)
    {
        mpMetaAction = new MetaPolygonAction(aPolygon);
    }

    PolygonDrawable(tools::Polygon aPolygon, tools::PolyPolygon aClipPolyPolygon)
        : maPolygon(aPolygon)
        , maClipPolyPolygon(aClipPolyPolygon)
        , mbClipped(false)
    {
        mpMetaAction = new MetaPolygonAction(aPolygon);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    bool Draw(OutputDevice* pRenderContext, tools::Polygon const& rPolygon) const;
    static bool Draw(OutputDevice* pRenderContext, tools::Polygon const& rPolygon,
              tools::PolyPolygon const& rClipPolyPolygon);

    tools::Polygon maPolygon;
    tools::PolyPolygon maClipPolyPolygon;
    bool mbClipped;
};

} // namespace vcl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
