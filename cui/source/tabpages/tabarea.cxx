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

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/dialogs.hrc>

#include <cuires.hrc>
#include <svx/xtable.hxx>
#include "svx/globl3d.hxx"
#include <svx/svdmodel.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "dlgname.hxx"
#include <dialmgr.hxx>

SvxAreaTabDialog::SvxAreaTabDialog
(
    vcl::Window* pParent,
    const SfxItemSet* pAttr,
    SdrModel* pModel,
    bool bShadow
)
    : SfxTabDialog( pParent,
                  "AreaDialog",
                  "cui/ui/areadialog.ui",
                  pAttr )
    , m_nAreaTabPage(0)
    , m_nShadowTabPage(0)
    , m_nTransparenceTabPage(0)
    , mpDrawModel          ( pModel ),
    mpColorList          ( pModel->GetColorList() ),
    mpNewColorList       ( pModel->GetColorList() ),
    mpGradientList       ( pModel->GetGradientList() ),
    mpNewGradientList    ( pModel->GetGradientList() ),
    mpHatchingList       ( pModel->GetHatchList() ),
    mpNewHatchingList    ( pModel->GetHatchList() ),
    mpBitmapList         ( pModel->GetBitmapList() ),
    mpNewBitmapList      ( pModel->GetBitmapList() ),
    mpPatternList        ( pModel->GetPatternList() ),
    mpNewPatternList     ( pModel->GetPatternList() ),
    mrOutAttrs           ( *pAttr ),

    mnColorListState ( ChangeType::NONE ),
    mnBitmapListState ( ChangeType::NONE ),
    mnPatternListState ( ChangeType::NONE ),
    mnGradientListState ( ChangeType::NONE ),
    mnHatchingListState ( ChangeType::NONE ),
    mnPageType( PT_AREA ),
    mnPos( 0 ),
    mbAreaTP( false )
{
    m_nAreaTabPage = AddTabPage( "RID_SVXPAGE_AREA", SvxAreaTabPage::Create, nullptr );

    if(bShadow)
    {
        m_nShadowTabPage = AddTabPage( "RID_SVXPAGE_SHADOW", SvxShadowTabPage::Create, nullptr );
    }
    else
    {
        RemoveTabPage( "RID_SVXPAGE_SHADOW" );
    }

    m_nTransparenceTabPage = AddTabPage( "RID_SVXPAGE_TRANSPARENCE", SvxTransparenceTabPage::Create,  nullptr);

    SetCurPageId( "RID_SVXPAGE_AREA" );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxAreaTabDialog, CancelHdlImpl ) );
}

short SvxAreaTabDialog::Ok()
{
    // RET_OK is returned, if at least one
    // TabPage returns sal_True in FillItemSet().
    // This happens by default at the moment.
    return( SfxTabDialog::Ok() );
}


IMPL_LINK_NOARG_TYPED(SvxAreaTabDialog, CancelHdlImpl, Button*, void)
{
    EndDialog();
}

void SvxAreaTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nAreaTabPage )
    {
            static_cast<SvxAreaTabPage&>(rPage).SetColorList( mpColorList );
            static_cast<SvxAreaTabPage&>(rPage).SetGradientList( mpGradientList );
            static_cast<SvxAreaTabPage&>(rPage).SetHatchingList( mpHatchingList );
            static_cast<SvxAreaTabPage&>(rPage).SetBitmapList( mpBitmapList );
            static_cast<SvxAreaTabPage&>(rPage).SetPageType( mnPageType );
            static_cast<SvxAreaTabPage&>(rPage).SetDlgType( 0 );
            static_cast<SvxAreaTabPage&>(rPage).SetPos( mnPos );
            static_cast<SvxAreaTabPage&>(rPage).SetAreaTP( &mbAreaTP );
            static_cast<SvxAreaTabPage&>(rPage).SetGrdChgd( &mnGradientListState );
            static_cast<SvxAreaTabPage&>(rPage).SetHtchChgd( &mnHatchingListState );
            static_cast<SvxAreaTabPage&>(rPage).SetBmpChgd( &mnBitmapListState );
            static_cast<SvxAreaTabPage&>(rPage).SetColorChgd( &mnColorListState );
            static_cast<SvxAreaTabPage&>(rPage).Construct();
            // ActivatePge() is not called the first time
            static_cast<SvxAreaTabPage&>(rPage).ActivatePage( mrOutAttrs );
    }
    else if (nId == m_nShadowTabPage)
    {
            static_cast<SvxShadowTabPage&>(rPage).SetColorList( mpColorList );
            static_cast<SvxShadowTabPage&>(rPage).SetPageType( mnPageType );
            static_cast<SvxShadowTabPage&>(rPage).SetDlgType( 0 );
            static_cast<SvxShadowTabPage&>(rPage).SetAreaTP( &mbAreaTP );
            static_cast<SvxShadowTabPage&>(rPage).SetColorChgd( &mnColorListState );
            static_cast<SvxShadowTabPage&>(rPage).Construct();
    }
    else if (nId == m_nTransparenceTabPage)
    {
            static_cast<SvxTransparenceTabPage&>(rPage).SetPageType( mnPageType );
            static_cast<SvxTransparenceTabPage&>(rPage).SetDlgType( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
