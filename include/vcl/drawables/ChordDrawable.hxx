/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_CHORDDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_CHORDDRAWABLE_HXX

#include <tools/gen.hxx>

#include <vcl/drawables/Drawable.hxx>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC ChordDrawable : public Drawable
{
public:
    ChordDrawable(tools::Rectangle aRect, Point aStartPt, Point aEndPt)
        : maRect(aRect)
        , maStartPt(aStartPt)
        , maEndPt(aEndPt)
    {
        mpMetaAction = new MetaChordAction(aRect, aStartPt, aEndPt);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool ShouldInitFillColor() const override { return false; }

    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    tools::Rectangle maRect;
    Point maStartPt;
    Point maEndPt;
};

} // namespace vcl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
