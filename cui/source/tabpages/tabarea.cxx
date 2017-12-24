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

#include <svx/xtable.hxx>
#include <svx/globl3d.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>
#include <dlgname.hxx>
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
    mnHatchingListState ( ChangeType::NONE )
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
    return SfxTabDialog::Ok();
}


IMPL_LINK_NOARG(SvxAreaTabDialog, CancelHdlImpl, Button*, void)
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
            static_cast<SvxAreaTabPage&>(rPage).SetPatternList( mpPatternList );
            static_cast<SvxAreaTabPage&>(rPage).SetGrdChgd( &mnGradientListState );
            static_cast<SvxAreaTabPage&>(rPage).SetHtchChgd( &mnHatchingListState );
            static_cast<SvxAreaTabPage&>(rPage).SetBmpChgd( &mnBitmapListState );
            static_cast<SvxAreaTabPage&>(rPage).SetPtrnChgd( &mnPatternListState );
            static_cast<SvxAreaTabPage&>(rPage).SetColorChgd( &mnColorListState );
            // ActivatePge() is not called the first time
            static_cast<SvxAreaTabPage&>(rPage).ActivatePage( mrOutAttrs );
    }
    else if (nId == m_nShadowTabPage)
    {
            static_cast<SvxShadowTabPage&>(rPage).SetColorList( mpColorList );
            static_cast<SvxShadowTabPage&>(rPage).SetColorChgd( &mnColorListState );
    }
    else if (nId == m_nTransparenceTabPage)
    {
            static_cast<SvxTransparenceTabPage&>(rPage).SetPageType( PageType::Area );
            static_cast<SvxTransparenceTabPage&>(rPage).SetDlgType( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
