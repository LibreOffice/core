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

VclPtr<PanelLayout> TextPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to TextPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to TextPropertyPanel::Create", nullptr, 1);

    return VclPtr<TextPropertyPanel>::Create(pParent, rxFrame);
}

TextPropertyPanel::TextPropertyPanel ( vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame )
    : PanelLayout(pParent, "SidebarTextPanel", "svx/ui/sidebartextpanel.ui", rxFrame)
    , mxFont(m_xBuilder->weld_toolbar("font"))
    , mxFontDispatch(new ToolbarUnoDispatcher(*mxFont, *m_xBuilder, rxFrame))
    , mxFontHeight(m_xBuilder->weld_toolbar("fontheight"))
    , mxFontHeightDispatch(new ToolbarUnoDispatcher(*mxFontHeight, *m_xBuilder, rxFrame))
    , mxFontEffects(m_xBuilder->weld_toolbar("fonteffects"))
    , mxFontEffectsDispatch(new ToolbarUnoDispatcher(*mxFontEffects, *m_xBuilder, rxFrame))
    , mxFontAdjust(m_xBuilder->weld_toolbar("fontadjust"))
    , mxFontAdjustDispatch(new ToolbarUnoDispatcher(*mxFontAdjust, *m_xBuilder, rxFrame))
    , mxToolBoxFontColorSw(m_xBuilder->weld_toolbar("colorbar_writer"))
    , mxToolBoxFontColorSwDispatch(new ToolbarUnoDispatcher(*mxToolBoxFontColorSw, *m_xBuilder, rxFrame))
    , mxToolBoxFontColor(m_xBuilder->weld_toolbar("colorbar_others"))
    , mxToolBoxFontColorDispatch(new ToolbarUnoDispatcher(*mxToolBoxFontColor, *m_xBuilder, rxFrame))
    , mxToolBoxBackgroundColor(m_xBuilder->weld_toolbar("colorbar_background"))
    , mxToolBoxBackgroundColorDispatch(new ToolbarUnoDispatcher(*mxToolBoxBackgroundColor, *m_xBuilder, rxFrame))
    , mxResetBar(m_xBuilder->weld_toolbar("resetattr"))
    , mxResetBarDispatch(new ToolbarUnoDispatcher(*mxResetBar, *m_xBuilder, rxFrame))
    , mxDefaultBar(m_xBuilder->weld_toolbar("defaultattr"))
    , mxDefaultBarDispatch(new ToolbarUnoDispatcher(*mxDefaultBar, *m_xBuilder, rxFrame))
    , mxPositionBar(m_xBuilder->weld_toolbar("position"))
    , mxPositionBarDispatch(new ToolbarUnoDispatcher(*mxPositionBar, *m_xBuilder, rxFrame))
    , mxSpacingBar(m_xBuilder->weld_toolbar("spacingbar"))
    , mxSpacingBarDispatch(new ToolbarUnoDispatcher(*mxSpacingBar, *m_xBuilder, rxFrame))
{
    bool isMobilePhone = false;
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    if (comphelper::LibreOfficeKit::isActive() &&
            pViewShell && pViewShell->isLOKMobilePhone())
        isMobilePhone = true;
    mxSpacingBar->set_visible(!isMobilePhone);

    m_pInitialFocusWidget = mxFont.get();
}

TextPropertyPanel::~TextPropertyPanel()
{
    disposeOnce();
}

void TextPropertyPanel::dispose()
{
    mxResetBarDispatch.reset();
    mxDefaultBarDispatch.reset();
    mxPositionBarDispatch.reset();
    mxSpacingBarDispatch.reset();
    mxToolBoxFontColorSwDispatch.reset();
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
    mxToolBoxFontColorSw.reset();
    mxToolBoxFontColor.reset();
    mxToolBoxBackgroundColor.reset();
    mxFontAdjust.reset();
    mxFontEffects.reset();
    mxFontHeight.reset();
    mxFont.reset();

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

    mxToolBoxFontColor->set_visible(!bWriterText);
    mxToolBoxFontColorSw->set_visible(bWriterText);
    mxToolBoxBackgroundColor->set_visible(bDrawText);
    mxResetBar->set_visible(bWriterText || bCalcText);
    mxDefaultBar->set_visible(bDrawText);
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
