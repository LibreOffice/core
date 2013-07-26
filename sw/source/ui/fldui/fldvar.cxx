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

#include <swtypes.hxx>
#include <sfx2/linkmgr.hxx>
#include <usrfld.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <ddefld.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <swmodule.hxx>
#include <fldvar.hxx>
#include <calc.hxx>
#include <svl/zformat.hxx>
#include <globals.hrc>
#include <fldui.hrc>
#include <fldtdlg.hrc>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldVarPage::SwFldVarPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage       ( pParent, SW_RES( TP_FLD_VAR ), rCoreSet ),
    aTypeFT         (this, SW_RES(FT_VARTYPE)),
    aTypeLB         (this, SW_RES(LB_VARTYPE)),
    aSelectionFT    (this, SW_RES(FT_VARSELECTION)),
    aSelectionLB    (this, SW_RES(LB_VARSELECTION)),
    aNameFT         (this, SW_RES(FT_VARNAME)),
    aNameED         (this, SW_RES(ED_VARNAME)),
    aValueFT        (this, SW_RES(FT_VARVALUE)),
    aValueED        (this, SW_RES(ED_VARVALUE)),
    aFormatFT       (this, SW_RES(FT_VARFORMAT)),
    aNumFormatLB    (this, SW_RES(LB_VARNUMFORMAT)),
    aFormatLB       (this, SW_RES(LB_VARFORMAT)),
    aChapterHeaderFT(this, SW_RES(FT_VARCHAPTERHEADER)),
    aChapterLevelFT (this, SW_RES(FT_VARCHAPTERLEVEL)),
    aChapterLevelLB (this, SW_RES(LB_VARCHAPTERLEVEL)),
    aInvisibleCB    (this, SW_RES(CB_VARINVISIBLE)),
    aSeparatorFT    (this, SW_RES(FT_VARSEPARATOR)),
    aSeparatorED    (this, SW_RES(ED_VARSEPARATOR)),
    aNewDelTBX      (this, SW_RES(TBX_VARNEWDEL)),
    bInit           (sal_True)
{
    FreeResource();

    aNewDelTBX.SetSizePixel( aNewDelTBX.CalcWindowSizePixel() );

    long nDelta = ( aValueED.GetSizePixel().Height() -
                    aNewDelTBX.GetSizePixel().Height() ) / 2;
    Point aNewPnt = aNewDelTBX.GetPosPixel();
    aNewPnt.Y() += nDelta;
    aNewDelTBX.SetPosPixel( aNewPnt );

    sOldValueFT = aValueFT.GetText();
    sOldNameFT = aNameFT.GetText();

    for (sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        aChapterLevelLB.InsertEntry(OUString::number(i));

    aChapterLevelLB.SelectEntryPos(0);
    //enable 'active' language selection
    aNumFormatLB.SetShowLanguageControl(sal_True);
}

SwFldVarPage::~SwFldVarPage()
{
}

void SwFldVarPage::Reset(const SfxItemSet& )
{
    SavePos(&aTypeLB);

    Init(); // general initialisation

    aTypeLB.SetUpdateMode(sal_False);
    aTypeLB.Clear();

    sal_uInt16 nPos, nTypeId;

    if (!IsFldEdit())
    {
        // initialise TypeListBox
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for (short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
            aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        SwField* pCurField = GetCurField();
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_SETINPFLD)
            nTypeId = TYP_INPUTFLD;
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

    aTypeLB.SetDoubleClickHdl       (LINK(this, SwFldVarPage, InsertHdl));
    aTypeLB.SetSelectHdl            (LINK(this, SwFldVarPage, TypeHdl));
    aSelectionLB.SetSelectHdl       (LINK(this, SwFldVarPage, SubTypeHdl));
    aSelectionLB.SetDoubleClickHdl  (LINK(this, SwFldVarPage, InsertHdl));
    aFormatLB.SetDoubleClickHdl     (LINK(this, SwFldVarPage, InsertHdl));
    aNumFormatLB.SetDoubleClickHdl  (LINK(this, SwFldVarPage, InsertHdl));
    aNameED.SetModifyHdl            (LINK(this, SwFldVarPage, ModifyHdl));
    aValueED.SetModifyHdl           (LINK(this, SwFldVarPage, ModifyHdl));
    aNewDelTBX.SetClickHdl          (LINK(this, SwFldVarPage, TBClickHdl));
    aChapterLevelLB.SetSelectHdl    (LINK(this, SwFldVarPage, ChapterHdl));
    aSeparatorED.SetModifyHdl       (LINK(this, SwFldVarPage, SeparatorHdl));

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(!IsRefresh() && sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            sal_uInt16 nVal = (sal_uInt16)sVal.ToInt32();
            if( USHRT_MAX != nVal )
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

    aTypeLB.SetUpdateMode(sal_True);

    if (IsFldEdit())
    {
        aSelectionLB.SaveValue();
        aFormatLB.SaveValue();
        nOldFormat = aNumFormatLB.GetFormat();
        aNameED.SaveValue();
        aValueED.SaveValue();
        aInvisibleCB.SaveValue();
        aChapterLevelLB.SaveValue();
        aSeparatorED.SaveValue();
    }
}

IMPL_LINK_NOARG(SwFldVarPage, TypeHdl)
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

    if (nOld != GetTypeSel() || nOld == LISTBOX_ENTRY_NOTFOUND)
    {
        bInit = sal_True;
        if (nOld != LISTBOX_ENTRY_NOTFOUND)
        {
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
        }

        aValueED.SetDropEnable(false);
        UpdateSubType();    // initialise selection-listboxes
    }

    bInit = sal_False;

    return 0;
}

IMPL_LINK( SwFldVarPage, SubTypeHdl, ListBox *, pBox )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());
    sal_uInt16 nSelPos = aSelectionLB.GetSelectEntryPos();

    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
        nSelPos = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(nSelPos);

    if (IsFldEdit() && (!pBox || bInit))
    {
        if (nTypeId != TYP_FORMELFLD)
            aNameED.SetText(GetFldMgr().GetCurFldPar1());

        aValueED.SetText(GetFldMgr().GetCurFldPar2());
    }

    if (aNameFT.GetText() != OUString(sOldNameFT))
        aNameFT.SetText(sOldNameFT);
    if (aValueFT.GetText() != OUString(sOldValueFT))
        aValueFT.SetText(sOldValueFT);

    aNumFormatLB.SetUpdateMode(sal_False);
    aFormatLB.SetUpdateMode(sal_False);
    FillFormatLB(nTypeId);

    sal_uInt16 nSize = aFormatLB.GetEntryCount();

    sal_Bool bValue = sal_False, bName = sal_False, bNumFmt = sal_False,
            bInvisible = sal_False, bSeparator = sal_False, bChapterLevel = sal_False;
    sal_Bool bFormat = nSize != 0;

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            // change or create user type
            SwUserFieldType* pType = (SwUserFieldType*)
                GetFldMgr().GetFldType(RES_USERFLD, nSelPos);

            if (pType)
            {
                if (!IsFldEdit())
                {
                    if (pBox || (bInit && !IsRefresh()))    // only when interacting via mouse
                    {
                        aNameED.SetText(pType->GetName());

                        if (pType->GetType() == UF_STRING)
                        {
                            aValueED.SetText(pType->GetContent());
                            aNumFormatLB.SelectEntryPos(0);
                        }
                        else
                            aValueED.SetText(pType->GetContent());
                    }
                }
                else
                    aValueED.SetText(pType->GetContent());
            }
            else
            {
                if (pBox)   // only when interacting via mouse
                {
                    aNameED.SetText(aEmptyStr);
                    aValueED.SetText(aEmptyStr);
                }
            }
            bValue = bName = bNumFmt = bInvisible = sal_True;

            aValueED.SetDropEnable(true);
            break;
        }

        case TYP_SETFLD:
            bValue = sal_True;

            bNumFmt = bInvisible = sal_True;

            if (!IsFldDlgHtmlMode())
                bName = sal_True;
            else
            {
                aNumFormatLB.Clear();
                sal_uInt16 nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
                aNumFormatLB.SelectEntryPos(0);
            }
            // is there a corresponding SetField
            if (IsFldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    String sName(aSelectionLB.GetSelectEntry());
                    aNameED.SetText(sName);

                    if (!IsFldDlgHtmlMode())
                    {
                        SwWrtShell *pSh = GetWrtShell();
                        if(!pSh)
                            pSh = ::GetActiveWrtShell();
                        if(pSh)
                        {
                            SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                    pSh->GetFldType(RES_SETEXPFLD, sName);

                            if (pSetTyp && pSetTyp->GetType() == nsSwGetSetExpType::GSE_STRING)
                                aNumFormatLB.SelectEntryPos(0); // textual
                        }
                    }
                }
            }
            if (IsFldEdit())
            {
                // GetFormula leads to problems with date formats because
                // only the numeric value without formating is returned.
                // It must be used though because otherwise in GetPar2 only
                // the value calculated by Kalkulator would be displayed
                // (instead of test2 = test + 1)
                aValueED.SetText(((SwSetExpField*)GetCurField())->GetFormula());
            }
            aValueED.SetDropEnable(true);
            break;

        case TYP_FORMELFLD:
            {
                bValue = sal_True;
                bNumFmt = sal_True;
                aValueFT.SetText(SW_RESSTR(STR_FORMULA));
                aValueED.SetDropEnable(true);
            }
            break;

        case TYP_GETFLD:
            {
                if (!IsFldEdit())
                {
                    aNameED.SetText(aEmptyStr);
                    aValueED.SetText(aEmptyStr);
                }

                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    String sName(aSelectionLB.GetSelectEntry());
                    if (!IsFldEdit())
                        aNameED.SetText(sName);

                    // is there a corresponding SetField
                    SwWrtShell *pSh = GetWrtShell();
                    if(!pSh)
                        pSh = ::GetActiveWrtShell();
                    if(pSh)
                    {
                        SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                pSh->GetFldType(RES_SETEXPFLD, sName);

                        if(pSetTyp)
                        {
                            if (pSetTyp->GetType() & nsSwGetSetExpType::GSE_STRING)    // textual?
                                bFormat = sal_True;
                            else                    // numeric
                                bNumFmt = sal_True;
                        }
                    }
                }
                else
                    bFormat = sal_False;

                EnableInsert(bFormat|bNumFmt);
            }
            break;

        case TYP_INPUTFLD:
            aValueFT.SetText(SW_RESSTR(STR_PROMPT));

            if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
            {
                bValue = bNumFmt = sal_True;

                String sName;

                sName = aSelectionLB.GetSelectEntry();
                aNameED.SetText( sName );

                // User- or SetField ?
                sal_uInt16 nInpType = 0;
                nInpType = static_cast< sal_uInt16 >(GetFldMgr().GetFldType(RES_USERFLD, sName) ? 0 : TYP_SETINPFLD);

                if (nInpType)   // SETEXPFLD
                {
                    // is there a corresponding SetField
                    SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                GetFldMgr().GetFldType(RES_SETEXPFLD, sName);

                    if(pSetTyp)
                    {
                        if (pSetTyp->GetType() == nsSwGetSetExpType::GSE_STRING)    // textual?
                        {
                            aNumFormatLB.Clear();

                            sal_uInt16 nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_USERVAR_TEXT), 0);
                            aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
                            aNumFormatLB.SelectEntryPos(0);
                        }
                    }
                    if (IsFldEdit() && (!pBox || bInit) )
                        aValueED.SetText(((SwSetExpField*)GetCurField())->GetPromptText());
                }
                else    // USERFLD
                    bFormat = bNumFmt = sal_False;
            }
            break;

        case TYP_DDEFLD:
            aValueFT.SetText(SW_RESSTR(STR_DDE_CMD));

            if (IsFldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    SwDDEFieldType* pType =
                        (SwDDEFieldType*) GetFldMgr().GetFldType(RES_DDEFLD, nSelPos);

                    if(pType)
                    {
                        aNameED.SetText(pType->GetName());

                        //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
                        //              That's not considered here yet
                        String sCmd( pType->GetCmd() );
                        sal_uInt16 nTmpPos = sCmd.SearchAndReplace( sfx2::cTokenSeparator, ' ' );
                        sCmd.SearchAndReplace( sfx2::cTokenSeparator, ' ', nTmpPos );

                        aValueED.SetText( sCmd );
                        aFormatLB.SelectEntryPos(pType->GetType());
                    }
                }
            }
            bName = bValue = sal_True;
            break;

        case TYP_SEQFLD:
            {
                bName = bValue = bSeparator = bChapterLevel = sal_True;

                SwFieldType* pFldTyp;
                if( IsFldEdit() )
                    pFldTyp = GetCurField()->GetTyp();
                else
                {
                    String sFldTypeName( aSelectionLB.GetEntry( nSelPos ));
                    if( sFldTypeName.Len() )
                        pFldTyp = GetFldMgr().GetFldType( RES_SETEXPFLD,
                                                          sFldTypeName );
                    else
                        pFldTyp = 0;
                }

                if( IsFldEdit() )
                    aValueED.SetText( ((SwSetExpField*)GetCurField())->
                                        GetFormula() );

                if( IsFldEdit() || pBox )   // only when interacting via mouse
                    aNameED.SetText( aSelectionLB.GetSelectEntry() );

                if( pFldTyp )
                {
                    sal_uInt8 nLevel = ((SwSetExpFieldType*)pFldTyp)->GetOutlineLvl();
                    if( 0x7f == nLevel )
                        aChapterLevelLB.SelectEntryPos( 0 );
                    else
                        aChapterLevelLB.SelectEntryPos( nLevel + 1 );
                    String sDelim = ((SwSetExpFieldType*)pFldTyp)->GetDelimiter();
                    aSeparatorED.SetText( sDelim );
                    ChapterHdl();
                }
            }
            break;

        case TYP_SETREFPAGEFLD:
            {
                bValue = sal_False;
                aValueFT.SetText( SW_RESSTR( STR_OFFSET ));

                if (IsFldEdit() || pBox)    // only when interacting via mouse
                    aNameED.SetText(aEmptyStr);

                if (nSelPos != 0 && nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    bValue = sal_True;      // SubType OFF - knows no Offset
                    if (IsFldEdit())
                        aValueED.SetText(OUString::number(((SwRefPageSetField*)GetCurField())->GetOffset()));
                }
            }
            break;

        case TYP_GETREFPAGEFLD:
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
            break;
    }

    aNumFormatLB.Show(bNumFmt);
    aFormatLB.Show(!bNumFmt);

    if (IsFldEdit())
        bName = sal_False;

    aFormatLB.Enable(bFormat);
    aFormatFT.Enable(bFormat|bNumFmt);
    aNameFT.Enable(bName);
    aNameED.Enable(bName);
    aValueFT.Enable(bValue);
    aValueED.Enable(bValue);

    Size aSz(aFormatLB.GetSizePixel());

    if (bChapterLevel)
        aSz.Height() = aFormatLB.LogicToPixel(Size(1, 94), MAP_APPFONT).Height();
    else
        aSz.Height() = aFormatLB.LogicToPixel(Size(1, 123), MAP_APPFONT).Height();

    aFormatLB.SetSizePixel(aSz);

    aInvisibleCB.Show(!bSeparator);
    aSeparatorFT.Show(bSeparator);
    aSeparatorED.Show(bSeparator);
    aChapterHeaderFT.Show(bChapterLevel);
    aChapterLevelFT.Show(bChapterLevel);
    aChapterLevelLB.Show(bChapterLevel);
    aInvisibleCB.Enable(bInvisible);

    ModifyHdl();    // apply/insert/delete status update

    aNumFormatLB.SetUpdateMode(sal_True);
    aFormatLB.SetUpdateMode(sal_True);

    if(aSelectionLB.IsCallAddSelection())
    {
        nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

        switch (nTypeId)
        {
            case TYP_FORMELFLD:
                {
                    nSelPos = aSelectionLB.GetSelectEntryPos();

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                        nSelPos = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(nSelPos);

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND && pBox && !bInit)
                    {
                        aValueED.ReplaceSelected(aSelectionLB.GetSelectEntry());
                        ModifyHdl();
                    }
                }
                break;
        }
        aSelectionLB.ResetCallAddSelection();
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description: renew types in SelectionBox
 --------------------------------------------------------------------*/
void SwFldVarPage::UpdateSubType()
{
    String sOldSel;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    SetSelectionSel(aSelectionLB.GetSelectEntryPos());
    if(GetSelectionSel() != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = aSelectionLB.GetEntry(GetSelectionSel());

    // fill Selection-Listbox
    aSelectionLB.SetUpdateMode(sal_False);
    aSelectionLB.Clear();

    std::vector<OUString> aList;
    GetFldMgr().GetSubTypes(nTypeId, aList);
    size_t nCount = aList.size();
    size_t nPos;

    for(size_t i = 0; i < nCount; ++i)
    {
        if (nTypeId != TYP_INPUTFLD || i)
        {
            if (!IsFldEdit())
            {
                nPos = aSelectionLB.InsertEntry(aList[i]);
                aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
            }
            else
            {
                sal_Bool bInsert = sal_False;

                switch (nTypeId)
                {
                    case TYP_INPUTFLD:
                        if (aList[i] == GetCurField()->GetPar1())
                            bInsert = sal_True;
                        break;

                    case TYP_FORMELFLD:
                        bInsert = sal_True;
                        break;

                    case TYP_GETFLD:
                        if (aList[i].equals(((const SwFormulaField*)GetCurField())->GetFormula()))
                            bInsert = sal_True;
                        break;

                    case TYP_SETFLD:
                    case TYP_USERFLD:
                        if (aList[i] == GetCurField()->GetTyp()->GetName())
                        {
                            bInsert = sal_True;
                            if (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE)
                                aInvisibleCB.Check();
                        }
                        break;

                    case TYP_SETREFPAGEFLD:
                        if ((((SwRefPageSetField*)GetCurField())->IsOn() && i) ||
                            (!((SwRefPageSetField*)GetCurField())->IsOn() && !i))
                            sOldSel = aList[i];

                        // allow all entries for selection:
                        nPos = aSelectionLB.InsertEntry(aList[i]);
                        aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
                        break;

                    default:
                        if (aList[i] == GetCurField()->GetPar1())
                            bInsert = sal_True;
                        break;
                }
                if (bInsert)
                {
                    nPos = aSelectionLB.InsertEntry(aList[i]);
                    aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
                    if (nTypeId != TYP_FORMELFLD)
                        break;
                }
            }
        }
    }

    sal_Bool bEnable = aSelectionLB.GetEntryCount() != 0;
    ListBox *pLB = 0;

    if (bEnable)
    {
        aSelectionLB.SelectEntry(sOldSel);
        if (!aSelectionLB.GetSelectEntryCount())
        {
            aSelectionLB.SelectEntryPos(0);
            pLB = &aSelectionLB;    // newly initialise all controls
        }
    }

    aSelectionLB.Enable( bEnable );
    aSelectionFT.Enable( bEnable );

    SubTypeHdl(pLB);
    aSelectionLB.SetUpdateMode(sal_True);
}

sal_uInt16 SwFldVarPage::FillFormatLB(sal_uInt16 nTypeId)
{
    String sOldSel, sOldNumSel;
    sal_uLong nOldNumFormat = 0;

    sal_uInt16 nFormatSel = aFormatLB.GetSelectEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = aFormatLB.GetEntry(nFormatSel);

    sal_uInt16 nNumFormatSel = aNumFormatLB.GetSelectEntryPos();
    if (nNumFormatSel != LISTBOX_ENTRY_NOTFOUND)
    {
        sOldNumSel = aNumFormatLB.GetEntry(nNumFormatSel);
        nOldNumFormat = aNumFormatLB.GetFormat();
    }

    // fill Format-Listbox
    aFormatLB.Clear();
    aNumFormatLB.Clear();
    sal_Bool bSpecialFmt = sal_False;

    if( TYP_GETREFPAGEFLD != nTypeId )
    {
        if (IsFldEdit())
        {
            bSpecialFmt = GetCurField()->GetFormat() == SAL_MAX_UINT32;

            if (!bSpecialFmt)
            {
                aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());
                sOldNumSel = aEmptyStr;
            }
            else
                if (nTypeId == TYP_GETFLD || nTypeId == TYP_FORMELFLD)
                    aNumFormatLB.SetFormatType(NUMBERFORMAT_NUMBER);
        }
        else
        {
            if (nOldNumFormat && nOldNumFormat != ULONG_MAX)
                aNumFormatLB.SetDefFormat(nOldNumFormat);
            else
                aNumFormatLB.SetFormatType(NUMBERFORMAT_NUMBER);
        }
    }

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            if (!IsFldEdit() || bSpecialFmt)
            {
                sal_uInt16 nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_MARK_TEXT), 0);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
                nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_USERVAR_CMD), 1);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
            }
        }
        break;

        case TYP_SETFLD:
        {
            if (!IsFldEdit() || bSpecialFmt)
            {
                sal_uInt16 nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
            }
        }
        break;

        case TYP_FORMELFLD:
        {
            sal_uInt16 nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
            aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
        }
        break;

        case TYP_GETFLD:
        {
            sal_uInt16 nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
            aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
        }
        break;
    }

    if (IsFldEdit() && bSpecialFmt)
    {
        if (nTypeId == TYP_USERFLD && (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_CMD))
            aNumFormatLB.SelectEntryPos(1);
        else
            aNumFormatLB.SelectEntryPos(0);
    }
    else
    {
        if (!nOldNumFormat && (nNumFormatSel = aNumFormatLB.GetEntryPos(sOldNumSel)) != LISTBOX_ENTRY_NOTFOUND)
            aNumFormatLB.SelectEntryPos(nNumFormatSel);
        else if (nOldNumFormat && nOldNumFormat == ULONG_MAX)
            aNumFormatLB.SelectEntry(sOldSel);
    }

    sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        sal_uInt16 nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
        sal_uInt16 nFldId = GetFldMgr().GetFormatId( nTypeId, i );
        aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>(nFldId) );
        if (IsFldEdit() && nFldId == GetCurField()->GetFormat())
            aFormatLB.SelectEntryPos( nPos );
    }

    if (nSize && (!IsFldEdit() || !aFormatLB.GetSelectEntryCount()))
    {
        aFormatLB.SelectEntry(sOldSel);

        if (!aFormatLB.GetSelectEntryCount())
        {
            aFormatLB.SelectEntry(SW_RESSTR(FMT_NUM_PAGEDESC));
            if (!aFormatLB.GetSelectEntryCount())
            {
                aFormatLB.SelectEntry(SW_RESSTR(FMT_NUM_ARABIC));
                if (!aFormatLB.GetSelectEntryCount())
                    aFormatLB.SelectEntryPos(0);
            }
        }
    }

    return nSize;
}

/*--------------------------------------------------------------------
    Description: Modify
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwFldVarPage, ModifyHdl)
{
    String sValue(aValueED.GetText());
    sal_Bool bHasValue = sValue.Len() != 0;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());
    bool bInsert = false, bApply = false, bDelete = false;

    String sName( aNameED.GetText() );
    xub_StrLen nLen = sName.Len();

    switch( nTypeId )
    {
    case TYP_DDEFLD:
    case TYP_USERFLD:
    case TYP_SETFLD:
    case TYP_SEQFLD:
        SwCalc::IsValidVarName( sName, &sName );
        if( sName.Len() != nLen )
        {
            nLen = sName.Len();
            Selection aSel(aNameED.GetSelection());
            aNameED.SetText( sName );
            aNameED.SetSelection( aSel );   // restore Cursorpos
        }
        break;
    }


    // check buttons
    switch (nTypeId)
    {
    case TYP_DDEFLD:
        if( nLen )
        {
            // is there already a corrensponding type
            bInsert = bApply = true;

            SwFieldType* pType = GetFldMgr().GetFldType(RES_DDEFLD, sName);

            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh && pType)
                bDelete = !pSh->IsUsed( *pType );
        }
        break;

    case TYP_USERFLD:
        if( nLen )
        {
            // is there already a corresponding type
            SwFieldType* pType = GetFldMgr().GetFldType(RES_USERFLD, sName);

            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh && pType)
                bDelete = !pSh->IsUsed( *pType );

            pType = GetFldMgr().GetFldType(RES_SETEXPFLD, sName);
            if (!pType) // no name conflict with variables
            {
                // user fields can also be inserted without content!
                // Bug #56845
                bInsert = bApply = true;
            }
        }
        break;

    default:
        bInsert = true;

        if (nTypeId == TYP_SETFLD || nTypeId == TYP_SEQFLD)
        {
            SwSetExpFieldType* pFldType = (SwSetExpFieldType*)
                GetFldMgr().GetFldType(RES_SETEXPFLD, sName);

            if (pFldType)
            {

                SwWrtShell *pSh = GetWrtShell();
                if(!pSh)
                    pSh = ::GetActiveWrtShell();
                if(pSh)
                {
                    const SwFldTypes* p = pSh->GetDoc()->GetFldTypes();
                    sal_uInt16 i;

                    for (i = 0; i < INIT_FLDTYPES; i++)
                    {
                        SwFieldType* pType = (*p)[ i ];
                        if (pType == pFldType)
                            break;
                    }

                    if (i >= INIT_FLDTYPES && !pSh->IsUsed(*pFldType))
                        bDelete = true;

                    if (nTypeId == TYP_SEQFLD && !(pFldType->GetType() & nsSwGetSetExpType::GSE_SEQ))
                        bInsert = false;

                    if (nTypeId == TYP_SETFLD && (pFldType->GetType() & nsSwGetSetExpType::GSE_SEQ))
                        bInsert = false;
                }
            }
            if (GetFldMgr().GetFldType(RES_USERFLD, sName))
                bInsert = false;
        }

        if( !nLen && ( nTypeId == TYP_SETFLD ||
                        (!IsFldEdit() && nTypeId == TYP_GETFLD ) ) )
            bInsert = false;

        if( (nTypeId == TYP_SETFLD || nTypeId == TYP_FORMELFLD) &&
            !bHasValue )
            bInsert = false;
        break;
    }

    aNewDelTBX.EnableItem(BT_VARAPPLY, bApply);
    aNewDelTBX.EnableItem(BT_VARDELETE, bDelete);
    EnableInsert(bInsert);

    return 0;
}

IMPL_LINK( SwFldVarPage, TBClickHdl, ToolBox *, pBox )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    switch (pBox->GetCurItemId())
    {
        case BT_VARDELETE:
        {
            if( nTypeId == TYP_USERFLD )
                GetFldMgr().RemoveFldType(RES_USERFLD, aSelectionLB.GetSelectEntry());
            else
            {
                sal_uInt16 nWhich;

                switch(nTypeId)
                {
                    case TYP_SETFLD:
                    case TYP_SEQFLD:
                        nWhich = RES_SETEXPFLD;
                        break;
                    default:
                        nWhich = RES_DDEFLD;
                        break;
                }

                GetFldMgr().RemoveFldType(nWhich, aSelectionLB.GetSelectEntry());
            }

            UpdateSubType();
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                pSh->SetModified();
            }
        }
        break;

        case BT_VARAPPLY:
        {
            String sName(aNameED.GetText()), sValue(aValueED.GetText());
            SwFieldType* pType = 0;
            sal_uInt16 nId = 0;
            sal_uInt16 nNumFormatPos = aNumFormatLB.GetSelectEntryPos();

            switch (nTypeId)
            {
                case TYP_USERFLD:   nId = RES_USERFLD;  break;
                case TYP_DDEFLD:    nId = RES_DDEFLD;   break;
                case TYP_SETFLD:    nId = RES_SETEXPFLD;break;
            }
            pType = GetFldMgr().GetFldType(nId, sName);

            sal_uLong nFormat = aFormatLB.GetSelectEntryPos();
            if (nFormat != LISTBOX_ENTRY_NOTFOUND)
                nFormat = (sal_uLong)aFormatLB.GetEntryData((sal_uInt16)nFormat);

            if (pType)  // change
            {
                SwWrtShell *pSh = GetWrtShell();
                if(!pSh)
                    pSh = ::GetActiveWrtShell();
                if(pSh)
                {
                    pSh->StartAllAction();

                    if (nTypeId == TYP_USERFLD)
                    {
                        if (nNumFormatPos != LISTBOX_ENTRY_NOTFOUND)
                        {
                            sal_uLong nFmt = nNumFormatPos == 0 ? 0 : aNumFormatLB.GetFormat();
                            if (nFmt)
                            {   // Switch language to office-language because Kalkulator expects
                                // String in office format and it should be fed into dialog like
                                // that
                                nFmt = SwValueField::GetSystemFormat(pSh->GetNumberFormatter(), nFmt);
                            }
                            ((SwUserFieldType*)pType)->SetContent(aValueED.GetText(), nFmt);
                            ((SwUserFieldType*)pType)->SetType(
                                nNumFormatPos == 0 ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR );
                        }
                    }
                    else
                    {
                        if (nFormat != LISTBOX_ENTRY_NOTFOUND)
                        {
                            // DDE-Topics/-Items can have blanks in their names!
                            //  That's not being considered here yet.
                            sal_uInt16 nTmpPos = sValue.SearchAndReplace( ' ', sfx2::cTokenSeparator );
                            sValue.SearchAndReplace( ' ', sfx2::cTokenSeparator, nTmpPos );
                            ((SwDDEFieldType*)pType)->SetCmd(sValue);
                            ((SwDDEFieldType*)pType)->SetType((sal_uInt16)nFormat);
                        }
                    }
                    pType->UpdateFlds();

                    pSh->EndAllAction();
                }
            }
            else        // new
            {
                if(nTypeId == TYP_USERFLD)
                {
                    SwWrtShell *pSh = GetWrtShell();
                    if(!pSh)
                        pSh = ::GetActiveWrtShell();
                    if(pSh)
                    {
                        SwUserFieldType aType( pSh->GetDoc(), sName );

                        if (nNumFormatPos != LISTBOX_ENTRY_NOTFOUND)
                        {
                            aType.SetType(nNumFormatPos == 0 ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR);
                            aType.SetContent( sValue, nNumFormatPos == 0 ? 0 : aNumFormatLB.GetFormat() );
                            aSelectionLB.InsertEntry(sName);
                            aSelectionLB.SelectEntry(sName);
                            GetFldMgr().InsertFldType( aType ); // Userfld new
                        }
                    }
                }
                else
                {
                    if (nFormat != LISTBOX_ENTRY_NOTFOUND)
                    {
                        // DDE-Topics/-Items can have blanks in their names!
                        //  That's not being considered here yet.
                        sal_uInt16 nTmpPos = sValue.SearchAndReplace( ' ', sfx2::cTokenSeparator );
                        sValue.SearchAndReplace( ' ', sfx2::cTokenSeparator, nTmpPos );

                        SwDDEFieldType aType(sName, sValue, (sal_uInt16)nFormat);
                        aSelectionLB.InsertEntry(sName);
                        aSelectionLB.SelectEntry(sName);
                        GetFldMgr().InsertFldType(aType);   // DDE-Field new
                    }
                }
            }
            if (IsFldEdit())
                GetFldMgr().GetCurFld();    // update FieldManager

            UpdateSubType();
        }
        break;
    }

    return sal_True;
}

IMPL_LINK_NOARG(SwFldVarPage, ChapterHdl)
{
    sal_Bool bEnable = aChapterLevelLB.GetSelectEntryPos() != 0;

    aSeparatorED.Enable(bEnable);
    aSeparatorFT.Enable(bEnable);
    SeparatorHdl();

    return 0;
}

IMPL_LINK_NOARG(SwFldVarPage, SeparatorHdl)
{
    sal_Bool bEnable = !aSeparatorED.GetText().isEmpty() ||
                    aChapterLevelLB.GetSelectEntryPos() == 0;
    EnableInsert(bEnable);

    return 0;
}

sal_Bool SwFldVarPage::FillItemSet(SfxItemSet& )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    String aVal(aValueED.GetText());
    String aName(aNameED.GetText());

    sal_uInt16 nSubType = aSelectionLB.GetSelectEntryPos();
    if(nSubType == LISTBOX_ENTRY_NOTFOUND)
        nSubType = 0;
    else
        nSubType = (sal_uInt16)(sal_uLong)aSelectionLB.GetEntryData(nSubType);

    sal_uLong nFormat;

    if (!aNumFormatLB.IsVisible())
    {
        nFormat = aFormatLB.GetSelectEntryPos();

        if(nFormat == LISTBOX_ENTRY_NOTFOUND)
            nFormat = 0;
        else
            nFormat = (sal_uLong)aFormatLB.GetEntryData((sal_uInt16)nFormat);
    }
    else
    {
        nFormat = aNumFormatLB.GetFormat();

        if (nFormat && nFormat != ULONG_MAX && aNumFormatLB.IsAutomaticLanguage())
        {
            // Switch language to office language because Kalkulator expects
            // String in office format and it should be fed into the dialog
            // like that
            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh)
            {
                nFormat = SwValueField::GetSystemFormat(pSh->GetNumberFormatter(), nFormat);
            }
        }
    }
    sal_Unicode cSeparator = ' ';
    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            nSubType = (nFormat == ULONG_MAX) ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR;

            if (nFormat == ULONG_MAX && aNumFormatLB.GetSelectEntry() == SW_RESSTR(FMT_USERVAR_CMD))
                nSubType |= nsSwExtendedSubType::SUB_CMD;

            if (aInvisibleCB.IsChecked())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_FORMELFLD:
        {
            nSubType = nsSwGetSetExpType::GSE_FORMULA;
            if (aNumFormatLB.IsVisible() && nFormat == ULONG_MAX)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            break;
        }
        case TYP_GETFLD:
        {
            nSubType &= 0xff00;
            if (aNumFormatLB.IsVisible() && nFormat == ULONG_MAX)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            break;
        }
        case TYP_INPUTFLD:
        {
            SwFieldType* pType = GetFldMgr().GetFldType(RES_USERFLD, aName);
            nSubType = static_cast< sal_uInt16 >((nSubType & 0xff00) | ((pType) ? INP_USR : INP_VAR));
            break;
        }

        case TYP_SETFLD:
        {
            if (IsFldDlgHtmlMode())
            {
                nSubType = 0x0100;
                nSubType = (nSubType & 0xff00) | nsSwGetSetExpType::GSE_STRING;
            }
            else
                nSubType = (nSubType & 0xff00) | ((nFormat == ULONG_MAX) ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR);

            if (aInvisibleCB.IsChecked())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_SEQFLD:
        {
            nSubType = aChapterLevelLB.GetSelectEntryPos();
            if (nSubType == 0)
                nSubType = 0x7f;
            else
            {
                nSubType--;
                OUString sSeparator = OUString(aSeparatorED.GetText()[0]);
                cSeparator = !sSeparator.isEmpty() ? sSeparator[0] : ' ';
            }
            break;
        }
        case TYP_GETREFPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
                aVal = aValueED.GetText();
            break;
    }

    if (!IsFldEdit() ||
        aNameED.GetSavedValue() != aNameED.GetText() ||
        aValueED.GetSavedValue() != aValueED.GetText() ||
        aSelectionLB.GetSavedValue() != aSelectionLB.GetSelectEntryPos() ||
        aFormatLB.GetSavedValue() != aFormatLB.GetSelectEntryPos() ||
        nOldFormat != aNumFormatLB.GetFormat() ||
        aInvisibleCB.GetState() != aInvisibleCB.GetSavedValue() ||
        aChapterLevelLB.GetSavedValue() != aChapterLevelLB.GetSelectEntryPos() ||
        aSeparatorED.GetSavedValue() != aSeparatorED.GetText())
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat,
                    cSeparator, aNumFormatLB.IsAutomaticLanguage() );
    }

    UpdateSubType();

    return sal_False;
}

SfxTabPage* SwFldVarPage::Create(   Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldVarPage( pParent, rAttrSet ) );
}

sal_uInt16 SwFldVarPage::GetGroup()
{
    return GRP_VAR;
}

SelectionListBox::SelectionListBox( SwFldVarPage* pDialog, const ResId& rResId ) :
    ListBox (pDialog, rResId),
    bCallAddSelection(false)
{
}

long SelectionListBox::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = ListBox::PreNotify( rNEvt );
    if ( rNEvt.GetType() == EVENT_KEYUP )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();
        const sal_uInt16 nModifier = aKeyCode.GetModifier();
        if( aKeyCode.GetCode() == KEY_SPACE && !nModifier)
            bCallAddSelection = true;
    }
    if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        const MouseEvent* pMEvt = rNEvt.GetMouseEvent();

        if (pMEvt && (pMEvt->IsMod1() || pMEvt->IsMod2()))  // Alt or Ctrl
            bCallAddSelection = true;
    }

    return nHandled;
}

void SwFldVarPage::FillUserData()
{
    String sData(OUString(USER_DATA_VERSION));
    sData += ';';
    sal_uInt16 nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData( nTypeSel );
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
