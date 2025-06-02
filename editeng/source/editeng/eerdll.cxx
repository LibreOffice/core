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


#include <unotools/resmgr.hxx>
#include <com/sun/star/linguistic2/LanguageGuessing.hpp>

#include <comphelper/processfactory.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/voiditem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <editeng/autokernitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/rubyitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/scripthintitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/justifyitem.hxx>
#include <tools/mapunit.hxx>
#include <tools/lazydelete.hxx>
#include <svl/itempool.hxx>
#include <editeng/editids.hrc>

using namespace ::com::sun::star;

EditDLL& EditDLL::Get()
{
    /**
      Prevent use-after-free errors during application shutdown.
      Previously this data was function-static, but then data in i18npool would
      be torn down before the destructor here ran, causing a crash.
    */
    static tools::DeleteOnDeinit< EditDLL > gaEditDll;
    return *gaEditDll.get();
}

ItemInfoPackage& getItemInfoPackageEditEngine()
{
    class ItemInfoPackageEditEngine : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, EE_ITEMS_END - EE_ITEMS_START + 1> ItemInfoArrayEditEngine;
        ItemInfoArrayEditEngine maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { EE_PARA_WRITINGDIR, new SvxFrameDirectionItem( SvxFrameDirection::Horizontal_LR_TB, EE_PARA_WRITINGDIR ), SID_ATTR_FRAMEDIRECTION, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_XMLATTRIBS, new SvXMLAttrContainerItem( EE_PARA_XMLATTRIBS ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE  },
            { EE_PARA_HANGINGPUNCTUATION, new SvxHangingPunctuationItem(false, EE_PARA_HANGINGPUNCTUATION), SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_FORBIDDENRULES, new SvxForbiddenRuleItem(true, EE_PARA_FORBIDDENRULES), SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_ASIANCJKSPACING, new SvxScriptSpaceItem( true, EE_PARA_ASIANCJKSPACING ), SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEMINFOFLAG_NONE  },

            // need to use dynamic default for this Item, the office tends to crash at shutdown
            // due to static stuff/cleanup at ~SvxNumRule (pStdNumFmt/pStdOutlineNumFmt)
            { EE_PARA_NUMBULLET, nullptr, SID_ATTR_NUMBERING_RULE, SFX_ITEMINFOFLAG_NONE  },

            { EE_PARA_HYPHENATE, new SfxBoolItem( EE_PARA_HYPHENATE, false ), 0, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_HYPHENATE_NO_CAPS, new SfxBoolItem( EE_PARA_HYPHENATE_NO_CAPS, false ), 0, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_HYPHENATE_NO_LAST_WORD, new SfxBoolItem( EE_PARA_HYPHENATE_NO_LAST_WORD, false ), 0, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_BULLETSTATE, new SfxBoolItem( EE_PARA_BULLETSTATE, true ), 0, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_OUTLLRSPACE, new SvxLRSpaceItem( EE_PARA_OUTLLRSPACE ), 0, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_OUTLLEVEL, new SfxInt16Item( EE_PARA_OUTLLEVEL, -1 ), SID_ATTR_PARA_OUTLLEVEL, SFX_ITEMINFOFLAG_NONE  },

            // needs on-demand initialization
            { EE_PARA_BULLET, nullptr, SID_ATTR_PARA_BULLET, SFX_ITEMINFOFLAG_NONE  },

            { EE_PARA_LRSPACE, new SvxLRSpaceItem( EE_PARA_LRSPACE ), SID_ATTR_LRSPACE, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_ULSPACE, new SvxULSpaceItem( EE_PARA_ULSPACE ), SID_ATTR_ULSPACE, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_SBL, new SvxLineSpacingItem( 0, EE_PARA_SBL ), SID_ATTR_PARA_LINESPACE, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_JUST, new SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ), SID_ATTR_PARA_ADJUST, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_TABS, new SvxTabStopItem( 0, 0, SvxTabAdjust::Left, EE_PARA_TABS ), SID_ATTR_TABSTOP, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_JUST_METHOD, new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, EE_PARA_JUST_METHOD ), SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD, SFX_ITEMINFOFLAG_NONE  },
            { EE_PARA_VER_JUST, new SvxVerJustifyItem( SvxCellVerJustify::Standard, EE_PARA_VER_JUST ), SID_ATTR_ALIGN_VER_JUSTIFY, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_COLOR, new SvxColorItem( COL_AUTO, EE_CHAR_COLOR ), SID_ATTR_CHAR_COLOR, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE  },

            // EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK and EE_CHAR_FONTINFO_CTL need on-demand initialization
            { EE_CHAR_FONTINFO, nullptr, SID_ATTR_CHAR_FONT, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE  },

            { EE_CHAR_FONTHEIGHT, new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT ), SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_FONTWIDTH, new SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH ), SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_WEIGHT, new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ), SID_ATTR_CHAR_WEIGHT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_UNDERLINE, new SvxUnderlineItem( LINESTYLE_NONE, EE_CHAR_UNDERLINE ), SID_ATTR_CHAR_UNDERLINE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_STRIKEOUT, new SvxCrossedOutItem( STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ), SID_ATTR_CHAR_STRIKEOUT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_ITALIC, new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ), SID_ATTR_CHAR_POSTURE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_OUTLINE, new SvxContourItem( false, EE_CHAR_OUTLINE ), SID_ATTR_CHAR_CONTOUR, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_SHADOW, new SvxShadowedItem( false, EE_CHAR_SHADOW ), SID_ATTR_CHAR_SHADOWED, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_ESCAPEMENT, new SvxEscapementItem( 0, 100, EE_CHAR_ESCAPEMENT ), SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_PAIRKERNING, new SvxAutoKernItem( false, EE_CHAR_PAIRKERNING ), SID_ATTR_CHAR_AUTOKERN, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_KERNING, new SvxKerningItem( 0, EE_CHAR_KERNING ), SID_ATTR_CHAR_KERNING, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_WLM, new SvxWordLineModeItem( false, EE_CHAR_WLM ), SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_LANGUAGE, new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE ), SID_ATTR_CHAR_LANGUAGE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_LANGUAGE_CJK, new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CJK ), SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_LANGUAGE_CTL, new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CTL ), SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEMINFOFLAG_NONE  },

            // see EE_CHAR_FONTINFO above
            { EE_CHAR_FONTINFO_CJK, nullptr, SID_ATTR_CHAR_CJK_FONT, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE  },
            { EE_CHAR_FONTINFO_CTL, nullptr, SID_ATTR_CHAR_CTL_FONT, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE  },

            { EE_CHAR_FONTHEIGHT_CJK, new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CJK ), SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_FONTHEIGHT_CTL, new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CTL ), SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_WEIGHT_CJK, new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ), SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_WEIGHT_CTL, new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ), SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_ITALIC_CJK, new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ), SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_ITALIC_CTL, new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ), SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_EMPHASISMARK, new SvxEmphasisMarkItem( FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK ), SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_RELIEF, new SvxCharReliefItem( FontRelief::NONE, EE_CHAR_RELIEF ), SID_ATTR_CHAR_RELIEF, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_XMLATTRIBS, new SvXMLAttrContainerItem( EE_CHAR_XMLATTRIBS ), 0, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE  },
            { EE_CHAR_OVERLINE, new SvxOverlineItem( LINESTYLE_NONE, EE_CHAR_OVERLINE ), SID_ATTR_CHAR_OVERLINE, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_CASEMAP, new SvxCaseMapItem( SvxCaseMap::NotMapped, EE_CHAR_CASEMAP ), SID_ATTR_CHAR_CASEMAP, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_GRABBAG, new SfxGrabBagItem( EE_CHAR_GRABBAG ), SID_ATTR_CHAR_GRABBAG, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_BKGCOLOR, new SvxColorItem( COL_AUTO, EE_CHAR_BKGCOLOR ), SID_ATTR_CHAR_BACK_COLOR, SFX_ITEMINFOFLAG_NONE  },
            { EE_CHAR_RUBY, new SvxRubyItem( EE_CHAR_RUBY ), SID_ATTR_CHAR_RUBY, SFX_ITEMINFOFLAG_NONE },
            { EE_CHAR_SCRIPT_HINT, new SvxScriptHintItem( EE_CHAR_SCRIPT_HINT ), SID_ATTR_CHAR_SCRIPT_HINT, SFX_ITEMINFOFLAG_NONE },
            { EE_FEATURE_TAB, new SfxVoidItem( EE_FEATURE_TAB ), 0, SFX_ITEMINFOFLAG_NONE  },
            { EE_FEATURE_LINEBR, new SfxVoidItem( EE_FEATURE_LINEBR ), 0, SFX_ITEMINFOFLAG_NONE  },
            { EE_FEATURE_NOTCONV, new SvxColorItem( COL_RED, EE_FEATURE_NOTCONV ), SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEMINFOFLAG_NONE  },
            { EE_FEATURE_FIELD, new SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD ), SID_FIELD, SFX_ITEMINFOFLAG_NONE  }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        ItemInfoPackageEditEngine()
        {
            // on-demand (but only once) as static defaults - above (and also
            // in constructor) the needed OutputDevice::Font stuff is not yet
            // initialized
            setItemAtItemInfoStatic(
                new SvxBulletItem(EE_PARA_BULLET),
                maItemInfos[EE_PARA_BULLET - EE_ITEMS_START]);

            // same for EE_CHAR_FONTINFO/EE_CHAR_FONTINFO_CJK/EE_CHAR_FONTINFO_CTL
            // doing here as static default will be done only once for LO runtime
            SvxFontItem* pFont(new SvxFontItem(EE_CHAR_FONTINFO));
            SvxFontItem* pFontCJK(new SvxFontItem(EE_CHAR_FONTINFO_CJK));
            SvxFontItem* pFontCTL(new SvxFontItem(EE_CHAR_FONTINFO_CTL));

            // Init DefFonts:
            GetDefaultFonts(*pFont, *pFontCJK, *pFontCTL);

            setItemAtItemInfoStatic(pFont, maItemInfos[EE_CHAR_FONTINFO - EE_ITEMS_START]);
            setItemAtItemInfoStatic(pFontCJK, maItemInfos[EE_CHAR_FONTINFO_CJK - EE_ITEMS_START]);
            setItemAtItemInfoStatic(pFontCTL, maItemInfos[EE_CHAR_FONTINFO_CTL - EE_ITEMS_START]);
        }

        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override
        {
            const ItemInfo& rRetval(maItemInfos[nIndex]);

            // return immediately if we have the static entry and Item
            if (nullptr != rRetval.getItem())
                return rRetval;

            // check for dynamic ItemInfo creation, needed here for SvxNumBulletItem
            if (EE_PARA_NUMBULLET == rRetval.getWhich())
                return *new ItemInfoDynamic(
                    rRetval,
                    new SvxNumBulletItem( SvxNumRule( SvxNumRuleFlags::NONE, 0, false ), EE_PARA_NUMBULLET ));

            // return in any case
            return rRetval;
        }
    };

    static std::unique_ptr<ItemInfoPackageEditEngine> g_aItemInfoPackageEditEngine;
    if (!g_aItemInfoPackageEditEngine)
        g_aItemInfoPackageEditEngine.reset(new ItemInfoPackageEditEngine);
    return *g_aItemInfoPackageEditEngine;
}

std::shared_ptr<SvxForbiddenCharactersTable> const & GlobalEditData::GetForbiddenCharsTable()
{
    if (!xForbiddenCharsTable)
        xForbiddenCharsTable = SvxForbiddenCharactersTable::makeForbiddenCharactersTable(::comphelper::getProcessComponentContext());
    return xForbiddenCharsTable;
}

uno::Reference< linguistic2::XLanguageGuessing > const & GlobalEditData::GetLanguageGuesser()
{
    if (!xLanguageGuesser.is())
    {
        xLanguageGuesser = linguistic2::LanguageGuessing::create( comphelper::getProcessComponentContext() );
    }
    return xLanguageGuesser;
}

OUString EditResId(TranslateId aId)
{
    return Translate::get(aId, Translate::Create("editeng"));
}

EditDLL::EditDLL()
    : pGlobalData( new GlobalEditData )
{
}

EditDLL::~EditDLL()
{
}

editeng::SharedVclResources::SharedVclResources()
    : m_pVirDev(VclPtr<VirtualDevice>::Create())
{
    m_pVirDev->SetMapMode(MapMode(MapUnit::MapTwip));
}

editeng::SharedVclResources::~SharedVclResources()
    { m_pVirDev.disposeAndClear(); }

VclPtr<VirtualDevice> const & editeng::SharedVclResources::GetVirtualDevice() const
    { return m_pVirDev; }

std::shared_ptr<editeng::SharedVclResources> EditDLL::GetSharedVclResources()
{
    SolarMutexGuard g;
    auto pLocked(pSharedVcl.lock());
    if(!pLocked)
        pSharedVcl = pLocked = std::make_shared<editeng::SharedVclResources>();
    return pLocked;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
