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


SwFldFuncPage::SwFldFuncPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SwFldPage(pParent, "FldFuncPage",
        "modules/swriter/ui/fldfuncpage.ui", rCoreSet)
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

    m_pNameED->SetModifyHdl(LINK(this, SwFldFuncPage, ModifyHdl));

    m_sOldValueFT = m_pValueFT->GetText();
    m_sOldNameFT = m_pNameFT->GetText();

    m_pCond1ED->ShowBrackets(false);
    m_pCond2ED->ShowBrackets(false);

}

SwFldFuncPage::~SwFldFuncPage()
{
}

void SwFldFuncPage::Reset(const SfxItemSet& )
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

        // fill Typ-Listbox
        for(short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = m_pTypeLB->InsertEntry(GetFldMgr().GetTypeStr(i));
            m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        nTypeId = GetCurField()->GetTypeId();
        nPos = m_pTypeLB->InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        m_pTypeLB->SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));

        if (nTypeId == TYP_MACROFLD)
        {
            String sName(GetCurField()->GetPar1());
            GetFldMgr().SetMacroPath(sName);
        }
    }

    // select old Pos
    RestorePos(m_pTypeLB);

    m_pTypeLB->SetDoubleClickHdl       (LINK(this, SwFldFuncPage, InsertHdl));
    m_pTypeLB->SetSelectHdl            (LINK(this, SwFldFuncPage, TypeHdl));
    m_pSelectionLB->SetSelectHdl       (LINK(this, SwFldFuncPage, SelectHdl));
    m_pSelectionLB->SetDoubleClickHdl  (LINK(this, SwFldFuncPage, InsertMacroHdl));
    m_pFormatLB->SetDoubleClickHdl     (LINK(this, SwFldFuncPage, InsertHdl));
    m_pMacroBT->SetClickHdl            (LINK(this, SwFldFuncPage, MacroHdl));
    Link aListModifyLk( LINK(this, SwFldFuncPage, ListModifyHdl));
    m_pListAddPB->SetClickHdl(aListModifyLk);
    m_pListRemovePB->SetClickHdl(aListModifyLk);
    m_pListUpPB->SetClickHdl(aListModifyLk);
    m_pListDownPB->SetClickHdl(aListModifyLk);
    m_pListItemED->SetReturnActionLink(aListModifyLk);
    Link aListEnableLk = LINK(this, SwFldFuncPage, ListEnableHdl);
    m_pListItemED->SetModifyHdl(aListEnableLk);
    m_pListItemsLB->SetSelectHdl(aListEnableLk);

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
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

    m_pTypeLB->SetUpdateMode(sal_True);

    if (IsFldEdit())
    {
        m_pNameED->SaveValue();
        m_pValueED->SaveValue();
        m_pCond1ED->SaveValue();
        m_pCond2ED->SaveValue();
        nOldFormat = GetCurField()->GetFormat();
    }
}

IMPL_LINK_NOARG(SwFldFuncPage, TypeHdl)
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

    if (nOld != GetTypeSel())
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

        // fill Selection-Listbox
        UpdateSubType();

        // fill Format-Listbox
        m_pFormatLB->Clear();

        sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());

        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            sal_uInt16 nPos = m_pFormatLB->InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
            m_pFormatLB->SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, i )) );
        }

        if (nSize)
        {
            if (IsFldEdit() && nTypeId == TYP_JUMPEDITFLD)
                m_pFormatLB->SelectEntry(SW_RESSTR(FMT_MARK_BEGIN + (sal_uInt16)GetCurField()->GetFormat()));

            if (!m_pFormatLB->GetSelectEntryCount())
                m_pFormatLB->SelectEntryPos(0);
        }

        sal_Bool bValue = sal_False, bName = sal_False, bMacro = sal_False, bInsert = sal_True;
        sal_Bool bFormat = nSize != 0;

        // two controls for conditional text
        sal_Bool bDropDown = TYP_DROPDOWN == nTypeId;
        sal_Bool bCondTxtFld = TYP_CONDTXTFLD == nTypeId;

        m_pCond1FT->Show(!bDropDown && bCondTxtFld);
        m_pCond1ED->Show(!bDropDown && bCondTxtFld);
        m_pCond2FT->Show(!bDropDown && bCondTxtFld);
        m_pCond2ED->Show(!bDropDown && bCondTxtFld);
        m_pValueGroup->Show(!bDropDown && !bCondTxtFld);
        m_pMacroBT->Show(!bDropDown);
        m_pNameED->Show(!bDropDown);
        m_pNameFT->Show(!bDropDown);

        m_pListGroup->Show(bDropDown);

        m_pNameED->SetDropEnable(false);

        if (IsFldEdit())
        {
            if(bDropDown)
            {
                const SwDropDownField* pDrop = (const SwDropDownField*)GetCurField();
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
            m_pNameED->SetText(aEmptyStr);
            m_pValueED->SetText(aEmptyStr);
        }
        if(bDropDown)
            ListEnableHdl(0);

        if( m_pNameFT->GetText() != OUString(m_sOldNameFT) )
            m_pNameFT->SetText(m_sOldNameFT);
        if (m_pValueFT->GetText() != OUString(m_sOldValueFT))
            m_pValueFT->SetText(m_sOldValueFT);

        switch (nTypeId)
        {
            case TYP_MACROFLD:
                bMacro = sal_True;
                if (!GetFldMgr().GetMacroPath().isEmpty())
                    bValue = sal_True;
                else
                    bInsert = sal_False;

                m_pNameFT->SetText(SW_RESSTR(STR_MACNAME));
                m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));
                m_pNameED->SetText(GetFldMgr().GetMacroName());
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_HIDDENPARAFLD:
                m_pNameFT->SetText(SW_RESSTR(STR_COND));
                m_pNameED->SetDropEnable(true);
                bName = sal_True;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_HIDDENTXTFLD:
            {
                m_pNameFT->SetText(SW_RESSTR(STR_COND));
                m_pNameED->SetDropEnable(true);
                m_pValueFT->SetText(SW_RESSTR(STR_INSTEXT));
                SwWrtShell* pSh = GetActiveWrtShell();
                if (!IsFldEdit() && pSh )
                    m_pValueED->SetText(pSh->GetSelTxt());
                bName = bValue = sal_True;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
            }
            break;

            case TYP_CONDTXTFLD:
                m_pNameFT->SetText(SW_RESSTR(STR_COND));
                m_pNameED->SetDropEnable(true);
                if (IsFldEdit())
                {
                    m_pCond1ED->SetText(GetCurField()->GetPar2().getToken(0, '|'));
                    m_pCond2ED->SetText(GetCurField()->GetPar2().getToken(1, '|'));
                }

                bName = bValue = sal_True;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_JUMPEDITFLD:
                m_pNameFT->SetText(SW_RESSTR(STR_JUMPEDITFLD));
                m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));
                bName = bValue = sal_True;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_INPUTFLD:
                m_pValueFT->SetText(SW_RESSTR(STR_PROMPT));
                bValue = sal_True;
                m_pNameED->SetAccessibleName(m_pNameFT->GetText());
                m_pValueED->SetAccessibleName(m_pValueFT->GetText());
                break;

            case TYP_COMBINED_CHARS:
                {
                    m_pNameFT->SetText(SW_RESSTR(STR_COMBCHRS_FT));
                    m_pNameED->SetDropEnable(true);
                    bName = sal_True;

                    const sal_Int32 nLen = m_pNameED->GetText().getLength();
                    if( !nLen || nLen > MAX_COMBINED_CHARACTERS )
                        bInsert = sal_False;
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

IMPL_LINK_NOARG(SwFldFuncPage, SelectHdl)
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    if( TYP_MACROFLD == nTypeId )
        m_pNameED->SetText( m_pSelectionLB->GetSelectEntry() );

    return 0;
}

IMPL_LINK_NOARG(SwFldFuncPage, InsertMacroHdl)
{
    SelectHdl();
    InsertHdl();

    return 0;
}

IMPL_LINK( SwFldFuncPage, ListModifyHdl, Control*, pControl)
{
    m_pListItemsLB->SetUpdateMode(sal_False);
    if(pControl == m_pListAddPB ||
            (pControl == m_pListItemED && m_pListAddPB->IsEnabled()))
    {
        String sEntry(m_pListItemED->GetText());
        m_pListItemsLB->InsertEntry(sEntry);
        m_pListItemsLB->SelectEntry(sEntry);
    }
    else if(m_pListItemsLB->GetSelectEntryCount())
    {
        sal_uInt16 nSelPos = m_pListItemsLB->GetSelectEntryPos();
        if(pControl == m_pListRemovePB)
        {
            m_pListItemsLB->RemoveEntry(nSelPos);
            m_pListItemsLB->SelectEntryPos(nSelPos ? nSelPos - 1 : 0);
        }
        else if(pControl == m_pListUpPB)
        {
            if(nSelPos)
            {
                String sEntry = m_pListItemsLB->GetSelectEntry();
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
                String sEntry = m_pListItemsLB->GetSelectEntry();
                m_pListItemsLB->RemoveEntry(nSelPos);
                nSelPos++;
                m_pListItemsLB->InsertEntry(sEntry, nSelPos);
                m_pListItemsLB->SelectEntryPos(nSelPos);
            }
        }
    }
    bDropDownLBChanged = true;
    m_pListItemsLB->SetUpdateMode(sal_True);
    ListEnableHdl(0);
    return 0;
}

IMPL_LINK_NOARG(SwFldFuncPage, ListEnableHdl)
{
    //enable "Add" button when text is in the Edit that's not already member of the box
    m_pListAddPB->Enable(!m_pListItemED->GetText().isEmpty() &&
                LISTBOX_ENTRY_NOTFOUND == m_pListItemsLB->GetEntryPos(m_pListItemED->GetText()));
    sal_Bool bEnableButtons = m_pListItemsLB->GetSelectEntryCount() > 0;
    m_pListRemovePB->Enable(bEnableButtons);
    m_pListUpPB->Enable(bEnableButtons && (m_pListItemsLB->GetSelectEntryPos() > 0));
    m_pListDownPB->Enable(bEnableButtons &&
                (m_pListItemsLB->GetSelectEntryPos() < (m_pListItemsLB->GetEntryCount() - 1)));

    return 0;
}

/*--------------------------------------------------------------------
     Description: renew types in SelectionBox
 --------------------------------------------------------------------*/
void SwFldFuncPage::UpdateSubType()
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    // fill Selction-Listbox
    m_pSelectionLB->SetUpdateMode(sal_False);
    m_pSelectionLB->Clear();

    std::vector<OUString> aLst;
    GetFldMgr().GetSubTypes(nTypeId, aLst);
    size_t nCount = aLst.size();

    for(size_t i = 0; i < nCount; ++i)
    {
        size_t nPos = m_pSelectionLB->InsertEntry(aLst[i]);
        m_pSelectionLB->SetEntryData(nPos, reinterpret_cast<void*>(i));
    }

    sal_Bool bEnable = nCount != 0;

    m_pSelectionLB->Enable( bEnable );

    if (bEnable)
    {
            m_pSelectionLB->SelectEntryPos(0);
    }

    if (nTypeId == TYP_MACROFLD)
    {
        const bool bHasMacro = !GetFldMgr().GetMacroPath().isEmpty();

        if (bHasMacro)
        {
            m_pNameED->SetText(GetFldMgr().GetMacroName());
            m_pValueGroup->Enable();
        }
        EnableInsert(bHasMacro);
    }

    m_pSelectionLB->SetUpdateMode(sal_True);
}

/*--------------------------------------------------------------------
    Description: call MacroBrowser, fill Listbox with Macros
 --------------------------------------------------------------------*/
IMPL_LINK( SwFldFuncPage, MacroHdl, Button *, pBtn )
{
    Window* pDefModalDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pBtn );

    String sMacro(TurnMacroString(m_pNameED->GetText()));
    while (sMacro.SearchAndReplace('.', ';') != STRING_NOTFOUND) ;

    if (GetFldMgr().ChooseMacro(sMacro))
        UpdateSubType();

    Application::SetDefDialogParent( pDefModalDlgParent );

    return 0;
}

sal_Bool SwFldFuncPage::FillItemSet(SfxItemSet& )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    sal_uInt16 nSubType = 0;

    sal_uLong nFormat = m_pFormatLB->GetSelectEntryPos();

    if(nFormat == LISTBOX_ENTRY_NOTFOUND)
        nFormat = 0;
    else
        nFormat = (sal_uLong)m_pFormatLB->GetEntryData((sal_uInt16)nFormat);

    String aVal(m_pValueED->GetText());
    String aName(m_pNameED->GetText());

    switch(nTypeId)
    {
        case TYP_INPUTFLD:
            nSubType = INP_TXT;
            // to prevent removal of CR/LF restore old content
            if(!m_pNameED->IsModified() && IsFldEdit())
                aName = GetCurField()->GetPar1();

            break;

        case TYP_MACROFLD:
            // use the full script URL, not the name in the Edit control
            aName = GetFldMgr().GetMacroPath();
            break;

        case TYP_CONDTXTFLD:
            aVal = m_pCond1ED->GetText();
            aVal += '|';
            aVal += m_pCond2ED->GetText();
            break;
        case TYP_DROPDOWN :
        {
            aName = m_pListNameED->GetText();
            for(sal_uInt16 i = 0; i < m_pListItemsLB->GetEntryCount(); i++)
            {
                if(i)
                    aVal += DB_DELIM;
                aVal += m_pListItemsLB->GetEntry(i);
            }
        }
        break;
        default:
            break;
    }

    if (!IsFldEdit() ||
        m_pNameED->GetSavedValue() != m_pNameED->GetText() ||
        m_pValueED->GetSavedValue() != m_pValueED->GetText() ||
        m_pCond1ED->GetSavedValue() != m_pCond1ED->GetText() ||
        m_pCond2ED->GetSavedValue() != m_pCond2ED->GetText() ||
        m_pListNameED->GetSavedValue() != m_pListNameED->GetText() ||
        bDropDownLBChanged ||
        nOldFormat != nFormat)
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat );
    }

    ModifyHdl();    // enable/disable Insert if applicable

    return sal_False;
}

String SwFldFuncPage::TurnMacroString(const String &rMacro)
{
    if (rMacro.Len())
    {
        // reverse content of aName
        String sTmp, sBuf;
        sal_Int32 nPos = 0;

        for (sal_uInt16 i = 0; i < 4 && nPos != -1; i++)
        {
            if (i == 3)
                sTmp = rMacro.Copy(nPos);
            else
                sTmp = rMacro.GetToken(0, '.', nPos);

            if( sBuf.Len() )
                sTmp += '.';
            sBuf.Insert( sTmp, 0 );
        }
        return sBuf;
    }

    return rMacro;
}

SfxTabPage* SwFldFuncPage::Create(  Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldFuncPage( pParent, rAttrSet ) );
}

sal_uInt16 SwFldFuncPage::GetGroup()
{
    return GRP_FKT;
}

void    SwFldFuncPage::FillUserData()
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

IMPL_LINK_NOARG(SwFldFuncPage, ModifyHdl)
{
    String aName(m_pNameED->GetText());
    const sal_uInt16 nLen = aName.Len();

    sal_Bool bEnable = sal_True;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)m_pTypeLB->GetEntryData(GetTypeSel());

    if( TYP_COMBINED_CHARS == nTypeId &&
        (!nLen || nLen > MAX_COMBINED_CHARACTERS ))
        bEnable = sal_False;

    EnableInsert( bEnable );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
