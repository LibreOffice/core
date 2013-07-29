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
    Window* pParent,
    const SfxItemSet* pAttr,
    SdrModel* pModel,
    const SdrView* /* pSdrView */
)
    : SfxTabDialog( pParent,
                  "AreaDialog",
                  "cui/ui/areadialog.ui",
                  pAttr )
    , m_nAreaTabPage(0)
    , m_nShadowTabPage(0)
    , m_nTransparenceTabPage(0)
    , m_nColorTabPage(0)
    , m_nGradientTabPage(0)
    , m_nHatchTabPage(0)
    , m_nBitmapTabPage(0)

    , mpDrawModel          ( pModel ),
    mpColorList          ( pModel->GetColorList() ),
    mpNewColorList       ( pModel->GetColorList() ),
    mpGradientList       ( pModel->GetGradientList() ),
    mpNewGradientList    ( pModel->GetGradientList() ),
    mpHatchingList       ( pModel->GetHatchList() ),
    mpNewHatchingList    ( pModel->GetHatchList() ),
    mpBitmapList         ( pModel->GetBitmapList() ),
    mpNewBitmapList      ( pModel->GetBitmapList() ),
    mrOutAttrs           ( *pAttr ),

    mnColorListState ( CT_NONE ),
    mnBitmapListState ( CT_NONE ),
    mnGradientListState ( CT_NONE ),
    mnHatchingListState ( CT_NONE ),
    mnPageType( PT_AREA ),
    mnDlgType( 0 ),
    mbAreaTP( sal_False )
{
    m_nAreaTabPage = AddTabPage( "RID_SVXPAGE_AREA", SvxAreaTabPage::Create, 0 );
    m_nShadowTabPage = AddTabPage( "RID_SVXPAGE_SHADOW", SvxShadowTabPage::Create, 0 );
    m_nTransparenceTabPage = AddTabPage( "RID_SVXPAGE_TRANSPARENCE", SvxTransparenceTabPage::Create,  0);
    m_nColorTabPage = AddTabPage( "RID_SVXPAGE_COLOR", SvxColorTabPage::Create, 0 );
    m_nGradientTabPage = AddTabPage( "RID_SVXPAGE_GRADIENT", SvxGradientTabPage::Create, 0 );
    m_nHatchTabPage = AddTabPage( "RID_SVXPAGE_HATCH", SvxHatchTabPage::Create, 0 );
    m_nBitmapTabPage = AddTabPage( "RID_SVXPAGE_BITMAP", SvxBitmapTabPage::Create,  0);

    SetCurPageId( "RID_SVXPAGE_AREA" );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxAreaTabDialog, CancelHdlImpl ) );
}

// -----------------------------------------------------------------------

SvxAreaTabDialog::~SvxAreaTabDialog()
{
}


// -----------------------------------------------------------------------

void SvxAreaTabDialog::SavePalettes()
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if( mpNewColorList != mpDrawModel->GetColorList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewColorList.get()) );
        SvxColorListItem aColorListItem( mpNewColorList, SID_COLOR_TABLE );
        if ( pShell )
            pShell->PutItem( aColorListItem );
        else
            mpDrawModel->GetItemPool().Put(aColorListItem,SID_COLOR_TABLE);
        mpColorList = mpDrawModel->GetColorList();
    }
    if( mpNewGradientList != mpDrawModel->GetGradientList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewGradientList.get()) );
        SvxGradientListItem aItem( mpNewGradientList, SID_GRADIENT_LIST );
        if ( pShell )
            pShell->PutItem( aItem );
        else
            mpDrawModel->GetItemPool().Put(aItem,SID_GRADIENT_LIST);
        mpGradientList = mpDrawModel->GetGradientList();
    }
    if( mpNewHatchingList != mpDrawModel->GetHatchList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewHatchingList.get()) );
        SvxHatchListItem aItem( mpNewHatchingList, SID_HATCH_LIST );
        if ( pShell )
            pShell->PutItem( aItem );
        else
            mpDrawModel->GetItemPool().Put(aItem,SID_HATCH_LIST);
        mpHatchingList = mpDrawModel->GetHatchList();
    }
    if( mpNewBitmapList != mpDrawModel->GetBitmapList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewBitmapList.get()) );
        SvxBitmapListItem aItem( mpNewBitmapList, SID_BITMAP_LIST );
        if ( pShell )
            pShell->PutItem( aItem );
        else
            mpDrawModel->GetItemPool().Put(aItem,SID_BITMAP_LIST);
        mpBitmapList = mpDrawModel->GetBitmapList();
    }

    // save the tables when they have been changed

    const OUString aPath( SvtPathOptions().GetPalettePath() );

    if( mnHatchingListState & CT_MODIFIED )
    {
        mpHatchingList->SetPath( aPath );
        mpHatchingList->Save();

        SvxHatchListItem aItem( mpHatchingList, SID_HATCH_LIST );
        // ToolBoxControls are informed:
        if ( pShell )
            pShell->PutItem( aItem );
        else
            mpDrawModel->GetItemPool().Put(aItem);
    }

    if( mnBitmapListState & CT_MODIFIED )
    {
        mpBitmapList->SetPath( aPath );
        mpBitmapList->Save();

        SvxBitmapListItem aItem( mpBitmapList, SID_BITMAP_LIST );
        // ToolBoxControls are informed:
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem);
        }
    }

    if( mnGradientListState & CT_MODIFIED )
    {
        mpGradientList->SetPath( aPath );
        mpGradientList->Save();

        SvxGradientListItem aItem( mpGradientList, SID_GRADIENT_LIST );
        // ToolBoxControls are informed:
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem);
        }
    }

    if( mnColorListState & CT_MODIFIED )
    {
        mpColorList->SetPath( aPath );
        mpColorList->Save();

        SvxColorListItem aItem( mpColorList, SID_COLOR_TABLE );
        // ToolBoxControls are informed:
        if ( pShell )
            pShell->PutItem( aItem );
        else
        {
            mpDrawModel->GetItemPool().Put(aItem);
        }
    }
}
// -----------------------------------------------------------------------

short SvxAreaTabDialog::Ok()
{
    SavePalettes();

    // RET_OK is returned, if at least one
    // TabPage returns sal_True in FillItemSet().
    // This happens by default at the moment.
    return( SfxTabDialog::Ok() );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxAreaTabDialog, CancelHdlImpl)
{
    SavePalettes();

    EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SvxAreaTabDialog, CancelHdlImpl, void *, p )

// -----------------------------------------------------------------------

void SvxAreaTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nAreaTabPage )
    {
            ( (SvxAreaTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( mpGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( mpHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( mpBitmapList );
            ( (SvxAreaTabPage&) rPage ).SetPageType( mnPageType );
            ( (SvxAreaTabPage&) rPage ).SetDlgType( mnDlgType );
            ( (SvxAreaTabPage&) rPage ).SetPos( mnPos );
            ( (SvxAreaTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &mnGradientListState );
            ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &mnHatchingListState );
            ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &mnBitmapListState );
            ( (SvxAreaTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxAreaTabPage&) rPage ).Construct();
            // ActivatePage() is not called the first time
            ( (SvxAreaTabPage&) rPage ).ActivatePage( mrOutAttrs );
    }
    else if (nId == m_nShadowTabPage)
    {
            ( (SvxShadowTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxShadowTabPage&) rPage ).SetPageType( mnPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( mnDlgType );
            ( (SvxShadowTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxShadowTabPage&) rPage ).Construct();
    }
    else if (nId == m_nGradientTabPage)
    {
            ( (SvxGradientTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxGradientTabPage&) rPage ).SetGradientList( mpGradientList );
            ( (SvxGradientTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxGradientTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxGradientTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxGradientTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxGradientTabPage&) rPage ).SetGrdChgd( &mnGradientListState );
            ( (SvxGradientTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxGradientTabPage&) rPage ).Construct();
    }
    else if (nId == m_nHatchTabPage)
    {
            ( (SvxHatchTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxHatchTabPage&) rPage ).SetHatchingList( mpHatchingList );
            ( (SvxHatchTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxHatchTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxHatchTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxHatchTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxHatchTabPage&) rPage ).SetHtchChgd( &mnHatchingListState );
            ( (SvxHatchTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxHatchTabPage&) rPage ).Construct();
    }
    else if (nId == m_nBitmapTabPage)
    {
            ( (SvxBitmapTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxBitmapTabPage&) rPage ).SetBitmapList( mpBitmapList );
            ( (SvxBitmapTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxBitmapTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxBitmapTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxBitmapTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxBitmapTabPage&) rPage ).SetBmpChgd( &mnBitmapListState );
            ( (SvxBitmapTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxBitmapTabPage&) rPage ).Construct();
    }
    else if (nId == m_nColorTabPage)
    {
            ( (SvxColorTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxColorTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxColorTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxColorTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxColorTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxColorTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxColorTabPage&) rPage ).Construct();
    }
    else if (nId == m_nTransparenceTabPage)
    {
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( mnPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( mnDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
