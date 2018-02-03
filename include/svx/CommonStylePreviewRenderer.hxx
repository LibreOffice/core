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

#include <memory>

#include <sfx2/objsh.hxx>
#include <sfx2/StylePreviewRenderer.hxx>
#include <svx/svxdllapi.h>
#include <rtl/ustring.hxx>
#include <vcl/color.hxx>
#include <tools/gen.hxx>

class OutputDevice;
class SfxStyleSheetBase;
class SvxFont;

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
                               SfxStyleSheetBase* pStyle, long nMaxHeight);
    virtual ~CommonStylePreviewRenderer() override;

    virtual bool recalculate() override;
    virtual Size getRenderSize() override;
    virtual bool render(const tools::Rectangle& aRectangle, RenderAlign eRenderAlign = RenderAlign::CENTER) override;
};

} // end namespace svx

#endif // INCLUDED_SVX_COMMONSTYLEPREVIEWRENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
