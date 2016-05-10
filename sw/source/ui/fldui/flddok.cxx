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

#include <vcl/svapp.hxx>
#include <flddat.hxx>
#include <docufld.hxx>
#include <globals.hrc>
#include <chpfld.hxx>
#include <fldui.hrc>
#include <flddok.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <svl/zformat.hxx>

#include <index.hrc>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFieldDokPage::SwFieldDokPage(vcl::Window* pParent, const SfxItemSet *const pCoreSet)
    : SwFieldPage(pParent, "FieldDocumentPage",
        "modules/swriter/ui/flddocumentpage.ui", pCoreSet)
    , nOldSel(0)
    , nOldFormat(0)
{
    get(m_pSelection, "selectframe");
    get(m_pFormat, "formatframe");
    get(m_pTypeLB, "type");
    m_pTypeLB->SetStyle(m_pTypeLB->GetStyle() | WB_SORT);
    get(m_pSelectionLB, "select");
    get(m_pValueFT, "valueft");
    get(m_pValueED, "value");
    get(m_pLevelFT, "levelft");
    get(m_pLevelED, "level");
    get(m_pDateFT, "daysft");
    get(m_pTimeFT, "minutesft");
    get(m_pDateOffsetED, "offset");
    get(m_pFormatLB, "format");
    m_pFormatLB->SetStyle(m_pFormatLB->GetStyle() | WB_SORT);
    get(m_pNumFormatLB, "numformat");
    get(m_pFixedCB, "fixed");

    long nHeight = m_pTypeLB->GetTextHeight() * 20;
    m_pTypeLB->set_height_request(nHeight);
    m_pSelectionLB->set_height_request(nHeight);
    m_pFormatLB->set_height_request(nHeight);

    long nWidth = m_pTypeLB->LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MAP_APPFONT)).Width();
    m_pTypeLB->set_width_request(nWidth);
    m_pSelectionLB->set_width_request(nWidth);
    m_pFormatLB->set_width_request(nWidth);

    m_pSelectionLB->SetDoubleClickHdl(LINK(this, SwFieldDokPage, ListBoxInsertHdl));
    m_pFormatLB->SetDoubleClickHdl(LINK(this, SwFieldDokPage, ListBoxInsertHdl));
    m_pNumFormatLB->SetDoubleClickHdl(LINK(this, SwFieldDokPage, NumFormatHdl));

    m_pLevelED->SetMax(MAXLEVEL);
    m_pDateOffsetED->SetMin(LONG_MIN);
    m_pDateOffsetED->SetMax(LONG_MAX);
    //enable 'active' language selection
    m_pNumFormatLB->SetShowLanguageControl(true);
}

SwFieldDokPage::~SwFieldDokPage()
{
    disposeOnce();
}

void SwFieldDokPage::dispose()
{
    m_pTypeLB.clear();
    m_pSelection.clear();
    m_pSelectionLB.clear();
    m_pValueFT.clear();
    m_pValueED.clear();
    m_pLevelFT.clear();
    m_pLevelED.clear();
    m_pDateFT.clear();
    m_pTimeFT.clear();
    m_pDateOffsetED.clear();
    m_pFormat.clear();
    m_pFormatLB.clear();
    m_pNumFormatLB.clear();
    m_pFixedCB.clear();
    SwFieldPage::dispose();
}


void SwFieldDokPage::Reset(const SfxItemSet* )
{
    SavePos(m_pTypeLB);
    Init(); // general initialisation

    // initialise TypeListBox
    const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

    m_pTypeLB->SetUpdateMode(false);
    m_pTypeLB->Clear();

    sal_Int32 nPos;

    if (!IsFieldEdit())
    {
        bool bPage = false;
        // fill Type-Listbox
        for(sal_uInt16 i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            const sal_uInt16 nTypeId = SwFieldMgr::GetTypeId(i);

            switch (nTypeId)
            {
                case TYP_PREVPAGEFLD:
                case TYP_NEXTPAGEFLD:
                case TYP_PAGENUMBERFLD:
                    if (!bPage)
                    {
                        nPos = m_pTypeLB->InsertEntry(SW_RESSTR(FMT_REF_PAGE));
                        m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(USHRT_MAX));
                        bPage = true;
                    }
                    break;

                default:
                    nPos = m_pTypeLB->InsertEntry(SwFieldMgr::GetTypeStr(i));
                    m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
                    break;
            }
        }
    }
    else
    {
        const SwField* pCurField = GetCurField();
        sal_uInt16 nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_FIXDATEFLD)
            nTypeId = TYP_DATEFLD;
        if (nTypeId == TYP_FIXTIMEFLD)
            nTypeId = TYP_TIMEFLD;
        nPos = m_pTypeLB->InsertEntry(SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));
        m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        m_pNumFormatLB->SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
            pSh = ::GetActiveWrtShell();
        if(pSh)
        {
            const SvNumberformat* pFormat = pSh->GetNumberFormatter()->GetEntry(pCurField->GetFormat());
            if(pFormat)
                m_pNumFormatLB->SetLanguage(pFormat->GetLanguage());
        }
    }

    // select old Pos
    RestorePos(m_pTypeLB);

    m_pTypeLB->SetUpdateMode(true);
    m_pTypeLB->SetDoubleClickHdl(LINK(this, SwFieldDokPage, ListBoxInsertHdl));
    m_pTypeLB->SetSelectHdl(LINK(this, SwFieldDokPage, TypeHdl));
    m_pFormatLB->SetSelectHdl(LINK(this, SwFieldDokPage, FormatHdl));

    if( !IsRefresh() )
    {
        const OUString sUserData = GetUserData();
        if (sUserData.getToken(0, ';').equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            const OUString sVal = sUserData.getToken(1, ';');
            const sal_uInt16 nVal = static_cast< sal_uInt16 >(sVal.toInt32());
            if(nVal != USHRT_MAX)
            {
                for(sal_Int32 i = 0; i < m_pTypeLB->GetEntryCount(); i++)
                    if(nVal == (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(i)))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(*m_pTypeLB);

    if (IsFieldEdit())
    {
        nOldSel = m_pSelectionLB->GetSelectEntryPos();
        nOldFormat = GetCurField()->GetFormat();
        m_pFixedCB->SaveValue();
        m_pValueED->SaveValue();
        m_pLevelED->SaveValue();
        m_pDateOffsetED->SaveValue();
    }
}

IMPL_LINK_NOARG_TYPED(SwFieldDokPage, TypeHdl, ListBox&, void)
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
        size_t nCount;

        m_pDateFT->Hide();
        m_pTimeFT->Hide();

        sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

        // fill Selection-Listbox
        m_pSelectionLB->Clear();

        if (nTypeId != USHRT_MAX)
        {
            std::vector<OUString> aLst;
            GetFieldMgr().GetSubTypes(nTypeId, aLst);

            if (nTypeId != TYP_AUTHORFLD)
                nCount = aLst.size();
            else
                nCount = GetFieldMgr().GetFormatCount(nTypeId, false, IsFieldDlgHtmlMode());

            size_t nPos;

            for(size_t i = 0; i < nCount; ++i)
            {
                if (!IsFieldEdit())
                {
                    if (nTypeId != TYP_AUTHORFLD)
                        nPos = m_pSelectionLB->InsertEntry(aLst[i]);
                    else
                        nPos = m_pSelectionLB->InsertEntry(GetFieldMgr().GetFormatStr(nTypeId, i));

                    m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                }
                else
                {
                    bool bInsert = false;

                    switch (nTypeId)
                    {
                        case TYP_DATEFLD:
                        case TYP_TIMEFLD:
                            nPos = m_pSelectionLB->InsertEntry(aLst[i]);
                            m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                            if (static_cast<SwDateTimeField*>(GetCurField())->IsFixed() && !i)
                                m_pSelectionLB->SelectEntryPos(nPos);
                            if (!static_cast<SwDateTimeField*>(GetCurField())->IsFixed() && i)
                                m_pSelectionLB->SelectEntryPos(nPos);
                            break;

                        case TYP_EXTUSERFLD:
                        case TYP_DOCSTATFLD:
                            nPos = m_pSelectionLB->InsertEntry(aLst[i]);
                            m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                            if (GetCurField()->GetSubType() == i)
                                m_pSelectionLB->SelectEntryPos(nPos);
                            break;

                        case TYP_AUTHORFLD:
                        {
                            const OUString sFormat(GetFieldMgr().GetFormatStr(nTypeId, i));
                            nPos = m_pSelectionLB->InsertEntry(sFormat);
                            m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                            m_pSelectionLB->SelectEntry(GetFieldMgr().GetFormatStr(nTypeId, GetCurField()->GetFormat()));
                            break;
                        }

                        default:
                            if (aLst[i] == GetCurField()->GetPar1())
                                bInsert = true;
                            break;
                    }
                    if (bInsert)
                    {
                        nPos = m_pSelectionLB->InsertEntry(aLst[i]);
                        m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                        break;
                    }
                }
            }
            m_pSelectionLB->SetSelectHdl(Link<ListBox&,void>());
        }
        else
        {
            AddSubType(TYP_PAGENUMBERFLD);
            AddSubType(TYP_PREVPAGEFLD);
            AddSubType(TYP_NEXTPAGEFLD);
            nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(0));
            nCount = 3;
            m_pSelectionLB->SetSelectHdl(LINK(this, SwFieldDokPage, SubTypeHdl));
        }

        bool bEnable = nCount != 0;

        if (bEnable && !m_pSelectionLB->GetSelectEntryCount())
            m_pSelectionLB->SelectEntryPos(0);

        m_pSelection->Enable( bEnable );

        // fill Format-Listbox
        sal_Int32 nSize = FillFormatLB(nTypeId);

        bool bValue = false, bLevel = false, bNumFormat = false, bOffset = false;
        bool bFormat = nSize != 0;
        bool bOneArea = false;
        bool bFixed = false;
        sal_uInt16 nFormatType = 0;

        switch (nTypeId)
        {
            case TYP_DATEFLD:
                bFormat = bNumFormat = bOneArea = bOffset = true;

                nFormatType = css::util::NumberFormat::DATE;

                m_pDateFT->Show();

                m_pDateOffsetED->SetFirst(-31);    // one month
                m_pDateOffsetED->SetLast(31);

                if (IsFieldEdit())
                    m_pDateOffsetED->SetValue( static_cast<SwDateTimeField*>(GetCurField())->GetOffset() / 24 / 60);
                break;

            case TYP_TIMEFLD:
                bFormat = bNumFormat = bOneArea = bOffset = true;

                nFormatType = css::util::NumberFormat::TIME;

                m_pTimeFT->Show();

                m_pDateOffsetED->SetFirst(-1440);  // one day
                m_pDateOffsetED->SetLast(1440);

                if (IsFieldEdit())
                    m_pDateOffsetED->SetValue( static_cast<SwDateTimeField*>(GetCurField())->GetOffset() );
                break;

            case TYP_PREVPAGEFLD:
            case TYP_NEXTPAGEFLD:
                if (IsFieldEdit())
                {
                    const sal_uInt16 nTmp = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pFormatLB->GetEntryData(
                                            m_pFormatLB->GetSelectEntryPos() ));

                    if(SVX_NUM_CHAR_SPECIAL != nTmp)
                    {
                        sal_Int32 nOff = GetCurField()->GetPar2().toInt32();
                        if( TYP_NEXTPAGEFLD == nTypeId && 1 != nOff )
                            m_pValueED->SetText(
                                OUString::number(nOff - 1) );
                        else if( TYP_PREVPAGEFLD == nTypeId && -1 != nOff )
                            m_pValueED->SetText(
                                OUString::number(nOff + 1) );
                        else
                            m_pValueED->SetText(aEmptyOUStr);
                    }
                    else
                        m_pValueED->SetText(static_cast<SwPageNumberField*>(GetCurField())->GetUserString());
                }
                bValue = true;
                break;

            case TYP_CHAPTERFLD:
                m_pValueFT->SetText(SW_RESSTR(STR_LEVEL));
                if (IsFieldEdit())
                    m_pLevelED->SetText(OUString::number(static_cast<SwChapterField*>(GetCurField())->GetLevel() + 1));
                bLevel = true;
                break;

            case TYP_PAGENUMBERFLD:
                m_pValueFT->SetText( SW_RESSTR( STR_OFFSET ));
                if (IsFieldEdit())
                    m_pValueED->SetText(GetCurField()->GetPar2());
                bValue = true;
                break;

            case TYP_EXTUSERFLD:
            case TYP_AUTHORFLD:
            case TYP_FILENAMEFLD:
                bFixed = true;
                break;

            default:
                break;
        }

        if (bNumFormat)
        {
            if (IsFieldEdit())
            {
                m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());

                if (m_pNumFormatLB->GetFormatType() == (css::util::NumberFormat::DATE|css::util::NumberFormat::TIME))
                {
                    // always set Format-Type because otherwise when date/time formats are combined,
                    // both formats would be displayed at the same time
                    m_pNumFormatLB->SetFormatType(0);
                    m_pNumFormatLB->SetFormatType(nFormatType);
                    // set correct format once again
                    m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());
                }
            }
            else
                m_pNumFormatLB->SetFormatType(nFormatType);

            m_pNumFormatLB->SetOneArea(bOneArea);
        }

        m_pFormatLB->Show(!bNumFormat);
        m_pNumFormatLB->Show(bNumFormat);

        m_pValueFT->Show(bValue);
        m_pValueED->Show(bValue);
        m_pLevelFT->Show(bLevel);
        m_pLevelED->Show(bLevel);
        m_pDateOffsetED->Show(bOffset);
        m_pFixedCB->Show(!bValue && !bLevel && !bOffset);

        m_pFormat->Enable(bFormat);
        m_pFixedCB->Enable(bFixed);

        if (IsFieldEdit())
            m_pFixedCB->Check( (GetCurField()->GetFormat() & AF_FIXED) != 0 && bFixed );

        if (m_pNumFormatLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
            m_pNumFormatLB->SelectEntryPos(0);
        m_pValueFT->Enable(bValue || bLevel || bOffset);
        m_pValueED->Enable(bValue);
    }
}

void SwFieldDokPage::AddSubType(sal_uInt16 nTypeId)
{
    const sal_Int32 nPos = m_pSelectionLB->InsertEntry(SwFieldType::GetTypeStr(nTypeId));
    m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
}

IMPL_LINK_NOARG_TYPED(SwFieldDokPage, SubTypeHdl, ListBox&, void)
{
    sal_Int32 nPos = m_pSelectionLB->GetSelectEntryPos();
    if(nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nPos));
    FillFormatLB(nTypeId);

    sal_uInt32 nTextRes = 0;
    switch (nTypeId)
    {
    case TYP_CHAPTERFLD:
        nTextRes = STR_LEVEL;
        break;

    case TYP_PREVPAGEFLD:
    case TYP_NEXTPAGEFLD:
        nTextRes = SVX_NUM_CHAR_SPECIAL == (sal_uInt16)reinterpret_cast<sal_uLong>(m_pFormatLB->GetEntryData(
                                        m_pFormatLB->GetSelectEntryPos() ))
                        ? STR_VALUE : STR_OFFSET;
        break;

    case TYP_PAGENUMBERFLD:
        nTextRes = STR_OFFSET;
        break;
    }

    if( nTextRes )
        m_pValueFT->SetText( SW_RESSTR( nTextRes ));
}

sal_Int32 SwFieldDokPage::FillFormatLB(sal_uInt16 nTypeId)
{
    // fill Format-Listbox
    m_pFormatLB->Clear();

    if (nTypeId == TYP_AUTHORFLD)
        return m_pFormatLB->GetEntryCount();

    const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(nTypeId, false, IsFieldDlgHtmlMode());

    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        const sal_Int32 nPos = m_pFormatLB->InsertEntry(GetFieldMgr().GetFormatStr(nTypeId, i));
        const sal_uInt16 nFormatId = GetFieldMgr().GetFormatId( nTypeId, i );
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>( nFormatId ));
        if (IsFieldEdit() && nFormatId == (GetCurField()->GetFormat() & ~AF_FIXED))
            m_pFormatLB->SelectEntryPos( nPos );
    }

    if( nSize && !m_pFormatLB->GetSelectEntryCount() )
    {
        m_pFormatLB->SelectEntry( SW_RESSTR(FMT_NUM_PAGEDESC) );
        if( !m_pFormatLB->GetSelectEntryCount() )
        {
            m_pFormatLB->SelectEntry( SW_RESSTR(FMT_NUM_ARABIC) );
            if( !m_pFormatLB->GetSelectEntryCount() )
                m_pFormatLB->SelectEntryPos( 0 );
        }
    }

    FormatHdl(*m_pFormatLB);

    return nSize;
}

IMPL_LINK_NOARG_TYPED(SwFieldDokPage, FormatHdl, ListBox&, void)
{
    sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

    if (nTypeId == USHRT_MAX)
    {
        sal_Int32 nPos = m_pSelectionLB->GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;

        nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nPos));
    }

    if (nTypeId == TYP_NEXTPAGEFLD || nTypeId == TYP_PREVPAGEFLD)
    {
        // Prev/Next - PageNumFields special treatment:
        sal_uInt16 nTmp = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pFormatLB->GetEntryData(
                                        m_pFormatLB->GetSelectEntryPos() ));
        const OUString sOldText( m_pValueFT->GetText() );
        const OUString sNewText( SW_RES( SVX_NUM_CHAR_SPECIAL == nTmp  ? STR_VALUE
                                                         : STR_OFFSET ));

        if( sOldText != sNewText )
            m_pValueFT->SetText( sNewText );

        if (sOldText != m_pValueFT->GetText())
            m_pValueED->SetText( aEmptyOUStr );
    }
}

bool SwFieldDokPage::FillItemSet(SfxItemSet* )
{
    sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

    if (nTypeId == USHRT_MAX)
    {
        sal_Int32 nPos = m_pSelectionLB->GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;
        nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nPos));
    }

    OUString aVal(m_pValueED->GetText());
    sal_uLong nFormat = 0;
    sal_uInt16 nSubType = 0;

    if (m_pFormatLB->IsEnabled())
    {
        sal_Int32 nPos = m_pFormatLB->GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nFormat = reinterpret_cast<sal_uLong>(m_pFormatLB->GetEntryData(nPos));
    }

    if (m_pSelectionLB->IsEnabled())
    {
        sal_Int32 nPos = m_pSelectionLB->GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nSubType = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nPos));
    }

    switch (nTypeId)
    {
        case TYP_AUTHORFLD:
            nFormat = nSubType;
            nSubType = 0;
            SAL_FALLTHROUGH;
        case TYP_EXTUSERFLD:
            nFormat |= m_pFixedCB->IsChecked() ? AF_FIXED : 0;
            break;

        case TYP_FILENAMEFLD:
            nFormat |= m_pFixedCB->IsChecked() ? FF_FIXED : 0;
            break;

        case TYP_DATEFLD:
        case TYP_TIMEFLD:
        {
            nFormat = m_pNumFormatLB->GetFormat();
            long nVal = static_cast< long >(m_pDateOffsetED->GetValue());
            if (nTypeId == TYP_DATEFLD)
                aVal = OUString::number(nVal * 60 * 24);
            else
                aVal = OUString::number(nVal);
            break;
        }

        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_PAGENUMBERFLD:
        case TYP_GETREFPAGEFLD:
        {
            if( SVX_NUM_CHAR_SPECIAL != nFormat &&
                (TYP_PREVPAGEFLD == nTypeId || TYP_NEXTPAGEFLD == nTypeId))
            {
                sal_Int32 nVal = m_pValueED->GetText().toInt32();
                aVal = OUString::number(nVal);
            }
            break;
        }

        case TYP_CHAPTERFLD:
            aVal = m_pLevelED->GetText();
            break;

        default:
            break;
    }

    if (!IsFieldEdit() ||
        nOldSel != m_pSelectionLB->GetSelectEntryPos() ||
        nOldFormat != nFormat ||
        m_pFixedCB->IsValueChangedFromSaved() ||
        m_pValueED->IsValueChangedFromSaved() ||
        m_pLevelED->IsValueChangedFromSaved() ||
        m_pDateOffsetED->IsValueChangedFromSaved())
    {
        InsertField( nTypeId, nSubType, aEmptyOUStr, aVal, nFormat, ' ', m_pNumFormatLB->IsAutomaticLanguage() );
    }

    return false;
}

VclPtr<SfxTabPage> SwFieldDokPage::Create( vcl::Window* pParent,
                                         const SfxItemSet *const pAttrSet)
{
    return VclPtr<SwFieldDokPage>::Create( pParent, pAttrSet );
}

sal_uInt16 SwFieldDokPage::GetGroup()
{
    return GRP_DOC;
}

void    SwFieldDokPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_pTypeLB->GetSelectEntryPos();
    const sal_uInt16 nTypeSel = ( LISTBOX_ENTRY_NOTFOUND == nEntryPos )
        ? USHRT_MAX : sal::static_int_cast< sal_uInt16 >
            (reinterpret_cast< sal_uIntPtr >(m_pTypeLB->GetEntryData( nEntryPos )));
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
