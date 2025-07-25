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
#include <wrtsh.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <swmodule.hxx>
#include "fldvar.hxx"
#include "flddinf.hxx"
#include <calc.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <o3tl/string_view.hxx>
#include <strings.hrc>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

SwFieldVarPage::SwFieldVarPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *const pCoreSet )
    : SwFieldPage(pPage, pController, u"modules/swriter/ui/fldvarpage.ui"_ustr, u"FieldVarPage"_ustr, pCoreSet)
    , m_xTypeLB(m_xBuilder->weld_tree_view(u"type"_ustr))
    , m_xSelection(m_xBuilder->weld_widget(u"selectframe"_ustr))
    , m_xSelectionLB(m_xBuilder->weld_tree_view(u"select"_ustr))
    , m_xNameFT(m_xBuilder->weld_label(u"nameft"_ustr))
    , m_xNameED(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xValueFT(m_xBuilder->weld_label(u"valueft"_ustr))
    , m_xValueED(new ConditionEdit<weld::TextView>(m_xBuilder->weld_text_view(u"value"_ustr)))
    , m_xFormat(m_xBuilder->weld_widget(u"formatframe"_ustr))
    , m_xNumFormatLB(new SwNumFormatTreeView(m_xBuilder->weld_tree_view(u"numformat"_ustr)))
    , m_xFormatLB(m_xBuilder->weld_tree_view(u"format"_ustr))
    , m_xChapterFrame(m_xBuilder->weld_widget(u"chapterframe"_ustr))
    , m_xChapterLevelLB(m_xBuilder->weld_combo_box(u"level"_ustr))
    , m_xInvisibleCB(m_xBuilder->weld_check_button(u"invisible"_ustr))
    , m_xSeparatorFT(m_xBuilder->weld_label(u"separatorft"_ustr))
    , m_xSeparatorED(m_xBuilder->weld_entry(u"separator"_ustr))
    , m_xNewPB(m_xBuilder->weld_button(u"apply"_ustr))
    , m_xDelPB(m_xBuilder->weld_button(u"delete"_ustr))
    , m_nOldFormat(0)
    , m_bInit(true)
{
    FillFieldSelect(*m_xTypeLB);
    m_xSelectionLB->make_sorted();
    FillFieldSelect(*m_xFormatLB);

    auto nWidth = m_xTypeLB->get_approximate_digit_width() * FIELD_COLUMN_WIDTH;
    auto nHeight = m_xTypeLB->get_height_rows(10);
    m_xTypeLB->set_size_request(nWidth, nHeight);
    m_xSelectionLB->set_size_request(nWidth, nHeight);
    m_xFormatLB->set_size_request(nWidth, nHeight/2);
    m_xNumFormatLB->get_widget().set_size_request(nWidth, nHeight/2);

    m_xValueED->get_widget().set_size_request(m_xValueED->get_widget().get_preferred_size().Width(),
                                   m_xValueED->get_widget().get_height_rows(6));

    m_sOldValueFT = m_xValueFT->get_label();
    m_sOldNameFT = m_xNameFT->get_label();

    for (sal_uInt16 i = 1; i <= MAXLEVEL; i++)
        m_xChapterLevelLB->append_text(OUString::number(i));

    m_xChapterLevelLB->set_active(0);
    //enable 'active' language selection
    m_xNumFormatLB->SetShowLanguageControl(true);

    // uitests
    m_xTypeLB->set_buildable_name(m_xTypeLB->get_buildable_name() + "-var");
    m_xNameED->set_buildable_name(m_xNameED->get_buildable_name() + "-var");
    m_xValueED->set_buildable_name(m_xValueED->get_buildable_name() + "-var");
    m_xNumFormatLB->set_buildable_name(m_xNumFormatLB->get_buildable_name() + "-var");
    m_xSelectionLB->set_buildable_name(m_xSelectionLB->get_buildable_name() + "-var");
    m_xFormatLB->set_buildable_name(m_xFormatLB->get_buildable_name() + "-var");
}

SwFieldVarPage::~SwFieldVarPage()
{
}

void SwFieldVarPage::Reset(const SfxItemSet* )
{
    SavePos(*m_xTypeLB);

    Init(); // general initialisation

    m_xTypeLB->freeze();
    m_xTypeLB->clear();

    SwFieldTypesEnum nTypeId;

    if (!IsFieldEdit())
    {
        // initialise TypeListBox
        const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

        for (short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = SwFieldMgr::GetTypeId(i);
            m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(i));
        }
    }
    else
    {
        auto nWidth = m_xTypeLB->get_approximate_digit_width() * FIELD_COLUMN_WIDTH;
        auto nHeight = m_xTypeLB->get_height_rows(8);
        m_xTypeLB->set_size_request(nWidth, nHeight);
        m_xSelectionLB->set_size_request(nWidth, nHeight);
        m_xFormatLB->set_size_request(nWidth, nHeight/2);
        auto size = m_xValueED->get_widget().get_size_request();
        m_xValueED->get_widget().set_size_request(size.getWidth(),
                                                  m_xValueED->get_widget().get_height_rows(6));
        const SwField* pCurField = GetCurField();
        assert(pCurField && "<SwFieldVarPage::Reset(..)> - <SwField> instance missing!");
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == SwFieldTypesEnum::SetInput)
            nTypeId = SwFieldTypesEnum::Input;
        m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));
        m_xNumFormatLB->SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell *pSh = GetWrtShell();
        if(!pSh)
            pSh = ::GetActiveWrtShell();
        if(pSh)
        {
            const SvNumberformat* pFormat = pSh->GetNumberFormatter()->GetEntry(pCurField->GetUntypedFormat());
            if(pFormat)
                m_xNumFormatLB->SetLanguage(pFormat->GetLanguage());
        }
    }

    m_xTypeLB->thaw();

    // select old Pos
    RestorePos(*m_xTypeLB);

    m_xTypeLB->connect_row_activated(LINK(this, SwFieldVarPage, TreeViewInsertHdl));
    m_xTypeLB->connect_selection_changed(LINK(this, SwFieldVarPage, TypeHdl));
    m_xSelectionLB->connect_selection_changed(LINK(this, SwFieldVarPage, SubTypeListBoxHdl));
    m_xSelectionLB->connect_row_activated(LINK(this, SwFieldVarPage, SubTypeInsertHdl));
    m_xFormatLB->connect_row_activated(LINK(this, SwFieldVarPage, TreeViewInsertHdl));
    m_xNumFormatLB->connect_row_activated(LINK(this, SwFieldVarPage, TreeViewInsertHdl));
    m_xNameED->connect_changed(LINK(this, SwFieldVarPage, ModifyHdl));
    m_xNewPB->connect_clicked(LINK(this, SwFieldVarPage, TBClickHdl));
    m_xDelPB->connect_clicked(LINK(this, SwFieldVarPage, TBClickHdl));
    m_xChapterLevelLB->connect_changed(LINK(this, SwFieldVarPage, ChapterHdl));
    m_xSeparatorED->connect_changed(LINK(this, SwFieldVarPage, SeparatorHdl));

    if( !IsRefresh() )
    {
        OUString sUserData = GetUserData();
        sal_Int32 nIdx{ 0 };
        if(!IsRefresh() && o3tl::equalsIgnoreAsciiCase(o3tl::getToken(sUserData, 0, ';', nIdx), u"" USER_DATA_VERSION_1))
        {
            std::u16string_view sVal = o3tl::getToken(sUserData, 0, ';', nIdx);
            sal_uInt16 nVal = o3tl::narrowing<sal_uInt16>(o3tl::toInt32(sVal));
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
        m_nOldFormat = m_xNumFormatLB->GetFormat();
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
        m_bInit = true;
        if (nOld != -1)
        {
            m_xNameED->set_text(OUString());
            m_xValueED->set_text(OUString());
        }

        m_xValueED->SetDropEnable(false);
        UpdateSubType();    // initialise selection-listboxes
    }

    m_bInit = false;
}

IMPL_LINK( SwFieldVarPage, SubTypeListBoxHdl, weld::TreeView&, rBox, void )
{
    SubTypeHdl(&rBox);
}

static inline sal_uInt32 lcl_getUsedNumFormat( const SwNumFormatTreeView& rNumFormatLB, bool& rbText )
{
    sal_uInt32 nNumberFormat = 0;
    const sal_Int32 nNumFormatPos = rNumFormatLB.get_selected_index();
    if (nNumFormatPos != -1)
    {
        nNumberFormat = rNumFormatLB.GetFormat();
        if ((rbText = (nNumFormatPos == 0 && nNumberFormat == SAL_MAX_UINT32)))
            nNumberFormat = 0;
    }
    return nNumberFormat;
}

void SwFieldVarPage::SubTypeHdl(const weld::TreeView* pBox)
{
    SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());
    sal_Int32 nSelPos = m_xSelectionLB->get_selected_index();
    size_t nSelData = SIZE_MAX;

    if (nSelPos != -1)
        nSelData = m_xSelectionLB->get_id(nSelPos).toUInt32();

    if (IsFieldEdit() && (!pBox || m_bInit))
    {
        if (nTypeId != SwFieldTypesEnum::Formel)
            m_xNameED->set_text(GetFieldMgr().GetCurFieldPar1());

        m_xValueED->set_text(GetFieldMgr().GetCurFieldPar2());
    }

    if (m_xNameFT->get_label() != m_sOldNameFT)
        m_xNameFT->set_label(m_sOldNameFT);
    if (m_xValueFT->get_label() != m_sOldValueFT)
        m_xValueFT->set_label(m_sOldValueFT);

    FillFormatLB(nTypeId);

    sal_Int32 nSize = m_xFormatLB->n_children();

    bool bValue = false, bName = false, bNumFormat = false,
            bInvisible = false, bShowChapterFrame = false;
    bool bFormat = nSize != 0;

    switch (nTypeId)
    {
        case SwFieldTypesEnum::User:
        {
            // change or create user type
            SwUserFieldType* pType = static_cast<SwUserFieldType*>(
                GetFieldMgr().GetFieldType(SwFieldIds::User, nSelData));

            if (pType)
            {
                if (!IsFieldEdit())
                {
                    if (pBox || (m_bInit && !IsRefresh()))    // only when interacting via mouse
                    {
                        m_xNameED->set_text(pType->GetName().toString());

                        if (pType->GetType() == SwUserType::String)
                        {
                            m_xValueED->set_text(pType->GetContent());
                            m_xNumFormatLB->select(0);
                        }
                        else
                        {
                            bool bText = false;
                            const sal_uInt32 nNumberFormat = lcl_getUsedNumFormat( *m_xNumFormatLB, bText);
                            m_xValueED->set_text(pType->GetInputOrDateTime(nNumberFormat));
                        }
                    }
                }
                else
                {
                    bool bText = false;
                    const sal_uInt32 nNumberFormat = lcl_getUsedNumFormat( *m_xNumFormatLB, bText);
                    m_xValueED->set_text(pType->GetInputOrDateTime(nNumberFormat));
                }
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

        case SwFieldTypesEnum::Set:
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

                            if (pSetTyp && pSetTyp->GetType() == SwGetSetExpType::String)
                                m_xNumFormatLB->select(0); // textual
                        }
                    }
                }
            }
            if (GetCurField() != nullptr && IsFieldEdit())
            {
                m_xValueED->set_text(static_cast<SwSetExpField*>(GetCurField())->GetInputOrDateTime());
            }
            m_xValueED->SetDropEnable(true);
            break;

        case SwFieldTypesEnum::Formel:
            {
                bValue = true;
                bNumFormat = true;
                m_xValueFT->set_label(SwResId(STR_FORMULA));
                m_xValueED->SetDropEnable(true);
            }
            break;

        case SwFieldTypesEnum::Get:
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
                            if (pSetTyp->GetType() & SwGetSetExpType::String)    // textual?
                                bFormat = true;
                            else                    // numeric
                                bNumFormat = true;
                        }
                    }
                }
                else
                    bFormat = false;

                EnableInsert(bFormat || bNumFormat, IsCurrentPage());
            }
            break;

        case SwFieldTypesEnum::Input:
            m_xValueFT->set_label(SwResId(STR_PROMPT));

            if (nSelPos != -1)
            {
                bValue = bNumFormat = true;

                OUString sName = m_xSelectionLB->get_selected_text();
                m_xNameED->set_text( sName );

                // User- or SetField ?
                if (!GetFieldMgr().GetFieldType(SwFieldIds::User, sName)) // SetExp
                {
                    // is there a corresponding SetField
                    SwSetExpFieldType* pSetTyp = static_cast<SwSetExpFieldType*>(
                                GetFieldMgr().GetFieldType(SwFieldIds::SetExp, sName));

                    if(pSetTyp)
                    {
                        if (pSetTyp->GetType() == SwGetSetExpType::String)    // textual?
                        {
                            m_xNumFormatLB->clear();
                            m_xNumFormatLB->append(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND), SwResId(FMT_USERVAR_TEXT));
                            m_xNumFormatLB->select(0);
                        }
                    }
                    if (GetCurField() && IsFieldEdit() && (!pBox || m_bInit) )
                        m_xValueED->set_text(static_cast<SwSetExpField*>(GetCurField())->GetPromptText());
                }
                else    // USERFLD
                    bFormat = bNumFormat = false;
            }
            break;

        case SwFieldTypesEnum::DDE:
            m_xValueFT->set_label(SwResId(STR_DDE_CMD));

            if (IsFieldEdit() || pBox)    // only when interacting via mouse
            {
                if (nSelPos != -1)
                {
                    SwDDEFieldType* pType =
                        static_cast<SwDDEFieldType*>( GetFieldMgr().GetFieldType(SwFieldIds::Dde, nSelData) );

                    if(pType)
                    {
                        m_xNameED->set_text(pType->GetName().toString());

                        //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
                        //              That's not considered here yet
                        OUString sCmd( pType->GetCmd() );
                        sal_Int32 nTmpPos = 0;
                        sCmd = sCmd.replaceFirst( OUStringChar(sfx2::cTokenSeparator), " ", &nTmpPos );
                        sCmd = sCmd.replaceFirst( OUStringChar(sfx2::cTokenSeparator), " ", &nTmpPos );

                        m_xValueED->set_text( sCmd );
                        m_xFormatLB->select(static_cast<int>(pType->GetType()));
                    }
                }
            }
            bName = bValue = true;
            break;

        case SwFieldTypesEnum::Sequence:
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
                        m_xChapterLevelLB->set_active(0);
                    else
                        m_xChapterLevelLB->set_active(nLevel + 1);
                    OUString sDelim = static_cast<SwSetExpFieldType*>(pFieldTyp)->GetDelimiter();
                    m_xSeparatorED->set_text( sDelim );
                    ChapterHdl(*m_xChapterLevelLB);
                }
            }
            break;

        case SwFieldTypesEnum::SetRefPage:
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

        case SwFieldTypesEnum::GetRefPage:
            m_xNameED->set_text(OUString());
            m_xValueED->set_text(OUString());
            break;

        default: break;
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
}

IMPL_LINK(SwFieldVarPage, SubTypeInsertHdl, weld::TreeView&, rBox, bool)
{
    if (!m_bInit)
    {
        SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());
        if (nTypeId == SwFieldTypesEnum::Formel)
        {
            auto nSelPos = m_xSelectionLB->get_selected_index();
            if (nSelPos != -1)
            {
                m_xValueED->replace_selection(m_xSelectionLB->get_text(nSelPos));
                ModifyHdl(*m_xNameED);
                return true;
            }
        }
    }
    TreeViewInsertHdl(rBox);
    return true;
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

    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());
    std::vector<OUString> aList;
    GetFieldMgr().GetSubTypes(nTypeId, aList);
    const size_t nCount = aList.size();
    for (size_t i = 0; i < nCount; ++i)
    {
        if (nTypeId != SwFieldTypesEnum::Input || i)
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
                    case SwFieldTypesEnum::Input:
                        if (GetCurField() && aList[i] == GetCurField()->GetPar1())
                            bInsert = true;
                        break;

                    case SwFieldTypesEnum::Formel:
                        bInsert = true;
                        break;

                    case SwFieldTypesEnum::Get:
                        if (GetCurField() && aList[i] == static_cast<const SwFormulaField*>(GetCurField())->GetFormula())
                            bInsert = true;
                        break;

                    case SwFieldTypesEnum::Set:
                        if (GetCurField() && aList[i] == GetCurField()->GetTyp()->GetName())
                        {
                            bInsert = true;
                            if (static_cast<SwSetExpField*>(GetCurField())->GetSubType() & SwGetSetExpType::Invisible)
                                m_xInvisibleCB->set_active(true);
                        }
                        break;
                    case SwFieldTypesEnum::User:
                        if (GetCurField() && aList[i] == GetCurField()->GetTyp()->GetName())
                        {
                            bInsert = true;
                            if (static_cast<SwUserField*>(GetCurField())->GetSubType() & SwUserType::Invisible)
                                m_xInvisibleCB->set_active(true);
                        }
                        break;

                    case SwFieldTypesEnum::SetRefPage:
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
                    if (nTypeId != SwFieldTypesEnum::Formel)
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

void SwFieldVarPage::FillFormatLB(SwFieldTypesEnum nTypeId)
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
    m_xFormatLB->freeze();
    m_xFormatLB->clear();
    m_xNumFormatLB->clear(); // flags list as dirty and needing refilling with stock entries
    bool bSpecialFormat = false;

    if( SwFieldTypesEnum::GetRefPage != nTypeId )
    {
        if (GetCurField() != nullptr && IsFieldEdit())
        {
            bSpecialFormat = GetCurField()->GetUntypedFormat() == NUMBERFORMAT_ENTRY_NOT_FOUND;

            if (!bSpecialFormat)
            {
                m_xNumFormatLB->SetDefFormat(GetCurField()->GetUntypedFormat());
                sOldNumSel.clear();
            }
            else if (nTypeId == SwFieldTypesEnum::Get || nTypeId == SwFieldTypesEnum::Formel)
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
        case SwFieldTypesEnum::User:
        {
            if (!IsFieldEdit() || bSpecialFormat)
            {
                OUString sId(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND));
                int nOldIndex = rWidget.get_selected_index();
                rWidget.insert(0, SwResId(FMT_MARK_TEXT), &sId, nullptr, nullptr);
                rWidget.insert(1, SwResId(FMT_USERVAR_CMD), &sId, nullptr, nullptr);
                if (nOldIndex != -1)
                    rWidget.select(nOldIndex + 2);
            }
        }
        break;

        case SwFieldTypesEnum::Set:
        {
            if (!IsFieldEdit() || bSpecialFormat)
            {
                OUString sId(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND));
                int nOldIndex = rWidget.get_selected_index();
                rWidget.insert(0, SwResId(FMT_SETVAR_TEXT), &sId, nullptr, nullptr);
                if (nOldIndex != -1)
                    rWidget.select(nOldIndex + 1);
            }
        }
        break;

        case SwFieldTypesEnum::Formel:
        case SwFieldTypesEnum::Get:
        {
            OUString sId(OUString::number(NUMBERFORMAT_ENTRY_NOT_FOUND));
            int nOldIndex = rWidget.get_selected_index();
            rWidget.insert(0, SwResId(FMT_GETVAR_NAME), &sId, nullptr, nullptr);
            if (nOldIndex != -1)
                rWidget.select(nOldIndex + 1);
        }
        break;

        default: break;
    }

    if (IsFieldEdit() && bSpecialFormat)
    {
        if (nTypeId == SwFieldTypesEnum::User && (static_cast<SwUserField*>(GetCurField())->GetSubType() & SwUserType::ShowCommand))
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

    OUString sSelectId;

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        const sal_uInt16 nFieldId = GetFieldMgr().GetFormatId( nTypeId, i );
        OUString sId(OUString::number(nFieldId));
        m_xFormatLB->append(sId, GetFieldMgr().GetFormatStr(nTypeId, i));
        if (IsFieldEdit() && GetCurField() && nFieldId == GetCurField()->GetUntypedFormat())
            sSelectId = sId;
    }

    m_xFormatLB->thaw();
    if (!sSelectId.isEmpty())
        m_xFormatLB->select_id(sSelectId);

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
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());
    bool bInsert = false, bApply = false, bDelete = false;

    OUString sName( m_xNameED->get_text() );
    sal_Int32 nLen = sName.getLength();

    switch( nTypeId )
    {
    case SwFieldTypesEnum::DDE:
    case SwFieldTypesEnum::User:
    case SwFieldTypesEnum::Set:
    case SwFieldTypesEnum::Sequence:
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
    default: break;
    }

    // check buttons
    switch (nTypeId)
    {
    case SwFieldTypesEnum::DDE:
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

    case SwFieldTypesEnum::User:
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

        if (nTypeId == SwFieldTypesEnum::Set || nTypeId == SwFieldTypesEnum::Sequence)
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
                        SwFieldType* pType = (*p)[ i ].get();
                        if (pType == pFieldType)
                            break;
                    }

                    if (i >= INIT_FLDTYPES && !pSh->IsUsed(*pFieldType))
                        bDelete = true;

                    if (nTypeId == SwFieldTypesEnum::Sequence && !(pFieldType->GetType() & SwGetSetExpType::Sequence))
                        bInsert = false;

                    if (nTypeId == SwFieldTypesEnum::Set && (pFieldType->GetType() & SwGetSetExpType::Sequence))
                        bInsert = false;
                }
            }
            if (GetFieldMgr().GetFieldType(SwFieldIds::User, sName))
                bInsert = false;
        }

        if (!nLen && (nTypeId == SwFieldTypesEnum::Set || nTypeId == SwFieldTypesEnum::Input ||
                        (!IsFieldEdit() && nTypeId == SwFieldTypesEnum::Get ) ) )
            bInsert = false;

        if( (nTypeId == SwFieldTypesEnum::Set || nTypeId == SwFieldTypesEnum::Formel) &&
            !bHasValue )
            bInsert = false;
        break;
    }

    m_xNewPB->set_sensitive(bApply);
    m_xDelPB->set_sensitive(bDelete);
    EnableInsert(bInsert, IsCurrentPage());
}

IMPL_LINK(SwFieldVarPage, TBClickHdl, weld::Button&, rBox, void)
{
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    if (&rBox == m_xDelPB.get())
    {
        if( nTypeId == SwFieldTypesEnum::User )
            GetFieldMgr().RemoveFieldType(SwFieldIds::User, m_xSelectionLB->get_selected_text());
        else
        {
            SwFieldIds nWhich;

            switch(nTypeId)
            {
                case SwFieldTypesEnum::Set:
                case SwFieldTypesEnum::Sequence:
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
            case SwFieldTypesEnum::User:   nId = SwFieldIds::User;  break;
            case SwFieldTypesEnum::DDE:    nId = SwFieldIds::Dde;   break;
            case SwFieldTypesEnum::Set:    nId = SwFieldIds::SetExp;break;
            default: break;
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

                if (nTypeId == SwFieldTypesEnum::User)
                {
                    if (nNumFormatPos != -1)
                    {
                        // The first listbox entry is Text and second is
                        // Formula and both are SAL_MAX_UINT32 :-/ but only if
                        // not another yet unlisted of Additional Formats was
                        // selected that may claim the top position :-/
                        bool bText = false;
                        sal_uInt32 nNumberFormat = lcl_getUsedNumFormat( *m_xNumFormatLB, bText);
                        if (nNumberFormat && nNumberFormat != SAL_MAX_UINT32)
                        {   // Switch language to office-language because calculator expects
                            // String in office format and it should be fed into dialog like
                            // that
                            nNumberFormat = SwValueField::GetSystemFormat(pSh->GetNumberFormatter(), nNumberFormat);
                        }
                        static_cast<SwUserFieldType*>(pType)->SetContent(m_xValueED->get_text(), nNumberFormat);
                        static_cast<SwUserFieldType*>(pType)->SetType(
                                bText ? SwUserType::String : SwUserType::Expr );
                    }
                }
                else
                {
                    if (nFormat != -1)
                    {
                        // DDE-Topics/-Items can have blanks in their names!
                        //  That's not being considered here yet.
                        sal_Int32 nTmpPos = 0;
                        sValue = sValue.replaceFirst( " ", OUStringChar(sfx2::cTokenSeparator), &nTmpPos );
                        sValue = sValue.replaceFirst( " ", OUStringChar(sfx2::cTokenSeparator), &nTmpPos );
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
            if(nTypeId == SwFieldTypesEnum::User)
            {
                SwWrtShell *pSh = GetWrtShell();
                if(!pSh)
                    pSh = ::GetActiveWrtShell();
                if(pSh)
                {
                    SwUserFieldType aType( pSh->GetDoc(), UIName(sName) );

                    if (nNumFormatPos != -1)
                    {
                        // The first listbox entry is Text and second is
                        // Formula and both are SAL_MAX_UINT32 :-/ but only if
                        // not another yet unlisted of Additional Formats was
                        // selected that may claim the top position :-/
                        bool bText = false;
                        sal_uInt32 nNumberFormat = lcl_getUsedNumFormat( *m_xNumFormatLB, bText);
                        aType.SetType(bText ? SwUserType::String : SwUserType::Expr);
                        aType.SetContent( sValue, nNumberFormat );
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
                    sValue = sValue.replaceFirst( " ", OUStringChar(sfx2::cTokenSeparator), &nTmpPos );
                    sValue = sValue.replaceFirst( " ", OUStringChar(sfx2::cTokenSeparator), &nTmpPos );

                    SwDDEFieldType aType(UIName(sName), sValue, static_cast<SfxLinkUpdateMode>(nFormat));
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

IMPL_LINK_NOARG(SwFieldVarPage, ChapterHdl, weld::ComboBox&, void)
{
    bool bEnable = m_xChapterLevelLB->get_active() != 0;

    m_xSeparatorED->set_sensitive(bEnable);
    m_xSeparatorFT->set_sensitive(bEnable);
    SeparatorHdl(*m_xSeparatorED);
}

IMPL_LINK_NOARG(SwFieldVarPage, SeparatorHdl, weld::Entry&, void)
{
    bool bEnable = !m_xSeparatorED->get_text().isEmpty() ||
                    m_xChapterLevelLB->get_active() == 0;
    EnableInsert(bEnable, IsCurrentPage());
}

bool SwFieldVarPage::FillItemSet(SfxItemSet* )
{
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

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
            // Switch language to office language because calculator expects
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
        case SwFieldTypesEnum::User:
        {
            SwGetSetExpType nUserSubType = (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND) ? SwGetSetExpType::String : SwGetSetExpType::Expr;

            if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND && m_xNumFormatLB->get_selected_text() == SwResId(FMT_USERVAR_CMD))
                nUserSubType |= SwGetSetExpType::Command;

            if (m_xInvisibleCB->get_active())
                nUserSubType |= SwGetSetExpType::Invisible;
            nSubType = static_cast<sal_uInt16>(nUserSubType);
            break;
        }
        case SwFieldTypesEnum::Formel:
        {
            SwGetSetExpType nFormSubType = SwGetSetExpType::Formula;
            if (m_xNumFormatLB->get_visible() && nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
                nFormSubType |= SwGetSetExpType::Command;
            nSubType = static_cast<sal_uInt16>(nFormSubType);
            break;
        }
        case SwFieldTypesEnum::Get:
        {
            nSubType &= 0xff00;
            if (m_xNumFormatLB->get_visible() && nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
                nSubType |= static_cast<sal_uInt16>(SwGetSetExpType::Command);
            break;
        }
        case SwFieldTypesEnum::Input:
        {
            SwFieldType* pType = GetFieldMgr().GetFieldType(SwFieldIds::User, aName);
            nSubType = static_cast< sal_uInt16 >((nSubType & 0xff00) | static_cast<sal_uInt16>(pType ? SwInputFieldSubType::User : SwInputFieldSubType::Var));
            break;
        }

        case SwFieldTypesEnum::Set:
        {
            SwGetSetExpType nSetSubType = SwGetSetExpType::None;
            if (IsFieldDlgHtmlMode())
                nSetSubType = SwGetSetExpType::Command | SwGetSetExpType::String;
            else
                nSetSubType = ((nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND) ? SwGetSetExpType::String : SwGetSetExpType::Expr);

            if (m_xInvisibleCB->get_active())
                nSetSubType |= SwGetSetExpType::Invisible;
            nSubType = static_cast<sal_uInt16>(nSetSubType);
            break;
        }
        case SwFieldTypesEnum::Sequence:
        {
            nSubType = static_cast< sal_uInt16 >(m_xChapterLevelLB->get_active());
            if (nSubType == 0)
                nSubType = 0x7f;
            else
            {
                nSubType--;
                OUString sSeparator(m_xSeparatorED->get_text()[0]);
                cSeparator = !sSeparator.isEmpty() ? sSeparator[0] : ' ';
            }
            break;
        }
        case SwFieldTypesEnum::GetRefPage:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
                aVal = m_xValueED->get_text();
            break;
        default: break;
    }

    if (!IsFieldEdit() ||
        m_xNameED->get_value_changed_from_saved() ||
        m_xValueED->get_value_changed_from_saved() ||
        m_xSelectionLB->get_value_changed_from_saved() ||
        m_xFormatLB->get_value_changed_from_saved() ||
        m_nOldFormat != m_xNumFormatLB->GetFormat() ||
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

std::unique_ptr<SfxTabPage> SwFieldVarPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                         const SfxItemSet *const pAttrSet)
{
    return std::make_unique<SwFieldVarPage>( pPage, pController, pAttrSet );
}

sal_uInt16 SwFieldVarPage::GetGroup()
{
    return GRP_VAR;
}

void SwFieldVarPage::FillUserData()
{
    OUString sData = u"" USER_DATA_VERSION ";"_ustr;
    sal_Int32 nTypeSel = m_xTypeLB->get_selected_index();
    if( -1 == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = m_xTypeLB->get_id(nTypeSel).toUInt32();
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
