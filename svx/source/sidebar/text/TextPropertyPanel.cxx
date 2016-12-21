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

using namespace css;
using namespace css::uno;

namespace svx { namespace sidebar {

VclPtr<vcl::Window> TextPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const vcl::EnumContext& rContext)
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

TextPropertyPanel::TextPropertyPanel ( vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame, SfxBindings* pBindings, const vcl::EnumContext& /*rContext*/ )
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
    mpToolBoxBackgroundColor.clear();

    maFontSizeControl.dispose();

    PanelLayout::dispose();
}

void TextPropertyPanel::HandleContextChange (
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
        return;

    maContext = rContext;

    bool bWriterText = false;
    bool bDrawText = false;
    bool bNeedTextSpacing = false;

    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application::Calc, Context::Cell):
        case CombinedEnumContext(Application::Calc, Context::Pivot):
            // bNeedTextSpacing = false;
            break;

        case CombinedEnumContext(Application::Calc, Context::EditCell):
            bNeedTextSpacing = true;
            break;

        case CombinedEnumContext(Application::Calc, Context::DrawText):
        case CombinedEnumContext(Application::WriterVariants, Context::DrawText):
        case CombinedEnumContext(Application::WriterVariants, Context::Annotation):
        case CombinedEnumContext(Application::DrawImpress, Context::DrawText):
        case CombinedEnumContext(Application::DrawImpress, Context::Text):
        case CombinedEnumContext(Application::DrawImpress, Context::Table):
        case CombinedEnumContext(Application::DrawImpress, Context::OutlineText):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
            bNeedTextSpacing = true;
            bDrawText = true;
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Text):
        case CombinedEnumContext(Application::WriterVariants, Context::Table):
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
