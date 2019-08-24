/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_INCLUDE_VCL_ROUNDRECTDRAWABLE_HXX
#define INCLUDED_INCLUDE_VCL_ROUNDRECTDRAWABLE_HXX

#include <tools/solar.h>
#include <tools/gen.hxx>

#include <vcl/metaact.hxx>
#include <vcl/drawables/Drawable.hxx>

class OutputDevice;

namespace vcl
{
class VCL_DLLPUBLIC RoundRectDrawable : public Drawable
{
public:
    RoundRectDrawable(tools::Rectangle aRect, sal_uLong nHorzRadius, sal_uLong nVertRadius)
        : maRect(aRect)
        , mnHorzRadius(nHorzRadius)
        , mnVertRadius(nVertRadius)
    {
        mpMetaAction = new MetaRoundRectAction(maRect, mnHorzRadius, mnVertRadius);
    }

protected:
    bool CanDraw(OutputDevice* pRenderContext) const override;

    bool DrawCommand(OutputDevice* pRenderContext) const override;

private:
    tools::Rectangle maRect;
    sal_uLong mnHorzRadius;
    sal_uLong mnVertRadius;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
