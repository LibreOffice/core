/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_DRAWABLES_RECTANGLEDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_DRAWABLES_RECTANGLEDRAWABLE_HXX

#include <tools/gen.hxx>

#include <vcl/drawables/Drawable.hxx>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC RectangleDrawable : public Drawable
{
public:
    RectangleDrawable(tools::Rectangle aRect)
        : maRect(aRect)
    {
        mpMetaAction = new MetaRectAction(aRect);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;
    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    tools::Rectangle maRect;
};

} // namespace vcl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
