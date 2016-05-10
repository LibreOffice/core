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

SwFieldDBPage::SwFieldDBPage(vcl::Window* pParent, const SfxItemSet *const pCoreSet)
    : SwFieldPage(pParent, "FieldDbPage",
        "modules/swriter/ui/flddbpage.ui", pCoreSet)
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

    m_pNumFormatLB->SetSelectHdl(LINK(this, SwFieldDBPage, NumSelectHdl));
    m_pDatabaseTLB->SetSelectHdl(LINK(this, SwFieldDBPage, TreeSelectHdl));
    m_pDatabaseTLB->SetDoubleClickHdl(LINK(this, SwFieldDBPage, TreeListBoxInsertHdl));

    m_pValueED->SetModifyHdl(LINK(this, SwFieldDBPage, ModifyHdl));
    m_pAddDBPB->SetClickHdl(LINK(this, SwFieldDBPage, AddDBHdl));
}

SwFieldDBPage::~SwFieldDBPage()
{
    disposeOnce();
}

void SwFieldDBPage::dispose()
{
    m_pTypeLB.clear();
    m_pDatabaseTLB.clear();
    m_pAddDBPB.clear();
    m_pCondition.clear();
    m_pConditionED.clear();
    m_pValue.clear();
    m_pValueED.clear();
    m_pDBFormatRB.clear();
    m_pNewFormatRB.clear();
    m_pNumFormatLB.clear();
    m_pFormatLB.clear();
    m_pFormat.clear();
    SwFieldPage::dispose();
}

// initialise TabPage
void SwFieldDBPage::Reset(const SfxItemSet*)
{
    Init(); // Allgemeine initialisierung

    m_pTypeLB->SetUpdateMode(false);
    const sal_Int32 nOldPos = m_pTypeLB->GetSelectEntryPos();
    m_sOldDBName = m_pDatabaseTLB->GetDBName(m_sOldTableName, m_sOldColumnName);

    m_pTypeLB->Clear();

    if (!IsFieldEdit())
    {
        // initialise TypeListBox
        const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

        for(sal_uInt16 i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            const sal_uInt16 nTypeId = SwFieldMgr::GetTypeId(i);
            const sal_Int32 nPos = m_pTypeLB->InsertEntry(SwFieldMgr::GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        const sal_uInt16 nTypeId = GetCurField()->GetTypeId();
        const sal_Int32 nPos = m_pTypeLB->InsertEntry(
            SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));
        m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
    }

    // select old Pos
    if (GetTypeSel() != LISTBOX_ENTRY_NOTFOUND)
        m_pTypeLB->SelectEntryPos(GetTypeSel());

    m_pFormatLB->Clear();

    const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(TYP_DBSETNUMBERFLD, false, IsFieldDlgHtmlMode());
    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        const sal_Int32 nEntryPos = m_pFormatLB->InsertEntry(GetFieldMgr().GetFormatStr(TYP_DBSETNUMBERFLD, i));
        const sal_uInt16 nFormatId = GetFieldMgr().GetFormatId( TYP_DBSETNUMBERFLD, i );
        m_pFormatLB->SetEntryData( nEntryPos, reinterpret_cast<void*>(nFormatId) );
        if( SVX_NUM_ARABIC == nFormatId )
            m_pFormatLB->SelectEntryPos( nEntryPos );
    }

    if (!IsFieldEdit())
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
                    if(nVal == (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(i)))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(nullptr);

    m_pTypeLB->SetUpdateMode(true);
    m_pTypeLB->SetSelectHdl(LINK(this, SwFieldDBPage, TypeListBoxHdl));
    m_pTypeLB->SetDoubleClickHdl(LINK(this, SwFieldDBPage, ListBoxInsertHdl));

    if (IsFieldEdit())
    {
        m_pConditionED->SaveValue();
        m_pValueED->SaveValue();
        m_sOldDBName = m_pDatabaseTLB->GetDBName(m_sOldTableName, m_sOldColumnName);
        m_nOldFormat = GetCurField()->GetFormat();
        m_nOldSubType = GetCurField()->GetSubType();
    }
}

bool SwFieldDBPage::FillItemSet(SfxItemSet* )
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
        const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));
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
            nFormat = reinterpret_cast<sal_uLong>(m_pFormatLB->GetEntryData(
                                m_pFormatLB->GetSelectEntryPos() ));
            break;
        }

        const OUString aVal(m_pValueED->GetText());
        OUString sTempTableName;
        OUString sTempColumnName;
        OUString sTempDBName = m_pDatabaseTLB->GetDBName(sTempTableName, sTempColumnName);
        bool bDBListBoxChanged = m_sOldDBName != sTempDBName ||
            m_sOldTableName != sTempTableName || m_sOldColumnName != sTempColumnName;
        if (!IsFieldEdit() ||
            m_pConditionED->IsValueChangedFromSaved() ||
            m_pValueED->GetSavedValue() != aVal ||
             bDBListBoxChanged ||
             m_nOldFormat != nFormat || m_nOldSubType != nSubType)
        {
            InsertField( nTypeId, nSubType, aName, aVal, nFormat);
        }
    }

    return false;
}

VclPtr<SfxTabPage> SwFieldDBPage::Create( vcl::Window* pParent,
                                        const SfxItemSet *const pAttrSet )
{
    return VclPtr<SwFieldDBPage>::Create( pParent, pAttrSet );
}

sal_uInt16 SwFieldDBPage::GetGroup()
{
    return GRP_DB;
}

IMPL_LINK_TYPED( SwFieldDBPage, TypeListBoxHdl, ListBox&, rBox, void )
{
    TypeHdl(&rBox);
}

void SwFieldDBPage::TypeHdl( ListBox* pBox )
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
        const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

        m_pDatabaseTLB->ShowColumns(nTypeId == TYP_DBFLD);

        if (IsFieldEdit())
        {
            SwDBData aData;
            OUString sColumnName;
            if (nTypeId == TYP_DBFLD)
            {
                aData = static_cast<SwDBField*>(GetCurField())->GetDBData();
                sColumnName = static_cast<SwDBFieldType*>(GetCurField()->GetTyp())->GetColumnName();
            }
            else
            {
                aData = static_cast<SwDBNameInfField*>(GetCurField())->GetDBData(pSh->GetDoc());
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

                if (IsFieldEdit())
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
                SAL_FALLTHROUGH;
            case TYP_DBNEXTSETFLD:
                bCond = true;
                if (IsFieldEdit())
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
                if( IsFieldEdit() )
                {
                    for( sal_Int32 nI = m_pFormatLB->GetEntryCount(); nI; )
                        if( GetCurField()->GetFormat() == reinterpret_cast<sal_uLong>(
                            m_pFormatLB->GetEntryData( --nI )))
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

        if (!IsFieldEdit())
        {
            m_pValueED->SetText(aEmptyOUStr);
            if (bCond)
                m_pConditionED->SetText("TRUE");
            else
                m_pConditionED->SetText(aEmptyOUStr);
        }

        CheckInsert();
    }
}

IMPL_LINK_TYPED( SwFieldDBPage, NumSelectHdl, ListBox&, rLB, void )
{
    m_pNewFormatRB->Check();
    m_aOldNumSelectHdl.Call(rLB);
}

void SwFieldDBPage::CheckInsert()
{
    bool bInsert = true;
    const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

    SvTreeListEntry* pEntry = m_pDatabaseTLB->GetCurEntry();

    if (pEntry)
    {
        pEntry = m_pDatabaseTLB->GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = m_pDatabaseTLB->GetParent(pEntry);

        bInsert &= pEntry != nullptr;
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

IMPL_LINK_TYPED( SwFieldDBPage, TreeSelectHdl, SvTreeListBox *, pBox, void )
{
    SvTreeListEntry* pEntry = pBox->GetCurEntry();
    if (pEntry)
    {
        const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

        pEntry = m_pDatabaseTLB->GetParent(pEntry);

        if (nTypeId == TYP_DBFLD && pEntry)
            pEntry = m_pDatabaseTLB->GetParent(pEntry);

        CheckInsert();

        if (nTypeId == TYP_DBFLD)
        {
            bool bNumFormat = false;

            if (pEntry != nullptr)
            {
                OUString sTableName;
                OUString sColumnName;
                sal_Bool bIsTable;
                OUString sDBName = m_pDatabaseTLB->GetDBName(sTableName, sColumnName, &bIsTable);
                bNumFormat = GetFieldMgr().IsDBNumeric(sDBName,
                            sTableName,
                            bIsTable,
                            sColumnName);
                if (!IsFieldEdit())
                    m_pDBFormatRB->Check();
            }

            m_pDBFormatRB->Enable(bNumFormat);
            m_pNewFormatRB->Enable(bNumFormat);
            m_pNumFormatLB->Enable(bNumFormat);
            m_pFormat->Enable(bNumFormat);
        }
    }
}

IMPL_LINK_NOARG_TYPED(SwFieldDBPage, AddDBHdl, Button*, void)
{
    OUString sNewDB = SwDBManager::LoadAndRegisterDataSource();
    if(!sNewDB.isEmpty())
    {
        m_pDatabaseTLB->AddDataSource(sNewDB);
    }
}

// Modify
IMPL_LINK_NOARG_TYPED(SwFieldDBPage, ModifyHdl, Edit&, void)
{
    CheckInsert();
}

void    SwFieldDBPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_pTypeLB->GetSelectEntryPos();
    const sal_uInt16 nTypeSel = ( LISTBOX_ENTRY_NOTFOUND == nEntryPos )
        ? USHRT_MAX : (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData( nEntryPos ));
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

void SwFieldDBPage::ActivateMailMergeAddress()
{
    sal_uLong nData = TYP_DBFLD;
    m_pTypeLB->SelectEntryPos(m_pTypeLB->GetEntryPos( reinterpret_cast<const void*>( nData ) ));
    m_pTypeLB->GetSelectHdl().Call(*m_pTypeLB);
    const SwDBData& rData = SW_MOD()->GetDBConfig()->GetAddressSource();
    m_pDatabaseTLB->Select(rData.sDataSource, rData.sCommand, aEmptyOUStr);
}

void SwFieldDBPage::SetWrtShell(SwWrtShell& rSh)
{
    m_pDatabaseTLB->SetWrtShell(rSh);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
