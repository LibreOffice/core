/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLE_PIXELDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLE_PIXELDRAWABLE_HXX

#include <tools/gen.hxx>
#include <tools/color.hxx>

#include <vcl/metaact.hxx>
#include <vcl/drawables/Drawable.hxx>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC PixelDrawable : public Drawable
{
public:
    PixelDrawable(Point aPt)
        : maPt(aPt)
        , mbUsesColor(false)
    {
        mpMetaAction = new MetaPointAction(aPt);
    }

    PixelDrawable(Point aPt, Color aColor)
        : maPt(aPt)
        , maColor(aColor)
        , mbUsesColor(true)
    {
        mpMetaAction = new MetaPixelAction(aPt, aColor);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool ShouldInitFillColor() const override { return false; }
    bool DrawAlphaVirtDev(OutputDevice* pRenderContext) const override;

    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    Point const maPt;
    Color const maColor;
    bool const mbUsesColor;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
