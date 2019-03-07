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

#undef SC_DLLIMPLEMENTATION

#include <mvtabdlg.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

ScMoveTableDlg::ScMoveTableDlg(weld::Window* pParent, const OUString& rDefault)
    : GenericDialogController(pParent, "modules/scalc/ui/movecopysheet.ui", "MoveCopySheetDialog")
    , maDefaultName(rDefault)
    , mnCurrentDocPos(0)
    , nDocument(0)
    , nTable(0)
    , bCopyTable(false)
    , bRenameTable(false)
    , mbEverEdited(false)
    , m_xBtnMove(m_xBuilder->weld_radio_button("move"))
    , m_xBtnCopy(m_xBuilder->weld_radio_button("copy"))
    , m_xLbDoc(m_xBuilder->weld_combo_box("toDocument"))
    , m_xLbTable(m_xBuilder->weld_tree_view("insertBefore"))
    , m_xEdTabName(m_xBuilder->weld_entry("newName"))
    , m_xFtWarn(m_xBuilder->weld_label("newNameWarn"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xUnusedLabel(m_xBuilder->weld_label("warnunused"))
    , m_xEmptyLabel(m_xBuilder->weld_label("warnempty"))
    , m_xInvalidLabel(m_xBuilder->weld_label("warninvalid"))
{
    assert(m_xLbDoc->get_count() == 2);
    msCurrentDoc = m_xLbDoc->get_text(0);
    msNewDoc = m_xLbDoc->get_text(1);
    m_xLbDoc->clear();
    assert(m_xLbDoc->get_count() == 0);

    m_xLbTable->set_size_request(-1, m_xLbTable->get_height_rows(8));

    msStrTabNameUsed = m_xUnusedLabel->get_label();
    msStrTabNameEmpty = m_xEmptyLabel->get_label();
    msStrTabNameInvalid = m_xInvalidLabel->get_label();

    Init();
}

ScMoveTableDlg::~ScMoveTableDlg()
{
}

void ScMoveTableDlg::GetTabNameString( OUString& rString ) const
{
    rString = m_xEdTabName->get_text();
}

void ScMoveTableDlg::SetForceCopyTable()
{
    m_xBtnCopy->set_active(true);
    m_xBtnMove->set_sensitive(false);
    m_xBtnCopy->set_sensitive(false);
}

void ScMoveTableDlg::EnableRenameTable(bool bFlag)
{
    bRenameTable = bFlag;
    m_xEdTabName->set_sensitive(bFlag);
    ResetRenameInput();
}

void ScMoveTableDlg::ResetRenameInput()
{
    if (mbEverEdited)
    {
        // Don't reset the name when the sheet name has ever been edited.
        // But check the name, as this is also called for change of copy/move
        // buttons and document listbox selection.
        CheckNewTabName();
        return;
    }

    if (!m_xEdTabName->get_sensitive())
    {
        m_xEdTabName->set_text(OUString());
        return;
    }

    bool bVal = m_xBtnCopy->get_active();
    if (bVal)
    {
        // copy
        ScDocument* pDoc = GetSelectedDoc();
        if (pDoc)
        {
            OUString aStr = maDefaultName;
            pDoc->CreateValidTabName(aStr);
            m_xEdTabName->set_text(aStr);
        }
        else
            m_xEdTabName->set_text(maDefaultName);
    }
    else
    {
        // move
        m_xEdTabName->set_text(maDefaultName);
    }

    CheckNewTabName();
}

void ScMoveTableDlg::CheckNewTabName()
{
    const OUString aNewName = m_xEdTabName->get_text();
    if (aNewName.isEmpty())
    {
        // New sheet name is empty.  This is not good.
        m_xFtWarn->show();
//TODO        m_xFtWarn->SetControlBackground(COL_YELLOW);
        m_xFtWarn->set_label(msStrTabNameEmpty);
        m_xBtnOk->set_sensitive(false);
        return;
    }

    if (!ScDocument::ValidTabName(aNewName))
    {
        // New sheet name contains invalid characters.
        m_xFtWarn->show();
//TODO        m_xFtWarn->SetControlBackground(COL_YELLOW);
        m_xFtWarn->set_label(msStrTabNameInvalid);
        m_xBtnOk->set_sensitive(false);
        return;
    }

    bool bMoveInCurrentDoc = m_xBtnMove->get_active() && m_xLbDoc->get_active() == mnCurrentDocPos;
    bool bFound = false;
    const int nLast = m_xLbTable->n_children();
    for (int i = 0; i<nLast && !bFound; ++i)
    {
        if (aNewName == m_xLbTable->get_text(i))
        {
            // Only for move within same document the same name is allowed.
            if (!bMoveInCurrentDoc || maDefaultName != m_xEdTabName->get_text())
                bFound = true;
        }
    }

    if ( bFound )
    {
        m_xFtWarn->show();
//TODO        m_xFtWarn->SetControlBackground(COL_YELLOW);
        m_xFtWarn->set_label(msStrTabNameUsed);
        m_xBtnOk->set_sensitive(false);
    }
    else
    {
        m_xFtWarn->hide();
//TODO        m_xFtWarn->SetControlBackground();
        m_xFtWarn->set_label(OUString());
        m_xBtnOk->set_sensitive(true);
    }
}

ScDocument* ScMoveTableDlg::GetSelectedDoc()
{
    return reinterpret_cast<ScDocument*>(m_xLbDoc->get_active_id().toUInt64());
}

void ScMoveTableDlg::Init()
{
    m_xBtnOk->connect_clicked(LINK(this, ScMoveTableDlg, OkHdl));
    m_xLbDoc->connect_changed(LINK(this, ScMoveTableDlg, SelHdl));
    m_xBtnCopy->connect_toggled(LINK( this, ScMoveTableDlg, CheckBtnHdl));
    m_xEdTabName->connect_changed(LINK( this, ScMoveTableDlg, CheckNameHdl));
    m_xBtnMove->set_active(true);
    m_xBtnCopy->set_active(false);
    m_xEdTabName->set_sensitive(false);
    m_xFtWarn->hide();
    InitDocListBox();
    SelHdl(*m_xLbDoc);
}

void ScMoveTableDlg::InitDocListBox()
{
    SfxObjectShell* pSh     = SfxObjectShell::GetFirst();
    ScDocShell*     pScSh   = nullptr;
    sal_uInt16          nSelPos = 0;
    sal_uInt16          i       = 0;

    m_xLbDoc->clear();
    m_xLbDoc->freeze();

    while ( pSh )
    {
        pScSh = dynamic_cast<ScDocShell*>( pSh  );

        if ( pScSh )
        {
            OUString aEntryName = pScSh->GetTitle();

            if ( pScSh == SfxObjectShell::Current() )
            {
                mnCurrentDocPos = nSelPos = i;
                aEntryName += " ";
                aEntryName += msCurrentDoc;
            }

            OUString sId(OUString::number(reinterpret_cast<sal_uInt64>(&pScSh->GetDocument())));
            m_xLbDoc->insert(i, aEntryName, &sId, nullptr, nullptr);

            i++;
        }
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    m_xLbDoc->thaw();
    m_xLbDoc->append_text(msNewDoc);
    m_xLbDoc->set_active(nSelPos);
}

// Handler:

IMPL_LINK( ScMoveTableDlg, CheckBtnHdl, weld::ToggleButton&, rBtn, void )
{
    if (&rBtn == m_xBtnCopy.get())
        ResetRenameInput();
}

IMPL_LINK_NOARG(ScMoveTableDlg, OkHdl, weld::Button&, void)
{
    const sal_Int32 nDocSel  = m_xLbDoc->get_active();
    const sal_Int32 nDocLast = m_xLbDoc->get_count() - 1;
    const sal_Int32 nTabSel  = m_xLbTable->get_selected_index();
    const sal_Int32 nTabLast = m_xLbTable->n_children() - 1;

    nDocument   = (nDocSel != nDocLast) ? nDocSel : SC_DOC_NEW;
    nTable      = (nTabSel != nTabLast) ? static_cast<SCTAB>(nTabSel) : SC_TAB_APPEND;
    bCopyTable  = m_xBtnCopy->get_active();

    if (bCopyTable)
    {
        // Return an empty string when the new name is the same as the
        // automatic name assigned by the document.
        OUString aCopyName = maDefaultName;
        ScDocument* pDoc = GetSelectedDoc();
        if (pDoc)
            pDoc->CreateValidTabName(aCopyName);
        if (aCopyName == m_xEdTabName->get_text())
            m_xEdTabName->set_text(OUString());
    }
    else
    {
        // Return an empty string, when the new name is the same as the
        // original name.
        if (maDefaultName == m_xEdTabName->get_text())
            m_xEdTabName->set_text(OUString());
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ScMoveTableDlg, SelHdl, weld::ComboBox&, void)
{
    ScDocument* pDoc = GetSelectedDoc();
    OUString aName;

    m_xLbTable->clear();
    m_xLbTable->freeze();
    if ( pDoc )
    {
        SCTAB nLast = pDoc->GetTableCount()-1;
        for (SCTAB i = 0; i <= nLast; ++i)
        {
            pDoc->GetName(i, aName);
            m_xLbTable->append_text(aName);
        }
    }
    m_xLbTable->append_text(ScResId(STR_MOVE_TO_END));
    m_xLbTable->thaw();
    m_xLbTable->select(0);
    ResetRenameInput();
}

IMPL_LINK_NOARG(ScMoveTableDlg, CheckNameHdl, weld::Entry&, void)
{
    mbEverEdited = true;
    CheckNewTabName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
