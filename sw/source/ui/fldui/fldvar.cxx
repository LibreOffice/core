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

SwFieldVarPage::SwFieldVarPage(TabPageParent pParent, const SfxItemSet *const pCoreSet )
    : SwFieldPage(pParent, "modules/swriter/ui/fldvarpage.ui", "FieldVarPage", pCoreSet)
    , m_xTypeLB(m_xBuilder->weld_tree_view("type"))
    , m_xSelection(m_xBuilder->weld_widget("selectframe"))
    , m_xSelectionLB(m_xBuilder->weld_tree_view("select"))
    , m_xNameFT(m_xBuilder->weld_label("nameft"))
    , m_xNameED(m_xBuilder->weld_entry("name"))
    , m_xValueFT(m_xBuilder->weld_label("valueft"))
    , m_xValueED(new SwConditionEdit(m_xBuilder->weld_entry("value")))
    , m_xFormat(m_xBuilder->weld_widget("formatframe"))
    , m_xNumFormatLB(new SwNumFormatTreeView(m_xBuilder->weld_tree_view("numformat")))
    , m_xFormatLB(m_xBuilder->weld_tree_view("format"))
    , m_xChapterFrame(m_xBuilder->weld_widget("chapterframe"))
    , m_xChapterLevelLB(m_xBuilder->weld_tree_view("level"))
    , m_xInvisibleCB(m_xBuilder->weld_check_button("invisible"))
    , m_xSeparatorFT(m_xBuilder->weld_label("separatorft"))
    , m_xSeparatorED(m_xBuilder->weld_entry("separator"))
    , m_xNewPB(m_xBuilder->weld_button("apply"))
    , m_xDelPB(m_xBuilder->weld_button("delete"))
    , nOldFormat(0)
    , bInit(true)
{
    FillFieldSelect(*m_xTypeLB);
    m_xSelectionLB->make_sorted();
    FillFieldSelect(*m_xFormatLB);

    auto nWidth = LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MapUnit::MapAppFont)).Width();
    auto nHeight = m_xTypeLB->get_height_rows(20);
    m_xTypeLB->set_size_request(nWidth, nHeight);
    m_xSelectionLB->set_size_request(nWidth, nHeight);
    m_xFormatLB->set_size_request(nWidth, nHeight/2);

    sOldValueFT = m_xValueFT->get_label();
    sOldNameFT = m_xNameFT->get_label();

    for (sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        m_xChapterLevelLB->append_text(OUString::number(i));

    m_xChapterLevelLB->select(0);
    //enable 'active' language selection
    m_xNumFormatLB->SetShowLanguageControl(true);
}

SwFieldVarPage::~SwFieldVarPage()
{
    disposeOnce();
}

void SwFieldVarPage::Reset(const SfxItemSet* )
{
    SavePos(*m_xTypeLB);

    Init(); // general initialisation

    m_xTypeLB->freeze();
    m_xTypeLB->clear();

    sal_uInt16 nTypeId;

    if (!IsFieldEdit())
    {
        // initialise TypeListBox
        const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

        for (short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = SwFieldMgr::GetTypeId(i);
            m_xTypeLB->append(OUString::number(nTypeId), SwFieldMgr::GetTypeStr(i));
        }
    }
    else
    {
        const SwField* pCurField = GetCurField();
        assert(pCurField && "<SwFieldVarPage::Reset(..)> - <SwField> instance missing!");
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_SETINPFLD)
            nTypeId = TYP_INPUTFLD;
        m_xTypeLB->append(OUString::number(nTypeId), SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));
        m_xNumFormatLB->SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
            pSh = ::GetActiveWrtShell();
        if(pSh)
        {
            const SvNumberformat* pFormat = pSh->GetNumberFormatter()->GetEntry(pCurField->GetFormat());
            if(pFormat)
                m_xNumFormatLB->SetLanguage(pFormat->GetLanguage());
        }
    }

    m_xTypeLB->thaw();

    // select old Pos
    RestorePos(*m_xTypeLB);

    m_xTypeLB->connect_row_activated(LINK(this, SwFieldVarPage, TreeViewInsertHdl));
    m_xTypeLB->connect_changed(LINK(this, SwFieldVarPage, TypeHdl));
    m_xSelectionLB->connect_changed(LINK(this, SwFieldVarPage, SubTypeListBoxHdl));
    m_xSelectionLB->connect_row_activated(LINK(this, SwFieldVarPage, SubTypeInsertHdl));
    m_xFormatLB->connect_row_activated(LINK(this, SwFieldVarPage, TreeViewInsertHdl));
    m_xNumFormatLB->connect_row_activated(LINK(this, SwFieldVarPage, TreeViewInsertHdl));
    m_xNameED->connect_changed(LINK(this, SwFieldVarPage, ModifyHdl));
    m_xValueED->connect_changed(LINK(this, SwFieldVarPage, ModifyHdl));
    m_xNewPB->connect_clicked(LINK(this, SwFieldVarPage, TBClickHdl));
    m_xDelPB->connect_clicked(LINK(this, SwFieldVarPage, TBClickHdl));
    m_xChapterLevelLB->connect_changed(LINK(this, SwFieldVarPage, ChapterHdl));
    m_xSeparatorED->connect_changed(LINK(this, SwFieldVarPage, SeparatorHdl));

    if( !IsRefresh() )
    {
        OUString sUserData = GetUserData();
        sal_Int32 nIdx{ 0 };
        if(!IsRefresh() && sUserData.getToken(0, ';', nIdx).equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            OUString sVal = sUserData.getToken(0, ';', nIdx);
            sal_uInt16 nVal = static_cast<sal_uInt16>(sVal.toInt32());
            if (USHRT_MAX != nVal)
            {
                for (sal_Int32 i = 0, nEntryCount = m_xTypeLB->n_children(); i < nEntryCount; i++)
                {
                    if (nVal == m_xTypeLB->get_id(i).toUInt32())
                    {
                        m_xTypeLB->select(i);
                        break;
                    }
                }
            }
        }
    }
    TypeHdl(*m_xTypeLB);

    if (IsFieldEdit())
    {
        m_xSelectionLB->save_value();
        m_xFormatLB->save_value();
        nOldFormat = m_xNumFormatLB->GetFormat();
        m_xNameED->save_value();
        m_xValueED->save_value();
        m_xInvisibleCB->save_state();
        m_xChapterLevelLB->save_value();
        m_xSeparatorED->save_value();
    }
}

IMPL_LINK_NOARG(SwFieldVarPage, TypeHdl, weld::TreeView&, void)
{
    // save old ListBoxPos
    const sal_Int32 nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(m_xTypeLB->get_selected_index());

    if(GetTypeSel() == -1)
    {
        SetTypeSel(0);
        m_xTypeLB->select(0);
    }

    if (nOld != GetTypeSel() || nOld == -1)
    {
        bInit = true;
        if (nOld != -1)
        {
            m_xNameED->set_text(OUString());
            m_xValueED->set_text(OUString());
        }

        m_xValueED->SetDropEnable(false);
        UpdateSubType();    // initialise selection-listboxes
    }

    bInit = false;
}

IMPL_LINK( SwFieldVarPage, SubTypeListBoxHdl, weld::TreeView&, rBox, void )
{
    SubTypeHdl(&rBox);
}

void SwFieldVarPage::SubTypeHdl(const weld::TreeView* pBox)
{
    sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();
    sal_Int32 nSelPos = m_xSelectionLB->get_selected_index();
    size_t nSelData = SIZE_MAX;

    if (nSelPos != -1)
        nSelData = m_xSelectionLB->get_id(nSelPos).toUInt32();

    if (IsFieldEdit() && (!pBox || bInit))
    {
        if (nTypeId != TYP_FORMELFLD)
            m_xNameED->set_text(GetFieldMgr().GetCurFieldPar1());

        m_xValueED->set_text(GetFieldMgr().GetCurFieldPar2());
    }

    if (m_xNameFT->get_label() != sOldNameFT)
        m_xNameFT->set_label(sOldNameFT);
    if (m_xValueFT->get_label() != sOldValueFT)
        m_xValueFT->set_label(sOldValueFT);

    m_xFormatLB->freeze();
    FillFormatLB(nTypeId);

    sal_Int32 nSize = m_xFormatLB->n_children();

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
                        m_xNameED->set_text(pType->GetName());

                        if (pType->GetType() == UF_STRING)
                        {
                            m_xValueED->set_text(pType->GetContent());
                            m_xNumFormatLB->select(0);
                        }
                        else
                            m_xValueED->set_text(pType->GetContent());
                    }
                }
                else
                    m_xValueED->set_text(pType->GetContent());
            }
            else
            {
                if (pBox)   // only when interacting via mouse
                {
                    m_xNameED->set_text(OUString());
                    m_xValueED->set_text(OUString());
                }
            }
            bValue = bName = bNumFormat = bInvisible = true;

            m_xValueED->SetDropEnable(true);
            break;
        }

        case TYP_SETFLD:
            bValue = true;

            bNumFormat = bInvisible = true;

            if (!IsFieldDlgHtmlMode())
                bName = true;
            else
            {
                m_xNumFormatLB->clear();
                m_xNumFormatLB->append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_SETVAR_TEXT));
                m_xNumFormatLB->select(0);
            }
            // is there a corresponding SetField
            if (IsFieldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != -1)
                {
                    OUString sName(m_xSelectionLB->get_selected_text());
                    m_xNameED->set_text(sName);

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
                                m_xNumFormatLB->select(0); // textual
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
                m_xValueED->set_text(static_cast<SwSetExpField*>(GetCurField())->GetFormula());
            }
            m_xValueED->SetDropEnable(true);
            break;

        case TYP_FORMELFLD:
            {
                bValue = true;
                bNumFormat = true;
                m_xValueFT->set_label(SwResId(STR_FORMULA));
                m_xValueED->SetDropEnable(true);
            }
            break;

        case TYP_GETFLD:
            {
                if (!IsFieldEdit())
                {
                    m_xNameED->set_text(OUString());
                    m_xValueED->set_text(OUString());
                }

                if (nSelPos != -1)
                {
                    OUString sName(m_xSelectionLB->get_selected_text());
                    if (!IsFieldEdit())
                        m_xNameED->set_text(sName);

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
            m_xValueFT->set_label(SwResId(STR_PROMPT));

            if (nSelPos != -1)
            {
                bValue = bNumFormat = true;

                OUString sName;

                sName = m_xSelectionLB->get_selected_text();
                m_xNameED->set_text( sName );

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
                            m_xNumFormatLB->clear();
                            m_xNumFormatLB->append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_USERVAR_TEXT));
                            m_xNumFormatLB->select(0);
                        }
                    }
                    if (GetCurField() && IsFieldEdit() && (!pBox || bInit) )
                        m_xValueED->set_text(static_cast<SwSetExpField*>(GetCurField())->GetPromptText());
                }
                else    // USERFLD
                    bFormat = bNumFormat = false;
            }
            break;

        case TYP_DDEFLD:
            m_xValueFT->set_label(SwResId(STR_DDE_CMD));

            if (IsFieldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != -1)
                {
                    SwDDEFieldType* pType =
                        static_cast<SwDDEFieldType*>( GetFieldMgr().GetFieldType(SwFieldIds::Dde, nSelData) );

                    if(pType)
                    {
                        m_xNameED->set_text(pType->GetName());

                        //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
                        //              That's not considered here yet
                        OUString sCmd( pType->GetCmd() );
                        sal_Int32 nTmpPos = 0;
                        sCmd = sCmd.replaceFirst( OUStringLiteral1(sfx2::cTokenSeparator), " ", &nTmpPos );
                        sCmd = sCmd.replaceFirst( OUStringLiteral1(sfx2::cTokenSeparator), " ", &nTmpPos );

                        m_xValueED->set_text( sCmd );
                        m_xFormatLB->select(static_cast<int>(pType->GetType()));
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
                    OUString sFieldTypeName(m_xSelectionLB->get_text(nSelPos));
                    if( !sFieldTypeName.isEmpty() )
                        pFieldTyp = GetFieldMgr().GetFieldType( SwFieldIds::SetExp,
                                                          sFieldTypeName );
                    else
                        pFieldTyp = nullptr;
                }

                if( GetCurField() && IsFieldEdit() )
                    m_xValueED->set_text( static_cast<SwSetExpField*>(GetCurField())->
                                        GetFormula() );

                if( IsFieldEdit() || pBox )   // only when interacting via mouse
                    m_xNameED->set_text( m_xSelectionLB->get_selected_text() );

                if( pFieldTyp )
                {
                    sal_uInt8 nLevel = static_cast<SwSetExpFieldType*>(pFieldTyp)->GetOutlineLvl();
                    if( 0x7f == nLevel )
                        m_xChapterLevelLB->select( 0 );
                    else
                        m_xChapterLevelLB->select( nLevel + 1 );
                    OUString sDelim = static_cast<SwSetExpFieldType*>(pFieldTyp)->GetDelimiter();
                    m_xSeparatorED->set_text( sDelim );
                    ChapterHdl(*m_xChapterLevelLB);
                }
            }
            break;

        case TYP_SETREFPAGEFLD:
            {
                bValue = false;
                m_xValueFT->set_label( SwResId( STR_OFFSET ));

                if (IsFieldEdit() || pBox)    // only when interacting via mouse
                    m_xNameED->set_text(OUString());

                if (nSelData != 0 && nSelData != SIZE_MAX)
                {
                    bValue = true;      // SubType OFF - knows no Offset
                    if (GetCurField() && IsFieldEdit())
                        m_xValueED->set_text(OUString::number(static_cast<SwRefPageSetField*>(GetCurField())->GetOffset()));
                }
            }
            break;

        case TYP_GETREFPAGEFLD:
            m_xNameED->set_text(OUString());
            m_xValueED->set_text(OUString());
            break;
    }

    m_xNumFormatLB->set_visible(bNumFormat);
    m_xFormatLB->set_visible(!bNumFormat);

    if (IsFieldEdit())
        bName = false;

    m_xFormat->set_sensitive(bFormat || bNumFormat);
    m_xNameFT->set_sensitive(bName);
    m_xNameED->set_sensitive(bName);
    m_xValueFT->set_sensitive(bValue);
    m_xValueED->set_sensitive(bValue);

    m_xInvisibleCB->set_visible(!bShowChapterFrame);
    m_xChapterFrame->set_visible(bShowChapterFrame);
    m_xInvisibleCB->set_sensitive(bInvisible);

    ModifyHdl(*m_xNameED);    // apply/insert/delete status update

    m_xFormatLB->thaw();
}

IMPL_LINK(SwFieldVarPage, SubTypeInsertHdl, weld::TreeView&, rBox, void)
{
    if (!bInit)
    {
        sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();
        if (nTypeId == TYP_FORMELFLD)
        {
            auto nSelPos = m_xSelectionLB->get_selected_index();
            if (nSelPos != -1)
            {
                m_xValueED->replace_selection(m_xSelectionLB->get_text(nSelPos));
                ModifyHdl(*m_xNameED);
                return;
            }
        }
    }
    TreeViewInsertHdl(rBox);
}

// renew types in SelectionBox
void SwFieldVarPage::UpdateSubType()
{
    SetSelectionSel(m_xSelectionLB->get_selected_index());

    OUString sOldSel;
    if (GetSelectionSel() != -1)
        sOldSel = m_xSelectionLB->get_text(GetSelectionSel());

    // fill Selection-Listbox
    m_xSelectionLB->freeze();
    m_xSelectionLB->clear();

    const sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();
    std::vector<OUString> aList;
    GetFieldMgr().GetSubTypes(nTypeId, aList);
    const size_t nCount = aList.size();
    for (size_t i = 0; i < nCount; ++i)
    {
        if (nTypeId != TYP_INPUTFLD || i)
        {
            if (!IsFieldEdit())
            {
                m_xSelectionLB->append(OUString::number(i), aList[i]);
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
                                m_xInvisibleCB->set_active(true);
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
                        m_xSelectionLB->append(OUString::number(i), aList[i]);
                        break;
                    }
                    default:
                        if (GetCurField() && aList[i] == GetCurField()->GetPar1())
                            bInsert = true;
                        break;
                }

                if (bInsert)
                {
                    m_xSelectionLB->append(OUString::number(i), aList[i]);
                    if (nTypeId != TYP_FORMELFLD)
                        break;
                }
            }
        }
    }

    m_xSelectionLB->thaw();

    const bool bEnable = m_xSelectionLB->n_children() != 0;
    weld::TreeView* pLB = nullptr;

    if (bEnable)
    {
        int nIndex = m_xSelectionLB->find_text(sOldSel);
        if (nIndex != -1)
            m_xSelectionLB->select(nIndex);
        else
        {
            m_xSelectionLB->select(0);
            pLB = m_xSelectionLB.get();    // newly initialise all controls
        }
    }

    m_xSelection->set_sensitive(bEnable);

    SubTypeHdl(pLB);
}

void SwFieldVarPage::FillFormatLB(sal_uInt16 nTypeId)
{
    OUString sOldSel;
    const sal_Int32 nFormatSel = m_xFormatLB->get_selected_index();
    if (nFormatSel != -1)
        sOldSel = m_xFormatLB->get_text(nFormatSel);

    weld::TreeView& rWidget = dynamic_cast<weld::TreeView&>(m_xNumFormatLB->get_widget());

    OUString sOldNumSel;
    sal_uInt32 nOldNumFormat = 0;
    sal_Int32 nNumFormatSel = rWidget.get_selected_index();
    if (nNumFormatSel != -1)
    {
        sOldNumSel = rWidget.get_text(nNumFormatSel);
        nOldNumFormat = m_xNumFormatLB->GetFormat();
    }

    // fill Format-Listbox
    m_xFormatLB->clear();
    rWidget.clear();
    bool bSpecialFormat = false;

    if( TYP_GETREFPAGEFLD != nTypeId )
    {
        if (GetCurField() != nullptr && IsFieldEdit())
        {
            bSpecialFormat = GetCurField()->GetFormat() == NUMBERFORMAT_ENTRY_NOT_FOUND;

            if (!bSpecialFormat)
            {
                m_xNumFormatLB->SetDefFormat(GetCurField()->GetFormat());
                sOldNumSel.clear();
            }
            else if (nTypeId == TYP_GETFLD || nTypeId == TYP_FORMELFLD)
            {
                m_xNumFormatLB->SetFormatType(SvNumFormatType::NUMBER);
            }
        }
        else
        {
            if (nOldNumFormat && nOldNumFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
                m_xNumFormatLB->SetDefFormat(nOldNumFormat);
            else
                m_xNumFormatLB->SetFormatType(SvNumFormatType::NUMBER);
        }
    }

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            if (!IsFieldEdit() || bSpecialFormat)
            {
                rWidget.append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_MARK_TEXT));
                rWidget.append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_USERVAR_CMD));
            }
        }
        break;

        case TYP_SETFLD:
        {
            if (!IsFieldEdit() || bSpecialFormat)
            {
                rWidget.append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_SETVAR_TEXT));
            }
        }
        break;

        case TYP_FORMELFLD:
        {
            rWidget.append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_GETVAR_NAME));
        }
        break;

        case TYP_GETFLD:
        {
            rWidget.append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_GETVAR_NAME));
        }
        break;
    }

    if (IsFieldEdit() && bSpecialFormat)
    {
        if (nTypeId == TYP_USERFLD && (GetCurField()->GetSubType() & nsSwExtendedSubType::SUB_CMD))
            rWidget.select(1);
        else
            rWidget.select(0);
    }
    else
    {
        if (!nOldNumFormat && (nNumFormatSel = rWidget.find_text(sOldNumSel)) != -1)
            rWidget.select(nNumFormatSel);
        else if (nOldNumFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
            rWidget.select_text(sOldSel);
    }

    const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(nTypeId, IsFieldDlgHtmlMode());

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        const sal_uInt16 nFieldId = GetFieldMgr().GetFormatId( nTypeId, i );
        OUString sId(OUString::number(nFieldId));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr(nTypeId, i));
        if (IsFieldEdit() && GetCurField() && nFieldId == GetCurField()->GetFormat())
            m_xFormatLB->select_id(sId);
    }

    if (nSize && (!IsFieldEdit() || m_xFormatLB->get_selected_index() == -1))
    {
        int nIndex = m_xFormatLB->find_text(sOldSel);
        if (nIndex == -1)
            nIndex = m_xFormatLB->find_text(SwResId(FMT_NUM_PAGEDESC));
        if (nIndex == -1)
            nIndex = m_xFormatLB->find_text(SwResId(FMT_NUM_ARABIC));
        if (nIndex == -1)
            nIndex = 0;
        m_xFormatLB->select(nIndex);
    }
}

// Modify
IMPL_LINK_NOARG(SwFieldVarPage, ModifyHdl, weld::Entry&, void)
{
    OUString sValue(m_xValueED->get_text());
    bool bHasValue = !sValue.isEmpty();
    const sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();
    bool bInsert = false, bApply = false, bDelete = false;

    OUString sName( m_xNameED->get_text() );
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
            int nStartPos, nEndPos;
            m_xNameED->get_selection_bounds(nStartPos, nEndPos);
            m_xNameED->set_text( sName );
            m_xNameED->select_region(nStartPos, nEndPos); // restore Cursorpos
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

    m_xNewPB->set_sensitive(bApply);
    m_xDelPB->set_sensitive(bDelete);
    EnableInsert(bInsert);
}

IMPL_LINK(SwFieldVarPage, TBClickHdl, weld::Button&, rBox, void)
{
    const sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    if (&rBox == m_xDelPB.get())
    {
        if( nTypeId == TYP_USERFLD )
            GetFieldMgr().RemoveFieldType(SwFieldIds::User, m_xSelectionLB->get_selected_text());
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

            GetFieldMgr().RemoveFieldType(nWhich, m_xSelectionLB->get_selected_text());
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
    else if (&rBox == m_xNewPB.get())
    {
        OUString sName(m_xNameED->get_text()), sValue(m_xValueED->get_text());
        SwFieldType* pType = nullptr;
        SwFieldIds nId = SwFieldIds::Database;
        sal_Int32 nNumFormatPos = m_xNumFormatLB->get_selected_index();

        switch (nTypeId)
        {
            case TYP_USERFLD:   nId = SwFieldIds::User;  break;
            case TYP_DDEFLD:    nId = SwFieldIds::Dde;   break;
            case TYP_SETFLD:    nId = SwFieldIds::SetExp;break;
        }
        pType = GetFieldMgr().GetFieldType(nId, sName);

        int nFormat = m_xFormatLB->get_selected_index();
        if (nFormat != -1)
            nFormat = m_xFormatLB->get_id(nFormat).toUInt32();

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
                    if (nNumFormatPos != -1)
                    {
                        sal_uLong nNumberFormat = nNumFormatPos == 0 ? 0 : m_xNumFormatLB->GetFormat();
                        if (nNumberFormat)
                        {   // Switch language to office-language because Kalkulator expects
                            // String in office format and it should be fed into dialog like
                            // that
                            nNumberFormat = SwValueField::GetSystemFormat(pSh->GetNumberFormatter(), nNumberFormat);
                        }
                        static_cast<SwUserFieldType*>(pType)->SetContent(m_xValueED->get_text(), nNumberFormat);
                        static_cast<SwUserFieldType*>(pType)->SetType(
                            nNumFormatPos == 0 ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR );
                    }
                }
                else
                {
                    if (nFormat != -1)
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

                    if (nNumFormatPos != -1)
                    {
                        aType.SetType(nNumFormatPos == 0 ? nsSwGetSetExpType::GSE_STRING : nsSwGetSetExpType::GSE_EXPR);
                        aType.SetContent( sValue, nNumFormatPos == 0 ? 0 : m_xNumFormatLB->GetFormat() );
                        m_xSelectionLB->append_text(sName);
                        m_xSelectionLB->select_text(sName);
                        GetFieldMgr().InsertFieldType( aType ); // Userfld new
                    }
                }
            }
            else
            {
                if (nFormat != -1)
                {
                    // DDE-Topics/-Items can have blanks in their names!
                    //  That's not being considered here yet.
                    sal_Int32 nTmpPos = 0;
                    sValue = sValue.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nTmpPos );
                    sValue = sValue.replaceFirst( " ", OUStringLiteral1(sfx2::cTokenSeparator), &nTmpPos );

                    SwDDEFieldType aType(sName, sValue, static_cast<SfxLinkUpdateMode>(nFormat));
                    m_xSelectionLB->append_text(sName);
                    m_xSelectionLB->select_text(sName);
                    GetFieldMgr().InsertFieldType(aType);   // DDE-Field new
                }
            }
        }
        if (IsFieldEdit())
            GetFieldMgr().GetCurField();    // update FieldManager

        UpdateSubType();
    }
}

IMPL_LINK_NOARG(SwFieldVarPage, ChapterHdl, weld::TreeView&, void)
{
    bool bEnable = m_xChapterLevelLB->get_selected_index() != 0;

    m_xSeparatorED->set_sensitive(bEnable);
    m_xSeparatorFT->set_sensitive(bEnable);
    SeparatorHdl(*m_xSeparatorED);
}

IMPL_LINK_NOARG(SwFieldVarPage, SeparatorHdl, weld::Entry&, void)
{
    bool bEnable = !m_xSeparatorED->get_text().isEmpty() ||
                    m_xChapterLevelLB->get_selected_index() == 0;
    EnableInsert(bEnable);
}

bool SwFieldVarPage::FillItemSet(SfxItemSet* )
{
    const sal_uInt16 nTypeId = m_xTypeLB->get_id(GetTypeSel()).toUInt32();

    OUString aVal(m_xValueED->get_text());
    OUString aName(m_xNameED->get_text());

    const sal_Int32 nSubPos = m_xSelectionLB->get_selected_index();
    sal_uInt16 nSubType = (nSubPos == -1) ? 0 : m_xSelectionLB->get_id(nSubPos).toUInt32();

    sal_uInt32 nFormat;

    if (!m_xNumFormatLB->get_visible())
    {
        sal_Int32 nFormatPos = m_xFormatLB->get_selected_index();

        if(nFormatPos == -1)
            nFormat = 0;
        else
            nFormat = m_xFormatLB->get_id(nFormatPos).toUInt32();
    }
    else
    {
        nFormat = m_xNumFormatLB->GetFormat();

        if (nFormat && nFormat != NUMBERFORMAT_ENTRY_NOT_FOUND && m_xNumFormatLB->IsAutomaticLanguage())
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

            if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND && m_xNumFormatLB->get_selected_text() == SwResId(FMT_USERVAR_CMD))
                nSubType |= nsSwExtendedSubType::SUB_CMD;

            if (m_xInvisibleCB->get_active())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_FORMELFLD:
        {
            nSubType = nsSwGetSetExpType::GSE_FORMULA;
            if (m_xNumFormatLB->get_visible() && nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            break;
        }
        case TYP_GETFLD:
        {
            nSubType &= 0xff00;
            if (m_xNumFormatLB->get_visible() && nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
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

            if (m_xInvisibleCB->get_active())
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            break;
        }
        case TYP_SEQFLD:
        {
            nSubType = static_cast< sal_uInt16 >(m_xChapterLevelLB->get_selected_index());
            if (nSubType == 0)
                nSubType = 0x7f;
            else
            {
                nSubType--;
                OUString sSeparator = OUString(m_xSeparatorED->get_text()[0]);
                cSeparator = !sSeparator.isEmpty() ? sSeparator[0] : ' ';
            }
            break;
        }
        case TYP_GETREFPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
                aVal = m_xValueED->get_text();
            break;
    }

    if (!IsFieldEdit() ||
        m_xNameED->get_value_changed_from_saved() ||
        m_xValueED->get_value_changed_from_saved() ||
        m_xSelectionLB->get_value_changed_from_saved() ||
        m_xFormatLB->get_value_changed_from_saved() ||
        nOldFormat != m_xNumFormatLB->GetFormat() ||
        m_xInvisibleCB->get_state_changed_from_saved() ||
        m_xChapterLevelLB->get_value_changed_from_saved() ||
        m_xSeparatorED->get_value_changed_from_saved())
    {
        InsertField( nTypeId, nSubType, aName, aVal, nFormat,
                    cSeparator, m_xNumFormatLB->IsAutomaticLanguage() );
    }

    UpdateSubType();

    return false;
}

VclPtr<SfxTabPage> SwFieldVarPage::Create( TabPageParent pParent,
                                         const SfxItemSet *const pAttrSet)
{
    return VclPtr<SwFieldVarPage>::Create( pParent, pAttrSet );
}

sal_uInt16 SwFieldVarPage::GetGroup()
{
    return GRP_VAR;
}

void SwFieldVarPage::FillUserData()
{
    OUString sData(USER_DATA_VERSION);
    sData += ";";
    sal_Int32 nTypeSel = m_xTypeLB->get_selected_index();
    if( -1 == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = m_xTypeLB->get_id(nTypeSel).toUInt32();
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
