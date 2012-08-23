/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <vcl/svapp.hxx>
#include <flddat.hxx>
#include <docufld.hxx>
#include <globals.hrc>
#include <chpfld.hxx>
#include <fldtdlg.hrc>
#include <fldui.hrc>
#include <flddok.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <svl/zformat.hxx>

#include <index.hrc>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldDokPage::SwFldDokPage(Window* pWindow, const SfxItemSet& rCoreSet ) :
    SwFldPage( pWindow, SW_RES( TP_FLD_DOK ), rCoreSet ),

    aTypeFT         (this, SW_RES(FT_DOKTYPE)),
    aTypeLB         (this, SW_RES(LB_DOKTYPE)),
    aSelectionFT    (this, SW_RES(FT_DOKSELECTION)),
    aSelectionLB    (this, SW_RES(LB_DOKSELECTION)),
    aValueFT        (this, SW_RES(FT_DOKVALUE)),
    aValueED        (this, SW_RES(ED_DOKVALUE)),
    aLevelED        (this, SW_RES(ED_DOKLEVEL)),
    aDateOffsetED   (this, SW_RES(ED_DOKDATEOFF)),

    aFormatFT       (this, SW_RES(FT_DOKFORMAT)),
    aFormatLB       (this, SW_RES(LB_DOKFORMAT)),
    aNumFormatLB    (this, SW_RES(LB_DOKNUMFORMAT)),
    aFixedCB        (this, SW_RES(CB_DOKFIXEDCONTENT)),

    sDateOffset     (SW_RES(STR_DOKDATEOFF)),
    sTimeOffset     (SW_RES(STR_DOKTIMEOFF)),
    aRootOpened     (SW_RES(BMP_DOKROOT_OPENED)),
    aRootClosed     (SW_RES(BMP_DOKROOT_CLOSED))
{
    FreeResource();

    aSelectionLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    aFormatLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    aNumFormatLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, NumFormatHdl));

    aLevelED.SetMax(MAXLEVEL);
    aDateOffsetED.SetMin(LONG_MIN);
    aDateOffsetED.SetMax(LONG_MAX);
    //enable 'active' language selection
    aNumFormatLB.SetShowLanguageControl(sal_True);
}

SwFldDokPage::~SwFldDokPage()
{
}

void SwFldDokPage::Reset(const SfxItemSet& )
{
    SavePos(&aTypeLB);
    Init(); // general initialisation

    // initialise TypeListBox
    const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

    aTypeLB.SetUpdateMode(sal_False);
    aTypeLB.Clear();

    sal_uInt16 nPos, nTypeId;
    sal_Bool bPage = sal_False;

    if (!IsFldEdit())
    {
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
                        nPos = aTypeLB.InsertEntry(SW_RESSTR(FMT_REF_PAGE));
                        aTypeLB.SetEntryData(nPos, (void*)USHRT_MAX);
                        bPage = sal_True;
                    }
                    break;

                default:
                    nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
                    aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
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
        nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        aNumFormatLB.SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
            pSh = ::GetActiveWrtShell();
        if(pSh)
        {
            const SvNumberformat* pFormat = pSh->GetNumberFormatter()->GetEntry(pCurField->GetFormat());
            if(pFormat)
                aNumFormatLB.SetLanguage(pFormat->GetLanguage());
        }
    }

    // select old Pos
    RestorePos(&aTypeLB);

    aTypeLB.SetUpdateMode(sal_True);
    aTypeLB.SetDoubleClickHdl(LINK(this, SwFldDokPage, InsertHdl));
    aTypeLB.SetSelectHdl(LINK(this, SwFldDokPage, TypeHdl));
    aFormatLB.SetSelectHdl(LINK(this, SwFldDokPage, FormatHdl));

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if( sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1 ))
        {
            String sVal = sUserData.GetToken(1, ';');
            sal_uInt16 nVal = static_cast< sal_uInt16 >(sVal.ToInt32());
            if(nVal != USHRT_MAX)
            {
                for(sal_uInt16 i = 0; i < aTypeLB.GetEntryCount(); i++)
                    if(nVal == (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(i))
                    {
                        aTypeLB.SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    if (IsFldEdit())
    {
        nOldSel = aSelectionLB.GetSelectEntryPos();
        nOldFormat = GetCurField()->GetFormat();
        aFixedCB.SaveValue();
        aValueED.SaveValue();
        aLevelED.SaveValue();
        aDateOffsetED.SaveValue();
    }
}

IMPL_LINK_NOARG(SwFldDokPage, TypeHdl)
{
    // save old ListBoxPos
    const sal_uInt16 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        aTypeLB.SelectEntryPos(0);
    }

    sal_uInt16 nCount;

    if (nOld != GetTypeSel())
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

        // fill Selection-Listbox
        aSelectionLB.Clear();

        if (nTypeId != USHRT_MAX)
        {
            std::vector<rtl::OUString> aLst;
            GetFldMgr().GetSubTypes(nTypeId, aLst);

            if (nTypeId != TYP_AUTHORFLD)
                nCount = aLst.size();
            else
                nCount = GetFldMgr().GetFormatCount(nTypeId, sal_False, IsFldDlgHtmlMode());

            size_t nPos;

            for(size_t i = 0; i < nCount; ++i)
            {
                if (!IsFldEdit())
                {
                    if (nTypeId != TYP_AUTHORFLD)
                        nPos = aSelectionLB.InsertEntry(aLst[i]);
                    else
                        nPos = aSelectionLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));

                    aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
                }
                else
                {
                    sal_Bool bInsert = sal_False;

                    switch (nTypeId)
                    {
                        case TYP_DATEFLD:
                        case TYP_TIMEFLD:
                            nPos = aSelectionLB.InsertEntry(aLst[i]);
                            aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
                            if (((SwDateTimeField*)GetCurField())->IsFixed() && !i)
                                aSelectionLB.SelectEntryPos(nPos);
                            if (!((SwDateTimeField*)GetCurField())->IsFixed() && i)
                                aSelectionLB.SelectEntryPos(nPos);
                            break;

                        case TYP_EXTUSERFLD:
                        case TYP_DOCSTATFLD:
                            nPos = aSelectionLB.InsertEntry(aLst[i]);
                            aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
                            if (GetCurField()->GetSubType() == i)
                                aSelectionLB.SelectEntryPos(nPos);
                            break;

                        case TYP_AUTHORFLD:
                        {
                            String sFmt(GetFldMgr().GetFormatStr(nTypeId, i));
                            nPos = aSelectionLB.InsertEntry(sFmt);
                            aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
                            aSelectionLB.SelectEntry(GetFldMgr().GetFormatStr(nTypeId, GetCurField()->GetFormat()));
                            break;
                        }

                        default:
                            if (aLst[i] == GetCurField()->GetPar1())
                                bInsert = sal_True;
                            break;
                    }
                    if (bInsert)
                    {
                        nPos = aSelectionLB.InsertEntry(aLst[i]);
                        aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
                        break;
                    }
                }
            }
            aSelectionLB.SetSelectHdl(Link());
        }
        else
        {
            AddSubType(TYP_PAGENUMBERFLD);
            AddSubType(TYP_PREVPAGEFLD);
            AddSubType(TYP_NEXTPAGEFLD);
            nTypeId = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(0);
            nCount = 3;
            aSelectionLB.SetSelectHdl(LINK(this, SwFldDokPage, SubTypeHdl));
        }

        sal_Bool bEnable = nCount != 0;

        if (bEnable && !aSelectionLB.GetSelectEntryCount())
            aSelectionLB.SelectEntryPos(0);

        aSelectionLB.Enable( bEnable );
        aSelectionFT.Enable( bEnable );

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
                aValueFT.SetText(sDateOffset);
                aDateOffsetED.SetFirst(-31);    // one month
                aDateOffsetED.SetLast(31);

                if (IsFldEdit())
                    aDateOffsetED.SetValue( ((SwDateTimeField*)GetCurField())->GetOffset() / 24 / 60);
                break;

            case TYP_TIMEFLD:
                bFormat = bNumFmt = bOneArea = bOffset = sal_True;

                nFmtType = NUMBERFORMAT_TIME;
                aValueFT.SetText(sTimeOffset);
                aDateOffsetED.SetFirst(-1440);  // one day
                aDateOffsetED.SetLast(1440);

                if (IsFldEdit())
                    aDateOffsetED.SetValue( ((SwDateTimeField*)GetCurField())->GetOffset() );
                break;

            case TYP_PREVPAGEFLD:
            case TYP_NEXTPAGEFLD:
                if (IsFldEdit())
                {
                    sal_uInt16 nTmp = (sal_uInt16)(sal_uLong)aFormatLB.GetEntryData(
                                            aFormatLB.GetSelectEntryPos() );
                    String sOldTxt(aValueFT.GetText());

                    if(SVX_NUM_CHAR_SPECIAL != nTmp)
                    {
                        sal_Int32 nOff = GetCurField()->GetPar2().toInt32();
                        if( TYP_NEXTPAGEFLD == nTypeId && 1 != nOff )
                            aValueED.SetText(
                                String::CreateFromInt32(nOff - 1) );
                        else if( TYP_PREVPAGEFLD == nTypeId && -1 != nOff )
                            aValueED.SetText(
                                String::CreateFromInt32(nOff + 1) );
                        else
                            aValueED.SetText(aEmptyStr);
                    }
                    else
                        aValueED.SetText(((SwPageNumberField*)GetCurField())->GetUserString());
                }
                bValue = sal_True;
                break;

            case TYP_CHAPTERFLD:
                aValueFT.SetText(SW_RESSTR(STR_LEVEL));
                if (IsFldEdit())
                    aLevelED.SetText(String::CreateFromInt32(((SwChapterField*)GetCurField())->GetLevel() + 1));
                bLevel = sal_True;
                break;

            case TYP_PAGENUMBERFLD:
                aValueFT.SetText( SW_RESSTR( STR_OFFSET ));
                if (IsFldEdit())
                    aValueED.SetText(GetCurField()->GetPar2());
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
                aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());

                if (aNumFormatLB.GetFormatType() == (NUMBERFORMAT_DATE|NUMBERFORMAT_TIME))
                {
                    // always set Format-Type because otherwise when date/time formats are combined,
                    // both formats would be displayed at the same time
                    aNumFormatLB.SetFormatType(0);
                    aNumFormatLB.SetFormatType(nFmtType);
                    // set correct format once again
                    aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());
                }
            }
            else
                aNumFormatLB.SetFormatType(nFmtType);

            aNumFormatLB.SetOneArea(bOneArea);
        }

        aFormatLB.Show(!bNumFmt);
        aNumFormatLB.Show(bNumFmt);

        Size aSz(aFormatLB.GetSizePixel());

        if (bValue | bLevel | bOffset)
            aSz.Height() = aFormatLB.LogicToPixel(Size(1, 137), MAP_APPFONT).Height();
        else
            aSz.Height() = aFormatLB.LogicToPixel(Size(1, 152), MAP_APPFONT).Height();

        aValueFT.Show(bValue | bLevel | bOffset);
        aValueED.Show(bValue);
        aLevelED.Show(bLevel);
        aDateOffsetED.Show(bOffset);
        aFixedCB.Show(!bValue && !bLevel && !bOffset);

        aFormatLB.SetSizePixel(aSz);
        aNumFormatLB.SetSizePixel(aSz);

        aFormatLB.Enable(bFormat);
        aFormatFT.Enable(bFormat);
        aFixedCB.Enable(bFixed);

        if (IsFldEdit())
            aFixedCB.Check( static_cast< sal_Bool >(((GetCurField()->GetFormat() & AF_FIXED) != 0) & bFixed));

        if (aNumFormatLB.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
            aNumFormatLB.SelectEntryPos(0);
        aValueFT.Enable(bValue | bLevel | bOffset);
        aValueED.Enable(bValue);
    }

    return 0;
}

void SwFldDokPage::AddSubType(sal_uInt16 nTypeId)
{
    sal_uInt16 nPos = aSelectionLB.InsertEntry(SwFieldType::GetTypeStr(nTypeId));
    aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
}

IMPL_LINK_NOARG(SwFldDokPage, SubTypeHdl)
{
    sal_uInt16 nPos = aSelectionLB.GetSelectEntryPos();
    if(nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(nPos);
    FillFormatLB(nTypeId);

    sal_uInt16 nTextRes = 0;
    switch (nTypeId)
    {
    case TYP_CHAPTERFLD:
        nTextRes = STR_LEVEL;
        break;

    case TYP_PREVPAGEFLD:
    case TYP_NEXTPAGEFLD:
        nTextRes = SVX_NUM_CHAR_SPECIAL == (sal_uInt16)(sal_uLong)aFormatLB.GetEntryData(
                                        aFormatLB.GetSelectEntryPos() )
                        ? STR_VALUE : STR_OFFSET;
        break;

    case TYP_PAGENUMBERFLD:
        nTextRes = STR_OFFSET;
        break;
    }

    if( nTextRes )
        aValueFT.SetText( SW_RESSTR( nTextRes ));

    return 0;
}

sal_uInt16 SwFldDokPage::FillFormatLB(sal_uInt16 nTypeId)
{
    // fill Format-Listbox
    aFormatLB.Clear();

    if (nTypeId == TYP_AUTHORFLD)
        return aFormatLB.GetEntryCount();

    sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, sal_False, IsFldDlgHtmlMode());

    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        sal_uInt16 nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
        sal_uInt16 nFmtId = GetFldMgr().GetFormatId( nTypeId, i );
        aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>( nFmtId ));
        if (IsFldEdit() && nFmtId == (GetCurField()->GetFormat() & ~AF_FIXED))
            aFormatLB.SelectEntryPos( nPos );
    }

    if( nSize && !aFormatLB.GetSelectEntryCount() )
    {
        aFormatLB.SelectEntry( SW_RESSTR(FMT_NUM_PAGEDESC) );
        if( !aFormatLB.GetSelectEntryCount() )
        {
            aFormatLB.SelectEntry( SW_RESSTR(FMT_NUM_ARABIC) );
            if( !aFormatLB.GetSelectEntryCount() )
                aFormatLB.SelectEntryPos( 0 );
        }
    }

    FormatHdl();

    return nSize;
}

IMPL_LINK_NOARG(SwFldDokPage, FormatHdl)
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    if (nTypeId == USHRT_MAX)
    {
        sal_uInt16 nPos = aSelectionLB.GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;

        nTypeId = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(nPos);
    }

    if (nTypeId == TYP_NEXTPAGEFLD || nTypeId == TYP_PREVPAGEFLD)
    {
        // Prev/Next - PageNumFields special treatment:
        sal_uInt16 nTmp = (sal_uInt16)(sal_uLong)aFormatLB.GetEntryData(
                                        aFormatLB.GetSelectEntryPos() );
        String sOldTxt( aValueFT.GetText() );
        String sNewTxt( SW_RES( SVX_NUM_CHAR_SPECIAL == nTmp  ? STR_VALUE
                                                         : STR_OFFSET ));

        if( sOldTxt != sNewTxt )
            aValueFT.SetText( sNewTxt );

        if (sOldTxt != aValueFT.GetText())
            aValueED.SetText( aEmptyStr );
    }

    return 0;
}

sal_Bool SwFldDokPage::FillItemSet(SfxItemSet& )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    if (nTypeId == USHRT_MAX)
    {
        sal_uInt16 nPos = aSelectionLB.GetSelectEntryPos();
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
            nPos = 0;
        nTypeId = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(nPos);
    }

    String aVal(aValueED.GetText());
    sal_uLong nFormat = 0;
    sal_uInt16 nSubType = 0;

    if (aFormatLB.IsEnabled())
    {
        sal_uInt16 nPos = aFormatLB.GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nFormat = (sal_uInt16)(sal_uLong)aFormatLB.GetEntryData(nPos);
    }

    if (aSelectionLB.IsEnabled())
    {
        sal_uInt16 nPos = aSelectionLB.GetSelectEntryPos();
        if(nPos != LISTBOX_ENTRY_NOTFOUND)
            nSubType = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(nPos);
    }

    switch (nTypeId)
    {
        case TYP_AUTHORFLD:
            nFormat = nSubType;
            nSubType = 0;
            // no break!
        case TYP_EXTUSERFLD:
            nFormat |= aFixedCB.IsChecked() ? AF_FIXED : 0;
            break;

        case TYP_FILENAMEFLD:
            nFormat |= aFixedCB.IsChecked() ? FF_FIXED : 0;
            break;

        case TYP_DATEFLD:
        case TYP_TIMEFLD:
        {
            nFormat = aNumFormatLB.GetFormat();
            long nVal = static_cast< long >(aDateOffsetED.GetValue());
            if (nTypeId == TYP_DATEFLD)
                aVal = String::CreateFromInt32(nVal * 60 * 24);
            else
                aVal = String::CreateFromInt32(nVal);
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
                sal_Int32 nVal = aValueED.GetText().ToInt32();
                aVal = String::CreateFromInt32(nVal);
            }
            break;
        }

        case TYP_CHAPTERFLD:
            aVal = aLevelED.GetText();
            break;

        default:
            break;
    }

    if (!IsFldEdit() ||
        nOldSel != aSelectionLB.GetSelectEntryPos() ||
        nOldFormat != nFormat ||
        aFixedCB.GetState() != aFixedCB.GetSavedValue() ||
        aValueED.GetText() != aValueED.GetSavedValue() ||
        aLevelED.GetText() != aLevelED.GetSavedValue() ||
        aDateOffsetED.GetText() != aDateOffsetED.GetSavedValue())
    {
        InsertFld( nTypeId, nSubType, aEmptyStr, aVal, nFormat, ' ', aNumFormatLB.IsAutomaticLanguage() );
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
    String sData(rtl::OUString(USER_DATA_VERSION));
    sData += ';';
    sal_uInt16 nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(aTypeLB.GetEntryData( nTypeSel )));
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
