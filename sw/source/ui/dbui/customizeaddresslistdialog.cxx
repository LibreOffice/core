/*************************************************************************
 *
 *  $RCSfile: customizeaddresslistdialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-29 09:30:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif
#pragma hdrstop

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _CUSTOMIZEADDRESSLISTDIALOG_HXX
#include <customizeaddresslistdialog.hxx>
#endif
#ifndef _CREATEADDRESSLISTDIALOG_HXX
#include <createaddresslistdialog.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#include <customizeaddresslistdialog.hrc>
#include <dbui.hrc>
#include <helpid.h>



/*-- 13.04.2004 14:27:21---------------------------------------------------

  -----------------------------------------------------------------------*/
SwCustomizeAddressListDialog::SwCustomizeAddressListDialog(
        Window* pParent, const SwCSVData& rOldData) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_CUSTOMIZE_ADDRESS_LIST)),
#pragma warning (disable : 4355)
    m_aFieldsFT( this, ResId(    FT_FIELDS)),
    m_aFieldsLB( this, ResId(    LB_FIELDS)),
    m_aAddPB( this, ResId(       PB_ADD)),
    m_aDeletePB( this, ResId(    PB_DELETE)),
    m_aRenamePB( this, ResId(    PB_RENAME)),
    m_aUpPB( this, ResId(        PB_UP)),
    m_aDownPB( this, ResId(      PB_DOWN)),
    m_aSeparatorFL( this, ResId( FL_SEPARATOR)),
    m_aOK( this, ResId(          PB_OK)),
    m_aCancel( this, ResId(      PB_CANCEL)),
    m_aHelp( this, ResId(        PB_HELP)),
#pragma warning (default : 4355)
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
/*-- 13.04.2004 14:34:07---------------------------------------------------

  -----------------------------------------------------------------------*/
SwCustomizeAddressListDialog::~SwCustomizeAddressListDialog()
{
}

/*-- 12.08.2004 12:58:00---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwCustomizeAddressListDialog, ListBoxSelectHdl_Impl, ListBox*, EMPTYARG)
{
    UpdateButtons();
    return 0;
}
/*-- 13.04.2004 15:02:14---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwCustomizeAddressListDialog, AddRenameHdl_Impl, PushButton*, pButton)
{
    bool bRename = pButton == &m_aRenamePB;
    USHORT nPos = m_aFieldsLB.GetSelectEntryPos();
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
            //add the new column
            sal_uInt32 nInsertPos = nPos + 1;
            m_pNewData->aDBColumnHeaders.insert(m_pNewData->aDBColumnHeaders.begin() + nInsertPos, sNew);
            //add a new entry into all data arrays
            String sTemp;
            ::std::vector< ::std::vector< ::rtl::OUString > >::iterator aDataIter;
            for( aDataIter = m_pNewData->aDBData.begin(); aDataIter != m_pNewData->aDBData.end(); ++aDataIter)
                aDataIter->insert(aDataIter->begin() + nInsertPos, sTemp);

        }
        m_aFieldsLB.InsertEntry(sNew, nPos);
        m_aFieldsLB.SelectEntryPos(nPos);
    }
    delete pDlg;
    UpdateButtons();
    return 0;
}
/*-- 13.04.2004 15:02:14---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwCustomizeAddressListDialog, DeleteHdl_Impl, PushButton*, pButton)
{
    USHORT nPos = m_aFieldsLB.GetSelectEntryPos();
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
/*-- 13.04.2004 15:02:15---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwCustomizeAddressListDialog, UpDownHdl_Impl, PushButton*, pButton)
{
    USHORT nPos;
    USHORT nOldPos = nPos = m_aFieldsLB.GetSelectEntryPos();
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
/*-- 19.04.2004 14:51:49---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwCustomizeAddressListDialog::UpdateButtons()
{
    USHORT nPos = m_aFieldsLB.GetSelectEntryPos();
    USHORT nEntries = m_aFieldsLB.GetEntryCount();
    m_aUpPB.Enable(nPos > 0 && nEntries > 0);
    m_aDownPB.Enable(nPos < nEntries -1);
    m_aDeletePB.Enable(nEntries > 0);
    m_aRenamePB.Enable(nEntries > 0);
}
/*-- 19.04.2004 14:51:49---------------------------------------------------

  -----------------------------------------------------------------------*/
SwCSVData*    SwCustomizeAddressListDialog::GetNewData()
{
    return m_pNewData;
}

/*-- 13.04.2004 13:48:41---------------------------------------------------

  -----------------------------------------------------------------------*/
SwAddRenameEntryDialog::SwAddRenameEntryDialog(
        Window* pParent, bool bRename, const ::std::vector< ::rtl::OUString >& rCSVHeader) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_ADD_RENAME_ENTRY)),
#pragma warning (disable : 4355)
    m_aFieldNameFT( this, ResId( FT_FIELDNAME)),
    m_aFieldNameED( this, ResId( ED_FIELDNAME)),
    m_aOK( this, ResId(          PB_OK)),
    m_aCancel( this, ResId(      PB_CANCEL)),
    m_aHelp( this, ResId(        PB_HELP)),
#pragma warning (default : 4355)
    m_rCSVHeader(rCSVHeader)
{
    if(bRename)
        SetText(String(ResId(ST_RENAME_TITLE)));
    else
        m_aOK.SetText(String(ResId(ST_ADD_BUTTON)));
    FreeResource();
    m_aFieldNameED.SetModifyHdl(LINK(this, SwAddRenameEntryDialog, ModifyHdl_Impl));
    ModifyHdl_Impl( &m_aFieldNameED );
}
/*-- 13.04.2004 13:48:41---------------------------------------------------

  -----------------------------------------------------------------------*/
SwAddRenameEntryDialog::~SwAddRenameEntryDialog()
{
}
/*-- 19.04.2004 15:31:34---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SwAddRenameEntryDialog, ModifyHdl_Impl, Edit*, pEdit)
{
    ::rtl::OUString sEntry = pEdit->GetText();
    BOOL bFound = sEntry.getLength() ? FALSE : TRUE;

    if(!bFound)
    {
        ::std::vector< ::rtl::OUString >::const_iterator aHeaderIter;
        for(aHeaderIter = m_rCSVHeader.begin();
                    aHeaderIter != m_rCSVHeader.end();
                    ++aHeaderIter)
            if(*aHeaderIter == sEntry)
            {
                bFound = TRUE;
                break;
            }
    }
    m_aOK.Enable(!bFound);
    return 0;
}
