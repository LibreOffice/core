/*************************************************************************
 *
 *  $RCSfile: fldfunc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _BASOBJ_HXX //autogen
#include <basctl/basobj.hxx>
#endif

#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif

#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif

#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FLDFUNC_HXX
#include <fldfunc.hxx>
#endif

#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1
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
    aMacroBT    (this, SW_RES(BT_FUNCMACRO))
{
    FreeResource();

    aNameED.SetPosPixel(Point(aNameED.GetPosPixel().X(), aFormatLB.GetPosPixel().Y()));

    sOldValueFT = aValueFT.GetText();
    sOldNameFT = aNameFT.GetText();

    aCond1ED.ShowBrackets(FALSE);
    aCond2ED.ShowBrackets(FALSE);

//  SwWrtShell* pSh = (SwWrtShell*)ViewShell::GetCurrShell();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

__EXPORT SwFldFuncPage::~SwFldFuncPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void __EXPORT SwFldFuncPage::Reset(const SfxItemSet& rSet)
{
    SavePos(&aTypeLB);
    Init(); // Allgemeine initialisierung

    aTypeLB.SetUpdateMode(FALSE);
    aTypeLB.Clear();

    USHORT nPos, nTypeId;
    BOOL bPage = FALSE;

    if (!IsFldEdit())
    {
        // TypeListBox initialisieren
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        // Typ-Listbox fuellen
        for(short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);
            nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
            aTypeLB.SetEntryData(nPos, (void*)nTypeId);
        }
    }
    else
    {
        nTypeId = GetCurField()->GetTypeId();
        nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        aTypeLB.SetEntryData(nPos, (void*)nTypeId);

        if (nTypeId == TYP_MACROFLD)
        {
            String sName(GetCurField()->GetPar1());
            BasicManager *pBasMgr = 0;
            GetFldMgr().SetMacroModule(0);

            if (sName.GetToken( 0, '.') == SFX_APP()->GetName())
                pBasMgr = SFX_APP()->GetBasicManager();
            else
            {
                SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();
                pBasMgr = pSh->GetView().GetDocShell()->GetBasicManager();
            }

            if (pBasMgr)
            {
                String sLibName(sName.GetToken(1, '.'));
                StarBASIC* pBasic = pBasMgr->GetLib(sLibName);

                if (pBasic)
                {
                    SbModule* pModule = BasicIDE::FindModule( pBasic, sName.GetToken( 2, '.') );
                    GetFldMgr().SetMacroModule(pModule);
                }
            }

            if (sName.Len())
            {
                // Inhalt von sName umdrehen
                String sBuf;
                String sTmp;
                USHORT nPos = 0;
                USHORT nCount = sName.GetTokenCount('.');

                for (int i = nCount - 1; i >= nCount - 4; i--)
                {
                    if (i == nCount - 4)
                    {
                        nPos = 0;
                        sName.GetToken(i, '.', nPos);
                        sTmp = sName.Copy(0, nPos - 1);
                    }
                    else
                    {
                        sTmp = sName.GetToken(i, '.');
                        sTmp += '.';
                    }

                    sBuf += sTmp;
                }

                sName = sBuf;
            }

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

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            USHORT nVal = sVal.ToInt32();
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldFuncPage, TypeHdl, ListBox *, pBox )
{
    // Alte ListBoxPos sichern
    const USHORT nOld = GetTypeSel();

    // Aktuelle ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        SetTypeSel(0);
        aTypeLB.SelectEntryPos(0);
    }

    if (nOld != GetTypeSel())
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        // Auswahl-Listbox fuellen
        UpdateSubType();

        // Format-Listbox fuellen
        aFormatLB.Clear();

        USHORT nSize = GetFldMgr().GetFormatCount(nTypeId, FALSE, IsFldDlgHtmlMode());

        for (USHORT i = 0; i < nSize; i++)
        {
            USHORT nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
            aFormatLB.SetEntryData( nPos, (void*)GetFldMgr().GetFormatId( nTypeId, i ) );
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

        // fuer Conditional Text zwei Controls
        if(nTypeId != TYP_CONDTXTFLD)
        {
            aCond1FT.Hide();
            aCond1ED.Hide();
            aCond2FT.Hide();
            aCond2ED.Hide();
            aValueFT.Show();
            aValueED.Show();
        }
        aNameED.EnableDrop(FALSE);

        if (IsFldEdit())
        {
            aNameED.SetText(GetCurField()->GetPar1());
            aValueED.SetText(GetCurField()->GetPar2());
        }
        else
        {
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
        }

        if (aNameFT.GetText() != sOldNameFT)
            aNameFT.SetText(sOldNameFT);
        if (aValueFT.GetText() != sOldValueFT)
            aValueFT.SetText(sOldValueFT);

        switch (nTypeId)
        {
            case TYP_MACROFLD:
                bMacro = TRUE;
                bShowSelection = TRUE;
                if (GetFldMgr().GetMacroPath().Len())
                    bValue = TRUE;
                else
                    bInsert = FALSE;

                aNameFT.SetText(SW_RESSTR(STR_MACNAME));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                aNameED.SetText(aSelectionLB.GetSelectEntry());
                break;

            case TYP_HIDDENPARAFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.EnableDrop(TRUE);
                bName = TRUE;
                break;

            case TYP_HIDDENTXTFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.EnableDrop(TRUE);
                aValueFT.SetText(SW_RESSTR(STR_INSTEXT));
                if (!IsFldEdit())
                    aValueED.SetText(::GetActiveView()->GetWrtShell().GetSelTxt());
                bName = bValue = TRUE;
                break;

            case TYP_CONDTXTFLD:
                aNameFT.SetText(SW_RESSTR(STR_COND));
                aNameED.EnableDrop(TRUE);
                if (IsFldEdit())
                {
                    aCond1ED.SetText(GetCurField()->GetPar2().GetToken(0, '|'));
                    aCond2ED.SetText(GetCurField()->GetPar2().GetToken(1, '|'));
                }

                bName = bValue = TRUE;

                aCond1FT.Show();
                aCond1ED.Show();
                aCond2FT.Show();
                aCond2ED.Show();
                aValueFT.Hide();
                aValueED.Hide();
                break;

            case TYP_JUMPEDITFLD:
                aNameFT.SetText(SW_RESSTR(STR_JUMPEDITFLD));
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bName = bValue = TRUE;
                break;

            case TYP_INPUTFLD:
                aValueFT.SetText(SW_RESSTR(STR_PROMPT));
                bValue = TRUE;
                // bShowSelection = TRUE;
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

        EnableInsert(bInsert);
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldFuncPage, SelectHdl, ListBox *, pBox )
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    if (nTypeId == TYP_MACROFLD)
        aNameED.SetText(aSelectionLB.GetSelectEntry());

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldFuncPage, InsertMacroHdl, ListBox *, pBox )
{
    SelectHdl();
    InsertHdl();

    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung: Typen in der SelectionBox erneuern
 --------------------------------------------------------------------*/


void SwFldFuncPage::UpdateSubType()
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    // Auswahl-Listbox fuellen
    aSelectionLB.SetUpdateMode(FALSE);
    aSelectionLB.Clear();

    SvStringsDtor& rLst = GetFldMgr().GetSubTypes(nTypeId);
    USHORT nCount = rLst.Count();

    for (USHORT i = 0; i < nCount; ++i)
    {
        USHORT nPos = aSelectionLB.InsertEntry(*rLst[i]);
        aSelectionLB.SetEntryData(nPos, (void*)i);
    }

    BOOL bEnable = nCount != 0;

    aSelectionLB.Enable( bEnable );
    aSelectionFT.Enable( bEnable );

    if (bEnable)
    {
        if (nTypeId == TYP_MACROFLD)
        {
            BOOL bHasMacro = GetFldMgr().GetMacroPath().Len() != 0;

            if (bHasMacro)
            {
                aSelectionLB.SelectEntry(GetFldMgr().GetMacroPath());
                aNameED.SetText(aSelectionLB.GetSelectEntry());
                aValueFT.Enable();
                aValueED.Enable();
            }
            EnableInsert(bHasMacro);
        }
        else
            aSelectionLB.SelectEntryPos(0);
    }
    aSelectionLB.SetUpdateMode(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung: MacroBrowser aufrufen, Listbox mit Macros fuellen
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldFuncPage, MacroHdl, Button *, pBtn )
{
    Window* pDefModalDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pBtn );

    String sMacro(TurnMacroString(aNameED.GetText()));
    while (sMacro.SearchAndReplace('.', ';') != STRING_NOTFOUND);

    if (GetFldMgr().ChooseMacro(sMacro))
        UpdateSubType();

    Application::SetDefDialogParent( pDefModalDlgParent );

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL __EXPORT SwFldFuncPage::FillItemSet(SfxItemSet& rSet)
{
    BOOL bPage = FALSE;
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
            break;

        case TYP_MACROFLD:
            aName = TurnMacroString(aName);
            break;

        case TYP_CONDTXTFLD:
            aVal = aCond1ED.GetText();
            aVal += '|';
            aVal += aCond2ED.GetText();
            break;

        default:
            break;
    }

    if (!IsFldEdit() ||
        aNameED.GetSavedValue() != aNameED.GetText() ||
        aValueED.GetSavedValue() != aValueED.GetText() ||
        aCond1ED.GetSavedValue() != aCond1ED.GetText() ||
        aCond2ED.GetSavedValue() != aCond2ED.GetText() ||
        nOldFormat != nFormat)
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat );
    }

    return FALSE;
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* __EXPORT SwFldFuncPage::Create(     Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldFuncPage( pParent, rAttrSet ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldFuncPage::GetGroup()
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
    USHORT nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (ULONG)aTypeLB.GetEntryData( nTypeSel );
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.28  2000/09/18 16:05:28  willem.vandorp
    OpenOffice header added.

    Revision 1.27  2000/09/04 11:43:02  tbe
    basicide, isetbrw, si, vcdlged moved from svx to basctl

    Revision 1.26  2000/06/30 08:52:52  os
    #76541# string assertions removed

    Revision 1.25  2000/05/23 18:36:44  jp
    Bugfixes for Unicode

    Revision 1.24  2000/04/18 15:17:31  os
    UNICODE

    Revision 1.23  2000/03/03 15:17:01  os
    StarView remainders removed

    Revision 1.22  1999/02/25 16:24:50  JP
    Bug #62438#: UserData nur auswerten, wenn kein Refresh ist


      Rev 1.21   25 Feb 1999 17:24:50   JP
   Bug #62438#: UserData nur auswerten, wenn kein Refresh ist

      Rev 1.20   21 Jan 1999 09:50:36   OS
   #59900# Fussnoten im Dialog korrekt sortieren; keine prot. Member

      Rev 1.19   12 Jan 1999 11:42:26   OS
   #60579# ausgewaehlten Typ in den UserData speichern

      Rev 1.18   10 Aug 1998 16:39:58   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.17   24 Jul 1998 13:57:08   OM
   #53246# Makros vorselektieren

      Rev 1.16   20 Jul 1998 11:31:00   OM
   #53244# Makros beim Bearbeiten nicht zersemmeln

      Rev 1.15   20 Jul 1998 09:38:52   OM
   #53278# Flackern vermeiden

      Rev 1.14   29 May 1998 16:41:18   OM
   #50667# Makrofelder bearbeiten

      Rev 1.13   18 May 1998 14:52:42   OM
   #50001 Keine eckigen Klammern in DB-Condition-Fields

      Rev 1.12   13 Mar 1998 16:30:42   OM
   #48197# Focus nach MacroDlg greppen

      Rev 1.11   07 Mar 1998 14:38:46   OM
   Feld nur bei Aenderung aktualisieren

      Rev 1.10   04 Mar 1998 08:37:50   MH
   chg: Syntax

      Rev 1.9   03 Feb 1998 09:24:58   OM
   Test

      Rev 1.8   02 Feb 1998 15:04:36   OM
   #46781# Macrofelder auch ausfuehren, wenn Dateiname mehr als 1 Punkt enthaelt

      Rev 1.7   09 Jan 1998 16:56:52   OM
   Bei Dok-Wechsel updaten

      Rev 1.6   08 Jan 1998 14:58:26   OM
   Traveling

      Rev 1.5   06 Jan 1998 18:12:54   OM
   Felbefehl-Dlg

      Rev 1.4   12 Dec 1997 16:10:06   OM
   AutoUpdate bei FocusWechsel u.a.

      Rev 1.3   11 Dec 1997 16:58:02   OM
   Feldumstellung

      Rev 1.2   25 Nov 1997 12:59:28   OM
   Funktionen-TP

      Rev 1.1   25 Nov 1997 12:38:42   OM
   Funktionen-TP

      Rev 1.0   04 Nov 1997 10:07:18   OM
   Initial revision.

------------------------------------------------------------------------*/


