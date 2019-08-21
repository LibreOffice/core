/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_DRAWABLES_DRAWABLE_HXX
#define INCLUDED_VCL_DRAWABLES_DRAWABLE_HXX

#include <vcl/dllapi.h>
#include <vcl/metaact.hxx>

class OutputDevice;
class MetaAction;
class SalGraphics;

namespace vcl
{
class VCL_DLLPUBLIC Drawable
{
public:
    Drawable()
        : mbUsesScaffolding(true)
    {
    }

    Drawable(bool bUseScaffolding)
        : mbUsesScaffolding(bUseScaffolding)
    {
    }

    virtual ~Drawable() {}

    bool execute(OutputDevice* pRenderContext) const;

protected:
    virtual bool ShouldAddAction() const;
    virtual bool CanDraw(OutputDevice* const) const { return true; }
    virtual bool ShouldInitClipRegion() const { return true; }
    virtual bool ShouldInitLineColor() const { return true; }
    virtual bool ShouldInitFillColor() const { return true; }
    virtual bool UseAlphaVirtDev() const { return true; }
    bool UseScaffolding() const { return mbUsesScaffolding; }

    virtual bool DrawCommand(OutputDevice* const pRenderContext) const = 0;

    virtual bool InitClipRegion(OutputDevice* const pRenderContext) const;
    virtual bool DrawAlphaVirtDev(OutputDevice* const pRenderContext) const;
    virtual void AddAction(OutputDevice* const pRenderContext) const;
    virtual void InitLineColor(OutputDevice* const pRenderContext) const;
    virtual void InitFillColor(OutputDevice* const pRenderContext) const;

    mutable SalGraphics* mpGraphics;
    bool mbUsesScaffolding;

    MetaAction* mpMetaAction;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
