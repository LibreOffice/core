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

#include "TextPropertyPanel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>

using namespace css;

namespace svx::sidebar {

std::unique_ptr<PanelLayout> TextPropertyPanel::Create (
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to TextPropertyPanel::Create"_ustr, nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(u"no XFrame given to TextPropertyPanel::Create"_ustr, nullptr, 1);

    return std::make_unique<TextPropertyPanel>(pParent, rxFrame);
}

TextPropertyPanel::TextPropertyPanel(weld::Widget* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, u"SidebarTextPanel"_ustr, u"svx/ui/sidebartextpanel.ui"_ustr)
    , mxFont(m_xBuilder->weld_toolbar(u"font"_ustr))
    , mxFontDispatch(new ToolbarUnoDispatcher(*mxFont, *m_xBuilder, rxFrame))
    , mxFontHeight(m_xBuilder->weld_toolbar(u"fontheight"_ustr))
    , mxFontHeightDispatch(new ToolbarUnoDispatcher(*mxFontHeight, *m_xBuilder, rxFrame))
    , mxFontEffects(m_xBuilder->weld_toolbar(u"fonteffects"_ustr))
    , mxFontEffectsDispatch(new ToolbarUnoDispatcher(*mxFontEffects, *m_xBuilder, rxFrame))
    , mxFontAdjust(m_xBuilder->weld_toolbar(u"fontadjust"_ustr))
    , mxFontAdjustDispatch(new ToolbarUnoDispatcher(*mxFontAdjust, *m_xBuilder, rxFrame))
    , mxToolBoxFontColor(m_xBuilder->weld_toolbar(u"colorbar"_ustr))
    , mxToolBoxFontColorDispatch(new ToolbarUnoDispatcher(*mxToolBoxFontColor, *m_xBuilder, rxFrame))
    , mxToolBoxBackgroundColor(m_xBuilder->weld_toolbar(u"colorbar_background"_ustr))
    , mxToolBoxBackgroundColorDispatch(new ToolbarUnoDispatcher(*mxToolBoxBackgroundColor, *m_xBuilder, rxFrame))
    , mxResetBar(m_xBuilder->weld_toolbar(u"resetattr"_ustr))
    , mxResetBarDispatch(new ToolbarUnoDispatcher(*mxResetBar, *m_xBuilder, rxFrame))
    , mxDefaultBar(m_xBuilder->weld_toolbar(u"defaultattr"_ustr))
    , mxDefaultBarDispatch(new ToolbarUnoDispatcher(*mxDefaultBar, *m_xBuilder, rxFrame))
    , mxPositionBar(m_xBuilder->weld_toolbar(u"position"_ustr))
    , mxPositionBarDispatch(new ToolbarUnoDispatcher(*mxPositionBar, *m_xBuilder, rxFrame))
    , mxSpacingBar(m_xBuilder->weld_toolbar(u"spacingbar"_ustr))
    , mxSpacingBarDispatch(new ToolbarUnoDispatcher(*mxSpacingBar, *m_xBuilder, rxFrame))
{
    bool isMobilePhone = false;
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    if (comphelper::LibreOfficeKit::isActive() &&
            pViewShell && pViewShell->isLOKMobilePhone())
        isMobilePhone = true;
    mxSpacingBar->set_visible(!isMobilePhone);
}

TextPropertyPanel::~TextPropertyPanel()
{
    mxResetBarDispatch.reset();
    mxDefaultBarDispatch.reset();
    mxPositionBarDispatch.reset();
    mxSpacingBarDispatch.reset();
    mxToolBoxFontColorDispatch.reset();
    mxToolBoxBackgroundColorDispatch.reset();
    mxFontAdjustDispatch.reset();
    mxFontEffectsDispatch.reset();
    mxFontHeightDispatch.reset();
    mxFontDispatch.reset();

    mxResetBar.reset();
    mxDefaultBar.reset();
    mxPositionBar.reset();
    mxSpacingBar.reset();
    mxToolBoxFontColor.reset();
    mxToolBoxBackgroundColor.reset();
    mxFontAdjust.reset();
    mxFontEffects.reset();
    mxFontHeight.reset();
    mxFont.reset();
}

void TextPropertyPanel::HandleContextChange (
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
        return;

    maContext = rContext;

    bool bWriterText = false;
    bool bDrawText = false;
    bool bCalcText = false;

    switch (maContext.GetCombinedContext_DI())
    {
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
            bDrawText = true;
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Text):
        case CombinedEnumContext(Application::WriterVariants, Context::Table):
            bWriterText = true;
            break;

        case CombinedEnumContext(Application::Calc, Context::Text):
        case CombinedEnumContext(Application::Calc, Context::Table):
        case CombinedEnumContext(Application::Calc, Context::Cell):
        case CombinedEnumContext(Application::Calc, Context::EditCell):
        case CombinedEnumContext(Application::Calc, Context::Grid):
            bCalcText = true;
            break;

        default:
            break;
    }

    mxToolBoxBackgroundColor->set_visible(bWriterText || bDrawText);
    mxResetBar->set_visible(bWriterText || bCalcText);
    mxDefaultBar->set_visible(bDrawText);
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
