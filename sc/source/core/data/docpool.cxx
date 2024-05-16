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

#include <sal/config.h>

#include <scitems.hxx>

#include <comphelper/string.hxx>
#include <i18nutil/unicode.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svl/itemiter.hxx>
#include <svl/stritem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/postitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/justifyitem.hxx>

#include <docpool.hxx>
#include <global.hxx>
#include <attrib.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <sc.hrc>

// ATTR_FONT_TWOLINES (not used) was changed to ATTR_USERDEF (not saved in binary format) in 641c

namespace {

SvxFontItem* getDefaultFontItem(LanguageType eLang, DefaultFontType nFontType, sal_uInt16 nItemId)
{
    vcl::Font aDefFont = OutputDevice::GetDefaultFont( nFontType, eLang, GetDefaultFontFlags::OnlyOne );
    SvxFontItem* pNewItem = new SvxFontItem( aDefFont.GetFamilyType(), aDefFont.GetFamilyName(), aDefFont.GetStyleName(),
            aDefFont.GetPitch(), aDefFont.GetCharSet(), nItemId );

    return pNewItem;
}

}

static ItemInfoPackage& getItemInfoPackageScDocument()
{
    class ItemInfoPackageScDocument : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, ATTR_ENDINDEX - ATTR_STARTINDEX + 1> ItemInfoArrayScDocument;
        ItemInfoArrayScDocument maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { ATTR_FONT, nullptr, SID_ATTR_CHAR_FONT, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_FONT_HEIGHT, new SvxFontHeightItem( 200, 100, ATTR_FONT_HEIGHT ), SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_WEIGHT, new SvxWeightItem( WEIGHT_NORMAL, ATTR_FONT_WEIGHT ), SID_ATTR_CHAR_WEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_POSTURE, new SvxPostureItem( ITALIC_NONE, ATTR_FONT_POSTURE ), SID_ATTR_CHAR_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_UNDERLINE, new SvxUnderlineItem( LINESTYLE_NONE, ATTR_FONT_UNDERLINE ), SID_ATTR_CHAR_UNDERLINE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_OVERLINE, new SvxOverlineItem( LINESTYLE_NONE, ATTR_FONT_OVERLINE ), SID_ATTR_CHAR_OVERLINE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_CROSSEDOUT, new SvxCrossedOutItem( STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT ), SID_ATTR_CHAR_STRIKEOUT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_CONTOUR, new SvxContourItem( false, ATTR_FONT_CONTOUR ), SID_ATTR_CHAR_CONTOUR, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_SHADOWED, new SvxShadowedItem( false, ATTR_FONT_SHADOWED ), SID_ATTR_CHAR_SHADOWED, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_COLOR, new SvxColorItem( COL_AUTO, ATTR_FONT_COLOR ), SID_ATTR_CHAR_COLOR, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_FONT_LANGUAGE, new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_FONT_LANGUAGE ), SID_ATTR_CHAR_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CJK_FONT, nullptr, SID_ATTR_CHAR_CJK_FONT, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_CJK_FONT_HEIGHT, new SvxFontHeightItem( 200, 100, ATTR_CJK_FONT_HEIGHT ), SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CJK_FONT_WEIGHT, new SvxWeightItem( WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT ), SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CJK_FONT_POSTURE, new SvxPostureItem( ITALIC_NONE, ATTR_CJK_FONT_POSTURE ), SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CJK_FONT_LANGUAGE, new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_CJK_FONT_LANGUAGE ), SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CTL_FONT, nullptr, SID_ATTR_CHAR_CTL_FONT, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_CTL_FONT_HEIGHT, new SvxFontHeightItem( 200, 100, ATTR_CTL_FONT_HEIGHT ), SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CTL_FONT_WEIGHT, new SvxWeightItem( WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT ), SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CTL_FONT_POSTURE, new SvxPostureItem( ITALIC_NONE, ATTR_CTL_FONT_POSTURE ), SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CTL_FONT_LANGUAGE, new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_CTL_FONT_LANGUAGE ), SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_EMPHASISMARK, new SvxEmphasisMarkItem( FontEmphasisMark::NONE, ATTR_FONT_EMPHASISMARK ), SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEMINFOFLAG_NONE },
            { ATTR_USERDEF, new SvXMLAttrContainerItem( ATTR_USERDEF ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_FONT_WORDLINE, new SvxWordLineModeItem(false, ATTR_FONT_WORDLINE ), SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FONT_RELIEF, new SvxCharReliefItem( FontRelief::NONE, ATTR_FONT_RELIEF ), SID_ATTR_CHAR_RELIEF, SFX_ITEMINFOFLAG_NONE },
            { ATTR_HYPHENATE, new ScHyphenateCell(), SID_ATTR_ALIGN_HYPHENATION, SFX_ITEMINFOFLAG_NONE },
            { ATTR_SCRIPTSPACE, new SvxScriptSpaceItem( false, ATTR_SCRIPTSPACE), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_HANGPUNCTUATION, new SvxHangingPunctuationItem( false, ATTR_HANGPUNCTUATION), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_FORBIDDEN_RULES, new SvxForbiddenRuleItem( false, ATTR_FORBIDDEN_RULES), SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEMINFOFLAG_NONE },
            { ATTR_HOR_JUSTIFY, new SvxHorJustifyItem( SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY), SID_ATTR_ALIGN_HOR_JUSTIFY, SFX_ITEMINFOFLAG_NONE },
            { ATTR_HOR_JUSTIFY_METHOD, new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_HOR_JUSTIFY_METHOD), SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD, SFX_ITEMINFOFLAG_NONE },
            { ATTR_INDENT, new ScIndentItem( 0 ), SID_ATTR_ALIGN_INDENT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_VER_JUSTIFY, new SvxVerJustifyItem( SvxCellVerJustify::Standard, ATTR_VER_JUSTIFY), SID_ATTR_ALIGN_VER_JUSTIFY, SFX_ITEMINFOFLAG_NONE },
            { ATTR_VER_JUSTIFY_METHOD, new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_VER_JUSTIFY_METHOD), SID_ATTR_ALIGN_VER_JUSTIFY_METHOD, SFX_ITEMINFOFLAG_NONE },
            { ATTR_STACKED, new ScVerticalStackCell(false), SID_ATTR_ALIGN_STACKED, SFX_ITEMINFOFLAG_NONE },
            { ATTR_ROTATE_VALUE, new ScRotateValueItem( 0_deg100 ), SID_ATTR_ALIGN_DEGREES, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_ROTATE_MODE, new SvxRotateModeItem( SVX_ROTATE_MODE_BOTTOM, ATTR_ROTATE_MODE ), SID_ATTR_ALIGN_LOCKPOS, SFX_ITEMINFOFLAG_NONE },
            { ATTR_VERTICAL_ASIAN, new SfxBoolItem( ATTR_VERTICAL_ASIAN ), SID_ATTR_ALIGN_ASIANVERTICAL, SFX_ITEMINFOFLAG_NONE },

            //  The default for the ATTR_WRITINGDIR cell attribute must by SvxFrameDirection::Environment,
            //  so that value is returned when asking for a default cell's attributes.
            //  The value from the page style is set as DefaultHorizontalTextDirection for the EditEngine.
            { ATTR_WRITINGDIR, new SvxFrameDirectionItem( SvxFrameDirection::Environment, ATTR_WRITINGDIR ), SID_ATTR_FRAMEDIRECTION, SFX_ITEMINFOFLAG_NONE },

            { ATTR_LINEBREAK, new ScLineBreakCell(), SID_ATTR_ALIGN_LINEBREAK, SFX_ITEMINFOFLAG_NONE },
            { ATTR_SHRINKTOFIT, new ScShrinkToFitCell(), SID_ATTR_ALIGN_SHRINKTOFIT, SFX_ITEMINFOFLAG_NONE },
            { ATTR_BORDER_TLBR, new SvxLineItem( ATTR_BORDER_TLBR ), SID_ATTR_BORDER_DIAG_TLBR, SFX_ITEMINFOFLAG_NONE },
            { ATTR_BORDER_BLTR, new SvxLineItem( ATTR_BORDER_BLTR ), SID_ATTR_BORDER_DIAG_BLTR, SFX_ITEMINFOFLAG_NONE },
            { ATTR_MARGIN, new SvxMarginItem( ATTR_MARGIN ), SID_ATTR_ALIGN_MARGIN, SFX_ITEMINFOFLAG_NONE },
            { ATTR_MERGE, new ScMergeAttr, 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_MERGE_FLAG, new ScMergeFlagAttr, 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_VALUE_FORMAT, new SfxUInt32Item( ATTR_VALUE_FORMAT, 0 ), SID_ATTR_NUMBERFORMAT_VALUE, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_LANGUAGE_FORMAT, new SvxLanguageItem( ScGlobal::eLnge, ATTR_LANGUAGE_FORMAT ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_BACKGROUND, new SvxBrushItem( COL_TRANSPARENT, ATTR_BACKGROUND ), SID_ATTR_BRUSH, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_PROTECTION, new ScProtectionAttr, SID_SCATTR_PROTECTION, SFX_ITEMINFOFLAG_NONE },
            { ATTR_BORDER, new SvxBoxItem( ATTR_BORDER ), SID_ATTR_BORDER_OUTER, SFX_ITEMINFOFLAG_NONE },
            { ATTR_BORDER_INNER,nullptr, SID_ATTR_BORDER_INNER, SFX_ITEMINFOFLAG_NONE },
            { ATTR_SHADOW, new SvxShadowItem( ATTR_SHADOW ), SID_ATTR_BORDER_SHADOW, SFX_ITEMINFOFLAG_NONE },
            { ATTR_VALIDDATA, new SfxUInt32Item( ATTR_VALIDDATA, 0 ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_CONDITIONAL, new ScCondFormatItem, 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_HYPERLINK, new SfxStringItem( ATTR_HYPERLINK, OUString() ) , 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_LRSPACE, new SvxLRSpaceItem( ATTR_LRSPACE ), SID_ATTR_LRSPACE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_ULSPACE, new SvxULSpaceItem( ATTR_ULSPACE ), SID_ATTR_ULSPACE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE, new SvxPageItem( ATTR_PAGE ), SID_ATTR_PAGE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_PAPERBIN, new SvxPaperBinItem( ATTR_PAGE_PAPERBIN ), SID_ATTR_PAGE_PAPERBIN, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_SIZE, new SvxSizeItem( ATTR_PAGE_SIZE ), SID_ATTR_PAGE_SIZE, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_HORCENTER, new SfxBoolItem( ATTR_PAGE_HORCENTER ), SID_ATTR_PAGE_EXT1, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_VERCENTER, new SfxBoolItem( ATTR_PAGE_VERCENTER ), SID_ATTR_PAGE_EXT2, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_ON, new SfxBoolItem( ATTR_PAGE_ON, true ), SID_ATTR_PAGE_ON, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_DYNAMIC, new SfxBoolItem( ATTR_PAGE_DYNAMIC, true ), SID_ATTR_PAGE_DYNAMIC, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_SHARED, new SfxBoolItem( ATTR_PAGE_SHARED, true ), SID_ATTR_PAGE_SHARED, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_SHARED_FIRST, new SfxBoolItem( ATTR_PAGE_SHARED_FIRST, true ), SID_ATTR_PAGE_SHARED_FIRST, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_NOTES, new SfxBoolItem( ATTR_PAGE_NOTES, false ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_GRID, new SfxBoolItem( ATTR_PAGE_GRID, false ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_HEADERS, new SfxBoolItem( ATTR_PAGE_HEADERS, false ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_CHARTS, new ScViewObjectModeItem( ATTR_PAGE_CHARTS ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_OBJECTS, new ScViewObjectModeItem( ATTR_PAGE_OBJECTS ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_DRAWINGS, new ScViewObjectModeItem( ATTR_PAGE_DRAWINGS ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_TOPDOWN, new SfxBoolItem( ATTR_PAGE_TOPDOWN, true ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_SCALE, new SfxUInt16Item( ATTR_PAGE_SCALE, 100 ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_SCALETOPAGES, new SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 1 ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_FIRSTPAGENO, new SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 1 ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_HEADERLEFT, new ScPageHFItem( ATTR_PAGE_HEADERLEFT ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_PAGE_FOOTERLEFT, new ScPageHFItem( ATTR_PAGE_FOOTERLEFT ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_PAGE_HEADERRIGHT, new ScPageHFItem( ATTR_PAGE_HEADERRIGHT ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_PAGE_FOOTERRIGHT, new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_PAGE_HEADERFIRST, new ScPageHFItem( ATTR_PAGE_HEADERFIRST ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_PAGE_FOOTERFIRST, new ScPageHFItem( ATTR_PAGE_FOOTERFIRST ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE },
            { ATTR_PAGE_HEADERSET, nullptr, SID_ATTR_PAGE_HEADERSET, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_FOOTERSET, nullptr, SID_ATTR_PAGE_FOOTERSET, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_FORMULAS, new SfxBoolItem( ATTR_PAGE_FORMULAS, false ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_NULLVALS, new SfxBoolItem( ATTR_PAGE_NULLVALS, true ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_PAGE_SCALETO, new ScPageScaleToItem( 1, 1 ), 0, SFX_ITEMINFOFLAG_NONE },
            { ATTR_HIDDEN, new SfxBoolItem( ATTR_HIDDEN, false ), 0, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        ItemInfoPackageScDocument()
        {
            LanguageType nDefLang, nCjkLang, nCtlLang;
            ScModule::GetSpellSettings( nDefLang, nCjkLang, nCtlLang );

            //  latin font from GetDefaultFonts is not used, DEFAULTFONT_LATIN_SPREADSHEET instead
            SvxFontItem* pStdFont = getDefaultFontItem(nDefLang, DefaultFontType::LATIN_SPREADSHEET, ATTR_FONT);
            SvxFontItem* pCjkFont = getDefaultFontItem(nCjkLang, DefaultFontType::CJK_SPREADSHEET, ATTR_CJK_FONT);
            SvxFontItem* pCtlFont = getDefaultFontItem(nCtlLang, DefaultFontType::CTL_SPREADSHEET, ATTR_CTL_FONT);
            setItemAtItemInfoStatic(pStdFont, maItemInfos[ATTR_FONT - ATTR_STARTINDEX]);
            setItemAtItemInfoStatic(pCjkFont, maItemInfos[ATTR_CJK_FONT - ATTR_STARTINDEX]);
            setItemAtItemInfoStatic(pCtlFont, maItemInfos[ATTR_CTL_FONT - ATTR_STARTINDEX]);

            SvxBoxInfoItem* pGlobalBorderInnerAttr = new SvxBoxInfoItem( ATTR_BORDER_INNER );
            pGlobalBorderInnerAttr->SetLine(nullptr, SvxBoxInfoItemLine::HORI);
            pGlobalBorderInnerAttr->SetLine(nullptr, SvxBoxInfoItemLine::VERT);
            pGlobalBorderInnerAttr->SetTable(true);
            pGlobalBorderInnerAttr->SetDist(true);
            pGlobalBorderInnerAttr->SetMinDist(false);
            setItemAtItemInfoStatic(pGlobalBorderInnerAttr, maItemInfos[ATTR_BORDER_INNER - ATTR_STARTINDEX]);
        }
        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& rPool) override
        {
            const ItemInfo& rRetval(maItemInfos[nIndex]);

            // return immediately if we have the static entry and Item
            if (nullptr != rRetval.getItem())
                return rRetval;

            if (ATTR_PAGE_HEADERSET == rRetval.getWhich())
            {
                SfxItemSet aSetItemItemSet(rPool,
                    svl::Items<
                        ATTR_BACKGROUND, ATTR_BACKGROUND,
                        ATTR_BORDER, ATTR_SHADOW,
                        ATTR_LRSPACE, ATTR_ULSPACE,
                        ATTR_PAGE_SIZE,  ATTR_PAGE_SIZE,
                        ATTR_PAGE_ON, ATTR_PAGE_SHARED_FIRST>);
                return *new ItemInfoDynamic(rRetval, new SvxSetItem(ATTR_PAGE_HEADERSET, aSetItemItemSet));
            }

            if (ATTR_PAGE_FOOTERSET == rRetval.getWhich())
            {
                SfxItemSet aSetItemItemSet(rPool,
                    svl::Items<
                        ATTR_BACKGROUND, ATTR_BACKGROUND,
                        ATTR_BORDER, ATTR_SHADOW,
                        ATTR_LRSPACE, ATTR_ULSPACE,
                        ATTR_PAGE_SIZE,  ATTR_PAGE_SIZE,
                        ATTR_PAGE_ON, ATTR_PAGE_SHARED_FIRST>);
                return *new ItemInfoDynamic(rRetval, new SvxSetItem(ATTR_PAGE_FOOTERSET, aSetItemItemSet));
            }

            // return in any case
            return rRetval;
        }
    };

    static std::unique_ptr<ItemInfoPackageScDocument> g_aItemInfoPackageScDocument;
    if (!g_aItemInfoPackageScDocument)
        g_aItemInfoPackageScDocument.reset(new ItemInfoPackageScDocument);
    return *g_aItemInfoPackageScDocument;
}

ScDocumentPool::ScDocumentPool()
: SfxItemPool(u"ScDocumentPool"_ustr)
{
    registerItemInfoPackage(getItemInfoPackageScDocument());
}

ScDocumentPool::~ScDocumentPool()
{
    sendShutdownHint();
    SetSecondaryPool(nullptr);
}

rtl::Reference<SfxItemPool> ScDocumentPool::Clone() const
{
    return new SfxItemPool(*this);
}

static bool lcl_HFPresentation
(
    const SfxPoolItem&  rItem,
    MapUnit             eCoreMetric,
    MapUnit             ePresentationMetric,
    OUString&           rText,
    const IntlWrapper& rIntl
)
{
    const SfxItemSet& rSet = static_cast<const SfxSetItem&>(rItem).GetItemSet();

    if ( const SfxBoolItem* pItem = rSet.GetItemIfSet(ATTR_PAGE_ON,false) )
    {
        if( !pItem->GetValue() )
            return false;
    }

    SfxItemIter aIter( rSet );

    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        sal_uInt16 nWhich = pItem->Which();

        OUString aText;

        switch( nWhich )
        {
            case ATTR_PAGE_ON:
            case ATTR_PAGE_DYNAMIC:
            case ATTR_PAGE_SHARED:
            case ATTR_PAGE_SHARED_FIRST:
            break;

            case ATTR_LRSPACE:
            {
                const SvxLRSpaceItem& rLRItem = static_cast<const SvxLRSpaceItem&>(*pItem);
                sal_uInt16 nPropLeftMargin  = rLRItem.GetPropLeft();
                sal_uInt16 nPropRightMargin = rLRItem.GetPropRight();
                sal_uInt16 nLeftMargin, nRightMargin;
                tools::Long nTmp;
                nTmp = rLRItem.GetLeft();
                nLeftMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);
                nTmp = rLRItem.GetRight();
                nRightMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);

                aText = EditResId(RID_SVXITEMS_LRSPACE_LEFT);
                if ( 100 != nPropLeftMargin )
                {
                    aText += unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( static_cast<tools::Long>(nLeftMargin),
                                           eCoreMetric, ePresentationMetric, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresentationMetric));
                }
                aText += cpDelim +
                    // We don't have a nPropFirstLineOffset
                    EditResId(RID_SVXITEMS_LRSPACE_RIGHT);
                if ( 100 != nPropRightMargin )
                {
                    aText += unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( static_cast<tools::Long>(nRightMargin),
                                            eCoreMetric, ePresentationMetric, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresentationMetric));
                }
            }
            break;

            default:
                pItem->GetPresentation( SfxItemPresentation::Complete, eCoreMetric, ePresentationMetric, aText, rIntl );

        }

        if ( aText.getLength() )
        {
            rText += aText + " + ";
        }
    }

    rText = comphelper::string::stripEnd(rText, ' ');
    rText = comphelper::string::stripEnd(rText, '+');
    rText = comphelper::string::stripEnd(rText, ' ');
    return true;
}

bool ScDocumentPool::GetPresentation(
    const SfxPoolItem&  rItem,
    MapUnit             ePresentationMetric,
    OUString&           rText,
    const IntlWrapper& rIntl ) const
{
    sal_uInt16  nW = rItem.Which();
    OUString aStrYes  ( ScResId(STR_YES) );
    OUString aStrNo   ( ScResId(STR_NO) );
    OUString aStrSep(u": "_ustr);

    bool ePresentationRet = true;
    switch( nW )
    {
        case ATTR_PAGE_TOPDOWN:
            rText = ScResId(STR_SCATTR_PAGE_PRINTDIR) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ?
                ScResId(STR_SCATTR_PAGE_TOPDOWN) :
                ScResId(STR_SCATTR_PAGE_LEFTRIGHT) ;
        break;

        case ATTR_PAGE_HEADERS:
            rText = ScResId(STR_SCATTR_PAGE_HEADERS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_NULLVALS:
            rText = ScResId(STR_SCATTR_PAGE_NULLVALS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_FORMULAS:
            rText = ScResId(STR_SCATTR_PAGE_FORMULAS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_NOTES:
            rText = ScResId(STR_SCATTR_PAGE_NOTES) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_GRID:
            rText = ScResId(STR_SCATTR_PAGE_GRID) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_SCALETOPAGES:
        {
            sal_uInt16  nPagNo = static_cast<const SfxUInt16Item&>(rItem).GetValue();

            if( nPagNo )
            {
                rText = ScResId( STR_SCATTR_PAGE_SCALETOPAGES ) + aStrSep;
                OUString aPages(ScResId(STR_SCATTR_PAGE_SCALE_PAGES, nPagNo));
                aPages = aPages.replaceFirst( "%1", OUString::number( nPagNo ) );
                rText += aPages;
            }
            else
            {
                ePresentationRet = false;
            }
        }
        break;

        case ATTR_PAGE_FIRSTPAGENO:
        {
            sal_uInt16  nPagNo = static_cast<const SfxUInt16Item&>(rItem).GetValue();

            if( nPagNo )
            {
                rText = ScResId(STR_SCATTR_PAGE_FIRSTPAGENO) + aStrSep;
                rText += OUString::number( nPagNo );
            }
            else
            {
                ePresentationRet = false;
            }
        }
        break;

        case ATTR_PAGE_SCALE:
        {
            sal_uInt16  nPercent = static_cast<const SfxUInt16Item &>(rItem).GetValue();

            if( nPercent )
            {
                rText = ScResId(STR_SCATTR_PAGE_SCALE) + aStrSep;
                rText += unicode::formatPercent(nPercent,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
            {
                ePresentationRet = false;
            }
        }
        break;

        case ATTR_PAGE_HEADERSET:
        {
            OUString  aBuffer;

            if( lcl_HFPresentation( rItem, GetMetric( nW ), ePresentationMetric, aBuffer, rIntl ) )
            {
                rText = ScResId(STR_HEADER) + " ( " + aBuffer + " ) ";
            }
        }
        break;

        case ATTR_PAGE_FOOTERSET:
        {
            OUString  aBuffer;

            if( lcl_HFPresentation( rItem, GetMetric( nW ), ePresentationMetric, aBuffer, rIntl ) )
            {
                rText = ScResId(STR_FOOTER) + " ( " + aBuffer + " ) ";
            }
        }
        break;

        default:
            ePresentationRet = rItem.GetPresentation( SfxItemPresentation::Complete, GetMetric( nW ), ePresentationMetric, rText, rIntl );
        break;
    }

    return ePresentationRet;
}

MapUnit ScDocumentPool::GetMetric( sal_uInt16 nWhich ) const
{
    // Own attributes in Twips, everything else in 1/100 mm
    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return MapUnit::MapTwip;
    else
        return MapUnit::Map100thMM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
