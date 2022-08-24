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
    , mnHeight(0)
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

static bool SetFont(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont)
{
    sal_uInt16 nWhich;
    if (GetWhich(rSet, nSlot, nWhich))
    {
        const auto& rFontItem = static_cast<const SvxFontItem&>(rSet.Get(nWhich));
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
        const auto& rFontHeightItem = static_cast<const SvxFontHeightItem&>(rSet.Get(nWhich));
        Size aFontSize(0, rFontHeightItem.GetHeight());
        aFontSize = mrOutputDev.LogicToPixel(aFontSize, MapMode(mrShell.GetMapUnit()));
        rFont.SetFontSize(aFontSize);
        return true;
    }
    return false;
}

bool CommonStylePreviewRenderer::recalculate()
{
    m_oFont.reset();
    m_oCJKFont.reset();
    m_oCTLFont.reset();

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
        auto aVal = static_cast<const SvxContourItem*>(pItem)->GetValue();
        aFont.SetOutline(aVal);
        aCJKFont.SetOutline(aVal);
        aCTLFont.SetOutline(aVal);
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_SHADOWED)) != nullptr)
    {
        auto aVal = static_cast<const SvxShadowedItem*>(pItem)->GetValue();
        aFont.SetShadow(aVal);
        aCJKFont.SetShadow(aVal);
        aCTLFont.SetShadow(aVal);
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_RELIEF)) != nullptr)
    {
        auto aVal = static_cast<const SvxCharReliefItem*>(pItem)->GetValue();
        aFont.SetRelief(aVal);
        aCJKFont.SetRelief(aVal);
        aCTLFont.SetRelief(aVal);
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_UNDERLINE)) != nullptr)
    {
        auto aVal = static_cast<const SvxUnderlineItem*>(pItem)->GetLineStyle();
        aFont.SetUnderline(aVal);
        aCJKFont.SetUnderline(aVal);
        aCTLFont.SetUnderline(aVal);
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_OVERLINE)) != nullptr)
    {
        auto aVal = static_cast<const SvxOverlineItem*>(pItem)->GetValue();
        aFont.SetOverline(aVal);
        aCJKFont.SetOverline(aVal);
        aCTLFont.SetOverline(aVal);
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_STRIKEOUT)) != nullptr)
    {
        auto aVal = static_cast<const SvxCrossedOutItem*>(pItem)->GetStrikeout();
        aFont.SetStrikeout(aVal);
        aCJKFont.SetStrikeout(aVal);
        aCTLFont.SetStrikeout(aVal);
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_CASEMAP)) != nullptr)
    {
        auto aVal = static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap();
        aFont.SetCaseMap(aVal);
        aCJKFont.SetCaseMap(aVal);
        aCTLFont.SetCaseMap(aVal);
    }
    if ((pItem = pItemSet->GetItem(SID_ATTR_CHAR_EMPHASISMARK)) != nullptr)
    {
        auto aVal = static_cast<const SvxEmphasisMarkItem*>(pItem)->GetEmphasisMark();
        aFont.SetEmphasisMark(aVal);
        aCJKFont.SetEmphasisMark(aVal);
        aCTLFont.SetEmphasisMark(aVal);
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
    CalcRenderSize();
    return true;
}

void CommonStylePreviewRenderer::CalcRenderSize()
{
    const OUString& rText = maStyleName;

    tools::Rectangle aTextRect;
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

        mrOutputDev.Push(vcl::PushFlags::FONT);

        Size aSize;
        if (oFont)
        {
            mrOutputDev.SetFont(*oFont);
            aSize = oFont->GetTextSize(mrOutputDev, rText, nStart, nEnd - nStart);
        }
        else
            aSize = Size(mrOutputDev.GetTextWidth(rText, nStart, nEnd - nStart), mrOutputDev.GetFont().GetFontHeight());

        tools::Rectangle aRect;
        mrOutputDev.GetTextBoundRect(aRect, rText, nStart, nStart, nEnd - nStart);
        aTextRect = aTextRect.Union(aRect);

        mrOutputDev.Pop();

        auto nWidth = aSize.Width();
        nHeight = std::max(nHeight, aSize.Height());
        if (nIdx >= maScriptChanges.size())
            break;

        maScriptChanges[nIdx++].textWidth = nWidth;

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

    double fRatio = 1;
    if (aTextRect.Bottom() > mnMaxHeight)
        fRatio = double(mnMaxHeight) / aTextRect.Bottom();

    mnHeight = std::min(tools::Long(nHeight * fRatio), mnMaxHeight);
    if (fRatio != 1)
    {
        Size aFontSize;
        if (m_oFont)
        {
            aFontSize = m_oFont->GetFontSize();
            m_oFont->SetFontSize(Size(aFontSize.Height() * fRatio, aFontSize.Width() * fRatio));
        }
        if (m_oCJKFont)
        {
            aFontSize = m_oCJKFont->GetFontSize();
            m_oCJKFont->SetFontSize(Size(aFontSize.Height() * fRatio, aFontSize.Width() * fRatio));
        }
        if (m_oCTLFont)
        {
            aFontSize = m_oCTLFont->GetFontSize();
            m_oCTLFont->SetFontSize(Size(aFontSize.Height() * fRatio, aFontSize.Width() * fRatio));
        }

        for (auto& aChange : maScriptChanges)
            aChange.textWidth *= fRatio;
    }
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
        if (aRectangle.GetHeight() > mnHeight)
            aFontDrawPosition.AdjustY((aRectangle.GetHeight() - mnHeight) / 2 );
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

        mrOutputDev.Push(vcl::PushFlags::FONT);

        if (oFont)
        {
            mrOutputDev.SetFont(*oFont);
            oFont->QuickDrawText(&mrOutputDev, aFontDrawPosition, rText, nStart, nEnd - nStart, {});
        }
        else
            mrOutputDev.DrawText(aFontDrawPosition, rText, nStart, nEnd - nStart);

        mrOutputDev.Pop();

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

    sal_Int16 nScript = mxBreak->getScriptType(maStyleName, 0);
    sal_Int32 nChg = 0;
    if (css::i18n::ScriptType::WEAK == nScript)
    {
        nChg = mxBreak->endOfScript(maStyleName, nChg, nScript);
        if (nChg < maStyleName.getLength())
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
