/*************************************************************************
 *
 *  $RCSfile: eerdll.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: cl $ $Date: 2002-02-25 16:08:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <eeng_pch.hxx>

#pragma hdrstop

#include <svtools/solar.hrc>
#include <eerdll.hxx>
#include <eerdll2.hxx>
#include <lspcitem.hxx>
#include <adjitem.hxx>
#include <tstpitem.hxx>
#include <bulitem.hxx>
#include <flditem.hxx>
#include <emphitem.hxx>
#include <scriptspaceitem.hxx>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <akrnitem.hxx>
#include <cntritem.hxx>
#include <colritem.hxx>
#include <crsditem.hxx>
#include <cscoitem.hxx>
#include <escpitem.hxx>
#include <fhgtitem.hxx>
#include <fontitem.hxx>
#include <kernitem.hxx>
#include <lrspitem.hxx>
#include <postitem.hxx>
#include <shdditem.hxx>
#include <udlnitem.hxx>
#include <ulspitem.hxx>
#include <wghtitem.hxx>
#include <wrlmitem.hxx>
#include <numitem.hxx>
#include <langitem.hxx>
#include <charscaleitem.hxx>
#include <charreliefitem.hxx>
#include <xmlcnitm.hxx>

#include <forbiddencharacterstable.hxx>

#include <comphelper/processfactory.hxx>

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

#ifndef SVX_LIGHT
        ppDefItems[0]  = new SvXMLAttrContainerItem( EE_PARA_XMLATTRIBS );
#else
        // no need to have alien attributes persistent
        ppDefItems[0]  = new SfxVoidItem( EE_PARA_XMLATTRIBS );
#endif // #ifndef SVX_LIGHT

        ppDefItems[1]  = new SfxBoolItem( EE_PARA_HANGINGPUNCTUATION, FALSE );
        ppDefItems[2]  = new SfxBoolItem( EE_PARA_FORBIDDENRULES, TRUE );
        ppDefItems[3]  = new SvxScriptSpaceItem( TRUE, EE_PARA_ASIANCJKSPACING );
        ppDefItems[4]  = new SvxNumBulletItem( aTmpNumRule, EE_PARA_NUMBULLET );
        ppDefItems[5]  = new SfxBoolItem( EE_PARA_HYPHENATE, FALSE );
        ppDefItems[6]  = new SfxUInt16Item( EE_PARA_BULLETSTATE, 0 );
        ppDefItems[7]  = new SvxLRSpaceItem( EE_PARA_OUTLLRSPACE );
        ppDefItems[8]  = new SfxUInt16Item( EE_PARA_OUTLLEVEL );
        ppDefItems[9]  = new SvxBulletItem( EE_PARA_BULLET );
        ppDefItems[10]  = new SvxLRSpaceItem( EE_PARA_LRSPACE );
        ppDefItems[11]  = new SvxULSpaceItem( EE_PARA_ULSPACE );
        ppDefItems[12]  = new SvxLineSpacingItem( 0, EE_PARA_SBL );
        ppDefItems[13]  = new SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST );
        ppDefItems[14]  = new SvxTabStopItem( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );

        // Zeichenattribute:
        ppDefItems[15]  = new SvxColorItem( Color( COL_AUTO ), EE_CHAR_COLOR );
        ppDefItems[16]  = new SvxFontItem( EE_CHAR_FONTINFO );
        ppDefItems[17] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT );
        ppDefItems[18] = new SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH );
        ppDefItems[19] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT );
        ppDefItems[20] = new SvxUnderlineItem( UNDERLINE_NONE, EE_CHAR_UNDERLINE );
        ppDefItems[21] = new SvxCrossedOutItem( STRIKEOUT_NONE, EE_CHAR_STRIKEOUT );
        ppDefItems[22] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC );
        ppDefItems[23] = new SvxContourItem( FALSE, EE_CHAR_OUTLINE );
        ppDefItems[24] = new SvxShadowedItem( FALSE, EE_CHAR_SHADOW );
        ppDefItems[25] = new SvxEscapementItem( 0, 100, EE_CHAR_ESCAPEMENT );
        ppDefItems[26] = new SvxAutoKernItem( FALSE, EE_CHAR_PAIRKERNING );
        ppDefItems[27] = new SvxKerningItem( 0, EE_CHAR_KERNING );
        ppDefItems[28] = new SvxWordLineModeItem( FALSE, EE_CHAR_WLM );
        ppDefItems[29] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE );
        ppDefItems[30] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CJK );
        ppDefItems[31] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CTL );
        ppDefItems[32] = new SvxFontItem( EE_CHAR_FONTINFO_CJK );
        ppDefItems[33] = new SvxFontItem( EE_CHAR_FONTINFO_CTL );
        ppDefItems[34] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CJK );
        ppDefItems[35] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CTL );
        ppDefItems[36] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK );
         ppDefItems[37] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL );
        ppDefItems[38] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK );
        ppDefItems[39] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL );
        ppDefItems[40] = new SvxEmphasisMarkItem( EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK );
        ppDefItems[41] = new SvxCharReliefItem( RELIEF_NONE, EE_CHAR_RELIEF );
        ppDefItems[42] = new SfxVoidItem( EE_CHAR_RUBI_DUMMY );
#ifndef SVX_LIGHT
        ppDefItems[43] = new SvXMLAttrContainerItem( EE_CHAR_XMLATTRIBS );
#else
        // no need to have alien attributes persistent
        ppDefItems[43] = new SfxVoidItem( EE_CHAR_XMLATTRIBS );
#endif // #ifndef SVX_LIGHT

        // Features
        ppDefItems[44] = new SfxVoidItem( EE_FEATURE_TAB );
        ppDefItems[45] = new SfxVoidItem( EE_FEATURE_LINEBR );
        ppDefItems[46] = new SvxCharSetColorItem( Color( COL_RED ), RTL_TEXTENCODING_DONTKNOW, EE_FEATURE_NOTCONV );
        ppDefItems[47] = new SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD );

        DBG_ASSERT( EDITITEMCOUNT == 48, "ITEMCOUNT geaendert, DefItems nicht angepasst!" );

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
    ResId( nId, EE_DLL()->GetResMgr() )
{
}

EditDLL::EditDLL()
{
    pGlobalData = new GlobalEditData;
    DBG_ASSERT( !*(EditDLL**)GetAppData(SHL_EDIT), "Noch eine EditDLL ?!" );
    *(EditDLL**)GetAppData(SHL_EDIT) = this;

#ifndef SVX_LIGHT
    ByteString aResMgrName( "svx" );
#else
    ByteString aResMgrName( "svl" );
#endif
    aResMgrName += ByteString::CreateFromInt32( SOLARUPD );
    pResMgr = ResMgr::CreateResMgr(
        aResMgrName.GetBuffer(), Application::GetSettings().GetUILanguage() );
}

EditDLL::~EditDLL()
{
    delete pResMgr;
    delete pGlobalData;
}
