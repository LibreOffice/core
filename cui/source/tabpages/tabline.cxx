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

#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TABLINE_CXX
#include <cuires.hrc>
#include "tabline.hrc"

#include "cuitabarea.hxx"
#include "cuitabline.hxx"
#include "dlgname.hxx"
#include <dialmgr.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"

SvxLineTabDialog::SvxLineTabDialog
(
    Window* pParent,
    const SfxItemSet* pAttr,
    SdrModel* pModel,
    const SdrObject* pSdrObj,
    sal_Bool bHasObj
) :

    SfxTabDialog    ( pParent, CUI_RES( RID_SVXDLG_LINE ), pAttr ),
    pDrawModel      ( pModel ),
    pObj            ( pSdrObj ),
    rOutAttrs       ( *pAttr ),
    pColorList      ( pModel->GetColorList() ),
    mpNewColorList  ( pModel->GetColorList() ),
    pDashList       ( pModel->GetDashList() ),
    pNewDashList    ( pModel->GetDashList() ),
    pLineEndList    ( pModel->GetLineEndList() ),
    pNewLineEndList ( pModel->GetLineEndList() ),
    bObjSelected    ( bHasObj ),
    nLineEndListState( CT_NONE ),
    nDashListState( CT_NONE ),
    mnColorListState( CT_NONE ),
    nPageType( 0 ), // We use it here primarily to get the right attributes with FillItemSet
    nDlgType( 0 ),
    nPosDashLb( 0 ),
    nPosLineEndLb( 0 ),
    mbAreaTP( sal_False )
{
    FreeResource();

    bool bLineOnly = false;
    if( pObj && pObj->GetObjInventor() == SdrInventor )
    {
        switch( pObj->GetObjIdentifier() )
        {
        case OBJ_LINE:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_FREELINE:
        case OBJ_MEASURE:
        case OBJ_EDGE:
            bLineOnly = true;

        default:
            break;
        }

    }

    AddTabPage( RID_SVXPAGE_LINE, SvxLineTabPage::Create, 0);
    if( bLineOnly )
        AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create, 0 );
    else
        RemoveTabPage( RID_SVXPAGE_SHADOW );

    AddTabPage( RID_SVXPAGE_LINE_DEF, SvxLineDefTabPage::Create, 0);
    AddTabPage( RID_SVXPAGE_LINEEND_DEF, SvxLineEndDefTabPage::Create, 0);

    SetCurPageId( RID_SVXPAGE_LINE );

    CancelButton& rBtnCancel = GetCancelButton();
    rBtnCancel.SetClickHdl( LINK( this, SvxLineTabDialog, CancelHdlImpl ) );
}

// -----------------------------------------------------------------------

SvxLineTabDialog::~SvxLineTabDialog()
{
}

// -----------------------------------------------------------------------

void SvxLineTabDialog::SavePalettes()
{
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if( mpNewColorList != pDrawModel->GetColorList() )
    {
        pDrawModel->SetPropertyList( static_cast<XPropertyList *>(mpNewColorList.get()) );
        if ( pShell )
            pShell->PutItem( SvxColorListItem( mpNewColorList, SID_COLOR_TABLE ) );
        pColorList = pDrawModel->GetColorList();
    }
    if( pNewDashList != pDrawModel->GetDashList() )
    {
        pDrawModel->SetPropertyList( static_cast<XPropertyList *>(pNewDashList.get()) );
        if ( pShell )
            pShell->PutItem( SvxDashListItem( pNewDashList, SID_DASH_LIST ) );
        pDashList = pDrawModel->GetDashList();
    }
    if( pNewLineEndList != pDrawModel->GetLineEndList() )
    {
        pDrawModel->SetPropertyList( static_cast<XPropertyList *>(pNewLineEndList.get()) );
        if ( pShell )
            pShell->PutItem( SvxLineEndListItem( pNewLineEndList, SID_LINEEND_LIST ) );
        pLineEndList = pDrawModel->GetLineEndList();
    }

    // Save the tables when they have been changed

    const String aPath( SvtPathOptions().GetPalettePath() );

    if( nDashListState & CT_MODIFIED )
    {
        pDashList->SetPath( aPath );
        pDashList->Save();

        // Notify ToolBoxControls
        if ( pShell )
            pShell->PutItem( SvxDashListItem( pDashList, SID_DASH_LIST ) );
    }

    if( nLineEndListState & CT_MODIFIED )
    {
        pLineEndList->SetPath( aPath );
        pLineEndList->Save();

        // Notify ToolBoxControls
        if ( pShell )
            pShell->PutItem( SvxLineEndListItem( pLineEndList, SID_LINEEND_LIST ) );
    }

    if( mnColorListState & CT_MODIFIED )
    {
        pColorList->SetPath( aPath );
        pColorList->Save();

        // Notify ToolBoxControls
        if ( pShell )
            pShell->PutItem( SvxColorListItem( pColorList, SID_COLOR_TABLE ) );
    }
}

// -----------------------------------------------------------------------

short SvxLineTabDialog::Ok()
{
    SavePalettes();

    // We return RET_OK if at least one TabPage in FillItemSet() returns sal_True.
    // We do this by default at the moment.
    return( SfxTabDialog::Ok() );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxLineTabDialog, CancelHdlImpl)
{
    SavePalettes();

    EndDialog( RET_CANCEL );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxLineTabDialog, CancelHdlImpl)

// -----------------------------------------------------------------------

void SvxLineTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
            ( (SvxLineTabPage&) rPage ).SetColorList( pColorList );
            ( (SvxLineTabPage&) rPage ).SetDashList( pDashList );
            ( (SvxLineTabPage&) rPage ).SetLineEndList( pLineEndList );
            ( (SvxLineTabPage&) rPage ).SetDlgType( nDlgType );
            ( (SvxLineTabPage&) rPage ).SetPageType( nPageType );
            ( (SvxLineTabPage&) rPage ).SetPosDashLb( &nPosDashLb );
            ( (SvxLineTabPage&) rPage ).SetPosLineEndLb( &nPosLineEndLb );
            ( (SvxLineTabPage&) rPage ).SetDashChgd( &nDashListState );
            ( (SvxLineTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
            ( (SvxLineTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineTabPage&) rPage ).Construct();
            ( (SvxLineTabPage&) rPage ).SetColorChgd( &mnColorListState );
            // ActivatePage() is not called the first time
            ( (SvxLineTabPage&) rPage ).ActivatePage( rOutAttrs );
        break;

        case RID_SVXPAGE_LINE_DEF:
            ( (SvxLineDefTabPage&) rPage ).SetDashList( pDashList );
            ( (SvxLineDefTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineDefTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineDefTabPage&) rPage ).SetPosDashLb( &nPosDashLb );
            ( (SvxLineDefTabPage&) rPage ).SetDashChgd( &nDashListState );
            ( (SvxLineDefTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineDefTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_LINEEND_DEF:
            ( (SvxLineEndDefTabPage&) rPage ).SetLineEndList( pLineEndList );
            ( (SvxLineEndDefTabPage&) rPage ).SetPolyObj( pObj );
            ( (SvxLineEndDefTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxLineEndDefTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineEndDefTabPage&) rPage ).SetPosLineEndLb( &nPosLineEndLb );
            ( (SvxLineEndDefTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
            ( (SvxLineEndDefTabPage&) rPage ).SetObjSelected( bObjSelected );
            ( (SvxLineEndDefTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_SHADOW:
        {
            ( (SvxShadowTabPage&) rPage ).SetColorList( pColorList );
            ( (SvxShadowTabPage&) rPage ).SetPageType( nPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( nDlgType );
            ( (SvxShadowTabPage&) rPage ).SetAreaTP( &mbAreaTP );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &mnColorListState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        }
        break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
