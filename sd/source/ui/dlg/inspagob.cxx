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

#include <inspagob.hxx>
#include <sdtreelb.hxx>

#include <strings.hrc>

#include <bitmaps.hlst>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>

SdInsertPagesObjsDlg::SdInsertPagesObjsDlg(
    weld::Window* pWindow, const SdDrawDocument* pInDoc,
    SfxMedium* pSfxMedium, const OUString& rFileName )
    : GenericDialogController(pWindow, "modules/sdraw/ui/insertslidesdialog.ui", "InsertSlidesDialog")
    , m_pMedium(pSfxMedium)
    , m_pDoc(pInDoc)
    , m_rName(rFileName)
    , m_xLbTree(new SdPageObjsTLV(m_xBuilder->weld_tree_view("tree")))
    , m_xCbxLink(m_xBuilder->weld_check_button("links"))
    , m_xCbxMasters(m_xBuilder->weld_check_button("backgrounds"))
{
    m_xLbTree->set_size_request(m_xLbTree->get_approximate_digit_width() * 48,
                                m_xLbTree->get_height_rows(12));

    m_xLbTree->SetViewFrame( pInDoc->GetDocSh()->GetViewShell()->GetViewFrame() );

    m_xLbTree->connect_changed(LINK(this, SdInsertPagesObjsDlg, SelectObjectHdl));

    // insert text
    if (!m_pMedium)
        m_xDialog->set_title(SdResId(STR_INSERT_TEXT));

    Reset();
}

SdInsertPagesObjsDlg::~SdInsertPagesObjsDlg()
{
}

/**
 * Fills the TreeLB dependent on the medium. Is not medium available, then
 * it is a text and not a draw document.
 */
void SdInsertPagesObjsDlg::Reset()
{
    if( m_pMedium )
    {
        m_xLbTree->set_selection_mode(SelectionMode::Multiple);

        // transfer ownership of Medium
        m_xLbTree->Fill( m_pDoc, m_pMedium, m_rName );
    }
    else
    {
        OUString sImgText(BMP_DOC_TEXT);
        m_xLbTree->InsertEntry(m_rName, sImgText);
    }

    m_xCbxMasters->set_active(true);
}

std::vector<OUString> SdInsertPagesObjsDlg::GetList( const sal_uInt16 nType )
{
    // With Draw documents, we have to return NULL on selection of the document
    if( m_pMedium )
    {
        // to ensure that bookmarks are opened
        // (when the whole document is selected)
        m_xLbTree->GetBookmarkDoc();

        // If the document is selected (too) or nothing is selected,
        // the whole document is inserted (but not more!)
        std::unique_ptr<weld::TreeIter> xIter(m_xLbTree->make_iterator());
        if (!m_xLbTree->get_iter_first(*xIter) || m_xLbTree->is_selected(*xIter))
            return std::vector<OUString>();
    }

    return m_xLbTree->GetSelectEntryList( nType );
}

/**
 *  is link checked
 */
bool SdInsertPagesObjsDlg::IsLink()
{
    return m_xCbxLink->get_active();
}

/**
 *  is link checked
 */
bool SdInsertPagesObjsDlg::IsRemoveUnnessesaryMasterPages() const
{
    return m_xCbxMasters->get_active();
}

/**
 * Enabled and selects end-color-LB
 */
IMPL_LINK_NOARG(SdInsertPagesObjsDlg, SelectObjectHdl, weld::TreeView&, void)
{
    m_xCbxLink->set_sensitive(m_xLbTree->IsLinkableSelected());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
