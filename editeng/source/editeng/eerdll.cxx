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

using namespace ::com::sun::star;

namespace
{
    class theEditDLL : public rtl::Static<EditDLL, theEditDLL> {};
}

EditDLL& EditDLL::Get()
{
    return theEditDLL::get();
}

GlobalEditData::GlobalEditData()
{
    ppDefItems = nullptr;
}

GlobalEditData::~GlobalEditData()
{
    // Destroy DefItems...
    // Or simply keep them, since at end of execution?!
    if ( ppDefItems )
        SfxItemPool::ReleaseDefaults( ppDefItems, EDITITEMCOUNT, true );
}

SfxPoolItem** GlobalEditData::GetDefItems()
{
    if ( !ppDefItems )
    {
        ppDefItems = new SfxPoolItem*[EDITITEMCOUNT];

        // Paragraph attributes:
        SvxNumRule aDefaultNumRule( SvxNumRuleFlags::NONE, 0, false );

        ppDefItems[0]  = new SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR );
        ppDefItems[1]  = new SvXMLAttrContainerItem( EE_PARA_XMLATTRIBS );
        ppDefItems[2]  = new SvxHangingPunctuationItem(false, EE_PARA_HANGINGPUNCTUATION);
        ppDefItems[3]  = new SvxForbiddenRuleItem(true, EE_PARA_FORBIDDENRULES);
        ppDefItems[4]  = new SvxScriptSpaceItem( true, EE_PARA_ASIANCJKSPACING );
        ppDefItems[5]  = new SvxNumBulletItem( aDefaultNumRule, EE_PARA_NUMBULLET );
        ppDefItems[6]  = new SfxBoolItem( EE_PARA_HYPHENATE, false );
        ppDefItems[7]  = new SfxBoolItem( EE_PARA_BULLETSTATE, true );
        ppDefItems[8]  = new SvxLRSpaceItem( EE_PARA_OUTLLRSPACE );
        ppDefItems[9]  = new SfxInt16Item( EE_PARA_OUTLLEVEL, -1 );
        ppDefItems[10] = new SvxBulletItem( EE_PARA_BULLET );
        ppDefItems[11] = new SvxLRSpaceItem( EE_PARA_LRSPACE );
        ppDefItems[12] = new SvxULSpaceItem( EE_PARA_ULSPACE );
        ppDefItems[13] = new SvxLineSpacingItem( 0, EE_PARA_SBL );
        ppDefItems[14] = new SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST );
        ppDefItems[15] = new SvxTabStopItem( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );
        ppDefItems[16] = new SvxJustifyMethodItem( SVX_JUSTIFY_METHOD_AUTO, EE_PARA_JUST_METHOD );
        ppDefItems[17] = new SvxVerJustifyItem( SVX_VER_JUSTIFY_STANDARD, EE_PARA_VER_JUST );

        // Character attributes:
        ppDefItems[18] = new SvxColorItem( Color( COL_AUTO ), EE_CHAR_COLOR );
        ppDefItems[19] = new SvxFontItem( EE_CHAR_FONTINFO );
        ppDefItems[20] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT );
        ppDefItems[21] = new SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH );
        ppDefItems[22] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT );
        ppDefItems[23] = new SvxUnderlineItem( LINESTYLE_NONE, EE_CHAR_UNDERLINE );
        ppDefItems[24] = new SvxCrossedOutItem( STRIKEOUT_NONE, EE_CHAR_STRIKEOUT );
        ppDefItems[25] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC );
        ppDefItems[26] = new SvxContourItem( false, EE_CHAR_OUTLINE );
        ppDefItems[27] = new SvxShadowedItem( false, EE_CHAR_SHADOW );
        ppDefItems[28] = new SvxEscapementItem( 0, 100, EE_CHAR_ESCAPEMENT );
        ppDefItems[29] = new SvxAutoKernItem( false, EE_CHAR_PAIRKERNING );
        ppDefItems[30] = new SvxKerningItem( 0, EE_CHAR_KERNING );
        ppDefItems[31] = new SvxWordLineModeItem( false, EE_CHAR_WLM );
        ppDefItems[32] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE );
        ppDefItems[33] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CJK );
        ppDefItems[34] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CTL );
        ppDefItems[35] = new SvxFontItem( EE_CHAR_FONTINFO_CJK );
        ppDefItems[36] = new SvxFontItem( EE_CHAR_FONTINFO_CTL );
        ppDefItems[37] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CJK );
        ppDefItems[38] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CTL );
        ppDefItems[39] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK );
        ppDefItems[40] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL );
        ppDefItems[41] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK );
        ppDefItems[42] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL );
        ppDefItems[43] = new SvxEmphasisMarkItem( FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK );
        ppDefItems[44] = new SvxCharReliefItem( FontRelief::NONE, EE_CHAR_RELIEF );
        ppDefItems[45] = new SfxVoidItem( EE_CHAR_RUBI_DUMMY );
        ppDefItems[46] = new SvXMLAttrContainerItem( EE_CHAR_XMLATTRIBS );
        ppDefItems[47] = new SvxOverlineItem( LINESTYLE_NONE, EE_CHAR_OVERLINE );
        ppDefItems[48] = new SvxCaseMapItem( SVX_CASEMAP_NOT_MAPPED, EE_CHAR_CASEMAP );
        ppDefItems[49] = new SfxGrabBagItem( EE_CHAR_GRABBAG );
        ppDefItems[50] = new SvxBackgroundColorItem( Color( COL_AUTO ), EE_CHAR_BKGCOLOR );
        // Features
        ppDefItems[51] = new SfxVoidItem( EE_FEATURE_TAB );
        ppDefItems[52] = new SfxVoidItem( EE_FEATURE_LINEBR );
        ppDefItems[53] = new SvxCharSetColorItem( Color( COL_RED ), RTL_TEXTENCODING_DONTKNOW, EE_FEATURE_NOTCONV );
        ppDefItems[54] = new SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD );

        assert(EDITITEMCOUNT == 55 && "ITEMCOUNT changed, adjust DefItems!");

        // Init DefFonts:
        GetDefaultFonts( *static_cast<SvxFontItem*>(ppDefItems[EE_CHAR_FONTINFO - EE_ITEMS_START]),
                         *static_cast<SvxFontItem*>(ppDefItems[EE_CHAR_FONTINFO_CJK - EE_ITEMS_START]),
                         *static_cast<SvxFontItem*>(ppDefItems[EE_CHAR_FONTINFO_CTL - EE_ITEMS_START]) );
    }

    return ppDefItems;
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
{
    pGlobalData = new GlobalEditData;
}

EditDLL::~EditDLL()
{
    delete pGlobalData;
}

static ResMgr* pResMgr=nullptr;

ResMgr* EditDLL::GetResMgr()
{
    if (!pResMgr)
        pResMgr = ResMgr::CreateResMgr("editeng", Application::GetSettings().GetUILanguageTag());
    return pResMgr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
