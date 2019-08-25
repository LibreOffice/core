/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_B2DPOLYPOLYLINEDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_B2DPOLYPOLYLINEDRAWABLE_HXX

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
class VCL_DLLPUBLIC B2DPolyPolyLineDrawable : public Drawable
{
public:
    B2DPolyPolyLineDrawable(basegfx::B2DPolyPolygon aLinePolyPolygon, LineInfo const aLineInfo,
                            bool bUsesScaffolding = true)
        : Drawable(bUsesScaffolding)
        , maLinePolyPolygon(aLinePolyPolygon)
        , maLineInfo(aLineInfo)
    {
    }

protected:
    bool ShouldAddAction() const override { return false; }
    bool CanDraw(OutputDevice* pRenderContext) const override;

    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    bool Draw(OutputDevice* pRenderContext, basegfx::B2DPolyPolygon const& rLinePolyPolygon,
              LineInfo const& rLineInfo) const;

    basegfx::B2DPolyPolygon maLinePolyPolygon;
    LineInfo maLineInfo;
};

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
