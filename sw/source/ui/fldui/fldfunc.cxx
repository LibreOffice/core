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
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#include <fldbas.hxx>
#include <docufld.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <wrtsh.hxx>
#include <swmodule.hxx>
#ifndef _FLDFUNC_HXX
#include <fldfunc.hxx>
#endif
#include <flddropdown.hxx>
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1

using namespace ::com::sun::star;

using rtl::OUString;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

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

    aCond1ED.ShowBrackets(sal_False);
    aCond2ED.ShowBrackets(sal_False);

//  SwWrtShell* pSh = (SwWrtShell*)ViewShell::GetCurrShell();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldFuncPage::~SwFldFuncPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldFuncPage::Reset(const SfxItemSet& )
{
    SavePos(&aTypeLB);
    Init(); // Allgemeine initialisierung

    aTypeLB.SetUpdateMode(sal_False);
    aTypeLB.Clear();

    sal_uInt16 nPos, nTypeId;

    if (!IsFldEdit())
    {
        // TypeListBox initialisieren
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        // Typ-Listbox fuellen
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

    // alte Pos selektieren
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldFuncPage, TypeHdl, ListBox *, EMPTYARG )
{
    // Alte ListBoxPos sichern
    const sal_uInt16 nOld = GetTypeSel();

    // Aktuelle ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        aTypeLB.SelectEntryPos(0);
    }

    if (nOld != GetTypeSel())
    {
        sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

        // Auswahl-Listbox fuellen
        UpdateSubType();

        // Format-Listbox fuellen
        aFormatLB.Clear();

        sal_uInt16 nSize = GetFldMgr().GetFormatCount(nTypeId, sal_False, IsFldDlgHtmlMode());

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
        sal_Bool bShowSelection = sal_False;
        sal_Bool bFormat = nSize != 0;

        // fuer Conditional Text zwei Controls
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

        aNameED.SetDropEnable(sal_False);

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
                bMacro = sal_True;
                if (GetFldMgr().GetMacroPath().Len())
                    bValue = sal_True;
                else
                    bInsert = sal_False;

                aNameFT.SetText(SW_RESSTR(STR_MACNAME));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                aNameED.SetText(GetFldMgr().GetMacroName());
                break;

            case TYP_HIDDENPARAFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(sal_True);
                bName = sal_True;
                break;

            case TYP_HIDDENTXTFLD:
            {
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(sal_True);
                aValueFT.SetText(SW_RESSTR(STR_INSTEXT));
                SwWrtShell* pSh = GetActiveWrtShell();
                if (!IsFldEdit() && pSh )
                    aValueED.SetText(pSh->GetSelTxt());
                bName = bValue = sal_True;
            }
            break;

            case TYP_CONDTXTFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.SetDropEnable(sal_True);
                if (IsFldEdit())
                {
                    aCond1ED.SetText(GetCurField()->GetPar2().GetToken(0, '|'));
                    aCond2ED.SetText(GetCurField()->GetPar2().GetToken(1, '|'));
                }

                bName = bValue = sal_True;
                break;

            case TYP_JUMPEDITFLD:
                aNameFT.SetText(SW_RESSTR(STR_JUMPEDITFLD));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bName = bValue = sal_True;
                break;

            case TYP_INPUTFLD:
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bValue = sal_True;
                // bShowSelection = sal_True;
                break;

            case TYP_COMBINED_CHARS:
                {
                    aNameFT.SetText(SW_RESSTR(STR_COMBCHRS_FT));
                    aNameED.SetDropEnable(sal_True);
                    bName = sal_True;

                    const sal_uInt16 nLen = aNameED.GetText().Len();
                    if( !nLen || nLen > MAX_COMBINED_CHARACTERS )
                        bInsert = sal_False;
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldFuncPage, SelectHdl, ListBox *, EMPTYARG )
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    if( TYP_MACROFLD == nTypeId )
        aNameED.SetText( aSelectionLB.GetSelectEntry() );

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldFuncPage, InsertMacroHdl, ListBox *, EMPTYARG )
{
    SelectHdl();
    InsertHdl();

    return 0;
}
/* -----------------16.06.2003 16:24-----------------

 --------------------------------------------------*/
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
/* -----------------17.06.2003 08:36-----------------

 --------------------------------------------------*/
IMPL_LINK( SwFldFuncPage, ListEnableHdl, void*, EMPTYARG)
{
    //enable "Add" button when text is in the Edit that's not already member of the box
    aListAddPB.Enable(aListItemED.GetText().Len() &&
                LISTBOX_ENTRY_NOTFOUND == aListItemsLB.GetEntryPos(aListItemED.GetText()));
    sal_Bool bEnableButtons = aListItemsLB.GetSelectEntryCount() > 0;
    aListRemovePB.Enable(bEnableButtons);
    aListUpPB.Enable(bEnableButtons && (aListItemsLB.GetSelectEntryPos() > 0));
    aListDownPB.Enable(bEnableButtons &&
                (aListItemsLB.GetSelectEntryPos() < (aListItemsLB.GetEntryCount() - 1)));

    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung: Typen in der SelectionBox erneuern
 --------------------------------------------------------------------*/


void SwFldFuncPage::UpdateSubType()
{
    sal_uInt16 nTypeId = (sal_uInt16)(sal_uLong)aTypeLB.GetEntryData(GetTypeSel());

    // Auswahl-Listbox fuellen
    aSelectionLB.SetUpdateMode(sal_False);
    aSelectionLB.Clear();

    SvStringsDtor aLst;
    GetFldMgr().GetSubTypes(nTypeId, aLst);
    sal_uInt16 nCount = aLst.Count();

    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        sal_uInt16 nPos = aSelectionLB.InsertEntry(*aLst[i]);
        aSelectionLB.SetEntryData(nPos, reinterpret_cast<void*>(i));
    }

    sal_Bool bEnable = nCount != 0;

    aSelectionLB.Enable( bEnable );
    aSelectionFT.Enable( bEnable );

    if (bEnable)
    {
            aSelectionLB.SelectEntryPos(0);
    }

    if (nTypeId == TYP_MACROFLD)
    {
        sal_Bool bHasMacro = GetFldMgr().GetMacroPath().Len() != 0;

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
    Beschreibung: MacroBrowser aufrufen, Listbox mit Macros fuellen
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

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

    ModifyHdl();    // Insert ggf enablen/disablen

    return sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

String SwFldFuncPage::TurnMacroString(const String &rMacro)
{
    if (rMacro.Len())
    {
        // Inhalt von aName umdrehen
        String sTmp, sBuf;
        sal_uInt16 nPos = 0;

        for (sal_uInt16 i = 0; i < 4 && nPos != STRING_NOTFOUND; i++)
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* SwFldFuncPage::Create(  Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldFuncPage( pParent, rAttrSet ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwFldFuncPage::GetGroup()
{
    return GRP_FKT;
}

/* -----------------12.01.99 10:09-------------------
 *
 * --------------------------------------------------*/
void    SwFldFuncPage::FillUserData()
{
    String sData( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( USER_DATA_VERSION )));
    sData += ';';
    sal_uInt16 nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = sal::static_int_cast< sal_uInt16 >(reinterpret_cast< sal_uIntPtr >(aTypeLB.GetEntryData( nTypeSel )));
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

IMPL_LINK( SwFldFuncPage, ModifyHdl, Edit *, EMPTYARG )
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




