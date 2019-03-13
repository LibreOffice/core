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
#include <vcl/builderfactory.hxx>
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
#include "fldvar.hxx"
#include "flddinf.hxx"
#include <calc.hxx>
#include <svl/zformat.hxx>
#include <globals.hrc>
#include <strings.hrc>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFieldVarPage::SwFieldVarPage(vcl::Window* pParent, const SfxItemSet *const pCoreSet )
    : SwFieldPage(pParent, "FieldVarPage",
        "modules/swriter/ui/fldvarpage.ui", pCoreSet)
    , nOldFormat(0)
    , bInit(true)
{
    get(m_pTypeLB, "type");
    FillFieldSelect(*m_pTypeLB);
    get(m_pSelection, "selectframe");
    get(m_pSelectionLB, "select");
    m_pSelectionLB->SetStyle(m_pSelectionLB->GetStyle() | WB_SORT);
    get(m_pFormat, "formatframe");
    get(m_pChapterFrame, "chapterframe");
    get(m_pNameFT, "nameft");
    get(m_pNameED, "name");
    get(m_pValueFT, "valueft");
    get(m_pValueED, "value");
    get(m_pNumFormatLB, "numformat");
    get(m_pFormatLB, "format");
    FillFieldSelect(*m_pFormatLB);
    get(m_pChapterLevelLB, "level");
    get(m_pInvisibleCB, "invisible");
    get(m_pSeparatorFT, "separatorft");
    get(m_pSeparatorED, "separator");
    get(m_pNewPB, "apply");
    get(m_pDelPB, "delete");

    long nHeight = m_pTypeLB->GetTextHeight() * 20;
    m_pTypeLB->set_height_request(nHeight);
    m_pSelectionLB->set_height_request(nHeight);
    m_pFormatLB->set_height_request(nHeight/2);

    long nWidth = m_pTypeLB->LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MapUnit::MapAppFont)).Width();
    m_pTypeLB->set_width_request(nWidth);
    m_pSelectionLB->set_width_request(nWidth);
    m_pFormatLB->set_width_request(nWidth);

    sOldValueFT = m_pValueFT->GetText();
    sOldNameFT = m_pNameFT->GetText();

    for (sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        m_pChapterLevelLB->InsertEntry(OUString::number(i));

    m_pChapterLevelLB->SelectEntryPos(0);
    //enable 'active' language selection
    m_pNumFormatLB->SetShowLanguageControl(true);
}

SwFieldVarPage::~SwFieldVarPage()
{
    disposeOnce();
}

void SwFieldVarPage::dispose()
{
    m_pTypeLB.clear();
    m_pSelection.clear();
    m_pSelectionLB.clear();
    m_pNameFT.clear();
    m_pNameED.clear();
    m_pValueFT.clear();
    m_pValueED.clear();
    m_pFormat.clear();
    m_pNumFormatLB.clear();
    m_pFormatLB.clear();
    m_pChapterFrame.clear();
    m_pChapterLevelLB.clear();
    m_pInvisibleCB.clear();
    m_pSeparatorFT.clear();
    m_pSeparatorED.clear();
    m_pNewPB.clear();
    m_pDelPB.clear();
    SwFieldPage::dispose();
}

void SwFieldVarPage::Reset(const SfxItemSet* )
{
    SavePos(m_pTypeLB);

    Init(); // general initialisation

    m_pTypeLB->SetUpdateMode(false);
    m_pTypeLB->Clear();

    sal_Int32 nPos;
    sal_uInt16 nTypeId;

    if (!IsFieldEdit())
    {
        // initialise TypeListBox
        const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

        for (short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = SwFieldMgr::GetTypeId(i);
            nPos = m_pTypeLB->InsertEntry(SwFieldMgr::GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        const SwField* pCurField = GetCurField();
        assert(pCurField && "<SwFieldVarPage::Reset(..)> - <SwField> instance missing!");
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_SETINPFLD)
            nTypeId = TYP_INPUTFLD;
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

    m_pTypeLB->SetDoubleClickHdl       (LINK(this, SwFieldVarPage, ListBoxInsertHdl));
    m_pTypeLB->SetSelectHdl            (LINK(this, SwFieldVarPage, TypeHdl));
    m_pSelectionLB->SetSelectHdl       (LINK(this, SwFieldVarPage, SubTypeListBoxHdl));
    m_pSelectionLB->SetDoubleClickHdl  (LINK(this, SwFieldVarPage, ListBoxInsertHdl));
    m_pFormatLB->SetDoubleClickHdl     (LINK(this, SwFieldVarPage, ListBoxInsertHdl));
    m_pNumFormatLB->SetDoubleClickHdl  (LINK(this, SwFieldVarPage, ListBoxInsertHdl));
    m_pNameED->SetModifyHdl            (LINK(this, SwFieldVarPage, ModifyHdl));
    m_pValueED->SetModifyHdl           (LINK(this, SwFieldVarPage, ModifyHdl));
    m_pNewPB->SetClickHdl              (LINK(this, SwFieldVarPage, TBClickHdl));
    m_pDelPB->SetClickHdl              (LINK(this, SwFieldVarPage, TBClickHdl));
    m_pChapterLevelLB->SetSelectHdl    (LINK(this, SwFieldVarPage, ChapterHdl));
    m_pSeparatorED->SetModifyHdl       (LINK(this, SwFieldVarPage, SeparatorHdl));

    if( !IsRefresh() )
    {
        OUString sUserData = GetUserData();
        sal_Int32 nIdx{ 0 };
        if(!IsRefresh() && sUserData.getToken(0, ';', nIdx).equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            OUString sVal = sUserData.getToken(0, ';', nIdx);
            sal_uInt16 nVal = static_cast<sal_uInt16>(sVal.toInt32());
            if( USHRT_MAX != nVal )
            {
                for(sal_Int32 i = 0; i < m_pTypeLB->GetEntryCount(); i++)
                    if(nVal == static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(i))))
                    {
                        m_pTypeLB->SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(*m_pTypeLB);

    m_pTypeLB->SetUpdateMode(true);

    if (IsFieldEdit())
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

IMPL_LINK_NOARG(SwFieldVarPage, TypeHdl, ListBox&, void)
{
    // save old ListBoxPos
    const sal_Int32 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_pTypeLB->GetSelectedEntryPos());

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
}

IMPL_LINK( SwFieldVarPage, SubTypeListBoxHdl, ListBox&, rBox, void )
{
    SubTypeHdl(&rBox);
}

void SwFieldVarPage::SubTypeHdl(ListBox const * pBox)
{
    sal_uInt16 nTypeId = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel())));
    sal_Int32 nSelPos = m_pSelectionLB->GetSelectedEntryPos();
    size_t nSelData = SIZE_MAX;

    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
        nSelData = static_cast<size_t>(reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nSelPos)));

    if (IsFieldEdit() && (!pBox || bInit))
    {
        if (nTypeId != TYP_FORMELFLD)
            m_pNameED->SetText(GetFieldMgr().GetCurFieldPar1());

        m_pValueED->SetText(GetFieldMgr().GetCurFieldPar2());
    }

    if (m_pNameFT->GetText() != sOldNameFT)
        m_pNameFT->SetText(sOldNameFT);
    if (m_pValueFT->GetText() != sOldValueFT)
        m_pValueFT->SetText(sOldValueFT);

    m_pNumFormatLB->SetUpdateMode(false);
    m_pFormatLB->SetUpdateMode(false);
    FillFormatLB(nTypeId);

    sal_Int32 nSize = m_pFormatLB->GetEntryCount();

    bool bValue = false, bName = false, bNumFormat = false,
            bInvisible = false, bShowChapterFrame = false;
    bool bFormat = nSize != 0;

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            // change or create user type
            SwUserFieldType* pType = static_cast<SwUserFieldType*>(
                GetFieldMgr().GetFieldType(SwFieldIds::User, nSelData));

            if (pType)
            {
                if (!IsFieldEdit())
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
            bValue = bName = bNumFormat = bInvisible = true;

            m_pValueED->SetDropEnable(true);
            break;
        }

        case TYP_SETFLD:
            bValue = true;

            bNumFormat = bInvisible = true;

            if (!IsFieldDlgHtmlMode())
                bName = true;
            else
            {
                m_pNumFormatLB->Clear();
                sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SwResId(FMT_SETVAR_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, reinterpret_cast<void *>(NUMBERFORMAT_ENTRY_NOT_FOUND));
                m_pNumFormatLB->SelectEntryPos(0);
            }
            // is there a corresponding SetField
            if (IsFieldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    OUString sName(m_pSelectionLB->GetSelectedEntry());
                    m_pNameED->SetText(sName);

                    if (!IsFieldDlgHtmlMode())
                    {
                        SwWrtShell *pSh = GetWrtShell();
                        if(!pSh)
                            pSh = ::GetActiveWrtShell();
                        if(pSh)
                        {
                            SwSetExpFieldType* pSetTyp = static_cast<SwSetExpFieldType*>(
                                    pSh->GetFieldType(SwFieldIds::SetExp, sName));

                            if (pSetTyp && pSetTyp->GetType() == nsSwGetSetExpType::GSE_STRING)
                                m_pNumFormatLB->SelectEntryPos(0); // textual
                        }
                    }
                }
            }
            if (GetCurField() != nullptr && IsFieldEdit())
            {
                // GetFormula leads to problems with date formats because
                // only the numeric value without formatting is returned.
                // It must be used though because otherwise in GetPar2 only
                // the value calculated by Kalkulator would be displayed
                // (instead of test2 = test + 1)
                m_pValueED->SetText(static_cast<SwSetExpField*>(GetCurField())->GetFormula());
            }
            m_pValueED->SetDropEnable(true);
            break;

        case TYP_FORMELFLD:
            {
                bValue = true;
                bNumFormat = true;
                m_pValueFT->SetText(SwResId(STR_FORMULA));
                m_pValueED->SetDropEnable(true);
            }
            break;

        case TYP_GETFLD:
            {
                if (!IsFieldEdit())
                {
                    m_pNameED->SetText(OUString());
                    m_pValueED->SetText(OUString());
                }

                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    OUString sName(m_pSelectionLB->GetSelectedEntry());
                    if (!IsFieldEdit())
                        m_pNameED->SetText(sName);

                    // is there a corresponding SetField
                    SwWrtShell *pSh = GetWrtShell();
                    if(!pSh)
                        pSh = ::GetActiveWrtShell();
                    if(pSh)
                    {
                        SwSetExpFieldType* pSetTyp = static_cast<SwSetExpFieldType*>(
                                pSh->GetFieldType(SwFieldIds::SetExp, sName));

                        if(pSetTyp)
                        {
                            if (pSetTyp->GetType() & nsSwGetSetExpType::GSE_STRING)    // textual?
                                bFormat = true;
                            else                    // numeric
                                bNumFormat = true;
                        }
                    }
                }
                else
                    bFormat = false;

                EnableInsert(bFormat || bNumFormat);
            }
            break;

        case TYP_INPUTFLD:
            m_pValueFT->SetText(SwResId(STR_PROMPT));

            if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
            {
                bValue = bNumFormat = true;

                OUString sName;

                sName = m_pSelectionLB->GetSelectedEntry();
                m_pNameED->SetText( sName );

                // User- or SetField ?
                const sal_uInt16 nInpType = static_cast< sal_uInt16 >
                    (GetFieldMgr().GetFieldType(SwFieldIds::User, sName) ? 0 : TYP_SETINPFLD);

                if (nInpType)   // SETEXPFLD
                {
                    // is there a corresponding SetField
                    SwSetExpFieldType* pSetTyp = static_cast<SwSetExpFieldType*>(
                                GetFieldMgr().GetFieldType(SwFieldIds::SetExp, sName));

                    if(pSetTyp)
                    {
                        if (pSetTyp->GetType() == nsSwGetSetExpType::GSE_STRING)    // textual?
                        {
                            m_pNumFormatLB->Clear();

                            sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SwResId(FMT_USERVAR_TEXT), 0);
                            m_pNumFormatLB->SetEntryData(nPos, reinterpret_cast<void *>(NUMBERFORMAT_ENTRY_NOT_FOUND));
                            m_pNumFormatLB->SelectEntryPos(0);
                        }
                    }
                    if (GetCurField() && IsFieldEdit() && (!pBox || bInit) )
                        m_pValueED->SetText(static_cast<SwSetExpField*>(GetCurField())->GetPromptText());
                }
                else    // USERFLD
                    bFormat = bNumFormat = false;
            }
            break;

        case TYP_DDEFLD:
            m_pValueFT->SetText(SwResId(STR_DDE_CMD));

            if (IsFieldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    SwDDEFieldType* pType =
                        static_cast<SwDDEFieldType*>( GetFieldMgr().GetFieldType(SwFieldIds::Dde, nSelData) );

                    if(pType)
                    {
                        m_pNameED->SetText(pType->GetName());

                        //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
                        //              That's not considered here yet
                        OUString sCmd( pType->GetCmd() );
                        sal_Int32 nTmpPos = 0;
                        sCmd = sCmd.replaceFirst( OUStringLiteral1(sfx2::cTokenSeparator), " ", &nTmpPos );
                        sCmd = sCmd.replaceFirst( OUStringLiteral1(sfx2::cTokenSeparator), " ", &nTmpPos );

                        m_pValueED->SetText( sCmd );
                        m_pFormatLB->SelectEntryPos(static_cast<int>(pType->GetType()));
                    }
                }
            }
            bName = bValue = true;
            break;

        case TYP_SEQFLD:
            {
                bName = bValue = bShowChapterFrame = true;

                SwFieldType* pFieldTyp;
                if( GetCurField() && IsFieldEdit() )
                    pFieldTyp = GetCurField()->GetTyp();
                else
                {
                    OUString sFieldTypeName( m_pSelectionLB->GetEntry( nSelPos ));
                    if( !sFieldTypeName.isEmpty() )
                        pFieldTyp = GetFieldMgr().GetFieldType( SwFieldIds::SetExp,
                                                          sFieldTypeName );
                    else
                        pFieldTyp = nullptr;
                }

                if( GetCurField() && IsFieldEdit() )
                    m_pValueED->SetText( static_cast<SwSetExpField*>(GetCurField())->
                                        GetFormula() );

                if( IsFieldEdit() || pBox )   // only when interacting via mouse
                    m_pNameED->SetText( m_pSelectionLB->GetSelectedEntry() );

                if( pFieldTyp )
                {
                    sal_uInt8 nLevel = static_cast<SwSetExpFieldType*>(pFieldTyp)->GetOutlineLvl();
                    if( 0x7f == nLevel )
                        m_pChapterLevelLB->SelectEntryPos( 0 );
                    else
                        m_pChapterLevelLB->SelectEntryPos( nLevel + 1 );
                    OUString sDelim = static_cast<SwSetExpFieldType*>(pFieldTyp)->GetDelimiter();
                    m_pSeparatorED->SetText( sDelim );
                    ChapterHdl(*m_pChapterLevelLB);
                }
            }
            break;

        case TYP_SETREFPAGEFLD:
            {
                bValue = false;
                m_pValueFT->SetText( SwResId( STR_OFFSET ));

                if (IsFieldEdit() || pBox)    // only when interacting via mouse
                    m_pNameED->SetText(OUString());

                if (nSelData != 0 && nSelData != SIZE_MAX)
                {
                    bValue = true;      // SubType OFF - knows no Offset
                    if (GetCurField() && IsFieldEdit())
                        m_pValueED->SetText(OUString::number(static_cast<SwRefPageSetField*>(GetCurField())->GetOffset()));
                }
            }
            break;

        case TYP_GETREFPAGEFLD:
            m_pNameED->SetText(OUString());
            m_pValueED->SetText(OUString());
            break;
    }

    m_pNumFormatLB->Show(bNumFormat);
    m_pFormatLB->Show(!bNumFormat);

    if (IsFieldEdit())
        bName = false;

    m_pFormat->Enable(bFormat || bNumFormat);
    m_pNameFT->Enable(bName);
    m_pNameED->Enable(bName);
    m_pValueFT->Enable(bValue);
    m_pValueED->Enable(bValue);

    m_pInvisibleCB->Show(!bShowChapterFrame);
    m_pChapterFrame->Show(bShowChapterFrame);
    m_pInvisibleCB->Enable(bInvisible);

    ModifyHdl(*m_pNameED);    // apply/insert/delete status update

    m_pNumFormatLB->SetUpdateMode(true);
    m_pFormatLB->SetUpdateMode(true);

    if(m_pSelectionLB->IsCallAddSelection())
    {
        nTypeId = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel())));

        switch (nTypeId)
        {
            case TYP_FORMELFLD:
                {
                    nSelPos = m_pSelectionLB->GetSelectedEntryPos();

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                    {
                        nSelData = static_cast<size_t>(reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nSelPos)));
                        if (nSelData != SIZE_MAX && pBox && !bInit)
                        {
                            m_pValueED->ReplaceSelected(m_pSelectionLB->GetSelectedEntry());
                            ModifyHdl(*m_pNameED);
                        }
                    }
                }
                break;
        }
        m_pSelectionLB->ResetCallAddSelection();
    }
}

// renew types in SelectionBox
void SwFieldVarPage::UpdateSubType()
{
    SetSelectionSel(m_pSelectionLB->GetSelectedEntryPos());

    OUString sOldSel;
    if (GetSelectionSel() != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = m_pSelectionLB->GetEntry(GetSelectionSel());

    // fill Selection-Listbox
    m_pSelectionLB->SetUpdateMode(false);
    m_pSelectionLB->Clear();

    const sal_uInt16 nTypeId = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel())));
    std::vector<OUString> aList;
    GetFieldMgr().GetSubTypes(nTypeId, aList);
    const size_t nCount = aList.size();
    for (size_t i = 0; i < nCount; ++i)
    {
        if (nTypeId != TYP_INPUTFLD || i)
        {
            if (!IsFieldEdit())
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
                        if (GetCurField() && aList[i] == static_cast<const SwFormulaField*>(GetCurField())->GetFormula())
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
                        if (GetCurField() != nullptr
                            && ((static_cast<SwRefPageSetField*>(GetCurField())->IsOn()
                                 && i) || (!static_cast<SwRefPageSetField*>(GetCurField())
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
    ListBox *pLB = nullptr;

    if (bEnable)
    {
        m_pSelectionLB->SelectEntry(sOldSel);
        if (!m_pSelectionLB->GetSelectedEntryCount())
        {
            m_pSelectionLB->SelectEntryPos(0);
            pLB = m_pSelectionLB;    // newly initialise all controls
        }
    }

    m_pSelection->Enable(bEnable);

    SubTypeHdl(pLB);
    m_pSelectionLB->SetUpdateMode(true);
}

void SwFieldVarPage::FillFormatLB(sal_uInt16 nTypeId)
{
    OUString sOldSel;
    const sal_Int32 nFormatSel = m_pFormatLB->GetSelectedEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = m_pFormatLB->GetEntry(nFormatSel);

    OUString sOldNumSel;
    sal_uInt32 nOldNumFormat = 0;
    sal_Int32 nNumFormatSel = m_pNumFormatLB->GetSelectedEntryPos();
    if (nNumFormatSel != LISTBOX_ENTRY_NOTFOUND)
    {
        sOldNumSel = m_pNumFormatLB->GetEntry(nNumFormatSel);
        nOldNumFormat = m_pNumFormatLB->GetFormat();
    }

    // fill Format-Listbox
    m_pFormatLB->Clear();
    m_pNumFormatLB->Clear();
    bool bSpecialFormat = false;

    if( TYP_GETREFPAGEFLD != nTypeId )
    {
        if (GetCurField() != nullptr && IsFieldEdit())
        {
            bSpecialFormat = GetCurField()->GetFormat() == NUMBERFORMAT_ENTRY_NOT_FOUND;

            if (!bSpecialFormat)
            {
                m_pNumFormatLB->SetDefFormat(GetCurField()->GetFormat());
                sOldNumSel.clear();
            }
            else if (nTypeId == TYP_GETFLD || nTypeId == TYP_FORMELFLD)
            {
                m_pNumFormatLB->SetFormatType(SvNumFormatType::NUMBER);
            }
        }
        else
        {
            if (nOldNumFormat && nOldNumFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
                m_pNumFormatLB->SetDefFormat(nOldNumFormat);
            else
                m_pNumFormatLB->SetFormatType(SvNumFormatType::NUMBER);
        }
    }

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            if (!IsFieldEdit() || bSpecialFormat)
            {
                sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SwResId(FMT_MARK_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, reinterpret_cast<void *>(NUMBERFORMAT_ENTRY_NOT_FOUND));
                nPos = m_pNumFormatLB->InsertEntry(SwResId(FMT_USERVAR_CMD), 1);
                m_pNumFormatLB->SetEntryData(nPos, reinterpret_cast<void *>(NUMBERFORMAT_ENTRY_NOT_FOUND));
            }
        }
        break;

        case TYP_SETFLD:
        {
            if (!IsFieldEdit() || bSpecialFormat)
            {
                sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SwResId(FMT_SETVAR_TEXT), 0);
                m_pNumFormatLB->SetEntryData(nPos, reinterpret_cast<void *>(NUMBERFORMAT_ENTRY_NOT_FOUND));
            }
        }
        break;

        case TYP_FORMELFLD:
        {
            sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SwResId(FMT_GETVAR_NAME), 0);
            m_pNumFormatLB->SetEntryData(nPos, reinterpret_cast<void *>(NUMBERFORMAT_ENTRY_NOT_FOUND));
        }
        break;

        case TYP_GETFLD:
        {
            sal_Int32 nPos = m_pNumFormatLB->InsertEntry(SwResId(FMT_GETVAR_NAME), 0);
            m_pNumFormatLB->SetEntryData(nPos, reinterpret_cast<void *>(NUMBERFORMAT_ENTRY_NOT_FOUND));
        }
        break;
    }

    if (IsFieldEdit() && bSpecialFormat)
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
        else if (nOldNumFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
            m_pNumFormatLB->SelectEntry(sOldSel);
    }

    const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(nTypeId, IsFieldDlgHtmlMode());

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        const sal_Int32 nPos = m_pFormatLB->InsertEntry(GetFieldMgr().GetFormatStr(nTypeId, i));
        const sal_uInt16 nFieldId = GetFieldMgr().GetFormatId( nTypeId, i );
        m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(nFieldId) );
        if (IsFieldEdit() && GetCurField() && nFieldId == GetCurField()->GetFormat())
            m_pFormatLB->SelectEntryPos( nPos );
    }

    if (nSize && (!IsFieldEdit() || !m_pFormatLB->GetSelectedEntryCount()))
    {
        m_pFormatLB->SelectEntry(sOldSel);

        if (!m_pFormatLB->GetSelectedEntryCount())
        {
            m_pFormatLB->SelectEntry(SwResId(FMT_NUM_PAGEDESC));
            if (!m_pFormatLB->GetSelectedEntryCount())
            {
                m_pFormatLB->SelectEntry(SwResId(FMT_NUM_ARABIC));
                if (!m_pFormatLB->GetSelectedEntryCount())
                    m_pFormatLB->SelectEntryPos(0);
            }
        }
    }
}

// Modify
IMPL_LINK_NOARG(SwFieldVarPage, ModifyHdl, Edit&, void)
{
    OUString sValue(m_pValueED->GetText());
    bool bHasValue = !sValue.isEmpty();
    const sal_uInt16 nTypeId = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel())));
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
            // is there already a corresponding type
            bInsert = bApply = true;

            SwFieldType* pType = GetFieldMgr().GetFieldType(SwFieldIds::Dde, sName);

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
            SwFieldType* pType = GetFieldMgr().GetFieldType(SwFieldIds::User, sName);

            SwWrtShell *pSh = GetWrtShell();
            if(!pSh)
                pSh = ::GetActiveWrtShell();
            if(pSh && pType)
                bDelete = !pSh->IsUsed( *pType );

            pType = GetFieldMgr().GetFieldType(SwFieldIds::SetExp, sName);
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
            SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(
                GetFieldMgr().GetFieldType(SwFieldIds::SetExp, sName));

            if (pFieldType)
            {

                SwWrtShell *pSh = GetWrtShell();
                if(!pSh)
                    pSh = ::GetActiveWrtShell();
                if(pSh)
                {
                    const SwFieldTypes* p = pSh->GetDoc()->getIDocumentFieldsAccess().GetFieldTypes();
                    sal_uInt16 i;

                    for (i = 0; i < INIT_FLDTYPES; i++)
                    {
                        SwFieldType* pType = (*p)[ i ];
                        if (pType == pFieldType)
                            break;
                    }

                    if (i >= INIT_FLDTYPES && !pSh->IsUsed(*pFieldType))
                        bDelete = true;

                    if (nTypeId == TYP_SEQFLD && !(pFieldType->GetType() & nsSwGetSetExpType::GSE_SEQ))
                        bInsert = false;

                    if (nTypeId == TYP_SETFLD && (pFieldType->GetType() & nsSwGetSetExpType::GSE_SEQ))
                        bInsert = false;
                }
            }
            if (GetFieldMgr().GetFieldType(SwFieldIds::User, sName))
                bInsert = false;
        }

        if (!nLen && (nTypeId == TYP_SETFLD || nTypeId == TYP_INPUTFLD ||
                        (!IsFieldEdit() && nTypeId == TYP_GETFLD ) ) )
            bInsert = false;

        if( (nTypeId == TYP_SETFLD || nTypeId == TYP_FORMELFLD) &&
            !bHasValue )
            bInsert = false;
        break;
    }

    m_pNewPB->Enable(bApply);
    m_pDelPB->Enable(bDelete);
    EnableInsert(bInsert);
}

IMPL_LINK( SwFieldVarPage, TBClickHdl, Button*, pBox, void )
{
    const sal_uInt16 nTypeId = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel())));

    if (pBox == m_pDelPB)
    {
        if( nTypeId == TYP_USERFLD )
            GetFieldMgr().RemoveFieldType(SwFieldIds::User, m_pSelectionLB->GetSelectedEntry());
        else
        {
            SwFieldIds nWhich;

            switch(nTypeId)
            {
                case TYP_SETFLD:
                case TYP_SEQFLD:
                    nWhich = SwFieldIds::SetExp;
                    break;
                default:
                    nWhich = SwFieldIds::Dde;
                    break;
            }

            GetFieldMgr().RemoveFieldType(nWhich, m_pSelectionLB->GetSelectedEntry());
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
    else if (pBox == m_pNewPB)
    {
        OUString sName(m_pNameED->GetText()), sValue(m_pValueED->GetText());
        SwFieldType* pType = nullptr;
        SwFieldIds nId = SwFieldIds::Database;
        sal_Int32 nNumFormatPos = m_pNumFormatLB->GetSelectedEntryPos();

        switch (nTypeId)
        {
            case TYP_USERFLD:   nId = SwFieldIds::User;  break;
            case TYP_DDEFLD:    nId = SwFieldIds::Dde;   break;
            case TYP_SETFLD:    nId = SwFieldIds::SetExp;break;
        }
        pType = GetFieldMgr().GetFieldType(nId, sName);

        sal_uLong nFormat = m_pFormatLB->GetSelectedEntryPos();
        if (nFormat != LISTBOX_ENTRY_NOTFOUND)
            nFormat = reinterpret_cast<sal_uLong>(m_pFormatLB->GetEntryData(static_cast<sal_Int32>(nFormat)));

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
                        sal_uLong nNumberFormat = nNumFormatPos == 0 ? 0 : m_pNumFormatLB->GetFormat();
                        if (nNumberFormat)
                        {   // Switch language to office-language because Kalkulator expects
                            // String in office format and it should be fed into dialog like
                            // that
                            nNumberFormat = SwValueField::GetSystemFormat(pSh->GetNumberFormatter(), nNumberFormat);
                        }
                        static_cast<SwUserFieldType*>(pType)->SetContent(m_pValueED->GetText(), nNumberFormat);
                        static_cast<SwUserFieldType*>(pType)->SetType(
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
                        sValue = sValue.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nTmpPos );
                        sValue = sValue.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nTmpPos );
                        static_cast<SwDDEFieldType*>(pType)->SetCmd(sValue);
                        static_cast<SwDDEFieldType*>(pType)->SetType(static_cast<SfxLinkUpdateMode>(nFormat));
                    }
                }
                pType->UpdateFields();

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
                        GetFieldMgr().InsertFieldType( aType ); // Userfld new
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
                    sValue = sValue.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nTmpPos );
                    sValue = sValue.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nTmpPos );

                    SwDDEFieldType aType(sName, sValue, static_cast<SfxLinkUpdateMode>(nFormat));
                    m_pSelectionLB->InsertEntry(sName);
                    m_pSelectionLB->SelectEntry(sName);
                    GetFieldMgr().InsertFieldType(aType);   // DDE-Field new
                }
            }
        }
        if (IsFieldEdit())
            GetFieldMgr().GetCurField();    // update FieldManager

        UpdateSubType();
    }
}

IMPL_LINK_NOARG(SwFieldVarPage, ChapterHdl, ListBox&, void)
{
    bool bEnable = m_pChapterLevelLB->GetSelectedEntryPos() != 0;

    m_pSeparatorED->Enable(bEnable);
    m_pSeparatorFT->Enable(bEnable);
    SeparatorHdl(*m_pSeparatorED);
}

IMPL_LINK_NOARG(SwFieldVarPage, SeparatorHdl, Edit&, void)
{
    bool bEnable = !m_pSeparatorED->GetText().isEmpty() ||
                    m_pChapterLevelLB->GetSelectedEntryPos() == 0;
    EnableInsert(bEnable);
}

bool SwFieldVarPage::FillItemSet(SfxItemSet* )
{
    const sal_uInt16 nTypeId = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel())));

    OUString aVal(m_pValueED->GetText());
    OUString aName(m_pNameED->GetText());

    const sal_Int32 nSubPos = m_pSelectionLB->GetSelectedEntryPos();
    sal_uInt16 nSubType = (nSubPos == LISTBOX_ENTRY_NOTFOUND) ? 0 :
        static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pSelectionLB->GetEntryData(nSubPos)));

    sal_uInt32 nFormat;

    if (!m_pNumFormatLB->IsVisible())
    {
        sal_Int32 nFormatPos = m_pFormatLB->GetSelectedEntryPos();

        if(nFormatPos == LISTBOX_ENTRY_NOTFOUND)
            nFormat = 0;
        else
            nFormat = static_cast<sal_uInt32>(reinterpret_cast<sal_uIntPtr>(m_pFormatLB->GetEntryData(nFormatPos)));
    }
    else
    {
        nFormat = m_pNumFormatLB->GetFormat();

        if (nFormat && nFormat != NUMBERFORMAT_ENTRY_NOT_FOUND && m_pNumFormatLB->IsAutomaticLanguage())
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
            nSubType = (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND) ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR;

            if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND && m_pNumFormatLB->GetSelectedEntry() == SwResId(FMT_USERVAR_CMD))
                nSubType |= nsSwExtendedSubType::SUB_CMD;

            if (m_pInvisibleCB->IsChecked())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_FORMELFLD:
        {
            nSubType = nsSwGetSetExpType::GSE_FORMULA;
            if (m_pNumFormatLB->IsVisible() && nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            break;
        }
        case TYP_GETFLD:
        {
            nSubType &= 0xff00;
            if (m_pNumFormatLB->IsVisible() && nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            break;
        }
        case TYP_INPUTFLD:
        {
            SwFieldType* pType = GetFieldMgr().GetFieldType(SwFieldIds::User, aName);
            nSubType = static_cast< sal_uInt16 >((nSubType & 0xff00) | (pType ? INP_USR : INP_VAR));
            break;
        }

        case TYP_SETFLD:
        {
            if (IsFieldDlgHtmlMode())
            {
                nSubType = 0x0100;
                nSubType = (nSubType & 0xff00) | nsSwGetSetExpType::GSE_STRING;
            }
            else
                nSubType = (nSubType & 0xff00) | ((nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND) ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR);

            if (m_pInvisibleCB->IsChecked())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_SEQFLD:
        {
            nSubType = static_cast< sal_uInt16 >(m_pChapterLevelLB->GetSelectedEntryPos());
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

    if (!IsFieldEdit() ||
        m_pNameED->IsValueChangedFromSaved() ||
        m_pValueED->IsValueChangedFromSaved() ||
        m_pSelectionLB->IsValueChangedFromSaved() ||
        m_pFormatLB->IsValueChangedFromSaved() ||
        nOldFormat != m_pNumFormatLB->GetFormat() ||
        m_pInvisibleCB->IsValueChangedFromSaved() ||
        m_pChapterLevelLB->IsValueChangedFromSaved() ||
        m_pSeparatorED->IsValueChangedFromSaved())
    {
        InsertField( nTypeId, nSubType, aName, aVal, nFormat,
                    cSeparator, m_pNumFormatLB->IsAutomaticLanguage() );
    }

    UpdateSubType();

    return false;
}

VclPtr<SfxTabPage> SwFieldVarPage::Create( TabPageParent pParent,
                                         const SfxItemSet *const pAttrSet)
{
    return VclPtr<SwFieldVarPage>::Create( pParent.pParent, pAttrSet );
}

sal_uInt16 SwFieldVarPage::GetGroup()
{
    return GRP_VAR;
}

SelectionListBox::SelectionListBox(vcl::Window* pParent, WinBits nStyle)
    : ListBox(pParent, nStyle)
    , bCallAddSelection(false)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSelectionListBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = BuilderUtils::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nBits |= WB_BORDER;

    VclPtrInstance<SelectionListBox> pListBox(pParent, nBits|WB_SIMPLEMODE);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

bool SelectionListBox::PreNotify( NotifyEvent& rNEvt )
{
    bool bHandled = ListBox::PreNotify( rNEvt );
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYUP )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
        const sal_uInt16 nModifier = aKeyCode.GetModifier();
        if( aKeyCode.GetCode() == KEY_SPACE && !nModifier)
            bCallAddSelection = true;
    }
    if ( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN )
    {
        const MouseEvent* pMEvt = rNEvt.GetMouseEvent();

        if (pMEvt && (pMEvt->IsMod1() || pMEvt->IsMod2()))  // Alt or Ctrl
            bCallAddSelection = true;
    }

    return bHandled;
}

void SwFieldVarPage::FillUserData()
{
    OUString sData(USER_DATA_VERSION);
    sData += ";";
    sal_Int32 nTypeSel = m_pTypeLB->GetSelectedEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData( nTypeSel )));
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
