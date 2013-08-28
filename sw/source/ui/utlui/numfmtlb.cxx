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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/localedatawrapper.hxx>
#include <i18nlangtag/lang.h>
#include <svl/zformat.hxx>
#include <svl/eitem.hxx>
#include <svx/svxids.hrc>
#include <svx/numinf.hxx>
#include <vcl/msgbox.hxx>
#include <svx/flagsdef.hxx>
#include <svl/itemset.hxx>
#include <docsh.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <numfmtlb.hxx>
#include <utlui.hrc>
#include "swabstdlg.hxx"
#include "dialog.hrc"
#include <unomid.h>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


// STATIC DATA

/**
 *  Description:
 *     nFormatType: Display the formats of this Type
 *     nDefFmt:     Select this format and possibly insert it
 */

NumFormatListBox::NumFormatListBox(Window* pWin, WinBits nStyle) :
    ListBox             ( pWin, nStyle ),
    nCurrFormatType     (-1),
    nStdEntry           (0),
    bOneArea            (sal_False),
    nDefFormat          (0),
    pVw                 (0),
    pOwnFormatter       (0),
    bShowLanguageControl(sal_False),
    bUseAutomaticLanguage(sal_True)
{
    Init(NUMBERFORMAT_NUMBER, true);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeNumFormatListBox(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;
    else
        nBits |= WB_BORDER;

    NumFormatListBox* pListBox = new NumFormatListBox(pParent, nBits|WB_SIMPLEMODE);

    if (bDropdown)
        pListBox->EnableAutoSize(true);

    return pListBox;
}

NumFormatListBox::NumFormatListBox( Window* pWin, SwView* pView,
                                    const ResId& rResId, short nFormatType,
                                    sal_uLong nDefFmt, sal_Bool bUsrFmts ) :
    ListBox             ( pWin, rResId ),
    nCurrFormatType     (-1),
    nStdEntry           (0),
    bOneArea            (sal_False),
    nDefFormat          (nDefFmt),
    pVw                 (pView),
    pOwnFormatter       (0),
    bShowLanguageControl(sal_False),
    bUseAutomaticLanguage(sal_True)
{
    Init(nFormatType, bUsrFmts);
}

void NumFormatListBox::Init(short nFormatType, sal_Bool bUsrFmts)
{
    SwView *pView = GetView();

    if (pView)
        eCurLanguage = pView->GetWrtShell().GetCurLang();
    else
        eCurLanguage = SvtSysLocale().GetLanguageTag().getLanguageType();

    if (bUsrFmts == sal_False)
    {
        pOwnFormatter = new SvNumberFormatter(comphelper::getProcessComponentContext(), eCurLanguage);
    }

    SetFormatType(nFormatType);
    SetDefFormat(nDefFormat);

    SetSelectHdl(LINK(this, NumFormatListBox, SelectHdl));
}

NumFormatListBox::~NumFormatListBox()
{
    delete pOwnFormatter;
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
        case NUMBERFORMAT_NUMBER:
            eOffsetStart=NF_NUMBER_START;
            eOffsetEnd=NF_NUMBER_END;
            break;

        case NUMBERFORMAT_PERCENT:
            eOffsetStart=NF_PERCENT_START;
            eOffsetEnd=NF_PERCENT_END;
            break;

        case NUMBERFORMAT_CURRENCY:
            eOffsetStart=NF_CURRENCY_START;
            eOffsetEnd=NF_CURRENCY_END;
            break;

        case NUMBERFORMAT_DATETIME:
            eOffsetStart=NF_DATE_START;
            eOffsetEnd=NF_TIME_END;
            break;

        case NUMBERFORMAT_DATE:
            eOffsetStart=NF_DATE_START;
            eOffsetEnd=NF_DATE_END;
            break;

        case NUMBERFORMAT_TIME:
            eOffsetStart=NF_TIME_START;
            eOffsetEnd=NF_TIME_END;
            break;

        case NUMBERFORMAT_SCIENTIFIC:
            eOffsetStart=NF_SCIENTIFIC_START;
            eOffsetEnd=NF_SCIENTIFIC_END;
            break;

        case NUMBERFORMAT_FRACTION:
            eOffsetStart=NF_FRACTION_START;
            eOffsetEnd=NF_FRACTION_END;
            break;

        case NUMBERFORMAT_LOGICAL:
            eOffsetStart=NF_BOOLEAN;
            eOffsetEnd=NF_BOOLEAN;
            break;

        case NUMBERFORMAT_TEXT:
            eOffsetStart=NF_TEXT;
            eOffsetEnd=NF_TEXT;
            break;

        case NUMBERFORMAT_ALL:
            eOffsetStart=NF_NUMERIC_START;
            eOffsetEnd = NfIndexTableOffset( NF_INDEX_TABLE_ENTRIES - 1 );
            break;

        default:
            OSL_FAIL("what a format?");
            break;
        }

        const SvNumberformat* pFmt;
        sal_uInt16 nPos, i = 0;
        sal_uLong  nFormat;
        Color* pCol;
        double fVal = GetDefValue( nFormatType );
        OUString sValue;

        sal_uLong nSysNumFmt = pFormatter->GetFormatIndex(
                                        NF_NUMBER_SYSTEM, eCurLanguage );
        sal_uLong nSysShortDateFmt = pFormatter->GetFormatIndex(
                                        NF_DATE_SYSTEM_SHORT, eCurLanguage );
        sal_uLong nSysLongDateFmt = pFormatter->GetFormatIndex(
                                        NF_DATE_SYSTEM_LONG, eCurLanguage );

        for( long nIndex = eOffsetStart; nIndex <= eOffsetEnd; ++nIndex )
        {
            nFormat = pFormatter->GetFormatIndex(
                            (NfIndexTableOffset)nIndex, eCurLanguage );
            pFmt = pFormatter->GetEntry( nFormat );

            if( nFormat == pFormatter->GetFormatIndex( NF_NUMBER_STANDARD,
                                                        eCurLanguage )
                || ((SvNumberformat*)pFmt)->GetOutputString( fVal, sValue, &pCol )
                || nFormatType == NUMBERFORMAT_UNDEFINED )
            {
                sValue = pFmt->GetFormatstring();
            }
            else if( nFormatType == NUMBERFORMAT_TEXT )
            {
                OUString sTxt("\"ABC\"");
                pFormatter->GetOutputString( sTxt, nFormat, sValue, &pCol);
            }

            if (nFormat != nSysNumFmt       &&
                nFormat != nSysShortDateFmt &&
                nFormat != nSysLongDateFmt)
            {
                nPos = InsertEntry( sValue );
                SetEntryData( nPos, (void*)nFormat );

                if( nFormat == pFormatter->GetStandardFormat(
                                        nFormatType, eCurLanguage ) )
                    nStdEntry = i;
                ++i;
            }
        }

        if (!pOwnFormatter)
        {
            nPos = InsertEntry(SW_RESSTR( STR_DEFINE_NUMBERFORMAT ));
            SetEntryData( nPos, NULL );
        }

        SelectEntryPos( nStdEntry );

        nCurrFormatType = nFormatType;
    }
}

void NumFormatListBox::SetDefFormat(const sal_uLong nDefFmt)
{
    if (nDefFmt == ULONG_MAX)
    {
        nDefFormat = nDefFmt;
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

    short nType = pFormatter->GetType(nDefFmt);

    SetFormatType(nType);

    sal_uLong nFormat = pFormatter->GetFormatForLanguageIfBuiltIn(nDefFmt, eCurLanguage);

    for (sal_uInt16 i = 0; i < GetEntryCount(); i++)
    {
        if (nFormat == (sal_uLong)GetEntryData(i))
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
    Color* pCol = 0;

    if (nType == NUMBERFORMAT_TEXT)
    {
        OUString sTxt("\"ABC\"");
        pFormatter->GetOutputString(sTxt, nDefFmt, sValue, &pCol);
    }
    else
        pFormatter->GetOutputString(fValue, nDefFmt, sValue, &pCol);

    sal_uInt16 nPos = 0;
    while ((sal_uLong)GetEntryData(nPos) == ULONG_MAX)
        nPos++;

    sal_uLong nSysNumFmt = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, eCurLanguage);
    sal_uLong nSysShortDateFmt = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_SHORT, eCurLanguage);
    sal_uLong nSysLongDateFmt = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_LONG, eCurLanguage);
    bool bSysLang = false;
    if( eCurLanguage == GetAppLanguage() )
        bSysLang = true;
    sal_uLong nNumFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysNumFmt, LANGUAGE_SYSTEM );
    sal_uLong nShortDateFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysShortDateFmt, LANGUAGE_SYSTEM );
    sal_uLong nLongDateFormatForLanguage = pFormatter->GetFormatForLanguageIfBuiltIn(nSysLongDateFmt, LANGUAGE_SYSTEM );

    if (
         nDefFmt == nSysNumFmt ||
         nDefFmt == nSysShortDateFmt ||
         nDefFmt == nSysLongDateFmt ||
         (
           bSysLang &&
           (
             nDefFmt == nNumFormatForLanguage ||
             nDefFmt == nShortDateFormatForLanguage ||
             nDefFmt == nLongDateFormatForLanguage
           )
         )
       )
    {
        sValue += String(SW_RES(RID_STR_SYSTEM));
    }

    nPos = InsertEntry(sValue, nPos);   // Insert as first numeric entry
    SetEntryData(nPos, (void*)nDefFmt);
    SelectEntryPos(nPos);
    nDefFormat = GetFormat();
}

sal_uLong NumFormatListBox::GetFormat() const
{
    sal_uInt16 nPos = GetSelectEntryPos();

    return (sal_uLong)GetEntryData(nPos);
}

IMPL_LINK( NumFormatListBox, SelectHdl, ListBox *, pBox )
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();
    String sDefine(SW_RES( STR_DEFINE_NUMBERFORMAT ));
    SwView *pView = GetView();

    if( pView && nPos == pBox->GetEntryCount() - 1 &&
        pBox->GetEntry( nPos ) == sDefine )
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

        SfxItemSet aCoreSet( rSh.GetAttrPool(),
            SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,
            SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO,
            SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
            SID_ATTR_NUMBERFORMAT_NOLANGUAGE, SID_ATTR_NUMBERFORMAT_NOLANGUAGE,
            SID_ATTR_NUMBERFORMAT_ADD_AUTO, SID_ATTR_NUMBERFORMAT_ADD_AUTO,
            0 );

        double fValue = GetDefValue( nCurrFormatType);

        sal_uLong nFormat = pFormatter->GetStandardFormat( nCurrFormatType, eCurLanguage);
        aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE, nFormat ));

        aCoreSet.Put( SvxNumberInfoItem( pFormatter, fValue,
                                            SID_ATTR_NUMBERFORMAT_INFO ) );

        if( (NUMBERFORMAT_DATE | NUMBERFORMAT_TIME) & nCurrFormatType )
            aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, bOneArea));

        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_NOLANGUAGE, !bShowLanguageControl));
        aCoreSet.Put(SfxBoolItem(SID_ATTR_NUMBERFORMAT_ADD_AUTO, bUseAutomaticLanguage));

        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( this, aCoreSet,
            GetView()->GetViewFrame()->GetFrame().GetFrameInterface(),
            RC_DLG_SWNUMFMTDLG );
        OSL_ENSURE(pDlg, "Dialogdiet fail!");

        if (RET_OK == pDlg->Execute())
        {
            const SfxPoolItem* pItem = pView->GetDocShell()->
                            GetItem( SID_ATTR_NUMBERFORMAT_INFO );

            if( pItem && 0 != ((SvxNumberInfoItem*)pItem)->GetDelCount() )
            {
                const sal_uInt32* pDelArr = ((SvxNumberInfoItem*)pItem)->GetDelArray();

                for ( sal_uInt16 i = 0; i < ((SvxNumberInfoItem*)pItem)->GetDelCount(); i++ )
                    pFormatter->DeleteEntry( pDelArr[i] );
            }

            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
            if( SFX_ITEM_SET == pOutSet->GetItemState(
                SID_ATTR_NUMBERFORMAT_VALUE, sal_False, &pItem ))
            {
                sal_uInt32 nNumberFormat = ((SfxUInt32Item*)pItem)->GetValue();
                // oj #105473# change order of calls
                const SvNumberformat* pFmt = pFormatter->GetEntry(nNumberFormat);
                if( pFmt )
                    eCurLanguage = pFmt->GetLanguage();
                // SetDefFormat uses eCurLanguage to look for if this format already in the list
                SetDefFormat(nNumberFormat);
            }
            if( bShowLanguageControl && SFX_ITEM_SET == pOutSet->GetItemState(
                SID_ATTR_NUMBERFORMAT_ADD_AUTO, sal_False, &pItem ))
            {
                bUseAutomaticLanguage = ((const SfxBoolItem*)pItem)->GetValue();
            }
        }
        else
            SetDefFormat(nFormat);

        delete pDlg;
    }
    return 0;
}

double NumFormatListBox::GetDefValue(const short nFormatType) const
{
    double fDefValue = 0.0;

    switch (nFormatType)
    {
        case NUMBERFORMAT_DATE:
        case NUMBERFORMAT_DATE|NUMBERFORMAT_TIME:
            fDefValue = SVX_NUMVAL_DATE;
            break;

        case NUMBERFORMAT_TIME:
            fDefValue = SVX_NUMVAL_TIME;
            break;

        case NUMBERFORMAT_TEXT:
        case NUMBERFORMAT_UNDEFINED:
            fDefValue = 0;
            break;

        case NUMBERFORMAT_CURRENCY:
            fDefValue = SVX_NUMVAL_CURRENCY;
            break;

        case NUMBERFORMAT_PERCENT:
            fDefValue = SVX_NUMVAL_PERCENT;
            break;

        case NUMBERFORMAT_LOGICAL:
            fDefValue = SVX_NUMVAL_BOOLEAN;
            break;

        default:
            fDefValue = SVX_NUMVAL_STANDARD;
            break;
    }

    return fDefValue;
}

void NumFormatListBox::Clear()
{
    ListBox::Clear();
    nCurrFormatType = -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
