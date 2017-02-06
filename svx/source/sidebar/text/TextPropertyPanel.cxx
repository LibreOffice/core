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
#include <vcl/toolbox.hxx>

using namespace css;

namespace svx { namespace sidebar {

VclPtr<vcl::Window> TextPropertyPanel::Create (
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
{
    get(mpToolBoxFontColorSw, "colorbar_writer");
    get(mpToolBoxFontColor, "colorbar_others");
    get(mpToolBoxBackgroundColor, "colorbar_background");
}

TextPropertyPanel::~TextPropertyPanel()
{
    disposeOnce();
}

void TextPropertyPanel::dispose()
{
    mpToolBoxFontColorSw.clear();
    mpToolBoxFontColor.clear();
    mpToolBoxBackgroundColor.clear();

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

        default:
            break;
    }

    mpToolBoxFontColor->Show(!bWriterText);
    mpToolBoxFontColorSw->Show(bWriterText);
    mpToolBoxBackgroundColor->Show(bDrawText);
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
