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

#include <i18nlangtag/lang.h>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svl/eitem.hxx>
#include <svx/svxids.hrc>
#include <svx/numinf.hxx>
#include <svx/flagsdef.hxx>
#include <svl/itemset.hxx>
#include <osl/diagnose.h>
#include <docsh.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <numfmtlb.hxx>
#include <strings.hrc>
#include <swabstdlg.hxx>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

/**
 *  Description:
 *     nFormatType: Display the formats of this Type
 *     nDefaultFormat: Select this format and possibly insert it
 */

namespace
{

bool lcl_isSystemFormat(sal_uInt32 nDefaultFormat, SvNumberFormatter* pFormatter, LanguageType eCurLanguage)
{
    const sal_uInt32 nSysNumFormat = pFormatter->GetFormatIndex(NF_NUMBER_SYSTEM, eCurLanguage);
    if (nDefaultFormat == nSysNumFormat)
        return true;
    const sal_uInt32 nSysShortDateFormat = pFormatter->GetFormatIndex(NF_DATE_SYSTEM_SHORT, eCurLanguage);
    if (nDefaultFormat == nSysShortDateFormat)
        return true;
    const sal_uInt32 nSysLongDateFormat = pFormatter->GetFormatIndex(NF_DATE_SYSTEM_LONG, eCurLanguage);
    if (nDefaultFormat == nSysLongDateFormat)
        return true;

    if ( eCurLanguage != GetAppLanguage() )
        return false;

    if (nDefaultFormat == pFormatter->GetFormatForLanguageIfBuiltIn(nSysNumFormat, LANGUAGE_SYSTEM))
        return true;
    if (nDefaultFormat == pFormatter->GetFormatForLanguageIfBuiltIn(nSysShortDateFormat, LANGUAGE_SYSTEM))
        return true;
    if (nDefaultFormat == pFormatter->GetFormatForLanguageIfBuiltIn(nSysLongDateFormat, LANGUAGE_SYSTEM))
        return true;

    return false;
}

}

double SwNumFormatBase::GetDefValue(const SvNumFormatType nFormatType)
{
    SvxNumValCategory nDefValue = SvxNumValCategory::Standard;

    switch (nFormatType)
    {
        case SvNumFormatType::DATE:
        case SvNumFormatType::DATE|SvNumFormatType::TIME:
            nDefValue = SvxNumValCategory::Date;
            break;

        case SvNumFormatType::TIME:
            nDefValue = SvxNumValCategory::Time;
            break;

        case SvNumFormatType::TEXT:
        case SvNumFormatType::UNDEFINED:
            nDefValue = SvxNumValCategory::Standard;
            break;

        case SvNumFormatType::CURRENCY:
            nDefValue = SvxNumValCategory::Currency;
            break;

        case SvNumFormatType::PERCENT:
            nDefValue = SvxNumValCategory::Percent;
            break;

        case SvNumFormatType::LOGICAL:
            nDefValue = SvxNumValCategory::Boolean;
            break;

        default:
            nDefValue = SvxNumValCategory::Standard;
            break;
    }

    return fSvxNumValConst[nDefValue];
}

SwNumFormatBase::SwNumFormatBase()
    : m_nStdEntry(0)
    , m_nDefFormat(0)
    , m_nCurrFormatType(SvNumFormatType::ALL)
    , m_bOneArea(false)
    , mbCurrFormatTypeNeedsInit(true)
    , m_bShowLanguageControl(false)
    , m_bUseAutomaticLanguage(true)
{
}

NumFormatListBox::NumFormatListBox(std::unique_ptr<weld::ComboBox> xControl)
    : mxControl(std::move(xControl))
{
    Init();
}

SwNumFormatTreeView::SwNumFormatTreeView(std::unique_ptr<weld::TreeView> xControl)
    : mxControl(std::move(xControl))
{
    Init();
}

void SwNumFormatBase::Init()
{
    SwView *pView = GetActiveView();

    if (pView)
        m_eCurLanguage = pView->GetWrtShell().GetCurLang();
    else
        m_eCurLanguage = SvtSysLocale().GetLanguageTag().getLanguageType();

    SetFormatType(SvNumFormatType::NUMBER);
    SetDefFormat(m_nDefFormat);
}

void NumFormatListBox::Init()
{
    SwNumFormatBase::Init();

    mxControl->connect_changed(LINK(this, NumFormatListBox, SelectHdl));
}

void SwNumFormatTreeView::Init()
{
    SwNumFormatBase::Init();

    mxControl->connect_changed(LINK(this, SwNumFormatTreeView, SelectHdl));
}

void SwNumFormatBase::SetFormatType(const SvNumFormatType nFormatType)
{
    if (!mbCurrFormatTypeNeedsInit &&
        (m_nCurrFormatType & nFormatType))   // there are mixed formats, like for example DateTime
        return;

    SwView *pView = GetActiveView();
    OSL_ENSURE(pView, "no view found");
    if(!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

    clear();    // Remove all entries from the Listbox

    NfIndexTableOffset eOffsetStart = NF_NUMBER_START;
    NfIndexTableOffset eOffsetEnd = NF_NUMBER_START;

    switch( nFormatType )
    {
    case SvNumFormatType::NUMBER:
        eOffsetStart=NF_NUMBER_START;
        eOffsetEnd=NF_NUMBER_END;
        break;

    case SvNumFormatType::PERCENT:
        eOffsetStart=NF_PERCENT_START;
        eOffsetEnd=NF_PERCENT_END;
        break;

    case SvNumFormatType::CURRENCY:
        eOffsetStart=NF_CURRENCY_START;
        eOffsetEnd=NF_CURRENCY_END;
        break;

    case SvNumFormatType::DATETIME:
        eOffsetStart=NF_DATE_START;
        eOffsetEnd=NF_TIME_END;
        break;

    case SvNumFormatType::DATE:
        eOffsetStart=NF_DATE_START;
        eOffsetEnd=NF_DATE_END;
        break;

    case SvNumFormatType::TIME:
        eOffsetStart=NF_TIME_START;
        eOffsetEnd=NF_TIME_END;
        break;

    case SvNumFormatType::SCIENTIFIC:
        eOffsetStart=NF_SCIENTIFIC_START;
        eOffsetEnd=NF_SCIENTIFIC_END;
        break;

    case SvNumFormatType::FRACTION:
        eOffsetStart=NF_FRACTION_START;
        eOffsetEnd=NF_FRACTION_END;
        break;

    case SvNumFormatType::LOGICAL:
        eOffsetStart=NF_BOOLEAN;
        eOffsetEnd=NF_BOOLEAN;
        break;

    case SvNumFormatType::TEXT:
        eOffsetStart=NF_TEXT;
        eOffsetEnd=NF_TEXT;
        break;

    case SvNumFormatType::ALL:
        eOffsetStart=NF_NUMERIC_START;
        eOffsetEnd = NfIndexTableOffset( NF_INDEX_TABLE_ENTRIES - 1 );
        break;

    default:
        OSL_FAIL("what a format?");
        break;
    }

    const SvNumberformat* pFormat;
    sal_Int32 i = 0;
    const Color* pCol;
    double fVal = SwNumFormatBase::GetDefValue(nFormatType);
    OUString sValue;

    const sal_uInt32 nSysNumFormat = pFormatter->GetFormatIndex(
                                    NF_NUMBER_SYSTEM, m_eCurLanguage );
    const sal_uInt32 nSysShortDateFormat = pFormatter->GetFormatIndex(
                                    NF_DATE_SYSTEM_SHORT, m_eCurLanguage );
    const sal_uInt32 nSysLongDateFormat = pFormatter->GetFormatIndex(
                                    NF_DATE_SYSTEM_LONG, m_eCurLanguage );

    for( tools::Long nIndex = eOffsetStart; nIndex <= eOffsetEnd; ++nIndex )
    {
        const sal_uInt32 nFormat = pFormatter->GetFormatIndex(
                        static_cast<NfIndexTableOffset>(nIndex), m_eCurLanguage );
        pFormat = pFormatter->GetEntry( nFormat );

        if( nFormat == pFormatter->GetFormatIndex( NF_NUMBER_STANDARD,
                                                    m_eCurLanguage )
            || const_cast<SvNumberformat*>(pFormat)->GetOutputString( fVal, sValue, &pCol )
            || nFormatType == SvNumFormatType::UNDEFINED )
        {
            sValue = pFormat->GetFormatstring();
        }
        else if( nFormatType == SvNumFormatType::TEXT )
        {
            pFormatter->GetOutputString( "\"ABC\"", nFormat, sValue, &pCol);
        }

        if (nFormat != nSysNumFormat       &&
            nFormat != nSysShortDateFormat &&
            nFormat != nSysLongDateFormat)
        {
            append(OUString::number(nFormat), sValue);

            if( nFormat == pFormatter->GetStandardFormat(
                                    nFormatType, m_eCurLanguage ) )
                m_nStdEntry = i;
            ++i;
        }
    }

    append_text(SwResId(STR_DEFINE_NUMBERFORMAT));

    set_active(m_nStdEntry);

    m_nCurrFormatType = nFormatType;
    mbCurrFormatTypeNeedsInit = false;

}

void SwNumFormatBase::SetDefFormat(const sal_uInt32 nDefaultFormat)
{
    if (nDefaultFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        m_nDefFormat = nDefaultFormat;
        return;
    }

    SwView *pView = GetActiveView();
    OSL_ENSURE(pView, "no view found");
    if(!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

    SvNumFormatType nType = pFormatter->GetType(nDefaultFormat);

    SetFormatType(nType);

    sal_uInt32 nFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nDefaultFormat, m_eCurLanguage);

    for (sal_Int32 i = 0, nCount = get_count(); i < nCount; ++i)
    {
        if (nFormat == get_id(i).toUInt32())
        {
            set_active(i);
            m_nStdEntry = i;
            m_nDefFormat = GetFormat();
            return;
        }
    }

    // No entry found:
    OUString sValue;
    const Color* pCol = nullptr;

    if (nType == SvNumFormatType::TEXT)
    {
        pFormatter->GetOutputString("\"ABC\"", nDefaultFormat, sValue, &pCol);
    }
    else
    {
        pFormatter->GetOutputString(SwNumFormatBase::GetDefValue(nType), nDefaultFormat, sValue, &pCol);
    }

    sal_Int32 nPos = 0;
    while (get_id(nPos).toUInt32() == NUMBERFORMAT_ENTRY_NOT_FOUND)
        nPos++;

    if ( lcl_isSystemFormat(nDefaultFormat, pFormatter, m_eCurLanguage) )
    {
        sValue += SwResId(RID_STR_SYSTEM);
    }

    insert_text(nPos, sValue);   // Insert as first numeric entry
    set_id(nPos, OUString::number(nDefaultFormat));
    set_active(nPos);
    m_nDefFormat = GetFormat();
}

sal_uInt32 NumFormatListBox::GetFormat() const
{
    return mxControl->get_active_id().toUInt32();
}

sal_uInt32 SwNumFormatTreeView::GetFormat() const
{
    return mxControl->get_selected_id().toUInt32();
}

void SwNumFormatBase::CallSelectHdl()
{
    const sal_Int32 nPos = get_active();
    OUString sDefine(SwResId( STR_DEFINE_NUMBERFORMAT ));
    SwView *pView = GetActiveView();

    if (!pView || nPos != get_count() - 1 || get_text(nPos) != sDefine)
        return;

    SwWrtShell &rSh = pView->GetWrtShell();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

    SfxItemSetFixed<
            SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,
            SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
            SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
                SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
            SID_ATTR_NUMBERFORMAT_ADD_AUTO,
                SID_ATTR_NUMBERFORMAT_ADD_AUTO>  aCoreSet( rSh.GetAttrPool() );

    double fValue = SwNumFormatBase::GetDefValue(m_nCurrFormatType);

    sal_uInt32 nFormat = pFormatter->GetStandardFormat( m_nCurrFormatType, m_eCurLanguage);
    aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, nFormat ));

    aCoreSet.Put( SvxNumberInfoItem( pFormatter, fValue,
                                        SID_ATTR_NUMBERFORMAT_INFO ) );

    if( (SvNumFormatType::DATE | SvNumFormatType::TIME) & m_nCurrFormatType )
        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, m_bOneArea));

    aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_NOLANGUAGE, !m_bShowLanguageControl));
    aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO, m_bUseAutomaticLanguage));

    // force deselect to break mouse lock on selected entry
    set_active(-1);

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateNumFormatDialog(&get_widget(), aCoreSet));

    if (RET_OK == pDlg->Execute())
    {
        const SvxNumberInfoItem* pFormatInfoItem = pView->GetDocShell()->
                        GetItem( SID_ATTR_NUMBERFORMAT_INFO );

        if( pFormatInfoItem )
        {
            for ( sal_uInt32 key : pFormatInfoItem->GetDelFormats() )
                pFormatter->DeleteEntry( key );
        }

        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
        if( const SfxUInt32Item* pFormatValueItem = pOutSet->GetItemIfSet(
            SID_ATTR_NUMBERFORMAT_VALUE, false ))
        {
            sal_uInt32 nNumberFormat = pFormatValueItem->GetValue();
            // oj #105473# change order of calls
            const SvNumberformat* pFormat = pFormatter->GetEntry(nNumberFormat);
            if( pFormat )
                m_eCurLanguage = pFormat->GetLanguage();
            // SetDefFormat uses eCurLanguage to look for if this format already in the list
            SetDefFormat(nNumberFormat);
        }
        const SfxBoolItem* pAddAutoItem;
        if( m_bShowLanguageControl && (pAddAutoItem = pOutSet->GetItemIfSet(
            SID_ATTR_NUMBERFORMAT_ADD_AUTO, false)))
        {
            m_bUseAutomaticLanguage = pAddAutoItem->GetValue();
        }
    }
    else
        SetDefFormat(nFormat);

}

IMPL_LINK_NOARG(NumFormatListBox, SelectHdl, weld::ComboBox&, void)
{
    CallSelectHdl();
}

IMPL_LINK_NOARG(SwNumFormatTreeView, SelectHdl, weld::TreeView&, void)
{
    CallSelectHdl();
}

void SwNumFormatBase::clear()
{
    mbCurrFormatTypeNeedsInit = true;
    m_nCurrFormatType = SvNumFormatType::ALL;
}

void NumFormatListBox::clear()
{
    mxControl->clear();
    SwNumFormatBase::clear();
}

void SwNumFormatTreeView::clear()
{
    mxControl->clear();
    SwNumFormatBase::clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
