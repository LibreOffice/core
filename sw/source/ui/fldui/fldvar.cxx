/*************************************************************************
 *
 *  $RCSfile: fldvar.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:59:11 $
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


#pragma hdrstop

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _LINKMGR_HXX //autogen
#include <so3/linkmgr.hxx>
#endif

#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FLDVAR_HXX
#include <fldvar.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif

#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
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

SwFldVarPage::SwFldVarPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage       ( pParent, SW_RES( TP_FLD_VAR ), rCoreSet ),
    aTypeFT         (this, SW_RES(FT_VARTYPE)),
    aTypeLB         (this, SW_RES(LB_VARTYPE)),
    aSelectionFT    (this, SW_RES(FT_VARSELECTION)),
    aSelectionLB    (this, SW_RES(LB_VARSELECTION)),
    aNameFT         (this, SW_RES(FT_VARNAME)),
    aNameED         (this, SW_RES(ED_VARNAME)),
    aValueFT        (this, SW_RES(FT_VARVALUE)),
    aValueED        (this, SW_RES(ED_VARVALUE)),
    aFormatFT       (this, SW_RES(FT_VARFORMAT)),
    aNumFormatLB    (this, SW_RES(LB_VARNUMFORMAT)),
    aFormatLB       (this, SW_RES(LB_VARFORMAT)),
    aChapterHeaderFT(this, SW_RES(FT_VARCHAPTERHEADER)),
    aChapterLevelFT (this, SW_RES(FT_VARCHAPTERLEVEL)),
    aChapterLevelLB (this, SW_RES(LB_VARCHAPTERLEVEL)),
    aInvisibleCB    (this, SW_RES(CB_VARINVISIBLE)),
    aSeparatorFT    (this, SW_RES(FT_VARSEPARATOR)),
    aSeparatorED    (this, SW_RES(ED_VARSEPARATOR)),
    aNewDelTBX      (this, SW_RES(TBX_VARNEWDEL)),
    bInit           (TRUE)
{
    FreeResource();

    aNewDelTBX.SetSizePixel( aNewDelTBX.CalcWindowSizePixel() );

    long nDelta = ( aValueED.GetSizePixel().Height() -
                    aNewDelTBX.GetSizePixel().Height() ) / 2;
    Point aNewPnt = aNewDelTBX.GetPosPixel();
    aNewPnt.Y() += nDelta;
    aNewDelTBX.SetPosPixel( aNewPnt );

    sOldValueFT = aValueFT.GetText();
    sOldNameFT = aNameFT.GetText();

    for (USHORT i = 1; i <= MAXLEVEL; i++)
        aChapterLevelLB.InsertEntry(String::CreateFromInt32(i));

    aChapterLevelLB.SelectEntryPos(0);
    //enable 'active' language selection
    aNumFormatLB.SetShowLanguageControl(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldVarPage::~SwFldVarPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldVarPage::Reset(const SfxItemSet& rSet)
{
    SavePos(&aTypeLB);

    Init(); // Allgemeine initialisierung

    aTypeLB.SetUpdateMode(FALSE);
    aTypeLB.Clear();

    USHORT nPos, nTypeId;

    if (!IsFldEdit())
    {
        // TypeListBox initialisieren
        const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

        for (short i = rRg.nStart; i < rRg.nEnd; ++i)
        {
            nTypeId = GetFldMgr().GetTypeId(i);

#ifndef DDE_AVAILABLE
            if (nTypeId != TYP_DDEFLD)
#endif
            {   // Nur unter WIN, WNT und OS/2 DDE-Fields
                nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i));
                aTypeLB.SetEntryData(nPos, (void*)nTypeId);
            }
        }
    }
    else
    {
        SwField* pCurField = GetCurField();
        nTypeId = pCurField->GetTypeId();
        if (nTypeId == TYP_SETINPFLD)
            nTypeId = TYP_INPUTFLD;
        nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(GetFldMgr().GetPos(nTypeId)));
        aTypeLB.SetEntryData(nPos, (void*)nTypeId);
        aNumFormatLB.SetAutomaticLanguage(pCurField->IsAutomaticLanguage());
        SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
        const SvNumberformat* pFormat = rSh.GetNumberFormatter()->GetEntry(pCurField->GetFormat());
        if(pFormat)
            aNumFormatLB.SetLanguage(pFormat->GetLanguage());
    }

    // alte Pos selektieren
    RestorePos(&aTypeLB);

    aTypeLB.SetDoubleClickHdl       (LINK(this, SwFldVarPage, InsertHdl));
    aTypeLB.SetSelectHdl            (LINK(this, SwFldVarPage, TypeHdl));
    aSelectionLB.SetSelectHdl       (LINK(this, SwFldVarPage, SubTypeHdl));
    aSelectionLB.SetDoubleClickHdl  (LINK(this, SwFldVarPage, InsertHdl));
    aFormatLB.SetDoubleClickHdl     (LINK(this, SwFldVarPage, InsertHdl));
    aNumFormatLB.SetDoubleClickHdl  (LINK(this, SwFldVarPage, InsertHdl));
    aNameED.SetModifyHdl            (LINK(this, SwFldVarPage, ModifyHdl));
    aValueED.SetModifyHdl           (LINK(this, SwFldVarPage, ModifyHdl));
    aNewDelTBX.SetClickHdl          (LINK(this, SwFldVarPage, TBClickHdl));
    aChapterLevelLB.SetSelectHdl    (LINK(this, SwFldVarPage, ChapterHdl));
    aSeparatorED.SetModifyHdl       (LINK(this, SwFldVarPage, SeparatorHdl));

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(!IsRefresh() && sUserData.GetToken(0, ';').EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
        {
            String sVal = sUserData.GetToken(1, ';');
            USHORT nVal = (USHORT)sVal.ToInt32();
            if( USHRT_MAX != nVal )
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
        aSelectionLB.SaveValue();
        aFormatLB.SaveValue();
        nOldFormat = aNumFormatLB.GetFormat();
        aNameED.SaveValue();
        aValueED.SaveValue();
        aInvisibleCB.SaveValue();
        aChapterLevelLB.SaveValue();
        aSeparatorED.SaveValue();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldVarPage, TypeHdl, ListBox *, pBox )
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

    if (nOld != GetTypeSel() || nOld == LISTBOX_ENTRY_NOTFOUND)
    {
        bInit = TRUE;
        if (nOld != LISTBOX_ENTRY_NOTFOUND)
        {
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
        }

        aValueED.SetDropEnable(FALSE);
        UpdateSubType();    // Auswahl-Listboxen initialisieren
    }

    bInit = FALSE;

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldVarPage, SubTypeHdl, ListBox *, pBox )
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());
    USHORT nSelPos = aSelectionLB.GetSelectEntryPos();

    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
        nSelPos = (USHORT)(ULONG)aSelectionLB.GetEntryData(nSelPos);

    if (IsFldEdit() && (!pBox || bInit))
    {
        if (nTypeId != TYP_FORMELFLD)
            aNameED.SetText(GetFldMgr().GetCurFldPar1());

        aValueED.SetText(GetFldMgr().GetCurFldPar2());
    }

    if (aNameFT.GetText() != sOldNameFT)
        aNameFT.SetText(sOldNameFT);
    if (aValueFT.GetText() != sOldValueFT)
        aValueFT.SetText(sOldValueFT);

    aNumFormatLB.SetUpdateMode(FALSE);
    aFormatLB.SetUpdateMode(FALSE);
    FillFormatLB(nTypeId);

    USHORT nSize = aFormatLB.GetEntryCount();

    BOOL bValue = FALSE, bName = FALSE, bNumFmt = FALSE,
            bInvisible = FALSE, bSeparator = FALSE, bChapterLevel = FALSE;
    BOOL bFormat = nSize != 0;

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            // Benutzertyp aendern oder anlegen
            SwUserFieldType* pType = (SwUserFieldType*)
                GetFldMgr().GetFldType(RES_USERFLD, nSelPos);

            if (pType)
            {
                if (!IsFldEdit())
                {
                    if (pBox || (bInit && !IsRefresh()))    // Nur bei Interaktion mit Maus
                    {
                        aNameED.SetText(pType->GetName());

                        if (pType->GetType() == UF_STRING)
                        {
                            aValueED.SetText(pType->GetContent());
                            aNumFormatLB.SelectEntryPos(0);
                        }
                        else
                            aValueED.SetText(pType->GetContent());
//                          aValueED.SetText(pType->GetContent(aNumFormatLB.GetFormat()));
                    }
                }
                else
                    aValueED.SetText(pType->GetContent());
            }
            else
            {
                if (pBox)   // Nur bei Interaktion mit Maus
                {
                    aNameED.SetText(aEmptyStr);
                    aValueED.SetText(aEmptyStr);
                }
            }
            bValue = bName = bNumFmt = bInvisible = TRUE;

            aValueED.SetDropEnable(TRUE);
            break;
        }

        case TYP_SETFLD:
            bValue = TRUE;

            bNumFmt = bInvisible = TRUE;

            if (!IsFldDlgHtmlMode())
                bName = TRUE;
            else
            {
                aNumFormatLB.Clear();
                USHORT nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
                aNumFormatLB.SelectEntryPos(0);
            }
            // gibt es ein entprechendes SetField
            if (IsFldEdit() || pBox)    // Nur bei Interaktion mit Maus
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    String sName(aSelectionLB.GetSelectEntry());
                    aNameED.SetText(sName);

                    if (!IsFldDlgHtmlMode())
                    {
                        SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                    ::GetActiveView()->GetWrtShell().
                                    GetFldType(RES_SETEXPFLD, sName);

                        if (pSetTyp && pSetTyp->GetType() == GSE_STRING)
                            aNumFormatLB.SelectEntryPos(0); // Textuell
                    }
                }
            }
            if (IsFldEdit())
            {
                // GetFormula fuehrt bei Datumsformaten zu Problemen,
                // da nur der numerische Wert ohne Formatierung returned wird.
                // Muss aber verwendet werden, da sonst bei GetPar2 nur der vom
                // Kalkulator errechnete Wert angezeigt werden wuerde
                // (statt test2 = test + 1)
                aValueED.SetText(((SwSetExpField*)GetCurField())->GetFormula());
            }
            aValueED.SetDropEnable(TRUE);
            break;

        case TYP_FORMELFLD:
            {
                bValue = TRUE;
                bNumFmt = TRUE;
                aValueFT.SetText(SW_RESSTR(STR_FORMULA));
                aValueED.SetDropEnable(TRUE);
            }
            break;

        case TYP_GETFLD:
            {
                if (!IsFldEdit())
                {
                    aNameED.SetText(aEmptyStr);
                    aValueED.SetText(aEmptyStr);
                }

                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    String sName(aSelectionLB.GetSelectEntry());
                    if (!IsFldEdit())
                        aNameED.SetText(sName);

                    // gibt es ein entprechendes SetField
                    SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                ::GetActiveView()->GetWrtShell().
                                GetFldType(RES_SETEXPFLD, sName);

                    if(pSetTyp)
                    {
                        if (pSetTyp->GetType() & GSE_STRING)    // Textuell?
                            bFormat = TRUE;
                        else                    // Numerisch
                            bNumFmt = TRUE;
                    }
                }
                else
                    bFormat = FALSE;

                EnableInsert(bFormat|bNumFmt);
            }
            break;

        case TYP_INPUTFLD:
            aValueFT.SetText(SW_RESSTR(STR_PROMPT));

            if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
            {
                bValue = bNumFmt = TRUE;

                String sName;

                sName = aSelectionLB.GetSelectEntry();
                aNameED.SetText( sName );

                // User- oder SetField ?
                USHORT nInpType = 0;
                nInpType = GetFldMgr().GetFldType(RES_USERFLD, sName) ? 0 : TYP_SETINPFLD;

                if (nInpType)   // SETEXPFLD
                {
                    // gibt es ein entprechendes SetField
                    SwSetExpFieldType* pSetTyp = (SwSetExpFieldType*)
                                GetFldMgr().GetFldType(RES_SETEXPFLD, sName);

                    if(pSetTyp)
                    {
                        if (pSetTyp->GetType() == GSE_STRING)   // Textuell?
                        {
                            aNumFormatLB.Clear();

                            USHORT nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_USERVAR_TEXT), 0);
                            aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
                            aNumFormatLB.SelectEntryPos(0);
                        }
                    }
                    if (IsFldEdit() && (!pBox || bInit) )
                        aValueED.SetText(((SwSetExpField*)GetCurField())->GetPromptText());
                }
                else    // USERFLD
                    bFormat = bNumFmt = FALSE;
            }
            break;

        case TYP_DDEFLD:
            aValueFT.SetText(SW_RESSTR(STR_DDE_CMD));

            if (IsFldEdit() || pBox)    // Nur bei Interaktion mit Maus
            {
                if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    SwDDEFieldType* pType =
                        (SwDDEFieldType*) GetFldMgr().GetFldType(RES_DDEFLD, nSelPos);

                    if(pType)
                    {
                        aNameED.SetText(pType->GetName());

                        //JP 28.08.95: DDE-Topics/-Items koennen Blanks in ihren
                        //              Namen haben! Wird hier noch nicht beachtet
                        String sCmd( pType->GetCmd() );
                        USHORT nTmpPos = sCmd.SearchAndReplace( so3::cTokenSeperator, ' ' );
                        sCmd.SearchAndReplace( so3::cTokenSeperator, ' ', nTmpPos );

                        aValueED.SetText( sCmd );
                        aFormatLB.SelectEntryPos(pType->GetType());
                    }
                }
            }
            bName = bValue = TRUE;
            break;

        case TYP_SEQFLD:
            {
                // aNumRB.Check(TRUE);
                bName = bValue = bSeparator = bChapterLevel = TRUE;

                SwFieldType* pFldTyp;
                if( IsFldEdit() )
                    pFldTyp = GetCurField()->GetTyp();
                else
                {
                    String sFldTypeName( aSelectionLB.GetEntry( nSelPos ));
                    if( sFldTypeName.Len() )
                        pFldTyp = GetFldMgr().GetFldType( RES_SETEXPFLD,
                                                          sFldTypeName );
                    else
                        pFldTyp = 0;
                }

                if( IsFldEdit() )
                    aValueED.SetText( ((SwSetExpField*)GetCurField())->
                                        GetFormula() );

                if( IsFldEdit() || pBox )   // Nur bei Interaktion mit Maus
                    aNameED.SetText( aSelectionLB.GetSelectEntry() );

                if( pFldTyp )
                {
                    BYTE nLevel = ((SwSetExpFieldType*)pFldTyp)->GetOutlineLvl();
                    if( 0x7f == nLevel )
                        aChapterLevelLB.SelectEntryPos( 0 );
                    else
                        aChapterLevelLB.SelectEntryPos( nLevel + 1 );
                    sal_Unicode cDelim = ((SwSetExpFieldType*)pFldTyp)->GetDelimiter();
                    if(cDelim)
                        aSeparatorED.SetText( cDelim );
                    else
                        aSeparatorED.SetText( aEmptyStr );
                    ChapterHdl();
                }
            }
            break;

        case TYP_SETREFPAGEFLD:
            {
                bValue = FALSE;
                aValueFT.SetText( SW_RESSTR( STR_OFFSET ));

                if (IsFldEdit() || pBox)    // Nur bei Interaktion mit Maus
                    aNameED.SetText(aEmptyStr);

                if (nSelPos != 0 && nSelPos != LISTBOX_ENTRY_NOTFOUND)
                {
                    bValue = TRUE;      // SubType OFF - kennt keinen Offset
                    if (IsFldEdit())
                        aValueED.SetText(String::CreateFromInt32(((SwRefPageSetField*)GetCurField())->GetOffset()));
                }
            }
            break;

        case TYP_GETREFPAGEFLD:
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
            break;
    }

    aNumFormatLB.Show(bNumFmt);
    aFormatLB.Show(!bNumFmt);

    if (IsFldEdit())
        bName = FALSE;

    aFormatLB.Enable(bFormat);
    aFormatFT.Enable(bFormat|bNumFmt);
    aNameFT.Enable(bName);
    aNameED.Enable(bName);
    aValueFT.Enable(bValue);
    aValueED.Enable(bValue);

    Size aSz(aFormatLB.GetSizePixel());

    if (bChapterLevel)
        aSz.Height() = aFormatLB.LogicToPixel(Size(1, 94), MAP_APPFONT).Height();
    else
        aSz.Height() = aFormatLB.LogicToPixel(Size(1, 123), MAP_APPFONT).Height();

    aFormatLB.SetSizePixel(aSz);

    aInvisibleCB.Show(!bSeparator);
    aSeparatorFT.Show(bSeparator);
    aSeparatorED.Show(bSeparator);
    aChapterHeaderFT.Show(bChapterLevel);
    aChapterLevelFT.Show(bChapterLevel);
    aChapterLevelLB.Show(bChapterLevel);
    aInvisibleCB.Enable(bInvisible);

    ModifyHdl();    // Anwenden/Einfuegen/Loeschen Status update

    aNumFormatLB.SetUpdateMode(TRUE);
    aFormatLB.SetUpdateMode(TRUE);

    if(aSelectionLB.IsCallAddSelection())
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        switch (nTypeId)
        {
            case TYP_FORMELFLD:
                {
                    USHORT nSelPos = aSelectionLB.GetSelectEntryPos();

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
                        nSelPos = (USHORT)(ULONG)aSelectionLB.GetEntryData(nSelPos);

                    if (nSelPos != LISTBOX_ENTRY_NOTFOUND && pBox && !bInit)
                    {
                        aValueED.ReplaceSelected(aSelectionLB.GetSelectEntry());
                        ModifyHdl();
                    }
                }
                break;
        }
        aSelectionLB.ResetCallAddSelection();
    }
    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung: Typen in der SelectionBox erneuern
 --------------------------------------------------------------------*/

void SwFldVarPage::UpdateSubType()
{
    String sOldSel;
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    SetSelectionSel(aSelectionLB.GetSelectEntryPos());
    if(GetSelectionSel() != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = aSelectionLB.GetEntry(GetSelectionSel());

    // Auswahl-Listbox fuellen
    aSelectionLB.SetUpdateMode(FALSE);
    aSelectionLB.Clear();

    SvStringsDtor aList;
    GetFldMgr().GetSubTypes(nTypeId, aList);
    USHORT nCount = aList.Count();
    USHORT nPos;

    for (USHORT i = 0; i < nCount; ++i)
    {
        if (nTypeId != TYP_INPUTFLD || i)
        {
            if (!IsFldEdit())
            {
                nPos = aSelectionLB.InsertEntry(*aList[i]);
                aSelectionLB.SetEntryData(nPos, (void*)i);
            }
            else
            {
                BOOL bInsert = FALSE;

                switch (nTypeId)
                {
                    case TYP_INPUTFLD:
                        if (*aList[i] == GetCurField()->GetPar1())
                            bInsert = TRUE;
                        break;

                    case TYP_FORMELFLD:
                        bInsert = TRUE;
                        break;

                    case TYP_GETFLD:
                        if (*aList[i] == ((SwFormulaField*)GetCurField())->GetFormula())
                            bInsert = TRUE;
                        break;

                    case TYP_SETFLD:
                    case TYP_USERFLD:
                        if (*aList[i] == GetCurField()->GetTyp()->GetName())
                        {
                            bInsert = TRUE;
                            if (GetCurField()->GetSubType() & SUB_INVISIBLE)
                                aInvisibleCB.Check();
                        }
                        break;

                    case TYP_SETREFPAGEFLD:
                        if ((((SwRefPageSetField*)GetCurField())->IsOn() && i) ||
                            (!((SwRefPageSetField*)GetCurField())->IsOn() && !i))
                            sOldSel = *aList[i];

                        // Alle Eintr„ge zur Auswahl zulassen:
                        nPos = aSelectionLB.InsertEntry(*aList[i]);
                        aSelectionLB.SetEntryData(nPos, (void*)i);
                        break;

                    default:
                        if (*aList[i] == GetCurField()->GetPar1())
                            bInsert = TRUE;
                        break;
                }
                if (bInsert)
                {
                    nPos = aSelectionLB.InsertEntry(*aList[i]);
                    aSelectionLB.SetEntryData(nPos, (void*)i);
                    if (nTypeId != TYP_FORMELFLD)
                        break;
                }
            }
        }
    }

    BOOL bEnable = aSelectionLB.GetEntryCount() != 0;
    ListBox *pLB = 0;

    if (bEnable)
    {
        aSelectionLB.SelectEntry(sOldSel);
        if (!aSelectionLB.GetSelectEntryCount())
        {
            aSelectionLB.SelectEntryPos(0);
            pLB = &aSelectionLB;    // Alle Controls neu initialisieren
        }
    }

    aSelectionLB.Enable( bEnable );
    aSelectionFT.Enable( bEnable );

    SubTypeHdl(pLB);
    aSelectionLB.SetUpdateMode(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldVarPage::FillFormatLB(USHORT nTypeId)
{
    String sOldSel, sOldNumSel;
    ULONG nOldNumFormat = 0;

    USHORT nFormatSel = aFormatLB.GetSelectEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = aFormatLB.GetEntry(nFormatSel);

    USHORT nNumFormatSel = aNumFormatLB.GetSelectEntryPos();
    if (nNumFormatSel != LISTBOX_ENTRY_NOTFOUND)
    {
        sOldNumSel = aNumFormatLB.GetEntry(nNumFormatSel);
        nOldNumFormat = aNumFormatLB.GetFormat();
    }

    // Format-Listbox fuellen
    aFormatLB.Clear();
    aNumFormatLB.Clear();
    BOOL bSpecialFmt = FALSE;

    if( TYP_GETREFPAGEFLD != nTypeId )
    {
        if (IsFldEdit())
        {
            bSpecialFmt = GetCurField()->GetFormat() == ULONG_MAX;

            if (!bSpecialFmt)
            {
                aNumFormatLB.SetDefFormat(GetCurField()->GetFormat());
                sOldNumSel = aEmptyStr;
            }
            else
                if (nTypeId == TYP_GETFLD || nTypeId == TYP_FORMELFLD)
                    aNumFormatLB.SetFormatType(NUMBERFORMAT_NUMBER);
        }
        else
        {
            if (nOldNumFormat && nOldNumFormat != ULONG_MAX)
                aNumFormatLB.SetDefFormat(nOldNumFormat);
            else
                aNumFormatLB.SetFormatType(NUMBERFORMAT_NUMBER);
        }
    }

    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            if (!IsFldEdit() || bSpecialFmt)
            {
                USHORT nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_MARK_TEXT), 0);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
                nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_USERVAR_CMD), 1);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
            }
        }
        break;

        case TYP_SETFLD:
        {
            if (!IsFldEdit() || bSpecialFmt)
            {
                USHORT nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_SETVAR_TEXT), 0);
                aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
            }
        }
        break;

        case TYP_FORMELFLD:
        {
            USHORT nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
            aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
        }
        break;

        case TYP_GETFLD:
        {
            USHORT nPos = aNumFormatLB.InsertEntry(SW_RESSTR(FMT_GETVAR_NAME), 0);
            aNumFormatLB.SetEntryData(nPos, (void *)ULONG_MAX);
        }
        break;
    }

    if (IsFldEdit() && bSpecialFmt)
    {
        if (nTypeId == TYP_USERFLD && (GetCurField()->GetSubType() & SUB_CMD))
            aNumFormatLB.SelectEntryPos(1);
        else
            aNumFormatLB.SelectEntryPos(0);
    }
    else
    {
        if (!nOldNumFormat && (nNumFormatSel = aNumFormatLB.GetEntryPos(sOldNumSel)) != LISTBOX_ENTRY_NOTFOUND)
            aNumFormatLB.SelectEntryPos(nNumFormatSel);
        else if (nOldNumFormat && nOldNumFormat == ULONG_MAX)
            aNumFormatLB.SelectEntry(sOldSel);
    }

    USHORT nSize = GetFldMgr().GetFormatCount(nTypeId, FALSE, IsFldDlgHtmlMode());

    for (USHORT i = 0; i < nSize; i++)
    {
        USHORT nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr(nTypeId, i));
        USHORT nFldId = GetFldMgr().GetFormatId( nTypeId, i );
        aFormatLB.SetEntryData( nPos, (void*)nFldId );
        if (IsFldEdit() && nFldId == GetCurField()->GetFormat())
            aFormatLB.SelectEntryPos( nPos );
    }

    if (nSize && (!IsFldEdit() || !aFormatLB.GetSelectEntryCount()))
    {
        aFormatLB.SelectEntry(sOldSel);

        if (!aFormatLB.GetSelectEntryCount())
        {
            aFormatLB.SelectEntry(SW_RESSTR(FMT_NUM_PAGEDESC));
            if (!aFormatLB.GetSelectEntryCount())
            {
                aFormatLB.SelectEntry(SW_RESSTR(FMT_NUM_ARABIC));
                if (!aFormatLB.GetSelectEntryCount())
                    aFormatLB.SelectEntryPos(0);
            }
        }
    }

    return nSize;
}

/*--------------------------------------------------------------------
    Beschreibung: Modify
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldVarPage, ModifyHdl, Edit *, EMPTYARG )
{
    String sValue(aValueED.GetText());
    BOOL bHasValue = sValue.Len() != 0;
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());
    BOOL bInsert = FALSE, bApply = FALSE, bDelete = FALSE;

    String sName( aNameED.GetText() );
    xub_StrLen nLen = sName.Len();

    switch( nTypeId )
    {
    case TYP_DDEFLD:
    case TYP_USERFLD:
    case TYP_SETFLD:
    case TYP_SEQFLD:
        SwCalc::IsValidVarName( sName, &sName );
        if( sName.Len() != nLen )
        {
            nLen = sName.Len();
            Selection aSel(aNameED.GetSelection());
            aNameED.SetText( sName );
            aNameED.SetSelection( aSel );   // Cursorpos restaurieren
        }
        break;
    }


    // Buttons ueberpruefen
    switch (nTypeId)
    {
    case TYP_DDEFLD:
        if( nLen )
        {
            // Gibts schon einen entsprechenden Type
            bInsert = bApply = TRUE;

            SwFieldType* pType = GetFldMgr().GetFldType(RES_DDEFLD, sName);

            if (pType)
                bDelete = !::GetActiveView()->GetWrtShell().IsUsed( *pType );
        }
        break;

    case TYP_USERFLD:
        if( nLen )
        {
            // Gibts schon einen entsprechenden Type
            SwFieldType* pType = GetFldMgr().GetFldType(RES_USERFLD, sName);

            if (pType)
                bDelete = !::GetActiveView()->GetWrtShell().IsUsed( *pType );

            pType = GetFldMgr().GetFldType(RES_SETEXPFLD, sName);
            if (!pType) // Kein Namenskonflikt mit Variablen
            {
                // Benutzerfelder duerfen auch ohne Inhalt eingefuegt werden!
                // Bug #56845
                bInsert = bApply = TRUE;
            }
        }
        break;

    default:
        bInsert = TRUE;

        if (nTypeId == TYP_SETFLD || nTypeId == TYP_SEQFLD)
        {
            SwSetExpFieldType* pFldType = (SwSetExpFieldType*)
                GetFldMgr().GetFldType(RES_SETEXPFLD, sName);

            if (pFldType)
            {

                SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
                const SwFldTypes* p = rSh.GetDoc()->GetFldTypes();
                USHORT i;

                for (i = 0; i < INIT_FLDTYPES; i++)
                {
                    SwFieldType* pType = (*p)[ i ];
                    if (pType == pFldType)
                        break;
                }

                if (i >= INIT_FLDTYPES && !rSh.IsUsed(*pFldType))
                    bDelete = TRUE;

                if (nTypeId == TYP_SEQFLD && !(pFldType->GetType() & GSE_SEQ))
                    bInsert = FALSE;

                if (nTypeId == TYP_SETFLD && (pFldType->GetType() & GSE_SEQ))
                    bInsert = FALSE;
            }
            if (GetFldMgr().GetFldType(RES_USERFLD, sName))
                bInsert = FALSE;
        }

        if( !nLen && ( nTypeId == TYP_SETFLD ||
                        (!IsFldEdit() && nTypeId == TYP_GETFLD ) ) )
            bInsert = FALSE;

        if( (nTypeId == TYP_SETFLD || nTypeId == TYP_FORMELFLD) &&
            !bHasValue )
            bInsert = FALSE;
        break;
    }

    aNewDelTBX.EnableItem(BT_VARAPPLY, bApply);
    aNewDelTBX.EnableItem(BT_VARDELETE, bDelete);
    EnableInsert(bInsert);

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldVarPage, TBClickHdl, ToolBox *, pBox )
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    switch (pBox->GetCurItemId())
    {
        case BT_VARDELETE:
        {
            if( nTypeId == TYP_USERFLD )
                GetFldMgr().RemoveFldType(RES_USERFLD, aSelectionLB.GetSelectEntry());
            else
            {
                USHORT nWhich;

                switch(nTypeId)
                {
                    case TYP_SETFLD:
                    case TYP_SEQFLD:
                        nWhich = RES_SETEXPFLD;
                        break;
                    default:
                        nWhich = RES_DDEFLD;
                        break;
                }

                GetFldMgr().RemoveFldType(nWhich, aSelectionLB.GetSelectEntry());
            }

            UpdateSubType();
            ::GetActiveView()->GetWrtShell().SetModified();
        }
        break;

        case BT_VARAPPLY:
        {
            String sName(aNameED.GetText()), sValue(aValueED.GetText());
            SwFieldType* pType = 0;
            USHORT nId;
            USHORT nNumFormatPos = aNumFormatLB.GetSelectEntryPos();

            switch (nTypeId)
            {
                case TYP_USERFLD:   nId = RES_USERFLD;  break;
                case TYP_DDEFLD:    nId = RES_DDEFLD;   break;
                case TYP_SETFLD:    nId = RES_SETEXPFLD;break;
            }
            pType = GetFldMgr().GetFldType(nId, sName);

            ULONG nFormat = aFormatLB.GetSelectEntryPos();
            if (nFormat != LISTBOX_ENTRY_NOTFOUND)
                nFormat = (ULONG)aFormatLB.GetEntryData((USHORT)nFormat);

            if (pType)  // Aendern
            {
                SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
                rSh.StartAllAction();

                if (nTypeId == TYP_USERFLD)
                {
                    if (nNumFormatPos != LISTBOX_ENTRY_NOTFOUND)
                    {
                        ULONG nFmt = nNumFormatPos == 0 ? 0 : aNumFormatLB.GetFormat();
                        if (nFmt)
                        {   // Sprache auf Office-Sprache umstellen, da String im Office
                            // Format vom Kalkulator erwartet wird und so in den Dlg
                            // eingegeben werden sollte
                            SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();
                            nFmt = SwValueField::GetSystemFormat(rSh.GetNumberFormatter(), nFmt);
                        }
                        ((SwUserFieldType*)pType)->SetContent(aValueED.GetText(), nFmt);
                        ((SwUserFieldType*)pType)->SetType(
                            nNumFormatPos == 0 ? GSE_STRING : GSE_EXPR );
                    }
                }
                else
                {
                    if (nFormat != LISTBOX_ENTRY_NOTFOUND)
                    {
                        //JP 28.08.95: DDE-Topics/-Items koennen Blanks in ihren
                        //              Namen haben! Wird hier noch nicht beachtet.
                        USHORT nTmpPos = sValue.SearchAndReplace( ' ', so3::cTokenSeperator );
                        sValue.SearchAndReplace( ' ', so3::cTokenSeperator, nTmpPos );
                        ((SwDDEFieldType*)pType)->SetCmd(sValue);
                        ((SwDDEFieldType*)pType)->SetType((USHORT)nFormat);
                    }
                }
                pType->UpdateFlds();

                rSh.EndAllAction();
            }
            else        // Neu
            {
                if(nTypeId == TYP_USERFLD)
                {
                    SwUserFieldType aType( ::GetActiveView()->GetWrtShellPtr()->GetDoc(), sName );

                    if (nNumFormatPos != LISTBOX_ENTRY_NOTFOUND)
                    {
                        aType.SetType(nNumFormatPos == 0 ? GSE_STRING : GSE_EXPR);
                        aType.SetContent( sValue, nNumFormatPos == 0 ? 0 : aNumFormatLB.GetFormat() );
                        aSelectionLB.InsertEntry(sName);
                        aSelectionLB.SelectEntry(sName);
                        GetFldMgr().InsertFldType( aType ); // Userfld Neu
                    }
                }
                else
                {
                    if (nFormat != LISTBOX_ENTRY_NOTFOUND)
                    {
                        //JP 28.08.95: DDE-Topics/-Items koennen Blanks in ihren
                        //              Namen haben! Wird hier noch nicht beachtet.
                        USHORT nTmpPos = sValue.SearchAndReplace( ' ', so3::cTokenSeperator );
                        sValue.SearchAndReplace( ' ', so3::cTokenSeperator, nTmpPos );

                        SwDDEFieldType aType(sName, sValue, (USHORT)nFormat);
                        aSelectionLB.InsertEntry(sName);
                        aSelectionLB.SelectEntry(sName);
                        GetFldMgr().InsertFldType(aType);   // DDE-Feld Neu
                    }
                }
            }
            if (IsFldEdit())
                GetFldMgr().GetCurFld();    // FieldManager Updaten

            UpdateSubType();
        }
        break;
    }

    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldVarPage, ChapterHdl, ListBox *, pLB )
{
    BOOL bEnable = aChapterLevelLB.GetSelectEntryPos() != 0;

    aSeparatorED.Enable(bEnable);
    aSeparatorFT.Enable(bEnable);
    SeparatorHdl();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldVarPage, SeparatorHdl, Edit *, pED )
{
    BOOL bEnable = aSeparatorED.GetText().Len() != 0 ||
                    aChapterLevelLB.GetSelectEntryPos() == 0;
    EnableInsert(bEnable);

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwFldVarPage::FillItemSet(SfxItemSet& rSet)
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    String aVal(aValueED.GetText());
    String aName(aNameED.GetText());

    USHORT nSubType = aSelectionLB.GetSelectEntryPos();
    if(nSubType == LISTBOX_ENTRY_NOTFOUND)
        nSubType = 0;
    else
        nSubType = (USHORT)(ULONG)aSelectionLB.GetEntryData(nSubType);

    ULONG nFormat;

    if (!aNumFormatLB.IsVisible())
    {
        nFormat = aFormatLB.GetSelectEntryPos();

        if(nFormat == LISTBOX_ENTRY_NOTFOUND)
            nFormat = 0;
        else
            nFormat = (ULONG)aFormatLB.GetEntryData((USHORT)nFormat);
    }
    else
    {
        nFormat = aNumFormatLB.GetFormat();

        if (nFormat && nFormat != ULONG_MAX && aNumFormatLB.IsAutomaticLanguage())
        {
            // Sprache auf Office-Sprache umstellen, da String im Office-
            // Format vom Kalkulator erwartet wird und so in den Dlg
            // eingegeben werden sollte
            SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
            SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();

            nFormat = SwValueField::GetSystemFormat(rSh.GetNumberFormatter(), nFormat);
        }
    }
    sal_Unicode cSeparator = ' ';
    switch (nTypeId)
    {
        case TYP_USERFLD:
        {
            nSubType = (nFormat == ULONG_MAX) ? GSE_STRING : GSE_EXPR;

            if (nFormat == ULONG_MAX && aNumFormatLB.GetSelectEntry() == SW_RESSTR(FMT_USERVAR_CMD))
                nSubType |= SUB_CMD;

            if (aInvisibleCB.IsChecked())
                nSubType |= SUB_INVISIBLE;
            break;
        }
        case TYP_FORMELFLD:
        {
            nSubType = GSE_FORMULA;
            if (aNumFormatLB.IsVisible() && nFormat == ULONG_MAX)
                nSubType |= SUB_CMD;
            break;
        }
        case TYP_GETFLD:
        {
            nSubType &= 0xff00;
            if (aNumFormatLB.IsVisible() && nFormat == ULONG_MAX)
                nSubType |= SUB_CMD;
            break;
        }
        case TYP_INPUTFLD:
        {
            SwFieldType* pType = GetFldMgr().GetFldType(RES_USERFLD, aName);
            nSubType = (nSubType & 0xff00) | ((pType) ? INP_USR : INP_VAR);
            break;
        }

        case TYP_SETFLD:
        {
            if (IsFldDlgHtmlMode())
            {
                nSubType = 0x0100;
                nSubType = (nSubType & 0xff00) | GSE_STRING;
            }
            else
                nSubType = (nSubType & 0xff00) | ((nFormat == ULONG_MAX) ? GSE_STRING : GSE_EXPR);

            if (aInvisibleCB.IsChecked())
                nSubType |= SUB_INVISIBLE;
            break;
        }
        case TYP_SEQFLD:
        {
            // nSubType = GSE_SEQ;  // GSE_SEQ wird im Fldmgr fest gesetzt, kann also entfallen
            nSubType = aChapterLevelLB.GetSelectEntryPos();
            if (nSubType == 0)
                nSubType = 0x7f;
            else
            {
                nSubType--;
                String sSeparator(aSeparatorED.GetText().GetChar(0));
                cSeparator = sSeparator.Len() ? sSeparator.GetChar(0) : ' ';
                //nSubType |= (USHORT)(((BYTE)) << 8);
            }
            break;
        }
        case TYP_GETREFPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
                aVal = aValueED.GetText();
            break;
    }

    if (!IsFldEdit() ||
        aNameED.GetSavedValue() != aNameED.GetText() ||
        aValueED.GetSavedValue() != aValueED.GetText() ||
        aSelectionLB.GetSavedValue() != aSelectionLB.GetSelectEntryPos() ||
        aFormatLB.GetSavedValue() != aFormatLB.GetSelectEntryPos() ||
        nOldFormat != aNumFormatLB.GetFormat() ||
        aInvisibleCB.GetState() != aInvisibleCB.GetSavedValue() ||
        aChapterLevelLB.GetSavedValue() != aChapterLevelLB.GetSelectEntryPos() ||
        aSeparatorED.GetSavedValue() != aSeparatorED.GetText())
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat,
                    cSeparator, aNumFormatLB.IsAutomaticLanguage() );
    }

    UpdateSubType();

    return FALSE;
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* SwFldVarPage::Create(   Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldVarPage( pParent, rAttrSet ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldVarPage::GetGroup()
{
    return GRP_VAR;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SelectionListBox::SelectionListBox( SwFldVarPage* pDialog, const ResId& rResId ) :
    ListBox (pDialog, rResId),
    pDlg    (pDialog),
    bCallAddSelection(FALSE)
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long SelectionListBox::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = ListBox::PreNotify( rNEvt );
    //BOOL bAddSel = FALSE;
    if ( rNEvt.GetType() == EVENT_KEYUP )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();
        const USHORT nModifier = aKeyCode.GetModifier();
        if( aKeyCode.GetCode() == KEY_SPACE && !nModifier)
//          bAddSel = TRUE;
            bCallAddSelection = TRUE;
    }
    if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        const MouseEvent* pMEvt = rNEvt.GetMouseEvent();

        if (pMEvt && (pMEvt->IsMod1() || pMEvt->IsMod2()))  // Alt oder Ctrl
            //bAddSel = TRUE;
            bCallAddSelection = TRUE;
    }

//  if (bAddSel)
//      pDlg->AddSelection(this);

    return nHandled;
}
/* -----------------12.01.99 11:14-------------------
 *
 * --------------------------------------------------*/
void SwFldVarPage::FillUserData()
{
    String sData(String::CreateFromAscii(USER_DATA_VERSION));
    sData += ';';
    USHORT nTypeSel = aTypeLB.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND == nTypeSel )
        nTypeSel = USHRT_MAX;
    else
        nTypeSel = (USHORT)(ULONG)aTypeLB.GetEntryData( nTypeSel );
    sData += String::CreateFromInt32( nTypeSel );
    SetUserData(sData);
}

