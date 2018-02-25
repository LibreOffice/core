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

#include <sfx2/docrepairdlg.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/undo.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/treelistentry.hxx>
#include <unotools/datetime.hxx>

static inline void lcl_ActionToBuffer(OUStringBuffer& aBuffer, SfxUndoAction* pAction,
                                      const OUString& sType, size_t nIndex, sal_Unicode aSep)
{
    aBuffer.append(sType)
        .append(aSep)
        .append(OUString::number(nIndex))
        .append(aSep)
        .append(pAction->GetComment())
        .append(aSep)
        .append(OUString::number(static_cast<sal_Int32>(pAction->GetViewShellId())))
        .append(aSep)
        .append(utl::toISO8601(pAction->GetDateTime().GetUNODateTime()));
}

SfxDocumentRepairDlg::SfxDocumentRepairDlg(vcl::Window* pParent, svl::IUndoManager* pUndoMngr)
    : ModalDialog(pParent, "DocumentRepair", "sfx/ui/documentrepairdialog.ui")
    , m_pUndoMngr(pUndoMngr)
{
    size_t nCount, itAction;
    long aTabs[] = { 5, 0, 60, 100, 320, 400 };
    const OUString sUndo(SvtResId(STR_UNDO));
    const OUString sRedo(SvtResId(STR_REDO));
    const sal_Unicode aSep = '\t';

    OUStringBuffer sItem;
    SfxUndoAction* pAction;
    SvTreeListEntry* pEntry;
    VclPtr<PushButton> pBtnJump;
    Size aSize(LogicToPixel(Size(290, 220), MapMode(MapUnit::MapAppFont)));

    SetUpdateMode(false);

    SvSimpleTableContainer* pTableContainer = get<SvSimpleTableContainer>("CONTAINER");
    pTableContainer->set_width_request(aSize.Width());
    pTableContainer->set_height_request(aSize.Height());

    sItem.append(SvtResId(STR_SVT_FILEVIEW_COLUMN_TYPE))
        .append(aSep)
        .append(SvtResId(STR_SVT_FILEVIEW_COLUMN_INDEX))
        .append(aSep)
        .append(SvtResId(STR_SVT_FILEVIEW_COLUMN_COMMENT))
        .append(aSep)
        .append(SvtResId(STR_SVT_FILEVIEW_COLUMN_VIEWID))
        .append(aSep)
        .append(SvtResId(STR_SVT_FILEVIEW_COLUMN_TIMESTAMP));

    m_pActions = VclPtr<SvSimpleTable>::Create(*pTableContainer);
    m_pActions->SetTabs(aTabs, MapUnit::MapPixel);
    m_pActions->InsertHeaderEntry(sItem.makeStringAndClear(), HEADERBAR_APPEND,
                                  HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER);
    m_pActions->SetDoubleClickHdl(LINK(this, SfxDocumentRepairDlg, OnDblClick));

    if (m_pUndoMngr)
    {
        nCount = pUndoMngr->GetUndoActionCount();
        for (itAction = 0; itAction < nCount; ++itAction)
        {
            pAction = pUndoMngr->GetUndoAction(itAction);
            if (pAction)
            {
                lcl_ActionToBuffer(sItem, pAction, sUndo, itAction, aSep);
                if ((pEntry = m_pActions->InsertEntryToColumn(sItem.makeStringAndClear())))
                    pEntry->SetUserData(reinterpret_cast<void*>(sal_uInt16(SID_UNDO)));
            }
        }

        nCount = pUndoMngr->GetRedoActionCount();
        for (itAction = 0; itAction < nCount; ++itAction)
        {
            pAction = pUndoMngr->GetRedoAction(nCount - itAction - 1);
            if (pAction)
            {
                lcl_ActionToBuffer(sItem, pAction, sRedo, itAction, aSep);
                if ((pEntry = m_pActions->InsertEntryToColumn(sItem.makeStringAndClear())))
                    pEntry->SetUserData(reinterpret_cast<void*>(sal_uInt16(SID_REDO)));
            }
        }
    }

    get(pBtnJump, "jump");
    pBtnJump->SetClickHdl(LINK(this, SfxDocumentRepairDlg, OnJumpClick));

    SetUpdateMode(true);
}

SfxDocumentRepairDlg::~SfxDocumentRepairDlg() { disposeOnce(); }

void SfxDocumentRepairDlg::dispose()
{
    m_pActions.disposeAndClear();
    ModalDialog::dispose();
}

void SfxDocumentRepairDlg::DoRepair()
{
    SvTreeListEntry* pEntry = m_pActions->FirstSelected();
    if (pEntry)
    {
        m_nType = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(pEntry->GetUserData()));
        m_nIndex = SvTabListBox::GetEntryText(pEntry, 1).toUInt32() + 1;
        EndDialog(RET_OK);
    }
    else
    {
        EndDialog(RET_NO);
    }
}

IMPL_LINK_NOARG(SfxDocumentRepairDlg, OnDblClick, SvTreeListBox*, bool)
{
    if (m_pActions->GetSelectionCount() > 0)
    {
        DoRepair();
    }
    return true;
}

IMPL_LINK_NOARG(SfxDocumentRepairDlg, OnJumpClick, Button*, void) { DoRepair(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
