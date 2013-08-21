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

SwFldDokPage::SwFldDokPage(Window* pParent, const SfxItemSet& rCoreSet )
    : SwFldPage(pParent, "FldDocumentPage",
        "modules/swriter/ui/flddocumentpage.ui", rCoreSet)
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

    m_pSelectionLB->SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    m_pFormatLB->SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    m_pNumFormatLB->SetDoubleClickHdl(LINK(this, SwFldDokPage, NumFormatHdl));

    m_pLevelED->SetMax(MAXLEVEL);
    m_pDateOffsetED->SetMin(LONG_MIN);
    m_pDateOffsetED->SetMax(LONG_MAX);
    //enable 'active' language selection
    m_pNumFormatLB->SetShowLanguageControl(sal_True);
}

SwFldDokPage::~SwFldDokPage()
{
}

void SwFldDokPage::Reset(const SfxItemSet& )
{
    SavePos(m_pTypeLB);
    Init(); // general initialisation

    // initialise TypeListBox
    const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

    m_pTypeLB->SetUpdateMode(sal_False);
    m_pTypeLB->Clear();

    sal_uInt16 nPos, nTypeId;

    if (!IsFldEdit())
    {
        bool bPage = false;
        // fill Type-Listbox
        for(short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);

            switch (nTypeId)
            {
                case TYP_PREVPAGEFLD:
                case TYP_NEXTPAGEFLD:
                case TYP_PAGENUMBERFLD:
                    if (!bPage)
                    {
                        nPos = m_pTypeLB->InsertEntry(SW_RESSTR(FMT_REF_PAGE));
                        m_pTypeLB->SetEntryData(nPos, (void*)USHRT_MAX);
                        bPage = true;
                    }
                    break;

                default:
                    nPos = m_pTypeLB->InsertEntry(GetFldMgr().GetTypeStr(i));
                    m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
                    break;
            }
        }
    }
    else
    {
        const SwField* pCurField = GetCurField();
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_FIXDATEFLD)
            nTypeId = TYP_DATEFLD;
        if (nTypeId == TYP_FIXTIMEFLD)
            nTypeId = TYP_TIMEFLD;
        nPos = m_pTypeLB->InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
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

    m_pTypeLB->SetUpdateMode(sal_True);
    m_pTypeLB->SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    m_pTypeLB->SetSelectHdl(LINK(this, SwFldDokPage, TypeHdl));
    m_pFormatLB->SetSelectHdl(LINK(this, SwFldDokPage, FormatHdl));

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if( sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1 ))
        {
            String sVal = sUserData.GetToken(1, ';');
            sal_uInt16 nVal = static_cast< sal_uInt16 >(sVal.ToInt32());
            if(nVal != USHRT_MAX)
            {
                for(sal_uInt16 i = 0; i < m_pTypeLB->GetEntryCount(); i++)
                    if(nVal == (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(i))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    if (IsFldEdit())
    {
        nOldSel = m_pSelectionLB->GetSelectEntryPos();
        nOldFormat = GetCurField()->GetFormat();
        m_pFixedCB->SaveValue();
        m_pValueED->SaveValue();
        m_pLevelED->SaveValue();
        m_pDateOffsetED->SaveValue();
    }
}

IMPL_LINK_NOARG(SwFldDokPage, TypeHdl)
{
    // save old ListBoxPos
    const sal_uInt16 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_pTypeLB->GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        m_pTypeLB->SelectEntryPos(0);
    }

    sal_uInt16 nCount;

    if (nOld != GetTypeSel())
    {
        m_pDateFT->Hide();
        m_pTimeFT->Hide();

        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        // fill Selection-Listbox
        m_pSelectionLB->Clear();

        if (nTypeId != USHRT_MAX)
        {
            std::vector<OUString> aLst;
            GetFldMgr().GetSubTypes(nTypeId, aLst);

            if (nTypeId != TYP_AUTHORFLD)
                nCount = aLst.size();
            else
                nCount = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());

            size_t nPos;

            for(size_t i = 0; i < nCount; ++i)
            {
                if (!IsFldEdit())
                {
                    if (nTypeId != TYP_AUTHORFLD)
                        nPos = m_pSelectionLB->InsertEntry(aLst[i]);
                    else
                        nPos = m_pSelectionLB->InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));

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
                            if (((SwDateTimeField*)GetCurField())->IsFixed() && !i)
                                m_pSelectionLB->SelectEntryPos(nPos);
                            if (!((SwDateTimeField*)GetCurField())->IsFixed() && i)
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
                            String sFmt(GetFldMgr().GetFormatStr(nTypeId, i));
                            nPos = m_pSelectionLB->InsertEntry(sFmt);
                            m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                            m_pSelectionLB->SelectEntry(GetFldMgr().GetFormatStr(nTypeId, GetCurField()->GetFormat()));
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
            m_pSelectionLB->SetSelectHdl(Link());
        }
        else
        {
            AddSubType(TYP_PAGENUMBERFLD);
            AddSubType(TYP_PREVPAGEFLD);
            AddSubType(TYP_NEXTPAGEFLD);
            nTypeId = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(0);
            nCount = 3;
            m_pSelectionLB->SetSelectHdl(LINK(this, SwFldDokPage, SubTypeHdl));
        }

        sal_Bool bEnable = nCount != 0;

        if (bEnable && !m_pSelectionLB->GetSelectEntryCount())
            m_pSelectionLB->SelectEntryPos(0);

        m_pSelection->Enable( bEnable );

        // fill Format-Listbox
        sal_uInt16 nSize = FillFormatLB(nTypeId);

        sal_Bool bValue = sal_False, bLevel = sal_False, bNumFmt = sal_False, bOffset = sal_False;
        sal_Bool bFormat = nSize != 0;
        sal_Bool bOneArea = sal_False;
        sal_Bool bFixed = sal_False;
        sal_uInt16 nFmtType = 0;

        switch (nTypeId)
        {
            case TYP_DATEFLD:
                bFormat = bNumFmt = bOneArea = bOffset = sal_True;

                nFmtType = NUMBERFORMAT_DATE;

                m_pDateFT->Show();

                m_pDateOffsetED->SetFirst(-31);    // one month
                m_pDateOffsetED->SetLast(31);

                if (IsFldEdit())
                    m_pDateOffsetED->SetValue( ((SwDateTimeField*)GetCurField())->GetOffset() / 24 / 60);
                break;

            case TYP_TIMEFLD:
                bFormat = bNumFmt = bOneArea = bOffset = sal_True;

                nFmtType = NUMBERFORMAT_TIME;

                m_pTimeFT->Show();

                m_pDateOffsetED->SetFirst(-1440);  // one day
                m_pDateOffsetED->SetLast(1440);

                if (IsFldEdit())
                    m_pDateOffsetED->SetValue( ((SwDateTimeField*)GetCurField())->GetOffset() );
                break;

            case TYP_PREVPAGEFLD:
            case TYP_NEXTPAGEFLD:
                if (IsFldEdit())
                {
                    sal_uInt16 nTmp = (sal_uInt16)(sal_uLong)m_pFormatLB->GetEntryData(
                                            m_pFormatLB->GetSelectEntryPos() );

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
                            m_pValueED->SetText(aEmptyStr);
                    }
                    else
                        m_pValueED->SetText(((SwPageNumberField*)GetCurField())->GetUserString());
                }
                bValue = sal_True;
                break;

            case TYP_CHAPTERFLD:
                m_pValueFT->SetText(SW_RESSTR(STR_LEVEL));
                if (IsFldEdit())
                    m_pLevelED->SetText(OUString::number(((SwChapterField*)GetCurField())->GetLevel() + 1));
                bLevel = sal_True;
                break;

            case TYP_PAGENUMBERFLD:
                m_pValueFT->SetText( SW_RESSTR( STR_OFFSET ));
                if (IsFldEdit())
                    m_pValueED->SetText(GetCurField()->GetPar2());
                bValue = sal_True;
                break;

            case TYP_EXTUSERFLD:
            case TYP_AUTHORFLD:
            case TYP_FILENAMEFLD:
                bFixed = sal_True;
                break;

            default:
                break;
        }

        if (bNumFmt)
        {
            if (IsFldEdit())
            {
                m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());

                if (m_pNumFormatLB->GetFormatType() == (NUMBERFORMAT_DATE|NUMBERFORMAT_TIME))
                {
                    // always set Format-Type because otherwise when date/time formats are combined,
                    // both formats would be displayed at the same time
                    m_pNumFormatLB->SetFormatType(0);
                    m_pNumFormatLB->SetFormatType(nFmtType);
                    // set correct format once again
                    m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());
                }
            }
            else
                m_pNumFormatLB->SetFormatType(nFmtType);

            m_pNumFormatLB->SetOneArea(bOneArea);
        }

        m_pFormatLB->Show(!bNumFmt);
        m_pNumFormatLB->Show(bNumFmt);

        m_pValueFT->Show(bValue);
        m_pValueED->Show(bValue);
        m_pLevelFT->Show(bLevel);
        m_pLevelED->Show(bLevel);
        m_pDateOffsetED->Show(bOffset);
        m_pFixedCB->Show(!bValue && !bLevel && !bOffset);

        m_pFormat->Enable(bFormat);
        m_pFixedCB->Enable(bFixed);

        if (IsFldEdit())
            m_pFixedCB->Check( static_cast< sal_Bool >(((GetCurField()->GetFormat() & AF_FIXED) != 0) & bFixed));

        if (m_pNumFormatLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
            m_pNumFormatLB->SelectEntryPos(0);
        m_pValueFT->Enable(bValue | bLevel | bOffset);
        m_pValueED->Enable(bValue);
    }

    return 0;
}

void SwFldDokPage::AddSubType(sal_uInt16 nTypeId)
{
    sal_uInt16 nPos = m_pSelectionLB->InsertEntry(SwFieldType::GetTypeStr(nTypeId));
    m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
}

IMPL_LINK_NOARG(SwFldDokPage, SubTypeHdl)
{
    sal_uInt16 nPos = m_pSelectionLB->GetSelectEntryPos();
    if(nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nPos);
    FillFormatLB(nTypeId);

    sal_uInt16 nTextRes = 0;
    switch (nTypeId)
    {
    case TYP_CHAPTERFLD:
        nTextRes = STR_LEVEL;
        break;

    case TYP_PREVPAGEFLD:
    case TYP_NEXTPAGEFLD:
        nTextRes = SVX_NUM_CHAR_SPECIAL == (sal_uInt16)(sal_uLong)m_pFormatLB->GetEntryData(
                                        m_pFormatLB->GetSelectEntryPos() )
                        ? STR_VALUE : STR_OFFSET;
        break;

    case TYP_PAGENUMBERFLD:
        nTextRes = STR_OFFSET;
        break;
    }

    if( nTextRes )
        m_pValueFT->SetText( SW_RESSTR( nTextRes ));

    return 0;
}

sal_uInt16 SwFldDokPage::FillFormatLB(sal_uInt16 nTypeId)
{
    // fill Format-Listbox
    m_pFormatLB->Clear();

    if (nTypeId == TYP_AUTHORFLD)
        return m_pFormatLB->GetEntryCount();

    sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());

    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        sal_uInt16 nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
        sal_uInt16 nFmtId = GetFldMgr().GetFormatId( nTypeId, i );
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>( nFmtId ));
        if (IsFldEdit() && nFmtId == (GetCurField()->GetFormat() & ~AF_FIXED))
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

    FormatHdl();

    return nSize;
}

IMPL_LINK_NOARG(SwFldDokPage, FormatHdl)
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    if (nTypeId == USHRT_MAX)
    {
        sal_uInt16 nPos = m_pSelectionLB->GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;

        nTypeId = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nPos);
    }

    if (nTypeId == TYP_NEXTPAGEFLD || nTypeId == TYP_PREVPAGEFLD)
    {
        // Prev/Next - PageNumFields special treatment:
        sal_uInt16 nTmp = (sal_uInt16)(sal_uLong)m_pFormatLB->GetEntryData(
                                        m_pFormatLB->GetSelectEntryPos() );
        OUString sOldTxt( m_pValueFT->GetText() );
        OUString sNewTxt( SW_RES( SVX_NUM_CHAR_SPECIAL == nTmp  ? STR_VALUE
                                                         : STR_OFFSET ));

        if( sOldTxt != sNewTxt )
            m_pValueFT->SetText( sNewTxt );

        if (sOldTxt != m_pValueFT->GetText())
            m_pValueED->SetText( aEmptyStr );
    }

    return 0;
}

sal_Bool SwFldDokPage::FillItemSet(SfxItemSet& )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    if (nTypeId == USHRT_MAX)
    {
        sal_uInt16 nPos = m_pSelectionLB->GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;
        nTypeId = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nPos);
    }

    String aVal(m_pValueED->GetText());
    sal_uLong nFormat = 0;
    sal_uInt16 nSubType = 0;

    if (m_pFormatLB->IsEnabled())
    {
        sal_uInt16 nPos = m_pFormatLB->GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nFormat = (sal_uInt16)(sal_uLong)m_pFormatLB->GetEntryData(nPos);
    }

    if (m_pSelectionLB->IsEnabled())
    {
        sal_uInt16 nPos = m_pSelectionLB->GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nSubType = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nPos);
    }

    switch (nTypeId)
    {
        case TYP_AUTHORFLD:
            nFormat = nSubType;
            nSubType = 0;
            // no break!
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

    if (!IsFldEdit() ||
        nOldSel != m_pSelectionLB->GetSelectEntryPos() ||
        nOldFormat != nFormat ||
        m_pFixedCB->GetState() != m_pFixedCB->GetSavedValue() ||
        m_pValueED->GetText() != m_pValueED->GetSavedValue() ||
        m_pLevelED->GetText() != m_pLevelED->GetSavedValue() ||
        m_pDateOffsetED->GetText() != m_pDateOffsetED->GetSavedValue())
    {
        InsertFld( nTypeId, nSubType, aEmptyStr, aVal, nFormat, ' ', m_pNumFormatLB->IsAutomaticLanguage() );
    }

    return sal_False;
}

SfxTabPage* SwFldDokPage::Create(   Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldDokPage( pParent, rAttrSet ) );
}

sal_uInt16 SwFldDokPage::GetGroup()
{
    return GRP_DOC;
}

void    SwFldDokPage::FillUserData()
{
    String sData(OUString(USER_DATA_VERSION));
    sData += ';';
    sal_uInt16 nTypeSel = m_pTypeLB->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(m_pTypeLB->GetEntryData( nTypeSel )));
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
