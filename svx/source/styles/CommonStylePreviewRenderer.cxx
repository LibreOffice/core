/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>
#include <CommonStylePreviewRenderer.hxx>

#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <svl/itemset.hxx>
#include <vcl/outdev.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/cmapitem.hxx>

#include <editeng/editids.hrc>

using namespace css;

namespace svx
{

CommonStylePreviewRenderer::CommonStylePreviewRenderer(
                                const SfxObjectShell& rShell, OutputDevice& rOutputDev,
                                SfxStyleSheetBase* pStyle, long nMaxHeight)
    : StylePreviewRenderer(rShell, rOutputDev, pStyle, nMaxHeight)
    , m_pFont()
    , maFontColor(COL_AUTO)
    , maHighlightColor(COL_AUTO)
    , maBackgroundColor(COL_AUTO)
    , maPixelSize()
    , maStyleName(mpStyle->GetName())
{
}

CommonStylePreviewRenderer::~CommonStylePreviewRenderer()
{}

bool CommonStylePreviewRenderer::recalculate()
{
    m_pFont.reset();

    std::unique_ptr<SfxItemSet> pItemSet(mpStyle->GetItemSetForPreview());

    if (!pItemSet) return false;

    std::unique_ptr<SvxFont> pFont(new SvxFont);

    const SfxPoolItem* pItem;

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_WEIGHT)) != nullptr)
    {
        pFont->SetWeight(static_cast<const SvxWeightItem*>(pItem)->GetWeight());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_POSTURE)) != nullptr)
    {
        pFont->SetItalic(static_cast<const SvxPostureItem*>(pItem)->GetPosture());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CONTOUR)) != nullptr)
    {
        pFont->SetOutline(static_cast< const SvxContourItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_SHADOWED)) != nullptr)
    {
        pFont->SetShadow(static_cast<const SvxShadowedItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_RELIEF)) != nullptr)
    {
        pFont->SetRelief(static_cast<const SvxCharReliefItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_UNDERLINE)) != nullptr)
    {
        pFont->SetUnderline(static_cast< const SvxUnderlineItem*>(pItem)->GetLineStyle());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_OVERLINE)) != nullptr)
    {
        pFont->SetOverline(static_cast<const SvxOverlineItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_STRIKEOUT)) != nullptr)
    {
        pFont->SetStrikeout(static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CASEMAP)) != nullptr)
    {
        pFont->SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_EMPHASISMARK)) != nullptr)
    {
        pFont->SetEmphasisMark(static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_COLOR)) != nullptr)
    {
        maFontColor = static_cast<const SvxColorItem*>(pItem)->GetValue();
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_BRUSH_CHAR)) != nullptr)
    {
        maHighlightColor = static_cast<const SvxBrushItem*>(pItem)->GetColor();
    }

    if (mpStyle->GetFamily() == SfxStyleFamily::Para)
    {
        if ((pItem = pItemSet->GetItem(XATTR_FILLSTYLE)) != nullptr)
        {
            css::drawing::FillStyle aFillStyle = static_cast<const XFillStyleItem*>(pItem)->GetValue();
            if (aFillStyle == drawing::FillStyle_SOLID)
            {
                if ((pItem = pItemSet->GetItem(XATTR_FILLCOLOR)) != nullptr)
                {
                    maBackgroundColor = static_cast<const XFillColorItem*>(pItem)->GetColorValue();
                }
            }
        }
    }

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_FONT)) != nullptr)
    {
        const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(pItem);
        if (IsStarSymbol(pFontItem->GetFamilyName()))
            return false;
        pFont->SetFamilyName(pFontItem->GetFamilyName());
        pFont->SetStyleName(pFontItem->GetStyleName());
    }
    else
    {
        return false;
    }

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_FONTHEIGHT)) != nullptr)
    {
        const SvxFontHeightItem* pFontHeightItem = static_cast<const SvxFontHeightItem*>(pItem);
        Size aFontSize(0, pFontHeightItem->GetHeight());
        maPixelSize = mrOutputDev.LogicToPixel(aFontSize, MapMode(mrShell.GetMapUnit()));
        pFont->SetFontSize(maPixelSize);

        vcl::Font aOldFont(mrOutputDev.GetFont());

        mrOutputDev.SetFont(*pFont);
        tools::Rectangle aTextRect;
        mrOutputDev.GetTextBoundRect(aTextRect, mpStyle->GetName());
        if (aTextRect.Bottom() > mnMaxHeight)
        {
            double ratio = double(mnMaxHeight) / aTextRect.Bottom();
            maPixelSize.setWidth( maPixelSize.Width() * ratio );
            maPixelSize.setHeight( maPixelSize.Height() * ratio );
            pFont->SetFontSize(maPixelSize);
        }
        mrOutputDev.SetFont(aOldFont);
    }
    else
    {
        return false;
    }

    m_pFont = std::move(pFont);
    maPixelSize = getRenderSize();
    return true;
}

Size CommonStylePreviewRenderer::getRenderSize() const
{
    assert(m_pFont);
    Size aPixelSize = m_pFont->GetTextSize(&mrOutputDev, maStyleName);

    if (aPixelSize.Height() > mnMaxHeight)
        aPixelSize.setHeight( mnMaxHeight );

    return aPixelSize;
}

bool CommonStylePreviewRenderer::render(const tools::Rectangle& aRectangle, RenderAlign eRenderAlign)
{
    const OUString& rText = maStyleName;

    // setup the device & draw
    mrOutputDev.Push(PushFlags::FONT | PushFlags::TEXTCOLOR | PushFlags::FILLCOLOR | PushFlags::TEXTFILLCOLOR);

    if (maBackgroundColor != COL_AUTO)
    {
        mrOutputDev.SetFillColor(maBackgroundColor);
        mrOutputDev.DrawRect(aRectangle);
    }

    if (m_pFont)
        mrOutputDev.SetFont(*m_pFont);

    if (maFontColor != COL_AUTO)
        mrOutputDev.SetTextColor(maFontColor);

    if (maHighlightColor != COL_AUTO)
        mrOutputDev.SetTextFillColor(maHighlightColor);

    Size aPixelSize(m_pFont ? maPixelSize : mrOutputDev.GetFont().GetFontSize());

    Point aFontDrawPosition = aRectangle.TopLeft();
    if (eRenderAlign == RenderAlign::CENTER)
    {
        if (aRectangle.GetHeight() > aPixelSize.Height())
            aFontDrawPosition.AdjustY((aRectangle.GetHeight() - aPixelSize.Height()) / 2 );
    }

    mrOutputDev.DrawText(aFontDrawPosition, rText);

    mrOutputDev.Pop();

    return true;
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
