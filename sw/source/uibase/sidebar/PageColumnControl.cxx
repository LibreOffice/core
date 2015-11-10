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
#include <vcl/settings.hxx>

namespace sw { namespace sidebar {

PageColumnControl::PageColumnControl(
    vcl::Window* pParent,
    PagePropertyPanel& rPanel,
    const sal_uInt16 nColumnType,
    const bool bLandscape )
    : svx::sidebar::PopupControl( pParent, SW_RES(RID_POPUP_SWPAGE_COLUMN) )
    , mpColumnValueSet( VclPtr<svx::sidebar::ValueSetWithTextControl>::Create( svx::sidebar::ValueSetWithTextControl::IMAGE_TEXT, this, SW_RES(VS_COLUMN) ) )
    , maMoreButton( VclPtr<PushButton>::Create( this, SW_RES(CB_COLUMN_MORE) ) )
    , mnColumnType( nColumnType )
    , mrPagePropPanel(rPanel)
{
    mpColumnValueSet->SetStyle( mpColumnValueSet->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT );
    mpColumnValueSet->SetColor(GetSettings().GetStyleSettings().GetMenuColor());

    if ( bLandscape )
    {
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_ONE_L)), nullptr, SW_RES(STR_ONE), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_TWO_L)), nullptr, SW_RES(STR_TWO), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_THREE_L)), nullptr, SW_RES(STR_THREE), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_LEFT_L)), nullptr, SW_RES(STR_LEFT), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_RIGHT_L)), nullptr, SW_RES(STR_RIGHT), nullptr );
    }
    else
    {
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_ONE)), nullptr, SW_RES(STR_ONE), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_TWO)), nullptr, SW_RES(STR_TWO), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_THREE)), nullptr, SW_RES(STR_THREE), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_LEFT)), nullptr, SW_RES(STR_LEFT), nullptr );
        mpColumnValueSet->AddItem(Image(SW_RES(IMG_RIGHT)), nullptr, SW_RES(STR_RIGHT), nullptr );
    }

    mpColumnValueSet->SetNoSelection();
    mpColumnValueSet->SetSelectHdl( LINK(this, PageColumnControl,ImplColumnHdl ) );
    mpColumnValueSet->Show();
    mpColumnValueSet->SelectItem( mnColumnType );
    mpColumnValueSet->SetFormat();
    mpColumnValueSet->Invalidate();
    mpColumnValueSet->StartSelection();

    maMoreButton->SetClickHdl( LINK( this, PageColumnControl, MoreButtonClickHdl_Impl ) );
    maMoreButton->GrabFocus();

    FreeResource();
}

PageColumnControl::~PageColumnControl()
{
    disposeOnce();
}

void PageColumnControl::dispose()
{
    mpColumnValueSet.disposeAndClear();
    maMoreButton.disposeAndClear();
    svx::sidebar::PopupControl::dispose();
}

IMPL_LINK_TYPED(PageColumnControl, ImplColumnHdl, ValueSet*, pControl, void)
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
}

IMPL_LINK_NOARG_TYPED(PageColumnControl, MoreButtonClickHdl_Impl, Button*, void)
{
    mrPagePropPanel.GetBindings()->GetDispatcher()->Execute( FN_FORMAT_PAGE_COLUMN_DLG, SfxCallMode::ASYNCHRON );

    mrPagePropPanel.ClosePageColumnPopup();
}

} } // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
