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
#include <strings.hrc>
#include "bitmaps.hlst"

#include <unomid.h>

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;

// edit insert-field
SwChangeDBDlg::SwChangeDBDlg(SwView& rVw)
    : SvxStandardDialog(&rVw.GetViewFrame()->GetWindow(), "ExchangeDatabasesDialog",
        "modules/swriter/ui/exchangedatabases.ui")
    , pSh(rVw.GetWrtShellPtr())
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

    m_pUsedDBTLB->SetSelectionMode(SelectionMode::Multiple);
    m_pUsedDBTLB->SetStyle(m_pUsedDBTLB->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    m_pUsedDBTLB->SetSpaceBetweenEntries(0);
    m_pUsedDBTLB->SetNodeBitmaps(Image(BitmapEx(RID_BMP_COLLAPSE)),
                                 Image(BitmapEx(RID_BMP_EXPAND)));

    Link<SvTreeListBox*,void> aLink = LINK(this, SwChangeDBDlg, TreeSelectHdl);

    m_pUsedDBTLB->SetSelectHdl(aLink);
    m_pUsedDBTLB->SetDeselectHdl(aLink);
    m_pAvailDBTLB->SetSelectHdl(aLink);
    m_pAvailDBTLB->SetSelectHdl(aLink);
    TreeSelectHdl(nullptr);
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
    SvTreeListEntry *pFirst = nullptr;
    SvTreeListEntry *pLast = nullptr;

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

    Image aTableImg(BitmapEx(RID_BMP_DBTABLE));
    Image aDBImg(BitmapEx(RID_BMP_DB));
    Image aQueryImg(BitmapEx(RID_BMP_DBQUERY));
    Image& rToInsert = nCommandType ? aQueryImg : aTableImg;
    while ((pParent = m_pUsedDBTLB->GetEntry(nParent++)) != nullptr)
    {
        if (sDBName == m_pUsedDBTLB->GetEntryText(pParent))
        {
            while ((pChild = m_pUsedDBTLB->GetEntry(pParent, nChild++)) != nullptr)
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
                          OUStringLiteral1(DB_DELIM) + m_pUsedDBTLB->GetEntryText( pEntry ) + OUStringLiteral1(DB_DELIM) +
                          OUString::number((int)reinterpret_cast<sal_uLong>(pEntry->GetUserData())));
            aDBNames.push_back(sTmp);
        }
        pEntry = m_pUsedDBTLB->NextSelected(pEntry);
    }

    pSh->StartAllAction();
    OUString sTableName;
    OUString sColumnName;
    sal_Bool bIsTable = false;
    const OUString DBName(m_pAvailDBTLB->GetDBName(sTableName, sColumnName, &bIsTable));
    const OUString sTemp = DBName
        + OUStringLiteral1(DB_DELIM)
        + sTableName
        + OUStringLiteral1(DB_DELIM)
        + OUString::number(bIsTable
                            ? CommandType::TABLE
                            : CommandType::QUERY);
    pSh->ChangeDBFields( aDBNames, sTemp);
    pSh->EndAllAction();
}

IMPL_LINK_NOARG(SwChangeDBDlg, ButtonHdl, Button*, void)
{
    OUString sTableName;
    OUString sColumnName;
    SwDBData aData;
    sal_Bool bIsTable = false;
    aData.sDataSource = m_pAvailDBTLB->GetDBName(sTableName, sColumnName, &bIsTable);
    aData.sCommand = sTableName;
    aData.nCommandType = bIsTable ? 0 : 1;
    pSh->ChgDBData(aData);
    ShowDBName(pSh->GetDBData());
    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(SwChangeDBDlg, TreeSelectHdl, SvTreeListBox*, void)
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
        m_pDocDBNameFT->SetText(SwResId(SW_STR_NONE));
    }
    else
    {
        const OUString sName(rDBData.sDataSource + "." + rDBData.sCommand);
        m_pDocDBNameFT->SetText(sName.replaceAll("~", "~~"));
    }
}

IMPL_LINK_NOARG(SwChangeDBDlg, AddDBHdl, Button*, void)
{
    const OUString sNewDB = SwDBManager::LoadAndRegisterDataSource();
    if (!sNewDB.isEmpty())
        m_pAvailDBTLB->AddDataSource(sNewDB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
