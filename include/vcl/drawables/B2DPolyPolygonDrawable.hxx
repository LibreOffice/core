/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_B2DPOLYPOLYGONDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_B2DPOLYPOLYGONDRAWABLE_HXX

#include <tools/gen.hxx>

#include <vcl/metaact.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/drawables/Drawable.hxx>

#include <memory>

class OutputDevice;

namespace vcl
{
// Caution: This method is nearly the same as
// OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency),
// so when changes are made here do not forget to make changes there, too

class VCL_DLLPUBLIC B2DPolyPolygonDrawable : public Drawable
{
public:
    B2DPolyPolygonDrawable(basegfx::B2DPolyPolygon aPolyPolygon)
        : Drawable(false)
        , maPolyPolygon(aPolyPolygon)
    {
        mpMetaAction = new MetaPolyPolygonAction(tools::PolyPolygon(aPolyPolygon));
    }

protected:
    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    basegfx::B2DPolyPolygon maPolyPolygon;
};

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
