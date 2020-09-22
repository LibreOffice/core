/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include <sfx2/objsh.hxx>
#include <sfx2/StylePreviewRenderer.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>

class OutputDevice;
class SfxStyleSheetBase;
class SvxFont;

namespace svx
{
class CommonStylePreviewRenderer final : public sfx2::StylePreviewRenderer
{
    std::unique_ptr<SvxFont> m_pFont;
    Color maFontColor;
    Color maHighlightColor;
    Color maBackgroundColor;
    Size maPixelSize;
    OUString maStyleName;

    Size getRenderSize() const;

public:
    CommonStylePreviewRenderer(const SfxObjectShell& rShell, OutputDevice& rOutputDev,
                               SfxStyleSheetBase* pStyle, long nMaxHeight);
    virtual ~CommonStylePreviewRenderer() override;

    virtual bool recalculate() override;
    virtual bool render(const tools::Rectangle& aRectangle,
                        RenderAlign eRenderAlign = RenderAlign::CENTER) override;
};

} // end namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
