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
#include <svl/itempool.hxx>
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

#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

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

static bool GetWhich(const SfxItemSet& rSet, sal_uInt16 nSlot, sal_uInt16& rWhich)
{
    rWhich = rSet.GetPool()->GetWhich(nSlot);
    return rSet.GetItemState(rWhich) >= SfxItemState::DEFAULT;
}

bool CommonStylePreviewRenderer::SetFont(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont)
{
    sal_uInt16 nWhich;
    if (GetWhich(rSet, nSlot, nWhich))
    {
        const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(rSet.Get(nWhich));
        rFont.SetFamily(rFontItem.GetFamily());
        rFont.SetFamilyName(rFontItem.GetFamilyName());
        rFont.SetPitch(rFontItem.GetPitch());
        rFont.SetCharSet(rFontItem.GetCharSet());
        rFont.SetStyleName(rFontItem.GetStyleName());
        return true;
    }
    return false;
}

bool CommonStylePreviewRenderer::SetFontSize(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont)
{
    sal_uInt16 nWhich;
    if (GetWhich(rSet, nSlot, nWhich))
    {
        const SvxFontHeightItem& rFontHeightItem = static_cast<const SvxFontHeightItem&>(rSet.Get(nWhich));
        Size aFontSize(0, rFontHeightItem.GetHeight());
        maPixelSize = mrOutputDev.LogicToPixel(aFontSize, MapMode(mrShell.GetMapUnit()));
        rFont.SetFontSize(maPixelSize);

        vcl::Font aOldFont(mrOutputDev.GetFont());

        mrOutputDev.SetFont(rFont);
        tools::Rectangle aTextRect;
        mrOutputDev.GetTextBoundRect(aTextRect, mpStyle->GetName());
        if (aTextRect.Bottom() > mnMaxHeight)
        {
            double ratio = double(mnMaxHeight) / aTextRect.Bottom();
            maPixelSize.setWidth( maPixelSize.Width() * ratio );
            maPixelSize.setHeight( maPixelSize.Height() * ratio );
            rFont.SetFontSize(maPixelSize);
        }
        mrOutputDev.SetFont(aOldFont);
        return true;
    }
    return false;
}

bool CommonStylePreviewRenderer::recalculate()
{
    m_oFont.reset();

    std::optional<SfxItemSet> pItemSet(mpStyle->GetItemSetForPreview());

    if (!pItemSet) return false;

    SvxFont aFont;
    SvxFont aCJKFont;
    SvxFont aCTLFont;

    const SfxPoolItem* pItem;

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_WEIGHT)) != nullptr)
        aFont.SetWeight(static_cast<const SvxWeightItem*>(pItem)->GetWeight());
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CJK_WEIGHT)) != nullptr)
        aCJKFont.SetWeight(static_cast<const SvxWeightItem*>(pItem)->GetWeight());
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CTL_WEIGHT)) != nullptr)
        aCTLFont.SetWeight(static_cast<const SvxWeightItem*>(pItem)->GetWeight());

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_POSTURE)) != nullptr)
        aFont.SetItalic(static_cast<const SvxPostureItem*>(pItem)->GetPosture());
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CJK_POSTURE)) != nullptr)
        aCJKFont.SetItalic(static_cast<const SvxPostureItem*>(pItem)->GetPosture());
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CTL_POSTURE)) != nullptr)
        aCTLFont.SetItalic(static_cast<const SvxPostureItem*>(pItem)->GetPosture());

    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CONTOUR)) != nullptr)
    {
        aFont.SetOutline(static_cast< const SvxContourItem*>(pItem)->GetValue());
        aCJKFont.SetOutline(static_cast< const SvxContourItem*>(pItem)->GetValue());
        aCTLFont.SetOutline(static_cast< const SvxContourItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_SHADOWED)) != nullptr)
    {
        aFont.SetShadow(static_cast<const SvxShadowedItem*>(pItem)->GetValue());
        aCJKFont.SetShadow(static_cast<const SvxShadowedItem*>(pItem)->GetValue());
        aCTLFont.SetShadow(static_cast<const SvxShadowedItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_RELIEF)) != nullptr)
    {
        aFont.SetRelief(static_cast<const SvxCharReliefItem*>(pItem)->GetValue());
        aCJKFont.SetRelief(static_cast<const SvxCharReliefItem*>(pItem)->GetValue());
        aCTLFont.SetRelief(static_cast<const SvxCharReliefItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_UNDERLINE)) != nullptr)
    {
        aFont.SetUnderline(static_cast< const SvxUnderlineItem*>(pItem)->GetLineStyle());
        aCJKFont.SetUnderline(static_cast< const SvxUnderlineItem*>(pItem)->GetLineStyle());
        aCTLFont.SetUnderline(static_cast< const SvxUnderlineItem*>(pItem)->GetLineStyle());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_OVERLINE)) != nullptr)
    {
        aFont.SetOverline(static_cast<const SvxOverlineItem*>(pItem)->GetValue());
        aCJKFont.SetOverline(static_cast<const SvxOverlineItem*>(pItem)->GetValue());
        aCTLFont.SetOverline(static_cast<const SvxOverlineItem*>(pItem)->GetValue());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_STRIKEOUT)) != nullptr)
    {
        aFont.SetStrikeout(static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout());
        aCJKFont.SetStrikeout(static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout());
        aCTLFont.SetStrikeout(static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CASEMAP)) != nullptr)
    {
        aFont.SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());
        aCJKFont.SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());
        aCTLFont.SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_EMPHASISMARK)) != nullptr)
    {
        aFont.SetEmphasisMark(static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark());
        aCJKFont.SetEmphasisMark(static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark());
        aCTLFont.SetEmphasisMark(static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark());
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

    if (SetFont(*pItemSet, SID_ATTR_CHAR_FONT, aFont) &&
        SetFontSize(*pItemSet, SID_ATTR_CHAR_FONTHEIGHT, aFont))
        m_oFont = aFont;

    if (SetFont(*pItemSet, SID_ATTR_CHAR_CJK_FONT, aCJKFont) &&
        SetFontSize(*pItemSet, SID_ATTR_CHAR_CJK_FONTHEIGHT, aCJKFont))
        m_oCJKFont = aCJKFont;

    if (SetFont(*pItemSet, SID_ATTR_CHAR_CTL_FONT, aCTLFont) &&
        SetFontSize(*pItemSet, SID_ATTR_CHAR_CTL_FONTHEIGHT, aCTLFont))
        m_oCTLFont = aCTLFont;

    CheckScript();
    maPixelSize = getRenderSize();
    return true;
}

Size CommonStylePreviewRenderer::getRenderSize()
{
    const OUString& rText = maStyleName;

    tools::Long nTextWidth = 0;
    tools::Long nHeight = 0;

    sal_uInt16 nScript;
    sal_uInt16 nIdx = 0;
    sal_Int32 nStart = 0;
    sal_Int32 nEnd;
    size_t nCnt = maScriptChanges.size();

    if (nCnt)
    {
        nEnd = maScriptChanges[nIdx].changePos;
        nScript = maScriptChanges[nIdx].scriptType;
    }
    else
    {
        nEnd = rText.getLength();
        nScript = css::i18n::ScriptType::LATIN;
    }

    do
    {
        auto oFont = (nScript == css::i18n::ScriptType::ASIAN) ?
                         m_oCJKFont :
                         ((nScript == css::i18n::ScriptType::COMPLEX) ?
                             m_oCTLFont :
                             m_oFont);

        Size aSize;
        if (oFont)
            aSize = oFont->GetTextSize(mrOutputDev, rText, nStart, nEnd - nStart);
        else
            aSize = Size(mrOutputDev.GetTextWidth(rText, nStart, nEnd - nStart), mrOutputDev.GetFont().GetFontHeight());

        auto nWidth = aSize.Width();
        nHeight = std::max(nHeight, aSize.Height());
        if (nIdx >= maScriptChanges.size())
            break;

        maScriptChanges[nIdx++].textWidth = nWidth;
        nTextWidth += nWidth;

        if (nEnd < rText.getLength() && nIdx < nCnt)
        {
            nStart = nEnd;
            nEnd = maScriptChanges[nIdx].changePos;
            nScript = maScriptChanges[nIdx].scriptType;
        }
        else
            break;
    }
    while(true);

    Size aPixelSize(nTextWidth, std::min(nHeight, mnMaxHeight));
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

    if (maFontColor != COL_AUTO)
        mrOutputDev.SetTextColor(maFontColor);

    if (maHighlightColor != COL_AUTO)
        mrOutputDev.SetTextFillColor(maHighlightColor);

    Point aFontDrawPosition = aRectangle.TopLeft();
    if (eRenderAlign == RenderAlign::CENTER)
    {
        if (aRectangle.GetHeight() > maPixelSize.Height())
            aFontDrawPosition.AdjustY((aRectangle.GetHeight() - maPixelSize.Height()) / 2 );
    }

    sal_uInt16 nScript;
    sal_uInt16 nIdx = 0;
    sal_Int32 nStart = 0;
    sal_Int32 nEnd;
    size_t nCnt = maScriptChanges.size();
    if (nCnt)
    {
        nEnd = maScriptChanges[nIdx].changePos;
        nScript = maScriptChanges[nIdx].scriptType;
    }
    else
    {
        nEnd = rText.getLength();
        nScript = css::i18n::ScriptType::LATIN;
    }

    do
    {
        auto oFont = (nScript == css::i18n::ScriptType::ASIAN)
                         ? m_oCJKFont
                         : ((nScript == css::i18n::ScriptType::COMPLEX)
                             ? m_oCTLFont
                             : m_oFont);
        if (oFont)
        {
            mrOutputDev.SetFont(*oFont);
            oFont->QuickDrawText(&mrOutputDev, aFontDrawPosition, rText, nStart, nEnd - nStart, {});
        }
        else
            mrOutputDev.DrawText(aFontDrawPosition, rText, nStart, nEnd - nStart);

        aFontDrawPosition.AdjustX(maScriptChanges[nIdx++].textWidth);
        if (nEnd < rText.getLength() && nIdx < nCnt)
        {
            nStart = nEnd;
            nEnd = maScriptChanges[nIdx].changePos;
            nScript = maScriptChanges[nIdx].scriptType;
        }
        else
            break;
    }
    while(true);

    mrOutputDev.Pop();

    return true;
}

void CommonStylePreviewRenderer::CheckScript()
{
    assert(!maStyleName.isEmpty()); // must have a preview text here!
    if (maStyleName == maScriptText)
        return; // already initialized

    maScriptText = maStyleName;
    maScriptChanges.clear();

    if (!mxBreak.is())
    {
        auto xContext = comphelper::getProcessComponentContext();
        mxBreak = css::i18n::BreakIterator::create(xContext);
    }

    sal_Int16 nScript = mxBreak->getScriptType(maScriptText, 0);
    sal_Int32 nChg = 0;
    if (css::i18n::ScriptType::WEAK == nScript)
    {
        nChg = mxBreak->endOfScript(maStyleName, nChg, nScript);
        if(nChg < maStyleName.getLength())
            nScript = mxBreak->getScriptType(maStyleName, nChg);
        else
            nScript = css::i18n::ScriptType::LATIN;
    }

    while (true)
    {
        nChg = mxBreak->endOfScript(maStyleName, nChg, nScript);
        maScriptChanges.emplace_back(nScript, nChg);
        if (nChg >= maStyleName.getLength() || nChg < 0)
            break;
        nScript = mxBreak->getScriptType(maStyleName, nChg);
    }
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
