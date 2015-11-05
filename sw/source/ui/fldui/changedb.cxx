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

#include <svtools/stdctrl.hxx>
#include <vcl/msgbox.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/treelistentry.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <dbmgr.hxx>
#include <fldmgr.hxx>
#include <expfld.hxx>
#include <txtatr.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <dbfld.hxx>
#include <changedb.hxx>

#include <fldui.hrc>
#include <globals.hrc>
#include <utlui.hrc>

#include <unomid.h>

using namespace css::container;
using namespace css::lang;
using namespace css::sdb;
using namespace css::uno;

// edit insert-field
SwChangeDBDlg::SwChangeDBDlg(SwView& rVw)
    : SvxStandardDialog(&rVw.GetViewFrame()->GetWindow(), "ExchangeDatabasesDialog",
        "modules/swriter/ui/exchangedatabases.ui")
    , aImageList(SW_RES(ILIST_DB_DLG))
    , pSh(rVw.GetWrtShellPtr())
    , pMgr( new SwFieldMgr() )
{
    get(m_pUsedDBTLB, "inuselb");
    get(m_pAvailDBTLB, "availablelb");
    get(m_pAddDBPB, "browse");
    get(m_pDocDBNameFT, "dbnameft");
    get(m_pDefineBT, "define");
    m_pAvailDBTLB->SetWrtShell(*pSh);
    FillDBPopup();

    ShowDBName(pSh->GetDBData());
    m_pDefineBT->SetClickHdl(LINK(this, SwChangeDBDlg, ButtonHdl));
    m_pAddDBPB->SetClickHdl(LINK(this, SwChangeDBDlg, AddDBHdl));

    m_pUsedDBTLB->SetSelectionMode(MULTIPLE_SELECTION);
    m_pUsedDBTLB->SetStyle(m_pUsedDBTLB->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    m_pUsedDBTLB->SetSpaceBetweenEntries(0);
    m_pUsedDBTLB->SetNodeBitmaps( aImageList.GetImage(IMG_COLLAPSE), aImageList.GetImage(IMG_EXPAND));

    Link<SvTreeListBox*,void> aLink = LINK(this, SwChangeDBDlg, TreeSelectHdl);

    m_pUsedDBTLB->SetSelectHdl(aLink);
    m_pUsedDBTLB->SetDeselectHdl(aLink);
    m_pAvailDBTLB->SetSelectHdl(aLink);
    m_pAvailDBTLB->SetSelectHdl(aLink);
    TreeSelectHdl(NULL);
}

// initialise database listboxes
void SwChangeDBDlg::FillDBPopup()
{
    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference<XDatabaseContext> xDBContext = DatabaseContext::create(xContext);
    const SwDBData& rDBData = pSh->GetDBData();
    m_pAvailDBTLB->Select(rDBData.sDataSource, rDBData.sCommand, aEmptyOUStr);

    std::vector<OUString> aAllDBNames;

    Sequence< OUString > aDBNames = xDBContext->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    sal_Int32 nDBCount = aDBNames.getLength();
    for(sal_Int32 i = 0; i < nDBCount; i++)
    {
        aAllDBNames.push_back(pDBNames[i]);
    }

    std::vector<OUString> aDBNameList;
    pSh->GetAllUsedDB( aDBNameList, &aAllDBNames );

    size_t nCount = aDBNameList.size();
    m_pUsedDBTLB->Clear();
    SvTreeListEntry *pFirst = 0;
    SvTreeListEntry *pLast = 0;

    for(size_t k = 0; k < nCount; k++)
    {
        pLast = Insert(aDBNameList[k].getToken(0, ';'));
        if (!pFirst)
            pFirst = pLast;
    }

    if (pFirst)
    {
        m_pUsedDBTLB->MakeVisible(pFirst);
        m_pUsedDBTLB->Select(pFirst);
    }

}

SvTreeListEntry* SwChangeDBDlg::Insert(const OUString& rDBName)
{
    const OUString sDBName(rDBName.getToken(0, DB_DELIM));
    const OUString sTableName(rDBName.getToken(1, DB_DELIM));
    sal_IntPtr nCommandType = rDBName.getToken(2, DB_DELIM).toInt32();
    SvTreeListEntry* pParent;
    SvTreeListEntry* pChild;

    sal_uLong nParent = 0;
    sal_uLong nChild = 0;

    Image aTableImg = aImageList.GetImage(IMG_DBTABLE);
    Image aDBImg = aImageList.GetImage(IMG_DB);
    Image aQueryImg = aImageList.GetImage(IMG_DBQUERY);
    Image& rToInsert = nCommandType ? aQueryImg : aTableImg;
    while ((pParent = m_pUsedDBTLB->GetEntry(nParent++)) != NULL)
    {
        if (sDBName == m_pUsedDBTLB->GetEntryText(pParent))
        {
            while ((pChild = m_pUsedDBTLB->GetEntry(pParent, nChild++)) != NULL)
            {
                if (sTableName == m_pUsedDBTLB->GetEntryText(pChild))
                    return pChild;
            }
            SvTreeListEntry* pRet = m_pUsedDBTLB->InsertEntry(sTableName, rToInsert, rToInsert, pParent);
            pRet->SetUserData(reinterpret_cast<void*>(nCommandType));
            return pRet;
        }
    }
    pParent = m_pUsedDBTLB->InsertEntry(sDBName, aDBImg, aDBImg);

    SvTreeListEntry* pRet = m_pUsedDBTLB->InsertEntry(sTableName, rToInsert, rToInsert, pParent);
    pRet->SetUserData(reinterpret_cast<void*>(nCommandType));
    return pRet;
}

// destroy dialog
SwChangeDBDlg::~SwChangeDBDlg()
{
    disposeOnce();
}

void SwChangeDBDlg::dispose()
{
    delete pMgr;
    m_pUsedDBTLB.clear();
    m_pAvailDBTLB.clear();
    m_pAddDBPB.clear();
    m_pDocDBNameFT.clear();
    m_pDefineBT.clear();
    SvxStandardDialog::dispose();
}

// close
void SwChangeDBDlg::Apply()
{
    UpdateFields();
}
void SwChangeDBDlg::UpdateFields()
{
    std::vector<OUString> aDBNames;
    aDBNames.reserve(m_pUsedDBTLB->GetSelectionCount());
    SvTreeListEntry* pEntry = m_pUsedDBTLB->FirstSelected();

    while( pEntry )
    {
        if( m_pUsedDBTLB->GetParent( pEntry ))
        {
            OUString sTmp(m_pUsedDBTLB->GetEntryText( m_pUsedDBTLB->GetParent( pEntry )) +
                          OUString(DB_DELIM) + m_pUsedDBTLB->GetEntryText( pEntry ) + OUString(DB_DELIM) +
                          OUString::number((int)reinterpret_cast<sal_uLong>(pEntry->GetUserData())));
            aDBNames.push_back(sTmp);
        }
        pEntry = m_pUsedDBTLB->NextSelected(pEntry);
    }

    pSh->StartAllAction();
    OUString sTableName;
    OUString sColumnName;
    sal_Bool bIsTable = sal_False;
    const OUString DBName(m_pAvailDBTLB->GetDBName(sTableName, sColumnName, &bIsTable));
    const OUString sTemp = DBName
        + OUString(DB_DELIM)
        + sTableName
        + OUString(DB_DELIM)
        + OUString::number(bIsTable
                            ? CommandType::TABLE
                            : CommandType::QUERY);
    pSh->ChangeDBFields( aDBNames, sTemp);
    pSh->EndAllAction();
}

IMPL_LINK_NOARG_TYPED(SwChangeDBDlg, ButtonHdl, Button*, void)
{
    OUString sTableName;
    OUString sColumnName;
    SwDBData aData;
    sal_Bool bIsTable = sal_False;
    aData.sDataSource = m_pAvailDBTLB->GetDBName(sTableName, sColumnName, &bIsTable);
    aData.sCommand = sTableName;
    aData.nCommandType = bIsTable ? 0 : 1;
    pSh->ChgDBData(aData);
    ShowDBName(pSh->GetDBData());
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG_TYPED(SwChangeDBDlg, TreeSelectHdl, SvTreeListBox*, void)
{
    SvTreeListEntry* pEntry = m_pAvailDBTLB->GetCurEntry();

    if (pEntry)
    {
        bool bEnable = false;
        if (m_pAvailDBTLB->GetParent(pEntry))
            bEnable = true;
        m_pDefineBT->Enable( bEnable );
    }
}

// convert database name for display
void SwChangeDBDlg::ShowDBName(const SwDBData& rDBData)
{
    if (rDBData.sDataSource.isEmpty() && rDBData.sCommand.isEmpty())
    {
        m_pDocDBNameFT->SetText(SW_RESSTR(SW_STR_NONE));
    }
    else
    {
        const OUString sName(rDBData.sDataSource + "." + rDBData.sCommand);
        m_pDocDBNameFT->SetText(sName.replaceAll("~", "~~"));
    }
}

IMPL_LINK_NOARG_TYPED(SwChangeDBDlg, AddDBHdl, Button*, void)
{
    const OUString sNewDB = SwDBManager::LoadAndRegisterDataSource();
    if (!sNewDB.isEmpty())
        m_pAvailDBTLB->AddDataSource(sNewDB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
