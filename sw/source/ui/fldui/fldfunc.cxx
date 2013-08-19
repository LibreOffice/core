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
#include <fldtdlg.hrc>

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

using namespace ::com::sun::star;


SwFldFuncPage::SwFldFuncPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage( pParent, SW_RES( TP_FLD_FUNC ), rCoreSet ),

    aTypeFT     (this, SW_RES(FT_FUNCTYPE)),
    aTypeLB     (this, SW_RES(LB_FUNCTYPE)),
    aSelectionLB(this, SW_RES(LB_FUNCSELECTION)),
    aFormatFT   (this, SW_RES(FT_FUNCFORMAT)),
    aFormatLB   (this, SW_RES(LB_FUNCFORMAT)),
    aNameFT     (this, SW_RES(FT_FUNCNAME)),
    aNameED     (this, SW_RES(ED_FUNCNAME)),
    aValueFT    (this, SW_RES(FT_FUNCVALUE)),
    aValueED    (this, SW_RES(ED_FUNCVALUE)),
    aCond1FT    (this, SW_RES(FT_FUNCCOND1)),
    aCond1ED    (this, SW_RES(ED_FUNCCOND1)),
    aCond2FT    (this, SW_RES(FT_FUNCCOND2)),
    aCond2ED    (this, SW_RES(ED_FUNCCOND2)),
    aMacroBT    (this, SW_RES(BT_FUNCMACRO)),
    aListItemFT(    this, SW_RES( FT_LISTITEM    )),
    aListItemED(    this, SW_RES( ED_LISTITEM    )),
    aListAddPB(     this, SW_RES( PB_LISTADD     )),
    aListItemsFT(   this, SW_RES( FT_LISTITEMS   )),
    aListItemsLB(   this, SW_RES( LB_LISTITEMS   )),
    aListRemovePB(  this, SW_RES( PB_LISTREMOVE  )),
    aListUpPB(      this, SW_RES( PB_LISTUP      )),
    aListDownPB(    this, SW_RES( PB_LISTDOWN    )),
    aListNameFT(    this, SW_RES( FT_LISTNAME    )),
    aListNameED(    this, SW_RES( ED_LISTNAME    )),
    bDropDownLBChanged(false)
{
    FreeResource();

    aNameED.SetPosPixel(Point(aNameED.GetPosPixel().X(), aFormatLB.GetPosPixel().Y()));

    aNameED.SetModifyHdl(LINK(this, SwFldFuncPage, ModifyHdl));

    sOldValueFT = aValueFT.GetText();
    sOldNameFT = aNameFT.GetText();

    aCond1ED.ShowBrackets(false);
    aCond2ED.ShowBrackets(false);

}

SwFldFuncPage::~SwFldFuncPage()
{
}

void SwFldFuncPage::Reset(const SfxItemSet& )
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

        // fill Typ-Listbox
        for(short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
            aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));
        }
    }
    else
    {
        nTypeId = GetCurField()->GetTypeId();
        nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        aTypeLB.SetEntryData(nPos, reinterpret_cast<void*>(nTypeId));

        if (nTypeId == TYP_MACROFLD)
        {
            String sName(GetCurField()->GetPar1());
            GetFldMgr().SetMacroPath(sName);
        }
    }

    // select old Pos
    RestorePos(&aTypeLB);

    aTypeLB.SetDoubleClickHdl       (LINK(this, SwFldFuncPage, InsertHdl));
    aTypeLB.SetSelectHdl            (LINK(this, SwFldFuncPage, TypeHdl));
    aSelectionLB.SetSelectHdl       (LINK(this, SwFldFuncPage, SelectHdl));
    aSelectionLB.SetDoubleClickHdl  (LINK(this, SwFldFuncPage, InsertMacroHdl));
    aFormatLB.SetDoubleClickHdl     (LINK(this, SwFldFuncPage, InsertHdl));
    aMacroBT.SetClickHdl            (LINK(this, SwFldFuncPage, MacroHdl));
    Link aListModifyLk( LINK(this, SwFldFuncPage, ListModifyHdl));
    aListAddPB.SetClickHdl(aListModifyLk);
    aListRemovePB.SetClickHdl(aListModifyLk);
    aListUpPB.SetClickHdl(aListModifyLk);
    aListDownPB.SetClickHdl(aListModifyLk);
    aListItemED.SetReturnActionLink(aListModifyLk);
    Link aListEnableLk = LINK(this, SwFldFuncPage, ListEnableHdl);
    aListItemED.SetModifyHdl(aListEnableLk);
    aListItemsLB.SetSelectHdl(aListEnableLk);

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
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

    aTypeLB.SetUpdateMode(sal_True);

    if (IsFldEdit())
    {
        aNameED.SaveValue();
        aValueED.SaveValue();
        aCond1ED.SaveValue();
        aCond2ED.SaveValue();
        nOldFormat = GetCurField()->GetFormat();
    }
}

IMPL_LINK_NOARG(SwFldFuncPage, TypeHdl)
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

    if (nOld != GetTypeSel())
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

        // fill Selection-Listbox
        UpdateSubType();

        // fill Format-Listbox
        aFormatLB.Clear();

        sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, false, IsFldDlgHtmlMode());

        for (sal_uInt16 i = 0; i < nSize; i++)
        {
            sal_uInt16 nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
            aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, i )) );
        }

        if (nSize)
        {
            if (IsFldEdit() && nTypeId == TYP_JUMPEDITFLD)
                aFormatLB.SelectEntry(SW_RESSTR(FMT_MARK_BEGIN + (sal_uInt16)GetCurField()->GetFormat()));

            if (!aFormatLB.GetSelectEntryCount())
                aFormatLB.SelectEntryPos(0);
        }

        sal_Bool bValue = sal_False, bName = sal_False, bMacro = sal_False, bInsert = sal_True;
        sal_Bool bFormat = nSize != 0;

        // two controls for conditional text
        sal_Bool bDropDown = TYP_DROPDOWN == nTypeId;
        sal_Bool bCondTxtFld = TYP_CONDTXTFLD == nTypeId;

        aCond1FT.Show(!bDropDown && bCondTxtFld);
        aCond1ED.Show(!bDropDown && bCondTxtFld);
        aCond2FT.Show(!bDropDown && bCondTxtFld);
        aCond2ED.Show(!bDropDown && bCondTxtFld);
        aValueFT.Show(!bDropDown && !bCondTxtFld);
        aValueED.Show(!bDropDown && !bCondTxtFld);
        aMacroBT.Show(!bDropDown);
        aNameED.Show(!bDropDown);
        aNameFT.Show(!bDropDown);

        aListItemFT.Show(bDropDown);
        aListItemED.Show(bDropDown);
        aListAddPB.Show(bDropDown);
        aListItemsFT.Show(bDropDown);
        aListItemsLB.Show(bDropDown);
        aListRemovePB.Show(bDropDown);
        aListUpPB.Show(bDropDown);
        aListDownPB.Show(bDropDown);
        aListNameFT.Show(bDropDown);
        aListNameED.Show(bDropDown);

        aNameED.SetDropEnable(false);

        if (IsFldEdit())
        {
            if(bDropDown)
            {
                const SwDropDownField* pDrop = (const SwDropDownField*)GetCurField();
                uno::Sequence<OUString> aItems = pDrop->GetItemSequence();
                const OUString* pArray = aItems.getConstArray();
                aListItemsLB.Clear();
                for(sal_Int32 i = 0; i < aItems.getLength(); i++)
                    aListItemsLB.InsertEntry(pArray[i]);
                aListItemsLB.SelectEntry(pDrop->GetSelectedItem());
                aListNameED.SetText(pDrop->GetPar2());
                aListNameED.SaveValue();
                bDropDownLBChanged = false;
            }
            else
            {
                aNameED.SetText(GetCurField()->GetPar1());
                aValueED.SetText(GetCurField()->GetPar2());
            }
        }
        else
        {
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
        }
        if(bDropDown)
            ListEnableHdl(0);

        if( aNameFT.GetText() != OUString(sOldNameFT) )
            aNameFT.SetText(sOldNameFT);
        if (aValueFT.GetText() != OUString(sOldValueFT))
            aValueFT.SetText(sOldValueFT);

        switch (nTypeId)
        {
            case TYP_MACROFLD:
                bMacro = sal_True;
                if (!GetFldMgr().GetMacroPath().isEmpty())
                    bValue = sal_True;
                else
                    bInsert = sal_False;

                aNameFT.SetText(SW_RESSTR(STR_MACNAME));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                aNameED.SetText(GetFldMgr().GetMacroName());
                aNameED.SetAccessibleName(aNameFT.GetText());
                aValueED.SetAccessibleName(aValueFT.GetText());
                break;

            case TYP_HIDDENPARAFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(true);
                bName = sal_True;
                aNameED.SetAccessibleName(aNameFT.GetText());
                aValueED.SetAccessibleName(aValueFT.GetText());
                break;

            case TYP_HIDDENTXTFLD:
            {
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(true);
                aValueFT.SetText(SW_RESSTR(STR_INSTEXT));
                SwWrtShell* pSh = GetActiveWrtShell();
                if (!IsFldEdit() && pSh )
                    aValueED.SetText(pSh->GetSelTxt());
                bName = bValue = sal_True;
                aNameED.SetAccessibleName(aNameFT.GetText());
                aValueED.SetAccessibleName(aValueFT.GetText());
            }
            break;

            case TYP_CONDTXTFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(true);
                if (IsFldEdit())
                {
                    aCond1ED.SetText(GetCurField()->GetPar2().getToken(0, '|'));
                    aCond2ED.SetText(GetCurField()->GetPar2().getToken(1, '|'));
                }

                bName = bValue = sal_True;
                aNameED.SetAccessibleName(aNameFT.GetText());
                aValueED.SetAccessibleName(aValueFT.GetText());
                break;

            case TYP_JUMPEDITFLD:
                aNameFT.SetText(SW_RESSTR(STR_JUMPEDITFLD));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bName = bValue = sal_True;
                aNameED.SetAccessibleName(aNameFT.GetText());
                aValueED.SetAccessibleName(aValueFT.GetText());
                break;

            case TYP_INPUTFLD:
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bValue = sal_True;
                aNameED.SetAccessibleName(aNameFT.GetText());
                aValueED.SetAccessibleName(aValueFT.GetText());
                break;

            case TYP_COMBINED_CHARS:
                {
                    aNameFT.SetText(SW_RESSTR(STR_COMBCHRS_FT));
                    aNameED.SetDropEnable(true);
                    bName = sal_True;

                    const sal_Int32 nLen = aNameED.GetText().getLength();
                    if( !nLen || nLen > MAX_COMBINED_CHARACTERS )
                        bInsert = sal_False;
                    aNameED.SetAccessibleName(aNameFT.GetText());
                    aValueED.SetAccessibleName(aValueFT.GetText());
                }
                break;
            case TYP_DROPDOWN :
            break;
            default:
                break;
        }

        aFormatLB.Show();
        aFormatFT.Show();
        aSelectionLB.Hide();

        aFormatLB.Enable(bFormat);
        aFormatFT.Enable(bFormat);
        aNameFT.Enable(bName);
        aNameED.Enable(bName);
        aValueFT.Enable(bValue);
        aValueED.Enable(bValue);
        aMacroBT.Enable(bMacro);

        EnableInsert( bInsert );
    }

    return 0;
}

IMPL_LINK_NOARG(SwFldFuncPage, SelectHdl)
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    if( TYP_MACROFLD == nTypeId )
        aNameED.SetText( aSelectionLB.GetSelectEntry() );

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
    aListItemsLB.SetUpdateMode(sal_False);
    if(pControl == &aListAddPB ||
            (pControl == &aListItemED && aListAddPB.IsEnabled()))
    {
        String sEntry(aListItemED.GetText());
        aListItemsLB.InsertEntry(sEntry);
        aListItemsLB.SelectEntry(sEntry);
    }
    else if(aListItemsLB.GetSelectEntryCount())
    {
        sal_uInt16 nSelPos = aListItemsLB.GetSelectEntryPos();
        if(pControl == &aListRemovePB)
        {
            aListItemsLB.RemoveEntry(nSelPos);
            aListItemsLB.SelectEntryPos(nSelPos ? nSelPos - 1 : 0);
        }
        else if(pControl == &aListUpPB)
        {
            if(nSelPos)
            {
                String sEntry = aListItemsLB.GetSelectEntry();
                aListItemsLB.RemoveEntry(nSelPos);
                nSelPos--;
                aListItemsLB.InsertEntry(sEntry, nSelPos);
                aListItemsLB.SelectEntryPos(nSelPos);
            }
        }
        else if(pControl == &aListDownPB)
        {
            if(nSelPos < aListItemsLB.GetEntryCount() - 1)
            {
                String sEntry = aListItemsLB.GetSelectEntry();
                aListItemsLB.RemoveEntry(nSelPos);
                nSelPos++;
                aListItemsLB.InsertEntry(sEntry, nSelPos);
                aListItemsLB.SelectEntryPos(nSelPos);
            }
        }
    }
    bDropDownLBChanged = true;
    aListItemsLB.SetUpdateMode(sal_True);
    ListEnableHdl(0);
    return 0;
}

IMPL_LINK_NOARG(SwFldFuncPage, ListEnableHdl)
{
    //enable "Add" button when text is in the Edit that's not already member of the box
    aListAddPB.Enable(!aListItemED.GetText().isEmpty() &&
                LISTBOX_ENTRY_NOTFOUND == aListItemsLB.GetEntryPos(aListItemED.GetText()));
    sal_Bool bEnableButtons = aListItemsLB.GetSelectEntryCount() > 0;
    aListRemovePB.Enable(bEnableButtons);
    aListUpPB.Enable(bEnableButtons && (aListItemsLB.GetSelectEntryPos() > 0));
    aListDownPB.Enable(bEnableButtons &&
                (aListItemsLB.GetSelectEntryPos() < (aListItemsLB.GetEntryCount() - 1)));

    return 0;
}

/*--------------------------------------------------------------------
     Description: renew types in SelectionBox
 --------------------------------------------------------------------*/
void SwFldFuncPage::UpdateSubType()
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    // fill Selction-Listbox
    aSelectionLB.SetUpdateMode(sal_False);
    aSelectionLB.Clear();

    std::vector<OUString> aLst;
    GetFldMgr().GetSubTypes(nTypeId, aLst);
    size_t nCount = aLst.size();

    for(size_t i = 0; i < nCount; ++i)
    {
        size_t nPos = aSelectionLB.InsertEntry(aLst[i]);
        aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
    }

    sal_Bool bEnable = nCount != 0;

    aSelectionLB.Enable( bEnable );

    if (bEnable)
    {
            aSelectionLB.SelectEntryPos(0);
    }

    if (nTypeId == TYP_MACROFLD)
    {
        const bool bHasMacro = !GetFldMgr().GetMacroPath().isEmpty();

        if (bHasMacro)
        {
            aNameED.SetText(GetFldMgr().GetMacroName());
            aValueFT.Enable();
            aValueED.Enable();
        }
        EnableInsert(bHasMacro);
    }

    aSelectionLB.SetUpdateMode(sal_True);
}

/*--------------------------------------------------------------------
    Description: call MacroBrowser, fill Listbox with Macros
 --------------------------------------------------------------------*/
IMPL_LINK( SwFldFuncPage, MacroHdl, Button *, pBtn )
{
    Window* pDefModalDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pBtn );

    String sMacro(TurnMacroString(aNameED.GetText()));
    while (sMacro.SearchAndReplace('.', ';') != STRING_NOTFOUND) ;

    if (GetFldMgr().ChooseMacro(sMacro))
        UpdateSubType();

    Application::SetDefDialogParent( pDefModalDlgParent );

    return 0;
}

sal_Bool SwFldFuncPage::FillItemSet(SfxItemSet& )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    sal_uInt16 nSubType = 0;

    sal_uLong nFormat = aFormatLB.GetSelectEntryPos();

    if(nFormat == LISTBOX_ENTRY_NOTFOUND)
        nFormat = 0;
    else
        nFormat = (sal_uLong)aFormatLB.GetEntryData((sal_uInt16)nFormat);

    String aVal(aValueED.GetText());
    String aName(aNameED.GetText());

    switch(nTypeId)
    {
        case TYP_INPUTFLD:
            nSubType = INP_TXT;
            // to prevent removal of CR/LF restore old content
            if(!aNameED.IsModified() && IsFldEdit())
                aName = GetCurField()->GetPar1();

            break;

        case TYP_MACROFLD:
            // use the full script URL, not the name in the Edit control
            aName = GetFldMgr().GetMacroPath();
            break;

        case TYP_CONDTXTFLD:
            aVal = aCond1ED.GetText();
            aVal += '|';
            aVal += aCond2ED.GetText();
            break;
        case TYP_DROPDOWN :
        {
            aName = aListNameED.GetText();
            for(sal_uInt16 i = 0; i < aListItemsLB.GetEntryCount(); i++)
            {
                if(i)
                    aVal += DB_DELIM;
                aVal += aListItemsLB.GetEntry(i);
            }
        }
        break;
        default:
            break;
    }

    if (!IsFldEdit() ||
        aNameED.GetSavedValue() != aNameED.GetText() ||
        aValueED.GetSavedValue() != aValueED.GetText() ||
        aCond1ED.GetSavedValue() != aCond1ED.GetText() ||
        aCond2ED.GetSavedValue() != aCond2ED.GetText() ||
        aListNameED.GetSavedValue() != aListNameED.GetText() ||
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
    sal_uInt16 nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(aTypeLB.GetEntryData( nTypeSel )));
    sData += OUString::number( nTypeSel );
    SetUserData(sData);
}

IMPL_LINK_NOARG(SwFldFuncPage, ModifyHdl)
{
    String aName(aNameED.GetText());
    const sal_uInt16 nLen = aName.Len();

    sal_Bool bEnable = sal_True;
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    if( TYP_COMBINED_CHARS == nTypeId &&
        (!nLen || nLen > MAX_COMBINED_CHARACTERS ))
        bEnable = sal_False;

    EnableInsert( bEnable );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
