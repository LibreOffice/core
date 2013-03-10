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
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include "svtools/treelistentry.hxx"

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

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;


/*--------------------------------------------------------------------
    Description: edit insert-field
 --------------------------------------------------------------------*/
SwChangeDBDlg::SwChangeDBDlg(SwView& rVw)
    : SvxStandardDialog(&rVw.GetViewFrame()->GetWindow(), "ExchangeDatabasesDialog",
        "modules/swriter/ui/exchangedatabases.ui")
    , aImageList(SW_RES(ILIST_DB_DLG))
    , pSh(rVw.GetWrtShellPtr())
    , pMgr( new SwFldMgr() )
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

    Link aLink = LINK(this, SwChangeDBDlg, TreeSelectHdl);

    m_pUsedDBTLB->SetSelectHdl(aLink);
    m_pUsedDBTLB->SetDeselectHdl(aLink);
    m_pAvailDBTLB->SetSelectHdl(aLink);
    m_pAvailDBTLB->SetSelectHdl(aLink);
    TreeSelectHdl();
}

/*--------------------------------------------------------------------
    Description: initialise database listboxes
 --------------------------------------------------------------------*/
void SwChangeDBDlg::FillDBPopup()
{
    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference<XDatabaseContext> xDBContext = DatabaseContext::create(xContext);

    const SwDBData& rDBData = pSh->GetDBData();
    String sDBName(rDBData.sDataSource);
    String sTableName(rDBData.sCommand);
    m_pAvailDBTLB->Select(sDBName, sTableName, aEmptyStr);

    std::vector<String> aAllDBNames;

    Sequence< ::rtl::OUString > aDBNames = xDBContext->getElementNames();
    const ::rtl::OUString* pDBNames = aDBNames.getConstArray();
    sal_Int32 nDBCount = aDBNames.getLength();
    for(sal_Int32 i = 0; i < nDBCount; i++)
    {
        aAllDBNames.push_back(pDBNames[i]);
    }

    std::vector<String> aDBNameList;
    pSh->GetAllUsedDB( aDBNameList, &aAllDBNames );

    size_t nCount = aDBNameList.size();
    m_pUsedDBTLB->Clear();
    SvTreeListEntry *pFirst = 0;
    SvTreeListEntry *pLast = 0;

    for(size_t k = 0; k < nCount; k++)
    {
        sDBName = aDBNameList[k];
        sDBName = sDBName.GetToken(0);
        pLast = Insert(sDBName);
        if (!pFirst)
            pFirst = pLast;
    }

    if (pFirst)
    {
        m_pUsedDBTLB->MakeVisible(pFirst);
        m_pUsedDBTLB->Select(pFirst);
    }

}

SvTreeListEntry* SwChangeDBDlg::Insert(const String& rDBName)
{
    String sDBName(rDBName.GetToken(0, DB_DELIM));
    String sTableName(rDBName.GetToken(1, DB_DELIM));
    sal_IntPtr nCommandType = rDBName.GetToken(2, DB_DELIM).ToInt32();
    SvTreeListEntry* pParent;
    SvTreeListEntry* pChild;

    sal_uInt16 nParent = 0;
    sal_uInt16 nChild = 0;

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
            pRet->SetUserData((void*)nCommandType);
            return pRet;
        }
    }
    pParent = m_pUsedDBTLB->InsertEntry(sDBName, aDBImg, aDBImg);

    SvTreeListEntry* pRet = m_pUsedDBTLB->InsertEntry(sTableName, rToInsert, rToInsert, pParent);
    pRet->SetUserData((void*)nCommandType);
    return pRet;
}

/*--------------------------------------------------------------------
    Description: destroy dialog
 --------------------------------------------------------------------*/
SwChangeDBDlg::~SwChangeDBDlg()
{
    delete pMgr;
}

/*--------------------------------------------------------------------
     Description:   close
 --------------------------------------------------------------------*/
void SwChangeDBDlg::Apply()
{
    UpdateFlds();
}

void SwChangeDBDlg::UpdateFlds()
{
    std::vector<String> aDBNames;
    aDBNames.reserve(m_pUsedDBTLB->GetSelectionCount());
    SvTreeListEntry* pEntry = m_pUsedDBTLB->FirstSelected();

    while( pEntry )
    {
        if( m_pUsedDBTLB->GetParent( pEntry ))
        {
            OUString sTmp(m_pUsedDBTLB->GetEntryText( m_pUsedDBTLB->GetParent( pEntry )) +
                          OUString(DB_DELIM) + m_pUsedDBTLB->GetEntryText( pEntry ) + OUString(DB_DELIM) +
                          OUString::number((int)(sal_uLong)pEntry->GetUserData()));
            aDBNames.push_back(sTmp);
        }
        pEntry = m_pUsedDBTLB->NextSelected(pEntry);
    }

    pSh->StartAllAction();
    String sTableName, sColumnName;
    sal_Bool bIsTable = sal_False;
    String sTemp(m_pAvailDBTLB->GetDBName(sTableName, sColumnName, &bIsTable));
    sTemp += DB_DELIM;
    sTemp += sTableName;
    sTemp += DB_DELIM;
    sTemp += bIsTable ? '0' : '1';
    pSh->ChangeDBFields( aDBNames, sTemp);
    pSh->EndAllAction();
}

IMPL_LINK_NOARG(SwChangeDBDlg, ButtonHdl)
{
    String sTableName, sColumnName;
    SwDBData aData;
    sal_Bool bIsTable = sal_False;
    aData.sDataSource = m_pAvailDBTLB->GetDBName(sTableName, sColumnName, &bIsTable);
    aData.sCommand = sTableName;
    aData.nCommandType = bIsTable ? 0 : 1;
    pSh->ChgDBData(aData);
    ShowDBName(pSh->GetDBData());
    EndDialog(RET_OK);

    return 0;
}

IMPL_LINK_NOARG(SwChangeDBDlg, TreeSelectHdl)
{
    sal_Bool bEnable = sal_False;

    SvTreeListEntry* pEntry = m_pAvailDBTLB->GetCurEntry();

    if (pEntry)
    {
        if (m_pAvailDBTLB->GetParent(pEntry))
            bEnable = sal_True;
        m_pDefineBT->Enable( bEnable );
    }
    return 0;
}

/*--------------------------------------------------------------------
    Description: convert database name for display
 --------------------------------------------------------------------*/
void SwChangeDBDlg::ShowDBName(const SwDBData& rDBData)
{
    String sTmp(rDBData.sDataSource);
    String sName;
    sTmp += '.';
    sTmp += (String)rDBData.sCommand;

    for (sal_uInt16 i = 0; i < sTmp.Len(); i++)
    {
        sName += sTmp.GetChar(i);
        if (sTmp.GetChar(i) == '~')
            sName += '~';
    }

    if (sName.EqualsAscii(".")) //empty
        sName = SW_RESSTR(SW_STR_NONE);

    m_pDocDBNameFT->SetText(sName);
}

IMPL_LINK_NOARG(SwChangeDBDlg, AddDBHdl)
{
    OUString sNewDB = SwNewDBMgr::LoadAndRegisterDataSource();
    if (!sNewDB.isEmpty())
        m_pAvailDBTLB->AddDataSource(sNewDB);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
