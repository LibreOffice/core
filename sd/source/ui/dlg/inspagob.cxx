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

#include "inspagob.hxx"

#include "strings.hrc"

#include "bitmaps.hlst"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"

SdInsertPagesObjsDlg::SdInsertPagesObjsDlg(
    vcl::Window* pWindow, const SdDrawDocument* pInDoc,
    SfxMedium* pSfxMedium, const OUString& rFileName )
    : ModalDialog(pWindow, "InsertSlidesDialog",
        "modules/sdraw/ui/insertslidesdialog.ui")
    , pMedium(pSfxMedium)
    , mpDoc(pInDoc)
    , rName(rFileName)
{
    get(m_pLbTree, "tree");
    get(m_pCbxMasters, "backgrounds");
    get(m_pCbxLink, "links");

    m_pLbTree->set_width_request(m_pLbTree->approximate_char_width() * 50);
    m_pLbTree->set_height_request(m_pLbTree->GetTextHeight() * 12);

    m_pLbTree->SetViewFrame( pInDoc->GetDocSh()->GetViewShell()->GetViewFrame() );

    m_pLbTree->SetSelectHdl( LINK( this, SdInsertPagesObjsDlg, SelectObjectHdl ) );

    // insert text
    if( !pMedium )
        SetText( SdResId( STR_INSERT_TEXT ) );

    Reset();
}

SdInsertPagesObjsDlg::~SdInsertPagesObjsDlg()
{
    disposeOnce();
}

void SdInsertPagesObjsDlg::dispose()
{
    m_pLbTree.clear();
    m_pCbxLink.clear();
    m_pCbxMasters.clear();
    ModalDialog::dispose();
}

/**
 * Fills the TreeLB dependent on the medium. Is not medium available, then
 * it is a text and not a draw document.
 */
void SdInsertPagesObjsDlg::Reset()
{
    if( pMedium )
    {
        m_pLbTree->SetSelectionMode( SelectionMode::Multiple );

        // transfer ownership of Medium
        m_pLbTree->Fill( mpDoc, pMedium, rName );
    }
    else
    {
        Color aColor( COL_WHITE );
        BitmapEx aBmpText(BMP_DOC_TEXT);
        //to-do, check if this is already transparent and remove intermedia bitmapex if it is
        Image aImgText(BitmapEx(aBmpText.GetBitmap(), aColor));
        m_pLbTree->InsertEntry( rName, aImgText, aImgText );
    }

    m_pCbxMasters->Check();
}

std::vector<OUString> SdInsertPagesObjsDlg::GetList( const sal_uInt16 nType )
{
    // With Draw documents, we have to return NULL on selection of the document
    if( pMedium )
    {
        // to ensure that bookmarks are opened
        // (when the whole document is selected)
        m_pLbTree->GetBookmarkDoc();

        // If the document is selected (too) or nothing is selected,
        // the whole document is inserted (but not more!)
        if( m_pLbTree->GetSelectionCount() == 0 ||
            ( m_pLbTree->IsSelected( m_pLbTree->First() ) ) )
            return std::vector<OUString>();
    }

    return m_pLbTree->GetSelectEntryList( nType );
}

/**
 *  is link checked
 */
bool SdInsertPagesObjsDlg::IsLink()
{
    return( m_pCbxLink->IsChecked() );
}

/**
 *  is link checked
 */
bool SdInsertPagesObjsDlg::IsRemoveUnnessesaryMasterPages() const
{
    return( m_pCbxMasters->IsChecked() );
}

/**
 * Enabled and selects end-color-LB
 */
IMPL_LINK_NOARG(SdInsertPagesObjsDlg, SelectObjectHdl, SvTreeListBox*, void)
{
    if( m_pLbTree->IsLinkableSelected() )
        m_pCbxLink->Enable();
    else
        m_pCbxLink->Disable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
