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

#include <flddat.hxx>
#include <docufld.hxx>
#include <strings.hrc>
#include <chpfld.hxx>
#include "flddok.hxx"
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFieldDokPage::SwFieldDokPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *const pCoreSet)
    : SwFieldPage(pPage, pController, "modules/swriter/ui/flddocumentpage.ui",
                  "FieldDocumentPage", pCoreSet)
    , nOldSel(0)
    , nOldFormat(0)
    , m_xTypeLB(m_xBuilder->weld_tree_view("type"))
    , m_xSelection(m_xBuilder->weld_widget("selectframe"))
    , m_xSelectionLB(m_xBuilder->weld_tree_view("select"))
    , m_xValueFT(m_xBuilder->weld_label("valueft"))
    , m_xValueED(m_xBuilder->weld_entry("value"))
    , m_xLevelFT(m_xBuilder->weld_label("levelft"))
    , m_xLevelED(m_xBuilder->weld_spin_button("level"))
    , m_xDateFT(m_xBuilder->weld_label("daysft"))
    , m_xTimeFT(m_xBuilder->weld_label("minutesft"))
    , m_xDateOffsetED(m_xBuilder->weld_spin_button("offset"))
    , m_xFormat(m_xBuilder->weld_widget("formatframe"))
    , m_xFormatLB(m_xBuilder->weld_tree_view("format"))
    , m_xNumFormatLB(new SwNumFormatTreeView(m_xBuilder->weld_tree_view("numformat")))
    , m_xFixedCB(m_xBuilder->weld_check_button("fixed"))
{
    m_xTypeLB->make_sorted();
    m_xFormatLB->make_sorted();

    auto nWidth = m_xTypeLB->get_approximate_digit_width() * FIELD_COLUMN_WIDTH;
    auto nHeight = m_xTypeLB->get_height_rows(10);

    m_xTypeLB->set_size_request(nWidth, nHeight);
    m_xSelectionLB->set_size_request(nWidth, nHeight);
    m_xFormatLB->set_size_request(nWidth * 2, nHeight);

    m_xSelectionLB->connect_row_activated(LINK(this, SwFieldDokPage, TreeViewInsertHdl));
    m_xFormatLB->connect_row_activated(LINK(this, SwFieldDokPage, TreeViewInsertHdl));
    m_xNumFormatLB->connect_row_activated(LINK(this, SwFieldDokPage, NumFormatHdl));

    m_xLevelED->set_max(MAXLEVEL);
    m_xDateOffsetED->set_range(INT_MIN, INT_MAX);
    //enable 'active' language selection
    m_xNumFormatLB->SetShowLanguageControl(true);

    // uitests
    m_xTypeLB->set_buildable_name(m_xTypeLB->get_buildable_name() + "-doc");
}

SwFieldDokPage::~SwFieldDokPage()
{
}

void SwFieldDokPage::Reset(const SfxItemSet* )
{
    SavePos(*m_xTypeLB);
    Init(); // general initialisation

    // initialise TypeListBox
    const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

    m_xTypeLB->freeze();
    m_xTypeLB->clear();

    if (!IsFieldEdit())
    {
        bool bPage = false;
        // fill Type-Listbox
        for(sal_uInt16 i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            const SwFieldTypesEnum nTypeId = SwFieldMgr::GetTypeId(i);

            switch (nTypeId)
            {
                case SwFieldTypesEnum::PreviousPage:
                case SwFieldTypesEnum::NextPage:
                case SwFieldTypesEnum::PageNumber:
                    if (!bPage)
                    {
                        m_xTypeLB->append(OUString::number(USHRT_MAX), SwResId(FMT_REF_PAGE));
                        bPage = true;
                    }
                    break;

                default:
                    m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(i));
                    break;
            }
        }
    }
    else
    {
        const SwField* pCurField = GetCurField();
        SwFieldTypesEnum nTypeId = pCurField->GetTypeId();
        if (nTypeId == SwFieldTypesEnum::FixedDate)
            nTypeId = SwFieldTypesEnum::Date;
        if (nTypeId == SwFieldTypesEnum::FixedTime)
            nTypeId = SwFieldTypesEnum::Time;
        m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));
        m_xNumFormatLB->SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
            pSh = ::GetActiveWrtShell();
        if(pSh)
        {
            const SvNumberformat* pFormat = pSh->GetNumberFormatter()->GetEntry(pCurField->GetFormat());
            if(pFormat)
                m_xNumFormatLB->SetLanguage(pFormat->GetLanguage());
        }
    }


    m_xTypeLB->thaw();

    // select old Pos
    RestorePos(*m_xTypeLB);

    m_xTypeLB->connect_row_activated(LINK(this, SwFieldDokPage, TreeViewInsertHdl));
    m_xTypeLB->connect_changed(LINK(this, SwFieldDokPage, TypeHdl));
    m_xFormatLB->connect_changed(LINK(this, SwFieldDokPage, FormatHdl));

    if( !IsRefresh() )
    {
        const OUString sUserData = GetUserData();
        sal_Int32 nIdx{ 0 };
        if (sUserData.getToken(0, ';', nIdx).equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            const sal_uInt16 nVal = static_cast< sal_uInt16 >(sUserData.getToken(0, ';', nIdx).toInt32());
            if(nVal != USHRT_MAX)
            {
                for (int i = 0, nEntryCount = m_xTypeLB->n_children(); i < nEntryCount; i++)
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
    TypeHdl(*m_xTypeLB);

    if (IsFieldEdit())
    {
        nOldSel = m_xSelectionLB->get_selected_index();
        nOldFormat = GetCurField()->GetFormat();
        m_xFixedCB->save_state();
        m_xValueED->save_value();
        m_xLevelED->save_value();
        m_xDateOffsetED->save_value();
    }
}

IMPL_LINK_NOARG(SwFieldDokPage, TypeHdl, weld::TreeView&, void)
{
    // save old ListBoxPos
    const sal_Int32 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_xTypeLB->get_selected_index());

    if(GetTypeSel() == -1)
    {
        SetTypeSel(0);
        m_xTypeLB->select(0);
    }

    if (nOld == GetTypeSel())
        return;

    size_t nCount;

    m_xDateFT->hide();
    m_xTimeFT->hide();

    SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    // fill Selection-Listbox
    m_xSelectionLB->clear();

    if (nTypeId != SwFieldTypesEnum::Unknown)
    {
        std::vector<OUString> aLst;
        GetFieldMgr().GetSubTypes(nTypeId, aLst);

        if (nTypeId != SwFieldTypesEnum::Author)
            nCount = aLst.size();
        else
            nCount = GetFieldMgr().GetFormatCount(nTypeId, IsFieldDlgHtmlMode());

        for (size_t i = 0; i < nCount; ++i)
        {
            if (!IsFieldEdit())
            {
                OUString sId(OUString::number(i));
                if (nTypeId != SwFieldTypesEnum::Author)
                    m_xSelectionLB->append(sId, aLst[i]);
                else
                    m_xSelectionLB->append(sId, GetFieldMgr().GetFormatStr(nTypeId, i));
            }
            else
            {
                bool bInsert = false;

                OUString sId(OUString::number(i));

                switch (nTypeId)
                {
                    case SwFieldTypesEnum::Date:
                    case SwFieldTypesEnum::Time:
                        m_xSelectionLB->append(sId, aLst[i]);
                        if (static_cast<SwDateTimeField*>(GetCurField())->IsFixed() && !i)
                            m_xSelectionLB->select_id(sId);
                        if (!static_cast<SwDateTimeField*>(GetCurField())->IsFixed() && i)
                            m_xSelectionLB->select_id(sId);
                        break;
                    case SwFieldTypesEnum::ExtendedUser:
                    case SwFieldTypesEnum::DocumentStatistics:
                        m_xSelectionLB->append(sId, aLst[i]);
                        if (GetCurField()->GetSubType() == i)
                            m_xSelectionLB->select_id(sId);
                        break;

                    case SwFieldTypesEnum::Author:
                    {
                        const OUString sFormat(GetFieldMgr().GetFormatStr(nTypeId, i));
                        m_xSelectionLB->append(sId, sFormat);
                        m_xSelectionLB->select_text(GetFieldMgr().GetFormatStr(nTypeId, GetCurField()->GetFormat()));
                        break;
                    }

                    default:
                        if (aLst[i] == GetCurField()->GetPar1())
                            bInsert = true;
                        break;
                }
                if (bInsert)
                {
                    m_xSelectionLB->append(sId, aLst[i]);
                    break;
                }
            }
        }
        m_xSelectionLB->connect_changed(Link<weld::TreeView&,void>());
    }
    else
    {
        AddSubType(SwFieldTypesEnum::PageNumber);
        AddSubType(SwFieldTypesEnum::PreviousPage);
        AddSubType(SwFieldTypesEnum::NextPage);
        nTypeId = static_cast<SwFieldTypesEnum>(m_xSelectionLB->get_id(0).toUInt32());
        nCount = 3;
        m_xSelectionLB->connect_changed(LINK(this, SwFieldDokPage, SubTypeHdl));
    }

    bool bEnable = nCount != 0;

    if (bEnable && m_xSelectionLB->get_selected_index() == -1)
        m_xSelectionLB->select(0);

    m_xSelection->set_sensitive( bEnable );

    // fill Format-Listbox
    sal_Int32 nSize = FillFormatLB(nTypeId);

    bool bValue = false, bLevel = false, bNumFormat = false, bOffset = false;
    bool bFormat = nSize != 0;
    bool bOneArea = false;
    bool bFixed = false;
    SvNumFormatType nFormatType = SvNumFormatType::ALL;

    switch (nTypeId)
    {
        case SwFieldTypesEnum::Date:
            bFormat = bNumFormat = bOneArea = bOffset = true;

            nFormatType = SvNumFormatType::DATE;

            m_xDateFT->show();

            m_xDateOffsetED->set_range(INT_MIN, INT_MAX);    // no limit

            if (IsFieldEdit())
                m_xDateOffsetED->set_value( static_cast<SwDateTimeField*>(GetCurField())->GetOffset() / 24 / 60);
            break;

        case SwFieldTypesEnum::Time:
            bFormat = bNumFormat = bOneArea = bOffset = true;

            nFormatType = SvNumFormatType::TIME;

            m_xTimeFT->show();

            m_xDateOffsetED->set_range(-1440, 1440);  // one day

            if (IsFieldEdit())
                m_xDateOffsetED->set_value( static_cast<SwDateTimeField*>(GetCurField())->GetOffset() );
            break;

        case SwFieldTypesEnum::PreviousPage:
        case SwFieldTypesEnum::NextPage:
            if (IsFieldEdit())
            {
                const sal_uInt16 nTmp = m_xFormatLB->get_selected_id().toUInt32();

                if(SVX_NUM_CHAR_SPECIAL != nTmp)
                {
                    sal_Int32 nOff = GetCurField()->GetPar2().toInt32();
                    if( SwFieldTypesEnum::NextPage == nTypeId && 1 != nOff )
                        m_xValueED->set_text(
                            OUString::number(nOff - 1) );
                    else if( SwFieldTypesEnum::PreviousPage == nTypeId && -1 != nOff )
                        m_xValueED->set_text(
                            OUString::number(nOff + 1) );
                    else
                        m_xValueED->set_text(OUString());
                }
                else
                    m_xValueED->set_text(static_cast<SwPageNumberField*>(GetCurField())->GetUserString());
            }
            bValue = true;
            break;

        case SwFieldTypesEnum::Chapter:
            m_xValueFT->set_label(SwResId(STR_LEVEL));
            if (IsFieldEdit())
                m_xLevelED->set_text(OUString::number(static_cast<SwChapterField*>(GetCurField())->GetLevel(GetWrtShell()->GetLayout()) + 1));
            bLevel = true;
            break;

        case SwFieldTypesEnum::PageNumber:
            m_xValueFT->set_label( SwResId( STR_OFFSET ));
            if (IsFieldEdit())
                m_xValueED->set_text(GetCurField()->GetPar2());
            bValue = true;
            break;

        case SwFieldTypesEnum::ExtendedUser:
        case SwFieldTypesEnum::Author:
        case SwFieldTypesEnum::Filename:
            bFixed = true;
            break;

        default:
            break;
    }

    if (bNumFormat)
    {
        if (IsFieldEdit())
        {
            m_xNumFormatLB->SetDefFormat(GetCurField()->GetFormat());

            if (m_xNumFormatLB->GetFormatType() == (SvNumFormatType::DATE|SvNumFormatType::TIME))
            {
                // always set Format-Type because otherwise when date/time formats are combined,
                // both formats would be displayed at the same time
                m_xNumFormatLB->SetFormatType(SvNumFormatType::ALL);
                m_xNumFormatLB->SetFormatType(nFormatType);
                // set correct format once again
                m_xNumFormatLB->SetDefFormat(GetCurField()->GetFormat());
            }
        }
        else
            m_xNumFormatLB->SetFormatType(nFormatType);

        m_xNumFormatLB->SetOneArea(bOneArea);
    }

    m_xFormatLB->set_visible(!bNumFormat);
    m_xNumFormatLB->set_visible(bNumFormat);

    m_xValueFT->set_visible(bValue);
    m_xValueED->set_visible(bValue);
    m_xLevelFT->set_visible(bLevel);
    m_xLevelED->set_visible(bLevel);
    m_xDateOffsetED->set_visible(bOffset);
    m_xFixedCB->set_visible(!bValue && !bLevel && !bOffset);

    m_xFormat->set_sensitive(bFormat);
    m_xFixedCB->set_sensitive(bFixed);

    if (IsFieldEdit())
        m_xFixedCB->set_active((GetCurField()->GetFormat() & AF_FIXED) != 0 && bFixed);

    if (m_xNumFormatLB->get_selected_index() == -1)
        m_xNumFormatLB->select(0);
    m_xValueFT->set_sensitive(bValue || bLevel || bOffset);
    m_xValueED->set_sensitive(bValue);
}

void SwFieldDokPage::AddSubType(SwFieldTypesEnum nTypeId)
{
    m_xSelectionLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldType::GetTypeStr(nTypeId));
}

IMPL_LINK_NOARG(SwFieldDokPage, SubTypeHdl, weld::TreeView&, void)
{
    sal_Int32 nPos = m_xSelectionLB->get_selected_index();
    if(nPos == -1)
        nPos = 0;

    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xSelectionLB->get_id(nPos).toUInt32());
    FillFormatLB(nTypeId);

    TranslateId pTextRes;
    switch (nTypeId)
    {
    case SwFieldTypesEnum::Chapter:
        pTextRes = STR_LEVEL;
        break;

    case SwFieldTypesEnum::PreviousPage:
    case SwFieldTypesEnum::NextPage:
        pTextRes = SVX_NUM_CHAR_SPECIAL == m_xFormatLB->get_selected_id().toUInt32()
                        ? STR_VALUE : STR_OFFSET;
        break;

    case SwFieldTypesEnum::PageNumber:
        pTextRes = STR_OFFSET;
        break;
    default: break;
    }

    if (pTextRes)
        m_xValueFT->set_label(SwResId(pTextRes));
}

sal_Int32 SwFieldDokPage::FillFormatLB(SwFieldTypesEnum nTypeId)
{
    // fill Format-Listbox
    m_xFormatLB->clear();

    if (nTypeId == SwFieldTypesEnum::Author)
        return m_xFormatLB->n_children();

    const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(nTypeId, IsFieldDlgHtmlMode());

    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        const sal_uInt16 nFormatId = GetFieldMgr().GetFormatId( nTypeId, i );
        OUString sId(OUString::number(nFormatId));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr(nTypeId, i));
    }

    if (IsFieldEdit())
    {
        m_xFormatLB->select_id(OUString::number(GetCurField()->GetFormat() & ~AF_FIXED));
    }

    FormatHdl(*m_xFormatLB);

    return nSize;
}

IMPL_LINK_NOARG(SwFieldDokPage, FormatHdl, weld::TreeView&, void)
{
    SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    if (nTypeId == SwFieldTypesEnum::Unknown)
    {
        sal_Int32 nPos = m_xSelectionLB->get_selected_index();
        if(nPos == -1)
            nPos = 0;

        nTypeId = static_cast<SwFieldTypesEnum>(m_xSelectionLB->get_id(nPos).toUInt32());
    }

    if (nTypeId != SwFieldTypesEnum::NextPage && nTypeId != SwFieldTypesEnum::PreviousPage)
        return;

    // Prev/Next - PageNumFields special treatment:
    sal_uInt16 nTmp = m_xFormatLB->get_selected_id().toUInt32();
    const OUString sOldText( m_xValueFT->get_label() );
    const OUString sNewText( SwResId( SVX_NUM_CHAR_SPECIAL == nTmp  ? STR_VALUE
                                                     : STR_OFFSET ));

    if (sOldText != sNewText)
        m_xValueFT->set_label(sNewText);

    if (sOldText != m_xValueFT->get_label())
        m_xValueED->set_text(OUString());
}

bool SwFieldDokPage::FillItemSet(SfxItemSet* )
{
    SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    if (nTypeId == SwFieldTypesEnum::Unknown)
    {
        sal_Int32 nPos = m_xSelectionLB->get_selected_index();
        if(nPos == -1)
            nPos = 0;
        nTypeId = static_cast<SwFieldTypesEnum>(m_xSelectionLB->get_id(nPos).toUInt32());
    }

    OUString aVal(m_xValueED->get_text());
    sal_uLong nFormat = 0;
    sal_uInt16 nSubType = 0;

    if (m_xFormatLB->get_sensitive())
    {
        sal_Int32 nPos = m_xFormatLB->get_selected_index();
        if(nPos != -1)
            nFormat = m_xFormatLB->get_id(nPos).toUInt32();
    }

    if (m_xSelectionLB->get_sensitive())
    {
        sal_Int32 nPos = m_xSelectionLB->get_selected_index();
        if(nPos != -1)
            nSubType = m_xSelectionLB->get_id(nPos).toUInt32();
    }

    switch (nTypeId)
    {
        case SwFieldTypesEnum::Author:
            nFormat = nSubType;
            nSubType = 0;
            [[fallthrough]];
        case SwFieldTypesEnum::ExtendedUser:
            nFormat |= m_xFixedCB->get_active() ? AF_FIXED : 0;
            break;

        case SwFieldTypesEnum::Filename:
            nFormat |= m_xFixedCB->get_active() ? FF_FIXED : 0;
            break;

        case SwFieldTypesEnum::Date:
        case SwFieldTypesEnum::Time:
        {
            nFormat = m_xNumFormatLB->GetFormat();
            tools::Long nVal = static_cast< tools::Long >(m_xDateOffsetED->get_value());
            if (nTypeId == SwFieldTypesEnum::Date)
                aVal = OUString::number(nVal * 60 * 24);
            else
                aVal = OUString::number(nVal);
            break;
        }

        case SwFieldTypesEnum::NextPage:
        case SwFieldTypesEnum::PreviousPage:
        case SwFieldTypesEnum::PageNumber:
        case SwFieldTypesEnum::GetRefPage:
        {
            if( SVX_NUM_CHAR_SPECIAL != nFormat &&
                (SwFieldTypesEnum::PreviousPage == nTypeId || SwFieldTypesEnum::NextPage == nTypeId))
            {
                sal_Int32 nVal = m_xValueED->get_text().toInt32();
                aVal = OUString::number(nVal);
            }
            break;
        }

        case SwFieldTypesEnum::Chapter:
            aVal = m_xLevelED->get_text();
            break;

        default:
            break;
    }

    if (!IsFieldEdit() ||
        nOldSel != m_xSelectionLB->get_selected_index() ||
        nOldFormat != nFormat ||
        m_xFixedCB->get_state_changed_from_saved() ||
        m_xValueED->get_value_changed_from_saved() ||
        m_xLevelED->get_value_changed_from_saved() ||
        m_xDateOffsetED->get_value_changed_from_saved())
    {
        InsertField(nTypeId, nSubType, OUString(), aVal, nFormat, ' ', m_xNumFormatLB->IsAutomaticLanguage());
    }

    return false;
}

std::unique_ptr<SfxTabPage> SwFieldDokPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet *const pAttrSet)
{
    return std::make_unique<SwFieldDokPage>(pPage, pController, pAttrSet);
}

sal_uInt16 SwFieldDokPage::GetGroup()
{
    return GRP_DOC;
}

void    SwFieldDokPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_xTypeLB->get_selected_index();
    const sal_uInt16 nTypeSel = ( -1 == nEntryPos )
        ? USHRT_MAX : m_xTypeLB->get_id(nEntryPos).toUInt32();
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
