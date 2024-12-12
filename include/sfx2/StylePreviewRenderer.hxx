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
#include <sfx2/objsh.hxx>

class OutputDevice;
class SfxStyleSheetBase;

namespace sfx2
{

class SFX2_DLLPUBLIC StylePreviewRenderer
{
protected:
    const SfxObjectShell& mrShell;
    OutputDevice& mrOutputDev;
    SfxStyleSheetBase* mpStyle;
    tools::Long mnMaxHeight;

public:
    enum class RenderAlign
    {
        TOP, CENTER
    };

    StylePreviewRenderer(const SfxObjectShell& rShell,
                         OutputDevice& rOutputDev,
                         SfxStyleSheetBase* pStyle,
                         tools::Long nMaxHeight)
        : mrShell(rShell)
        , mrOutputDev(rOutputDev)
        , mpStyle(pStyle)
        , mnMaxHeight(nMaxHeight)
    {}

    virtual ~StylePreviewRenderer()
    {}

    virtual bool recalculate() = 0;
    virtual bool render(const tools::Rectangle& aRectangle, RenderAlign eRenderAlign = RenderAlign::CENTER) = 0;
};

} // end namespace sfx2

SFX2_DLLPUBLIC Color ColorHash(std::u16string_view rString);

#endif // INCLUDED_SVX_STYLEPREVIEWRENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
