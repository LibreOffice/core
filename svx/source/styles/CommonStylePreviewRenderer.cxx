/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/CommonStylePreviewRenderer.hxx>

#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <svl/itemset.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xcolit.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/lineitem.hxx>
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
        pFont->SetRelief(static_cast<FontRelief>(static_cast<const SvxCharReliefItem*>(pItem)->GetValue()));
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_UNDERLINE)) != nullptr)
    {
        pFont->SetUnderline(static_cast< const SvxUnderlineItem*>(pItem)->GetLineStyle());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_OVERLINE)) != nullptr)
    {
        pFont->SetOverline(static_cast<FontUnderline>(static_cast<const SvxOverlineItem*>(pItem)->GetValue()));
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
        maFontColor = Color(static_cast<const SvxColorItem*>(pItem)->GetValue());
    }

    if (mpStyle->GetFamily() == SFX_STYLE_FAMILY_PARA)
    {
        if ((pItem = pItemSet->GetItem(XATTR_FILLSTYLE)) != nullptr)
        {
            sal_uInt16 aFillStyle = static_cast<const XFillStyleItem*>(pItem)->GetValue();
            if (aFillStyle == drawing::FillStyle_SOLID)
            {
                if ((pItem = pItemSet->GetItem(XATTR_FILLCOLOR)) != nullptr)
                {
                    maBackgroundColor = Color(static_cast<const XFillColorItem*>(pItem)->GetColorValue());
                }
            }
        }
    }

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_FONT)) != nullptr)
    {
        const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(pItem);
        pFont->SetName(pFontItem->GetFamilyName());
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
        maPixelSize = Size(mrOutputDev.LogicToPixel(aFontSize, mrShell.GetMapUnit()));
        pFont->SetSize(maPixelSize);

        vcl::Font aOldFont(mrOutputDev.GetFont());

        mrOutputDev.SetFont(*pFont);
        Rectangle aTextRect;
        mrOutputDev.GetTextBoundRect(aTextRect, mpStyle->GetName());
        if (aTextRect.Bottom() > mnMaxHeight)
        {
            double ratio = double(mnMaxHeight) / aTextRect.Bottom();
            maPixelSize.Width() *= ratio;
            maPixelSize.Height() *= ratio;
            pFont->SetSize(maPixelSize);
        }
        mrOutputDev.SetFont(aOldFont);
    }
    else
    {
        return false;
    }

    m_pFont = std::move(pFont);
    return true;
}

Size CommonStylePreviewRenderer::getRenderSize()
{
    assert(m_pFont);
    maPixelSize = m_pFont->GetTextSize(&mrOutputDev, maStyleName);
    if (maPixelSize.Height() > mnMaxHeight)
        maPixelSize.Height() = mnMaxHeight;
    return maPixelSize;
}

bool CommonStylePreviewRenderer::render(const Rectangle& aRectangle)
{
    // setup the device & draw
    vcl::Font aOldFont(mrOutputDev.GetFont());
    Color aOldColor(mrOutputDev.GetTextColor());
    Color aOldFillColor(mrOutputDev.GetFillColor());

    if (maBackgroundColor != COL_AUTO)
    {
        mrOutputDev.SetFillColor(maBackgroundColor);
        mrOutputDev.DrawRect(aRectangle);
    }

    if (m_pFont)
    {
        mrOutputDev.SetFont(*m_pFont);
    }
    if (maFontColor != COL_AUTO)
        mrOutputDev.SetTextColor(maFontColor);

    Size aPixelSize((m_pFont) ? maPixelSize : mrOutputDev.GetFont().GetSize());

    Point aFontDrawPosition = aRectangle.TopLeft();
    if (aRectangle.GetHeight() > aPixelSize.Height())
        aFontDrawPosition.Y() += (aRectangle.GetHeight() - aPixelSize.Height()) / 2;

    mrOutputDev.DrawText(aFontDrawPosition, maStyleName);

    mrOutputDev.SetFillColor(aOldFillColor);
    mrOutputDev.SetTextColor(aOldColor);
    mrOutputDev.SetFont(aOldFont);

    return true;
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
