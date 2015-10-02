/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_STYLEPREVIEWRENDERER_HXX
#define INCLUDED_SFX2_STYLEPREVIEWRENDERER_HXX

#include <sfx2/dllapi.h>
#include <vcl/outdev.hxx>
#include <rsc/rscsfx.hxx>
#include <svl/style.hxx>
#include <sfx2/objsh.hxx>

namespace sfx2
{

class SFX2_DLLPUBLIC StylePreviewRenderer
{
protected:
    const SfxObjectShell& mrShell;
    OutputDevice& mrOutputDev;
    SfxStyleSheetBase* mpStyle;
    long mnMaxHeight;
    OUString msRenderText;

public:
    enum class RenderAlign
    {
        TOP, CENTER, BOTTOM
    };

    StylePreviewRenderer(const SfxObjectShell& rShell,
                         OutputDevice& rOutputDev,
                         SfxStyleSheetBase* pStyle,
                         long nMaxHeight = 32)
        : mrShell(rShell)
        , mrOutputDev(rOutputDev)
        , mpStyle(pStyle)
        , mnMaxHeight(nMaxHeight)
        , msRenderText()
    {}

    virtual ~StylePreviewRenderer()
    {}

    virtual bool recalculate() = 0;
    virtual Size getRenderSize() = 0;
    virtual bool render(const Rectangle& aRectangle, RenderAlign eRenderAlign = RenderAlign::CENTER) = 0;
};

} // end namespace sfx2

#endif // INCLUDED_SVX_STYLEPREVIEWRENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
