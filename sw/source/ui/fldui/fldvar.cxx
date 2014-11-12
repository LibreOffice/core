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
#include <IDocumentFieldsAccess.hxx>
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

SwFldVarPage::SwFldVarPage(vcl::Window* pParent, const SfxItemSet& rCoreSet )
    : SwFldPage(pParent, "FldVarPage",
        "modules/swriter/ui/fldvarpage.ui", rCoreSet)
    , nOldFormat(0)
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
    m_pNumFormatLB->SetShowLanguageControl(true);
}

SwFldVarPage::~SwFldVarPage()
{
}

void SwFldVarPage::Reset(const SfxItemSet* )
{
    SavePos(m_pTypeLB);

    Init(); // general initialisation

    m_pTypeLB->SetUpdateMode(false);
    m_pTypeLB->Clear();

    sal_Int32 nPos;
    sal_uInt16 nTypeId;

    if (!IsFldEdit())
    {
        // initialise TypeListBox
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for (short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = SwFldMgr::GetTypeId(i);
            nPos = m_pTypeLB->InsertEntry(SwFldMgr::GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        const SwField* pCurField = GetCurField();
        assert(pCurField && "<SwFldVarPage::Reset(..)> - <SwField> instance missing!");
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_SETINPFLD)
            nTypeId = TYP_INPUTFLD;
        nPos = m_pTypeLB->InsertEntry(SwFldMgr::GetTypeStr(SwFldMgr::GetPos(nTypeId)));
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
        OUString sUserData = GetUserData();
        if(!IsRefresh() && sUserData.getToken(0, ';').equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            OUString sVal = sUserData.getToken(1, ';');
            sal_uInt16 nVal = (sal_uInt16)sVal.toInt32();
            if( USHRT_MAX != nVal )
            {
                for(sal_Int32 i = 0; i < m_pTypeLB->GetEntryCount(); i++)
                    if(nVal == (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(i))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    m_pTypeLB->SetUpdateMode(true);

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
    const sal_Int32 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_pTypeLB->GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        m_pTypeLB->SelectEntryPos(0);
    }

    if (nOld != GetTypeSel() || nOld == LISTBOX_ENTRY_NOTFOUND)
    {
        bInit = true;
        if (nOld != LISTBOX_ENTRY_NOTFOUND)
        {
            m_pNameED->SetText(OUString());
            m_pValueED->SetText(OUString());
        }

        m_pValueED->SetDropEnable(false);
        UpdateSubType();    // initialise selection-listboxes
    }

    bInit = false;

    return 0;
}

IMPL_LINK( SwFldVarPage, SubTypeHdl, ListBox *, pBox )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());
    sal_Int32 nSelPos = m_pSelectionLB->GetSelectEntryPos();
    sal_uInt16 nSelData = USHRT_MAX;

    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
        nSelData = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nSelPos);

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

    m_pNumFormatLB->SetUpdateMode(false);
    m_pFormatLB->SetUpdateMode(false);
    FillFormatLB(nTypeId);

    sal_Int32 nSize = m_pFormatLB->GetEntryCount();

    bool bValue = false, bName = false, bNumFmt = false,
            bInvisible = false, bShowChapterFrame = false;
    bool bFormat = nSize != 0;

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            // change or create user type
            SwUserFieldType* pType = (SwUserFieldType*)
                GetFldMgr().GetFldType(RES_USERFLD, nSelData);

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
                    m_pNameED->SetText(OUString());
                    m_pValueED->SetText(OUString());
                }
            }
            bValue = bName = bNumFmt = bInvisible = true;

            m_pValueED->SetDropEnable(true);
            break;
        }

        case TYP_SETFLD:
            bValue = true;

            bNumFmt = bInvisible = true;

            if (!IsFldDlgHtmlMode())
                bName = true;
            else
            {
                m_pNumFormatLB->Clear();
                sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
                m_pNumFormatLB->SelectEntryPos(0);
            }
            // is there a corresponding SetField
            if (IsFldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    OUString sName(m_pSelectionLB->GetSelectEntry());
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
            if (GetCurField() != NULL && IsFldEdit())
            {
                // GetFormula leads to problems with date formats because
                // only the numeric value without formatting is returned.
                // It must be used though because otherwise in GetPar2 only
                // the value calculated by Kalkulator would be displayed
                // (instead of test2 = test + 1)
                m_pValueED->SetText(((SwSetExpField*)GetCurField())->GetFormula());
            }
            m_pValueED->SetDropEnable(true);
            break;

        case TYP_FORMELFLD:
            {
                bValue = true;
                bNumFmt = true;
                m_pValueFT->SetText(SW_RESSTR(STR_FORMULA));
                m_pValueED->SetDropEnable(true);
            }
            break;

        case TYP_GETFLD:
            {
                if (!IsFldEdit())
                {
                    m_pNameED->SetText(OUString());
                    m_pValueED->SetText(OUString());
                }

                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    OUString sName(m_pSelectionLB->GetSelectEntry());
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
                                bFormat = true;
                            else                    // numeric
                                bNumFmt = true;
                        }
                    }
                }
                else
                    bFormat = false;

                EnableInsert(bFormat || bNumFmt);
            }
            break;

        case TYP_INPUTFLD:
            m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));

            if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
            {
                bValue = bNumFmt = true;

                OUString sName;

                sName = m_pSelectionLB->GetSelectEntry();
                m_pNameED->SetText( sName );

                // User- or SetField ?
                const sal_uInt16 nInpType = static_cast< sal_uInt16 >
                    (GetFldMgr().GetFldType(RES_USERFLD, sName) ? 0 : TYP_SETINPFLD);

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

                            sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_USERVAR_TEXT), 0);
                            m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
                            m_pNumFormatLB->SelectEntryPos(0);
                        }
                    }
                    if (GetCurField() && IsFldEdit() && (!pBox || bInit) )
                        m_pValueED->SetText(((SwSetExpField*)GetCurField())->GetPromptText());
                }
                else    // USERFLD
                    bFormat = bNumFmt = false;
            }
            break;

        case TYP_DDEFLD:
            m_pValueFT->SetText(SW_RESSTR(STR_DDE_CMD));

            if (IsFldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    SwDDEFieldType* pType =
                        (SwDDEFieldType*) GetFldMgr().GetFldType(RES_DDEFLD, nSelData);

                    if(pType)
                    {
                        m_pNameED->SetText(pType->GetName());

                        //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
                        //              That's not considered here yet
                        OUString sCmd( pType->GetCmd() );
                        sal_Int32 nTmpPos = 0;
                        sCmd = sCmd.replaceFirst( OUString(sfx2::cTokenSeparator), " ", &nTmpPos );
                        sCmd = sCmd.replaceFirst( OUString(sfx2::cTokenSeparator), " ", &nTmpPos );

                        m_pValueED->SetText( sCmd );
                        m_pFormatLB->SelectEntryPos(pType->GetType());
                    }
                }
            }
            bName = bValue = true;
            break;

        case TYP_SEQFLD:
            {
                bName = bValue = bShowChapterFrame = true;

                SwFieldType* pFldTyp;
                if( GetCurField() && IsFldEdit() )
                    pFldTyp = GetCurField()->GetTyp();
                else
                {
                    OUString sFldTypeName( m_pSelectionLB->GetEntry( nSelPos ));
                    if( !sFldTypeName.isEmpty() )
                        pFldTyp = GetFldMgr().GetFldType( RES_SETEXPFLD,
                                                          sFldTypeName );
                    else
                        pFldTyp = 0;
                }

                if( GetCurField() && IsFldEdit() )
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
                    OUString sDelim = ((SwSetExpFieldType*)pFldTyp)->GetDelimiter();
                    m_pSeparatorED->SetText( sDelim );
                    ChapterHdl();
                }
            }
            break;

        case TYP_SETREFPAGEFLD:
            {
                bValue = false;
                m_pValueFT->SetText( SW_RESSTR( STR_OFFSET ));

                if (IsFldEdit() || pBox)    // only when interacting via mouse
                    m_pNameED->SetText(OUString());

                if (nSelPos != 0 && nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    bValue = true;      // SubType OFF - knows no Offset
                    if (GetCurField() && IsFldEdit())
                        m_pValueED->SetText(OUString::number(((SwRefPageSetField*)GetCurField())->GetOffset()));
                }
            }
            break;

        case TYP_GETREFPAGEFLD:
            m_pNameED->SetText(OUString());
            m_pValueED->SetText(OUString());
            break;
    }

    m_pNumFormatLB->Show(bNumFmt);
    m_pFormatLB->Show(!bNumFmt);

    if (IsFldEdit())
        bName = false;

    m_pFormat->Enable(bFormat || bNumFmt);
    m_pNameFT->Enable(bName);
    m_pNameED->Enable(bName);
    m_pValueFT->Enable(bValue);
    m_pValueED->Enable(bValue);

    m_pInvisibleCB->Show(!bShowChapterFrame);
    m_pChapterFrame->Show(bShowChapterFrame);
    m_pInvisibleCB->Enable(bInvisible);

    ModifyHdl();    // apply/insert/delete status update

    m_pNumFormatLB->SetUpdateMode(true);
    m_pFormatLB->SetUpdateMode(true);

    if(m_pSelectionLB->IsCallAddSelection())
    {
        nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        switch (nTypeId)
        {
            case TYP_FORMELFLD:
                {
                    nSelPos = m_pSelectionLB->GetSelectEntryPos();

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                        nSelData = (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nSelPos);
                    else
                        nSelData = USHRT_MAX;

                    if (nSelData != USHRT_MAX && pBox && !bInit)
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

// renew types in SelectionBox
void SwFldVarPage::UpdateSubType()
{
    SetSelectionSel(m_pSelectionLB->GetSelectEntryPos());

    OUString sOldSel;
    if (GetSelectionSel() != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = m_pSelectionLB->GetEntry(GetSelectionSel());

    // fill Selection-Listbox
    m_pSelectionLB->SetUpdateMode(false);
    m_pSelectionLB->Clear();

    const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());
    std::vector<OUString> aList;
    GetFldMgr().GetSubTypes(nTypeId, aList);
    const size_t nCount = aList.size();
    for (size_t i = 0; i < nCount; ++i)
    {
        if (nTypeId != TYP_INPUTFLD || i)
        {
            if (!IsFldEdit())
            {
                const size_t nPos = m_pSelectionLB->InsertEntry(aList[i]);
                m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
            }
            else
            {
                bool bInsert = false;

                switch (nTypeId)
                {
                    case TYP_INPUTFLD:
                        if (GetCurField() && aList[i] == GetCurField()->GetPar1())
                            bInsert = true;
                        break;

                    case TYP_FORMELFLD:
                        bInsert = true;
                        break;

                    case TYP_GETFLD:
                        if (GetCurField() && aList[i] == ((const SwFormulaField*)GetCurField())->GetFormula())
                            bInsert = true;
                        break;

                    case TYP_SETFLD:
                    case TYP_USERFLD:
                        if (GetCurField() && aList[i] == GetCurField()->GetTyp()->GetName())
                        {
                            bInsert = true;
                            if (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE)
                                m_pInvisibleCB->Check();
                        }
                        break;

                    case TYP_SETREFPAGEFLD:
                    {
                        if (GetCurField() != NULL
                            && ((((SwRefPageSetField*)GetCurField())->IsOn()
                                 && i) || (!((SwRefPageSetField*)GetCurField())
                                                ->IsOn() && !i)))
                        {
                            sOldSel = aList[i];
                        }

                        // allow all entries for selection:
                        const size_t nPos = m_pSelectionLB->InsertEntry(aList[i]);
                        m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));

                        break;
                    }
                    default:
                        if (GetCurField() && aList[i] == GetCurField()->GetPar1())
                            bInsert = true;
                        break;
                }

                if (bInsert)
                {
                    const size_t nPos = m_pSelectionLB->InsertEntry(aList[i]);
                    m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
                    if (nTypeId != TYP_FORMELFLD)
                        break;
                }
            }
        }
    }

    const bool bEnable = m_pSelectionLB->GetEntryCount() != 0;
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
    m_pSelectionLB->SetUpdateMode(true);
}

sal_Int32 SwFldVarPage::FillFormatLB(sal_uInt16 nTypeId)
{
    OUString sOldSel;
    const sal_Int32 nFormatSel = m_pFormatLB->GetSelectEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = m_pFormatLB->GetEntry(nFormatSel);

    OUString sOldNumSel;
    sal_uLong nOldNumFormat = 0;
    sal_Int32 nNumFormatSel = m_pNumFormatLB->GetSelectEntryPos();
    if (nNumFormatSel != LISTBOX_ENTRY_NOTFOUND)
    {
        sOldNumSel = m_pNumFormatLB->GetEntry(nNumFormatSel);
        nOldNumFormat = m_pNumFormatLB->GetFormat();
    }

    // fill Format-Listbox
    m_pFormatLB->Clear();
    m_pNumFormatLB->Clear();
    bool bSpecialFmt = false;

    if( TYP_GETREFPAGEFLD != nTypeId )
    {
        if (GetCurField() != NULL && IsFldEdit())
        {
            bSpecialFmt = GetCurField()->GetFormat() == SAL_MAX_UINT32;

            if (!bSpecialFmt)
            {
                m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());
                sOldNumSel.clear();
            }
            else if (nTypeId == TYP_GETFLD || nTypeId == TYP_FORMELFLD)
            {
                m_pNumFormatLB->SetFormatType(NUMBERFORMAT_NUMBER);
            }
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
                sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_MARK_TEXT), 0);
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
                sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
            }
        }
        break;

        case TYP_FORMELFLD:
        {
            sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
            m_pNumFormatLB->SetEntryData(nPos, (void *)ULONG_MAX);
        }
        break;

        case TYP_GETFLD:
        {
            sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
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

    const sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        const sal_Int32 nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
        const sal_uInt16 nFldId = GetFldMgr().GetFormatId( nTypeId, i );
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(nFldId) );
        if (IsFldEdit() && GetCurField() && nFldId == GetCurField()->GetFormat())
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

// Modify
IMPL_LINK_NOARG(SwFldVarPage, ModifyHdl)
{
    OUString sValue(m_pValueED->GetText());
    bool bHasValue = !sValue.isEmpty();
    const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());
    bool bInsert = false, bApply = false, bDelete = false;

    OUString sName( m_pNameED->GetText() );
    sal_Int32 nLen = sName.getLength();

    switch( nTypeId )
    {
    case TYP_DDEFLD:
    case TYP_USERFLD:
    case TYP_SETFLD:
    case TYP_SEQFLD:
        SwCalc::IsValidVarName( sName, &sName );
        if ( sName.getLength() != nLen )
        {
            nLen = sName.getLength();
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
                    const SwFldTypes* p = pSh->GetDoc()->getIDocumentFieldsAccess().GetFldTypes();
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
    const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    const sal_uInt16 nCurId = pBox->GetCurItemId();

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
        OUString sName(m_pNameED->GetText()), sValue(m_pValueED->GetText());
        SwFieldType* pType = 0;
        sal_uInt16 nId = 0;
        sal_Int32 nNumFormatPos = m_pNumFormatLB->GetSelectEntryPos();

        switch (nTypeId)
        {
            case TYP_USERFLD:   nId = RES_USERFLD;  break;
            case TYP_DDEFLD:    nId = RES_DDEFLD;   break;
            case TYP_SETFLD:    nId = RES_SETEXPFLD;break;
        }
        pType = GetFldMgr().GetFldType(nId, sName);

        sal_uLong nFormat = m_pFormatLB->GetSelectEntryPos();
        if (nFormat != LISTBOX_ENTRY_NOTFOUND)
            nFormat = (sal_uLong)m_pFormatLB->GetEntryData((sal_Int32)nFormat);

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
                        sal_Int32 nTmpPos = 0;
                        sValue = sValue.replaceFirst( " ", OUString(sfx2::cTokenSeparator), &nTmpPos );
                        sValue = sValue.replaceFirst( " ", OUString(sfx2::cTokenSeparator), &nTmpPos );
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
                    sal_Int32 nTmpPos = 0;
                    sValue = sValue.replaceFirst( " ", OUString(sfx2::cTokenSeparator), &nTmpPos );
                    sValue = sValue.replaceFirst( " ", OUString(sfx2::cTokenSeparator), &nTmpPos );

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
    bool bEnable = m_pChapterLevelLB->GetSelectEntryPos() != 0;

    m_pSeparatorED->Enable(bEnable);
    m_pSeparatorFT->Enable(bEnable);
    SeparatorHdl();

    return 0;
}

IMPL_LINK_NOARG(SwFldVarPage, SeparatorHdl)
{
    bool bEnable = !m_pSeparatorED->GetText().isEmpty() ||
                    m_pChapterLevelLB->GetSelectEntryPos() == 0;
    EnableInsert(bEnable);

    return 0;
}

bool SwFldVarPage::FillItemSet(SfxItemSet* )
{
    const sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    OUString aVal(m_pValueED->GetText());
    OUString aName(m_pNameED->GetText());

    const sal_Int32 nSubPos = m_pSelectionLB->GetSelectEntryPos();
    sal_uInt16 nSubType = (nSubPos == LISTBOX_ENTRY_NOTFOUND) ? 0 :
        (sal_uInt16)(sal_uLong)m_pSelectionLB->GetEntryData(nSubPos);

    sal_uLong nFormat;

    if (!m_pNumFormatLB->IsVisible())
    {
        sal_Int32 nFormatPos = m_pFormatLB->GetSelectEntryPos();

        if(nFormatPos == LISTBOX_ENTRY_NOTFOUND)
            nFormat = 0;
        else
            nFormat = (sal_uLong)m_pFormatLB->GetEntryData(nFormatPos);
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
        m_pNameED->IsValueChangedFromSaved() ||
        m_pValueED->IsValueChangedFromSaved() ||
        m_pSelectionLB->IsValueChangedFromSaved() ||
        m_pFormatLB->IsValueChangedFromSaved() ||
        nOldFormat != m_pNumFormatLB->GetFormat() ||
        m_pInvisibleCB->IsValueChangedFromSaved() ||
        m_pChapterLevelLB->IsValueChangedFromSaved() ||
        m_pSeparatorED->IsValueChangedFromSaved())
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat,
                    cSeparator, m_pNumFormatLB->IsAutomaticLanguage() );
    }

    UpdateSubType();

    return false;
}

SfxTabPage* SwFldVarPage::Create(   vcl::Window* pParent,
                        const SfxItemSet* rAttrSet )
{
    return ( new SwFldVarPage( pParent, *rAttrSet ) );
}

sal_uInt16 SwFldVarPage::GetGroup()
{
    return GRP_VAR;
}

SelectionListBox::SelectionListBox(vcl::Window* pParent, WinBits nStyle)
    : ListBox(pParent, nStyle)
    , bCallAddSelection(false)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSelectionListBox(vcl::Window *pParent, VclBuilder::stringmap &rMap)
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

bool SelectionListBox::PreNotify( NotifyEvent& rNEvt )
{
    bool nHandled = ListBox::PreNotify( rNEvt );
    if ( rNEvt.GetType() == EVENT_KEYUP )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
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
    OUString sData(USER_DATA_VERSION);
    sData += ";";
    sal_Int32 nTypeSel = m_pTypeLB->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData( nTypeSel );
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
