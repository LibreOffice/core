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

#include <swtypes.hxx>
#include <customizeaddresslistdialog.hxx>
#include <createaddresslistdialog.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/msgbox.hxx>
#include <dbui.hrc>
#include <helpid.h>

SwCustomizeAddressListDialog::SwCustomizeAddressListDialog(
        Window* pParent, const SwCSVData& rOldData)
    : SfxModalDialog(pParent, "CustomizeAddrListDialog",
        "modules/swriter/ui/customizeaddrlistdialog.ui")
    , m_pNewData( new SwCSVData(rOldData))
{
    get(m_pFieldsLB, "treeview");
    m_pFieldsLB->SetDropDownLineCount(14);
    get(m_pAddPB, "add");
    get(m_pDeletePB, "delete");
    get(m_pRenamePB, "rename");
    get(m_pUpPB, "up");
    get(m_pDownPB, "down");

    m_pFieldsLB->SetSelectHdl(LINK(this, SwCustomizeAddressListDialog, ListBoxSelectHdl_Impl));
    Link aAddRenameLk = LINK(this, SwCustomizeAddressListDialog, AddRenameHdl_Impl );
    m_pAddPB->SetClickHdl(aAddRenameLk);
    m_pRenamePB->SetClickHdl(aAddRenameLk);
    m_pDeletePB->SetClickHdl(LINK(this, SwCustomizeAddressListDialog, DeleteHdl_Impl ));
    Link aUpDownLk = LINK(this, SwCustomizeAddressListDialog, UpDownHdl_Impl);
    m_pUpPB->SetClickHdl(aUpDownLk);
    m_pDownPB->SetClickHdl(aUpDownLk);

    std::vector< OUString >::iterator aHeaderIter;

    for(aHeaderIter = m_pNewData->aDBColumnHeaders.begin();
                aHeaderIter != m_pNewData->aDBColumnHeaders.end(); ++aHeaderIter)
        m_pFieldsLB->InsertEntry(*aHeaderIter);

    m_pFieldsLB->SelectEntryPos(0);
    UpdateButtons();
}

SwCustomizeAddressListDialog::~SwCustomizeAddressListDialog()
{
}

IMPL_LINK_NOARG(SwCustomizeAddressListDialog, ListBoxSelectHdl_Impl)
{
    UpdateButtons();
    return 0;
}

IMPL_LINK(SwCustomizeAddressListDialog, AddRenameHdl_Impl, PushButton*, pButton)
{
    bool bRename = pButton == m_pRenamePB;
    sal_uInt16 nPos = m_pFieldsLB->GetSelectEntryPos();
    if(nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    SwAddRenameEntryDialog* pDlg;
    if (bRename)
        pDlg = new SwRenameEntryDialog(pButton, m_pNewData->aDBColumnHeaders);
    else
        pDlg = new SwAddEntryDialog(pButton, m_pNewData->aDBColumnHeaders);
    if(bRename)
    {
        String aTemp = m_pFieldsLB->GetEntry(nPos);
        pDlg->SetFieldName(aTemp);
    }
    if(RET_OK == pDlg->Execute())
    {
        String sNew = pDlg->GetFieldName();
        if(bRename)
        {
            m_pNewData->aDBColumnHeaders[nPos] = sNew;
            m_pFieldsLB->RemoveEntry(nPos);
        }
        else
        {
            if ( m_pFieldsLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
                ++nPos; // append the new entry behind the selected
            //add the new column
            m_pNewData->aDBColumnHeaders.insert(m_pNewData->aDBColumnHeaders.begin() + nPos, sNew);
            //add a new entry into all data arrays
            String sTemp;
            std::vector< std::vector< OUString > >::iterator aDataIter;
            for( aDataIter = m_pNewData->aDBData.begin(); aDataIter != m_pNewData->aDBData.end(); ++aDataIter)
                aDataIter->insert(aDataIter->begin() + nPos, sTemp);

        }

        m_pFieldsLB->InsertEntry(sNew, nPos);
        m_pFieldsLB->SelectEntryPos(nPos);
    }
    delete pDlg;
    UpdateButtons();
    return 0;
}

IMPL_LINK_NOARG(SwCustomizeAddressListDialog, DeleteHdl_Impl)
{
    sal_uInt16 nPos = m_pFieldsLB->GetSelectEntryPos();
    m_pFieldsLB->RemoveEntry(m_pFieldsLB->GetSelectEntryPos());
    m_pFieldsLB->SelectEntryPos(nPos > m_pFieldsLB->GetEntryCount() - 1 ? nPos - 1 : nPos);

    //remove the column
    m_pNewData->aDBColumnHeaders.erase(m_pNewData->aDBColumnHeaders.begin() + nPos);
    //remove the data
    std::vector< std::vector< OUString > >::iterator aDataIter;
    for( aDataIter = m_pNewData->aDBData.begin(); aDataIter != m_pNewData->aDBData.end(); ++aDataIter)
        aDataIter->erase(aDataIter->begin() + nPos);

    UpdateButtons();
    return 0;
}

IMPL_LINK(SwCustomizeAddressListDialog, UpDownHdl_Impl, PushButton*, pButton)
{
    sal_uInt16 nPos;
    sal_uInt16 nOldPos = nPos = m_pFieldsLB->GetSelectEntryPos();
    String aTemp = m_pFieldsLB->GetEntry(nPos);
    m_pFieldsLB->RemoveEntry( nPos );
    if(pButton == m_pUpPB)
        --nPos;
    else
        ++nPos;
    m_pFieldsLB->InsertEntry(aTemp, nPos);
    m_pFieldsLB->SelectEntryPos(nPos);
    //align m_pNewData
    OUString sHeader = m_pNewData->aDBColumnHeaders[nOldPos];
    m_pNewData->aDBColumnHeaders.erase(m_pNewData->aDBColumnHeaders.begin() + nOldPos);
    m_pNewData->aDBColumnHeaders.insert(m_pNewData->aDBColumnHeaders.begin() + nPos, sHeader);
    std::vector< std::vector< OUString > >::iterator aDataIter;
    for( aDataIter = m_pNewData->aDBData.begin(); aDataIter != m_pNewData->aDBData.end(); ++aDataIter)
    {
        OUString sData = (*aDataIter)[nOldPos];
        aDataIter->erase(aDataIter->begin() + nOldPos);
        aDataIter->insert(aDataIter->begin() + nPos, sData);
    }

    UpdateButtons();
    return 0;
}

void SwCustomizeAddressListDialog::UpdateButtons()
{
    sal_uInt16 nPos = m_pFieldsLB->GetSelectEntryPos();
    sal_uInt16 nEntries = m_pFieldsLB->GetEntryCount();
    m_pUpPB->Enable(nPos > 0 && nEntries > 0);
    m_pDownPB->Enable(nPos < nEntries -1);
    m_pDeletePB->Enable(nEntries > 0);
    m_pRenamePB->Enable(nEntries > 0);
}

SwCSVData*    SwCustomizeAddressListDialog::GetNewData()
{
    return m_pNewData;
}

SwAddRenameEntryDialog::SwAddRenameEntryDialog(
        Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
        const std::vector< OUString >& rCSVHeader)
    : SfxModalDialog(pParent, rID, rUIXMLDescription)
    , m_rCSVHeader(rCSVHeader)
{
    get(m_pOK, "ok");
    get(m_pFieldNameED, "entry");
    m_pFieldNameED->SetModifyHdl(LINK(this, SwAddRenameEntryDialog, ModifyHdl_Impl));
    ModifyHdl_Impl(m_pFieldNameED);
}

IMPL_LINK(SwAddRenameEntryDialog, ModifyHdl_Impl, Edit*, pEdit)
{
    OUString sEntry = pEdit->GetText();
    sal_Bool bFound = sEntry.isEmpty();

    if(!bFound)
    {
        std::vector< OUString >::const_iterator aHeaderIter;
        for(aHeaderIter = m_rCSVHeader.begin();
                    aHeaderIter != m_rCSVHeader.end();
                    ++aHeaderIter)
            if(*aHeaderIter == sEntry)
            {
                bFound = sal_True;
                break;
            }
    }
    m_pOK->Enable(!bFound);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
