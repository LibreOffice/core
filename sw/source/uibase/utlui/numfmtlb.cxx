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

#include <hintids.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18nlangtag/lang.h>
#include <svl/zformat.hxx>
#include <svl/eitem.hxx>
#include <svx/svxids.hrc>
#include <svx/numinf.hxx>
#include <vcl/builderfactory.hxx>
#include <svx/flagsdef.hxx>
#include <svl/itemset.hxx>
#include <docsh.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <numfmtlb.hxx>
#include <strings.hrc>
#include <swabstdlg.hxx>
#include <unomid.h>
#include <sfx2/viewfrm.hxx>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

/**
 *  Description:
 *     nFormatType: Display the formats of this Type
 *     nDefaultFormat: Select this format and possibly insert it
 */

NumFormatListBox::NumFormatListBox(vcl::Window* pWin, WinBits nStyle) :
    ListBox             ( pWin, nStyle ),
    nCurrFormatType     (SvNumFormatType::ALL),
    mbCurrFormatTypeNeedsInit(true),
    nStdEntry           (0),
    bOneArea            (false),
    nDefFormat          (0),
    bShowLanguageControl(false),
    bUseAutomaticLanguage(true)
{
    Init();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeNumFormatListBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = BuilderUtils::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;
    else
        nBits |= WB_BORDER;

    VclPtrInstance<NumFormatListBox> pListBox(pParent, nBits|WB_SIMPLEMODE);

    if (bDropdown)
        pListBox->EnableAutoSize(true);

    rRet = pListBox;
}

void NumFormatListBox::Init()
{
    SwView *pView = GetActiveView();

    if (pView)
        eCurLanguage = pView->GetWrtShell().GetCurLang();
    else
        eCurLanguage = SvtSysLocale().GetLanguageTag().getLanguageType();

    SetFormatType(SvNumFormatType::NUMBER);
    SetDefFormat(nDefFormat);

    SetSelectHdl(LINK(this, NumFormatListBox, SelectHdl));
}

NumFormatListBox::~NumFormatListBox()
{
    disposeOnce();
}

void NumFormatListBox::SetFormatType(const SvNumFormatType nFormatType)
{
    if (!mbCurrFormatTypeNeedsInit &&
        (nCurrFormatType & nFormatType))   // there are mixed formats, like for example DateTime
        return;

    SwView *pView = GetActiveView();
    OSL_ENSURE(pView, "no view found");
    if(!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

    Clear();    // Remove all entries from the Listbox

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
    Color* pCol;
    double fVal = GetDefValue( nFormatType );
    OUString sValue;

    const sal_uInt32 nSysNumFormat = pFormatter->GetFormatIndex(
                                    NF_NUMBER_SYSTEM, eCurLanguage );
    const sal_uInt32 nSysShortDateFormat = pFormatter->GetFormatIndex(
                                    NF_DATE_SYSTEM_SHORT, eCurLanguage );
    const sal_uInt32 nSysLongDateFormat = pFormatter->GetFormatIndex(
                                    NF_DATE_SYSTEM_LONG, eCurLanguage );

    for( long nIndex = eOffsetStart; nIndex <= eOffsetEnd; ++nIndex )
    {
        const sal_uInt32 nFormat = pFormatter->GetFormatIndex(
                        static_cast<NfIndexTableOffset>(nIndex), eCurLanguage );
        pFormat = pFormatter->GetEntry( nFormat );

        if( nFormat == pFormatter->GetFormatIndex( NF_NUMBER_STANDARD,
                                                    eCurLanguage )
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
            const sal_Int32 nPos = InsertEntry( sValue );
            SetEntryData( nPos, reinterpret_cast<void*>(nFormat) );

            if( nFormat == pFormatter->GetStandardFormat(
                                    nFormatType, eCurLanguage ) )
                nStdEntry = i;
            ++i;
        }
    }

    const sal_Int32 nPos = InsertEntry(SwResId( STR_DEFINE_NUMBERFORMAT ));
    SetEntryData( nPos, nullptr );

    SelectEntryPos( nStdEntry );

    nCurrFormatType = nFormatType;
    mbCurrFormatTypeNeedsInit = false;

}

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

void NumFormatListBox::SetDefFormat(const sal_uInt32 nDefaultFormat)
{
    if (nDefaultFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        nDefFormat = nDefaultFormat;
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

    sal_uInt32 nFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nDefaultFormat, eCurLanguage);

    for (sal_Int32 i = 0; i < GetEntryCount(); i++)
    {
        if (nFormat == static_cast<sal_uInt32>(reinterpret_cast<sal_uIntPtr>(GetEntryData(i))))
        {
            SelectEntryPos(i);
            nStdEntry = i;
            nDefFormat = GetFormat();
            return;
        }
    }

    // No entry found:
    OUString sValue;
    Color* pCol = nullptr;

    if (nType == SvNumFormatType::TEXT)
    {
        pFormatter->GetOutputString("\"ABC\"", nDefaultFormat, sValue, &pCol);
    }
    else
    {
        pFormatter->GetOutputString(GetDefValue(nType), nDefaultFormat, sValue, &pCol);
    }

    sal_Int32 nPos = 0;
    while (static_cast<sal_uInt32>(reinterpret_cast<sal_uIntPtr>(GetEntryData(nPos))) == NUMBERFORMAT_ENTRY_NOT_FOUND)
        nPos++;

    if ( lcl_isSystemFormat(nDefaultFormat, pFormatter, eCurLanguage) )
    {
        sValue += SwResId(RID_STR_SYSTEM);
    }

    nPos = InsertEntry(sValue, nPos);   // Insert as first numeric entry
    SetEntryData(nPos, reinterpret_cast<void*>(nDefaultFormat));
    SelectEntryPos(nPos);
    nDefFormat = GetFormat();
}

sal_uInt32 NumFormatListBox::GetFormat() const
{
    sal_Int32 nPos = GetSelectedEntryPos();

    return static_cast<sal_uInt32>(reinterpret_cast<sal_uIntPtr>(GetEntryData(nPos)));
}

IMPL_LINK( NumFormatListBox, SelectHdl, ListBox&, rBox, void )
{
    const sal_Int32 nPos = rBox.GetSelectedEntryPos();
    OUString sDefine(SwResId( STR_DEFINE_NUMBERFORMAT ));
    SwView *pView = GetActiveView();

    if( !pView || nPos != rBox.GetEntryCount() - 1 ||
        rBox.GetEntry( nPos ) != sDefine )
        return;

    SwWrtShell &rSh = pView->GetWrtShell();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

    SfxItemSet aCoreSet(
        rSh.GetAttrPool(),
        svl::Items<
            SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,
            SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
            SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
                SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
            SID_ATTR_NUMBERFORMAT_ADD_AUTO,
                SID_ATTR_NUMBERFORMAT_ADD_AUTO>{});

    double fValue = GetDefValue( nCurrFormatType);

    sal_uInt32 nFormat = pFormatter->GetStandardFormat( nCurrFormatType, eCurLanguage);
    aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, nFormat ));

    aCoreSet.Put( SvxNumberInfoItem( pFormatter, fValue,
                                        SID_ATTR_NUMBERFORMAT_INFO ) );

    if( (SvNumFormatType::DATE | SvNumFormatType::TIME) & nCurrFormatType )
        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, bOneArea));

    aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_NOLANGUAGE, !bShowLanguageControl));
    aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO, bUseAutomaticLanguage));

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateNumFormatDialog(GetFrameWeld(), aCoreSet));

    if (RET_OK == pDlg->Execute())
    {
        const SfxPoolItem* pItem = pView->GetDocShell()->
                        GetItem( SID_ATTR_NUMBERFORMAT_INFO );

        if( pItem )
        {
            for ( sal_uInt32 key : static_cast<const SvxNumberInfoItem*>(pItem)->GetDelFormats() )
                pFormatter->DeleteEntry( key );
        }

        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
        if( SfxItemState::SET == pOutSet->GetItemState(
            SID_ATTR_NUMBERFORMAT_VALUE, false, &pItem ))
        {
            sal_uInt32 nNumberFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
            // oj #105473# change order of calls
            const SvNumberformat* pFormat = pFormatter->GetEntry(nNumberFormat);
            if( pFormat )
                eCurLanguage = pFormat->GetLanguage();
            // SetDefFormat uses eCurLanguage to look for if this format already in the list
            SetDefFormat(nNumberFormat);
        }
        if( bShowLanguageControl && SfxItemState::SET == pOutSet->GetItemState(
            SID_ATTR_NUMBERFORMAT_ADD_AUTO, false, &pItem ))
        {
            bUseAutomaticLanguage = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        }
    }
    else
        SetDefFormat(nFormat);

}

double NumFormatListBox::GetDefValue(const SvNumFormatType nFormatType)
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

void NumFormatListBox::Clear()
{
    ListBox::Clear();
    mbCurrFormatTypeNeedsInit = true;
    nCurrFormatType = SvNumFormatType::ALL;
}

SwNumFormatListBox::SwNumFormatListBox(std::unique_ptr<weld::ComboBox> xControl)
    : nCurrFormatType(SvNumFormatType::ALL)
    , mbCurrFormatTypeNeedsInit(true)
    , nStdEntry(0)
    , nDefFormat(0)
    , bUseAutomaticLanguage(true)
    , mxControl(std::move(xControl))
{
    Init();
}

void SwNumFormatListBox::Init()
{
    SwView *pView = GetActiveView();

    if (pView)
        eCurLanguage = pView->GetWrtShell().GetCurLang();
    else
        eCurLanguage = SvtSysLocale().GetLanguageTag().getLanguageType();

    SetFormatType(SvNumFormatType::NUMBER);
    SetDefFormat(nDefFormat);

    mxControl->connect_changed(LINK(this, SwNumFormatListBox, SelectHdl));
}

SwNumFormatListBox::~SwNumFormatListBox()
{
}

void SwNumFormatListBox::SetFormatType(const SvNumFormatType nFormatType)
{
    if (!mbCurrFormatTypeNeedsInit &&
        (nCurrFormatType & nFormatType))   // there are mixed formats, like for example DateTime
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
    Color* pCol;
    double fVal = NumFormatListBox::GetDefValue(nFormatType);
    OUString sValue;

    const sal_uInt32 nSysNumFormat = pFormatter->GetFormatIndex(
                                    NF_NUMBER_SYSTEM, eCurLanguage );
    const sal_uInt32 nSysShortDateFormat = pFormatter->GetFormatIndex(
                                    NF_DATE_SYSTEM_SHORT, eCurLanguage );
    const sal_uInt32 nSysLongDateFormat = pFormatter->GetFormatIndex(
                                    NF_DATE_SYSTEM_LONG, eCurLanguage );

    for( long nIndex = eOffsetStart; nIndex <= eOffsetEnd; ++nIndex )
    {
        const sal_uInt32 nFormat = pFormatter->GetFormatIndex(
                        static_cast<NfIndexTableOffset>(nIndex), eCurLanguage );
        pFormat = pFormatter->GetEntry( nFormat );

        if( nFormat == pFormatter->GetFormatIndex( NF_NUMBER_STANDARD,
                                                    eCurLanguage )
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
            mxControl->append(OUString::number(nFormat), sValue);

            if( nFormat == pFormatter->GetStandardFormat(
                                    nFormatType, eCurLanguage ) )
                nStdEntry = i;
            ++i;
        }
    }

    mxControl->append_text(SwResId(STR_DEFINE_NUMBERFORMAT));

    mxControl->set_active(nStdEntry);

    nCurrFormatType = nFormatType;
    mbCurrFormatTypeNeedsInit = false;

}

void SwNumFormatListBox::SetDefFormat(const sal_uInt32 nDefaultFormat)
{
    if (nDefaultFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        nDefFormat = nDefaultFormat;
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

    sal_uInt32 nFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nDefaultFormat, eCurLanguage);

    for (sal_Int32 i = 0, nCount = mxControl->get_count(); i < nCount; ++i)
    {
        if (nFormat == mxControl->get_id(i).toUInt32())
        {
            mxControl->set_active(i);
            nStdEntry = i;
            nDefFormat = GetFormat();
            return;
        }
    }

    // No entry found:
    OUString sValue;
    Color* pCol = nullptr;

    if (nType == SvNumFormatType::TEXT)
    {
        pFormatter->GetOutputString("\"ABC\"", nDefaultFormat, sValue, &pCol);
    }
    else
    {
        pFormatter->GetOutputString(NumFormatListBox::GetDefValue(nType), nDefaultFormat, sValue, &pCol);
    }

    sal_Int32 nPos = 0;
    while (mxControl->get_id(nPos).toUInt32() == NUMBERFORMAT_ENTRY_NOT_FOUND)
        nPos++;

    if ( lcl_isSystemFormat(nDefaultFormat, pFormatter, eCurLanguage) )
    {
        sValue += SwResId(RID_STR_SYSTEM);
    }

    mxControl->insert_text(nPos, sValue);   // Insert as first numeric entry
    mxControl->set_id(nPos, OUString::number(nDefaultFormat));
    mxControl->set_active(nPos);
    nDefFormat = GetFormat();
}

sal_uInt32 SwNumFormatListBox::GetFormat() const
{
    return mxControl->get_active_id().toUInt32();
}

void SwNumFormatListBox::CallSelectHdl()
{
    const sal_Int32 nPos = mxControl->get_active();
    OUString sDefine(SwResId( STR_DEFINE_NUMBERFORMAT ));
    SwView *pView = GetActiveView();

    if( !pView || nPos != mxControl->get_count() - 1 ||
        mxControl->get_text(nPos) != sDefine )
        return;

    SwWrtShell &rSh = pView->GetWrtShell();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

    SfxItemSet aCoreSet(
        rSh.GetAttrPool(),
        svl::Items<
            SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,
            SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
            SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
                SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
            SID_ATTR_NUMBERFORMAT_ADD_AUTO,
                SID_ATTR_NUMBERFORMAT_ADD_AUTO>{});

    double fValue = NumFormatListBox::GetDefValue(nCurrFormatType);

    sal_uInt32 nFormat = pFormatter->GetStandardFormat( nCurrFormatType, eCurLanguage);
    aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, nFormat ));

    aCoreSet.Put( SvxNumberInfoItem( pFormatter, fValue,
                                        SID_ATTR_NUMBERFORMAT_INFO ) );

    if( (SvNumFormatType::DATE | SvNumFormatType::TIME) & nCurrFormatType )
        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, false));

    aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_NOLANGUAGE, true));
    aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO, bUseAutomaticLanguage));

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateNumFormatDialog(mxControl.get(), aCoreSet));

    if (RET_OK == pDlg->Execute())
    {
        const SfxPoolItem* pItem = pView->GetDocShell()->
                        GetItem( SID_ATTR_NUMBERFORMAT_INFO );

        if( pItem )
        {
            for ( sal_uInt32 key : static_cast<const SvxNumberInfoItem*>(pItem)->GetDelFormats() )
                pFormatter->DeleteEntry( key );
        }

        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
        if( SfxItemState::SET == pOutSet->GetItemState(
            SID_ATTR_NUMBERFORMAT_VALUE, false, &pItem ))
        {
            sal_uInt32 nNumberFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
            // oj #105473# change order of calls
            const SvNumberformat* pFormat = pFormatter->GetEntry(nNumberFormat);
            if( pFormat )
                eCurLanguage = pFormat->GetLanguage();
            // SetDefFormat uses eCurLanguage to look for if this format already in the list
            SetDefFormat(nNumberFormat);
        }

    }
    else
        SetDefFormat(nFormat);

}

IMPL_LINK_NOARG(SwNumFormatListBox, SelectHdl, weld::ComboBox&, void)
{
    CallSelectHdl();
}

void SwNumFormatListBox::clear()
{
    mxControl->clear();
    mbCurrFormatTypeNeedsInit = true;
    nCurrFormatType = SvNumFormatType::ALL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
