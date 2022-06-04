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
                                SfxStyleSheetBase* pStyle, tools::Long nMaxHeight)
    : StylePreviewRenderer(rShell, rOutputDev, pStyle, nMaxHeight)
    , maFontColor(COL_AUTO)
    , maHighlightColor(COL_AUTO)
    , maBackgroundColor(COL_AUTO)
    , maStyleName(mpStyle->GetName())
{
}

CommonStylePreviewRenderer::~CommonStylePreviewRenderer()
{}

bool CommonStylePreviewRenderer::recalculate()
{
    m_oFont.reset();

    std::optional<SfxItemSet> pItemSet(mpStyle->GetItemSetForPreview());

    if (!pItemSet) return false;

    SvxFont aFont;

    const SfxPoolItem* pItem;

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_WEIGHT)) != nullptr)
    {
        aFont.SetWeight(static_cast<const SvxWeightItem*>(pItem)->GetWeight());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_POSTURE)) != nullptr)
    {
        aFont.SetItalic(static_cast<const SvxPostureItem*>(pItem)->GetPosture());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CONTOUR)) != nullptr)
    {
        aFont.SetOutline(static_cast< const SvxContourItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_SHADOWED)) != nullptr)
    {
        aFont.SetShadow(static_cast<const SvxShadowedItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_RELIEF)) != nullptr)
    {
        aFont.SetRelief(static_cast<const SvxCharReliefItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_UNDERLINE)) != nullptr)
    {
        aFont.SetUnderline(static_cast< const SvxUnderlineItem*>(pItem)->GetLineStyle());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_OVERLINE)) != nullptr)
    {
        aFont.SetOverline(static_cast<const SvxOverlineItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_STRIKEOUT)) != nullptr)
    {
        aFont.SetStrikeout(static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CASEMAP)) != nullptr)
    {
        aFont.SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_EMPHASISMARK)) != nullptr)
    {
        aFont.SetEmphasisMark(static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark());
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
        aFont.SetFamilyName(pFontItem->GetFamilyName());
        aFont.SetStyleName(pFontItem->GetStyleName());
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
        aFont.SetFontSize(maPixelSize);

        vcl::Font aOldFont(mrOutputDev.GetFont());

        mrOutputDev.SetFont(aFont);
        tools::Rectangle aTextRect;
        mrOutputDev.GetTextBoundRect(aTextRect, mpStyle->GetName());
        if (aTextRect.Bottom() > mnMaxHeight)
        {
            double ratio = double(mnMaxHeight) / aTextRect.Bottom();
            maPixelSize.setWidth( maPixelSize.Width() * ratio );
            maPixelSize.setHeight( maPixelSize.Height() * ratio );
            aFont.SetFontSize(maPixelSize);
        }
        mrOutputDev.SetFont(aOldFont);
    }
    else
    {
        return false;
    }

    m_oFont = aFont;
    maPixelSize = getRenderSize();
    return true;
}

Size CommonStylePreviewRenderer::getRenderSize() const
{
    assert(m_oFont);
    Size aPixelSize = m_oFont->GetTextSize(mrOutputDev, maStyleName);

    if (aPixelSize.Height() > mnMaxHeight)
        aPixelSize.setHeight( mnMaxHeight );

    return aPixelSize;
}

bool CommonStylePreviewRenderer::render(const tools::Rectangle& aRectangle, RenderAlign eRenderAlign)
{
    const OUString& rText = maStyleName;

    // setup the device & draw
    mrOutputDev.Push(vcl::PushFlags::FONT | vcl::PushFlags::TEXTCOLOR | vcl::PushFlags::FILLCOLOR | vcl::PushFlags::TEXTFILLCOLOR);

    if (maBackgroundColor != COL_AUTO)
    {
        mrOutputDev.SetFillColor(maBackgroundColor);
        mrOutputDev.DrawRect(aRectangle);
    }

    if (m_oFont)
        mrOutputDev.SetFont(*m_oFont);

    if (maFontColor != COL_AUTO)
        mrOutputDev.SetTextColor(maFontColor);

    if (maHighlightColor != COL_AUTO)
        mrOutputDev.SetTextFillColor(maHighlightColor);

    Size aPixelSize(m_oFont ? maPixelSize : mrOutputDev.GetFont().GetFontSize());

    Point aFontDrawPosition = aRectangle.TopLeft();
    if (eRenderAlign == RenderAlign::CENTER)
    {
        if (aRectangle.GetHeight() > aPixelSize.Height())
            aFontDrawPosition.AdjustY((aRectangle.GetHeight() - aPixelSize.Height()) / 2 );
    }

    if (m_oFont)
        m_oFont->QuickDrawText( &mrOutputDev, aFontDrawPosition, rText, 0, rText.getLength(), {} );
    else
        mrOutputDev.DrawText(aFontDrawPosition, rText);

    mrOutputDev.Pop();

    return true;
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
