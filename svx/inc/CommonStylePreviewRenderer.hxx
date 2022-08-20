/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <optional>
#include <vector>

#include <editeng/svxfont.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/StylePreviewRenderer.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/BreakIterator.hpp>

class OutputDevice;
class SfxStyleSheetBase;

using namespace css;

namespace svx
{
class CommonStylePreviewRenderer final : public sfx2::StylePreviewRenderer
{
    std::optional<SvxFont> m_oFont;
    std::optional<SvxFont> m_oCJKFont;
    std::optional<SvxFont> m_oCTLFont;
    Color maFontColor;
    Color maHighlightColor;
    Color maBackgroundColor;
    Size maPixelSize;
    OUString maStyleName;
    OUString maScriptText;
    css::uno::Reference<css::i18n::XBreakIterator> mxBreak;
    struct ScriptInfo
    {
        tools::Long textWidth;
        sal_uInt16 scriptType;
        sal_Int32 changePos;
        ScriptInfo(sal_uInt16 scrptType, sal_Int32 position)
            : textWidth(0)
            , scriptType(scrptType)
            , changePos(position)
        {
        }
    };
    std::vector<ScriptInfo> maScriptChanges;

    bool SetFontSize(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont);
    Size getRenderSize();
    void CheckScript();

public:
    CommonStylePreviewRenderer(const SfxObjectShell& rShell, OutputDevice& rOutputDev,
                               SfxStyleSheetBase* pStyle, tools::Long nMaxHeight);
    virtual ~CommonStylePreviewRenderer() override;

    virtual bool recalculate() override;
    virtual bool render(const tools::Rectangle& aRectangle,
                        RenderAlign eRenderAlign = RenderAlign::CENTER) override;
};

} // end namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
