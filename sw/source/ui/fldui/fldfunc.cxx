/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


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

using rtl::OUString;

SwFldFuncPage::SwFldFuncPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage( pParent, SW_RES( TP_FLD_FUNC ), rCoreSet ),

    aTypeFT     (this, SW_RES(FT_FUNCTYPE)),
    aTypeLB     (this, SW_RES(LB_FUNCTYPE)),
    aSelectionFT(this, SW_RES(FT_FUNCSELECTION)),
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

    aCond1ED.ShowBrackets(FALSE);
    aCond2ED.ShowBrackets(FALSE);

}

SwFldFuncPage::~SwFldFuncPage()
{
}

void SwFldFuncPage::Reset(const SfxItemSet& )
{
    SavePos(&aTypeLB);
    Init(); // general initialisation

    aTypeLB.SetUpdateMode(FALSE);
    aTypeLB.Clear();

    USHORT nPos, nTypeId;

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
            USHORT nVal = static_cast< USHORT >(sVal.ToInt32());
            if(nVal != USHRT_MAX)
            {
                for(USHORT i = 0; i < aTypeLB.GetEntryCount(); i++)
                    if(nVal == (USHORT)(ULONG)aTypeLB.GetEntryData(i))
                    {
                        aTypeLB.SelectEntryPos(i);
                        break;
                    }
            }
        }
    }
    TypeHdl(0);

    aTypeLB.SetUpdateMode(TRUE);

    if (IsFldEdit())
    {
        aNameED.SaveValue();
        aValueED.SaveValue();
        aCond1ED.SaveValue();
        aCond2ED.SaveValue();
        nOldFormat = GetCurField()->GetFormat();
    }
}

IMPL_LINK( SwFldFuncPage, TypeHdl, ListBox *, EMPTYARG )
{
    // save old ListBoxPos
    const USHORT nOld = GetTypeSel();

    // current ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        aTypeLB.SelectEntryPos(0);
    }

    if (nOld != GetTypeSel())
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        // fill Selection-Listbox
        UpdateSubType();

        // fill Format-Listbox
        aFormatLB.Clear();

        USHORT nSize = GetFldMgr().GetFormatCount(nTypeId, FALSE, IsFldDlgHtmlMode());

        for (USHORT i = 0; i < nSize; i++)
        {
            USHORT nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
            aFormatLB.SetEntryData( nPos, reinterpret_cast<void*>(GetFldMgr().GetFormatId( nTypeId, i )) );
        }

        if (nSize)
        {
            if (IsFldEdit() && nTypeId == TYP_JUMPEDITFLD)
                aFormatLB.SelectEntry(SW_RESSTR(FMT_MARK_BEGIN + (USHORT)GetCurField()->GetFormat()));

            if (!aFormatLB.GetSelectEntryCount())
                aFormatLB.SelectEntryPos(0);
        }

        BOOL bValue = FALSE, bName = FALSE, bMacro = FALSE, bInsert = TRUE;
        BOOL bShowSelection = FALSE;
        BOOL bFormat = nSize != 0;

        // two controls for conditional text
        BOOL bDropDown = TYP_DROPDOWN == nTypeId;
        BOOL bCondTxtFld = TYP_CONDTXTFLD == nTypeId;

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

        aNameED.SetDropEnable(FALSE);

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

        if( aNameFT.GetText() != sOldNameFT )
            aNameFT.SetText(sOldNameFT);
        if (aValueFT.GetText() != sOldValueFT)
            aValueFT.SetText(sOldValueFT);

        switch (nTypeId)
        {
            case TYP_MACROFLD:
                bMacro = TRUE;
                if (GetFldMgr().GetMacroPath().Len())
                    bValue = TRUE;
                else
                    bInsert = FALSE;

                aNameFT.SetText(SW_RESSTR(STR_MACNAME));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                aNameED.SetText(GetFldMgr().GetMacroName());
                break;

            case TYP_HIDDENPARAFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(TRUE);
                bName = TRUE;
                break;

            case TYP_HIDDENTXTFLD:
            {
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(TRUE);
                aValueFT.SetText(SW_RESSTR(STR_INSTEXT));
                SwWrtShell* pSh = GetActiveWrtShell();
                if (!IsFldEdit() && pSh )
                    aValueED.SetText(pSh->GetSelTxt());
                bName = bValue = TRUE;
            }
            break;

            case TYP_CONDTXTFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(TRUE);
                if (IsFldEdit())
                {
                    aCond1ED.SetText(GetCurField()->GetPar2().GetToken(0, '|'));
                    aCond2ED.SetText(GetCurField()->GetPar2().GetToken(1, '|'));
                }

                bName = bValue = TRUE;
                break;

            case TYP_JUMPEDITFLD:
                aNameFT.SetText(SW_RESSTR(STR_JUMPEDITFLD));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bName = bValue = TRUE;
                break;

            case TYP_INPUTFLD:
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bValue = TRUE;
                break;

            case TYP_COMBINED_CHARS:
                {
                    aNameFT.SetText(SW_RESSTR(STR_COMBCHRS_FT));
                    aNameED.SetDropEnable(TRUE);
                    bName = TRUE;

                    const USHORT nLen = aNameED.GetText().Len();
                    if( !nLen || nLen > MAX_COMBINED_CHARACTERS )
                        bInsert = FALSE;
                }
                break;
            case TYP_DROPDOWN :
            break;
            default:
                break;
        }

        if (bShowSelection)
        {
            aSelectionLB.Show();
            aSelectionFT.Show();
            aFormatLB.Hide();
            aFormatFT.Hide();
        }
        else
        {
            aFormatLB.Show();
            aFormatFT.Show();
            aSelectionLB.Hide();
            aSelectionFT.Hide();
        }

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

IMPL_LINK( SwFldFuncPage, SelectHdl, ListBox *, EMPTYARG )
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    if( TYP_MACROFLD == nTypeId )
        aNameED.SetText( aSelectionLB.GetSelectEntry() );

    return 0;
}

IMPL_LINK( SwFldFuncPage, InsertMacroHdl, ListBox *, EMPTYARG )
{
    SelectHdl();
    InsertHdl();

    return 0;
}

IMPL_LINK( SwFldFuncPage, ListModifyHdl, Control*, pControl)
{
    aListItemsLB.SetUpdateMode(FALSE);
    if(pControl == &aListAddPB ||
            (pControl == &aListItemED && aListAddPB.IsEnabled()))
    {
        String sEntry(aListItemED.GetText());
        aListItemsLB.InsertEntry(sEntry);
        aListItemsLB.SelectEntry(sEntry);
    }
    else if(aListItemsLB.GetSelectEntryCount())
    {
        USHORT nSelPos = aListItemsLB.GetSelectEntryPos();
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
    aListItemsLB.SetUpdateMode(TRUE);
    ListEnableHdl(0);
    return 0;
}

IMPL_LINK( SwFldFuncPage, ListEnableHdl, void*, EMPTYARG)
{
    //enable "Add" button when text is in the Edit that's not already member of the box
    aListAddPB.Enable(aListItemED.GetText().Len() &&
                LISTBOX_ENTRY_NOTFOUND == aListItemsLB.GetEntryPos(aListItemED.GetText()));
    BOOL bEnableButtons = aListItemsLB.GetSelectEntryCount() > 0;
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
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    // fill Selction-Listbox
    aSelectionLB.SetUpdateMode(FALSE);
    aSelectionLB.Clear();

    SvStringsDtor aLst;
    GetFldMgr().GetSubTypes(nTypeId, aLst);
    USHORT nCount = aLst.Count();

    for (USHORT i = 0; i < nCount; ++i)
    {
        USHORT nPos = aSelectionLB.InsertEntry(*aLst[i]);
        aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
    }

    BOOL bEnable = nCount != 0;

    aSelectionLB.Enable( bEnable );
    aSelectionFT.Enable( bEnable );

    if (bEnable)
    {
            aSelectionLB.SelectEntryPos(0);
    }

    if (nTypeId == TYP_MACROFLD)
    {
        BOOL bHasMacro = GetFldMgr().GetMacroPath().Len() != 0;

        if (bHasMacro)
        {
            aNameED.SetText(GetFldMgr().GetMacroName());
            aValueFT.Enable();
            aValueED.Enable();
        }
        EnableInsert(bHasMacro);
    }

    aSelectionLB.SetUpdateMode(TRUE);
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

BOOL SwFldFuncPage::FillItemSet(SfxItemSet& )
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    USHORT nSubType = 0;

    ULONG nFormat = aFormatLB.GetSelectEntryPos();

    if(nFormat == LISTBOX_ENTRY_NOTFOUND)
        nFormat = 0;
    else
        nFormat = (ULONG)aFormatLB.GetEntryData((USHORT)nFormat);

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
            for(USHORT i = 0; i < aListItemsLB.GetEntryCount(); i++)
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

    return FALSE;
}

String SwFldFuncPage::TurnMacroString(const String &rMacro)
{
    if (rMacro.Len())
    {
        // reverse content of aName
        String sTmp, sBuf;
        USHORT nPos = 0;

        for (USHORT i = 0; i < 4 && nPos != STRING_NOTFOUND; i++)
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

USHORT SwFldFuncPage::GetGroup()
{
    return GRP_FKT;
}

void    SwFldFuncPage::FillUserData()
{
    String sData( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( USER_DATA_VERSION )));
    sData += ';';
    USHORT nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = sal::static_int_cast< USHORT >(reinterpret_cast< sal_uIntPtr >(aTypeLB.GetEntryData( nTypeSel )));
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

IMPL_LINK( SwFldFuncPage, ModifyHdl, Edit *, EMPTYARG )
{
    String aName(aNameED.GetText());
    const USHORT nLen = aName.Len();

    BOOL bEnable = TRUE;
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    if( TYP_COMBINED_CHARS == nTypeId &&
        (!nLen || nLen > MAX_COMBINED_CHARACTERS ))
        bEnable = FALSE;

    EnableInsert( bEnable );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
