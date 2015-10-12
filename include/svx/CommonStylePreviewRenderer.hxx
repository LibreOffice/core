/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_COMMONSTYLEPREVIEWRENDERER_HXX
#define INCLUDED_SVX_COMMONSTYLEPREVIEWRENDERER_HXX

#include <vcl/outdev.hxx>
#include <rsc/rscsfx.hxx>
#include <editeng/svxfont.hxx>
#include <svx/svxdllapi.h>

#include <sfx2/StylePreviewRenderer.hxx>

namespace svx
{

class SVX_DLLPUBLIC CommonStylePreviewRenderer : public sfx2::StylePreviewRenderer
{
    std::unique_ptr<SvxFont> m_pFont;
    Color maFontColor;
    Color maBackgroundColor;
    Size maPixelSize;
    OUString maStyleName;

public:
    CommonStylePreviewRenderer(const SfxObjectShell& rShell, OutputDevice& rOutputDev,
                               SfxStyleSheetBase* pStyle, long nMaxHeight = 32);
    virtual ~CommonStylePreviewRenderer();

    virtual bool recalculate() override;
    virtual Size getRenderSize() override;
    virtual bool render(const Rectangle& aRectangle, RenderAlign eRenderAlign = RenderAlign::CENTER) override;
};

} // end namespace svx

#endif // INCLUDED_SVX_COMMONSTYLEPREVIEWRENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
