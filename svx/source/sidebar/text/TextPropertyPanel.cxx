/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "TextPropertyPanel.hrc"
#include "TextPropertyPanel.hxx"

#include <editeng/kernitem.hxx>
#include <editeng/udlnitem.hxx>
#include <rtl/ref.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/unitconv.hxx>

#include <vcl/toolbox.hxx>
#include <svx/sidebar/PopupContainer.hxx>


using namespace css;
using namespace css::uno;

namespace svx { namespace sidebar {

VclPtr<vcl::Window> TextPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to TextPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to TextPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to TextPropertyPanel::Create", nullptr, 2);

    return VclPtr<TextPropertyPanel>::Create(
                pParent,
                rxFrame,
                pBindings,
                rContext);
}

TextPropertyPanel::TextPropertyPanel ( vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame, SfxBindings* pBindings, const ::sfx2::sidebar::EnumContext& /*rContext*/ )
    : PanelLayout(pParent, "SidebarTextPanel", "svx/ui/sidebartextpanel.ui", rxFrame),
        maFontSizeControl   (SID_ATTR_CHAR_FONTHEIGHT,  *pBindings, *this, OUString("FontHeight"),   rxFrame),

        maContext()
{
    get(mpToolBoxFont, "fonteffects");
    get(mpToolBoxIncDec, "fontadjust");
    get(mpToolBoxSpacing, "spacingbar");
    get(mpToolBoxFontColorSw, "colorbar_writer");
    get(mpToolBoxFontColor, "colorbar_others");
    get(mpToolBoxBackgroundColor, "colorbar_background");

    //init state
    mpHeightItem = nullptr;
}

TextPropertyPanel::~TextPropertyPanel()
{
    disposeOnce();
}

void TextPropertyPanel::dispose()
{
    mpToolBoxFont.clear();
    mpToolBoxIncDec.clear();
    mpToolBoxSpacing.clear();
    mpToolBoxFontColorSw.clear();
    mpToolBoxFontColor.clear();

    maFontSizeControl.dispose();

    PanelLayout::dispose();
}

void TextPropertyPanel::HandleContextChange (
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if (maContext == rContext)
        return;

    maContext = rContext;

    bool bWriterText = false;
    bool bDrawText = false;
    bool bNeedTextSpacing = false;

    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application_Calc, Context_Cell):
        case CombinedEnumContext(Application_Calc, Context_Pivot):
            // bNeedTextSpacing = false;
            break;

        case CombinedEnumContext(Application_Calc, Context_EditCell):
            bNeedTextSpacing = true;
            break;

        case CombinedEnumContext(Application_Calc, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Text):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            bNeedTextSpacing = true;
            bDrawText = true;
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Text):
        case CombinedEnumContext(Application_WriterVariants, Context_Table):
            bNeedTextSpacing = true;
            bWriterText = true;
            break;

        default:
            break;
    }

    mpToolBoxSpacing->Enable(bNeedTextSpacing);
    mpToolBoxFontColor->Show(!bWriterText);
    mpToolBoxFontColorSw->Show(bWriterText);
    mpToolBoxBackgroundColor->Show(bDrawText);
}

void TextPropertyPanel::NotifyItemUpdate (
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    switch(nSID)
    {
    case SID_ATTR_CHAR_FONTHEIGHT:
        {
            if (  eState >= SfxItemState::DEFAULT && dynamic_cast<const SvxFontHeightItem*>( pState) !=  nullptr )
                mpHeightItem = const_cast<SvxFontHeightItem*>(static_cast<const SvxFontHeightItem*>(pState));
            else
                mpHeightItem = nullptr;
        }
        break;
    case SID_ATTR_CHAR_KERNING:
        {
            mpToolBoxSpacing->Enable(bIsEnabled);
        }
        break;
    }
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
