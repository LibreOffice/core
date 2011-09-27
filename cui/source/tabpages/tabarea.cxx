/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <tools/ref.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TABAREA_CXX

#include <cuires.hrc>
#include <svx/xtable.hxx>
#include "svx/globl3d.hxx"
#include <svx/svdmodel.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "tabarea.hrc"
#include "dlgname.hxx"
#include <dialmgr.hxx>

SvxAreaTabDialog::SvxAreaTabDialog
(
    Window* pParent,
    const SfxItemSet* pAttr,
    SdrModel* pModel,
    const SdrView* /* pSdrView */
) :

    SfxTabDialog( pParent, CUI_RES( RID_SVXDLG_AREA ), pAttr ),

    mpDrawModel          ( pModel ),
    mpColorList           ( pModel->GetColorList() ),
    mpNewColorList        ( pModel->GetColorList() ),
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
    mnPos( 0 ),
    mbAreaTP( sal_False )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create,  0);
    AddTabPage( RID_SVXPAGE_COLOR, SvxColorTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_GRADIENT, SvxGradientTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_HATCH, SvxHatchTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BITMAP, SvxBitmapTabPage::Create,  0);

    SetCurPageId( RID_SVXPAGE_AREA );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxAreaTabDialog, CancelHdlImpl ) );
//! rBtnCancel.SetText( CUI_RESSTR( RID_SVXSTR_CLOSE ) );
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

    // Speichern der Tabellen, wenn sie geaendert wurden.

    const String aPath( SvtPathOptions().GetPalettePath() );

    if( mnHatchingListState & CT_MODIFIED )
    {
        mpHatchingList->SetPath( aPath );
        mpHatchingList->Save();

        SvxHatchListItem aItem( mpHatchingList, SID_HATCH_LIST );
        // ToolBoxControls werden benachrichtigt:
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
        // ToolBoxControls werden benachrichtigt:
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
        // ToolBoxControls werden benachrichtigt:
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
        // ToolBoxControls werden benachrichtigt:
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

    // Es wird RET_OK zurueckgeliefert, wenn wenigstens eine
    // TabPage in FillItemSet() sal_True zurueckliefert. Dieses
    // geschieht z.Z. standardmaessig.
    return( SfxTabDialog::Ok() );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxAreaTabDialog, CancelHdlImpl, void *, EMPTYARG)
{
    SavePalettes();

    EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_INLINE_END( SvxAreaTabDialog, CancelHdlImpl, void *, p )

// -----------------------------------------------------------------------

void SvxAreaTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_AREA:
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
            // ActivatePage() wird das erste mal nicht gerufen
            ( (SvxAreaTabPage&) rPage ).ActivatePage( mrOutAttrs );

        break;

        case RID_SVXPAGE_SHADOW:
        {
            ( (SvxShadowTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxShadowTabPage&) rPage ).SetPageType( mnPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( mnDlgType );
            ( (SvxShadowTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_GRADIENT:
            ( (SvxGradientTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxGradientTabPage&) rPage ).SetGradientList( mpGradientList );
            ( (SvxGradientTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxGradientTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxGradientTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxGradientTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxGradientTabPage&) rPage ).SetGrdChgd( &mnGradientListState );
            ( (SvxGradientTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxGradientTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_HATCH:
            ( (SvxHatchTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxHatchTabPage&) rPage ).SetHatchingList( mpHatchingList );
            ( (SvxHatchTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxHatchTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxHatchTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxHatchTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxHatchTabPage&) rPage ).SetHtchChgd( &mnHatchingListState );
            ( (SvxHatchTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxHatchTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_BITMAP:
            ( (SvxBitmapTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxBitmapTabPage&) rPage ).SetBitmapList( mpBitmapList );
            ( (SvxBitmapTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxBitmapTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxBitmapTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxBitmapTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxBitmapTabPage&) rPage ).SetBmpChgd( &mnBitmapListState );
            ( (SvxBitmapTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxBitmapTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_COLOR:
            ( (SvxColorTabPage&) rPage ).SetColorList( mpColorList );
            ( (SvxColorTabPage&) rPage ).SetPageType( &mnPageType );
            ( (SvxColorTabPage&) rPage ).SetDlgType( &mnDlgType );
            ( (SvxColorTabPage&) rPage ).SetPos( &mnPos );
            ( (SvxColorTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxColorTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxColorTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_TRANSPARENCE:
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( mnPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( mnDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
        break;

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
