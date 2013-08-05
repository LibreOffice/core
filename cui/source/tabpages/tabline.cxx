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

#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dialogs.hrc>

#include <cuires.hrc>

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

    SfxTabDialog    ( pParent
                      , "LineDialog"
                      , "cui/ui/linedialog.ui"
                      , pAttr ),
    m_nLineTabPage(0),
    m_nShadowTabPage(0),
    m_nStyleTabPage(0),
    m_nEndTabPage(0),
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

    m_nLineTabPage = AddTabPage( "RID_SVXPAGE_LINE", SvxLineTabPage::Create, 0);
    if( bLineOnly )
        m_nShadowTabPage = AddTabPage( "RID_SVXPAGE_SHADOW", SvxShadowTabPage::Create, 0 );
    else
        RemoveTabPage( "RID_SVXPAGE_SHADOW" );

    m_nStyleTabPage = AddTabPage( "RID_SVXPAGE_LINE_DEF", SvxLineDefTabPage::Create, 0);
    m_nEndTabPage = AddTabPage( "RID_SVXPAGE_LINEEND_DEF", SvxLineEndDefTabPage::Create, 0);

    SetCurPageId( "RID_SVXPAGE_LINE" );

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
    if( nId ==  m_nLineTabPage)
    {
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
    }
    else if(nId == m_nStyleTabPage)
    {
        ( (SvxLineDefTabPage&) rPage ).SetDashList( pDashList );
        ( (SvxLineDefTabPage&) rPage ).SetDlgType( &nDlgType );
        ( (SvxLineDefTabPage&) rPage ).SetPageType( &nPageType );
        ( (SvxLineDefTabPage&) rPage ).SetPosDashLb( &nPosDashLb );
        ( (SvxLineDefTabPage&) rPage ).SetDashChgd( &nDashListState );
        ( (SvxLineDefTabPage&) rPage ).SetObjSelected( bObjSelected );
        ( (SvxLineDefTabPage&) rPage ).Construct();
    }
    else if(nId == m_nEndTabPage)
    {
        ( (SvxLineEndDefTabPage&) rPage ).SetLineEndList( pLineEndList );
        ( (SvxLineEndDefTabPage&) rPage ).SetPolyObj( pObj );
        ( (SvxLineEndDefTabPage&) rPage ).SetDlgType( &nDlgType );
        ( (SvxLineEndDefTabPage&) rPage ).SetPageType( &nPageType );
        ( (SvxLineEndDefTabPage&) rPage ).SetPosLineEndLb( &nPosLineEndLb );
        ( (SvxLineEndDefTabPage&) rPage ).SetLineEndChgd( &nLineEndListState );
        ( (SvxLineEndDefTabPage&) rPage ).SetObjSelected( bObjSelected );
        ( (SvxLineEndDefTabPage&) rPage ).Construct();
    }
    else if (nId == m_nShadowTabPage)
    {
        ( (SvxShadowTabPage&) rPage ).SetColorList( pColorList );
        ( (SvxShadowTabPage&) rPage ).SetPageType( nPageType );
        ( (SvxShadowTabPage&) rPage ).SetDlgType( nDlgType );
        ( (SvxShadowTabPage&) rPage ).SetAreaTP( &mbAreaTP );
        ( (SvxShadowTabPage&) rPage ).SetColorChgd( &mnColorListState );
        ( (SvxShadowTabPage&) rPage ).Construct();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
