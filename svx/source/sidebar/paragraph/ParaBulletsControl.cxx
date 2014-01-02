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
#include "ParaBulletsControl.hxx"
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
#include <svx/nbdtmg.hxx>
#include <svx/nbdtmgfact.hxx>
#include <vcl/settings.hxx>

namespace svx { namespace sidebar {

ParaBulletsControl::ParaBulletsControl(
    Window* pParent,
    svx::sidebar::ParaPropertyPanel& rPanel )
    : PopupControl( pParent,SVX_RES(RID_POPUPPANEL_PARAPAGE_BULLETS) )
    , maBulletsVS( this,SVX_RES(VS_VALUES) )
    , maMoreButton( this,SVX_RES(CB_BULLET_MORE) )
    , mrParaPropertyPanel( rPanel )
    , mpBindings( mrParaPropertyPanel.GetBindings() )
{
    FreeResource();

    maBulletsVS.SetColCount(3);
    maBulletsVS.SetLineCount(3);
    maBulletsVS.SetStyle( maBulletsVS.GetStyle() | WB_ITEMBORDER |WB_NO_DIRECTSELECT);
    maBulletsVS.SetExtraSpacing(BULLET_IMAGE_SPACING);
    maBulletsVS.SetItemWidth(BULLET_IMAGE_WIDTH);
    maBulletsVS.SetItemHeight(BULLET_IMAGE_HEIGHT);
    maBulletsVS.InsertItem( DEFAULT_NONE );
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= DEFAULT_BULLET_TYPES; ++nVSIdx )
    {
        maBulletsVS.InsertItem( nVSIdx );
    }

    maBulletsVS.SetItemText( DEFAULT_NONE, SVX_RESSTR( RID_SVXSTR_NUMBULLET_NONE ));
    NBOTypeMgrBase* pBullets = NBOutlineTypeMgrFact::CreateInstance(eNBOType::MIXBULLETS);
    if ( pBullets )
    {
        for( sal_uInt16 nIndex = 0; nIndex < DEFAULT_BULLET_TYPES; ++nIndex )
        {
            maBulletsVS.SetItemText( nIndex + 1, pBullets->GetDescription(nIndex) );
        }
    }

    maBulletsVS.Show();
    maBulletsVS.SetSelectHdl(LINK(this, ParaBulletsControl, BulletSelectHdl_Impl));

    maBulletsVS.SetColor( GetSettings().GetStyleSettings().GetHighContrastMode()
                          ? GetSettings().GetStyleSettings().GetMenuColor()
                          : sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ) );
    maBulletsVS.SetBackground( GetSettings().GetStyleSettings().GetHighContrastMode()
                               ? GetSettings().GetStyleSettings().GetMenuColor()
                               : sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ) );

    maMoreButton.SetClickHdl(LINK(this, ParaBulletsControl, MoreButtonClickHdl_Impl));

}


ParaBulletsControl::~ParaBulletsControl()
{
}


void ParaBulletsControl::UpdateValueSet()
{
    maBulletsVS.StateChanged(STATE_CHANGE_STYLE);
    maBulletsVS.StateChanged(STATE_CHANGE_INITSHOW);

    const sal_uInt16 nTypeIndex = mrParaPropertyPanel.GetBulletTypeIndex();
    if ( nTypeIndex != (sal_uInt16)0xFFFF )
        maBulletsVS.SelectItem( nTypeIndex );
    else
    {
        maBulletsVS.SelectItem(0);
    }
    maMoreButton.GrabFocus();
}


IMPL_LINK(ParaBulletsControl, BulletSelectHdl_Impl, ValueSet*, EMPTYARG)
{
    const sal_uInt16 nIdx = maBulletsVS.GetSelectItemId();
    SfxUInt16Item aItem( FN_SVX_SET_BULLET, nIdx );
    if (mpBindings)
        mpBindings->GetDispatcher()->Execute( FN_SVX_SET_BULLET, SFX_CALLMODE_RECORD, &aItem, 0L );

    mrParaPropertyPanel.EndBulletsPopupMode();

    return 0;
}


IMPL_LINK(ParaBulletsControl, MoreButtonClickHdl_Impl, void*, EMPTYARG)
{
    if (mpBindings)
        mpBindings->GetDispatcher()->Execute( SID_OUTLINE_BULLET, SFX_CALLMODE_ASYNCHRON );

    mrParaPropertyPanel.EndBulletsPopupMode();

    return 0;
}

}} // end of namespace sidebar


