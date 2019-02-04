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

#include <svx/fontlb.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/viewdataentry.hxx>

SvLBoxFontString::SvLBoxFontString()
    : SvLBoxString()
{
}

SvLBoxFontString::~SvLBoxFontString()
{
}

std::unique_ptr<SvLBoxItem> SvLBoxFontString::Clone(SvLBoxItem const *) const
{
    return std::unique_ptr<SvLBoxItem>(new SvLBoxFontString);
}

void SvLBoxFontString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                             const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{
    rRenderContext.Push(PushFlags::FONT);
    vcl::Font aNewFont;
    bool bSel = pView->IsSelected();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    aNewFont.SetColor(bSel ? rStyleSettings.GetHighlightTextColor() : rStyleSettings.GetFieldTextColor());

    rRenderContext.SetFont(aNewFont);
    SvLBoxString::Paint(rPos, rDev, rRenderContext, pView, rEntry);
    rRenderContext.Pop();
}

void SvLBoxFontString::InitViewData( SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData )
{
    vcl::Font aOldFont( pView->GetFont() );
    pView->Control::SetFont( vcl::Font() );
    SvLBoxString::InitViewData( pView, pEntry, pViewData);
    pView->Control::SetFont( aOldFont );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
