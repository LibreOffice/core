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
#include "ParaNumberingControl.hxx"
#include "ParaPropertyPanel.hrc"
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <unotools/viewoptions.hxx>
#include <editeng/kernitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <svtools/unitconv.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/nbdtmg.hxx>
#include <svx/nbdtmgfact.hxx>
#include <editeng/unolingu.hxx>
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;

namespace svx { namespace sidebar {

Reference<XDefaultNumberingProvider> lcl_GetNumberingProvider()
{
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xI = xMSF->createInstance(
        ::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
    Reference<XDefaultNumberingProvider> xRet(xI, UNO_QUERY);
//  DBG_ASSERT(xRet.is(), "service missing: \"com.sun.star.text.DefaultNumberingProvider\"")

    return xRet;
}

ParaNumberingControl::ParaNumberingControl(Window* pParent, svx::sidebar::ParaPropertyPanel& rPanel):
    PopupControl( pParent,SVX_RES(RID_POPUPPANEL_PARAPAGE_NUMBERING)),
    maNumberVS(this,SVX_RES(VS_NUMBERING)),
    maFISep(this,SVX_RES(IMG_SEPERATOR_NUMBERING)),
    maMoreButton(this,SVX_RES(CB_NUMBERING_MORE) ),
    mrParaPropertyPanel(rPanel),
    mpBindings(NULL)
{
    FreeResource();
    mpBindings = mrParaPropertyPanel.GetBindings();

    maNumberVS.SetStyle(maNumberVS.GetStyle() | WB_NO_DIRECTSELECT);
    maNumberVS.SetExtraSpacing(NUM_IMAGE_SPACING);
    //add by wj for sym2_7246 high contrast
    if(GetSettings().GetStyleSettings().GetHighContrastMode())
        maNumberVS.SetBackground(GetSettings().GetStyleSettings().GetMenuColor());
    else
        maNumberVS.SetBackground(Color(244,245,249));

    maNumberVS.SetItemWidth(NUM_IMAGE_WIDTH);
    maNumberVS.SetItemHeight(NUM_IMAGE_HEIGHT);

    Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
    if(xDefNum.is())
    {
        Sequence< Sequence< PropertyValue > > aNumberings;
        Locale aLocale = GetSettings().GetLanguageTag().getLocale();
        try
        {
            aNumberings =
                xDefNum->getDefaultContinuousNumberingLevels( aLocale );
        }
        catch(Exception&)
        {
        }
        Reference<XNumberingFormatter> xFormat(xDefNum, UNO_QUERY);
        maNumberVS.SetNumberingSettings(aNumberings, xFormat, aLocale);
    }

    maNumberVS.Show();
    maNumberVS.SetSelectHdl(LINK(this, ParaNumberingControl, NumSelectHdl_Impl));

    /*maMoreButton.SetDefBkColor(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_DropDownBackground ));//Color(244,245,249)//for high contrast
    maMoreButton.SetHoverBkColor(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ) );//Color( 93, 120, 163 )
    maMoreButton.SetHoverTxtColor( sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Color_PanelTitleFont ) );//Color( 255, 255, 255 )
    maMoreButton.SetIcoPosX( 2);*/
    maNumberVS.SetColor(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));
    maNumberVS.SetBackground(GetSettings().GetStyleSettings().GetHighContrastMode()?
        GetSettings().GetStyleSettings().GetMenuColor():
        sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ));

    maMoreButton.SetClickHdl(LINK(this, ParaNumberingControl, MoreButtonClickHdl_Impl));

}

ParaNumberingControl::~ParaNumberingControl()
{
}


IMPL_LINK(ParaNumberingControl, NumSelectHdl_Impl, ValueSet*, EMPTYARG)
{
    sal_uInt16 nIdx = maNumberVS.GetSelectItemId();
    SfxUInt16Item aItem(FN_SVX_SET_NUMBER, nIdx);
    if (mpBindings)
        mpBindings->GetDispatcher()->Execute( FN_SVX_SET_NUMBER, SFX_CALLMODE_RECORD, &aItem, 0L );

    mrParaPropertyPanel.EndNumberingPopupMode();

    return 0;
}

IMPL_LINK(ParaNumberingControl, MoreButtonClickHdl_Impl, void*, EMPTYARG)
{
    if (mpBindings)
        mpBindings->GetDispatcher()->Execute( SID_OUTLINE_BULLET, SFX_CALLMODE_ASYNCHRON );

    mrParaPropertyPanel.EndNumberingPopupMode();

    return 0;
}

void ParaNumberingControl::UpdateValueSet()
{
    maNumberVS.StateChanged(STATE_CHANGE_STYLE);
    maNumberVS.StateChanged(STATE_CHANGE_INITSHOW);
}

void ParaNumberingControl::ToGetFocus()
{
    sal_uInt16 nTypeIndex = mrParaPropertyPanel.GetNumTypeIndex();
    if ( nTypeIndex != (sal_uInt16)0xFFFF )
        maNumberVS.SelectItem( nTypeIndex );
    else
    {
        maNumberVS.SelectItem(0);
    }
    maMoreButton.GrabFocus();
}

}} // end of namespace sidebar



