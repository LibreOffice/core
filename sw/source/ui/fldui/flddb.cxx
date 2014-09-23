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

#include <fldui.hrc>
#include <flddb.hxx>
#include <dbconfig.hxx>
#include <dbmgr.hxx>

#define USER_DATA_VERSION_1     "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldDBPage::SwFldDBPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SwFldPage(pParent, "FldDbPage",
        "modules/swriter/ui/flddbpage.ui", rCoreSet)
    , m_nOldFormat(0)
    , m_nOldSubType(0)
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

    m_aOldNumSelectHdl = m_pNumFormatLB->GetSelectHdl();

    m_pNumFormatLB->SetSelectHdl(LINK(this, SwFldDBPage, NumSelectHdl));
    m_pDatabaseTLB->SetSelectHdl(LINK(this, SwFldDBPage, TreeSelectHdl));
    m_pDatabaseTLB->SetDoubleClickHdl(LINK(this, SwFldDBPage, InsertHdl));

    m_pValueED->SetModifyHdl(LINK(this, SwFldDBPage, ModifyHdl));
    m_pAddDBPB->SetClickHdl(LINK(this, SwFldDBPage, AddDBHdl));
}

SwFldDBPage::~SwFldDBPage()
{
}

// initialise TabPage
void SwFldDBPage::Reset(const SfxItemSet*)
{
    Init(); // Allgemeine initialisierung

    m_pTypeLB->SetUpdateMode(false);
    const sal_Int32 nOldPos = m_pTypeLB->GetSelectEntryPos();
    m_sOldDBName = m_pDatabaseTLB->GetDBName(m_sOldTableName, m_sOldColumnName);

    m_pTypeLB->Clear();

    if (!IsFldEdit())
    {
        // initialise TypeListBox
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for(sal_uInt16 i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            const sal_uInt16 nTypeId = SwFldMgr::GetTypeId(i);
            const sal_Int32 nPos = m_pTypeLB->InsertEntry(SwFldMgr::GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        const sal_uInt16 nTypeId = GetCurField()->GetTypeId();
        const sal_Int32 nPos = m_pTypeLB->InsertEntry(
            SwFldMgr::GetTypeStr(SwFldMgr::GetPos(nTypeId)));
        m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
    }

    // select old Pos
    if (GetTypeSel() != LISTBOX_ENTRY_NOTFOUND)
        m_pTypeLB->SelectEntryPos(GetTypeSel());

    m_pFormatLB->Clear();

    const sal_uInt16 nSize = GetFldMgr().GetFormatCount(TYP_DBSETNUMBERFLD, false, IsFldDlgHtmlMode());
    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        const sal_Int32 nEntryPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr(TYP_DBSETNUMBERFLD, i));
        const sal_uInt16 nFmtId = GetFldMgr().GetFormatId( TYP_DBSETNUMBERFLD, i );
        m_pFormatLB->SetEntryData( nEntryPos, reinterpret_cast<void*>(nFmtId) );
        if( SVX_NUM_ARABIC == nFmtId )
            m_pFormatLB->SelectEntryPos( nEntryPos );
    }

    if (!IsFldEdit())
    {
        if (nOldPos != LISTBOX_ENTRY_NOTFOUND)
            m_pTypeLB->SelectEntryPos(nOldPos);

        if (!m_sOldDBName.isEmpty())
        {
            m_pDatabaseTLB->Select(m_sOldDBName, m_sOldTableName, m_sOldColumnName);
        }
        else
        {
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                SwDBData aTmp(pSh->GetDBData());
                m_pDatabaseTLB->Select(aTmp.sDataSource, aTmp.sCommand, aEmptyOUStr);
            }
        }
    }

    if( !IsRefresh() )
    {
        OUString sUserData = GetUserData();
        if (sUserData.getToken(0, ';').equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            const sal_uInt16 nVal = (sal_uInt16)sUserData.getToken(1, ';').toInt32();
            if(nVal != USHRT_MAX)
            {
                for (sal_Int32 i = 0; i < m_pTypeLB->GetEntryCount(); ++i)
                    if(nVal == (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(i))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    m_pTypeLB->SetUpdateMode(true);
    m_pTypeLB->SetSelectHdl(LINK(this, SwFldDBPage, TypeHdl));
    m_pTypeLB->SetDoubleClickHdl(LINK(this, SwFldDBPage, InsertHdl));

    if (IsFldEdit())
    {
        m_pConditionED->SaveValue();
        m_pValueED->SaveValue();
        m_sOldDBName = m_pDatabaseTLB->GetDBName(m_sOldTableName, m_sOldColumnName);
        m_nOldFormat = GetCurField()->GetFormat();
        m_nOldSubType = GetCurField()->GetSubType();
    }
}

bool SwFldDBPage::FillItemSet(SfxItemSet* )
{
    OUString sTableName;
    OUString sColumnName;
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
        const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());
        sal_uLong nFormat = 0;
        sal_uInt16 nSubType = 0;

        OUString sDBName = aData.sDataSource
            + OUString(DB_DELIM)
            + aData.sCommand
            + OUString(DB_DELIM)
            + OUString::number(aData.nCommandType)
            + OUString(DB_DELIM);
        if (!sColumnName.isEmpty())
        {
            sDBName += sColumnName + OUString(DB_DELIM);
        }
        OUString aName = sDBName + m_pConditionED->GetText();

        switch (nTypeId)
        {
        case TYP_DBFLD:
            nFormat = m_pNumFormatLB->GetFormat();
            if (m_pNewFormatRB->IsEnabled() && m_pNewFormatRB->IsChecked())
                nSubType = nsSwExtendedSubType::SUB_OWN_FMT;
            aName = sDBName;
            break;

        case TYP_DBSETNUMBERFLD:
            nFormat = (sal_uLong)m_pFormatLB->GetEntryData(
                                m_pFormatLB->GetSelectEntryPos() );
            break;
        }

        const OUString aVal(m_pValueED->GetText());
        OUString sTempTableName;
        OUString sTempColumnName;
        OUString sTempDBName = m_pDatabaseTLB->GetDBName(sTempTableName, sTempColumnName);
        bool bDBListBoxChanged = m_sOldDBName != sTempDBName ||
            m_sOldTableName != sTempTableName || m_sOldColumnName != sTempColumnName;
        if (!IsFldEdit() ||
            m_pConditionED->IsValueChangedFromSaved() ||
            m_pValueED->GetSavedValue() != aVal ||
             bDBListBoxChanged ||
             m_nOldFormat != nFormat || m_nOldSubType != nSubType)
        {
            InsertFld( nTypeId, nSubType, aName, aVal, nFormat);
        }
    }

    return false;
}

SfxTabPage* SwFldDBPage::Create(    vcl::Window* pParent,
                        const SfxItemSet* rAttrSet )
{
    return ( new SwFldDBPage( pParent, *rAttrSet ) );
}

sal_uInt16 SwFldDBPage::GetGroup()
{
    return GRP_DB;
}

IMPL_LINK( SwFldDBPage, TypeHdl, ListBox *, pBox )
{
    // save old ListBoxPos
    const sal_Int32 nOld = GetTypeSel();

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
        bool bCond = false, bSetNo = false, bFormat = false, bDBFormat = false;
        const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        m_pDatabaseTLB->ShowColumns(nTypeId == TYP_DBFLD);

        if (IsFldEdit())
        {
            SwDBData aData;
            OUString sColumnName;
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
                bFormat = true;
                bDBFormat = true;
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
                bSetNo = true;
                // no break!
            case TYP_DBNEXTSETFLD:
                bCond = true;
                if (IsFldEdit())
                {
                    m_pConditionED->SetText(GetCurField()->GetPar1());
                    m_pValueED->SetText(GetCurField()->GetPar2());
                }
                break;

            case TYP_DBNAMEFLD:
                break;

            case TYP_DBSETNUMBERFLD:
                bFormat = true;
                m_pNewFormatRB->Check();
                m_pNumFormatLB->Hide();
                m_pFormatLB->Show();
                if( IsFldEdit() )
                {
                    for( sal_Int32 nI = m_pFormatLB->GetEntryCount(); nI; )
                        if( GetCurField()->GetFormat() == (sal_uLong)
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
            m_pNewFormatRB->Enable(bDBFormat || bFormat);
            m_pNumFormatLB->Enable(bDBFormat);
            m_pFormatLB->Enable(bFormat);
        }
        m_pFormat->Enable(bDBFormat || bFormat);

        if (!IsFldEdit())
        {
            m_pValueED->SetText(aEmptyOUStr);
            if (bCond)
                m_pConditionED->SetText("TRUE");
            else
                m_pConditionED->SetText(aEmptyOUStr);
        }

        CheckInsert();
    }

    return 0;
}

IMPL_LINK( SwFldDBPage, NumSelectHdl, NumFormatListBox *, pLB )
{
    m_pNewFormatRB->Check();
    m_aOldNumSelectHdl.Call(pLB);

    return 0;
}

void SwFldDBPage::CheckInsert()
{
    bool bInsert = true;
    const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    SvTreeListEntry* pEntry = m_pDatabaseTLB->GetCurEntry();

    if (pEntry)
    {
        pEntry = m_pDatabaseTLB->GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = m_pDatabaseTLB->GetParent(pEntry);

        bInsert &= pEntry != 0;
    }
    else
        bInsert = false;

    if (nTypeId == TYP_DBNUMSETFLD)
    {
        bool bHasValue = !m_pValueED->GetText().isEmpty();

        bInsert &= bHasValue;
    }

    EnableInsert(bInsert);
}

IMPL_LINK( SwFldDBPage, TreeSelectHdl, SvTreeListBox *, pBox )
{
    SvTreeListEntry* pEntry = pBox->GetCurEntry();
    if (pEntry)
    {
        const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        pEntry = m_pDatabaseTLB->GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = m_pDatabaseTLB->GetParent(pEntry);

        CheckInsert();

        if (nTypeId == TYP_DBFLD)
        {
            bool bNumFormat = false;

            if (pEntry != 0)
            {
                OUString sTableName;
                OUString sColumnName;
                sal_Bool bIsTable;
                OUString sDBName = m_pDatabaseTLB->GetDBName(sTableName, sColumnName, &bIsTable);
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
    OUString sNewDB = SwDBManager::LoadAndRegisterDataSource();
    if(!sNewDB.isEmpty())
    {
        m_pDatabaseTLB->AddDataSource(sNewDB);
    }
    return 0;
}

// Modify
IMPL_LINK_NOARG(SwFldDBPage, ModifyHdl)
{
    CheckInsert();
    return 0;
}

void    SwFldDBPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_pTypeLB->GetSelectEntryPos();
    const sal_uInt16 nTypeSel = ( LISTBOX_ENTRY_NOTFOUND == nEntryPos )
        ? USHRT_MAX : (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData( nEntryPos );
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

void SwFldDBPage::ActivateMailMergeAddress()
{
    sal_uLong nData = TYP_DBFLD;
    m_pTypeLB->SelectEntryPos(m_pTypeLB->GetEntryPos( (const void*) nData ));
    m_pTypeLB->GetSelectHdl().Call(m_pTypeLB);
    const SwDBData& rData = SW_MOD()->GetDBConfig()->GetAddressSource();
    m_pDatabaseTLB->Select(rData.sDataSource, rData.sCommand, aEmptyOUStr);
}

void SwFldDBPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pDatabaseTLB->SetWrtShell(rSh);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
