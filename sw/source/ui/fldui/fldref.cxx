/*************************************************************************
 *
 *  $RCSfile: fldref.cxx,v $
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

#include "swtypes.hxx"

#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
#ifndef _BOOKMRK_HXX //autogen
#include <bookmrk.hxx>
#endif
#ifndef _EXPFLD_HXX //autogen
#include <expfld.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FLDREF_HXX
#include <fldref.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif

#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

#define REFFLDFLAG          0x4000
#define REFFLDFLAG_BOOKMARK 0x4800
#define REFFLDFLAG_FOOTNOTE 0x5000
#define REFFLDFLAG_ENDNOTE  0x6000

USHORT  nFldDlgFmtSel       = 0;

#define USER_DATA_VERSION_1 "1"
#define USER_DATA_VERSION USER_DATA_VERSION_1
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldRefPage::SwFldRefPage(Window* pParent, const SfxItemSet& rCoreSet ) :
    SwFldPage( pParent, SW_RES( TP_FLD_REF ), rCoreSet ),

    aTypeFT         (this, SW_RES(FT_REFTYPE)),
    aTypeLB         (this, SW_RES(LB_REFTYPE)),
    aSelectionFT    (this, SW_RES(FT_REFSELECTION)),
    aSelectionLB    (this, SW_RES(LB_REFSELECTION)),
    aFormatFT       (this, SW_RES(FT_REFFORMAT)),
    aFormatLB       (this, SW_RES(LB_REFFORMAT)),
    aNameFT         (this, SW_RES(FT_REFNAME)),
    aNameED         (this, SW_RES(ED_REFNAME)),
    aValueFT        (this, SW_RES(FT_REFVALUE)),
    aValueED        (this, SW_RES(ED_REFVALUE)),

    sBookmarkTxt    (SW_RES(STR_REFBOOKMARK)),
    sFootnoteTxt    (SW_RES(STR_REFFOOTNOTE)),
    sEndnoteTxt     (SW_RES(STR_REFENDNOTE))
{
    FreeResource();

    aNameED.SetModifyHdl(LINK(this, SwFldRefPage, ModifyHdl));

//  SwWrtShell* pSh = (SwWrtShell*)ViewShell::GetCurrShell();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldRefPage::~SwFldRefPage()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldRefPage::Reset(const SfxItemSet& )
{
    if (!IsFldEdit())
        SavePos(&aTypeLB);
    SetSelectionSel(LISTBOX_ENTRY_NOTFOUND);
    SetTypeSel(LISTBOX_ENTRY_NOTFOUND);
    Init(); // Allgemeine initialisierung

    // TypeListBox initialisieren
    aTypeLB.SetUpdateMode(FALSE);
    aTypeLB.Clear();

    // Typ-Listbox fuellen

    // mit den Sequence-Typen auffuellen
    SwView *pView = ::GetActiveView();
    ASSERT(pView, View fehlt);
    SwWrtShell &rSh = pView->GetWrtShell();
    USHORT nPos;

    USHORT nFldTypeCnt = rSh.GetFldTypeCount(RES_SETEXPFLD);

    for (USHORT n = 0; n < nFldTypeCnt; ++n)
    {
        SwSetExpFieldType* pType = (SwSetExpFieldType*)rSh.GetFldType(n, RES_SETEXPFLD);

        if ((GSE_SEQ & pType->GetType()) && pType->GetDepends() && rSh.IsUsed(*pType))
        {
            nPos = aTypeLB.InsertEntry(pType->GetName());
            aTypeLB.SetEntryData(nPos, (void*)(REFFLDFLAG | n));
        }
    }

    // Textmarken - jetzt immer (wegen Globaldokumenten)
    nFldTypeCnt = rSh.GetBookmarkCnt(TRUE);
    nPos = aTypeLB.InsertEntry(sBookmarkTxt);
    aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_BOOKMARK);

    // Fussnoten:
    if( rSh.HasFtns() )
    {
        nPos = aTypeLB.InsertEntry(sFootnoteTxt);
        aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_FOOTNOTE);
    }

    // Endnoten:
    if( rSh.HasFtns(TRUE) )
    {
        nPos = aTypeLB.InsertEntry(sEndnoteTxt);
        aTypeLB.SetEntryData(nPos, (void*)REFFLDFLAG_ENDNOTE);
    }

    // Referenz setzen / einfuegen
    const SwFldGroupRgn& rRg = GetFldMgr().GetGroupRange(IsFldDlgHtmlMode(), GetGroup());

    for (short i = rRg.nStart; i < rRg.nEnd; ++i)
    {
        const USHORT nTypeId = GetFldMgr().GetTypeId(i);

        if (!IsFldEdit() || nTypeId != TYP_SETREFFLD)
        {
            nPos = aTypeLB.InsertEntry(GetFldMgr().GetTypeStr(i), i - rRg.nStart);
            aTypeLB.SetEntryData(nPos, (void*)nTypeId);
        }
    }

    // alte Pos selektieren
    if (!IsFldEdit())
        RestorePos(&aTypeLB);

    aTypeLB.SetUpdateMode(TRUE);
    aTypeLB.SetDoubleClickHdl       (LINK(this, SwFldRefPage, InsertHdl));
    aTypeLB.SetSelectHdl            (LINK(this, SwFldRefPage, TypeHdl));
    aSelectionLB.SetSelectHdl       (LINK(this, SwFldRefPage, SubTypeHdl));
    aSelectionLB.SetDoubleClickHdl  (LINK(this, SwFldRefPage, InsertHdl));
    aFormatLB.SetDoubleClickHdl     (LINK(this, SwFldRefPage, InsertHdl));

    nFldDlgFmtSel = 0;

    if( !IsRefresh() )
    {
        String sUserData = GetUserData();
        if(!IsRefresh() && sUserData.GetToken(0, ';').
                                EqualsIgnoreCaseAscii(USER_DATA_VERSION_1))
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

    if (IsFldEdit())
    {
        aTypeLB.SaveValue();
        aSelectionLB.SaveValue();
        aFormatLB.SaveValue();
        aNameED.SaveValue();
        aValueED.SaveValue();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldRefPage, TypeHdl, ListBox *, pBox )
{
    // Alte ListBoxPos sichern
    const USHORT nOld = GetTypeSel();

    // Aktuelle ListBoxPos
    SetTypeSel(aTypeLB.GetSelectEntryPos());

    if(GetTypeSel() == LISTBOX_ENTRY_NOTFOUND)
    {
        if (IsFldEdit())
        {
            // Positionen selektieren
            String sName;
            USHORT nFlag = 0;

            switch( GetCurField()->GetSubType() )
            {
                case REF_BOOKMARK:
                    sName = sBookmarkTxt;
                    nFlag = REFFLDFLAG_BOOKMARK;
                    break;

                case REF_FOOTNOTE:
                    sName = sFootnoteTxt;
                    nFlag = REFFLDFLAG_FOOTNOTE;
                    break;

                case REF_ENDNOTE:
                    sName = sEndnoteTxt;
                    nFlag = REFFLDFLAG_ENDNOTE;
                    break;

                case REF_SETREFATTR:
                    sName = SW_RESSTR(STR_GETREFFLD);
                    nFlag = REF_SETREFATTR;
                    break;

                case REF_SEQUENCEFLD:
                    sName = ((SwGetRefField*)GetCurField())->GetSetRefName();
                    nFlag = REFFLDFLAG;
                    break;
            }

            if (aTypeLB.GetEntryPos(sName) == LISTBOX_ENTRY_NOTFOUND)   // Referenz zu gel”schter Marke
            {
                USHORT nPos = aTypeLB.InsertEntry(sName);
                aTypeLB.SetEntryData(nPos, (void*)nFlag);
            }

            aTypeLB.SelectEntry(sName);
            SetTypeSel(aTypeLB.GetSelectEntryPos());
        }
        else
        {
            SetTypeSel(0);
            aTypeLB.SelectEntryPos(0);
        }
    }

    if (nOld != GetTypeSel())
    {
        USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

        // Auswahl-Listbox fuellen
        UpdateSubType();

        BOOL bName = FALSE;
        nFldDlgFmtSel = 0;

        if ((!IsFldEdit() || aSelectionLB.GetEntryCount()) && nOld != LISTBOX_ENTRY_NOTFOUND)
        {
            aNameED.SetText(aEmptyStr);
            aValueED.SetText(aEmptyStr);
        }

        switch (nTypeId)
        {
            case TYP_GETREFFLD:
                if (REFFLDFLAG & (USHORT)(ULONG)aTypeLB.GetEntryData(nOld))
                    // dann bleibt die alte bestehen
                    nFldDlgFmtSel = aFormatLB.GetSelectEntryPos();
                bName = TRUE;
                break;

            case TYP_SETREFFLD:
                bName = TRUE;
                break;

            case REFFLDFLAG_BOOKMARK:
                bName = TRUE;
                // kein break!!!
            default:
                if( REFFLDFLAG & nTypeId )
                {
                    USHORT nOldId = (USHORT)(ULONG)aTypeLB.GetEntryData(nOld);
                    if( nOldId & REFFLDFLAG || nOldId == TYP_GETREFFLD )
                        // dann bleibt die alte bestehen
                        nFldDlgFmtSel = aFormatLB.GetSelectEntryPos();
                }
                break;
        }

        aNameED.Enable(bName);
        aNameFT.Enable(bName);

        // Format-Listbox fuellen
        USHORT nSize = FillFormatLB(nTypeId);
        BOOL bFormat = nSize != 0;
        aFormatLB.Enable(bFormat);
        aFormatFT.Enable(bFormat);

        SubTypeHdl();
        ModifyHdl();
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldRefPage, SubTypeHdl, ListBox *, pBox )
{
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            if (!IsFldEdit() || aSelectionLB.GetSelectEntryCount())
            {
                aNameED.SetText(aSelectionLB.GetSelectEntry());
                ModifyHdl(&aNameED);
            }
            break;

        case TYP_SETREFFLD:
            aValueED.SetText(::GetActiveView()->GetWrtShell().GetSelTxt());
            break;

        default:
            if (!IsFldEdit() || aSelectionLB.GetSelectEntryCount())
                aNameED.SetText(aSelectionLB.GetSelectEntry());
            break;
    }

    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung: Typen in SelectionLB erneuern
 --------------------------------------------------------------------*/

void SwFldRefPage::UpdateSubType()
{
    SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();
    SwGetRefField* pRefFld = (SwGetRefField*)GetCurField();
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    String sOldSel;

    USHORT nSelectionSel = aSelectionLB.GetSelectEntryPos();
    if (nSelectionSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = aSelectionLB.GetEntry(nSelectionSel);

    aSelectionLB.SetUpdateMode(FALSE);
    aSelectionLB.Clear();

    if (IsFldEdit() && !sOldSel.Len())
        sOldSel = String::CreateFromInt32( pRefFld->GetSeqNo() + 1 );

    if( REFFLDFLAG & nTypeId )
    {
        if (nTypeId == REFFLDFLAG_BOOKMARK)     // TextMarken!
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle()|WB_SORT);
            // alle Textmarken besorgen
            USHORT nCnt = pSh->GetBookmarkCnt(TRUE);
            for( USHORT n = 0; n < nCnt; ++n )
            {
                const SwBookmark& rBkmk = pSh->GetBookmark( n, TRUE );
                aSelectionLB.InsertEntry( rBkmk.GetName() );
            }
            if (IsFldEdit())
                sOldSel = pRefFld->GetSetRefName();
        }
        else if (nTypeId == REFFLDFLAG_FOOTNOTE)
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle() & ~WB_SORT);
            SwSeqFldList aArr;
            USHORT nCnt = pSh->GetSeqFtnList( aArr );
            USHORT nFnd = 0;

            for( USHORT n = 0; n < nCnt; ++n )
            {
                aSelectionLB.InsertEntry( aArr[ n ]->sDlgEntry );
                if (IsFldEdit() && pRefFld->GetSeqNo() == aArr[ n ]->nSeqNo)
                    sOldSel = aArr[n]->sDlgEntry;
            }
        }
        else if (nTypeId == REFFLDFLAG_ENDNOTE)
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle() & ~WB_SORT);
            SwSeqFldList aArr;
            USHORT nCnt = pSh->GetSeqFtnList( aArr, TRUE );
            USHORT nFnd = 0;

            for( USHORT n = 0; n < nCnt; ++n )
            {
                aSelectionLB.InsertEntry( aArr[ n ]->sDlgEntry );
                if (IsFldEdit() && pRefFld->GetSeqNo() == aArr[ n ]->nSeqNo)
                    sOldSel = aArr[n]->sDlgEntry;
            }
        }
        else
        {
            aSelectionLB.SetStyle(aSelectionLB.GetStyle()|WB_SORT);
            // zum Seq-FeldTyp die Felder besorgen:

            SwSetExpFieldType* pType = (SwSetExpFieldType*)pSh->GetFldType(
                                nTypeId & ~REFFLDFLAG, RES_SETEXPFLD );
            if( pType )
            {
                SwSeqFldList aArr;
                sOldSel.Erase();

                USHORT nCnt = pType->GetSeqFldList( aArr );
                for( USHORT n = 0; n < nCnt; ++n )
                {
                    aSelectionLB.InsertEntry( aArr[ n ]->sDlgEntry );
                    if (IsFldEdit() && !sOldSel.Len() &&
                        aArr[ n ]->nSeqNo == pRefFld->GetSeqNo())
                        sOldSel = aArr[ n ]->sDlgEntry;
                }

                if (IsFldEdit() && !sOldSel.Len())
                    sOldSel = String::CreateFromInt32( pRefFld->GetSeqNo() + 1);
            }
        }
    }
    else
    {
        SvStringsDtor& rLst = GetFldMgr().GetSubTypes(nTypeId);
        for (USHORT i = 0; i < rLst.Count(); ++i)
            aSelectionLB.InsertEntry(*rLst[i]);

        if (IsFldEdit())
            sOldSel = pRefFld->GetSetRefName();
    }

    aSelectionLB.SetUpdateMode(TRUE);

    // Enable oder Disable
    BOOL bEnable = aSelectionLB.GetEntryCount() != 0;
    aSelectionLB.Enable( bEnable );
    aSelectionFT.Enable( bEnable );

    if ( bEnable )
    {
        aSelectionLB.SelectEntry(sOldSel);
        if (!aSelectionLB.GetSelectEntryCount() && !IsFldEdit())
            aSelectionLB.SelectEntryPos(0);
    }

    if (IsFldEdit() && !aSelectionLB.GetSelectEntryCount()) // Falls die Referenz schon geloescht wurde...
        aNameED.SetText(sOldSel);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldRefPage::FillFormatLB(USHORT nTypeId)
{
    String sOldSel;

    USHORT nFormatSel = aFormatLB.GetSelectEntryPos();
    if (nFormatSel != LISTBOX_ENTRY_NOTFOUND)
        sOldSel = aFormatLB.GetEntry(nFormatSel);

    // Format-Listbox fuellen
    aFormatLB.Clear();

    // Referenz hat weniger als die Beschriftung
    USHORT nSize;
    switch (nTypeId)
    {
    case TYP_GETREFFLD:
    case REFFLDFLAG_BOOKMARK:
    case REFFLDFLAG_FOOTNOTE:
    case REFFLDFLAG_ENDNOTE:
        nSize = FMT_REF_PAGE_PGDSC - FMT_REF_BEGIN + 1;
        break;

    default:
        nSize = GetFldMgr().GetFormatCount( (REFFLDFLAG & nTypeId)
                                                ? TYP_GETREFFLD : nTypeId,
                                            FALSE, IsFldDlgHtmlMode() );
        break;
    }

    if (REFFLDFLAG & nTypeId)
        nTypeId = TYP_GETREFFLD;

    for (USHORT i = 0; i < nSize; i++)
    {
        USHORT nPos = aFormatLB.InsertEntry(GetFldMgr().GetFormatStr( nTypeId, i ));
        aFormatLB.SetEntryData( nPos, (void*)GetFldMgr().GetFormatId( nTypeId, i ));
    }

    if (nSize)
    {
        if (!IsFldEdit())
            aFormatLB.SelectEntry(sOldSel);
        else
            aFormatLB.SelectEntry(SW_RESSTR(FMT_REF_BEGIN + (USHORT)GetCurField()->GetFormat()));

        if (!aFormatLB.GetSelectEntryCount())
        {
            aFormatLB.SelectEntryPos(nFldDlgFmtSel);
            if (!aFormatLB.GetSelectEntryCount())
                aFormatLB.SelectEntryPos(0);
        }
    }

    return nSize;
}

/*--------------------------------------------------------------------
    Beschreibung: Modify
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldRefPage, ModifyHdl, Edit *, EMPTYARG )
{
    String aName(aNameED.GetText());
    const USHORT nLen = aName.Len();

    BOOL bEnable = TRUE;
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    if ((nTypeId == TYP_SETREFFLD && !GetFldMgr().CanInsertRefMark(aName)) ||
        (nLen == 0 && (nTypeId == TYP_GETREFFLD || nTypeId == TYP_SETREFFLD ||
                       nTypeId == REFFLDFLAG_BOOKMARK)))
        bEnable = FALSE;

    EnableInsert(bEnable);

    aSelectionLB.SelectEntry(aName);

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwFldRefPage::FillItemSet(SfxItemSet& )
{
    BOOL bPage = FALSE;
    BOOL bModified = FALSE;
    USHORT nTypeId = (USHORT)(ULONG)aTypeLB.GetEntryData(GetTypeSel());

    USHORT nSubType = 0;
    ULONG nFormat;

    nFormat = aFormatLB.GetSelectEntryPos();

    if(nFormat == LISTBOX_ENTRY_NOTFOUND)
        nFormat = 0;
    else
        nFormat = (ULONG)aFormatLB.GetEntryData((USHORT)nFormat);

    String aVal(aValueED.GetText());
    String aName(aNameED.GetText());

    switch(nTypeId)
    {
        case TYP_GETREFFLD:
            // aName = aSelectionLB.GetSelectEntry();
            nSubType = REF_SETREFATTR;
            break;

        case TYP_SETREFFLD:
        {
            SwFieldType* pType = GetFldMgr().GetFldType(RES_SETEXPFLD, aName);

            if(!pType)  // Nur einfuegen, wenn es den Namen noch nicht gibt
            {
                aSelectionLB.InsertEntry(aName);
                aSelectionLB.Enable();
                aSelectionFT.Enable();
            }
            break;
        }
    }

    SwGetRefField* pRefFld = (SwGetRefField*)GetCurField();

    if (REFFLDFLAG & nTypeId)
    {
        SwView *pView = ::GetActiveView();
        ASSERT(pView, View fehlt);
        SwWrtShell &rSh = pView->GetWrtShell();

        if (nTypeId == REFFLDFLAG_BOOKMARK)     // TextMarken!
        {
            aName = aNameED.GetText();
            nTypeId = TYP_GETREFFLD;
            nSubType = REF_BOOKMARK;
        }
        else if (REFFLDFLAG_FOOTNOTE == nTypeId)        // Fussnoten
        {
            SwSeqFldList aArr;
            _SeqFldLstElem aElem( aSelectionLB.GetSelectEntry(), 0 );

            USHORT nPos;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_FOOTNOTE;
            aName.Erase();

            if (rSh.GetSeqFtnList(aArr) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = String::CreateFromInt32( aArr[nPos]->nSeqNo );

                if (IsFldEdit() && COMPARE_EQUAL != aVal.CompareTo(pRefFld->GetSeqNo()))
                    bModified = TRUE; // Kann bei Feldern passieren, deren Referenz geloescht wurde
            }
            else if (IsFldEdit())
                aVal = String::CreateFromInt32( pRefFld->GetSeqNo() );
        }
        else if (REFFLDFLAG_ENDNOTE == nTypeId)         // Endnoten
        {
            SwSeqFldList aArr;
            _SeqFldLstElem aElem( aSelectionLB.GetSelectEntry(), 0 );

            USHORT nPos;

            nTypeId = TYP_GETREFFLD;
            nSubType = REF_ENDNOTE;
            aName.Erase();

            if (rSh.GetSeqFtnList(aArr, TRUE) && aArr.SeekEntry(aElem, &nPos))
            {
                aVal = String::CreateFromInt32( aArr[nPos]->nSeqNo );

                if (IsFldEdit() && COMPARE_EQUAL != aVal.CompareTo(pRefFld->GetSeqNo()))
                    bModified = TRUE; // Kann bei Feldern passieren, deren Referenz geloescht wurde
            }
            else if (IsFldEdit())
                aVal = String::CreateFromInt32( pRefFld->GetSeqNo() );
        }
        else                                // SeqenceFelder
        {
            // zum Seq-FeldTyp die Felder besorgen:
            SwSetExpFieldType* pType = (SwSetExpFieldType*)rSh.GetFldType(
                                    nTypeId & ~REFFLDFLAG, RES_SETEXPFLD );
            if( pType )
            {
                SwSeqFldList aArr;
                _SeqFldLstElem aElem( aSelectionLB.GetSelectEntry(), 0 );

                USHORT nPos;

                nTypeId = TYP_GETREFFLD;
                nSubType = REF_SEQUENCEFLD;
                aName = pType->GetName();

                if (pType->GetSeqFldList(aArr) && aArr.SeekEntry(aElem, &nPos))
                {
                    aVal = String::CreateFromInt32( aArr[nPos]->nSeqNo );

                    if (IsFldEdit() && COMPARE_EQUAL != aVal.CompareTo(pRefFld->GetSeqNo()))
                        bModified = TRUE; // Kann bei Feldern passieren, deren Referenz geloescht wurde
                }
                else if (IsFldEdit())
                    aVal = String::CreateFromInt32( pRefFld->GetSeqNo() );
            }
        }
    }

    if (IsFldEdit() && nTypeId == TYP_GETREFFLD)
    {
        aVal.Insert('|', 0);
        aVal.Insert(String::CreateFromInt32(nSubType), 0);
    }

    if (!IsFldEdit() || bModified ||
        aNameED.GetSavedValue() != aNameED.GetText() ||
        aValueED.GetSavedValue() != aValueED.GetText() ||
        aTypeLB.GetSavedValue() != aTypeLB.GetSelectEntryPos() ||
        aSelectionLB.GetSavedValue() != aSelectionLB.GetSelectEntryPos() ||
        aFormatLB.GetSavedValue() != aFormatLB.GetSelectEntryPos())
    {
        InsertFld( nTypeId, nSubType, aName, aVal, nFormat );
    }

    ModifyHdl();    // Insert ggf enablen/disablen

    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* SwFldRefPage::Create(   Window* pParent,
                        const SfxItemSet& rAttrSet )
{
    return ( new SwFldRefPage( pParent, rAttrSet ) );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwFldRefPage::GetGroup()
{
    return GRP_REF;
}

/* -----------------12.01.99 10:09-------------------
 *
 * --------------------------------------------------*/
void    SwFldRefPage::FillUserData()
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
    Revision 1.32  2000/09/18 16:05:29  willem.vandorp
    OpenOffice header added.

    Revision 1.31  2000/08/31 10:07:13  os
    #78315# CreateFromInt32

    Revision 1.30  2000/07/20 08:10:52  jp
    Unicode-Change-Error: call createfromint32

    Revision 1.29  2000/06/30 08:52:53  os
    #76541# string assertions removed

    Revision 1.28  2000/06/19 14:06:07  os
    #72953# call SubTypeHdl more often

    Revision 1.27  2000/05/23 18:36:48  jp
    Bugfixes for Unicode

    Revision 1.26  2000/04/18 15:17:32  os
    UNICODE

    Revision 1.25  1999/09/28 11:10:35  os
    #68076# call modify handler if a name is selected

    Revision 1.24  1999/04/06 09:27:54  JP
    Task #64320#: bei Referenz auf Nummernkreis auch die Nummer als Format anbieten


      Rev 1.23   06 Apr 1999 11:27:54   JP
   Task #64320#: bei Referenz auf Nummernkreis auch die Nummer als Format anbieten

      Rev 1.22   25 Feb 1999 17:24:52   JP
   Bug #62438#: UserData nur auswerten, wenn kein Refresh ist

      Rev 1.21   21 Jan 1999 09:46:22   OS
   #59900# Fussnoten im Dialog korrekt sortieren; keine prot. Member

      Rev 1.20   20 Jan 1999 14:25:40   MIB
   Keine leeren Refs auf Textmarken

      Rev 1.19   12 Jan 1999 11:42:54   OS
   #60579# ausgewaehlten Typ in den UserData speichern

      Rev 1.18   10 Aug 1998 16:41:12   JP
   Bug #54796#: neue NumerierungsTypen (WW97 kompatibel)

      Rev 1.17   23 Jun 1998 14:43:10   OM
   Referenzen und Textmarken mit beliebigem Namen einfuegen

      Rev 1.16   22 Jun 1998 17:58:02   OM
   Vorbereitung fuer eigenstaendige Referenzfelder

      Rev 1.15   18 May 1998 15:21:34   OM
   #50222 Korrekte Formate fuer Endnoten

      Rev 1.14   27 Mar 1998 16:56:42   OM
   Nur bei Aenderung Dok modifizieren

      Rev 1.13   23 Feb 1998 07:26:36   OS
   GetBookmark/ Cnt mit Bookmark-Flag

      Rev 1.12   16 Feb 1998 10:10:22   OM
   Fuer Solaris nicht direkt von void* auf ushort casten

      Rev 1.11   30 Jan 1998 15:25:36   OM
   Endnoten

      Rev 1.10   27 Jan 1998 22:43:52   JP
   GetNumDepend durch GetDepends ersetzt

      Rev 1.9   09 Jan 1998 16:56:52   OM
   Bei Dok-Wechsel updaten

      Rev 1.8   08 Jan 1998 14:58:26   OM
   Traveling

      Rev 1.7   08 Jan 1998 10:19:04   OM
   Referenzen editieren

      Rev 1.6   07 Jan 1998 17:17:42   OM
   Referenzen editieren

      Rev 1.5   19 Dec 1997 18:24:26   OM
   Feldbefehl-bearbeiten Dlg

      Rev 1.4   12 Dec 1997 16:10:34   OM
   AutoUpdate bei FocusWechsel u.a.

      Rev 1.3   11 Dec 1997 16:58:50   OM
   Feldumstellung

      Rev 1.2   08 Dec 1997 12:32:22   MA
   vorb. Endnoten

      Rev 1.1   24 Nov 1997 14:40:24   OM
   Referenz-TP

      Rev 1.0   04 Nov 1997 10:07:20   OM
   Initial revision.

------------------------------------------------------------------------*/

