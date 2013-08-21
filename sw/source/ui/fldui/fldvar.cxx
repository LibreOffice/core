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

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFldVarPage::SwFldVarPage(Window* pParent, const SfxItemSet& rCoreSet )
    : SwFldPage(pParent, "FldVarPage",
        "modules/swriter/ui/fldvarpage.ui", rCoreSet)
    , bInit(true)
{
    get(m_pTypeLB, "type");
    get(m_pSelection, "selectframe");
    get(m_pSelectionLB, "select");
    get(m_pFormat, "formatframe");
    get(m_pChapterFrame, "chapterframe");
    get(m_pNameFT, "nameft");
    get(m_pNameED, "name");
    get(m_pValueFT, "valueft");
    get(m_pValueED, "value");
    get(m_pNumFormatLB, "numformat");
    get(m_pFormatLB, "format");
    get(m_pChapterLevelLB, "level");
    get(m_pInvisibleCB, "invisible");
    get(m_pSeparatorFT, "separatorft");
    get(m_pSeparatorED, "separator");
    get(m_pNewDelTBX, "toolbar");

    long nHeight = m_pTypeLB->GetTextHeight() * 20;
    m_pTypeLB->set_height_request(nHeight);
    m_pSelectionLB->set_height_request(nHeight);
    m_pFormatLB->set_height_request(nHeight/2);

    long nWidth = m_pTypeLB->LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MAP_APPFONT)).Width();
    m_pTypeLB->set_width_request(nWidth);
    m_pSelectionLB->set_width_request(nWidth);
    m_pFormatLB->set_width_request(nWidth);

    m_nApplyId = m_pNewDelTBX->GetItemId("apply");
    m_nDeleteId = m_pNewDelTBX->GetItemId("delete");

    sOldValueFT = m_pValueFT->GetText();
    sOldNameFT = m_pNameFT->GetText();

    for (sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        m_pChapterLevelLB->InsertEntry(OUString::number(i));

    m_pChapterLevelLB->SelectEntryPos(0);
    //enable 'active' language selection
    m_pNumFormatLB->SetShowLanguageControl(sal_True);
}

SwFldVarPage::~SwFldVarPage()
{
}

void SwFldVarPage::Reset(const SfxItemSet& )
{
    SavePos(m_pTypeLB);

    Init(); // general initialisation

    m_pTypeLB->SetUpdateMode(sal_False);
    m_pTypeLB->Clear();

    sal_uInt16 nPos, nTypeId;

    if (!IsFldEdit())
    {
        // initialise TypeListBox
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for (short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = m_pTypeLB->InsertEntry(GetFldMgr().GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        SwField* pCurField = GetCurField();
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_SETINPFLD)
            nTypeId = TYP_INPUTFLD;
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

    m_pTypeLB->SetDoubleClickHdl       (LINK(this, SwFldVarPage, InsertHdl));
    m_pTypeLB->SetSelectHdl            (LINK(this, SwFldVarPage, TypeHdl));
    m_pSelectionLB->SetSelectHdl       (LINK(this, SwFldVarPage, SubTypeHdl));
    m_pSelectionLB->SetDoubleClickHdl  (LINK(this, SwFldVarPage, InsertHdl));
    m_pFormatLB->SetDoubleClickHdl     (LINK(this, SwFldVarPage, InsertHdl));
    m_pNumFormatLB->SetDoubleClickHdl  (LINK(this, SwFldVarPage, InsertHdl));
    m_pNameED->SetModifyHdl            (LINK(this, SwFldVarPage, ModifyHdl));
    m_pValueED->SetModifyHdl           (LINK(this, SwFldVarPage, ModifyHdl));
    m_pNewDelTBX->SetClickHdl          (LINK(this, SwFldVarPage, TBClickHdl));
    m_pChapterLevelLB->SetSelectHdl    (LINK(this, SwFldVarPage, ChapterHdl));
    m_pSeparatorED->SetModifyHdl       (LINK(this, SwFldVarPage, SeparatorHdl));

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(!IsRefresh() && sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            sal_uInt16 nVal = (sal_uInt16)sVal.ToInt32();
            if( USHRT_MAX != nVal )
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

    m_pTypeLB->SetUpdateMode(sal_True);

    if (IsFldEdit())
    {
        m_pSelectionLB->SaveValue();
        m_pFormatLB->SaveValue();
        nOldFormat = m_pNumFormatLB->GetFormat();
        m_pNameED->SaveValue();
        m_pValueED->SaveValue();
        m_pInvisibleCB->SaveValue();
        m_pChapterLevelLB->SaveValue();
        m_pSeparatorED->SaveValue();
    }
}

IMPL_LINK_NOARG(SwFldVarPage, TypeHdl)
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

    if (nOld != GetTypeSel() || nOld == LISTBOX_ENTRY_NOTFOUND)
    {
        bInit = sal_True;
        if (nOld != LISTBOX_ENTRY_NOTFOUND)
        {
            m_pNameED->SetText(aEmptyStr);
            m_pValueED->SetText(aEmptyStr);
        }

        m_pValueED->SetDropEnable(false);
        UpdateSubType();    // initialise selection-listboxes
    }

    bInit = sal_False;

    return 0;
}

IMPL_LINK( SwFldVarPage, SubTypeHdl, ListBox *, pBox )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());
    sal_uInt16 nSelPos = m_pSelectionLB->GetSelectEntryPos();

    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
        nSelPos = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nSelPos);

    if (IsFldEdit() && (!pBox || bInit))
    {
        if (nTypeId != TYP_FORMELFLD)
            m_pNameED->SetText(GetFldMgr().GetCurFldPar1());

        m_pValueED->SetText(GetFldMgr().GetCurFldPar2());
    }

    if (m_pNameFT->GetText() != OUString(sOldNameFT))
        m_pNameFT->SetText(sOldNameFT);
    if (m_pValueFT->GetText() != OUString(sOldValueFT))
        m_pValueFT->SetText(sOldValueFT);

    m_pNumFormatLB->SetUpdateMode(sal_False);
    m_pFormatLB->SetUpdateMode(sal_False);
    FillFormatLB(nTypeId);

    sal_uInt16 nSize = m_pFormatLB->GetEntryCount();

    sal_Bool bValue = sal_False, bName = sal_False, bNumFmt = sal_False,
            bInvisible = sal_False, bShowChapterFrame = sal_False;
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
                        m_pNameED->SetText(pType->GetName());

                        if (pType->GetType() == UF_STRING)
                        {
                            m_pValueED->SetText(pType->GetContent());
                            m_pNumFormatLB->SelectEntryPos(0);
                        }
                        else
                            m_pValueED->SetText(pType->GetContent());
                    }
                }
                else
                    m_pValueED->SetText(pType->GetContent());
            }
            else
            {
                if (pBox)   // only when interacting via mouse
                {
                    m_pNameED->SetText(aEmptyStr);
                    m_pValueED->SetText(aEmptyStr);
                }
            }
            bValue = bName = bNumFmt = bInvisible = sal_True;

            m_pValueED->SetDropEnable(true);
            break;
        }

        case TYP_SETFLD:
            bValue = sal_True;

            bNumFmt = bInvisible = sal_True;

            if (!IsFldDlgHtmlMode())
                bName = sal_True;
            else
            {
                m_pNumFormatLB->Clear();
                sal_uInt16 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
                m_pNumFormatLB->SelectEntryPos(0);
            }
            // is there a corresponding SetField
            if (IsFldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    String sName(m_pSelectionLB->GetSelectEntry());
                    m_pNameED->SetText(sName);

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
                                m_pNumFormatLB->SelectEntryPos(0); // textual
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
                m_pValueED->SetText(((SwSetExpField*)GetCurField())->GetFormula());
            }
            m_pValueED->SetDropEnable(true);
            break;

        case TYP_FORMELFLD:
            {
                bValue = sal_True;
                bNumFmt = sal_True;
                m_pValueFT->SetText(SW_RESSTR(STR_FORMULA));
                m_pValueED->SetDropEnable(true);
            }
            break;

        case TYP_GETFLD:
            {
                if (!IsFldEdit())
                {
                    m_pNameED->SetText(aEmptyStr);
                    m_pValueED->SetText(aEmptyStr);
                }

                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    String sName(m_pSelectionLB->GetSelectEntry());
                    if (!IsFldEdit())
                        m_pNameED->SetText(sName);

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
            m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));

            if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
            {
                bValue = bNumFmt = sal_True;

                String sName;

                sName = m_pSelectionLB->GetSelectEntry();
                m_pNameED->SetText( sName );

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
                            m_pNumFormatLB->Clear();

                            sal_uInt16 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_USERVAR_TEXT), 0);
                            m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
                            m_pNumFormatLB->SelectEntryPos(0);
                        }
                    }
                    if (IsFldEdit() && (!pBox || bInit) )
                        m_pValueED->SetText(((SwSetExpField*)GetCurField())->GetPromptText());
                }
                else    // USERFLD
                    bFormat = bNumFmt = sal_False;
            }
            break;

        case TYP_DDEFLD:
            m_pValueFT->SetText(SW_RESSTR(STR_DDE_CMD));

            if (IsFldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    SwDDEFieldType* pType =
                        (SwDDEFieldType*) GetFldMgr().GetFldType(RES_DDEFLD, nSelPos);

                    if(pType)
                    {
                        m_pNameED->SetText(pType->GetName());

                        //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
                        //              That's not considered here yet
                        String sCmd( pType->GetCmd() );
                        sal_uInt16 nTmpPos = sCmd.SearchAndReplace( sfx2::cTokenSeparator, ' ' );
                        sCmd.SearchAndReplace( sfx2::cTokenSeparator, ' ', nTmpPos );

                        m_pValueED->SetText( sCmd );
                        m_pFormatLB->SelectEntryPos(pType->GetType());
                    }
                }
            }
            bName = bValue = sal_True;
            break;

        case TYP_SEQFLD:
            {
                bName = bValue = bShowChapterFrame = true;

                SwFieldType* pFldTyp;
                if( IsFldEdit() )
                    pFldTyp = GetCurField()->GetTyp();
                else
                {
                    String sFldTypeName( m_pSelectionLB->GetEntry( nSelPos ));
                    if( sFldTypeName.Len() )
                        pFldTyp = GetFldMgr().GetFldType( RES_SETEXPFLD,
                                                          sFldTypeName );
                    else
                        pFldTyp = 0;
                }

                if( IsFldEdit() )
                    m_pValueED->SetText( ((SwSetExpField*)GetCurField())->
                                        GetFormula() );

                if( IsFldEdit() || pBox )   // only when interacting via mouse
                    m_pNameED->SetText( m_pSelectionLB->GetSelectEntry() );

                if( pFldTyp )
                {
                    sal_uInt8 nLevel = ((SwSetExpFieldType*)pFldTyp)->GetOutlineLvl();
                    if( 0x7f == nLevel )
                        m_pChapterLevelLB->SelectEntryPos( 0 );
                    else
                        m_pChapterLevelLB->SelectEntryPos( nLevel + 1 );
                    String sDelim = ((SwSetExpFieldType*)pFldTyp)->GetDelimiter();
                    m_pSeparatorED->SetText( sDelim );
                    ChapterHdl();
                }
            }
            break;

        case TYP_SETREFPAGEFLD:
            {
                bValue = sal_False;
                m_pValueFT->SetText( SW_RESSTR( STR_OFFSET ));

                if (IsFldEdit() || pBox)    // only when interacting via mouse
                    m_pNameED->SetText(aEmptyStr);

                if (nSelPos != 0 && nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    bValue = sal_True;      // SubType OFF - knows no Offset
                    if (IsFldEdit())
                        m_pValueED->SetText(OUString::number(((SwRefPageSetField*)GetCurField())->GetOffset()));
                }
            }
            break;

        case TYP_GETREFPAGEFLD:
            m_pNameED->SetText(aEmptyStr);
            m_pValueED->SetText(aEmptyStr);
            break;
    }

    m_pNumFormatLB->Show(bNumFmt);
    m_pFormatLB->Show(!bNumFmt);

    if (IsFldEdit())
        bName = sal_False;

    m_pFormat->Enable(bFormat|bNumFmt);
    m_pNameFT->Enable(bName);
    m_pNameED->Enable(bName);
    m_pValueFT->Enable(bValue);
    m_pValueED->Enable(bValue);

    m_pInvisibleCB->Show(!bShowChapterFrame);
    m_pChapterFrame->Show(bShowChapterFrame);
    m_pInvisibleCB->Enable(bInvisible);

    ModifyHdl();    // apply/insert/delete status update

    m_pNumFormatLB->SetUpdateMode(sal_True);
    m_pFormatLB->SetUpdateMode(sal_True);

    if(m_pSelectionLB->IsCallAddSelection())
    {
        nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        switch (nTypeId)
        {
            case TYP_FORMELFLD:
                {
                    nSelPos = m_pSelectionLB->GetSelectEntryPos();

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                        nSelPos = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nSelPos);

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND && pBox && !bInit)
                    {
                        m_pValueED->ReplaceSelected(m_pSelectionLB->GetSelectEntry());
                        ModifyHdl();
                    }
                }
                break;
        }
        m_pSelectionLB->ResetCallAddSelection();
    }
    return 0;
}

/*--------------------------------------------------------------------
     Description: renew types in SelectionBox
 --------------------------------------------------------------------*/
void SwFldVarPage::UpdateSubType()
{
    String sOldSel;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    SetSelectionSel(m_pSelectionLB->GetSelectEntryPos());
    if(GetSelectionSel() != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = m_pSelectionLB->GetEntry(GetSelectionSel());

    // fill Selection-Listbox
    m_pSelectionLB->SetUpdateMode(sal_False);
    m_pSelectionLB->Clear();

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
                nPos = m_pSelectionLB->InsertEntry(aList[i]);
                m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
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
                                m_pInvisibleCB->Check();
                        }
                        break;

                    case TYP_SETREFPAGEFLD:
                        if ((((SwRefPageSetField*)GetCurField())->IsOn() && i) ||
                            (!((SwRefPageSetField*)GetCurField())->IsOn() && !i))
                            sOldSel = aList[i];

                        // allow all entries for selection:
                        nPos = m_pSelectionLB->InsertEntry(aList[i]);
                        m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                        break;

                    default:
                        if (aList[i] == GetCurField()->GetPar1())
                            bInsert = sal_True;
                        break;
                }
                if (bInsert)
                {
                    nPos = m_pSelectionLB->InsertEntry(aList[i]);
                    m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                    if (nTypeId != TYP_FORMELFLD)
                        break;
                }
            }
        }
    }

    sal_Bool bEnable = m_pSelectionLB->GetEntryCount() != 0;
    ListBox *pLB = 0;

    if (bEnable)
    {
        m_pSelectionLB->SelectEntry(sOldSel);
        if (!m_pSelectionLB->GetSelectEntryCount())
        {
            m_pSelectionLB->SelectEntryPos(0);
            pLB = m_pSelectionLB;    // newly initialise all controls
        }
    }

    m_pSelection->Enable(bEnable);

    SubTypeHdl(pLB);
    m_pSelectionLB->SetUpdateMode(sal_True);
}

sal_uInt16 SwFldVarPage::FillFormatLB(sal_uInt16 nTypeId)
{
    String sOldSel, sOldNumSel;
    sal_uLong nOldNumFormat = 0;

    sal_uInt16 nFormatSel = m_pFormatLB->GetSelectEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = m_pFormatLB->GetEntry(nFormatSel);

    sal_uInt16 nNumFormatSel = m_pNumFormatLB->GetSelectEntryPos();
    if (nNumFormatSel != LISTBOX_ENTRY_NOTFOUND)
    {
        sOldNumSel = m_pNumFormatLB->GetEntry(nNumFormatSel);
        nOldNumFormat = m_pNumFormatLB->GetFormat();
    }

    // fill Format-Listbox
    m_pFormatLB->Clear();
    m_pNumFormatLB->Clear();
    sal_Bool bSpecialFmt = sal_False;

    if( TYP_GETREFPAGEFLD != nTypeId )
    {
        if (IsFldEdit())
        {
            bSpecialFmt = GetCurField()->GetFormat() == SAL_MAX_UINT32;

            if (!bSpecialFmt)
            {
                m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());
                sOldNumSel = aEmptyStr;
            }
            else
                if (nTypeId == TYP_GETFLD || nTypeId == TYP_FORMELFLD)
                    m_pNumFormatLB->SetFormatType(NUMBERFORMAT_NUMBER);
        }
        else
        {
            if (nOldNumFormat && nOldNumFormat != ULONG_MAX)
                m_pNumFormatLB->SetDefFormat(nOldNumFormat);
            else
                m_pNumFormatLB->SetFormatType(NUMBERFORMAT_NUMBER);
        }
    }

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            if (!IsFldEdit() || bSpecialFmt)
            {
                sal_uInt16 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_MARK_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
                nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_USERVAR_CMD), 1);
                m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
            }
        }
        break;

        case TYP_SETFLD:
        {
            if (!IsFldEdit() || bSpecialFmt)
            {
                sal_uInt16 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
            }
        }
        break;

        case TYP_FORMELFLD:
        {
            sal_uInt16 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
            m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
        }
        break;

        case TYP_GETFLD:
        {
            sal_uInt16 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
            m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
        }
        break;
    }

    if (IsFldEdit() && bSpecialFmt)
    {
        if (nTypeId == TYP_USERFLD && (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_CMD))
            m_pNumFormatLB->SelectEntryPos(1);
        else
            m_pNumFormatLB->SelectEntryPos(0);
    }
    else
    {
        if (!nOldNumFormat && (nNumFormatSel = m_pNumFormatLB->GetEntryPos(sOldNumSel)) != LISTBOX_ENTRY_NOTFOUND)
            m_pNumFormatLB->SelectEntryPos(nNumFormatSel);
        else if (nOldNumFormat && nOldNumFormat == ULONG_MAX)
            m_pNumFormatLB->SelectEntry(sOldSel);
    }

    sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        sal_uInt16 nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
        sal_uInt16 nFldId = GetFldMgr().GetFormatId( nTypeId, i );
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(nFldId) );
        if (IsFldEdit() && nFldId == GetCurField()->GetFormat())
            m_pFormatLB->SelectEntryPos( nPos );
    }

    if (nSize && (!IsFldEdit() || !m_pFormatLB->GetSelectEntryCount()))
    {
        m_pFormatLB->SelectEntry(sOldSel);

        if (!m_pFormatLB->GetSelectEntryCount())
        {
            m_pFormatLB->SelectEntry(SW_RESSTR(FMT_NUM_PAGEDESC));
            if (!m_pFormatLB->GetSelectEntryCount())
            {
                m_pFormatLB->SelectEntry(SW_RESSTR(FMT_NUM_ARABIC));
                if (!m_pFormatLB->GetSelectEntryCount())
                    m_pFormatLB->SelectEntryPos(0);
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
    String sValue(m_pValueED->GetText());
    sal_Bool bHasValue = sValue.Len() != 0;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());
    bool bInsert = false, bApply = false, bDelete = false;

    String sName( m_pNameED->GetText() );
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
            Selection aSel(m_pNameED->GetSelection());
            m_pNameED->SetText( sName );
            m_pNameED->SetSelection( aSel );   // restore Cursorpos
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

    m_pNewDelTBX->EnableItem(m_nApplyId, bApply);
    m_pNewDelTBX->EnableItem(m_nDeleteId, bDelete);
    EnableInsert(bInsert);

    return 0;
}

IMPL_LINK( SwFldVarPage, TBClickHdl, ToolBox *, pBox )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    sal_uInt16 nCurId = pBox->GetCurItemId();

    if (nCurId == m_nDeleteId)
    {
        if( nTypeId == TYP_USERFLD )
            GetFldMgr().RemoveFldType(RES_USERFLD, m_pSelectionLB->GetSelectEntry());
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

            GetFldMgr().RemoveFldType(nWhich, m_pSelectionLB->GetSelectEntry());
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
    else if (nCurId == m_nApplyId)
    {
        String sName(m_pNameED->GetText()), sValue(m_pValueED->GetText());
        SwFieldType* pType = 0;
        sal_uInt16 nId = 0;
        sal_uInt16 nNumFormatPos = m_pNumFormatLB->GetSelectEntryPos();

        switch (nTypeId)
        {
            case TYP_USERFLD:   nId = RES_USERFLD;  break;
            case TYP_DDEFLD:    nId = RES_DDEFLD;   break;
            case TYP_SETFLD:    nId = RES_SETEXPFLD;break;
        }
        pType = GetFldMgr().GetFldType(nId, sName);

        sal_uLong nFormat = m_pFormatLB->GetSelectEntryPos();
        if (nFormat != LISTBOX_ENTRY_NOTFOUND)
            nFormat = (sal_uLong)m_pFormatLB->GetEntryData((sal_uInt16)nFormat);

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
                        sal_uLong nFmt = nNumFormatPos == 0 ? 0 : m_pNumFormatLB->GetFormat();
                        if (nFmt)
                        {   // Switch language to office-language because Kalkulator expects
                            // String in office format and it should be fed into dialog like
                            // that
                            nFmt = SwValueField::GetSystemFormat(pSh->GetNumberFormatter(), nFmt);
                        }
                        ((SwUserFieldType*)pType)->SetContent(m_pValueED->GetText(), nFmt);
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
                        aType.SetContent( sValue, nNumFormatPos == 0 ? 0 : m_pNumFormatLB->GetFormat() );
                        m_pSelectionLB->InsertEntry(sName);
                        m_pSelectionLB->SelectEntry(sName);
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
                    m_pSelectionLB->InsertEntry(sName);
                    m_pSelectionLB->SelectEntry(sName);
                    GetFldMgr().InsertFldType(aType);   // DDE-Field new
                }
            }
        }
        if (IsFldEdit())
            GetFldMgr().GetCurFld();    // update FieldManager

        UpdateSubType();
    }

    return sal_True;
}

IMPL_LINK_NOARG(SwFldVarPage, ChapterHdl)
{
    sal_Bool bEnable = m_pChapterLevelLB->GetSelectEntryPos() != 0;

    m_pSeparatorED->Enable(bEnable);
    m_pSeparatorFT->Enable(bEnable);
    SeparatorHdl();

    return 0;
}

IMPL_LINK_NOARG(SwFldVarPage, SeparatorHdl)
{
    sal_Bool bEnable = !m_pSeparatorED->GetText().isEmpty() ||
                    m_pChapterLevelLB->GetSelectEntryPos() == 0;
    EnableInsert(bEnable);

    return 0;
}

sal_Bool SwFldVarPage::FillItemSet(SfxItemSet& )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    String aVal(m_pValueED->GetText());
    String aName(m_pNameED->GetText());

    sal_uInt16 nSubType = m_pSelectionLB->GetSelectEntryPos();
    if(nSubType == LISTBOX_ENTRY_NOTFOUND)
        nSubType = 0;
    else
        nSubType = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nSubType);

    sal_uLong nFormat;

    if (!m_pNumFormatLB->IsVisible())
    {
        nFormat = m_pFormatLB->GetSelectEntryPos();

        if(nFormat == LISTBOX_ENTRY_NOTFOUND)
            nFormat = 0;
        else
            nFormat = (sal_uLong)m_pFormatLB->GetEntryData((sal_uInt16)nFormat);
    }
    else
    {
        nFormat = m_pNumFormatLB->GetFormat();

        if (nFormat && nFormat != ULONG_MAX && m_pNumFormatLB->IsAutomaticLanguage())
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

            if (nFormat == ULONG_MAX && m_pNumFormatLB->GetSelectEntry() == SW_RESSTR(FMT_USERVAR_CMD))
                nSubType |= nsSwExtendedSubType::SUB_CMD;

            if (m_pInvisibleCB->IsChecked())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_FORMELFLD:
        {
            nSubType = nsSwGetSetExpType::GSE_FORMULA;
            if (m_pNumFormatLB->IsVisible() && nFormat == ULONG_MAX)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            break;
        }
        case TYP_GETFLD:
        {
            nSubType &= 0xff00;
            if (m_pNumFormatLB->IsVisible() && nFormat == ULONG_MAX)
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

            if (m_pInvisibleCB->IsChecked())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_SEQFLD:
        {
            nSubType = m_pChapterLevelLB->GetSelectEntryPos();
            if (nSubType == 0)
                nSubType = 0x7f;
            else
            {
                nSubType--;
                OUString sSeparator = OUString(m_pSeparatorED->GetText()[0]);
                cSeparator = !sSeparator.isEmpty() ? sSeparator[0] : ' ';
            }
            break;
        }
        case TYP_GETREFPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
                aVal = m_pValueED->GetText();
            break;
    }

    if (!IsFldEdit() ||
        m_pNameED->GetSavedValue() != m_pNameED->GetText() ||
        m_pValueED->GetSavedValue() != m_pValueED->GetText() ||
        m_pSelectionLB->GetSavedValue() != m_pSelectionLB->GetSelectEntryPos() ||
        m_pFormatLB->GetSavedValue() != m_pFormatLB->GetSelectEntryPos() ||
        nOldFormat != m_pNumFormatLB->GetFormat() ||
        m_pInvisibleCB->GetState() != m_pInvisibleCB->GetSavedValue() ||
        m_pChapterLevelLB->GetSavedValue() != m_pChapterLevelLB->GetSelectEntryPos() ||
        m_pSeparatorED->GetSavedValue() != m_pSeparatorED->GetText())
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat,
                    cSeparator, m_pNumFormatLB->IsAutomaticLanguage() );
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

SelectionListBox::SelectionListBox(Window* pParent, WinBits nStyle)
    : ListBox(pParent, nStyle)
    , bCallAddSelection(false)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSelectionListBox(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = VclBuilder::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nBits |= WB_BORDER;

    SelectionListBox* pListBox = new SelectionListBox(pParent, nBits|WB_SIMPLEMODE);
    pListBox->EnableAutoSize(true);

    return pListBox;
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
    sal_uInt16 nTypeSel = m_pTypeLB->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData( nTypeSel );
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
