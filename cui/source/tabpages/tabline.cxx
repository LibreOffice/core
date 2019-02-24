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

#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dialogs.hrc>
#include <cuitabarea.hxx>
#include <cuitabline.hxx>
#include <dlgname.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>

SvxLineTabDialog::SvxLineTabDialog(weld::Window* pParent, const SfxItemSet* pAttr,
    SdrModel* pModel, const SdrObject* pSdrObj, bool bHasObj)
    : SfxTabDialogController(pParent, "cui/ui/linedialog.ui", "LineDialog", pAttr)
    , pDrawModel(pModel)
    , pObj(pSdrObj)
    , pColorList(pModel->GetColorList())
    , mpNewColorList(pModel->GetColorList())
    , pDashList(pModel->GetDashList())
    , pNewDashList(pModel->GetDashList())
    , pLineEndList(pModel->GetLineEndList())
    , pNewLineEndList(pModel->GetLineEndList())
    , bObjSelected(bHasObj)
    , nLineEndListState(ChangeType::NONE)
    , nDashListState(ChangeType::NONE)
    , mnColorListState(ChangeType::NONE)
    , nPageType(PageType::Area) // We use it here primarily to get the right attributes with FillItemSet
    , nPosDashLb(0)
    , nPosLineEndLb(0)
{
    bool bLineOnly = false;
    if( pObj && pObj->GetObjInventor() == SdrInventor::Default )
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
            break;

        default:
            break;
        }

    }

    AddTabPage("RID_SVXPAGE_LINE", SvxLineTabPage::Create, nullptr);
    if( bLineOnly )
        AddTabPage("RID_SVXPAGE_SHADOW", SvxShadowTabPage::Create, nullptr);
    else
        RemoveTabPage( "RID_SVXPAGE_SHADOW" );

    AddTabPage("RID_SVXPAGE_LINE_DEF", SvxLineDefTabPage::Create, nullptr);
    AddTabPage("RID_SVXPAGE_LINEEND_DEF", SvxLineEndDefTabPage::Create, nullptr);

    weld::Button& rBtnCancel = GetCancelButton();
    rBtnCancel.connect_clicked(LINK(this, SvxLineTabDialog, CancelHdlImpl));
}

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
    OUString aPalettePath(SvtPathOptions().GetPalettePath());
    OUString aPath;
    sal_Int32 nIndex = 0;
    do
    {
        aPath = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    if( nDashListState & ChangeType::MODIFIED )
    {
        pDashList->SetPath( aPath );
        pDashList->Save();

        // Notify ToolBoxControls
        if ( pShell )
            pShell->PutItem( SvxDashListItem( pDashList, SID_DASH_LIST ) );
    }

    if( nLineEndListState & ChangeType::MODIFIED )
    {
        pLineEndList->SetPath( aPath );
        pLineEndList->Save();

        // Notify ToolBoxControls
        if ( pShell )
            pShell->PutItem( SvxLineEndListItem( pLineEndList, SID_LINEEND_LIST ) );
    }

    if( mnColorListState & ChangeType::MODIFIED )
    {
        pColorList->SetPath( aPath );
        pColorList->Save();

        // Notify ToolBoxControls
        if ( pShell )
            pShell->PutItem( SvxColorListItem( pColorList, SID_COLOR_TABLE ) );
    }
}

short SvxLineTabDialog::Ok()
{
    SavePalettes();

    // We return RET_OK if at least one TabPage in FillItemSet() returns sal_True.
    // We do this by default at the moment.
    return SfxTabDialogController::Ok();
}

IMPL_LINK_NOARG(SvxLineTabDialog, CancelHdlImpl, weld::Button&, void)
{
    SavePalettes();

    m_xDialog->response(RET_CANCEL);
}

void SvxLineTabDialog::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    if (rId == "RID_SVXPAGE_LINE")
    {
        static_cast<SvxLineTabPage&>(rPage).SetDashList( pDashList );
        static_cast<SvxLineTabPage&>(rPage).SetLineEndList( pLineEndList );
        static_cast<SvxLineTabPage&>(rPage).SetDlgType( 0 );
        static_cast<SvxLineTabPage&>(rPage).SetPageType( nPageType );
        static_cast<SvxLineTabPage&>(rPage).SetPosDashLb( &nPosDashLb );
        static_cast<SvxLineTabPage&>(rPage).SetPosLineEndLb( &nPosLineEndLb );
        static_cast<SvxLineTabPage&>(rPage).SetDashChgd( &nDashListState );
        static_cast<SvxLineTabPage&>(rPage).SetLineEndChgd( &nLineEndListState );
        static_cast<SvxLineTabPage&>(rPage).SetObjSelected( bObjSelected );
        static_cast<SvxLineTabPage&>(rPage).Construct();
        static_cast<SvxLineTabPage&>(rPage).SetColorChgd( &mnColorListState );
    }
    else if (rId == "RID_SVXPAGE_LINE_DEF")
    {
        static_cast<SvxLineDefTabPage&>(rPage).SetDashList( pDashList );
        static_cast<SvxLineDefTabPage&>(rPage).SetDlgType( 0 );
        static_cast<SvxLineDefTabPage&>(rPage).SetPageType( &nPageType );
        static_cast<SvxLineDefTabPage&>(rPage).SetPosDashLb( &nPosDashLb );
        static_cast<SvxLineDefTabPage&>(rPage).SetDashChgd( &nDashListState );
        static_cast<SvxLineDefTabPage&>(rPage).Construct();
    }
    else if (rId == "RID_SVXPAGE_LINEEND_DEF")
    {
        static_cast<SvxLineEndDefTabPage&>(rPage).SetLineEndList( pLineEndList );
        static_cast<SvxLineEndDefTabPage&>(rPage).SetPolyObj( pObj );
        static_cast<SvxLineEndDefTabPage&>(rPage).SetDlgType( 0 );
        static_cast<SvxLineEndDefTabPage&>(rPage).SetPageType( &nPageType );
        static_cast<SvxLineEndDefTabPage&>(rPage).SetPosLineEndLb( &nPosLineEndLb );
        static_cast<SvxLineEndDefTabPage&>(rPage).SetLineEndChgd( &nLineEndListState );
        static_cast<SvxLineEndDefTabPage&>(rPage).Construct();
    }
    else if (rId == "RID_SVXPAGE_SHADOW")
    {
        static_cast<SvxShadowTabPage&>(rPage).SetColorList( pColorList );
        static_cast<SvxShadowTabPage&>(rPage).SetPageType( nPageType );
        static_cast<SvxShadowTabPage&>(rPage).SetDlgType( 0 );
        static_cast<SvxShadowTabPage&>(rPage).SetColorChgd( &mnColorListState );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
