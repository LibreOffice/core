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
#include <svl/itempool.hxx>
#include <svl/grabbagitem.hxx>
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
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/justifyitem.hxx>
#include <rtl/instance.hxx>
#include <tools/mapunit.hxx>

using namespace ::com::sun::star;

namespace
{
    class theEditDLL : public rtl::Static<EditDLL, theEditDLL> {};
}

EditDLL& EditDLL::Get()
{
    return theEditDLL::get();
}

DefItems::DefItems()
    : mvDefItems(EDITITEMCOUNT)
{
    std::vector<SfxPoolItem*>& rDefItems = mvDefItems;

    // Paragraph attributes:
    SvxNumRule aDefaultNumRule( SvxNumRuleFlags::NONE, 0, false );

    rDefItems[0]  = new SvxFrameDirectionItem( SvxFrameDirection::Horizontal_LR_TB, EE_PARA_WRITINGDIR );
    rDefItems[1]  = new SvXMLAttrContainerItem( EE_PARA_XMLATTRIBS );
    rDefItems[2]  = new SvxHangingPunctuationItem(false, EE_PARA_HANGINGPUNCTUATION);
    rDefItems[3]  = new SvxForbiddenRuleItem(true, EE_PARA_FORBIDDENRULES);
    rDefItems[4]  = new SvxScriptSpaceItem( true, EE_PARA_ASIANCJKSPACING );
    rDefItems[5]  = new SvxNumBulletItem( aDefaultNumRule, EE_PARA_NUMBULLET );
    rDefItems[6]  = new SfxBoolItem( EE_PARA_HYPHENATE, false );
    rDefItems[7]  = new SfxBoolItem( EE_PARA_HYPHENATE_NO_CAPS, false );
    rDefItems[8]  = new SfxBoolItem( EE_PARA_BULLETSTATE, true );
    rDefItems[9]  = new SvxLRSpaceItem( EE_PARA_OUTLLRSPACE );
    rDefItems[10]  = new SfxInt16Item( EE_PARA_OUTLLEVEL, -1 );
    rDefItems[11] = new SvxBulletItem( EE_PARA_BULLET );
    rDefItems[12] = new SvxLRSpaceItem( EE_PARA_LRSPACE );
    rDefItems[13] = new SvxULSpaceItem( EE_PARA_ULSPACE );
    rDefItems[14] = new SvxLineSpacingItem( 0, EE_PARA_SBL );
    rDefItems[15] = new SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST );
    rDefItems[16] = new SvxTabStopItem( 0, 0, SvxTabAdjust::Left, EE_PARA_TABS );
    rDefItems[17] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, EE_PARA_JUST_METHOD );
    rDefItems[18] = new SvxVerJustifyItem( SvxCellVerJustify::Standard, EE_PARA_VER_JUST );

    // Character attributes:
    rDefItems[19] = new SvxColorItem( COL_AUTO, EE_CHAR_COLOR );
    rDefItems[20] = new SvxFontItem( EE_CHAR_FONTINFO );
    rDefItems[21] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT );
    rDefItems[22] = new SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH );
    rDefItems[23] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT );
    rDefItems[24] = new SvxUnderlineItem( LINESTYLE_NONE, EE_CHAR_UNDERLINE );
    rDefItems[25] = new SvxCrossedOutItem( STRIKEOUT_NONE, EE_CHAR_STRIKEOUT );
    rDefItems[26] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC );
    rDefItems[27] = new SvxContourItem( false, EE_CHAR_OUTLINE );
    rDefItems[28] = new SvxShadowedItem( false, EE_CHAR_SHADOW );
    rDefItems[29] = new SvxEscapementItem( 0, 100, EE_CHAR_ESCAPEMENT );
    rDefItems[30] = new SvxAutoKernItem( false, EE_CHAR_PAIRKERNING );
    rDefItems[31] = new SvxKerningItem( 0, EE_CHAR_KERNING );
    rDefItems[32] = new SvxWordLineModeItem( false, EE_CHAR_WLM );
    rDefItems[33] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE );
    rDefItems[34] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CJK );
    rDefItems[35] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CTL );
    rDefItems[36] = new SvxFontItem( EE_CHAR_FONTINFO_CJK );
    rDefItems[37] = new SvxFontItem( EE_CHAR_FONTINFO_CTL );
    rDefItems[38] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CJK );
    rDefItems[39] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CTL );
    rDefItems[40] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK );
    rDefItems[41] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL );
    rDefItems[42] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK );
    rDefItems[43] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL );
    rDefItems[44] = new SvxEmphasisMarkItem( FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK );
    rDefItems[45] = new SvxCharReliefItem( FontRelief::NONE, EE_CHAR_RELIEF );
    rDefItems[46] = new SfxVoidItem( EE_CHAR_RUBI_DUMMY );
    rDefItems[47] = new SvXMLAttrContainerItem( EE_CHAR_XMLATTRIBS );
    rDefItems[48] = new SvxOverlineItem( LINESTYLE_NONE, EE_CHAR_OVERLINE );
    rDefItems[49] = new SvxCaseMapItem( SvxCaseMap::NotMapped, EE_CHAR_CASEMAP );
    rDefItems[50] = new SfxGrabBagItem( EE_CHAR_GRABBAG );
    rDefItems[51] = new SvxColorItem( COL_AUTO, EE_CHAR_BKGCOLOR );
    // Features
    rDefItems[52] = new SfxVoidItem( EE_FEATURE_TAB );
    rDefItems[53] = new SfxVoidItem( EE_FEATURE_LINEBR );
    rDefItems[54] = new SvxColorItem( COL_RED, EE_FEATURE_NOTCONV );
    rDefItems[55] = new SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD );

    assert(EDITITEMCOUNT == 56 && "ITEMCOUNT changed, adjust DefItems!");

    // Init DefFonts:
    GetDefaultFonts( *static_cast<SvxFontItem*>(rDefItems[EE_CHAR_FONTINFO - EE_ITEMS_START]),
                     *static_cast<SvxFontItem*>(rDefItems[EE_CHAR_FONTINFO_CJK - EE_ITEMS_START]),
                     *static_cast<SvxFontItem*>(rDefItems[EE_CHAR_FONTINFO_CTL - EE_ITEMS_START]) );
}

DefItems::~DefItems()
{
    for (auto& rItem : mvDefItems)
        delete rItem;
}

std::shared_ptr<DefItems> GlobalEditData::GetDefItems()
{
    auto xDefItems = m_xDefItems.lock();
    if (!xDefItems)
    {
        xDefItems = std::make_shared<DefItems>();
        m_xDefItems = xDefItems;
    }
    return xDefItems;
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
