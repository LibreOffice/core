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

#include <utility>
#include <vcl/toolkit/treelistbox.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/viewdataentry.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/decoview.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/settings.hxx>

// ***************************************************************
// class SvLBoxString
// ***************************************************************


SvLBoxString::SvLBoxString(OUString aStr)
    : mbEmphasized(false)
    , mbCustom(false)
    , meAlign(TxtAlign::Left)
    , maText(std::move(aStr))
{
}

SvLBoxString::SvLBoxString()
    : mbEmphasized(false)
    , mbCustom(false)
    , meAlign(TxtAlign::Left)
{
}

SvLBoxString::~SvLBoxString()
{
}

SvLBoxItemType SvLBoxString::GetType() const
{
    return SvLBoxItemType::String;
}

namespace
{
    void drawSeparator(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRegion)
    {
        auto popIt = rRenderContext.ScopedPush(vcl::PushFlags::LINECOLOR);
        const StyleSettings& rStyle = rRenderContext.GetSettings().GetStyleSettings();
        Point aTmpPos = rRegion.TopLeft();
        Size aSize = rRegion.GetSize();
        aTmpPos.AdjustY(aSize.Height() / 2 );
        rRenderContext.SetLineColor(rStyle.GetShadowColor());
        rRenderContext.DrawLine(aTmpPos, Point(aSize.Width() + aTmpPos.X(), aTmpPos.Y()));
    }
}

void SvLBoxString::Paint(
    const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    DrawTextFlags nStyle = (rDev.IsEnabled() && !mbDisabled) ? DrawTextFlags::NONE : DrawTextFlags::Disable;
    if (rEntry.IsSeparator())
    {
        Point aStartPos(0, rPos.Y() - 2);
        tools::Rectangle aRegion(aStartPos, Size(rDev.GetSizePixel().Width(), 4));
        drawSeparator(rRenderContext, aRegion);
        return;
    }

    Size aSize;
    if (rDev.TextCenterAndClipEnabled())
    {
        nStyle |= DrawTextFlags::PathEllipsis | DrawTextFlags::Center;
        aSize.setWidth( rDev.GetEntryWidth() );
    }
    else
    {
        switch (meAlign)
        {
            case TxtAlign::Left:
            {
                nStyle |= DrawTextFlags::Left;
                aSize.setWidth(GetWidth(rDev, rEntry));
                break;
            }
            case TxtAlign::Center:
            {
                nStyle |= DrawTextFlags::Center;
                aSize.setWidth(rDev.GetBoundingRect(rEntry).getOpenWidth());
                break;
            }
            case TxtAlign::Right:
            {
                nStyle |= DrawTextFlags::Right;
                aSize.setWidth(rDev.GetBoundingRect(rEntry).getOpenWidth());
                break;
            }
        }
    }
    aSize.setHeight(GetHeight(rDev, rEntry));

    if (mbEmphasized)
    {
        rRenderContext.Push();
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        rRenderContext.SetFont(aFont);
    }

    tools::Rectangle aRect(rPos, aSize);

    if (mbCustom)
        rDev.DrawCustomEntry(rRenderContext, aRect, rEntry);
    else
        rRenderContext.DrawText(aRect, maText, nStyle);

    if (mbEmphasized)
        rRenderContext.Pop();
}

std::unique_ptr<SvLBoxItem> SvLBoxString::Clone(SvLBoxItem const * pSource) const
{
    std::unique_ptr<SvLBoxString> pNew(new SvLBoxString);

    const SvLBoxString* pOther = static_cast<const SvLBoxString*>(pSource);
    pNew->maText = pOther->maText;
    pNew->mbEmphasized = pOther->mbEmphasized;
    pNew->mbCustom = pOther->mbCustom;
    pNew->meAlign = pOther->meAlign;

    return std::unique_ptr<SvLBoxItem>(pNew.release());
}

void SvLBoxString::InitViewData(SvTreeListBox& rView, SvTreeListEntry& rEntry,
                                SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = &rView.GetViewDataItem(rEntry, *this);

    if (rEntry.IsSeparator())
    {
        pViewData->mnWidth = -1;
        pViewData->mnHeight = 0;
        return;
    }

    if (mbEmphasized)
    {
        rView.GetOutDev()->Push();
        vcl::Font aFont(rView.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        rView.Control::SetFont(aFont);
    }

    if (mbCustom)
    {
        Size aSize = rView.MeasureCustomEntry(*rView.GetOutDev(), rEntry);
        pViewData->mnWidth = aSize.Width();
        pViewData->mnHeight = aSize.Height();
    }
    else
    {
        pViewData->mnWidth = -1; // calc on demand
        pViewData->mnHeight = rView.GetTextHeight();
    }

    if (mbEmphasized)
        rView.GetOutDev()->Pop();
}

int SvLBoxString::CalcWidth(const SvTreeListBox& rView) const
{
    return rView.GetTextWidth(maText);
}

// ***************************************************************
// class SvLBoxContextBmp
// ***************************************************************

SvLBoxContextBmp::SvLBoxContextBmp(const Image& aBmp1, const Image& aBmp2, bool bExpanded)
    : m_aImage1(aBmp1)
    , m_aImage2(aBmp2)
    , m_bExpanded(bExpanded)
{
}

SvLBoxContextBmp::~SvLBoxContextBmp()
{
}

SvLBoxItemType SvLBoxContextBmp::GetType() const
{
    return SvLBoxItemType::ContextBmp;
}

void SvLBoxContextBmp::InitViewData(SvTreeListBox& rView, SvTreeListEntry& rEntry,
                                    SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = &rView.GetViewDataItem(rEntry, *this);
    Size aSize = m_aImage1.GetSizePixel();
    pViewData->mnWidth = aSize.Width();
    pViewData->mnHeight = aSize.Height();
}

void SvLBoxContextBmp::Paint(
    const Point& _rPos, SvTreeListBox& _rDev, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{

    // get the image.
    const Image& rImage = pView->IsExpanded() != m_bExpanded ? m_aImage1 : m_aImage2;

    bool _bSemiTransparent = bool( SvTLEntryFlags::SEMITRANSPARENT & rEntry.GetFlags( ) );
    // draw
    DrawImageFlags nStyle = (_rDev.IsEnabled() && !mbDisabled) ? DrawImageFlags::NONE : DrawImageFlags::Disable;
    if (_bSemiTransparent)
        nStyle |= DrawImageFlags::SemiTransparent;
    rRenderContext.DrawImage(_rPos, rImage, nStyle);
}

std::unique_ptr<SvLBoxItem> SvLBoxContextBmp::Clone(SvLBoxItem const * pSource) const
{
    const SvLBoxContextBmp* pContextBmp = static_cast<const SvLBoxContextBmp*>(pSource);
    return std::make_unique<SvLBoxContextBmp>(pContextBmp->m_aImage1, pContextBmp->m_aImage2,
                                              pContextBmp->m_bExpanded);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
