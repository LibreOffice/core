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
#include <strings.hrc>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include "fldfunc.hxx"
#include "flddinf.hxx"
#include <flddropdown.hxx>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

using namespace ::com::sun::star;

SwFieldFuncPage::SwFieldFuncPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *const pCoreSet)
    : SwFieldPage(pPage, pController, "modules/swriter/ui/fldfuncpage.ui", "FieldFuncPage", pCoreSet)
    , nOldFormat(0)
    , bDropDownLBChanged(false)
    , m_xTypeLB(m_xBuilder->weld_tree_view("type"))
    , m_xSelectionLB(m_xBuilder->weld_tree_view("select"))
    , m_xFormat(m_xBuilder->weld_widget("formatframe"))
    , m_xFormatLB(m_xBuilder->weld_tree_view("format"))
    , m_xNameFT(m_xBuilder->weld_label("nameft"))
    , m_xNameED(new ConditionEdit(m_xBuilder->weld_entry("condFunction")))
    , m_xValueGroup(m_xBuilder->weld_widget("valuegroup"))
    , m_xValueFT(m_xBuilder->weld_label("valueft"))
    , m_xValueED(m_xBuilder->weld_entry("value"))
    , m_xCond1FT(m_xBuilder->weld_label("cond1ft"))
    , m_xCond1ED(new ConditionEdit(m_xBuilder->weld_entry("cond1")))
    , m_xCond2FT(m_xBuilder->weld_label("cond2ft"))
    , m_xCond2ED(new ConditionEdit(m_xBuilder->weld_entry("cond2")))
    , m_xMacroBT(m_xBuilder->weld_button("macro"))
    , m_xListGroup(m_xBuilder->weld_widget("listgroup"))
    , m_xListItemFT(m_xBuilder->weld_label("itemft"))
    , m_xListItemED(m_xBuilder->weld_entry("item"))
    , m_xListAddPB(m_xBuilder->weld_button("add"))
    , m_xListItemsFT(m_xBuilder->weld_label("listitemft"))
    , m_xListItemsLB(m_xBuilder->weld_tree_view("listitems"))
    , m_xListRemovePB(m_xBuilder->weld_button("remove"))
    , m_xListUpPB(m_xBuilder->weld_button("up"))
    , m_xListDownPB(m_xBuilder->weld_button("down"))
    , m_xListNameFT(m_xBuilder->weld_label("listnameft"))
    , m_xListNameED(m_xBuilder->weld_entry("listname"))
{
    FillFieldSelect(*m_xSelectionLB);
    FillFieldSelect(*m_xFormatLB);
    m_xListItemsLB->set_size_request(m_xListItemED->get_preferred_size().Width(),
                                     m_xListItemsLB->get_height_rows(5));

    auto nWidth = m_xTypeLB->get_approximate_digit_width() * FIELD_COLUMN_WIDTH;
    auto nHeight = m_xTypeLB->get_height_rows(10);
    m_xTypeLB->set_size_request(nWidth, nHeight);
    m_xFormatLB->set_size_request(nWidth, nHeight);

    m_xNameED->connect_changed(LINK(this, SwFieldFuncPage, ModifyHdl));

    m_sOldValueFT = m_xValueFT->get_label();
    m_sOldNameFT = m_xNameFT->get_label();

    m_xCond1ED->ShowBrackets(false);
    m_xCond2ED->ShowBrackets(false);

    // uitests
    m_xTypeLB->set_buildable_name(m_xTypeLB->get_buildable_name() + "-func");
}

SwFieldFuncPage::~SwFieldFuncPage()
{
}

void SwFieldFuncPage::Reset(const SfxItemSet* )
{
    SavePos(*m_xTypeLB);
    Init(); // general initialisation

    m_xTypeLB->freeze();
    m_xTypeLB->clear();

    if (!IsFieldEdit())
    {
        // initialise TypeListBox
        const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

        // fill Typ-Listbox
        for(sal_uInt16 i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            const SwFieldTypesEnum nTypeId = SwFieldMgr::GetTypeId(i);
            m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(i));
        }
    }
    else
    {
        const SwFieldTypesEnum nTypeId = GetCurField()->GetTypeId();
        m_xTypeLB->append(OUString::number(static_cast<sal_uInt16>(nTypeId)), SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));

        if (nTypeId == SwFieldTypesEnum::Macro)
        {
            GetFieldMgr().SetMacroPath(GetCurField()->GetPar1());
        }
    }

    m_xTypeLB->connect_row_activated(LINK(this, SwFieldFuncPage, TreeViewInsertHdl));
    m_xTypeLB->connect_changed(LINK(this, SwFieldFuncPage, TypeHdl));
    m_xSelectionLB->connect_changed(LINK(this, SwFieldFuncPage, SelectHdl));
    m_xSelectionLB->connect_row_activated(LINK(this, SwFieldFuncPage, InsertMacroHdl));
    m_xFormatLB->connect_row_activated(LINK(this, SwFieldFuncPage, TreeViewInsertHdl));
    m_xMacroBT->connect_clicked(LINK(this, SwFieldFuncPage, MacroHdl));
    Link<weld::Button&,void> aListModifyLk( LINK(this, SwFieldFuncPage, ListModifyButtonHdl));
    m_xListAddPB->connect_clicked(aListModifyLk);
    m_xListRemovePB->connect_clicked(aListModifyLk);
    m_xListUpPB->connect_clicked(aListModifyLk);
    m_xListDownPB->connect_clicked(aListModifyLk);
    m_xListItemED->connect_activate(LINK(this, SwFieldFuncPage, ListModifyReturnActionHdl));
    Link<weld::Entry&,void> aListEnableLk = LINK(this, SwFieldFuncPage, ListEnableHdl);
    m_xListItemED->connect_changed(aListEnableLk);
    m_xListItemsLB->connect_changed(LINK(this, SwFieldFuncPage, ListEnableListBoxHdl));

    int nSelect = -1;
    if( !IsRefresh() )
    {
        const OUString sUserData = GetUserData();
        sal_Int32 nIdx{ 0 };
        if(sUserData.getToken(0, ';', nIdx).equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            const sal_uInt16 nVal = static_cast< sal_uInt16 >(sUserData.getToken(0, ';', nIdx).toInt32());
            if(nVal != USHRT_MAX)
            {
                for (sal_Int32 i = 0, nEntryCount = m_xTypeLB->n_children(); i < nEntryCount; ++i)
                {
                    if (nVal == m_xTypeLB->get_id(i).toUInt32())
                    {
                        nSelect = i;
                        break;
                    }
                }
            }
        }
    }

    m_xTypeLB->thaw();
    if (nSelect != -1)
        m_xTypeLB->select(nSelect);
    else
    {
        // select old Pos
        RestorePos(*m_xTypeLB);
    }
    TypeHdl(*m_xTypeLB);

    if (IsFieldEdit())
    {
        m_xNameED->save_value();
        m_xValueED->save_value();
        m_xCond1ED->save_value();
        m_xCond2ED->save_value();
        nOldFormat = GetCurField()->GetFormat();
    }
}

const TranslateId FMT_MARK_ARY[] =
{
    FMT_MARK_TEXT,
    FMT_MARK_TABLE,
    FMT_MARK_FRAME,
    FMT_MARK_GRAFIC,
    FMT_MARK_OLE
};

IMPL_LINK_NOARG(SwFieldFuncPage, TypeHdl, weld::TreeView&, void)
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

    if (nOld == GetTypeSel())
        return;

    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    // fill Selection-Listbox
    UpdateSubType();

    // fill Format-Listbox
    m_xFormatLB->clear();

    const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(nTypeId, IsFieldDlgHtmlMode());

    for (sal_uInt16 i = 0; i < nSize; i++)
    {
        m_xFormatLB->append(OUString::number(GetFieldMgr().GetFormatId(nTypeId, i)),
                              GetFieldMgr().GetFormatStr(nTypeId, i));
    }

    if (nSize)
    {
        if (IsFieldEdit() && nTypeId == SwFieldTypesEnum::JumpEdit)
            m_xFormatLB->select_text(SwResId(FMT_MARK_ARY[GetCurField()->GetFormat()]));

        if (m_xFormatLB->get_selected_index() == -1)
            m_xFormatLB->select(0);
    }

    bool bValue = false, bName = false, bMacro = false, bInsert = true;
    bool bFormat = nSize != 0;

    // two controls for conditional text
    bool bDropDown = SwFieldTypesEnum::Dropdown == nTypeId;
    bool bCondTextField = SwFieldTypesEnum::ConditionalText == nTypeId;

    m_xCond1FT->set_visible(!bDropDown && bCondTextField);
    m_xCond1ED->set_visible(!bDropDown && bCondTextField);
    m_xCond2FT->set_visible(!bDropDown && bCondTextField);
    m_xCond2ED->set_visible(!bDropDown && bCondTextField);
    m_xValueGroup->set_visible(!bDropDown && !bCondTextField);
    m_xMacroBT->set_visible(!bDropDown);
    m_xNameED->set_visible(!bDropDown);
    m_xNameFT->set_visible(!bDropDown);

    m_xListGroup->set_visible(bDropDown);

    m_xNameED->SetDropEnable(false);

    if (IsFieldEdit())
    {
        if(bDropDown)
        {
            const SwDropDownField* pDrop = static_cast<const SwDropDownField*>(GetCurField());
            const uno::Sequence<OUString> aItems = pDrop->GetItemSequence();
            m_xListItemsLB->clear();
            for (const OUString& rItem : aItems)
                m_xListItemsLB->append_text(rItem);
            m_xListItemsLB->select_text(pDrop->GetSelectedItem());
            m_xListNameED->set_text(pDrop->GetPar2());
            m_xListNameED->save_value();
            bDropDownLBChanged = false;
        }
        else
        {
            m_xNameED->set_text(GetCurField()->GetPar1());
            m_xValueED->set_text(GetCurField()->GetPar2());
        }
    }
    else
    {
        m_xNameED->set_text(OUString());
        m_xValueED->set_text(OUString());
    }
    if(bDropDown)
        ListEnableHdl(*m_xListItemED);

    if (m_xNameFT->get_label() != m_sOldNameFT)
        m_xNameFT->set_label(m_sOldNameFT);
    if (m_xValueFT->get_label() != m_sOldValueFT)
        m_xValueFT->set_label(m_sOldValueFT);

    switch (nTypeId)
    {
        case SwFieldTypesEnum::Macro:
            bMacro = true;
            if (!GetFieldMgr().GetMacroPath().isEmpty())
                bValue = true;
            else
                bInsert = false;

            m_xNameFT->set_label(SwResId(STR_MACNAME));
            m_xValueFT->set_label(SwResId(STR_PROMPT));
            m_xNameED->set_text(GetFieldMgr().GetMacroName());
            m_xNameED->set_accessible_name(m_xNameFT->get_label());
            m_xValueED->set_accessible_name(m_xValueFT->get_label());
            break;

        case SwFieldTypesEnum::HiddenParagraph:
            m_xNameFT->set_label(SwResId(STR_COND));
            m_xNameED->SetDropEnable(true);
            bName = true;
            m_xNameED->set_accessible_name(m_xNameFT->get_label());
            m_xValueED->set_accessible_name(m_xValueFT->get_label());
            break;

        case SwFieldTypesEnum::HiddenText:
        {
            m_xNameFT->set_label(SwResId(STR_COND));
            m_xNameED->SetDropEnable(true);
            m_xValueFT->set_label(SwResId(STR_INSTEXT));
            SwWrtShell* pSh = GetActiveWrtShell();
            if (!IsFieldEdit() && pSh )
                m_xValueED->set_text(pSh->GetSelText());
            bName = bValue = true;
            m_xNameED->set_accessible_name(m_xNameFT->get_label());
            m_xValueED->set_accessible_name(m_xValueFT->get_label());
        }
        break;

        case SwFieldTypesEnum::ConditionalText:
            m_xNameFT->set_label(SwResId(STR_COND));
            m_xNameED->SetDropEnable(true);
            if (IsFieldEdit())
            {
                sal_Int32 nIdx{ 0 };
                m_xCond1ED->set_text(GetCurField()->GetPar2().getToken(0, '|', nIdx));
                m_xCond2ED->set_text(GetCurField()->GetPar2().getToken(0, '|', nIdx));
            }

            bName = bValue = true;
            m_xNameED->set_accessible_name(m_xNameFT->get_label());
            m_xValueED->set_accessible_name(m_xValueFT->get_label());
            break;

        case SwFieldTypesEnum::JumpEdit:
            m_xNameFT->set_label(SwResId(STR_JUMPEDITFLD));
            m_xValueFT->set_label(SwResId(STR_PROMPT));
            bName = bValue = true;
            m_xNameED->set_accessible_name(m_xNameFT->get_label());
            m_xValueED->set_accessible_name(m_xValueFT->get_label());
            break;

        case SwFieldTypesEnum::Input:
            m_xValueFT->set_label(SwResId(STR_PROMPT));
            bValue = true;
            m_xNameED->set_accessible_name(m_xNameFT->get_label());
            m_xValueED->set_accessible_name(m_xValueFT->get_label());
            break;

        case SwFieldTypesEnum::CombinedChars:
            {
                m_xNameFT->set_label(SwResId(STR_COMBCHRS_FT));
                m_xNameED->SetDropEnable(true);
                bName = true;

                const sal_Int32 nLen = m_xNameED->get_text().getLength();
                if( !nLen || nLen > MAX_COMBINED_CHARACTERS )
                    bInsert = false;
                m_xNameED->set_accessible_name(m_xNameFT->get_label());
                m_xValueED->set_accessible_name(m_xValueFT->get_label());
            }
            break;
        case SwFieldTypesEnum::Dropdown :
        break;
        default:
            break;
    }

    m_xSelectionLB->hide();

    m_xFormat->set_sensitive(bFormat);
    m_xNameFT->set_sensitive(bName);
    m_xNameED->set_sensitive(bName);
    m_xValueGroup->set_sensitive(bValue);
    m_xMacroBT->set_sensitive(bMacro);

    EnableInsert( bInsert );
}

IMPL_LINK_NOARG(SwFieldFuncPage, SelectHdl, weld::TreeView&, void)
{
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    if( SwFieldTypesEnum::Macro == nTypeId )
        m_xNameED->set_text( m_xSelectionLB->get_selected_text() );
}

IMPL_LINK_NOARG(SwFieldFuncPage, InsertMacroHdl, weld::TreeView&, bool)
{
    SelectHdl(*m_xSelectionLB);
    InsertHdl(nullptr);
    return true;
}

IMPL_LINK(SwFieldFuncPage, ListModifyButtonHdl, weld::Button&, rControl, void)
{
    ListModifyHdl(&rControl);
}

IMPL_LINK(SwFieldFuncPage, ListModifyReturnActionHdl, weld::Entry&, rControl, bool)
{
    ListModifyHdl(&rControl);
    return true;
}

void SwFieldFuncPage::ListModifyHdl(const weld::Widget* pControl)
{
    if (pControl == m_xListAddPB.get() ||
            (pControl == m_xListItemED.get() && m_xListAddPB->get_sensitive()))
    {
        const OUString sEntry(m_xListItemED->get_text());
        m_xListItemsLB->append_text(sEntry);
        m_xListItemsLB->select_text(sEntry);
    }
    else if (m_xListItemsLB->get_selected_index() != -1)
    {
        sal_Int32 nSelPos = m_xListItemsLB->get_selected_index();
        if (pControl == m_xListRemovePB.get())
        {
            m_xListItemsLB->remove(nSelPos);
            m_xListItemsLB->select(nSelPos ? nSelPos - 1 : 0);
        }
        else if (pControl == m_xListUpPB.get())
        {
            if(nSelPos)
            {
                const OUString sEntry = m_xListItemsLB->get_selected_text();
                m_xListItemsLB->remove(nSelPos);
                nSelPos--;
                m_xListItemsLB->insert_text(nSelPos, sEntry);
                m_xListItemsLB->select(nSelPos);
            }
        }
        else if (pControl == m_xListDownPB.get())
        {
            if( nSelPos < m_xListItemsLB->n_children() - 1)
            {
                const OUString sEntry = m_xListItemsLB->get_selected_text();
                m_xListItemsLB->remove(nSelPos);
                nSelPos++;
                m_xListItemsLB->insert_text(nSelPos, sEntry);
                m_xListItemsLB->select(nSelPos);
            }
        }
    }
    bDropDownLBChanged = true;
    ListEnableHdl(*m_xListItemED);
}

IMPL_LINK_NOARG(SwFieldFuncPage, ListEnableListBoxHdl, weld::TreeView&, void)
{
    ListEnableHdl(*m_xListItemED);
}

IMPL_LINK_NOARG(SwFieldFuncPage, ListEnableHdl, weld::Entry&, void)
{
    //enable "Add" button when text is in the Edit that's not already member of the box
    m_xListAddPB->set_sensitive(!m_xListItemED->get_text().isEmpty() &&
                -1 == m_xListItemsLB->find_text(m_xListItemED->get_text()));
    bool bEnableButtons = m_xListItemsLB->get_selected_index() != -1;
    m_xListRemovePB->set_sensitive(bEnableButtons);
    m_xListUpPB->set_sensitive(bEnableButtons && (m_xListItemsLB->get_selected_index() > 0));
    m_xListDownPB->set_sensitive(bEnableButtons &&
                (m_xListItemsLB->get_selected_index() < (m_xListItemsLB->n_children() - 1)));
}

// renew types in SelectionBox
void SwFieldFuncPage::UpdateSubType()
{
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    // fill Selection-Listbox
    m_xSelectionLB->freeze();
    m_xSelectionLB->clear();

    std::vector<OUString> aLst;
    GetFieldMgr().GetSubTypes(nTypeId, aLst);
    const size_t nCount = aLst.size();

    for (size_t i = 0; i < nCount; ++i)
        m_xSelectionLB->append(OUString::number(i), aLst[i]);
    m_xSelectionLB->thaw();

    bool bEnable = nCount != 0;

    m_xSelectionLB->set_sensitive( bEnable );

    if (bEnable)
        m_xSelectionLB->select(0);

    if (nTypeId == SwFieldTypesEnum::Macro)
    {
        const bool bHasMacro = !GetFieldMgr().GetMacroPath().isEmpty();

        if (bHasMacro)
        {
            m_xNameED->set_text(GetFieldMgr().GetMacroName());
            m_xValueGroup->set_sensitive(true);
        }
        EnableInsert(bHasMacro);
    }
}

// call MacroBrowser, fill Listbox with Macros
IMPL_LINK_NOARG( SwFieldFuncPage, MacroHdl, weld::Button&, void)
{
    if (GetFieldMgr().ChooseMacro(GetFrameWeld()))
        UpdateSubType();
}

bool SwFieldFuncPage::FillItemSet(SfxItemSet* )
{
    const SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    sal_uInt16 nSubType = 0;

    const sal_Int32 nEntryPos = m_xFormatLB->get_selected_index();
    const sal_uLong nFormat = (nEntryPos == -1)
        ? 0 : m_xFormatLB->get_id(nEntryPos).toUInt32();

    OUString aVal(m_xValueED->get_text());
    OUString aName(m_xNameED->get_text());

    switch(nTypeId)
    {
        case SwFieldTypesEnum::Input:
            nSubType = INP_TXT;
            // to prevent removal of CR/LF restore old content
            if (!m_xNameED->get_value_changed_from_saved() && IsFieldEdit())
                aName = GetCurField()->GetPar1();

            break;

        case SwFieldTypesEnum::Macro:
            // use the full script URL, not the name in the Edit control
            aName = GetFieldMgr().GetMacroPath();
            break;

        case SwFieldTypesEnum::ConditionalText:
            aVal = m_xCond1ED->get_text() + "|" + m_xCond2ED->get_text();
            break;
        case SwFieldTypesEnum::Dropdown :
        {
            aName = m_xListNameED->get_text();
            for (sal_Int32 i = 0, nEntryCount = m_xListItemsLB->n_children(); i < nEntryCount; ++i)
            {
                if(i)
                    aVal += OUStringChar(DB_DELIM);
                aVal += m_xListItemsLB->get_text(i);
            }
        }
        break;
        default:
            break;
    }

    if (!IsFieldEdit() ||
        m_xNameED->get_value_changed_from_saved() ||
        m_xValueED->get_value_changed_from_saved() ||
        m_xCond1ED->get_value_changed_from_saved() ||
        m_xCond2ED->get_value_changed_from_saved() ||
        m_xListNameED->get_value_changed_from_saved() ||
        bDropDownLBChanged ||
        nOldFormat != nFormat)
    {
        InsertField( nTypeId, nSubType, aName, aVal, nFormat );
    }

    ModifyHdl(m_xNameED->get_widget());    // enable/disable Insert if applicable

    return false;
}

std::unique_ptr<SfxTabPage> SwFieldFuncPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet *const pAttrSet)
{
    return std::make_unique<SwFieldFuncPage>(pPage, pController, pAttrSet);
}

sal_uInt16 SwFieldFuncPage::GetGroup()
{
    return GRP_FKT;
}

void    SwFieldFuncPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_xTypeLB->get_selected_index();
    const sal_uInt16 nTypeSel = ( -1 == nEntryPos )
        ? USHRT_MAX
        : m_xTypeLB->get_id(nEntryPos).toUInt32();
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

IMPL_LINK_NOARG(SwFieldFuncPage, ModifyHdl, weld::Entry&, void)
{
    const sal_Int32 nLen = m_xNameED->get_text().getLength();

    bool bEnable = true;
    SwFieldTypesEnum nTypeId = static_cast<SwFieldTypesEnum>(m_xTypeLB->get_id(GetTypeSel()).toUInt32());

    if( SwFieldTypesEnum::CombinedChars == nTypeId &&
        (!nLen || nLen > MAX_COMBINED_CHARACTERS ))
        bEnable = false;

    EnableInsert( bEnable );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
