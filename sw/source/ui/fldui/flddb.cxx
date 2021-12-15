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
#include <wrtsh.hxx>
#include <dbfld.hxx>
#include <doc.hxx>

#include "flddb.hxx"
#include <dbconfig.hxx>
#include <dbmgr.hxx>

#define USER_DATA_VERSION_1     "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFieldDBPage::SwFieldDBPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *const pCoreSet)
    : SwFieldPage(pPage, pController, "modules/swriter/ui/flddbpage.ui", "FieldDbPage", pCoreSet)
    , m_nOldFormat(0)
    , m_nOldSubType(0)
    , m_xTypeLB(m_xBuilder->weld_tree_view("type"))
    , m_xDatabaseTLB(new SwDBTreeList(m_xBuilder->weld_tree_view("select")))
    , m_xAddDBPB(m_xBuilder->weld_button("browse"))
    , m_xCondition(m_xBuilder->weld_widget("condgroup"))
    , m_xConditionED(new ConditionEdit(m_xBuilder->weld_entry("condition")))
    , m_xValue(m_xBuilder->weld_widget("recgroup"))
    , m_xValueED(m_xBuilder->weld_entry("recnumber"))
    , m_xDBFormatRB(m_xBuilder->weld_radio_button("fromdatabasecb"))
    , m_xNewFormatRB(m_xBuilder->weld_radio_button("userdefinedcb"))
    , m_xNumFormatLB(new NumFormatListBox(m_xBuilder->weld_combo_box("numformat")))
    , m_xFormatLB(m_xBuilder->weld_combo_box("format"))
    , m_xFormat(m_xBuilder->weld_widget("formatframe"))
{
    SetTypeSel(-1); //TODO

    m_xTypeLB->make_sorted();
    m_xFormatLB->make_sorted();

    auto nWidth = m_xTypeLB->get_approximate_digit_width() * FIELD_COLUMN_WIDTH;
    auto nHeight = m_xTypeLB->get_height_rows(10);
    m_xTypeLB->set_size_request(nWidth, nHeight);
    m_xDatabaseTLB->set_size_request(nWidth*2, nHeight);

    m_xNumFormatLB->connect_changed(LINK(this, SwFieldDBPage, NumSelectHdl));
    m_xDatabaseTLB->connect_changed(LINK(this, SwFieldDBPage, TreeSelectHdl));
    m_xDatabaseTLB->connect_row_activated(LINK(this, SwFieldDBPage, TreeViewInsertHdl));

    m_xValueED->connect_changed(LINK(this, SwFieldDBPage, ModifyHdl));
    m_xAddDBPB->connect_clicked(LINK(this, SwFieldDBPage, AddDBHdl));

    // uitests
    m_xTypeLB->set_buildable_name(m_xTypeLB->get_buildable_name() + "-db");
}

SwFieldDBPage::~SwFieldDBPage()
{
    // If we have no stored SwWrtShell, it means we didn't do anything useful - no need to revoke.
    if (SwWrtShell* pSh = GetWrtShell())
    {
        // This would cleanup in the case of cancelled dialog
        SwDBManager* pDbManager = pSh->GetDoc()->GetDBManager();
        if (pDbManager)
            pDbManager->RevokeLastRegistrations();
    }
}

// initialise TabPage
void SwFieldDBPage::Reset(const SfxItemSet*)
{
    Init(); // general initialization

    const sal_Int32 nOldPos = m_xTypeLB->get_selected_index();
    m_xTypeLB->freeze();
    m_sOldDBName = m_xDatabaseTLB->GetDBName(m_sOldTableName, m_sOldColumnName);

    m_xTypeLB->clear();

    if (!IsFieldEdit())
    {
        // initialise TypeListBox
        const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

        for(sal_uInt16 i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            const SwFieldTypesEnum nTypeId = SwFieldMgr::GetTypeId(i);
            m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(i));
        }
    }
    else
    {
        const SwFieldTypesEnum nTypeId = GetCurField()->GetTypeId();
        m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)),
                          SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));
    }

    m_xTypeLB->thaw();

    // select old Pos
    if (GetTypeSel() != -1)
        m_xTypeLB->select(GetTypeSel());

    m_xFormatLB->clear();

    const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(SwFieldTypesEnum::DatabaseSetNumber, IsFieldDlgHtmlMode());
    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        const sal_uInt16 nFormatId = GetFieldMgr().GetFormatId( SwFieldTypesEnum::DatabaseSetNumber, i );
        OUString sId(OUString::number(nFormatId));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr(SwFieldTypesEnum::DatabaseSetNumber, i));
        if (SVX_NUM_ARABIC == nFormatId)
            m_xFormatLB->set_active_id(sId);
    }

    if (!IsFieldEdit())
    {
        if (nOldPos != -1)
            m_xTypeLB->select(nOldPos);

        if (!m_sOldDBName.isEmpty())
        {
            m_xDatabaseTLB->Select(m_sOldDBName, m_sOldTableName, m_sOldColumnName);
        }
        else
        {
            SwWrtShell *pSh = CheckAndGetWrtShell();
            if(pSh)
            {
                SwDBData aTmp(pSh->GetDBData());
                m_xDatabaseTLB->Select(aTmp.sDataSource, aTmp.sCommand, u"");
            }
        }
    }

    if( !IsRefresh() )
    {
        const OUString sUserData = GetUserData();
        sal_Int32 nIdx{ 0 };
        if (sUserData.getToken(0, ';', nIdx).equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            const sal_uInt16 nVal = o3tl::narrowing<sal_uInt16>(sUserData.getToken(0, ';', nIdx).toInt32());
            if (nVal != USHRT_MAX)
            {
                for (sal_Int32 i = 0, nEntryCount = m_xTypeLB->n_children(); i < nEntryCount; ++i)
                {
                    if (nVal == m_xTypeLB->get_id(i).toUInt32())
                    {
                        m_xTypeLB->select(i);
                        break;
                    }
                }
            }
        }
    }
    TypeHdl(nullptr);

    m_xTypeLB->connect_changed(LINK(this, SwFieldDBPage, TypeListBoxHdl));
    m_xTypeLB->connect_row_activated(LINK(this, SwFieldDBPage, TreeViewInsertHdl));

    if (IsFieldEdit())
    {
        m_xConditionED->save_value();
        m_xValueED->save_value();
        m_sOldDBName = m_xDatabaseTLB->GetDBName(m_sOldTableName, m_sOldColumnName);
        m_nOldFormat = GetCurField()->GetFormat();
        m_nOldSubType = GetCurField()->GetSubType();
    }
}

// SwFieldDBPage may ask for password to select current document's data source,
// so only do that when activating the page, not when dialog is creating all pages
bool SwFieldDBPage::DeferResetToFirstActivation() { return true; }

bool SwFieldDBPage::FillItemSet(SfxItemSet* )
{
    OUString sTableName;
    OUString sColumnName;
    SwDBData aData;
    sal_Bool bIsTable;
    aData.sDataSource = m_xDatabaseTLB->GetDBName(sTableName, sColumnName, &bIsTable);
    aData.sCommand = sTableName;
    aData.nCommandType = bIsTable ? 0 : 1;
    SwWrtShell *pSh = CheckAndGetWrtShell();
    assert(pSh);

    SwDBManager* pDbManager = pSh->GetDoc()->GetDBManager();
    if (pDbManager)
        pDbManager->CommitLastRegistrations();

    if (aData.sDataSource.isEmpty())
        aData = pSh->GetDBData();

    if(!aData.sDataSource.isEmpty())       // without database no new field command
    {
        const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());
        sal_uLong nFormat = 0;
        sal_uInt16 nSubType = 0;

        OUString sDBName = aData.sDataSource
            + OUStringChar(DB_DELIM)
            + aData.sCommand
            + OUStringChar(DB_DELIM)
            + OUString::number(aData.nCommandType)
            + OUStringChar(DB_DELIM);
        if (!sColumnName.isEmpty())
        {
            sDBName += sColumnName + OUStringChar(DB_DELIM);
        }
        OUString aName = sDBName + m_xConditionED->get_text();

        switch (nTypeId)
        {
        case SwFieldTypesEnum::Database:
            nFormat = m_xNumFormatLB->GetFormat();
            if (m_xNewFormatRB->get_sensitive() && m_xNewFormatRB->get_active())
                nSubType = nsSwExtendedSubType::SUB_OWN_FMT;
            aName = sDBName;
            break;

        case SwFieldTypesEnum::DatabaseSetNumber:
            nFormat = m_xFormatLB->get_active_id().toUInt32();
            break;
        default: break;
        }

        const OUString aVal(m_xValueED->get_text());
        OUString sTempTableName;
        OUString sTempColumnName;
        OUString sTempDBName = m_xDatabaseTLB->GetDBName(sTempTableName, sTempColumnName);
        bool bDBListBoxChanged = m_sOldDBName != sTempDBName ||
            m_sOldTableName != sTempTableName || m_sOldColumnName != sTempColumnName;
        if (!IsFieldEdit() ||
            m_xConditionED->get_value_changed_from_saved() ||
            m_xValueED->get_saved_value() != aVal ||
             bDBListBoxChanged ||
             m_nOldFormat != nFormat || m_nOldSubType != nSubType)
        {
            InsertField( nTypeId, nSubType, aName, aVal, nFormat);
        }
    }

    return false;
}

std::unique_ptr<SfxTabPage> SwFieldDBPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                        const SfxItemSet *const pAttrSet )
{
    return std::make_unique<SwFieldDBPage>( pPage, pController, pAttrSet );
}

sal_uInt16 SwFieldDBPage::GetGroup()
{
    return GRP_DB;
}

IMPL_LINK( SwFieldDBPage, TypeListBoxHdl, weld::TreeView&, rBox, void )
{
    TypeHdl(&rBox);
}

void SwFieldDBPage::TypeHdl(const weld::TreeView* pBox)
{
    // save old ListBoxPos
    const sal_Int32 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_xTypeLB->get_selected_index());

    if (GetTypeSel() == -1)
    {
        SetTypeSel(0);
        m_xTypeLB->select(0);
    }

    if (nOld == GetTypeSel())
        return;

    SwWrtShell *pSh = CheckAndGetWrtShell();
    assert(pSh);
    bool bCond = false, bSetNo = false, bFormat = false, bDBFormat = false;
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    m_xDatabaseTLB->ShowColumns(nTypeId == SwFieldTypesEnum::Database);

    if (IsFieldEdit())
    {
        SwDBData aData;
        OUString sColumnName;
        if (nTypeId == SwFieldTypesEnum::Database)
        {
            aData = static_cast<SwDBField*>(GetCurField())->GetDBData();
            sColumnName = static_cast<SwDBFieldType*>(GetCurField()->GetTyp())->GetColumnName();
        }
        else
        {
            aData = static_cast<SwDBNameInfField*>(GetCurField())->GetDBData(pSh->GetDoc());
        }
        m_xDatabaseTLB->Select(aData.sDataSource, aData.sCommand, sColumnName);
    }

    switch (nTypeId)
    {
        case SwFieldTypesEnum::Database:
        {
            bFormat = true;
            bDBFormat = true;
            m_xNumFormatLB->show();
            m_xFormatLB->hide();

            weld::Widget& rWidget = m_xNumFormatLB->get_widget();
            m_xNewFormatRB->set_accessible_relation_label_for(&rWidget);
            rWidget.set_accessible_relation_labeled_by(m_xNewFormatRB.get());
            m_xFormatLB->set_accessible_relation_label_for(nullptr);

            if (pBox)   // type was changed by user
                m_xDBFormatRB->set_active(true);

            if (IsFieldEdit())
            {
                if (GetCurField()->GetFormat() != 0 && GetCurField()->GetFormat() != SAL_MAX_UINT32)
                    m_xNumFormatLB->SetDefFormat(GetCurField()->GetFormat());

                if (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_OWN_FMT)
                    m_xNewFormatRB->set_active(true);
                else
                    m_xDBFormatRB->set_active(true);
            }
            break;
        }
        case SwFieldTypesEnum::DatabaseNumberSet:
            bSetNo = true;
            [[fallthrough]];
        case SwFieldTypesEnum::DatabaseNextSet:
            bCond = true;
            if (IsFieldEdit())
            {
                m_xConditionED->set_text(GetCurField()->GetPar1());
                m_xValueED->set_text(GetCurField()->GetPar2());
            }
            break;

        case SwFieldTypesEnum::DatabaseName:
            break;

        case SwFieldTypesEnum::DatabaseSetNumber:
        {
            bFormat = true;
            m_xNewFormatRB->set_active(true);
            m_xNumFormatLB->hide();
            m_xFormatLB->show();

            m_xNewFormatRB->set_accessible_relation_label_for(m_xFormatLB.get());
            m_xFormatLB->set_accessible_relation_labeled_by(m_xNewFormatRB.get());
            weld::Widget& rWidget = m_xNumFormatLB->get_widget();
            rWidget.set_accessible_relation_label_for(nullptr);

            if( IsFieldEdit() )
            {
                for (sal_Int32 nI = m_xFormatLB->get_count(); nI;)
                {
                    if (GetCurField()->GetFormat() == m_xFormatLB->get_id(--nI).toUInt32())
                    {
                        m_xFormatLB->set_active( nI );
                        break;
                    }
                }
            }
            break;
        }
        default: break;
    }

    m_xCondition->set_sensitive(bCond);
    m_xValue->set_sensitive(bSetNo);
    if (nTypeId != SwFieldTypesEnum::Database)
    {
        m_xDBFormatRB->set_sensitive(bDBFormat);
        m_xNewFormatRB->set_sensitive(bDBFormat || bFormat);
        m_xNumFormatLB->set_sensitive(bDBFormat);
        m_xFormatLB->set_sensitive(bFormat);
    }
    m_xFormat->set_sensitive(bDBFormat || bFormat);

    if (!IsFieldEdit())
    {
        m_xValueED->set_text(OUString());
        if (bCond)
            m_xConditionED->set_text("TRUE");
        else
            m_xConditionED->set_text(OUString());
    }

    CheckInsert();
}

IMPL_LINK_NOARG(SwFieldDBPage, NumSelectHdl, weld::ComboBox&, void)
{
    m_xNewFormatRB->set_active(true);
    m_xNumFormatLB->CallSelectHdl();
}

void SwFieldDBPage::CheckInsert()
{
    bool bInsert = true;
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    std::unique_ptr<weld::TreeIter> xIter(m_xDatabaseTLB->make_iterator());
    if (m_xDatabaseTLB->get_selected(xIter.get()))
    {
        bool bEntry = m_xDatabaseTLB->iter_parent(*xIter);

        if (nTypeId == SwFieldTypesEnum::Database && bEntry)
            bEntry = m_xDatabaseTLB->iter_parent(*xIter);

        bInsert &= bEntry;
    }
    else
        bInsert = false;

    if (nTypeId == SwFieldTypesEnum::DatabaseNumberSet)
    {
        bool bHasValue = !m_xValueED->get_text().isEmpty();

        bInsert &= bHasValue;
    }

    EnableInsert(bInsert);
}

IMPL_LINK(SwFieldDBPage, TreeSelectHdl, weld::TreeView&, rBox, void)
{
    std::unique_ptr<weld::TreeIter> xIter(rBox.make_iterator());
    if (!rBox.get_selected(xIter.get()))
        return;

    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    bool bEntry = m_xDatabaseTLB->iter_parent(*xIter);

    if (nTypeId == SwFieldTypesEnum::Database && bEntry)
        bEntry = m_xDatabaseTLB->iter_parent(*xIter);

    CheckInsert();

    if (nTypeId != SwFieldTypesEnum::Database)
        return;

    bool bNumFormat = false;

    if (bEntry)
    {
        OUString sTableName;
        OUString sColumnName;
        sal_Bool bIsTable;
        OUString sDBName = m_xDatabaseTLB->GetDBName(sTableName, sColumnName, &bIsTable);
        bNumFormat = GetFieldMgr().IsDBNumeric(sDBName,
                    sTableName,
                    bIsTable,
                    sColumnName);
        if (!IsFieldEdit())
            m_xDBFormatRB->set_active(true);
    }

    m_xDBFormatRB->set_sensitive(bNumFormat);
    m_xNewFormatRB->set_sensitive(bNumFormat);
    m_xNumFormatLB->set_sensitive(bNumFormat);
    m_xFormat->set_sensitive(bNumFormat);
}

IMPL_LINK_NOARG(SwFieldDBPage, AddDBHdl, weld::Button&, void)
{
    if (SwWrtShell* pSh = CheckAndGetWrtShell())
    {
        OUString sNewDB
            = SwDBManager::LoadAndRegisterDataSource(GetFrameWeld(), pSh->GetDoc()->GetDocShell());
        if (!sNewDB.isEmpty())
        {
            m_xDatabaseTLB->AddDataSource(sNewDB);
        }
    }
}

// Modify
IMPL_LINK_NOARG(SwFieldDBPage, ModifyHdl, weld::Entry&, void)
{
    CheckInsert();
}

void    SwFieldDBPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_xTypeLB->get_selected_index();
    const sal_uInt16 nTypeSel = ( -1 == nEntryPos )
        ? USHRT_MAX : m_xTypeLB->get_id(nEntryPos).toUInt32();
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

void SwFieldDBPage::ActivateMailMergeAddress()
{
    m_xTypeLB->select_id(OUString::number(static_cast<sal_uInt16>(SwFieldTypesEnum::Database)));
    TypeListBoxHdl(*m_xTypeLB);
    const SwDBData& rData = SW_MOD()->GetDBConfig()->GetAddressSource();
    m_xDatabaseTLB->Select(rData.sDataSource, rData.sCommand, u"");
}

void SwFieldDBPage::SetWrtShell(SwWrtShell& rSh)
{
    // We need to remember the shell to be able to call correct SwDBManager
    SwFieldPage::SetWrtShell(&rSh);
    m_xDatabaseTLB->SetWrtShell(rSh);
}

SwWrtShell* SwFieldDBPage::CheckAndGetWrtShell()
{
    SwWrtShell* pSh = GetWrtShell();
    if (!pSh)
    {
        pSh = ::GetActiveWrtShell();
        if (pSh) // this is not guaranteed: e.g., activating print preview with dialog active
            SetWrtShell(*pSh);
    }
    return pSh;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
