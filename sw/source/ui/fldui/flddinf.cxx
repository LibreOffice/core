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

#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svtools/treelistentry.hxx>

#include <helpid.h>
#include <swtypes.hxx>
#include <globals.hrc>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <wrtsh.hxx>

#include <fldui.hrc>

#include <flddinf.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

using namespace nsSwDocInfoSubType;
using namespace com::sun::star;

SwFieldDokInfPage::SwFieldDokInfPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    :  SwFieldPage(pParent, "FieldDocInfoPage",
        "modules/swriter/ui/flddocinfopage.ui", rCoreSet)
    , pSelEntry(0)
    , nOldSel(0)
    , nOldFormat(0)
{
    get(m_pTypeTLB, "type");
    get(m_pSelection, "selectframe");
    get(m_pFormat, "formatframe");
    get(m_pSelectionLB, "select");
    get(m_pFormatLB, "format");
    get(m_pFixedCB, "fixed");

    long nHeight = m_pTypeTLB->GetTextHeight() * 20;
    m_pTypeTLB->set_height_request(nHeight);
    m_pSelectionLB->set_height_request(nHeight);
    m_pFormatLB->set_height_request(nHeight);

    long nWidth = m_pTypeTLB->LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MAP_APPFONT)).Width();
    m_pTypeTLB->set_width_request(nWidth);
    m_pFormatLB->set_width_request(nWidth);
    m_pSelectionLB->set_width_request(nWidth);

    m_pTypeTLB->SetSelectionMode(SINGLE_SELECTION);
    m_pTypeTLB->SetStyle(m_pTypeTLB->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    m_pTypeTLB->SetOptimalImageIndent();
    // Don't set font, so that the control's font is adopted!
    // Otherwise at wrong font bug to OV.
    m_pTypeTLB->SetSpaceBetweenEntries(0);

    m_pTypeTLB->SetNodeDefaultImages();
    //enable 'active' language selection
    m_pFormatLB->SetShowLanguageControl(true);

    const SfxUnoAnyItem* pItem = rCoreSet.GetItem<SfxUnoAnyItem>(SID_DOCINFO, false);
    if ( pItem )
        pItem->GetValue() >>= xCustomPropertySet;
}

SwFieldDokInfPage::~SwFieldDokInfPage()
{
    disposeOnce();
}

void SwFieldDokInfPage::dispose()
{
    m_pTypeTLB.clear();
    m_pSelection.clear();
    m_pSelectionLB.clear();
    m_pFormat.clear();
    m_pFormatLB.clear();
    m_pFixedCB.clear();
    SwFieldPage::dispose();
}

void SwFieldDokInfPage::Reset(const SfxItemSet* )
{
    Init(); // general initialisation

    // initialise TypeListBox
    m_pTypeTLB->SetUpdateMode(false);
    m_pTypeTLB->Clear();
    pSelEntry = 0;

    // display SubTypes in TypeLB
    SvTreeListEntry* pEntry = 0;

    SvTreeListEntry* pInfo = 0;

    sal_uInt16 nSubType = USHRT_MAX;
    if (IsFieldEdit())
    {
        const SwField* pCurField = GetCurField();
        nSubType = static_cast<const SwDocInfoField*>(pCurField)->GetSubType() & 0xff;
        if( nSubType == DI_CUSTOM )
        {
            m_sOldCustomFieldName = static_cast<const SwDocInfoField*>(pCurField)->GetName();
        }
        m_pFormatLB->SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell *pSh = GetWrtShell();
        if(pSh)
        {
            const SvNumberformat* pFormat = pSh->GetNumberFormatter()->GetEntry(pCurField->GetFormat());
            if(pFormat)
                m_pFormatLB->SetLanguage(pFormat->GetLanguage());
        }
    }

    sal_Int32 nSelEntryData = LISTBOX_ENTRY_NOTFOUND;
    const OUString sUserData = GetUserData();
    if (sUserData.getToken(0, ';').equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
    {
        nSelEntryData = sUserData.getToken(1, ';').toInt32();
    }

    std::vector<OUString> aLst;
    GetFieldMgr().GetSubTypes(TYP_DOCINFOFLD, aLst);
    for(size_t i = 0; i < aLst.size(); ++i)
    {
        if (!IsFieldEdit() || nSubType == i)
        {
            if (DI_CUSTOM == i)
            {
                if(xCustomPropertySet.is() )
                {
                    uno::Reference< beans::XPropertySetInfo > xSetInfo = xCustomPropertySet->getPropertySetInfo();
                    const uno::Sequence< beans::Property > rProperties = xSetInfo->getProperties();

                    if( rProperties.getLength() )
                    {
                        pInfo = m_pTypeTLB->InsertEntry( OUString(SW_RES( STR_CUSTOM )) );
                        pInfo->SetUserData(reinterpret_cast<void*>(USHRT_MAX));

                        for (sal_Int32 n=0; n < rProperties.getLength(); n++)
                        {
                            const OUString sEntry = rProperties[n].Name;
                            pEntry = m_pTypeTLB->InsertEntry(sEntry, pInfo);
                            if (m_sOldCustomFieldName == sEntry)
                            {
                                pSelEntry = pEntry;
                                m_pTypeTLB->Expand( pInfo );
                            }
                            pEntry->SetUserData(reinterpret_cast<void*>(i));
                        }
                    }
                }
            }
            else
            {
                if (!(IsFieldDlgHtmlMode() && (i == DI_EDIT || i == DI_THEMA || i == DI_PRINT)))
                {
                    pEntry = m_pTypeTLB->InsertEntry(aLst[i]);
                    pEntry->SetUserData(reinterpret_cast<void*>(i));
                }
            }
            if(static_cast<size_t>(nSelEntryData) == i)
                pSelEntry = pEntry;
        }
    }

    // select old Pos
    if (pSelEntry != 0)
    {
        m_pTypeTLB->Select(pSelEntry);
        nSubType = (sal_uInt16)reinterpret_cast<sal_uLong>(pSelEntry->GetUserData());
    }
    else if ( m_pTypeTLB->GetEntry(0) )
    {
        pSelEntry = m_pTypeTLB->GetEntry(0);
        nSubType = (sal_uInt16)reinterpret_cast<sal_uLong>(pSelEntry->GetUserData());
    }

    FillSelectionLB(nSubType);
    if ( pSelEntry )
        TypeHdl(NULL);

    m_pTypeTLB->SetUpdateMode(true);
    m_pTypeTLB->SetSelectHdl(LINK(this, SwFieldDokInfPage, TypeHdl));
    m_pTypeTLB->SetDoubleClickHdl(LINK(this, SwFieldDokInfPage, TreeListBoxInsertHdl));
    m_pSelectionLB->SetSelectHdl(LINK(this, SwFieldDokInfPage, SubTypeHdl));
    m_pSelectionLB->SetDoubleClickHdl(LINK(this, SwFieldDokInfPage, ListBoxInsertHdl));
    m_pFormatLB->SetDoubleClickHdl(LINK(this, SwFieldDokInfPage, ListBoxInsertHdl));

    if (IsFieldEdit())
    {
        nOldSel = m_pSelectionLB->GetSelectEntryPos();
        nOldFormat = GetCurField()->GetFormat();
        m_pFixedCB->SaveValue();
    }
}

IMPL_LINK_NOARG_TYPED(SwFieldDokInfPage, TypeHdl, SvTreeListBox*, void)
{
    // save old ListBoxPos
    SvTreeListEntry* pOldEntry = pSelEntry;

    // current ListBoxPos
    pSelEntry = m_pTypeTLB->FirstSelected();

    if(!pSelEntry)
    {
        pSelEntry = m_pTypeTLB->GetEntry(0);
        m_pTypeTLB->Select(pSelEntry);
    }
    else if (pOldEntry != pSelEntry)
        FillSelectionLB((sal_uInt16)reinterpret_cast<sal_uLong>(pSelEntry->GetUserData()));

    SubTypeHdl(*m_pSelectionLB);
}

IMPL_LINK_NOARG_TYPED(SwFieldDokInfPage, SubTypeHdl, ListBox&, void)
{
    sal_uInt16 nSubType = (sal_uInt16)reinterpret_cast<sal_uLong>(pSelEntry->GetUserData());
    sal_Int32 nPos = m_pSelectionLB->GetSelectEntryPos();
    sal_uInt16 nExtSubType;
    sal_uInt16 nNewType = 0;

    if (nSubType != DI_EDIT)
    {
        if (nPos == LISTBOX_ENTRY_NOTFOUND)
        {
            if (!m_pSelectionLB->GetEntryCount())
            {
                m_pFormatLB->Clear();
                m_pFormat->Enable(false);
                if( nSubType == DI_CUSTOM )
                {
                    //find out which type the custom field has - for a start set to DATE format
                    const OUString sName = m_pTypeTLB->GetEntryText(pSelEntry);
                    try
                    {
                        uno::Any aVal = xCustomPropertySet->getPropertyValue( sName );
                        const uno::Type& rValueType = aVal.getValueType();
                        if( rValueType == ::cppu::UnoType<util::DateTime>::get())
                        {
                            nNewType = css::util::NumberFormat::DATETIME;
                        }
                        else if( rValueType == ::cppu::UnoType<util::Date>::get())
                        {
                            nNewType = css::util::NumberFormat::DATE;
                        }
                        else if( rValueType == ::cppu::UnoType<util::Time>::get())
                        {
                            nNewType = css::util::NumberFormat::TIME;
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

        nExtSubType = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nPos));
    }
    else
        nExtSubType = DI_SUB_TIME;

    sal_uInt16 nOldType = 0;
    bool bEnable = false;
    bool bOneArea = false;

    if (m_pFormatLB->IsEnabled())
        nOldType = m_pFormatLB->GetFormatType();

    switch (nExtSubType)
    {
        case DI_SUB_AUTHOR:
            break;

        case DI_SUB_DATE:
            nNewType = css::util::NumberFormat::DATE;
            bOneArea = true;
            break;

        case DI_SUB_TIME:
            nNewType = css::util::NumberFormat::TIME;
            bOneArea = true;
            break;
    }
    if (!nNewType)
    {
        m_pFormatLB->Clear();
    }
    else
    {
        if (nOldType != nNewType)
        {
            m_pFormatLB->SetFormatType(nNewType);
            m_pFormatLB->SetOneArea(bOneArea);
        }
        bEnable = true;
    }

    sal_uLong nFormat = IsFieldEdit() ? static_cast<SwDocInfoField*>(GetCurField())->GetFormat() : 0;

    sal_uInt16 nOldSubType = IsFieldEdit() ? (static_cast<SwDocInfoField*>(GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFieldEdit())
    {
        nPos = m_pSelectionLB->GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            nSubType = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nPos));

            nOldSubType &= ~DI_SUB_FIXED;
            if (nOldSubType == nSubType)
            {
                if (!nFormat && (nNewType == css::util::NumberFormat::DATE || nNewType == css::util::NumberFormat::TIME))
                {
                    SwWrtShell *pSh = GetWrtShell();
                    if(pSh)
                    {
                        SvNumberFormatter* pFormatter = pSh->GetNumberFormatter();
                        LanguageType eLang = m_pFormatLB->GetCurLanguage();
                        if (nNewType == css::util::NumberFormat::DATE)
                            nFormat = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLang);
                        else if (nNewType == css::util::NumberFormat::TIME)
                            nFormat = pFormatter->GetFormatIndex( NF_TIME_HHMM, eLang);
                    }
                }
                m_pFormatLB->SetDefFormat(nFormat);
            }
        }
        else if( (nSubType == DI_CUSTOM)  && (nNewType != 0) )
        {
            m_pFormatLB->SetDefFormat(nFormat);
        }
    }

    m_pFormat->Enable(bEnable);

    if (bEnable && m_pFormatLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
    {
        m_pFormatLB->SelectEntryPos(0);
    }
}

sal_Int32 SwFieldDokInfPage::FillSelectionLB(sal_uInt16 nSubType)
{
    // fill Format-Listbox
    sal_uInt16 nTypeId = TYP_DOCINFOFLD;

    EnableInsert(nSubType != USHRT_MAX);

    if (nSubType == USHRT_MAX)  // Info-Text
        nSubType = DI_SUBTYPE_BEGIN;

    m_pSelectionLB->Clear();

    sal_uInt16 nSize = 0;
    sal_Int32 nSelPos = LISTBOX_ENTRY_NOTFOUND;
    sal_uInt16 nExtSubType = IsFieldEdit() ? (static_cast<SwDocInfoField*>(GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFieldEdit())
    {
        m_pFixedCB->Check((nExtSubType & DI_SUB_FIXED) != 0);
        nExtSubType = ((nExtSubType & ~DI_SUB_FIXED) >> 8) - 1;
    }

    if (nSubType < DI_CREATE || nSubType == DI_DOCNO || nSubType == DI_EDIT|| nSubType == DI_CUSTOM )
    {
        // Format Box is empty for Title and Time
    }
    else
    {
        nSize = GetFieldMgr().GetFormatCount(nTypeId, false, IsFieldDlgHtmlMode());
        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            sal_Int32 nPos = m_pSelectionLB->InsertEntry(GetFieldMgr().GetFormatStr(nTypeId, i));
            m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(GetFieldMgr().GetFormatId(nTypeId, i)));
            if (IsFieldEdit() && i == nExtSubType)
                nSelPos = nPos;
        }
    }

    bool bEnable = nSize != 0;

    if (nSize)
    {
        if (!m_pSelectionLB->GetSelectEntryCount())
            m_pSelectionLB->SelectEntryPos(nSelPos == USHRT_MAX ? 0 : nSelPos);

        bEnable = true;
    }

    m_pSelection->Enable(bEnable);

    return nSize;
}

bool SwFieldDokInfPage::FillItemSet(SfxItemSet* )
{
    if (!pSelEntry || (sal_uInt16)reinterpret_cast<sal_uLong>(pSelEntry->GetUserData()) == USHRT_MAX)
        return false;

    sal_uInt16 nSubType = (sal_uInt16)reinterpret_cast<sal_uLong>(pSelEntry->GetUserData());

    sal_uLong nFormat = 0;

    sal_Int32 nPos = m_pSelectionLB->GetSelectEntryPos();

    OUString aName;
    if (DI_CUSTOM == nSubType)
        aName = m_pTypeTLB->GetEntryText(pSelEntry);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        nSubType |= (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nPos));

    if (m_pFixedCB->IsChecked())
        nSubType |= DI_SUB_FIXED;

    nPos = m_pFormatLB->GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
        nFormat = m_pFormatLB->GetFormat();

    if (!IsFieldEdit() || nOldSel != m_pSelectionLB->GetSelectEntryPos() ||
        nOldFormat != nFormat || m_pFixedCB->IsValueChangedFromSaved()
        || (DI_CUSTOM == nSubType && aName != m_sOldCustomFieldName ))
    {
        InsertField(TYP_DOCINFOFLD, nSubType, aName, aEmptyOUStr, nFormat,
                ' ', m_pFormatLB->IsAutomaticLanguage());
    }

    return false;
}

VclPtr<SfxTabPage> SwFieldDokInfPage::Create( vcl::Window* pParent,
                                            const SfxItemSet* rAttrSet )
{
    return VclPtr<SwFieldDokInfPage>::Create( pParent, *rAttrSet );
}

sal_uInt16 SwFieldDokInfPage::GetGroup()
{
    return GRP_REG;
}

void    SwFieldDokInfPage::FillUserData()
{
    SvTreeListEntry* pEntry = m_pTypeTLB->FirstSelected();
    sal_uInt16 nTypeSel = pEntry ? sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(pEntry->GetUserData())) : USHRT_MAX;
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
