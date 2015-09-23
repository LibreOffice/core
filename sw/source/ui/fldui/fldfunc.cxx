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

#include <sfx2/app.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <docsh.hxx>
#include <swtypes.hxx>
#include <globals.hrc>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <fldfunc.hxx>
#include <flddropdown.hxx>
#include <fldui.hrc>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

using namespace ::com::sun::star;

SwFieldFuncPage::SwFieldFuncPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SwFieldPage(pParent, "FieldFuncPage",
        "modules/swriter/ui/fldfuncpage.ui", rCoreSet)
    , nOldFormat(0)
    , bDropDownLBChanged(false)
{
    get(m_pTypeLB, "type");
    get(m_pFormat, "formatframe");
    get(m_pSelectionLB, "select");
    get(m_pFormatLB, "format");
    get(m_pNameFT, "nameft");
    get(m_pNameED, "name");
    get(m_pValueGroup, "valuegroup");
    get(m_pValueFT, "valueft");
    get(m_pValueED, "value");
    get(m_pCond1FT, "cond1ft");
    get(m_pCond1ED, "cond1");
    get(m_pCond2FT, "cond2ft");
    get(m_pCond2ED, "cond2");
    get(m_pMacroBT, "macro");

    get(m_pListGroup, "listgroup");
    get(m_pListItemFT, "itemft");
    get(m_pListItemED, "item");
    get(m_pListAddPB, "add");
    get(m_pListItemsFT, "listitemft");
    get(m_pListItemsLB, "listitems");
    m_pListItemsLB->SetDropDownLineCount(5);
    m_pListItemsLB->set_width_request(m_pListItemED->GetOptimalSize().Width());
    get(m_pListRemovePB, "remove");
    get(m_pListUpPB, "up");
    get(m_pListDownPB, "down");
    get(m_pListNameFT, "listnameft");
    get(m_pListNameED, "listname");

    long nHeight = m_pTypeLB->GetTextHeight() * 20;
    m_pTypeLB->set_height_request(nHeight);
    m_pFormatLB->set_height_request(nHeight);

    long nWidth = m_pTypeLB->LogicToPixel(Size(FIELD_COLUMN_WIDTH, 0), MapMode(MAP_APPFONT)).Width();
    m_pTypeLB->set_width_request(nWidth);
    m_pFormatLB->set_width_request(nWidth);

    m_pNameED->SetModifyHdl(LINK(this, SwFieldFuncPage, ModifyHdl));

    m_sOldValueFT = m_pValueFT->GetText();
    m_sOldNameFT = m_pNameFT->GetText();

    m_pCond1ED->ShowBrackets(false);
    m_pCond2ED->ShowBrackets(false);
}

SwFieldFuncPage::~SwFieldFuncPage()
{
    disposeOnce();
}

void SwFieldFuncPage::dispose()
{
    m_pTypeLB.clear();
    m_pSelectionLB.clear();
    m_pFormat.clear();
    m_pFormatLB.clear();
    m_pNameFT.clear();
    m_pNameED.clear();
    m_pValueGroup.clear();
    m_pValueFT.clear();
    m_pValueED.clear();
    m_pCond1FT.clear();
    m_pCond1ED.clear();
    m_pCond2FT.clear();
    m_pCond2ED.clear();
    m_pMacroBT.clear();
    m_pListGroup.clear();
    m_pListItemFT.clear();
    m_pListItemED.clear();
    m_pListAddPB.clear();
    m_pListItemsFT.clear();
    m_pListItemsLB.clear();
    m_pListRemovePB.clear();
    m_pListUpPB.clear();
    m_pListDownPB.clear();
    m_pListNameFT.clear();
    m_pListNameED.clear();
    SwFieldPage::dispose();
}

void SwFieldFuncPage::Reset(const SfxItemSet* )
{
    SavePos(m_pTypeLB);
    Init(); // general initialisation

    m_pTypeLB->SetUpdateMode(false);
    m_pTypeLB->Clear();

    sal_Int32 nPos;

    if (!IsFieldEdit())
    {
        // initialise TypeListBox
        const SwFieldGroupRgn& rRg = SwFieldMgr::GetGroupRange(IsFieldDlgHtmlMode(), GetGroup());

        // fill Typ-Listbox
        for(sal_uInt16 i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            const sal_uInt16 nTypeId = SwFieldMgr::GetTypeId(i);
            nPos = m_pTypeLB->InsertEntry(SwFieldMgr::GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        const sal_uInt16 nTypeId = GetCurField()->GetTypeId();
        nPos = m_pTypeLB->InsertEntry(SwFieldMgr::GetTypeStr(SwFieldMgr::GetPos(nTypeId)));
        m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));

        if (nTypeId == TYP_MACROFLD)
        {
            GetFieldMgr().SetMacroPath(GetCurField()->GetPar1());
        }
    }

    // select old Pos
    RestorePos(m_pTypeLB);

    m_pTypeLB->SetDoubleClickHdl       (LINK(this, SwFieldFuncPage, ListBoxInsertHdl));
    m_pTypeLB->SetSelectHdl            (LINK(this, SwFieldFuncPage, TypeHdl));
    m_pSelectionLB->SetSelectHdl       (LINK(this, SwFieldFuncPage, SelectHdl));
    m_pSelectionLB->SetDoubleClickHdl  (LINK(this, SwFieldFuncPage, InsertMacroHdl));
    m_pFormatLB->SetDoubleClickHdl     (LINK(this, SwFieldFuncPage, ListBoxInsertHdl));
    m_pMacroBT->SetClickHdl            (LINK(this, SwFieldFuncPage, MacroHdl));
    Link<Button*,void> aListModifyLk( LINK(this, SwFieldFuncPage, ListModifyButtonHdl));
    m_pListAddPB->SetClickHdl(aListModifyLk);
    m_pListRemovePB->SetClickHdl(aListModifyLk);
    m_pListUpPB->SetClickHdl(aListModifyLk);
    m_pListDownPB->SetClickHdl(aListModifyLk);
    m_pListItemED->SetReturnActionLink(LINK(this, SwFieldFuncPage, ListModifyReturnActionHdl));
    Link<> aListEnableLk = LINK(this, SwFieldFuncPage, ListEnableHdl);
    m_pListItemED->SetModifyHdl(aListEnableLk);
    m_pListItemsLB->SetSelectHdl(aListEnableLk);

    if( !IsRefresh() )
    {
        OUString sUserData = GetUserData();
        if(sUserData.getToken(0, ';').equalsIgnoreAsciiCase(USER_DATA_VERSION_1))
        {
            OUString sVal = sUserData.getToken(1, ';');
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
    TypeHdl(0);

    m_pTypeLB->SetUpdateMode(true);

    if (IsFieldEdit())
    {
        m_pNameED->SaveValue();
        m_pValueED->SaveValue();
        m_pCond1ED->SaveValue();
        m_pCond2ED->SaveValue();
        nOldFormat = GetCurField()->GetFormat();
    }
}

IMPL_LINK_NOARG(SwFieldFuncPage, TypeHdl)
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
        const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

        // fill Selection-Listbox
        UpdateSubType();

        // fill Format-Listbox
        m_pFormatLB->Clear();

        const sal_uInt16 nSize = GetFieldMgr().GetFormatCount(nTypeId, false, IsFieldDlgHtmlMode());

        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            sal_Int32 nPos = m_pFormatLB->InsertEntry(GetFieldMgr().GetFormatStr(nTypeId, i));
            m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(GetFieldMgr().GetFormatId( nTypeId, i )) );
        }

        if (nSize)
        {
            if (IsFieldEdit() && nTypeId == TYP_JUMPEDITFLD)
                m_pFormatLB->SelectEntry(SW_RESSTR(FMT_MARK_BEGIN + GetCurField()->GetFormat()));

            if (!m_pFormatLB->GetSelectEntryCount())
                m_pFormatLB->SelectEntryPos(0);
        }

        bool bValue = false, bName = false, bMacro = false, bInsert = true;
        bool bFormat = nSize != 0;

        // two controls for conditional text
        bool bDropDown = TYP_DROPDOWN == nTypeId;
        bool bCondTextField = TYP_CONDTXTFLD == nTypeId;

        m_pCond1FT->Show(!bDropDown && bCondTextField);
        m_pCond1ED->Show(!bDropDown && bCondTextField);
        m_pCond2FT->Show(!bDropDown && bCondTextField);
        m_pCond2ED->Show(!bDropDown && bCondTextField);
        m_pValueGroup->Show(!bDropDown && !bCondTextField);
        m_pMacroBT->Show(!bDropDown);
        m_pNameED->Show(!bDropDown);
        m_pNameFT->Show(!bDropDown);

        m_pListGroup->Show(bDropDown);

        m_pNameED->SetDropEnable(false);

        if (IsFieldEdit())
        {
            if(bDropDown)
            {
                const SwDropDownField* pDrop = static_cast<const SwDropDownField*>(GetCurField());
                uno::Sequence<OUString> aItems = pDrop->GetItemSequence();
                const OUString* pArray = aItems.getConstArray();
                m_pListItemsLB->Clear();
                for(sal_Int32 i = 0; i < aItems.getLength(); i++)
                    m_pListItemsLB->InsertEntry(pArray[i]);
                m_pListItemsLB->SelectEntry(pDrop->GetSelectedItem());
                m_pListNameED->SetText(pDrop->GetPar2());
                m_pListNameED->SaveValue();
                bDropDownLBChanged = false;
            }
            else
            {
                m_pNameED->SetText(GetCurField()->GetPar1());
                m_pValueED->SetText(GetCurField()->GetPar2());
            }
        }
        else
        {
            m_pNameED->SetText(OUString());
            m_pValueED->SetText(OUString());
        }
        if(bDropDown)
            ListEnableHdl(0);

        if (m_pNameFT->GetText() != m_sOldNameFT)
            m_pNameFT->SetText(m_sOldNameFT);
        if (m_pValueFT->GetText() != m_sOldValueFT)
            m_pValueFT->SetText(m_sOldValueFT);

        switch (nTypeId)
        {
            case TYP_MACROFLD:
                bMacro = true;
                if (!GetFieldMgr().GetMacroPath().isEmpty())
                    bValue = true;
                else
                    bInsert = false;

                m_pNameFT->SetText(SW_RESSTR(STR_MACNAME));
                m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));
                m_pNameED->SetText(GetFieldMgr().GetMacroName());
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_HIDDENPARAFLD:
                m_pNameFT->SetText(SW_RESSTR(STR_COND));
                m_pNameED->SetDropEnable(true);
                bName = true;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_HIDDENTXTFLD:
            {
                m_pNameFT->SetText(SW_RESSTR(STR_COND));
                m_pNameED->SetDropEnable(true);
                m_pValueFT->SetText(SW_RESSTR(STR_INSTEXT));
                SwWrtShell* pSh = GetActiveWrtShell();
                if (!IsFieldEdit() && pSh )
                    m_pValueED->SetText(pSh->GetSelText());
                bName = bValue = true;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
            }
            break;

            case TYP_CONDTXTFLD:
                m_pNameFT->SetText(SW_RESSTR(STR_COND));
                m_pNameED->SetDropEnable(true);
                if (IsFieldEdit())
                {
                    m_pCond1ED->SetText(GetCurField()->GetPar2().getToken(0, '|'));
                    m_pCond2ED->SetText(GetCurField()->GetPar2().getToken(1, '|'));
                }

                bName = bValue = true;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_JUMPEDITFLD:
                m_pNameFT->SetText(SW_RESSTR(STR_JUMPEDITFLD));
                m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));
                bName = bValue = true;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_INPUTFLD:
                m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));
                bValue = true;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_COMBINED_CHARS:
                {
                    m_pNameFT->SetText(SW_RESSTR(STR_COMBCHRS_FT));
                    m_pNameED->SetDropEnable(true);
                    bName = true;

                    const sal_Int32 nLen = m_pNameED->GetText().getLength();
                    if( !nLen || nLen > MAX_COMBINED_CHARACTERS )
                        bInsert = false;
                    m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                    m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                }
                break;
            case TYP_DROPDOWN :
            break;
            default:
                break;
        }

        m_pSelectionLB->Hide();

        m_pFormat->Enable(bFormat);
        m_pNameFT->Enable(bName);
        m_pNameED->Enable(bName);
        m_pValueGroup->Enable(bValue);
        m_pMacroBT->Enable(bMacro);

        EnableInsert( bInsert );
    }

    return 0;
}

IMPL_LINK_NOARG(SwFieldFuncPage, SelectHdl)
{
    const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

    if( TYP_MACROFLD == nTypeId )
        m_pNameED->SetText( m_pSelectionLB->GetSelectEntry() );

    return 0;
}

IMPL_LINK_NOARG_TYPED(SwFieldFuncPage, InsertMacroHdl, ListBox&, void)
{
    SelectHdl(NULL);
    InsertHdl(nullptr);
}

IMPL_LINK_TYPED( SwFieldFuncPage, ListModifyButtonHdl, Button*, pControl, void)
{
    ListModifyHdl(pControl);
}
IMPL_LINK_TYPED( SwFieldFuncPage, ListModifyReturnActionHdl, ReturnActionEdit&, rControl, void)
{
    ListModifyHdl(&rControl);
}
void SwFieldFuncPage::ListModifyHdl(Control* pControl)
{
    m_pListItemsLB->SetUpdateMode(false);
    if(pControl == m_pListAddPB ||
            (pControl == m_pListItemED && m_pListAddPB->IsEnabled()))
    {
        const OUString sEntry(m_pListItemED->GetText());
        m_pListItemsLB->InsertEntry(sEntry);
        m_pListItemsLB->SelectEntry(sEntry);
    }
    else if(m_pListItemsLB->GetSelectEntryCount())
    {
        sal_Int32 nSelPos = m_pListItemsLB->GetSelectEntryPos();
        if(pControl == m_pListRemovePB)
        {
            m_pListItemsLB->RemoveEntry(nSelPos);
            m_pListItemsLB->SelectEntryPos(nSelPos ? nSelPos - 1 : 0);
        }
        else if(pControl == m_pListUpPB)
        {
            if(nSelPos)
            {
                const OUString sEntry = m_pListItemsLB->GetSelectEntry();
                m_pListItemsLB->RemoveEntry(nSelPos);
                nSelPos--;
                m_pListItemsLB->InsertEntry(sEntry, nSelPos);
                m_pListItemsLB->SelectEntryPos(nSelPos);
            }
        }
        else if(pControl == m_pListDownPB)
        {
            if(nSelPos < m_pListItemsLB->GetEntryCount() - 1)
            {
                const OUString sEntry = m_pListItemsLB->GetSelectEntry();
                m_pListItemsLB->RemoveEntry(nSelPos);
                nSelPos++;
                m_pListItemsLB->InsertEntry(sEntry, nSelPos);
                m_pListItemsLB->SelectEntryPos(nSelPos);
            }
        }
    }
    bDropDownLBChanged = true;
    m_pListItemsLB->SetUpdateMode(true);
    ListEnableHdl(0);
}

IMPL_LINK_NOARG(SwFieldFuncPage, ListEnableHdl)
{
    //enable "Add" button when text is in the Edit that's not already member of the box
    m_pListAddPB->Enable(!m_pListItemED->GetText().isEmpty() &&
                LISTBOX_ENTRY_NOTFOUND == m_pListItemsLB->GetEntryPos(m_pListItemED->GetText()));
    bool bEnableButtons = m_pListItemsLB->GetSelectEntryCount() > 0;
    m_pListRemovePB->Enable(bEnableButtons);
    m_pListUpPB->Enable(bEnableButtons && (m_pListItemsLB->GetSelectEntryPos() > 0));
    m_pListDownPB->Enable(bEnableButtons &&
                (m_pListItemsLB->GetSelectEntryPos() < (m_pListItemsLB->GetEntryCount() - 1)));

    return 0;
}

// renew types in SelectionBox
void SwFieldFuncPage::UpdateSubType()
{
    const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

    // fill Selection-Listbox
    m_pSelectionLB->SetUpdateMode(false);
    m_pSelectionLB->Clear();

    std::vector<OUString> aLst;
    GetFieldMgr().GetSubTypes(nTypeId, aLst);
    const size_t nCount = aLst.size();

    for(size_t i = 0; i < nCount; ++i)
    {
        size_t nPos = m_pSelectionLB->InsertEntry(aLst[i]);
        m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
    }

    bool bEnable = nCount != 0;

    m_pSelectionLB->Enable( bEnable );

    if (bEnable)
    {
            m_pSelectionLB->SelectEntryPos(0);
    }

    if (nTypeId == TYP_MACROFLD)
    {
        const bool bHasMacro = !GetFieldMgr().GetMacroPath().isEmpty();

        if (bHasMacro)
        {
            m_pNameED->SetText(GetFieldMgr().GetMacroName());
            m_pValueGroup->Enable();
        }
        EnableInsert(bHasMacro);
    }

    m_pSelectionLB->SetUpdateMode(true);
}

// call MacroBrowser, fill Listbox with Macros
IMPL_LINK_TYPED( SwFieldFuncPage, MacroHdl, Button *, pBtn, void )
{
    vcl::Window* pDefModalDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pBtn );
    const OUString sMacro(TurnMacroString(m_pNameED->GetText()).replaceAll(".", ";"));
    if (GetFieldMgr().ChooseMacro(sMacro))
        UpdateSubType();

    Application::SetDefDialogParent( pDefModalDlgParent );
}

bool SwFieldFuncPage::FillItemSet(SfxItemSet* )
{
    const sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

    sal_uInt16 nSubType = 0;

    const sal_Int32 nEntryPos = m_pFormatLB->GetSelectEntryPos();
    const sal_uLong nFormat = (nEntryPos == LISTBOX_ENTRY_NOTFOUND)
        ? 0 : reinterpret_cast<sal_uLong>(m_pFormatLB->GetEntryData(nEntryPos));

    OUString aVal(m_pValueED->GetText());
    OUString aName(m_pNameED->GetText());

    switch(nTypeId)
    {
        case TYP_INPUTFLD:
            nSubType = INP_TXT;
            // to prevent removal of CR/LF restore old content
            if(!m_pNameED->IsModified() && IsFieldEdit())
                aName = GetCurField()->GetPar1();

            break;

        case TYP_MACROFLD:
            // use the full script URL, not the name in the Edit control
            aName = GetFieldMgr().GetMacroPath();
            break;

        case TYP_CONDTXTFLD:
            aVal = m_pCond1ED->GetText() + "|" + m_pCond2ED->GetText();
            break;
        case TYP_DROPDOWN :
        {
            aName = m_pListNameED->GetText();
            for(sal_Int32 i = 0; i < m_pListItemsLB->GetEntryCount(); i++)
            {
                if(i)
                    aVal += OUString(DB_DELIM);
                aVal += m_pListItemsLB->GetEntry(i);
            }
        }
        break;
        default:
            break;
    }

    if (!IsFieldEdit() ||
        m_pNameED->IsValueChangedFromSaved() ||
        m_pValueED->IsValueChangedFromSaved() ||
        m_pCond1ED->IsValueChangedFromSaved() ||
        m_pCond2ED->IsValueChangedFromSaved() ||
        m_pListNameED->IsValueChangedFromSaved() ||
        bDropDownLBChanged ||
        nOldFormat != nFormat)
    {
        InsertField( nTypeId, nSubType, aName, aVal, nFormat );
    }

    ModifyHdl(NULL);    // enable/disable Insert if applicable

    return false;
}

OUString SwFieldFuncPage::TurnMacroString(const OUString &rMacro)
{
    if (!rMacro.isEmpty())
    {
        // reverse content of aName
        OUString sMacro;
        sal_Int32 nPos = 0;

        for (int i = 0; i < 4 && nPos != -1; i++)
        {
            OUString sTmp = (i == 3)
                ? rMacro.copy(nPos)
                : rMacro.getToken(0, '.', nPos);

            sMacro = !sMacro.isEmpty()
                ? sTmp + "." + sMacro
                : sTmp;
        }
        return sMacro;
    }

    return rMacro;
}

VclPtr<SfxTabPage> SwFieldFuncPage::Create( vcl::Window* pParent,
                                          const SfxItemSet* rAttrSet )
{
    return VclPtr<SwFieldFuncPage>::Create( pParent, *rAttrSet );
}

sal_uInt16 SwFieldFuncPage::GetGroup()
{
    return GRP_FKT;
}

void    SwFieldFuncPage::FillUserData()
{
    const sal_Int32 nEntryPos = m_pTypeLB->GetSelectEntryPos();
    const sal_uInt16 nTypeSel = ( LISTBOX_ENTRY_NOTFOUND == nEntryPos )
        ? USHRT_MAX
        : sal::static_int_cast< sal_uInt16 >
            (reinterpret_cast< sal_uIntPtr >(m_pTypeLB->GetEntryData( nEntryPos )));
    SetUserData(USER_DATA_VERSION ";" + OUString::number( nTypeSel ));
}

IMPL_LINK_NOARG(SwFieldFuncPage, ModifyHdl)
{
    const sal_Int32 nLen = m_pNameED->GetText().getLength();

    bool bEnable = true;
    sal_uInt16 nTypeId = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pTypeLB->GetEntryData(GetTypeSel()));

    if( TYP_COMBINED_CHARS == nTypeId &&
        (!nLen || nLen > MAX_COMBINED_CHARACTERS ))
        bEnable = false;

    EnableInsert( bEnable );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
