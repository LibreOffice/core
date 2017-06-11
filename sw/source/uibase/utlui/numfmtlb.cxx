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
#include <comphelper/processfactory.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18nlangtag/lang.h>
#include <svl/zformat.hxx>
#include <svl/eitem.hxx>
#include <svx/svxids.hrc>
#include <svx/numinf.hxx>
#include <vcl/msgbox.hxx>
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
#include "swabstdlg.hxx"
#include "dialog.hrc"
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
    nCurrFormatType     (-1),
    nStdEntry           (0),
    bOneArea            (false),
    nDefFormat          (0),
    pVw                 (nullptr),
    pOwnFormatter       (nullptr),
    bShowLanguageControl(false),
    bUseAutomaticLanguage(true)
{
    Init();
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL makeNumFormatListBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

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
    SwView *pView = GetView();

    if (pView)
        eCurLanguage = pView->GetWrtShell().GetCurLang();
    else
        eCurLanguage = SvtSysLocale().GetLanguageTag().getLanguageType();

    SetFormatType(css::util::NumberFormat::NUMBER);
    SetDefFormat(nDefFormat);

    SetSelectHdl(LINK(this, NumFormatListBox, SelectHdl));
}

NumFormatListBox::~NumFormatListBox()
{
    disposeOnce();
}

void NumFormatListBox::dispose()
{
    delete pOwnFormatter;
    ListBox::dispose();
}

SwView* NumFormatListBox::GetView()
{
    if( pVw )
        return pVw;
    return ::GetActiveView();
}

void NumFormatListBox::SetFormatType(const short nFormatType)
{
    if (nCurrFormatType == -1 ||
        (nCurrFormatType & nFormatType) == 0)   // there are mixed formats, like for example DateTime
    {
        SvNumberFormatter* pFormatter;

        if( pOwnFormatter )
            pFormatter = pOwnFormatter;
        else
        {
            SwView *pView = GetView();
            OSL_ENSURE(pView, "no view found");
            if(!pView)
                return;
            SwWrtShell &rSh = pView->GetWrtShell();
            pFormatter = rSh.GetNumberFormatter();
        }

        Clear();    // Remove all entries from the Listbox

        NfIndexTableOffset eOffsetStart = NF_NUMBER_START;
        NfIndexTableOffset eOffsetEnd = NF_NUMBER_START;

        switch( nFormatType )
        {
        case css::util::NumberFormat::NUMBER:
            eOffsetStart=NF_NUMBER_START;
            eOffsetEnd=NF_NUMBER_END;
            break;

        case css::util::NumberFormat::PERCENT:
            eOffsetStart=NF_PERCENT_START;
            eOffsetEnd=NF_PERCENT_END;
            break;

        case css::util::NumberFormat::CURRENCY:
            eOffsetStart=NF_CURRENCY_START;
            eOffsetEnd=NF_CURRENCY_END;
            break;

        case css::util::NumberFormat::DATETIME:
            eOffsetStart=NF_DATE_START;
            eOffsetEnd=NF_TIME_END;
            break;

        case css::util::NumberFormat::DATE:
            eOffsetStart=NF_DATE_START;
            eOffsetEnd=NF_DATE_END;
            break;

        case css::util::NumberFormat::TIME:
            eOffsetStart=NF_TIME_START;
            eOffsetEnd=NF_TIME_END;
            break;

        case css::util::NumberFormat::SCIENTIFIC:
            eOffsetStart=NF_SCIENTIFIC_START;
            eOffsetEnd=NF_SCIENTIFIC_END;
            break;

        case css::util::NumberFormat::FRACTION:
            eOffsetStart=NF_FRACTION_START;
            eOffsetEnd=NF_FRACTION_END;
            break;

        case css::util::NumberFormat::LOGICAL:
            eOffsetStart=NF_BOOLEAN;
            eOffsetEnd=NF_BOOLEAN;
            break;

        case css::util::NumberFormat::TEXT:
            eOffsetStart=NF_TEXT;
            eOffsetEnd=NF_TEXT;
            break;

        case css::util::NumberFormat::ALL:
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
                            (NfIndexTableOffset)nIndex, eCurLanguage );
            pFormat = pFormatter->GetEntry( nFormat );

            if( nFormat == pFormatter->GetFormatIndex( NF_NUMBER_STANDARD,
                                                        eCurLanguage )
                || const_cast<SvNumberformat*>(pFormat)->GetOutputString( fVal, sValue, &pCol )
                || nFormatType == css::util::NumberFormat::UNDEFINED )
            {
                sValue = pFormat->GetFormatstring();
            }
            else if( nFormatType == css::util::NumberFormat::TEXT )
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

        if (!pOwnFormatter)
        {
            const sal_Int32 nPos = InsertEntry(SwResId( STR_DEFINE_NUMBERFORMAT ));
            SetEntryData( nPos, nullptr );
        }

        SelectEntryPos( nStdEntry );

        nCurrFormatType = nFormatType;
    }
}

void NumFormatListBox::SetDefFormat(const sal_uLong nDefaultFormat)
{
    if (nDefaultFormat == ULONG_MAX)
    {
        nDefFormat = nDefaultFormat;
        return;
    }

    SvNumberFormatter* pFormatter;
    if (pOwnFormatter)
        pFormatter = pOwnFormatter;
    else
    {
        SwView *pView = GetView();
        OSL_ENSURE(pView, "no view found");
        if(!pView)
            return;
        SwWrtShell &rSh = pView->GetWrtShell();
        pFormatter = rSh.GetNumberFormatter();
    }

    short nType = pFormatter->GetType(nDefaultFormat);

    SetFormatType(nType);

    sal_uLong nFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nDefaultFormat, eCurLanguage);

    for (sal_Int32 i = 0; i < GetEntryCount(); i++)
    {
        if (nFormat == reinterpret_cast<sal_uLong>(GetEntryData(i)))
        {
            SelectEntryPos(i);
            nStdEntry = i;
            nDefFormat = GetFormat();
            return;
        }
    }

    // No entry found:
    double fValue = GetDefValue(nType);
    OUString sValue;
    Color* pCol = nullptr;

    if (nType == css::util::NumberFormat::TEXT)
    {
        pFormatter->GetOutputString("\"ABC\"", nDefaultFormat, sValue, &pCol);
    }
    else
    {
        pFormatter->GetOutputString(fValue, nDefaultFormat, sValue, &pCol);
    }

    sal_Int32 nPos = 0;
    while (reinterpret_cast<sal_uLong>(GetEntryData(nPos)) == ULONG_MAX)
        nPos++;

    const sal_uInt32 nSysNumFormat = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, eCurLanguage);
    const sal_uInt32 nSysShortDateFormat = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eCurLanguage);
    const sal_uInt32 nSysLongDateFormat = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_LONG, eCurLanguage);
    bool bSysLang = false;
    if( eCurLanguage == GetAppLanguage() )
        bSysLang = true;
    const sal_uInt32 nNumFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysNumFormat, LANGUAGE_SYSTEM );
    const sal_uInt32 nShortDateFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysShortDateFormat, LANGUAGE_SYSTEM );
    const sal_uInt32 nLongDateFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysLongDateFormat, LANGUAGE_SYSTEM );

    if (
         nDefaultFormat == nSysNumFormat ||
         nDefaultFormat == nSysShortDateFormat ||
         nDefaultFormat == nSysLongDateFormat ||
         (
           bSysLang &&
           (
             nDefaultFormat == nNumFormatForLanguage ||
             nDefaultFormat == nShortDateFormatForLanguage ||
             nDefaultFormat == nLongDateFormatForLanguage
           )
         )
       )
    {
        sValue += SwResId(RID_STR_SYSTEM);
    }

    nPos = InsertEntry(sValue, nPos);   // Insert as first numeric entry
    SetEntryData(nPos, reinterpret_cast<void*>(nDefaultFormat));
    SelectEntryPos(nPos);
    nDefFormat = GetFormat();
}

sal_uLong NumFormatListBox::GetFormat() const
{
    sal_Int32 nPos = GetSelectEntryPos();

    return reinterpret_cast<sal_uLong>(GetEntryData(nPos));
}

IMPL_LINK( NumFormatListBox, SelectHdl, ListBox&, rBox, void )
{
    const sal_Int32 nPos = rBox.GetSelectEntryPos();
    OUString sDefine(SwResId( STR_DEFINE_NUMBERFORMAT ));
    SwView *pView = GetView();

    if( pView && nPos == rBox.GetEntryCount() - 1 &&
        rBox.GetEntry( nPos ) == sDefine )
    {
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

        sal_uLong nFormat = pFormatter->GetStandardFormat( nCurrFormatType, eCurLanguage);
        aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, nFormat ));

        aCoreSet.Put( SvxNumberInfoItem( pFormatter, fValue,
                                            SID_ATTR_NUMBERFORMAT_INFO ) );

        if( (css::util::NumberFormat::DATE | css::util::NumberFormat::TIME) & nCurrFormatType )
            aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, bOneArea));

        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_NOLANGUAGE, !bShowLanguageControl));
        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO, bUseAutomaticLanguage));

        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        ScopedVclPtr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog( this, aCoreSet,
            GetView()->GetViewFrame()->GetFrame().GetFrameInterface(),
            RC_DLG_SWNUMFMTDLG ));
        OSL_ENSURE(pDlg, "Dialog creation failed!");

        if (RET_OK == pDlg->Execute())
        {
            const SfxPoolItem* pItem = pView->GetDocShell()->
                            GetItem( SID_ATTR_NUMBERFORMAT_INFO );

            if( pItem && 0 != static_cast<const SvxNumberInfoItem*>(pItem)->GetDelCount() )
            {
                const sal_uInt32* pDelArr = static_cast<const SvxNumberInfoItem*>(pItem)->GetDelArray();

                for ( sal_uInt32 i = 0; i < static_cast<const SvxNumberInfoItem*>(pItem)->GetDelCount(); i++ )
                    pFormatter->DeleteEntry( pDelArr[i] );
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
}

double NumFormatListBox::GetDefValue(const short nFormatType)
{
    SvxNumValCategory nDefValue = SvxNumValCategory::Standard;

    switch (nFormatType)
    {
        case css::util::NumberFormat::DATE:
        case css::util::NumberFormat::DATE|css::util::NumberFormat::TIME:
            nDefValue = SvxNumValCategory::Date;
            break;

        case css::util::NumberFormat::TIME:
            nDefValue = SvxNumValCategory::Time;
            break;

        case css::util::NumberFormat::TEXT:
        case css::util::NumberFormat::UNDEFINED:
            nDefValue = SvxNumValCategory::Standard;
            break;

        case css::util::NumberFormat::CURRENCY:
            nDefValue = SvxNumValCategory::Currency;
            break;

        case css::util::NumberFormat::PERCENT:
            nDefValue = SvxNumValCategory::Percent;
            break;

        case css::util::NumberFormat::LOGICAL:
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
    nCurrFormatType = -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
