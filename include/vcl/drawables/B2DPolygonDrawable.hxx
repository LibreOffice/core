/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLE_B2DPOLYGONDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLE_B2DPOLYGONDRAWABLE_HXX

#include <basegfx/polygon/b2dpolygon.hxx>

#include <vcl/metaact.hxx>
#include <vcl/drawables/Drawable.hxx>

#include <memory>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC B2DPolygonDrawable : public Drawable
{
public:
    B2DPolygonDrawable(basegfx::B2DPolygon const aB2DPolygon)
        : Drawable(false)
        , maB2DPolygon(aB2DPolygon)
    {
    }

protected:
    virtual bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    bool Draw(OutputDevice* pRenderContext, basegfx::B2DPolygon const& rB2DPolygon) const;

    basegfx::B2DPolygon maB2DPolygon;
};
} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
