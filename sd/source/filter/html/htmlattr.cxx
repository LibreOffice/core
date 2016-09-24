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

#include "htmlattr.hxx"
#include "htmlex.hxx"
#include <vcl/decoview.hxx>
#include <vcl/builderfactory.hxx>

SdHtmlAttrPreview::SdHtmlAttrPreview(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle)
{
}

VCL_BUILDER_DECL_FACTORY(SdHtmlAttrPreview)
{
    WinBits nWinStyle = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;

    rRet = VclPtr<SdHtmlAttrPreview>::Create(pParent, nWinStyle);
}

SdHtmlAttrPreview::~SdHtmlAttrPreview()
{
}

void SdHtmlAttrPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    DecorationView aDecoView(&rRenderContext);

    Rectangle aTextRect;
    aTextRect.SetSize(GetOutputSize());

    rRenderContext.SetLineColor(m_aBackColor);
    rRenderContext.SetFillColor(m_aBackColor);
    rRenderContext.DrawRect(rRect);
    rRenderContext.SetFillColor();

    int nHeight = (aTextRect.Bottom() - aTextRect.Top()) >> 2;
    aTextRect.Bottom() =  nHeight + aTextRect.Top();

    rRenderContext.SetTextColor(m_aTextColor);
    rRenderContext.DrawText(aTextRect, SD_RESSTR(STR_HTMLATTR_TEXT), DrawTextFlags::Center | DrawTextFlags::VCenter);

    aTextRect.Move(0,nHeight);
    rRenderContext.SetTextColor(m_aLinkColor);
    rRenderContext.DrawText(aTextRect, SD_RESSTR(STR_HTMLATTR_LINK), DrawTextFlags::Center | DrawTextFlags::VCenter);

    aTextRect.Move(0,nHeight);
    rRenderContext.SetTextColor(m_aALinkColor);
    rRenderContext.DrawText(aTextRect, SD_RESSTR(STR_HTMLATTR_ALINK), DrawTextFlags::Center | DrawTextFlags::VCenter);

    aTextRect.Move(0,nHeight);
    rRenderContext.SetTextColor(m_aVLinkColor);
    rRenderContext.DrawText(aTextRect, SD_RESSTR(STR_HTMLATTR_VLINK), DrawTextFlags::Center | DrawTextFlags::VCenter);
}

void SdHtmlAttrPreview::SetColors(Color& aBack, Color& aText, Color& aLink,
                                  Color& aVLink, Color& aALink)
{
    m_aBackColor = aBack;
    m_aTextColor = aText;
    m_aLinkColor = aLink;
    m_aVLinkColor = aVLink;
    m_aALinkColor = aALink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
