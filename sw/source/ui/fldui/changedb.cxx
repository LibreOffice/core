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
#include <utlui.hrc>
#include <changedb.hrc>

#include <unomid.h>

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;


/*--------------------------------------------------------------------
    Description: edit insert-field
 --------------------------------------------------------------------*/
SwChangeDBDlg::SwChangeDBDlg(SwView& rVw) :
    SvxStandardDialog(&rVw.GetViewFrame()->GetWindow(), SW_RES(DLG_CHANGE_DB)),

    aDBListFL   (this, SW_RES(FL_DBLIST     )),
    aUsedDBFT   (this, SW_RES(FT_USEDDB     )),
    aAvailDBFT  (this, SW_RES(FT_AVAILDB    )),
    aUsedDBTLB  (this, SW_RES(TLB_USEDDB    )),
    aAvailDBTLB (this, SW_RES(TLB_AVAILDB   ), 0),
    aAddDBPB    (this, SW_RES(PB_ADDDB)),
    aDescFT     (this, SW_RES(FT_DESC       )),
    aDocDBTextFT(this, SW_RES(FT_DOCDBTEXT  )),
    aDocDBNameFT(this, SW_RES(FT_DOCDBNAME  )),
    aOKBT       (this, SW_RES(BT_OK         )),
    aCancelBT   (this, SW_RES(BT_CANCEL     )),
    aHelpBT     (this, SW_RES(BT_HELP       )),
    aImageList      (SW_RES(ILIST_DB_DLG    )),
    pSh(rVw.GetWrtShellPtr()),
    pMgr( new SwFldMgr() )
{
    aAvailDBTLB.SetWrtShell(*pSh);
    FillDBPopup();

    FreeResource();

    ShowDBName(pSh->GetDBData());
    aOKBT.SetClickHdl(LINK(this, SwChangeDBDlg, ButtonHdl));
    aAddDBPB.SetClickHdl(LINK(this, SwChangeDBDlg, AddDBHdl));

    aUsedDBTLB.SetSelectionMode(MULTIPLE_SELECTION);
    aUsedDBTLB.SetStyle(aUsedDBTLB.GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    aUsedDBTLB.SetSpaceBetweenEntries(0);
    aUsedDBTLB.SetNodeBitmaps( aImageList.GetImage(IMG_COLLAPSE), aImageList.GetImage(IMG_EXPAND));

    Link aLink = LINK(this, SwChangeDBDlg, TreeSelectHdl);

    aUsedDBTLB.SetSelectHdl(aLink);
    aUsedDBTLB.SetDeselectHdl(aLink);
    aAvailDBTLB.SetSelectHdl(aLink);
    aAvailDBTLB.SetDeselectHdl(aLink);
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
    aAvailDBTLB.Select(sDBName, sTableName, aEmptyStr);

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
    aUsedDBTLB.Clear();
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
        aUsedDBTLB.MakeVisible(pFirst);
        aUsedDBTLB.Select(pFirst);
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
    while ((pParent = aUsedDBTLB.GetEntry(nParent++)) != NULL)
    {
        if (sDBName == aUsedDBTLB.GetEntryText(pParent))
        {
            while ((pChild = aUsedDBTLB.GetEntry(pParent, nChild++)) != NULL)
            {
                if (sTableName == aUsedDBTLB.GetEntryText(pChild))
                    return pChild;
            }
            SvTreeListEntry* pRet = aUsedDBTLB.InsertEntry(sTableName, rToInsert, rToInsert, pParent);
            pRet->SetUserData((void*)nCommandType);
            return pRet;
        }
    }
    pParent = aUsedDBTLB.InsertEntry(sDBName, aDBImg, aDBImg);

    SvTreeListEntry* pRet = aUsedDBTLB.InsertEntry(sTableName, rToInsert, rToInsert, pParent);
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
    aDBNames.reserve(aUsedDBTLB.GetSelectionCount());
    SvTreeListEntry* pEntry = aUsedDBTLB.FirstSelected();

    while( pEntry )
    {
        if( aUsedDBTLB.GetParent( pEntry ))
        {
            String* pTmp = new String( aUsedDBTLB.GetEntryText(
                                            aUsedDBTLB.GetParent( pEntry )));
            *pTmp += DB_DELIM;
            *pTmp += aUsedDBTLB.GetEntryText( pEntry );
            *pTmp += DB_DELIM;
            int nCommandType = (int)(sal_uLong)pEntry->GetUserData();
            *pTmp += String::CreateFromInt32(nCommandType);
            aDBNames.push_back(*pTmp);
        }
        pEntry = aUsedDBTLB.NextSelected(pEntry);
    }

    pSh->StartAllAction();
    String sTableName, sColumnName;
    sal_Bool bIsTable = sal_False;
    String sTemp(aAvailDBTLB.GetDBName(sTableName, sColumnName, &bIsTable));
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
    aData.sDataSource = aAvailDBTLB.GetDBName(sTableName, sColumnName, &bIsTable);
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

    SvTreeListEntry* pEntry = aAvailDBTLB.GetCurEntry();

    if (pEntry)
    {
        if (aAvailDBTLB.GetParent(pEntry))
            bEnable = sal_True;
        aOKBT.Enable( bEnable );
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

    aDocDBNameFT.SetText(sName);
}

IMPL_LINK_NOARG(SwChangeDBDlg, AddDBHdl)
{
    String sNewDB = SwNewDBMgr::LoadAndRegisterDataSource();
    if(sNewDB.Len())
        aAvailDBTLB.AddDataSource(sNewDB);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
