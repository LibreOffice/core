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

#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>

#include <swtypes.hxx>
#include <flddinf.hrc>
#include <strings.hrc>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <wrtsh.hxx>

#include "flddinf.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

using namespace nsSwDocInfoSubType;
using namespace com::sun::star;

void FillFieldSelect(weld::TreeView& rListBox)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(FLD_SELECT); ++i)
        rListBox.append_text(SwResId(FLD_SELECT[i]));
}

SwFieldDokInfPage::SwFieldDokInfPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *const pCoreSet)
    :  SwFieldPage(pPage, pController, "modules/swriter/ui/flddocinfopage.ui", "FieldDocInfoPage", pCoreSet)
    , nOldSel(0)
    , nOldFormat(0)
    , m_xTypeTLB(m_xBuilder->weld_tree_view("type"))
    , m_xSelection(m_xBuilder->weld_widget("selectframe"))
    , m_xSelectionLB(m_xBuilder->weld_tree_view("select"))
    , m_xFormat(m_xBuilder->weld_widget("formatframe"))
    , m_xFormatLB(new SwNumFormatTreeView(m_xBuilder->weld_tree_view("format")))
    , m_xFixedCB(m_xBuilder->weld_check_button("fixed"))
{
    m_xTypeTLB->make_sorted();
    FillFieldSelect(*m_xSelectionLB);

    auto nWidth = m_xTypeTLB->get_approximate_digit_width() * FIELD_COLUMN_WIDTH;
    auto nHeight = m_xTypeTLB->get_height_rows(10);
    m_xTypeTLB->set_size_request(nWidth, nHeight);
    m_xFormatLB->get_widget().set_size_request(nWidth * 2, nHeight);
    m_xSelectionLB->set_size_request(nWidth, nHeight);

    //enable 'active' language selection
    m_xFormatLB->SetShowLanguageControl(true);

    const SfxUnoAnyItem* pItem = pCoreSet
        ? pCoreSet->GetItem<SfxUnoAnyItem>(SID_DOCINFO, false)
        : nullptr;
    if ( pItem )
        pItem->GetValue() >>= xCustomPropertySet;

    // uitests
    m_xTypeTLB->set_buildable_name(m_xTypeTLB->get_buildable_name() + "-docinf");
}

SwFieldDokInfPage::~SwFieldDokInfPage()
{
}

void SwFieldDokInfPage::Reset(const SfxItemSet* )
{
    Init(); // general initialisation

    // initialise TypeListBox
    m_xTypeTLB->freeze();
    m_xTypeTLB->clear();
    m_xSelEntry.reset();

    // display SubTypes in TypeLB
    sal_uInt16 nSubType = USHRT_MAX;
    if (IsFieldEdit())
    {
        const SwField* pCurField = GetCurField();
        nSubType = static_cast<const SwDocInfoField*>(pCurField)->GetSubType() & 0xff;
        if( nSubType == DI_CUSTOM )
        {
            m_sOldCustomFieldName = static_cast<const SwDocInfoField*>(pCurField)->GetName();
        }
        m_xFormatLB->SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell *pSh = GetWrtShell();
        if(pSh)
        {
            const SvNumberformat* pFormat = pSh->GetNumberFormatter()->GetEntry(pCurField->GetFormat());
            if(pFormat)
                m_xFormatLB->SetLanguage(pFormat->GetLanguage());
        }
    }

    sal_Int32 nSelEntryData = -1;
    const OUString sUserData = GetUserData();
    sal_Int32 nIdx{ 0 };
    if (sUserData.getToken(0, ';', nIdx).equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
    {
        nSelEntryData = sUserData.getToken(0, ';', nIdx).toInt32();
    }

    std::vector<OUString> aLst;
    GetFieldMgr().GetSubTypes(SwFieldTypesEnum::DocumentInfo, aLst);
    std::unique_ptr<weld::TreeIter> xEntry(m_xTypeTLB->make_iterator());
    std::unique_ptr<weld::TreeIter> xExpandEntry;
    for(size_t i = 0; i < aLst.size(); ++i)
    {
        if (!IsFieldEdit() || nSubType == i)
        {
            const OUString sId(OUString::number(i));
            if (DI_CUSTOM == i)
            {
                if(xCustomPropertySet.is() )
                {
                    uno::Reference< beans::XPropertySetInfo > xSetInfo = xCustomPropertySet->getPropertySetInfo();
                    const uno::Sequence< beans::Property > rProperties = xSetInfo->getProperties();

                    if( rProperties.hasElements() )
                    {
                        std::unique_ptr<weld::TreeIter> xInfo(m_xTypeTLB->make_iterator());

                        OUString sText(SwResId(STR_CUSTOM_FIELD));
                        OUString sEntryId(OUString::number(USHRT_MAX));
                        m_xTypeTLB->insert(nullptr, -1, &sText, &sEntryId, nullptr,
                                           nullptr, false, xInfo.get());
                        for (const auto& rProperty : rProperties)
                        {
                            const OUString sEntry = rProperty.Name;

                            m_xTypeTLB->insert(xInfo.get(), -1, &sEntry, &sId,
                                               nullptr, nullptr, false, xEntry.get());
                            if (m_sOldCustomFieldName == sEntry)
                            {
                                m_xSelEntry = m_xTypeTLB->make_iterator(xEntry.get());
                                xExpandEntry = m_xTypeTLB->make_iterator(xInfo.get());
                            }
                        }
                    }
                }
            }
            else
            {
                if (!(IsFieldDlgHtmlMode() && (i == DI_EDIT || i == DI_SUBJECT || i == DI_PRINT)))
                {
                    m_xTypeTLB->insert(nullptr, -1, &aLst[i], &sId,
                                       nullptr, nullptr, false, xEntry.get());
                }
            }
            if (static_cast<size_t>(nSelEntryData) == i)
                m_xSelEntry = std::move(xEntry);
        }
    }

    m_xTypeTLB->thaw();

    if (xExpandEntry)
        m_xTypeTLB->expand_row(*xExpandEntry);

    // select old Pos
    if (m_xSelEntry)
    {
        m_xTypeTLB->select(*m_xSelEntry);
        nSubType = m_xTypeTLB->get_id(*m_xSelEntry).toUInt32();
    }
    else
    {
        m_xSelEntry = m_xTypeTLB->make_iterator();
        if (m_xTypeTLB->get_iter_first(*m_xSelEntry))
            nSubType = m_xTypeTLB->get_id(*m_xSelEntry).toUInt32();
        else
            m_xSelEntry.reset();
    }

    FillSelectionLB(nSubType);
    if (m_xSelEntry)
        TypeHdl(*m_xTypeTLB);

    m_xTypeTLB->connect_changed(LINK(this, SwFieldDokInfPage, TypeHdl));
    m_xTypeTLB->connect_row_activated(LINK(this, SwFieldDokInfPage, TreeViewInsertHdl));
    m_xSelectionLB->connect_changed(LINK(this, SwFieldDokInfPage, SubTypeHdl));
    m_xSelectionLB->connect_row_activated(LINK(this, SwFieldDokInfPage, TreeViewInsertHdl));
    m_xFormatLB->connect_row_activated(LINK(this, SwFieldDokInfPage, TreeViewInsertHdl));

    if (IsFieldEdit())
    {
        nOldSel = m_xSelectionLB->get_selected_index();
        nOldFormat = GetCurField()->GetFormat();
        m_xFixedCB->save_state();
    }
}

IMPL_LINK_NOARG(SwFieldDokInfPage, TypeHdl, weld::TreeView&, void)
{
    // current ListBoxPos
    if (!m_xTypeTLB->get_selected(m_xSelEntry.get()) &&
        m_xTypeTLB->get_iter_first(*m_xSelEntry))
    {
        m_xTypeTLB->select(*m_xSelEntry);
    }
    FillSelectionLB(m_xTypeTLB->get_id(*m_xSelEntry).toUInt32());
    SubTypeHdl(*m_xSelectionLB);
}

IMPL_LINK_NOARG(SwFieldDokInfPage, SubTypeHdl, weld::TreeView&, void)
{
    sal_uInt16 nSubType = m_xTypeTLB->get_id(*m_xSelEntry).toUInt32();
    sal_Int32 nPos = m_xSelectionLB->get_selected_index();
    sal_uInt16 nExtSubType;
    SvNumFormatType nNewType = SvNumFormatType::ALL;

    if (nSubType != DI_EDIT)
    {
        if (nPos == -1)
        {
            if (!m_xSelectionLB->n_children())
            {
                m_xFormatLB->clear();
                m_xFormat->set_sensitive(false);
                if( nSubType == DI_CUSTOM )
                {
                    //find out which type the custom field has - for a start set to DATE format
                    const OUString sName = m_xTypeTLB->get_text(*m_xSelEntry);
                    try
                    {
                        uno::Any aVal = xCustomPropertySet->getPropertyValue( sName );
                        const uno::Type& rValueType = aVal.getValueType();
                        if( rValueType == ::cppu::UnoType<util::DateTime>::get())
                        {
                            nNewType = SvNumFormatType::DATETIME;
                        }
                        else if( rValueType == ::cppu::UnoType<util::Date>::get())
                        {
                            nNewType = SvNumFormatType::DATE;
                        }
                        else if( rValueType == ::cppu::UnoType<util::Time>::get())
                        {
                            nNewType = SvNumFormatType::TIME;
                        }
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }
                else
                    return;
            }
            nPos = 0;
        }

        nExtSubType = m_xSelectionLB->get_id(nPos).toUInt32();
    }
    else
        nExtSubType = DI_SUB_TIME;

    SvNumFormatType nOldType = SvNumFormatType::ALL;
    bool bEnable = false;
    bool bOneArea = false;

    if (m_xFormatLB->get_active())
        nOldType = m_xFormatLB->GetFormatType();

    switch (nExtSubType)
    {
        case DI_SUB_AUTHOR:
            break;

        case DI_SUB_DATE:
            nNewType = SvNumFormatType::DATE;
            bOneArea = true;
            break;

        case DI_SUB_TIME:
            nNewType = SvNumFormatType::TIME;
            bOneArea = true;
            break;
    }
    if (nNewType == SvNumFormatType::ALL)
    {
        m_xFormatLB->clear();
    }
    else
    {
        if (nOldType != nNewType)
        {
            m_xFormatLB->SetFormatType(nNewType);
            m_xFormatLB->SetOneArea(bOneArea);
        }
        bEnable = true;
    }

    sal_uInt32 nFormat = IsFieldEdit() ? static_cast<SwDocInfoField*>(GetCurField())->GetFormat() : 0;

    sal_uInt16 nOldSubType = IsFieldEdit() ? (static_cast<SwDocInfoField*>(GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFieldEdit())
    {
        nPos = m_xSelectionLB->get_selected_index();
        if (nPos != -1)
        {
            nSubType = m_xSelectionLB->get_id(nPos).toUInt32();

            nOldSubType &= ~DI_SUB_FIXED;
            if (nOldSubType == nSubType)
            {
                if (!nFormat && (nNewType == SvNumFormatType::DATE || nNewType == SvNumFormatType::TIME))
                {
                    SwWrtShell *pSh = GetWrtShell();
                    if(pSh)
                    {
                        SvNumberFormatter* pFormatter = pSh->GetNumberFormatter();
                        LanguageType eLang = m_xFormatLB->GetCurLanguage();
                        if (nNewType == SvNumFormatType::DATE)
                            nFormat = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLang);
                        else if (nNewType == SvNumFormatType::TIME)
                            nFormat = pFormatter->GetFormatIndex( NF_TIME_HHMM, eLang);
                    }
                }
                m_xFormatLB->SetDefFormat(nFormat);
            }
        }
        else if( (nSubType == DI_CUSTOM)  && (nNewType != SvNumFormatType::ALL) )
        {
            m_xFormatLB->SetDefFormat(nFormat);
        }
    }

    m_xFormat->set_sensitive(bEnable);

    if (bEnable && m_xFormatLB->get_selected_index() == -1)
        m_xFormatLB->select(0);
}

sal_Int32 SwFieldDokInfPage::FillSelectionLB(sal_uInt16 nSubType)
{
    // fill Format-Listbox
    SwFieldTypesEnum nTypeId = SwFieldTypesEnum::DocumentInfo;

    EnableInsert(nSubType != USHRT_MAX);

    if (nSubType == USHRT_MAX)  // Info-Text
        nSubType = DI_SUBTYPE_BEGIN;

    m_xSelectionLB->clear();

    sal_uInt16 nSize = 0;
    sal_Int32 nSelPos = -1;
    sal_uInt16 nExtSubType = IsFieldEdit() ? (static_cast<SwDocInfoField*>(GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFieldEdit())
    {
        m_xFixedCB->set_active((nExtSubType & DI_SUB_FIXED) != 0);
        nExtSubType = ((nExtSubType & ~DI_SUB_FIXED) >> 8) - 1;
    }

    if (nSubType < DI_CREATE || nSubType == DI_DOCNO || nSubType == DI_EDIT|| nSubType == DI_CUSTOM )
    {
        // Format Box is empty for Title and Time
    }
    else
    {
        nSize = GetFieldMgr().GetFormatCount(nTypeId, IsFieldDlgHtmlMode());
        for (sal_uInt16 i = 0; i < nSize; ++i)
        {
            OUString sId(OUString::number(GetFieldMgr().GetFormatId(nTypeId, i)));
            m_xSelectionLB->append(sId, GetFieldMgr().GetFormatStr(nTypeId, i));
            if (IsFieldEdit() && i == nExtSubType)
                nSelPos = i;
        }
    }

    bool bEnable = nSize != 0;

    if (nSize)
    {
        if (m_xSelectionLB->get_selected_index() == -1)
            m_xSelectionLB->select(nSelPos == USHRT_MAX ? 0 : nSelPos);
        bEnable = true;
    }

    m_xSelection->set_sensitive(bEnable);

    return nSize;
}

bool SwFieldDokInfPage::FillItemSet(SfxItemSet* )
{
    if (!m_xSelEntry)
        return false;

    sal_uInt16 nSubType = m_xTypeTLB->get_id(*m_xSelEntry).toUInt32();
    if (nSubType == USHRT_MAX)
        return false;

    sal_uInt32 nFormat = 0;

    sal_Int32 nPos = m_xSelectionLB->get_selected_index();

    OUString aName;
    if (DI_CUSTOM == nSubType)
        aName = m_xTypeTLB->get_text(*m_xSelEntry);

    if (nPos != -1)
        nSubType |= m_xSelectionLB->get_id(nPos).toUInt32();

    if (m_xFixedCB->get_active())
        nSubType |= DI_SUB_FIXED;

    nPos = m_xFormatLB->get_selected_index();
    if(nPos != -1)
        nFormat = m_xFormatLB->GetFormat();

    if (!IsFieldEdit() || nOldSel != m_xSelectionLB->get_selected_index() ||
        nOldFormat != nFormat || m_xFixedCB->get_state_changed_from_saved()
        || (DI_CUSTOM == nSubType && aName != m_sOldCustomFieldName ))
    {
        InsertField(SwFieldTypesEnum::DocumentInfo, nSubType, aName, OUString(), nFormat,
                ' ', m_xFormatLB->IsAutomaticLanguage());
    }

    return false;
}

std::unique_ptr<SfxTabPage> SwFieldDokInfPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet *const pAttrSet)
{
    return std::make_unique<SwFieldDokInfPage>(pPage, pController, pAttrSet);
}

sal_uInt16 SwFieldDokInfPage::GetGroup()
{
    return GRP_REG;
}

void SwFieldDokInfPage::FillUserData()
{
    int nEntry = m_xTypeTLB->get_selected_index();
    sal_uInt16 nTypeSel = nEntry != -1 ? m_xTypeTLB->get_id(nEntry).toUInt32() : USHRT_MAX;
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
