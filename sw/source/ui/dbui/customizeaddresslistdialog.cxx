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

#include <swtypes.hxx>
#include <customizeaddresslistdialog.hxx>
#include <createaddresslistdialog.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/msgbox.hxx>
#include <customizeaddresslistdialog.hrc>
#include <dbui.hrc>
#include <helpid.h>

SwCustomizeAddressListDialog::SwCustomizeAddressListDialog(
        Window* pParent, const SwCSVData& rOldData) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_CUSTOMIZE_ADDRESS_LIST)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aFieldsFT( this, SW_RES(    FT_FIELDS)),
    m_aFieldsLB( this, SW_RES(    LB_FIELDS)),
    m_aAddPB( this, SW_RES(       PB_ADD)),
    m_aDeletePB( this, SW_RES(    PB_DELETE)),
    m_aRenamePB( this, SW_RES(    PB_RENAME)),
    m_aUpPB( this, SW_RES(        PB_UP)),
    m_aDownPB( this, SW_RES(      PB_DOWN)),
    m_aSeparatorFL( this, SW_RES( FL_SEPARATOR)),
    m_aOK( this, SW_RES(          PB_OK)),
    m_aCancel( this, SW_RES(      PB_CANCEL)),
    m_aHelp( this, SW_RES(        PB_HELP)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_pNewData( new SwCSVData(rOldData))
{
    FreeResource();
    m_aFieldsLB.SetSelectHdl(LINK(this, SwCustomizeAddressListDialog, ListBoxSelectHdl_Impl));
    Link aAddRenameLk = LINK(this, SwCustomizeAddressListDialog, AddRenameHdl_Impl );
    m_aAddPB.SetClickHdl(aAddRenameLk);
    m_aRenamePB.SetClickHdl(aAddRenameLk);
    m_aDeletePB.SetClickHdl(LINK(this, SwCustomizeAddressListDialog, DeleteHdl_Impl ));
    Link aUpDownLk = LINK(this, SwCustomizeAddressListDialog, UpDownHdl_Impl);
    m_aUpPB.SetClickHdl(aUpDownLk);
    m_aDownPB.SetClickHdl(aUpDownLk);

    ::std::vector< ::rtl::OUString >::iterator aHeaderIter;

    for(aHeaderIter = m_pNewData->aDBColumnHeaders.begin();
                aHeaderIter != m_pNewData->aDBColumnHeaders.end(); ++aHeaderIter)
        m_aFieldsLB.InsertEntry(*aHeaderIter);

    m_aFieldsLB.SelectEntryPos(0);
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
    bool bRename = pButton == &m_aRenamePB;
    sal_uInt16 nPos = m_aFieldsLB.GetSelectEntryPos();
    if(nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    SwAddRenameEntryDialog* pDlg =
            new SwAddRenameEntryDialog(pButton, bRename, m_pNewData->aDBColumnHeaders);
    if(bRename)
    {
        String aTemp = m_aFieldsLB.GetEntry(nPos);
        pDlg->SetFieldName(aTemp);
    }
    if(RET_OK == pDlg->Execute())
    {
        String sNew = pDlg->GetFieldName();
        if(bRename)
        {
            m_pNewData->aDBColumnHeaders[nPos] = sNew;
            m_aFieldsLB.RemoveEntry(nPos);
        }
        else
        {
            if ( m_aFieldsLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
                ++nPos; // append the new entry behind the selected
            //add the new column
            m_pNewData->aDBColumnHeaders.insert(m_pNewData->aDBColumnHeaders.begin() + nPos, sNew);
            //add a new entry into all data arrays
            String sTemp;
            ::std::vector< ::std::vector< ::rtl::OUString > >::iterator aDataIter;
            for( aDataIter = m_pNewData->aDBData.begin(); aDataIter != m_pNewData->aDBData.end(); ++aDataIter)
                aDataIter->insert(aDataIter->begin() + nPos, sTemp);

        }

        m_aFieldsLB.InsertEntry(sNew, nPos);
        m_aFieldsLB.SelectEntryPos(nPos);
    }
    delete pDlg;
    UpdateButtons();
    return 0;
}

IMPL_LINK_NOARG(SwCustomizeAddressListDialog, DeleteHdl_Impl)
{
    sal_uInt16 nPos = m_aFieldsLB.GetSelectEntryPos();
    m_aFieldsLB.RemoveEntry(m_aFieldsLB.GetSelectEntryPos());
    m_aFieldsLB.SelectEntryPos(nPos > m_aFieldsLB.GetEntryCount() - 1 ? nPos - 1 : nPos);

    //remove the column
    m_pNewData->aDBColumnHeaders.erase(m_pNewData->aDBColumnHeaders.begin() + nPos);
    //remove the data
    ::std::vector< ::std::vector< ::rtl::OUString > >::iterator aDataIter;
    for( aDataIter = m_pNewData->aDBData.begin(); aDataIter != m_pNewData->aDBData.end(); ++aDataIter)
        aDataIter->erase(aDataIter->begin() + nPos);

    UpdateButtons();
    return 0;
}

IMPL_LINK(SwCustomizeAddressListDialog, UpDownHdl_Impl, PushButton*, pButton)
{
    sal_uInt16 nPos;
    sal_uInt16 nOldPos = nPos = m_aFieldsLB.GetSelectEntryPos();
    String aTemp = m_aFieldsLB.GetEntry(nPos);
    m_aFieldsLB.RemoveEntry( nPos );
    if(pButton == &m_aUpPB)
        --nPos;
    else
        ++nPos;
    m_aFieldsLB.InsertEntry(aTemp, nPos);
    m_aFieldsLB.SelectEntryPos(nPos);
    //align m_pNewData
    ::rtl::OUString sHeader = m_pNewData->aDBColumnHeaders[nOldPos];
    m_pNewData->aDBColumnHeaders.erase(m_pNewData->aDBColumnHeaders.begin() + nOldPos);
    m_pNewData->aDBColumnHeaders.insert(m_pNewData->aDBColumnHeaders.begin() + nPos, sHeader);
    ::std::vector< ::std::vector< ::rtl::OUString > >::iterator aDataIter;
    for( aDataIter = m_pNewData->aDBData.begin(); aDataIter != m_pNewData->aDBData.end(); ++aDataIter)
    {
        ::rtl::OUString sData = (*aDataIter)[nOldPos];
        aDataIter->erase(aDataIter->begin() + nOldPos);
        aDataIter->insert(aDataIter->begin() + nPos, sData);
    }

    UpdateButtons();
    return 0;
}

void SwCustomizeAddressListDialog::UpdateButtons()
{
    sal_uInt16 nPos = m_aFieldsLB.GetSelectEntryPos();
    sal_uInt16 nEntries = m_aFieldsLB.GetEntryCount();
    m_aUpPB.Enable(nPos > 0 && nEntries > 0);
    m_aDownPB.Enable(nPos < nEntries -1);
    m_aDeletePB.Enable(nEntries > 0);
    m_aRenamePB.Enable(nEntries > 0);
}

SwCSVData*    SwCustomizeAddressListDialog::GetNewData()
{
    return m_pNewData;
}

SwAddRenameEntryDialog::SwAddRenameEntryDialog(
        Window* pParent, bool bRename, const ::std::vector< ::rtl::OUString >& rCSVHeader) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_ADD_RENAME_ENTRY)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aFieldNameFT( this, SW_RES( FT_FIELDNAME)),
    m_aFieldNameED( this, SW_RES( ED_FIELDNAME)),
    m_aOK( this, SW_RES(          PB_OK)),
    m_aCancel( this, SW_RES(      PB_CANCEL)),
    m_aHelp( this, SW_RES(        PB_HELP)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_rCSVHeader(rCSVHeader)
{
    if(bRename)
        SetText(String(SW_RES(ST_RENAME_TITLE)));
    else
        m_aOK.SetText(String(SW_RES(ST_ADD_BUTTON)));
    FreeResource();
    m_aFieldNameED.SetModifyHdl(LINK(this, SwAddRenameEntryDialog, ModifyHdl_Impl));
    ModifyHdl_Impl( &m_aFieldNameED );
}

SwAddRenameEntryDialog::~SwAddRenameEntryDialog()
{
}

IMPL_LINK(SwAddRenameEntryDialog, ModifyHdl_Impl, Edit*, pEdit)
{
    ::rtl::OUString sEntry = pEdit->GetText();
    sal_Bool bFound = sEntry.isEmpty();

    if(!bFound)
    {
        ::std::vector< ::rtl::OUString >::const_iterator aHeaderIter;
        for(aHeaderIter = m_rCSVHeader.begin();
                    aHeaderIter != m_rCSVHeader.end();
                    ++aHeaderIter)
            if(*aHeaderIter == sEntry)
            {
                bFound = sal_True;
                break;
            }
    }
    m_aOK.Enable(!bFound);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
