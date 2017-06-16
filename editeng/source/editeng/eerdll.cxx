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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/linguistic2/LanguageGuessing.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <svl/solar.hrc>
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
#include <editeng/numdef.hxx>
#include <svl/itempool.hxx>
#include <svl/grabbagitem.hxx>
#include <vcl/virdev.hxx>

#include <editeng/autokernitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/charsetcoloritem.hxx>
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
{
    ppDefItems = new std::vector<SfxPoolItem*>(EDITITEMCOUNT);
    std::vector<SfxPoolItem*>& rDefItems = *ppDefItems;

    // Paragraph attributes:
    SvxNumRule aDefaultNumRule( SvxNumRuleFlags::NONE, 0, false );

    rDefItems[0]  = new SvxFrameDirectionItem( SvxFrameDirection::Horizontal_LR_TB, EE_PARA_WRITINGDIR );
    rDefItems[1]  = new SvXMLAttrContainerItem( EE_PARA_XMLATTRIBS );
    rDefItems[2]  = new SvxHangingPunctuationItem(false, EE_PARA_HANGINGPUNCTUATION);
    rDefItems[3]  = new SvxForbiddenRuleItem(true, EE_PARA_FORBIDDENRULES);
    rDefItems[4]  = new SvxScriptSpaceItem( true, EE_PARA_ASIANCJKSPACING );
    rDefItems[5]  = new SvxNumBulletItem( aDefaultNumRule, EE_PARA_NUMBULLET );
    rDefItems[6]  = new SfxBoolItem( EE_PARA_HYPHENATE, false );
    rDefItems[7]  = new SfxBoolItem( EE_PARA_BULLETSTATE, true );
    rDefItems[8]  = new SvxLRSpaceItem( EE_PARA_OUTLLRSPACE );
    rDefItems[9]  = new SfxInt16Item( EE_PARA_OUTLLEVEL, -1 );
    rDefItems[10] = new SvxBulletItem( EE_PARA_BULLET );
    rDefItems[11] = new SvxLRSpaceItem( EE_PARA_LRSPACE );
    rDefItems[12] = new SvxULSpaceItem( EE_PARA_ULSPACE );
    rDefItems[13] = new SvxLineSpacingItem( 0, EE_PARA_SBL );
    rDefItems[14] = new SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST );
    rDefItems[15] = new SvxTabStopItem( 0, 0, SvxTabAdjust::Left, EE_PARA_TABS );
    rDefItems[16] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, EE_PARA_JUST_METHOD );
    rDefItems[17] = new SvxVerJustifyItem( SVX_VER_JUSTIFY_STANDARD, EE_PARA_VER_JUST );

    // Character attributes:
    rDefItems[18] = new SvxColorItem( Color( COL_AUTO ), EE_CHAR_COLOR );
    rDefItems[19] = new SvxFontItem( EE_CHAR_FONTINFO );
    rDefItems[20] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT );
    rDefItems[21] = new SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH );
    rDefItems[22] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT );
    rDefItems[23] = new SvxUnderlineItem( LINESTYLE_NONE, EE_CHAR_UNDERLINE );
    rDefItems[24] = new SvxCrossedOutItem( STRIKEOUT_NONE, EE_CHAR_STRIKEOUT );
    rDefItems[25] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC );
    rDefItems[26] = new SvxContourItem( false, EE_CHAR_OUTLINE );
    rDefItems[27] = new SvxShadowedItem( false, EE_CHAR_SHADOW );
    rDefItems[28] = new SvxEscapementItem( 0, 100, EE_CHAR_ESCAPEMENT );
    rDefItems[29] = new SvxAutoKernItem( false, EE_CHAR_PAIRKERNING );
    rDefItems[30] = new SvxKerningItem( 0, EE_CHAR_KERNING );
    rDefItems[31] = new SvxWordLineModeItem( false, EE_CHAR_WLM );
    rDefItems[32] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE );
    rDefItems[33] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CJK );
    rDefItems[34] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CTL );
    rDefItems[35] = new SvxFontItem( EE_CHAR_FONTINFO_CJK );
    rDefItems[36] = new SvxFontItem( EE_CHAR_FONTINFO_CTL );
    rDefItems[37] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CJK );
    rDefItems[38] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CTL );
    rDefItems[39] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK );
    rDefItems[40] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL );
    rDefItems[41] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK );
    rDefItems[42] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL );
    rDefItems[43] = new SvxEmphasisMarkItem( FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK );
    rDefItems[44] = new SvxCharReliefItem( FontRelief::NONE, EE_CHAR_RELIEF );
    rDefItems[45] = new SfxVoidItem( EE_CHAR_RUBI_DUMMY );
    rDefItems[46] = new SvXMLAttrContainerItem( EE_CHAR_XMLATTRIBS );
    rDefItems[47] = new SvxOverlineItem( LINESTYLE_NONE, EE_CHAR_OVERLINE );
    rDefItems[48] = new SvxCaseMapItem( SvxCaseMap::NotMapped, EE_CHAR_CASEMAP );
    rDefItems[49] = new SfxGrabBagItem( EE_CHAR_GRABBAG );
    rDefItems[50] = new SvxBackgroundColorItem( Color( COL_AUTO ), EE_CHAR_BKGCOLOR );
    // Features
    rDefItems[51] = new SfxVoidItem( EE_FEATURE_TAB );
    rDefItems[52] = new SfxVoidItem( EE_FEATURE_LINEBR );
    rDefItems[53] = new SvxCharSetColorItem( Color( COL_RED ), RTL_TEXTENCODING_DONTKNOW, EE_FEATURE_NOTCONV );
    rDefItems[54] = new SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD );

    assert(EDITITEMCOUNT == 55 && "ITEMCOUNT changed, adjust DefItems!");

    // Init DefFonts:
    GetDefaultFonts( *static_cast<SvxFontItem*>(rDefItems[EE_CHAR_FONTINFO - EE_ITEMS_START]),
                     *static_cast<SvxFontItem*>(rDefItems[EE_CHAR_FONTINFO_CJK - EE_ITEMS_START]),
                     *static_cast<SvxFontItem*>(rDefItems[EE_CHAR_FONTINFO_CTL - EE_ITEMS_START]) );
}

DefItems::~DefItems()
{
    SfxItemPool::ReleaseDefaults(ppDefItems, true);
}

std::shared_ptr<DefItems> GlobalEditData::GetDefItems()
{
    auto xDefItems = m_xDefItems.lock();
    if (!xDefItems)
    {
        xDefItems.reset(new DefItems);
        m_xDefItems = xDefItems;
    }
    return xDefItems;
}

rtl::Reference<SvxForbiddenCharactersTable> const & GlobalEditData::GetForbiddenCharsTable()
{
    if ( !xForbiddenCharsTable.is() )
    {
        xForbiddenCharsTable = new SvxForbiddenCharactersTable( ::comphelper::getProcessComponentContext() );
    }
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

EditResId::EditResId(sal_uInt16 nId)
    : ResId(nId, *EditDLL::GetResMgr())
{
}

EditDLL::EditDLL()
    : pGlobalData( new GlobalEditData )
{
}

EditDLL::~EditDLL()
{
}

static ResMgr* pResMgr=nullptr;

ResMgr* EditDLL::GetResMgr()
{
    if (!pResMgr)
        pResMgr = ResMgr::CreateResMgr("editeng", Application::GetSettings().GetUILanguageTag());
    return pResMgr;
}


editeng::SharedVclResources::SharedVclResources()
    : m_pVirDev(VclPtr<VirtualDevice>::Create())
{
    m_pVirDev->SetMapMode(MapUnit::MapTwip);
}

editeng::SharedVclResources::~SharedVclResources()
    { m_pVirDev.disposeAndClear(); }

VclPtr<VirtualDevice> editeng::SharedVclResources::GetVirtualDevice()
    { return m_pVirDev; }

std::shared_ptr<editeng::SharedVclResources> EditDLL::GetSharedVclResources()
{
    SolarMutexGuard g;
    auto pLocked(pSharedVcl.lock());
    if(!pLocked)
        pSharedVcl = pLocked = std::shared_ptr<editeng::SharedVclResources>(new editeng::SharedVclResources());
    return pLocked;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
