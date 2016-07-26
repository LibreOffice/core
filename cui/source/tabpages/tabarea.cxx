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
    , m_nColorTabPage(0)
    , m_nGradientTabPage(0)
    , m_nHatchTabPage(0)
    //, m_nBitmapTabPage(0)
    , m_nPatternTabPage(0)

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
    m_nColorTabPage = AddTabPage( "RID_SVXPAGE_COLOR", SvxColorTabPage::Create, nullptr );
    m_nGradientTabPage = AddTabPage( "RID_SVXPAGE_GRADIENT", SvxGradientTabPage::Create, nullptr );
    m_nHatchTabPage = AddTabPage( "RID_SVXPAGE_HATCH", SvxHatchTabPage::Create, nullptr );
    m_nBitmapTabPage = AddTabPage( "RID_SVXPAGE_BITMAP", SvxBitmapTabPage::Create, nullptr );
    m_nPatternTabPage = AddTabPage( "RID_SVXPAGE_PATTERN", SvxPatternTabPage::Create,  nullptr);

    SetCurPageId( "RID_SVXPAGE_AREA" );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxAreaTabDialog, CancelHdlImpl ) );
}

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
    if( mpNewPatternList != mpDrawModel->GetPatternList() )
    {
        mpDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewPatternList.get()) );
        SvxPatternListItem aItem( mpNewPatternList, SID_PATTERN_LIST );
        if( pShell )
            pShell->PutItem( aItem );
        else
            mpDrawModel->GetItemPool().Put(aItem,SID_PATTERN_LIST);
        mpPatternList = mpDrawModel->GetPatternList();
    }

    // save the tables when they have been changed

    OUString aPalettePath(SvtPathOptions().GetPalettePath());
    OUString aPath;
    sal_Int32 nIndex = 0;
    do
    {
        aPath = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    if( mnHatchingListState & ChangeType::MODIFIED )
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

    if( mnBitmapListState & ChangeType::MODIFIED )
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

    if( mnPatternListState & ChangeType::MODIFIED )
    {
        mpPatternList->SetPath( aPath );
        mpPatternList->Save();

        SvxPatternListItem aItem( mpPatternList, SID_PATTERN_LIST );
        // ToolBoxControls are informed:
        if( pShell )
            pShell->PutItem( aItem );
        else
            mpDrawModel->GetItemPool().Put(aItem);
    }

    if( mnGradientListState & ChangeType::MODIFIED )
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

    if (mnColorListState & ChangeType::MODIFIED && mpColorList.is())
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


short SvxAreaTabDialog::Ok()
{
    SavePalettes();

    // RET_OK is returned, if at least one
    // TabPage returns sal_True in FillItemSet().
    // This happens by default at the moment.
    return( SfxTabDialog::Ok() );
}


IMPL_LINK_NOARG_TYPED(SvxAreaTabDialog, CancelHdlImpl, Button*, void)
{
    SavePalettes();

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
    else if (nId == m_nGradientTabPage)
    {
            static_cast<SvxGradientTabPage&>(rPage).SetColorList( mpColorList );
            static_cast<SvxGradientTabPage&>(rPage).SetGradientList( mpGradientList );
            static_cast<SvxGradientTabPage&>(rPage).SetPageType( &mnPageType );
            static_cast<SvxGradientTabPage&>(rPage).SetDlgType( 0 );
            static_cast<SvxGradientTabPage&>(rPage).SetPos( &mnPos );
            static_cast<SvxGradientTabPage&>(rPage).SetAreaTP( &mbAreaTP );
            static_cast<SvxGradientTabPage&>(rPage).SetGrdChgd( &mnGradientListState );
            static_cast<SvxGradientTabPage&>(rPage).SetColorChgd( &mnColorListState );
            static_cast<SvxGradientTabPage&>(rPage).Construct();
    }
    else if (nId == m_nHatchTabPage)
    {
            static_cast<SvxHatchTabPage&>(rPage).SetColorList( mpColorList );
            static_cast<SvxHatchTabPage&>(rPage).SetHatchingList( mpHatchingList );
            static_cast<SvxHatchTabPage&>(rPage).SetPageType( &mnPageType );
            static_cast<SvxHatchTabPage&>(rPage).SetDlgType( 0 );
            static_cast<SvxHatchTabPage&>(rPage).SetPos( &mnPos );
            static_cast<SvxHatchTabPage&>(rPage).SetAreaTP( &mbAreaTP );
            static_cast<SvxHatchTabPage&>(rPage).SetHtchChgd( &mnHatchingListState );
            static_cast<SvxHatchTabPage&>(rPage).SetColorChgd( &mnColorListState );
            static_cast<SvxHatchTabPage&>(rPage).Construct();
    }
    else if (nId == m_nBitmapTabPage )
    {
            static_cast<SvxBitmapTabPage&>(rPage).SetBitmapList( mpBitmapList );
            static_cast<SvxBitmapTabPage&>(rPage).SetPageType( &mnPageType );
            static_cast<SvxBitmapTabPage&>(rPage).SetDlgType( 0 );
            static_cast<SvxBitmapTabPage&>(rPage).SetPos( &mnPos );
            static_cast<SvxBitmapTabPage&>(rPage).SetAreaTP( &mbAreaTP );
            static_cast<SvxBitmapTabPage&>(rPage).SetBmpChgd( &mnBitmapListState );
            static_cast<SvxBitmapTabPage&>(rPage).Construct();
    }
    else if (nId == m_nPatternTabPage)
    {
            static_cast<SvxPatternTabPage&>(rPage).SetColorList( mpColorList );
            static_cast<SvxPatternTabPage&>(rPage).SetPatternList( mpPatternList );
            static_cast<SvxPatternTabPage&>(rPage).SetPageType( &mnPageType );
            static_cast<SvxPatternTabPage&>(rPage).SetDlgType( 0 );
            static_cast<SvxPatternTabPage&>(rPage).SetPos( &mnPos );
            static_cast<SvxPatternTabPage&>(rPage).SetAreaTP( &mbAreaTP );
            static_cast<SvxPatternTabPage&>(rPage).SetPtrnChgd( &mnPatternListState );
            static_cast<SvxPatternTabPage&>(rPage).SetColorChgd( &mnColorListState );
            static_cast<SvxPatternTabPage&>(rPage).Construct();
    }
    else if (nId == m_nColorTabPage)
    {
            static_cast<SvxColorTabPage&>(rPage).SetColorList( mpColorList );
            static_cast<SvxColorTabPage&>(rPage).SetPageType( &mnPageType );
            static_cast<SvxColorTabPage&>(rPage).SetDlgType( 0 );
            static_cast<SvxColorTabPage&>(rPage).SetPos( &mnPos );
            static_cast<SvxColorTabPage&>(rPage).SetAreaTP( &mbAreaTP );
            static_cast<SvxColorTabPage&>(rPage).SetColorChgd( &mnColorListState );
            static_cast<SvxColorTabPage&>(rPage).Construct();
    }
    else if (nId == m_nTransparenceTabPage)
    {
            static_cast<SvxTransparenceTabPage&>(rPage).SetPageType( mnPageType );
            static_cast<SvxTransparenceTabPage&>(rPage).SetDlgType( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
