/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLE_POLYPOLYGONDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLE_POLYPOLYGONDRAWABLE_HXX

#include <tools/poly.hxx>

#include <vcl/metaact.hxx>
#include <vcl/drawables/Drawable.hxx>

#include <memory>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC PolyPolygonDrawable : public Drawable
{
public:
    PolyPolygonDrawable(tools::PolyPolygon const aPolyPolygon)
        : maPolyPolygon(aPolyPolygon)
        , mbUsesClipping(false)
        , mbRecursive(false)
    {
        mpMetaAction = new MetaPolyPolygonAction(aPolyPolygon);
    }

    PolyPolygonDrawable(tools::PolyPolygon const aPolyPolygon,
                        tools::PolyPolygon const aClipPolyPolygon)
        : Drawable(false)
        , maPolyPolygon(aPolyPolygon)
        , maClipPolyPolygon(aClipPolyPolygon)
        , mbUsesClipping(true)
        , mbRecursive(false)
    {
    }

    PolyPolygonDrawable(sal_uInt16 nPoly, tools::PolyPolygon const aPolyPoly)
        : Drawable(false)
        , maPolyPolygon(aPolyPoly)
        , mnPoly(nPoly)
        , mbUsesClipping(false)
        , mbRecursive(true)
    {
    }

protected:
    virtual bool CanDraw(OutputDevice* pRenderContext) const override;

    virtual bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    bool Draw(OutputDevice* pRenderContext, tools::PolyPolygon const& rPolyPolygon) const;
    bool Draw(OutputDevice* pRenderContext, tools::PolyPolygon const& rPolyPolygon,
              tools::PolyPolygon const& rClipPolyPolygon) const;
    bool Draw(OutputDevice* pRenderContext, sal_uInt16 nPoly,
              tools::PolyPolygon const& rPolyPoly) const;

    tools::PolyPolygon maPolyPolygon;
    tools::PolyPolygon maClipPolyPolygon;
    sal_uInt16 mnPoly;
    bool mbUsesClipping;
    bool mbRecursive;
};
} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
