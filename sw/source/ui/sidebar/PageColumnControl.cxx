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

#include "PageColumnControl.hxx"
#include "PagePropertyPanel.hxx"
#include "PagePropertyPanel.hrc"

#include <cmdid.h>
#include <swtypes.hxx>

#include <svx/sidebar/ValueSetWithTextControl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace sw { namespace sidebar {

PageColumnControl::PageColumnControl(
    Window* pParent,
    PagePropertyPanel& rPanel,
    const sal_uInt16 nColumnType,
    const bool bLandscape )
    : ::svx::sidebar::PopupControl( pParent, SW_RES(RID_POPUP_SWPAGE_COLUMN) )
    , mpColumnValueSet( new ::svx::sidebar::ValueSetWithTextControl( ::svx::sidebar::ValueSetWithTextControl::IMAGE_TEXT, this, SW_RES(VS_COLUMN) ) )
    , maMoreButton( this, SW_RES(CB_COLUMN_MORE) )
    , mnColumnType( nColumnType )
    , mrPagePropPanel(rPanel)
{
    mpColumnValueSet->SetStyle( mpColumnValueSet->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT );
    mpColumnValueSet->SetColor(GetSettings().GetStyleSettings().GetMenuColor());

    if ( bLandscape )
    {
        mpColumnValueSet->AddItem( SW_RES(IMG_ONE_L), 0, SW_RES(STR_ONE), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_TWO_L), 0, SW_RES(STR_TWO), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_THREE_L), 0, SW_RES(STR_THREE), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_LEFT_L), 0, SW_RES(STR_LEFT), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_RIGHT_L), 0, SW_RES(STR_RIGHT), 0 );
    }
    else
    {
        mpColumnValueSet->AddItem( SW_RES(IMG_ONE), 0, SW_RES(STR_ONE), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_TWO), 0, SW_RES(STR_TWO), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_THREE), 0, SW_RES(STR_THREE), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_LEFT), 0, SW_RES(STR_LEFT), 0 );
        mpColumnValueSet->AddItem( SW_RES(IMG_RIGHT), 0, SW_RES(STR_RIGHT), 0 );
    }

    mpColumnValueSet->SetNoSelection();
    mpColumnValueSet->SetSelectHdl( LINK(this, PageColumnControl,ImplColumnHdl ) );
    mpColumnValueSet->Show();
    mpColumnValueSet->SelectItem( mnColumnType );
    mpColumnValueSet->Format();
    mpColumnValueSet->StartSelection();

    maMoreButton.SetClickHdl( LINK( this, PageColumnControl, MoreButtonClickHdl_Impl ) );
    maMoreButton.GrabFocus();

    FreeResource();
}


PageColumnControl::~PageColumnControl(void)
{
    delete mpColumnValueSet;
}


IMPL_LINK(PageColumnControl, ImplColumnHdl, void *, pControl)
{
    mpColumnValueSet->SetNoSelection();
    if ( pControl == mpColumnValueSet )
    {
        const sal_uInt32 nColumnType = mpColumnValueSet->GetSelectItemId();
        if ( nColumnType != mnColumnType )
        {
            mnColumnType = nColumnType;
            mrPagePropPanel.ExecuteColumnChange( mnColumnType );
        }
    }

    mrPagePropPanel.ClosePageColumnPopup();
    return 0;
}

IMPL_LINK(PageColumnControl, MoreButtonClickHdl_Impl, void *, EMPTYARG)
{
    mrPagePropPanel.GetBindings()->GetDispatcher()->Execute( FN_FORMAT_PAGE_COLUMN_DLG, SFX_CALLMODE_ASYNCHRON );

    mrPagePropPanel.ClosePageColumnPopup();
    return 0;
}


} } // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
