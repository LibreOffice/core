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

#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <globals.hrc>
#include <dbfld.hxx>
#include <fldtdlg.hxx>
#include <numrule.hxx>

#include <fldtdlg.hrc>
#include <fldui.hrc>
#include <flddb.hxx>
#include <dbconfig.hxx>
#include <dbmgr.hxx>

#define USER_DATA_VERSION_1     "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldDBPage::SwFldDBPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SwFldPage(pParent, "FldDbPage",
        "modules/swriter/ui/flddbpage.ui", rCoreSet)
{
    get(m_pTypeLB, "type");
    m_pTypeLB->SetStyle(m_pTypeLB->GetStyle() | WB_SORT);
    get(m_pDatabaseTLB, "select");
    get(m_pCondition, "condgroup");
    get(m_pConditionED, "condition");
    get(m_pValue, "recgroup");
    get(m_pValueED, "recnumber");
    get(m_pFormat, "formatframe");
    get(m_pAddDBPB, "browse");
    get(m_pDBFormatRB, "fromdatabasecb");
    get(m_pNewFormatRB, "userdefinedcb");
    get(m_pNumFormatLB, "numformat");
    get(m_pFormatLB, "format");
    m_pFormatLB->SetStyle(m_pFormatLB->GetStyle() | WB_SORT);

    long nHeight = m_pTypeLB->GetTextHeight() * 14;
    m_pTypeLB->set_height_request(nHeight);
    m_pDatabaseTLB->set_height_request(nHeight);

    long nWidth = m_pTypeLB->LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MAP_APPFONT)).Width();
    m_pTypeLB->set_width_request(nWidth);
    m_pDatabaseTLB->set_width_request(nWidth*2);

    aOldNumSelectHdl = m_pNumFormatLB->GetSelectHdl();

    m_pNumFormatLB->SetSelectHdl(LINK(this, SwFldDBPage, NumSelectHdl));
    m_pDatabaseTLB->SetSelectHdl(LINK(this, SwFldDBPage, TreeSelectHdl));
    m_pDatabaseTLB->SetDoubleClickHdl(LINK(this, SwFldDBPage, InsertHdl));

    m_pValueED->SetModifyHdl(LINK(this, SwFldDBPage, ModifyHdl));
    m_pAddDBPB->SetClickHdl(LINK(this, SwFldDBPage, AddDBHdl));
}

SwFldDBPage::~SwFldDBPage()
{
}

/*--------------------------------------------------------------------
    Description: initialise TabPage
 --------------------------------------------------------------------*/
void SwFldDBPage::Reset(const SfxItemSet&)
{
    Init(); // Allgemeine initialisierung

    m_pTypeLB->SetUpdateMode(sal_False);
    sal_uInt16 nOldPos = m_pTypeLB->GetSelectEntryPos();
    sOldDBName = m_pDatabaseTLB->GetDBName(sOldTableName, sOldColumnName);

    m_pTypeLB->Clear();

    sal_uInt16 nPos, nTypeId, i;

    if (!IsFldEdit())
    {
        // initialise TypeListBox
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for(i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = m_pTypeLB->InsertEntry(GetFldMgr().GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        nTypeId = GetCurField()->GetTypeId();
        nPos = m_pTypeLB->InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
    }

    // select old Pos
    if (GetTypeSel() != LISTBOX_ENTRY_NOTFOUND)
        m_pTypeLB->SelectEntryPos(GetTypeSel());

    m_pFormatLB->Clear();

    sal_uInt16 nSize = GetFldMgr().GetFormatCount(TYP_DBSETNUMBERFLD, false, IsFldDlgHtmlMode());
    for( i = 0; i < nSize; ++i )
    {
        sal_uInt16 nEntryPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr(TYP_DBSETNUMBERFLD, i));
        sal_uInt16 nFmtId = GetFldMgr().GetFormatId( TYP_DBSETNUMBERFLD, i );
        m_pFormatLB->SetEntryData( nEntryPos, reinterpret_cast<void*>(nFmtId) );
        if( SVX_NUM_ARABIC == nFmtId )
            m_pFormatLB->SelectEntryPos( nEntryPos );
    }

    if (!IsFldEdit())
    {
        if (nOldPos != LISTBOX_ENTRY_NOTFOUND)
            m_pTypeLB->SelectEntryPos(nOldPos);

        if (sOldDBName.Len())
        {
            m_pDatabaseTLB->Select(sOldDBName, sOldTableName, sOldColumnName);
        }
        else
        {
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                SwDBData aTmp(pSh->GetDBData());
                m_pDatabaseTLB->Select(aTmp.sDataSource, aTmp.sCommand, aEmptyStr);
            }
        }
    }

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            sal_uInt16 nVal = (sal_uInt16)sVal.ToInt32();
            if(nVal != USHRT_MAX)
            {
                for(i = 0; i < m_pTypeLB->GetEntryCount(); i++)
                    if(nVal == (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(i))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    m_pTypeLB->SetUpdateMode(sal_True);
    m_pTypeLB->SetSelectHdl(LINK(this, SwFldDBPage, TypeHdl));
    m_pTypeLB->SetDoubleClickHdl(LINK(this, SwFldDBPage, InsertHdl));

    if (IsFldEdit())
    {
        m_pConditionED->SaveValue();
        m_pValueED->SaveValue();
        sOldDBName = m_pDatabaseTLB->GetDBName(sOldTableName, sOldColumnName);
        nOldFormat = GetCurField()->GetFormat();
        nOldSubType = GetCurField()->GetSubType();
    }
}

sal_Bool SwFldDBPage::FillItemSet(SfxItemSet& )
{
    String sTableName, sColumnName;
    SwDBData aData;
    sal_Bool bIsTable;
    aData.sDataSource = m_pDatabaseTLB->GetDBName(sTableName, sColumnName, &bIsTable);
    aData.sCommand = sTableName;
    aData.nCommandType = bIsTable ? 0 : 1;
    SwWrtShell *pSh = GetWrtShell();
    if(!pSh)
        pSh = ::GetActiveWrtShell();

    if (aData.sDataSource.isEmpty())
        aData = pSh->GetDBData();

    if(!aData.sDataSource.isEmpty())       // without database no new field command
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());
        String aVal(m_pValueED->GetText());
        String aName(m_pConditionED->GetText());
        sal_uLong nFormat = 0;
        sal_uInt16 nSubType = 0;

        String sDBName = aData.sDataSource;
        sDBName += DB_DELIM;
        sDBName += (String)aData.sCommand;
        sDBName += DB_DELIM;
        sDBName += OUString::number(aData.nCommandType);
        sDBName += DB_DELIM;
        if(sColumnName.Len())
        {
            sDBName += sColumnName;
            sDBName += DB_DELIM;
        }
        aName.Insert(sDBName, 0);

        switch (nTypeId)
        {
        case TYP_DBFLD:
            nFormat = m_pNumFormatLB->GetFormat();
            if (m_pNewFormatRB->IsEnabled() && m_pNewFormatRB->IsChecked())
                nSubType = nsSwExtendedSubType::SUB_OWN_FMT;
            aName = sDBName;
            break;

        case TYP_DBSETNUMBERFLD:
            nFormat = (sal_uInt16)(sal_uLong)m_pFormatLB->GetEntryData(
                                m_pFormatLB->GetSelectEntryPos() );
            break;
        }


        String sTempDBName, sTempTableName, sTempColumnName;
        sTempDBName = m_pDatabaseTLB->GetDBName(sTempTableName, sTempColumnName);
        sal_Bool bDBListBoxChanged = sOldDBName != sTempDBName ||
            sOldTableName != sTempTableName || sOldColumnName != sTempColumnName;
        if (!IsFldEdit() ||
            m_pConditionED->GetSavedValue() != m_pConditionED->GetText() ||
            m_pValueED->GetSavedValue() != m_pValueED->GetText() ||
             bDBListBoxChanged ||
             nOldFormat != nFormat || nOldSubType != nSubType)
        {
            InsertFld( nTypeId, nSubType, aName, aVal, nFormat);
        }
    }

    return sal_False;
}

SfxTabPage* SwFldDBPage::Create(    Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldDBPage( pParent, rAttrSet ) );
}

sal_uInt16 SwFldDBPage::GetGroup()
{
    return GRP_DB;
}

IMPL_LINK( SwFldDBPage, TypeHdl, ListBox *, pBox )
{
    // save old ListBoxPos
    const sal_uInt16 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_pTypeLB->GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        m_pTypeLB->SelectEntryPos(0);
    }

    if (nOld != GetTypeSel())
    {
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
            pSh = ::GetActiveWrtShell();
        sal_Bool bCond = sal_False, bSetNo = sal_False, bFormat = sal_False, bDBFormat = sal_False;
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        m_pDatabaseTLB->ShowColumns(nTypeId == TYP_DBFLD);

        if (IsFldEdit())
        {
            SwDBData aData;
            String sColumnName;
            if (nTypeId == TYP_DBFLD)
            {
                aData = ((SwDBField*)GetCurField())->GetDBData();
                sColumnName = ((SwDBFieldType*)GetCurField()->GetTyp())->GetColumnName();
            }
            else
            {
                aData = ((SwDBNameInfField*)GetCurField())->GetDBData(pSh->GetDoc());
            }
            m_pDatabaseTLB->Select(aData.sDataSource, aData.sCommand, sColumnName);
        }

        switch (nTypeId)
        {
            case TYP_DBFLD:
                bFormat = sal_True;
                bDBFormat = sal_True;
                m_pNumFormatLB->Show();
                m_pFormatLB->Hide();

                if (pBox)   // type was changed by user
                    m_pDBFormatRB->Check();

                if (IsFldEdit())
                {
                    if (GetCurField()->GetFormat() != 0 && GetCurField()->GetFormat() != SAL_MAX_UINT32)
                        m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());

                    if (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_OWN_FMT)
                        m_pNewFormatRB->Check();
                    else
                        m_pDBFormatRB->Check();
                }
                break;

            case TYP_DBNUMSETFLD:
                bSetNo = sal_True;
                // no break!
            case TYP_DBNEXTSETFLD:
                bCond = sal_True;
                if (IsFldEdit())
                {
                    m_pConditionED->SetText(GetCurField()->GetPar1());
                    m_pValueED->SetText(GetCurField()->GetPar2());
                }
                break;

            case TYP_DBNAMEFLD:
                break;

            case TYP_DBSETNUMBERFLD:
                bFormat = sal_True;
                m_pNewFormatRB->Check();
                m_pNumFormatLB->Hide();
                m_pFormatLB->Show();
                if( IsFldEdit() )
                {
                    for( sal_uInt16 nI = m_pFormatLB->GetEntryCount(); nI; )
                        if( GetCurField()->GetFormat() == (sal_uInt16)(sal_uLong)
                            m_pFormatLB->GetEntryData( --nI ))
                        {
                            m_pFormatLB->SelectEntryPos( nI );
                            break;
                        }
                }
                break;
        }

        m_pCondition->Enable(bCond);
        m_pValue->Enable(bSetNo);
        if (nTypeId != TYP_DBFLD)
        {
            m_pDBFormatRB->Enable(bDBFormat);
            m_pNewFormatRB->Enable(bDBFormat|bFormat);
            m_pNumFormatLB->Enable(bDBFormat);
            m_pFormatLB->Enable(bFormat);
        }
        m_pFormat->Enable(bDBFormat|bFormat);

        if (!IsFldEdit())
        {
            m_pValueED->SetText(aEmptyStr);
            if (bCond)
                m_pConditionED->SetText(OUString("TRUE"));
            else
                m_pConditionED->SetText(aEmptyStr);
        }

        CheckInsert();
    }

    return 0;
}

IMPL_LINK( SwFldDBPage, NumSelectHdl, NumFormatListBox *, pLB )
{
    m_pNewFormatRB->Check();
    aOldNumSelectHdl.Call(pLB);

    return 0;
}

void SwFldDBPage::CheckInsert()
{
    sal_Bool bInsert = sal_True;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    SvTreeListEntry* pEntry = m_pDatabaseTLB->GetCurEntry();

    if (pEntry)
    {
        pEntry = m_pDatabaseTLB->GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = m_pDatabaseTLB->GetParent(pEntry);

        bInsert &= pEntry != 0;
    }
    else
        bInsert = sal_False;

    if (nTypeId == TYP_DBNUMSETFLD)
    {
        sal_Bool bHasValue = !m_pValueED->GetText().isEmpty();

        bInsert &= bHasValue;
    }

    EnableInsert(bInsert);
}

IMPL_LINK( SwFldDBPage, TreeSelectHdl, SvTreeListBox *, pBox )
{
    SvTreeListEntry* pColEntry;
    SvTreeListEntry* pEntry = pColEntry = pBox->GetCurEntry();
    if (pEntry)
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        pEntry = m_pDatabaseTLB->GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = m_pDatabaseTLB->GetParent(pEntry);

        CheckInsert();

        if (nTypeId == TYP_DBFLD)
        {
            sal_Bool bNumFormat = sal_False;

            if (pEntry != 0)
            {
                String sTableName;
                String sColumnName;
                sal_Bool bIsTable;
                String sDBName = m_pDatabaseTLB->GetDBName(sTableName, sColumnName, &bIsTable);
                bNumFormat = GetFldMgr().IsDBNumeric(sDBName,
                            sTableName,
                            bIsTable,
                            sColumnName);
                if (!IsFldEdit())
                    m_pDBFormatRB->Check();
            }

            m_pDBFormatRB->Enable(bNumFormat);
            m_pNewFormatRB->Enable(bNumFormat);
            m_pNumFormatLB->Enable(bNumFormat);
            m_pFormat->Enable(bNumFormat);
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SwFldDBPage, AddDBHdl)
{
    String sNewDB = SwNewDBMgr::LoadAndRegisterDataSource();
    if(sNewDB.Len())
    {
        m_pDatabaseTLB->AddDataSource(sNewDB);
    }
    return 0;
}

/*--------------------------------------------------------------------
    Description: Modify
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwFldDBPage, ModifyHdl)
{
    CheckInsert();
    return 0;
}

void    SwFldDBPage::FillUserData()
{
    String sData(OUString(USER_DATA_VERSION));
    sData += ';';
    sal_uInt16 nTypeSel = m_pTypeLB->GetSelectEntryPos();

    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData( nTypeSel );
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

void SwFldDBPage::ActivateMailMergeAddress()
{
    sal_uLong nData = TYP_DBFLD;
    m_pTypeLB->SelectEntryPos(m_pTypeLB->GetEntryPos( (const void*) nData ));
    m_pTypeLB->GetSelectHdl().Call(m_pTypeLB);
    const SwDBData& rData = SW_MOD()->GetDBConfig()->GetAddressSource();
    m_pDatabaseTLB->Select(rData.sDataSource, rData.sCommand, aEmptyStr);
}

void SwFldDBPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pDatabaseTLB->SetWrtShell(rSh);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
