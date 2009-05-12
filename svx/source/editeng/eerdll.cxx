/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: eerdll.cxx,v $
 * $Revision: 1.31.148.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <eeng_pch.hxx>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <svtools/solar.hrc>
#include <eerdll.hxx>
#include <eerdll2.hxx>
#include <svx/lspcitem.hxx>
#include <svx/adjitem.hxx>
#include <svx/tstpitem.hxx>
#include <bulitem.hxx>
#include <svx/flditem.hxx>
#include <svx/emphitem.hxx>
#include <svx/scriptspaceitem.hxx>
#include <svtools/itempool.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

#include <svx/akrnitem.hxx>
#include <svx/cntritem.hxx>
#include <svx/colritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/cscoitem.hxx>
#include <svx/escpitem.hxx>
#include <svx/fhgtitem.hxx>
#include <fontitem.hxx>
#include <svx/kernitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/postitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/wrlmitem.hxx>
#include <svx/numitem.hxx>
#include <svx/langitem.hxx>
#include <svx/charscaleitem.hxx>
#include <svx/charreliefitem.hxx>
#include <svx/frmdiritem.hxx>
#include <xmlcnitm.hxx>

#include <forbiddencharacterstable.hxx>

#include <comphelper/processfactory.hxx>

static EditDLL* pDLL=0;

using namespace ::com::sun::star;

EditDLL* EditDLL::Get()
{
    if ( !pDLL )
        pDLL = new EditDLL;
    return pDLL;
}

GlobalEditData::GlobalEditData()
{
    ppDefItems = NULL;
    pStdRefDevice = NULL;
}

GlobalEditData::~GlobalEditData()
{
    // DefItems zerstoeren...
    // Oder einfach stehen lassen, da sowieso App-Ende?!
    if ( ppDefItems )
        SfxItemPool::ReleaseDefaults( ppDefItems, EDITITEMCOUNT, TRUE );
    delete pStdRefDevice;
}

SfxPoolItem** GlobalEditData::GetDefItems()
{
    if ( !ppDefItems )
    {
        ppDefItems = new SfxPoolItem*[EDITITEMCOUNT];

        // Absatzattribute:
        SvxNumRule aTmpNumRule( 0, 0, FALSE );

        ppDefItems[0]  = new SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR );
        ppDefItems[1]  = new SvXMLAttrContainerItem( EE_PARA_XMLATTRIBS );
        ppDefItems[2]  = new SfxBoolItem( EE_PARA_HANGINGPUNCTUATION, FALSE );
        ppDefItems[3]  = new SfxBoolItem( EE_PARA_FORBIDDENRULES, TRUE );
        ppDefItems[4]  = new SvxScriptSpaceItem( TRUE, EE_PARA_ASIANCJKSPACING );
        ppDefItems[5]  = new SvxNumBulletItem( aTmpNumRule, EE_PARA_NUMBULLET );
        ppDefItems[6]  = new SfxBoolItem( EE_PARA_HYPHENATE, FALSE );
        ppDefItems[7]  = new SfxBoolItem( EE_PARA_BULLETSTATE, TRUE );
        ppDefItems[8]  = new SvxLRSpaceItem( EE_PARA_OUTLLRSPACE );
        ppDefItems[9]  = new SfxInt16Item( EE_PARA_OUTLLEVEL, -1 );
        ppDefItems[10]  = new SvxBulletItem( EE_PARA_BULLET );
        ppDefItems[11]  = new SvxLRSpaceItem( EE_PARA_LRSPACE );
        ppDefItems[12]  = new SvxULSpaceItem( EE_PARA_ULSPACE );
        ppDefItems[13]  = new SvxLineSpacingItem( 0, EE_PARA_SBL );
        ppDefItems[14]  = new SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST );
        ppDefItems[15]  = new SvxTabStopItem( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );

        // Zeichenattribute:
        ppDefItems[16]  = new SvxColorItem( Color( COL_AUTO ), EE_CHAR_COLOR );
        ppDefItems[17]  = new SvxFontItem( EE_CHAR_FONTINFO );
        ppDefItems[18] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT );
        ppDefItems[19] = new SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH );
        ppDefItems[20] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT );
        ppDefItems[21] = new SvxUnderlineItem( UNDERLINE_NONE, EE_CHAR_UNDERLINE );
        ppDefItems[22] = new SvxCrossedOutItem( STRIKEOUT_NONE, EE_CHAR_STRIKEOUT );
        ppDefItems[23] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC );
        ppDefItems[24] = new SvxContourItem( FALSE, EE_CHAR_OUTLINE );
        ppDefItems[25] = new SvxShadowedItem( FALSE, EE_CHAR_SHADOW );
        ppDefItems[26] = new SvxEscapementItem( 0, 100, EE_CHAR_ESCAPEMENT );
        ppDefItems[27] = new SvxAutoKernItem( FALSE, EE_CHAR_PAIRKERNING );
        ppDefItems[28] = new SvxKerningItem( 0, EE_CHAR_KERNING );
        ppDefItems[29] = new SvxWordLineModeItem( FALSE, EE_CHAR_WLM );
        ppDefItems[30] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE );
        ppDefItems[31] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CJK );
        ppDefItems[32] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CTL );
        ppDefItems[33] = new SvxFontItem( EE_CHAR_FONTINFO_CJK );
        ppDefItems[34] = new SvxFontItem( EE_CHAR_FONTINFO_CTL );
        ppDefItems[35] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CJK );
        ppDefItems[36] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CTL );
        ppDefItems[37] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK );
         ppDefItems[38] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL );
        ppDefItems[39] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK );
        ppDefItems[40] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL );
        ppDefItems[41] = new SvxEmphasisMarkItem( EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK );
        ppDefItems[42] = new SvxCharReliefItem( RELIEF_NONE, EE_CHAR_RELIEF );
        ppDefItems[43] = new SfxVoidItem( EE_CHAR_RUBI_DUMMY );
#ifndef SVX_LIGHT
        ppDefItems[44] = new SvXMLAttrContainerItem( EE_CHAR_XMLATTRIBS );
#else
        // no need to have alien attributes persistent
        ppDefItems[44] = new SfxVoidItem( EE_CHAR_XMLATTRIBS );
#endif // #ifndef SVX_LIGHT
        ppDefItems[45] = new SvxOverlineItem( UNDERLINE_NONE, EE_CHAR_OVERLINE );

        // Features
        ppDefItems[46] = new SfxVoidItem( EE_FEATURE_TAB );
        ppDefItems[47] = new SfxVoidItem( EE_FEATURE_LINEBR );
        ppDefItems[48] = new SvxCharSetColorItem( Color( COL_RED ), RTL_TEXTENCODING_DONTKNOW, EE_FEATURE_NOTCONV );
        ppDefItems[49] = new SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD );

        DBG_ASSERT( EDITITEMCOUNT == 50, "ITEMCOUNT geaendert, DefItems nicht angepasst!" );

        // Init DefFonts:
        GetDefaultFonts( *(SvxFontItem*)ppDefItems[EE_CHAR_FONTINFO - EE_ITEMS_START],
                         *(SvxFontItem*)ppDefItems[EE_CHAR_FONTINFO_CJK - EE_ITEMS_START],
                         *(SvxFontItem*)ppDefItems[EE_CHAR_FONTINFO_CTL - EE_ITEMS_START] );
    }

    return ppDefItems;
}

vos::ORef<SvxForbiddenCharactersTable> GlobalEditData::GetForbiddenCharsTable()
{
    if ( !xForbiddenCharsTable.isValid() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        xForbiddenCharsTable = new SvxForbiddenCharactersTable( xMSF );
    }
    return xForbiddenCharsTable;
}

uno::Reference< linguistic2::XLanguageGuessing > GlobalEditData::GetLanguageGuesser()
{
    if (!xLanguageGuesser.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMgr ( comphelper::getProcessServiceFactory() );
        if (xMgr.is())
        {
            xLanguageGuesser = uno::Reference< linguistic2::XLanguageGuessing >(
                    xMgr->createInstance(
                        rtl::OUString::createFromAscii( "com.sun.star.linguistic2.LanguageGuessing" ) ),
                        uno::UNO_QUERY );
        }
    }
    return xLanguageGuesser;
}

OutputDevice* GlobalEditData::GetStdRefDevice()
{
    if ( !pStdRefDevice )
    {
        pStdRefDevice = new VirtualDevice;
        pStdRefDevice->SetMapMode( MAP_TWIP );
    }
    return pStdRefDevice;
}

EditResId::EditResId( USHORT nId ):
    ResId( nId, *EE_DLL()->GetResMgr() )
{
}

EditDLL::EditDLL()
{
    pGlobalData = new GlobalEditData;
    ByteString aResMgrName( "svx" );
    pResMgr = ResMgr::CreateResMgr(
        aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
}

EditDLL::~EditDLL()
{
    delete pResMgr;
    delete pGlobalData;
}
