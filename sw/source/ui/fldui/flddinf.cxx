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
#include "svtools/treelistentry.hxx"

#include <helpid.h>
#include <swtypes.hxx>
#include <globals.hrc>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <wrtsh.hxx>

#include <fldui.hrc>

#include <fldtdlg.hrc>
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

SwFldDokInfPage::SwFldDokInfPage(Window* pParent, const SfxItemSet& rCoreSet)
    :  SwFldPage(pParent, "FldDocInfoPage",
        "modules/swriter/ui/flddocinfopage.ui", rCoreSet)
    , pSelEntry(0)
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
    m_pTypeTLB->SetIndent(10);
    // Don't set font, so that the control's font is adobted!
    // Otherwise at wrong font bug to OV.
    m_pTypeTLB->SetSpaceBetweenEntries(0);

    m_pTypeTLB->SetNodeDefaultImages();
    //enable 'active' language selection
    m_pFormatLB->SetShowLanguageControl(sal_True);

    SFX_ITEMSET_ARG( &rCoreSet, pItem, SfxUnoAnyItem, SID_DOCINFO, sal_False );
    if ( pItem )
        pItem->GetValue() >>= xCustomPropertySet;
}

SwFldDokInfPage::~SwFldDokInfPage()
{
}

void SwFldDokInfPage::Reset(const SfxItemSet& )
{
    Init(); // general initialisation

    // initialise TypeListBox
    m_pTypeTLB->SetUpdateMode(sal_False);
    m_pTypeTLB->Clear();
    pSelEntry = 0;

    // display SubTypes in TypeLB
    sal_uInt16 nTypeId = TYP_DOCINFOFLD;
    SvTreeListEntry* pEntry = 0;

    SvTreeListEntry* pInfo = 0;

    sal_uInt16 nSubType = USHRT_MAX;
    if (IsFldEdit())
    {
        const SwField* pCurField = GetCurField();
        nSubType = ((SwDocInfoField*)pCurField)->GetSubType() & 0xff;
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

    sal_uInt16 nSelEntryData = USHRT_MAX;
    String sUserData = GetUserData();
    if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
    {
        String sVal = sUserData.GetToken(1, ';');
        nSelEntryData = static_cast< sal_uInt16 >(sVal.ToInt32());
    }

    std::vector<OUString> aLst;
    GetFldMgr().GetSubTypes(nTypeId, aLst);
    for(size_t i = 0; i < aLst.size(); ++i)
    {
        if (!IsFldEdit() || nSubType == i)
        {
            if (DI_CUSTOM == i)
            {
                if(xCustomPropertySet.is() )
                {
                    uno::Reference< beans::XPropertySetInfo > xSetInfo = xCustomPropertySet->getPropertySetInfo();
                    const uno::Sequence< beans::Property > rProperties = xSetInfo->getProperties();

                    if( rProperties.getLength() )
                    {
                        pInfo = m_pTypeTLB->InsertEntry( String(SW_RES( STR_CUSTOM )) );
                        pInfo->SetUserData(reinterpret_cast<void*>(USHRT_MAX));

                        for (sal_Int32 n=0; n < rProperties.getLength(); n++)
                        {
                            OUString sEntry = rProperties[n].Name;
                            pEntry = m_pTypeTLB->InsertEntry(sEntry, pInfo);
                            if(m_sOldCustomFieldName.equals( sEntry ))
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
                if (!(IsFldDlgHtmlMode() && (i == DI_EDIT || i == DI_THEMA || i == DI_PRINT)))
                {
                    pEntry = m_pTypeTLB->InsertEntry(aLst[i]);
                    pEntry->SetUserData(reinterpret_cast<void*>(i));
                }
            }
            if(nSelEntryData == i)
                pSelEntry = pEntry;
        }
    }

    // select old Pos
    if (pSelEntry != 0)
    {
        m_pTypeTLB->Select(pSelEntry);
        nSubType = (sal_uInt16)(sal_uLong)pSelEntry->GetUserData();
    }
    else if ( m_pTypeTLB->GetEntry(0) )
    {
        pSelEntry = m_pTypeTLB->GetEntry(0);
        nSubType = (sal_uInt16)(sal_uLong)pSelEntry->GetUserData();
    }

    FillSelectionLB(nSubType);
    if ( pSelEntry )
        TypeHdl();

    m_pTypeTLB->SetUpdateMode(sal_True);
    m_pTypeTLB->SetSelectHdl(LINK(this, SwFldDokInfPage, TypeHdl));
    m_pTypeTLB->SetDoubleClickHdl(LINK(this, SwFldDokInfPage, InsertHdl));
    m_pSelectionLB->SetSelectHdl(LINK(this, SwFldDokInfPage, SubTypeHdl));
    m_pSelectionLB->SetDoubleClickHdl(LINK(this, SwFldDokInfPage, InsertHdl));
    m_pFormatLB->SetDoubleClickHdl(LINK(this, SwFldDokInfPage, InsertHdl));

    if (IsFldEdit())
    {
        nOldSel = m_pSelectionLB->GetSelectEntryPos();
        nOldFormat = GetCurField()->GetFormat();
        m_pFixedCB->SaveValue();
    }
}

IMPL_LINK_NOARG(SwFldDokInfPage, TypeHdl)
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
        FillSelectionLB((sal_uInt16)(sal_uLong)pSelEntry->GetUserData());

    SubTypeHdl();

    return 0;
}

IMPL_LINK_NOARG(SwFldDokInfPage, SubTypeHdl)
{
    sal_uInt16 nSubType = (sal_uInt16)(sal_uLong)pSelEntry->GetUserData();
    sal_uInt16 nPos = m_pSelectionLB->GetSelectEntryPos();
    sal_uInt16 nExtSubType;
    sal_uInt16 nNewType = 0;

    if (nSubType != DI_EDIT)
    {
        if (nPos == LISTBOX_ENTRY_NOTFOUND)
        {
            if (!m_pSelectionLB->GetEntryCount())
            {
                m_pFormatLB->Clear();
                m_pFormat->Enable(sal_False);
                if( nSubType == DI_CUSTOM )
                {
                    //find out which type the custom field has - for a start set to DATE format
                    OUString sName = m_pTypeTLB->GetEntryText(pSelEntry);
                    try
                    {
                        uno::Any aVal = xCustomPropertySet->getPropertyValue( sName );
                        const uno::Type& rValueType = aVal.getValueType();
                        if( rValueType == ::getCppuType( (util::DateTime*)0 ))
                        {
                            nNewType = NUMBERFORMAT_DATETIME;
                        }
                        else if( rValueType == ::getCppuType( (util::Date*)0 ))
                        {
                            nNewType = NUMBERFORMAT_DATE;
                        }
                        else if( rValueType == ::getCppuType( (util::Time*)0 ))
                        {
                            nNewType = NUMBERFORMAT_TIME;
                        }
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }
                else
                    return 0;
            }
            nPos = 0;
        }

        nExtSubType = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nPos);
    }
    else
        nExtSubType = DI_SUB_TIME;

    sal_uInt16 nOldType = 0;
    sal_Bool bEnable = sal_False;
    sal_Bool bOneArea = sal_False;

    if (m_pFormatLB->IsEnabled())
        nOldType = m_pFormatLB->GetFormatType();

    switch (nExtSubType)
    {
        case DI_SUB_AUTHOR:
            break;

        case DI_SUB_DATE:
            nNewType = NUMBERFORMAT_DATE;
            bOneArea = sal_True;
            break;

        case DI_SUB_TIME:
            nNewType = NUMBERFORMAT_TIME;
            bOneArea = sal_True;
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
        bEnable = sal_True;
    }

    sal_uLong nFormat = IsFldEdit() ? ((SwDocInfoField*)GetCurField())->GetFormat() : 0;

    sal_uInt16 nOldSubType = IsFldEdit() ? (((SwDocInfoField*)GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFldEdit())
    {
        nPos = m_pSelectionLB->GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            nSubType = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nPos);

            nOldSubType &= ~DI_SUB_FIXED;
            if (nOldSubType == nSubType)
            {
                if (!nFormat && (nNewType == NUMBERFORMAT_DATE || nNewType == NUMBERFORMAT_TIME))
                {
                    SwWrtShell *pSh = GetWrtShell();
                    if(pSh)
                    {
                        SvNumberFormatter* pFormatter = pSh->GetNumberFormatter();
                        LanguageType eLang = m_pFormatLB->GetCurLanguage();
                        if (nNewType == NUMBERFORMAT_DATE)
                            nFormat = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLang);
                        else if (nNewType == NUMBERFORMAT_TIME)
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

    return 0;
}

sal_uInt16 SwFldDokInfPage::FillSelectionLB(sal_uInt16 nSubType)
{
    // fill Format-Listbox
    sal_uInt16 nTypeId = TYP_DOCINFOFLD;

    EnableInsert(nSubType != USHRT_MAX);

    if (nSubType == USHRT_MAX)  // Info-Text
        nSubType = DI_SUBTYPE_BEGIN;

    m_pSelectionLB->Clear();

    sal_uInt16 nSize = 0;
    sal_uInt16 nSelPos = USHRT_MAX;
    sal_uInt16 nExtSubType = IsFldEdit() ? (((SwDocInfoField*)GetCurField())->GetSubType() & 0xff00) : 0;

    if (IsFldEdit())
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
        nSize = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());
        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            sal_uInt16 nPos = m_pSelectionLB->InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
            m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId(nTypeId, i)));
            if (IsFldEdit() && i == nExtSubType)
                nSelPos = nPos;
        }
    }

    sal_Bool bEnable = nSize != 0;

    if (nSize)
    {
        if (!m_pSelectionLB->GetSelectEntryCount())
            m_pSelectionLB->SelectEntryPos(nSelPos == USHRT_MAX ? 0 : nSelPos);

        bEnable = sal_True;
    }

    m_pSelection->Enable(bEnable);

    return nSize;
}

sal_Bool SwFldDokInfPage::FillItemSet(SfxItemSet& )
{
    if (!pSelEntry || (sal_uInt16)(sal_uLong)pSelEntry->GetUserData() == USHRT_MAX)
        return sal_False;

    sal_uInt16 nTypeId = TYP_DOCINFOFLD;
    sal_uInt16 nSubType = (sal_uInt16)(sal_uLong)pSelEntry->GetUserData();

    sal_uLong nFormat = 0;

    sal_uInt16 nPos = m_pSelectionLB->GetSelectEntryPos();

    OUString aName;
    if (DI_CUSTOM == nSubType)
        aName = m_pTypeTLB->GetEntryText(pSelEntry);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        nSubType |= (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nPos);

    if (m_pFixedCB->IsChecked())
        nSubType |= DI_SUB_FIXED;

    nPos = m_pFormatLB->GetSelectEntryPos();
    if(nPos != LISTBOX_ENTRY_NOTFOUND)
        nFormat = m_pFormatLB->GetFormat();

    if (!IsFldEdit() || nOldSel != m_pSelectionLB->GetSelectEntryPos() ||
        nOldFormat != nFormat || m_pFixedCB->GetState() != m_pFixedCB->GetSavedValue()
        || (DI_CUSTOM == nSubType && !aName.equals( m_sOldCustomFieldName )))
    {
        InsertFld(nTypeId, nSubType, aName, aEmptyStr, nFormat,
                ' ', m_pFormatLB->IsAutomaticLanguage());
    }

    return sal_False;
}

SfxTabPage* SwFldDokInfPage::Create(    Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldDokInfPage( pParent, rAttrSet ) );
}

sal_uInt16 SwFldDokInfPage::GetGroup()
{
    return GRP_REG;
}

void    SwFldDokInfPage::FillUserData()
{
    String sData(OUString(USER_DATA_VERSION));
    sData += ';';
    SvTreeListEntry* pEntry = m_pTypeTLB->FirstSelected();
    sal_uInt16 nTypeSel = pEntry ? sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(pEntry->GetUserData())) : USHRT_MAX;
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
