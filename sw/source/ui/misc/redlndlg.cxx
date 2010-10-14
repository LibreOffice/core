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


#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS


#include <redline.hxx>
#include <tools/datetime.hxx>
#include <vcl/msgbox.hxx>
#include <svl/svstdarr.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#ifndef _REDLNDLG_HXX
#define _REDLNACCEPTDLG
#include <redlndlg.hxx>
#endif
#include <swwait.hxx>
#include <uitool.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <misc.hrc>
#include <redlndlg.hrc>
#include <shells.hrc>

// -> #111827#
#include <comcore.hrc>
#include <swundo.hxx>
#include <undobj.hxx>
#include <SwRewriter.hxx>
// <- #111827#

#include <vector>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <unomid.h>

#include <docsh.hxx>

#include <IDocumentRedlineAccess.hxx>

SFX_IMPL_MODELESSDIALOG( SwRedlineAcceptChild, FN_REDLINE_ACCEPT )

SV_IMPL_PTRARR(SwRedlineDataParentArr, SwRedlineDataParentPtr)
SV_IMPL_OP_PTRARR_SORT(SwRedlineDataParentSortArr, SwRedlineDataParentPtr)
SV_IMPL_PTRARR(SwRedlineDataChildArr, SwRedlineDataChildPtr)
SV_IMPL_PTRARR(SvLBoxEntryArr, SvLBoxEntryPtr)

static USHORT nSortMode = 0xffff;
static BOOL   bSortDir = TRUE;

SwRedlineAcceptChild::SwRedlineAcceptChild( Window* _pParent,
                                            USHORT nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
    SwChildWinWrapper( _pParent, nId )
{
    pWindow = new SwModelessRedlineAcceptDlg( pBindings, this, _pParent);

    ((SwModelessRedlineAcceptDlg *)pWindow)->Initialize(pInfo);
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/
BOOL SwRedlineAcceptChild::ReInitDlg(SwDocShell *pDocSh)
{
    BOOL bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == TRUE)  // Sofort aktualisieren, Dok-Wechsel
        ((SwModelessRedlineAcceptDlg*)GetWindow())->Activate();

    return bRet;
}

SwModelessRedlineAcceptDlg::SwModelessRedlineAcceptDlg( SfxBindings* _pBindings,
                                                        SwChildWinWrapper* pChild,
                                                        Window *_pParent) :
    SfxModelessDialog(_pBindings, pChild, _pParent, SW_RES(DLG_REDLINE_ACCEPT)),
    pChildWin       (pChild)
{
    pImplDlg = new SwRedlineAcceptDlg(this);

    FreeResource();
}

void SwModelessRedlineAcceptDlg::Activate()
{
    SwView *pView = ::GetActiveView();

    if (!pView) // Kann passieren, wenn man auf eine andere App umschaltet, wenn
        return; // vorher eine Listbox im Dialog den Focus hatte (eigentlich THs Bug)

    SwDocShell *pDocSh = pView->GetDocShell();

    if (pChildWin->GetOldDocShell() != pDocSh)
    {   // Dok-Wechsel
        SwWait aWait( *pDocSh, FALSE );
        SwWrtShell* pSh = pView->GetWrtShellPtr();

        pChildWin->SetOldDocShell(pDocSh);  // Rekursion vermeiden (durch Modified-Hdl)

        BOOL bMod = pSh->IsModified();
        SfxBoolItem aShow(FN_REDLINE_SHOW, TRUE);
        pSh->GetView().GetViewFrame()->GetDispatcher()->Execute(
            FN_REDLINE_SHOW, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aShow, 0L);
        if (!bMod)
            pSh->ResetModified();
        pImplDlg->Init();

        return;
    }

    pImplDlg->Activate();
}

void SwModelessRedlineAcceptDlg::Initialize(SfxChildWinInfo *pInfo)
{
    String aStr;
    if (pInfo != NULL)
        pImplDlg->Initialize(pInfo->aExtraString);

    SfxModelessDialog::Initialize(pInfo);
}

void SwModelessRedlineAcceptDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxModelessDialog::FillInfo(rInfo);
    pImplDlg->FillInfo(rInfo.aExtraString);
}

void SwModelessRedlineAcceptDlg::Resize()
{
    pImplDlg->Resize();
    SfxModelessDialog::Resize();
}

SwModelessRedlineAcceptDlg::~SwModelessRedlineAcceptDlg()
{
    delete pImplDlg;
}

SwRedlineAcceptDlg::SwRedlineAcceptDlg(Dialog *pParent, BOOL bAutoFmt) :
    pParentDlg      (pParent),
    aTabPagesCTRL   (pParent, SW_RES(CTRL_TABPAGES)),
    aPopup          (SW_RES(MN_REDLINE_POPUP)),
    sInserted       (SW_RES(STR_REDLINE_INSERTED)),
    sDeleted        (SW_RES(STR_REDLINE_DELETED)),
    sFormated       (SW_RES(STR_REDLINE_FORMATED)),
    sTableChgd      (SW_RES(STR_REDLINE_TABLECHG)),
    sFmtCollSet     (SW_RES(STR_REDLINE_FMTCOLLSET)),
    sAutoFormat     (SW_RES(STR_REDLINE_AUTOFMT)),
    bOnlyFormatedRedlines( FALSE ),
    bHasReadonlySel ( FALSE ),
    bRedlnAutoFmt   (bAutoFmt),
    bInhibitActivate( false )
{
    aTabPagesCTRL.SetHelpId(HID_REDLINE_CTRL);
    pTPView = aTabPagesCTRL.GetViewPage();
    pTable = pTPView->GetTableControl();

    pTPView->InsertWriterHeader();
    pTPView->SetAcceptClickHdl(LINK(this, SwRedlineAcceptDlg, AcceptHdl));
    pTPView->SetAcceptAllClickHdl(LINK(this, SwRedlineAcceptDlg, AcceptAllHdl));
    pTPView->SetRejectClickHdl(LINK(this, SwRedlineAcceptDlg, RejectHdl));
    pTPView->SetRejectAllClickHdl(LINK(this, SwRedlineAcceptDlg, RejectAllHdl));
    pTPView->SetUndoClickHdl(LINK(this, SwRedlineAcceptDlg, UndoHdl));

    aTabPagesCTRL.GetFilterPage()->SetReadyHdl(LINK(this, SwRedlineAcceptDlg, FilterChangedHdl));

    ListBox *pActLB = aTabPagesCTRL.GetFilterPage()->GetLbAction();
    pActLB->InsertEntry(sInserted);
    pActLB->InsertEntry(sDeleted);
    pActLB->InsertEntry(sFormated);
    pActLB->InsertEntry(sTableChgd);

    if (HasRedlineAutoFmt())
    {
        pActLB->InsertEntry(sFmtCollSet);
        pActLB->InsertEntry(sAutoFormat);
        pTPView->ShowUndo(TRUE);
        pTPView->DisableUndo();     // Noch gibts keine UNDO-Events
    }

    pActLB->SelectEntryPos(0);

    pTable->SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    pTable->SetNodeDefaultImages();
    pTable->SetSelectionMode(MULTIPLE_SELECTION);
    pTable->SetHighlightRange(1);

    static long aStaticTabs[]=
    {
        4,10,70,120,170
    };

    pTable->SetTabs(aStaticTabs);

    // Minimalgroesse setzen
    Size aMinSz(aTabPagesCTRL.GetMinSizePixel());
    Point aPos(aTabPagesCTRL.GetPosPixel());

    aMinSz.Width() += (aPos.X() * 2 - 1);
    aMinSz.Height() += (aPos.Y() * 2 - 1);
    pParentDlg->SetMinOutputSizePixel(aMinSz);

    if (pParentDlg->GetOutputSizePixel().Width() < aMinSz.Width())
        pParentDlg->SetOutputSizePixel(Size(aMinSz.Width(), pParentDlg->GetOutputSizePixel().Height()));
    if (pParentDlg->GetOutputSizePixel().Height() < aMinSz.Height())
        pParentDlg->SetOutputSizePixel(Size(pParentDlg->GetOutputSizePixel().Width(), aMinSz.Height()));

    pTable->SortByCol(nSortMode, bSortDir);

    aOldSelectHdl = pTable->GetSelectHdl();
    aOldDeselectHdl = pTable->GetDeselectHdl();
    pTable->SetSelectHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    pTable->SetDeselectHdl(LINK(this, SwRedlineAcceptDlg, DeselectHdl));
    pTable->SetCommandHdl(LINK(this, SwRedlineAcceptDlg, CommandHdl));

    // Flackern der Buttons vermeiden:
    aDeselectTimer.SetTimeout(100);
    aDeselectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));

    // Mehrfachselektion der selben Texte vermeiden:
    aSelectTimer.SetTimeout(100);
    aSelectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, GotoHdl));
}

SwRedlineAcceptDlg::~SwRedlineAcceptDlg()
{
}

void SwRedlineAcceptDlg::Init(USHORT nStart)
{
    SwWait aWait( *::GetActiveView()->GetDocShell(), FALSE );
    pTable->SetUpdateMode(FALSE);
    aUsedSeqNo.Remove((USHORT)0, aUsedSeqNo.Count());

    if (nStart)
        RemoveParents(nStart, aRedlineParents.Count() - 1);
    else
    {
        pTable->Clear();
        aRedlineChilds.DeleteAndDestroy(0, aRedlineChilds.Count());
        aRedlineParents.DeleteAndDestroy(nStart, aRedlineParents.Count() - nStart);
    }

    // Parents einfuegen
    InsertParents(nStart);
    InitAuthors();

    pTable->SetUpdateMode(TRUE);
    // #i69618# this moves the list box to the right position, visually
    SvLBoxEntry* pSelEntry = pTable->FirstSelected();
    if( pSelEntry )
        pTable->MakeVisible( pSelEntry, sal_True ); //#i70937#, force the scroll
}

void SwRedlineAcceptDlg::InitAuthors()
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();

    SvxTPFilter *pFilterPage = aTabPagesCTRL.GetFilterPage();

    String sAuthor;
    SvStringsSortDtor aStrings;
    String sOldAuthor(pFilterPage->GetSelectedAuthor());
    pFilterPage->ClearAuthors();

    String sParent;
    USHORT nCount = pSh->GetRedlineCount();

    bOnlyFormatedRedlines = TRUE;
    bHasReadonlySel = FALSE;
    BOOL bIsNotFormated = FALSE;
    USHORT i;

    // Autoren ermitteln
    for ( i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);

        if( bOnlyFormatedRedlines && nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType() )
            bOnlyFormatedRedlines = FALSE;

//JP 27.9.2001: make no sense if we handle readonly sections
//      if( !bHasReadonlySel && rRedln.HasReadonlySel() )
//          bHasReadonlySel = TRUE;

        String *pAuthor = new String(rRedln.GetAuthorString());
        if (!aStrings.Insert(pAuthor))
            delete pAuthor;

        for (USHORT nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
        {
            pAuthor = new String(rRedln.GetAuthorString(nStack));
            if (!aStrings.Insert(pAuthor))
                delete pAuthor;
        }
    }

    for (i = 0; i < aStrings.Count(); i++)
        pFilterPage->InsertAuthor(*aStrings[i]);

    if (pFilterPage->SelectAuthor(sOldAuthor) == LISTBOX_ENTRY_NOTFOUND && aStrings.Count())
        pFilterPage->SelectAuthor(*aStrings[0]);

    BOOL bEnable = pTable->GetEntryCount() != 0 && !pSh->getIDocumentRedlineAccess()->GetRedlinePassword().getLength();
    BOOL bSel = pTable->FirstSelected() != 0;

    SvLBoxEntry* pSelEntry = pTable->FirstSelected();
    while (pSelEntry)
    {
        USHORT nPos = GetRedlinePos(*pSelEntry);
        const SwRedline& rRedln = pSh->GetRedline( nPos );

        bIsNotFormated |= nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType();
        pSelEntry = pTable->NextSelected(pSelEntry);
    }

    pTPView->EnableAccept( bEnable && bSel );
    pTPView->EnableReject( bEnable && bIsNotFormated && bSel );
    pTPView->EnableAcceptAll( bEnable && !bHasReadonlySel );
    pTPView->EnableRejectAll( bEnable && !bHasReadonlySel &&
                                !bOnlyFormatedRedlines );
}

String SwRedlineAcceptDlg::GetRedlineText( const SwRedline& rRedln,
                                        DateTime &rDateTime, USHORT nStack)
{
    String sEntry(GetActionText(rRedln, nStack));
    sEntry += '\t';
    sEntry += rRedln.GetAuthorString(nStack);
    sEntry += '\t';

    const DateTime &rDT = rRedln.GetTimeStamp(nStack);
    rDateTime = rDT;

    sEntry += GetAppLangDateTimeString( rDT );
    sEntry += '\t';

    sEntry += rRedln.GetComment(nStack);

    return sEntry;
}

const String &SwRedlineAcceptDlg::GetActionText(const SwRedline& rRedln, USHORT nStack)
{
    switch( rRedln.GetType(nStack) )
    {
        case nsRedlineType_t::REDLINE_INSERT:   return sInserted;
        case nsRedlineType_t::REDLINE_DELETE:   return sDeleted;
        case nsRedlineType_t::REDLINE_FORMAT:   return sFormated;
        case nsRedlineType_t::REDLINE_TABLE:    return sTableChgd;
        case nsRedlineType_t::REDLINE_FMTCOLL:  return sFmtCollSet;
        default:;//prevent warning
    }

    return aEmptyStr;
}

void SwRedlineAcceptDlg::Resize()
{
    Size aSz(pParentDlg->GetOutputSizePixel());

    Point aPos(aTabPagesCTRL.GetPosPixel());

    aSz.Width() -= (aPos.X() * 2 - 1);
    aSz.Height() -= (aPos.Y() * 2 - 1);

    aTabPagesCTRL.SetOutputSizePixel(aSz);
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Aktivierung neu initialisieren
 --------------------------------------------------------------------*/
void SwRedlineAcceptDlg::Activate()
{
    // prevent update if flag is set (#102547#)
    if( bInhibitActivate )
        return;

    SwView *pView = ::GetActiveView();
    SwWait aWait( *pView->GetDocShell(), FALSE );

    aUsedSeqNo.Remove((USHORT)0, aUsedSeqNo.Count());

    if (!pView) // Kann passieren, wenn man auf eine andere App umschaltet, wenn
        return; // vorher eine Listbox im Dialog den Focus hatte (eigentlich THs Bug)

/*  if (HasRedlineAutoFmt())
    {
        Init();
        return;
    }*/

    // Hat sich was geaendert?
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    USHORT nCount = pSh->GetRedlineCount();

    // Anzahl und Pointer ueberpruefen
    SwRedlineDataParent *pParent = 0;
    USHORT i;

    for ( i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);

        if (i >= aRedlineParents.Count())
        {
            // Neue Eintraege wurden angehaengt
            Init(i);
            return;
        }

        pParent = aRedlineParents[i];
        if (&rRedln.GetRedlineData() != pParent->pData)
        {
            // Redline-Parents wurden eingefuegt, geaendert oder geloescht
            if ((i = CalcDiff(i, FALSE)) == USHRT_MAX)
                return;
            continue;
        }

        const SwRedlineData *pRedlineData = rRedln.GetRedlineData().Next();
        const SwRedlineDataChild *pBackupData = pParent->pNext;

        if (!pRedlineData && pBackupData)
        {
            // Redline-Childs wurden geloescht
            if ((i = CalcDiff(i, TRUE)) == USHRT_MAX)
                return;
            continue;
        }
        else
        {
            while (pRedlineData)
            {
                if (pRedlineData != pBackupData->pChild)
                {
                    // Redline-Childs wurden eingefuegt, geaendert oder geloescht
                    if ((i = CalcDiff(i, TRUE)) == USHRT_MAX)
                        return;
                    continue;
                }
                if (pBackupData)
                    pBackupData = pBackupData->pNext;
                pRedlineData = pRedlineData->Next();
            }
        }
    }

    if (nCount != aRedlineParents.Count())
    {
        // Redlines wurden am Ende geloescht
        Init(nCount);
        return;
    }

    // Kommentar ueberpruefen
    for (i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);
        pParent = aRedlineParents[i];

        if(!rRedln.GetComment().Equals(pParent->sComment))
        {
            if (pParent->pTLBParent)
            {
                // Nur Kommentar aktualisieren
                String sComment(rRedln.GetComment());
                sComment.SearchAndReplaceAll((sal_Unicode)_LF,(sal_Unicode)' ');
                pTable->SetEntryText(sComment, pParent->pTLBParent, 3);
            }
            pParent->sComment = rRedln.GetComment();
        }
    }

    InitAuthors();
}

USHORT SwRedlineAcceptDlg::CalcDiff(USHORT nStart, BOOL bChild)
{
    if (!nStart)
    {
        Init();
        return USHRT_MAX;
    }

    pTable->SetUpdateMode(FALSE);
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    USHORT nAutoFmt = HasRedlineAutoFmt() ? nsRedlineType_t::REDLINE_FORM_AUTOFMT : 0;
    SwRedlineDataParent *pParent = aRedlineParents[nStart];
    const SwRedline& rRedln = pSh->GetRedline(nStart);

    if (bChild)     // Sollte eigentlich nie vorkommen, aber sicher ist sicher...
    {
        // Alle Childs des Eintrags wegwerfen und neu initialisieren
        SwRedlineDataChildPtr pBackupData = (SwRedlineDataChildPtr)pParent->pNext;
        SwRedlineDataChildPtr pNext;

        while (pBackupData)
        {
            pNext = (SwRedlineDataChildPtr)pBackupData->pNext;
            if (pBackupData->pTLBChild)
                pTable->RemoveEntry(pBackupData->pTLBChild);

            aRedlineChilds.DeleteAndDestroy(aRedlineChilds.GetPos(pBackupData), 1);
            pBackupData = pNext;
        }
        pParent->pNext = 0;

        // Neue Childs einfuegen
        InsertChilds(pParent, rRedln, nAutoFmt);

        pTable->SetUpdateMode(TRUE);
        return nStart;
    }

    // Wurden Eintraege geloescht?
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    USHORT i;
    for ( i = nStart + 1; i < aRedlineParents.Count(); i++)
    {
        if (aRedlineParents[i]->pData == pRedlineData)
        {
            // Eintraege von nStart bis i-1 entfernen
            RemoveParents(nStart, i - 1);
            pTable->SetUpdateMode(TRUE);
            return nStart - 1;
        }
    }

    // Wurden Eintraege eingefuegt?
    USHORT nCount = pSh->GetRedlineCount();
    pRedlineData = aRedlineParents[nStart]->pData;

    for (i = nStart + 1; i < nCount; i++)
    {
        if (&pSh->GetRedline(i).GetRedlineData() == pRedlineData)
        {
            // Eintraege von nStart bis i-1 einfuegen
            InsertParents(nStart, i - 1);
            pTable->SetUpdateMode(TRUE);
            return nStart - 1;
        }
    }

    pTable->SetUpdateMode(TRUE);
    Init(nStart);   // Alle Eintraege bis zum Ende abgleichen
    return USHRT_MAX;
}

void SwRedlineAcceptDlg::InsertChilds(SwRedlineDataParent *pParent, const SwRedline& rRedln, const USHORT nAutoFmt)
{
    String sChild;
    SwRedlineDataChild *pLastRedlineChild = 0;
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    BOOL bAutoFmt = (rRedln.GetRealType() & nAutoFmt) != 0;

    const String *pAction = &GetActionText(rRedln);
    BOOL bValidParent = !sFilterAction.Len() || sFilterAction == *pAction;
    bValidParent = bValidParent && pTable->IsValidEntry(&rRedln.GetAuthorString(), &rRedln.GetTimeStamp(), &rRedln.GetComment());
    if (nAutoFmt)
    {
        USHORT nPos;

        if (pParent->pData->GetSeqNo() && !aUsedSeqNo.Insert(pParent, nPos))    // Gibts schon
        {
            if (pParent->pTLBParent)
            {
                pTable->SetEntryText(sAutoFormat, aUsedSeqNo[nPos]->pTLBParent, 0);
                pTable->RemoveEntry(pParent->pTLBParent);
                pParent->pTLBParent = 0;
            }
            return;
        }
        bValidParent = bValidParent && bAutoFmt;
    }
    BOOL bValidTree = bValidParent;

    for (USHORT nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
    {
        pRedlineData = pRedlineData->Next();

        SwRedlineDataChildPtr pRedlineChild = new SwRedlineDataChild;
        pRedlineChild->pChild = pRedlineData;
        aRedlineChilds.Insert(pRedlineChild, aRedlineChilds.Count());

        if ( pLastRedlineChild )
            pLastRedlineChild->pNext = pRedlineChild;
        else
            pParent->pNext = pRedlineChild;

        pAction = &GetActionText(rRedln, nStack);
        BOOL bValidChild = !sFilterAction.Len() || sFilterAction == *pAction;
        bValidChild = bValidChild && pTable->IsValidEntry(&rRedln.GetAuthorString(nStack), &rRedln.GetTimeStamp(nStack), &rRedln.GetComment());
        if (nAutoFmt)
            bValidChild = bValidChild && bAutoFmt;
        bValidTree |= bValidChild;

        if (bValidChild)
        {
            RedlinData *pData = new RedlinData;
            pData->pData = pRedlineChild;
            pData->bDisabled = TRUE;
            sChild = GetRedlineText(rRedln, pData->aDateTime, nStack);

            SvLBoxEntry* pChild = pTable->InsertEntry(sChild, pData, pParent->pTLBParent);

            pRedlineChild->pTLBChild = pChild;
            if (!bValidParent)
                pTable->Expand(pParent->pTLBParent);
        }
        else
            pRedlineChild->pTLBChild = 0;

        pLastRedlineChild = pRedlineChild;
    }

    if (pLastRedlineChild)
        pLastRedlineChild->pNext = 0;

    if (!bValidTree && pParent->pTLBParent)
    {
        pTable->RemoveEntry(pParent->pTLBParent);
        pParent->pTLBParent = 0;
        if (nAutoFmt)
            aUsedSeqNo.Remove(pParent);
    }
}

void SwRedlineAcceptDlg::RemoveParents(USHORT nStart, USHORT nEnd)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    USHORT nCount = pSh->GetRedlineCount();

    SvLBoxEntryArr aLBoxArr;

    // Wegen Bug der TLB, die bei Remove den SelectHandler IMMER ruft:
    pTable->SetSelectHdl(aOldSelectHdl);
    pTable->SetDeselectHdl(aOldDeselectHdl);
    BOOL bChildsRemoved = FALSE;
    pTable->SelectAll(FALSE);

    // Hinter dem letzten Eintrag Cursor setzen, da sonst Performance-Problem in TLB.
    // TLB wuerde sonst bei jedem Remove den Cursor erneut umsetzen (teuer)
    USHORT nPos = Min((USHORT)nCount, (USHORT)aRedlineParents.Count());
    SvLBoxEntry *pCurEntry = NULL;
    while( ( pCurEntry == NULL ) && ( nPos > 0 ) )
    {
        --nPos;
        pCurEntry = aRedlineParents[nPos]->pTLBParent;
    }

    if (pCurEntry)
        pTable->SetCurEntry(pCurEntry);

    SvLBoxTreeList* pModel = pTable->GetModel();

    for (USHORT i = nStart; i <= nEnd; i++)
    {
        if (!bChildsRemoved && aRedlineParents[i]->pNext)
        {
            SwRedlineDataChildPtr pChildPtr = (SwRedlineDataChildPtr)aRedlineParents[i]->pNext;
            USHORT nChildPos = aRedlineChilds.GetPos(pChildPtr);

            if (nChildPos != USHRT_MAX)
            {
                USHORT nChilds = 0;

                while (pChildPtr)
                {
                    pChildPtr = (SwRedlineDataChildPtr)pChildPtr->pNext;
                    nChilds++;
                }

                aRedlineChilds.DeleteAndDestroy(nChildPos, nChilds);
                bChildsRemoved = TRUE;
            }
        }
        SvLBoxEntry *pEntry = aRedlineParents[i]->pTLBParent;
        if (pEntry)
        {
            long nIdx = aLBoxArr.Count() - 1L;
            ULONG nAbsPos = pModel->GetAbsPos(pEntry);
            while (nIdx >= 0 &&
                    pModel->GetAbsPos(aLBoxArr[ static_cast< USHORT >(nIdx) ]) > nAbsPos)
                nIdx--;
            aLBoxArr.Insert( pEntry, static_cast< USHORT >(++nIdx) );
        }
    }

    // TLB von hinten abraeumen
    long nIdx = (long)aLBoxArr.Count() - 1L;
    while (nIdx >= 0)
        pTable->RemoveEntry(aLBoxArr[ static_cast< USHORT >(nIdx--) ]);

    pTable->SetSelectHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    pTable->SetDeselectHdl(LINK(this, SwRedlineAcceptDlg, DeselectHdl));
    // Durch Remove wurde leider wieder dauernd von der TLB selektiert...
    pTable->SelectAll(FALSE);

    aRedlineParents.DeleteAndDestroy( nStart, nEnd - nStart + 1);
}

void SwRedlineAcceptDlg::InsertParents(USHORT nStart, USHORT nEnd)
{
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    USHORT nAutoFmt = HasRedlineAutoFmt() ? nsRedlineType_t::REDLINE_FORM_AUTOFMT : 0;

    String sParent;
    USHORT nCount = pSh->GetRedlineCount();
    nEnd = Min((USHORT)nEnd, (USHORT)(nCount - 1)); // Handelt auch nEnd=USHRT_MAX (bis zum Ende) ab

    if (nEnd == USHRT_MAX)
        return;     // Keine Redlines im Dokument

    RedlinData *pData;
    SvLBoxEntry *pParent;
    SwRedlineDataParentPtr pRedlineParent;
    const SwRedline* pCurrRedline;
    if( !nStart && !pTable->FirstSelected() )
    {
        pCurrRedline = pSh->GetCurrRedline();
        if( !pCurrRedline )
        {
            pSh->SwCrsrShell::Push();
            if( 0 == (pCurrRedline = pSh->SelNextRedline()))
                pCurrRedline = pSh->SelPrevRedline();
            pSh->SwCrsrShell::Pop( FALSE );
        }
    }
    else
        pCurrRedline = 0;

    for (USHORT i = nStart; i <= nEnd; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);
        const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();

        pRedlineParent = new SwRedlineDataParent;
        pRedlineParent->pData    = pRedlineData;
        pRedlineParent->pNext    = 0;
        String sComment(rRedln.GetComment());
        sComment.SearchAndReplaceAll((sal_Unicode)_LF,(sal_Unicode)' ');
        pRedlineParent->sComment = sComment;
        aRedlineParents.Insert(pRedlineParent, i);

        pData = new RedlinData;
        pData->pData = pRedlineParent;
        pData->bDisabled = FALSE;

        sParent = GetRedlineText(rRedln, pData->aDateTime);
        pParent = pTable->InsertEntry(sParent, pData, 0, i);
        if( pCurrRedline == &rRedln )
        {
            pTable->SetCurEntry( pParent );
            pTable->Select( pParent );
            pTable->MakeVisible( pParent );
        }

        pRedlineParent->pTLBParent = pParent;

        InsertChilds(pRedlineParent, rRedln, nAutoFmt);
    }
}

void SwRedlineAcceptDlg::CallAcceptReject( BOOL bSelect, BOOL bAccept )
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    SvLBoxEntry* pEntry = bSelect ? pTable->FirstSelected() : pTable->First();
    ULONG nPos = LONG_MAX;

    typedef std::vector<SvLBoxEntry*> ListBoxEntries_t;
    ListBoxEntries_t aRedlines;

    // don't activate
    DBG_ASSERT( bInhibitActivate == false,
                "recursive call of CallAcceptReject?");
    bInhibitActivate = true;

    // collect redlines-to-be-accepted/rejected in aRedlines vector
    while( pEntry )
    {
        if( !pTable->GetParent( pEntry ) )
        {
            if( bSelect && LONG_MAX == nPos )
                nPos = pTable->GetModel()->GetAbsPos( pEntry );

            RedlinData *pData = (RedlinData *)pEntry->GetUserData();

            if( !pData->bDisabled )
                aRedlines.push_back( pEntry );
        }

        pEntry = bSelect ? pTable->NextSelected(pEntry) : pTable->Next(pEntry);
    }

    BOOL (SwEditShell:: *FnAccRej)( USHORT ) = &SwEditShell::AcceptRedline;
    if( !bAccept )
        FnAccRej = &SwEditShell::RejectRedline;

    SwWait aWait( *pSh->GetView().GetDocShell(), TRUE );
    pSh->StartAction();

    // #111827#
    if (aRedlines.size() > 1)
    {
        String aTmpStr;
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UNDO_ARG1,
                              String::CreateFromInt32(aRedlines.size()));
            aTmpStr = aRewriter.Apply(String(SW_RES(STR_N_REDLINES)));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UNDO_ARG1, aTmpStr);

        pSh->StartUndo(bAccept? UNDO_ACCEPT_REDLINE : UNDO_REJECT_REDLINE,
                       &aRewriter);
    }

    // accept/reject the the redlines in aRedlines. The absolute
    // position may change during the process (e.g. when two redlines
    // are merged in result of another one being deleted), so the
    // position must be resolved late and checked before using it.
    // (cf #102547#)
    ListBoxEntries_t::iterator aEnd = aRedlines.end();
    for( ListBoxEntries_t::iterator aIter = aRedlines.begin();
         aIter != aEnd;
         aIter++ )
    {
        USHORT nPosition = GetRedlinePos( **aIter );
        if( nPosition != USHRT_MAX )
            (pSh->*FnAccRej)( nPosition );
    }

    // #111827#
    if (aRedlines.size() > 1)
        pSh->EndUndo(bAccept? UNDO_ACCEPT_REDLINE : UNDO_REJECT_REDLINE);

    pSh->EndAction();

    bInhibitActivate = false;
    Activate();

    if( ULONG_MAX != nPos && pTable->GetEntryCount() )
    {
        if( nPos >= pTable->GetEntryCount() )
            nPos = pTable->GetEntryCount() - 1;
        pEntry = pTable->GetEntry( nPos );
        if( !pEntry && nPos-- )
            pEntry = pTable->GetEntry( nPos );
        if( pEntry )
        {
            pTable->Select( pEntry );
            pTable->MakeVisible( pEntry );
            pTable->SetCurEntry(pEntry);
        }
    }
    pTPView->EnableUndo();
}

USHORT SwRedlineAcceptDlg::GetRedlinePos( const SvLBoxEntry& rEntry ) const
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    return pSh->FindRedlineOfData( *((SwRedlineDataParent*)((RedlinData *)
                                    rEntry.GetUserData())->pData)->pData );
}

IMPL_LINK( SwRedlineAcceptDlg, AcceptHdl, void*, EMPTYARG)
{
    CallAcceptReject( TRUE, TRUE );
    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, AcceptAllHdl, void*, EMPTYARG )
{
    CallAcceptReject( FALSE, TRUE );
    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, RejectHdl, void*, EMPTYARG )
{
    CallAcceptReject( TRUE, FALSE );
    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, RejectAllHdl, void*, EMPTYARG )
{
    CallAcceptReject( FALSE, FALSE );
    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, UndoHdl, void*, EMPTYARG )
{
    SwView * pView = ::GetActiveView();
    pView->GetViewFrame()->GetDispatcher()->
                Execute(SID_UNDO, SFX_CALLMODE_SYNCHRON);
    pTPView->EnableUndo(pView->GetSlotState(SID_UNDO) != 0);

    Activate();

    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, FilterChangedHdl, void*, EMPTYARG )
{
    SvxTPFilter *pFilterTP = aTabPagesCTRL.GetFilterPage();

    if (pFilterTP->IsAction())
        sFilterAction = pFilterTP->GetLbAction()->GetSelectEntry();
    else
        sFilterAction = aEmptyStr;

    Init();

    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, DeselectHdl, void*, EMPTYARG )
{
    // Flackern der Buttons vermeiden:
    aDeselectTimer.Start();

    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, SelectHdl, void*, EMPTYARG )
{
    aDeselectTimer.Stop();
    aSelectTimer.Start();

    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, GotoHdl, void*, EMPTYARG )
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    aSelectTimer.Stop();

    BOOL bIsNotFormated = FALSE;
    BOOL bSel = FALSE;
//  BOOL bReadonlySel = FALSE;

    //#98883# don't select redlines while the dialog is not focussed
    //#107938# But not only ask pTable if it has the focus. To move
    //         the selection to the selected redline any child of pParentDlg
    //         may the focus.
    SvLBoxEntry* pSelEntry = 0;

    if (pParentDlg->HasChildPathFocus())
        pSelEntry = pTable->FirstSelected();

    if( pSelEntry )
    {
        SvLBoxEntry* pActEntry = pSelEntry;
        pSh->StartAction();
        pSh->EnterStdMode();
        pSh->SetCareWin(pParentDlg);

        while (pSelEntry)
        {
            if (pTable->GetParent(pSelEntry))
            {
                pActEntry = pTable->GetParent(pSelEntry);

                if (pTable->IsSelected(pActEntry))
                {
                    pSelEntry = pActEntry = pTable->NextSelected(pSelEntry);
                    continue;   // Nicht zweimal selektieren
                }
            }
            else
                bSel = TRUE;

            // #98864# find the selected redline (ignore, if the redline is already gone)
            USHORT nPos = GetRedlinePos(*pActEntry);
            if( nPos != USHRT_MAX )
            {

                const SwRedline& rRedln = pSh->GetRedline( nPos );
                bIsNotFormated |= nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType();

//JP 27.9.2001: make no sense if we handle readonly sections
//          if( !bReadonlySel && rRedln.HasReadonlySel() )
//              bReadonlySel = TRUE;

                if (pSh->GotoRedline(nPos, TRUE))
                {
                    pSh->SetInSelect();
                    pSh->EnterAddMode();
                }
            }

            pSelEntry = pActEntry = pTable->NextSelected(pSelEntry);
        }

        pSh->LeaveAddMode();
        pSh->EndAction();
        pSh->SetCareWin(NULL);
    }
    BOOL bEnable = !pSh->getIDocumentRedlineAccess()->GetRedlinePassword().getLength();
    pTPView->EnableAccept( bEnable && bSel /*&& !bReadonlySel*/ );
    pTPView->EnableReject( bEnable && bSel && bIsNotFormated /*&& !bReadonlySel*/ );
    pTPView->EnableRejectAll( bEnable && !bOnlyFormatedRedlines && !bHasReadonlySel );

    return 0;
}

IMPL_LINK( SwRedlineAcceptDlg, CommandHdl, void*, EMPTYARG )
{
    const CommandEvent aCEvt(pTable->GetCommandEvent());

    switch ( aCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
            SvLBoxEntry* pEntry = pTable->FirstSelected();
            const SwRedline *pRed = 0;

            if (pEntry)
            {
                SvLBoxEntry* pTopEntry = pEntry;

                if (pTable->GetParent(pEntry))
                    pTopEntry = pTable->GetParent(pEntry);

                USHORT nPos = GetRedlinePos(*pTopEntry);

                // Bei geschuetzten Bereichen kommentieren disablen
                if ((pRed = pSh->GotoRedline(nPos, TRUE)) != 0)
                {
                    if( pSh->IsCrsrPtAtEnd() )
                        pSh->SwapPam();
                    pSh->SetInSelect();
                }
            }

            aPopup.EnableItem( MN_EDIT_COMMENT, pEntry && pRed &&
                                            !pTable->GetParent(pEntry) &&
                                            !pTable->NextSelected(pEntry)
//JP 27.9.2001: make no sense if we handle readonly sections
//                                          && pRed->HasReadonlySel()
                                            );

            aPopup.EnableItem( MN_SUB_SORT, pTable->First() != 0 );
            USHORT nColumn = pTable->GetSortedCol();
            if (nColumn == 0xffff)
                nColumn = 4;

            PopupMenu *pSubMenu = aPopup.GetPopupMenu(MN_SUB_SORT);
            if (pSubMenu)
            {
                for (USHORT i = MN_SORT_ACTION; i < MN_SORT_ACTION + 5; i++)
                    pSubMenu->CheckItem(i, FALSE);

                pSubMenu->CheckItem(nColumn + MN_SORT_ACTION);
            }

            USHORT nRet = aPopup.Execute(pTable, aCEvt.GetMousePosPixel());

            switch( nRet )
            {
                case MN_EDIT_COMMENT:
                {
                    String sComment;
                    if (pEntry)
                    {
                        if (pTable->GetParent(pEntry))
                            pEntry = pTable->GetParent(pEntry);

                        USHORT nPos = GetRedlinePos(*pEntry);
                        const SwRedline &rRedline = pSh->GetRedline(nPos);


                        /* enable again once we have redline comments in the margin
                        sComment = rRedline.GetComment();
                        if ( sComment == String(::rtl::OUString::createFromAscii("")) )
                            GetActiveView()->GetDocShell()->Broadcast(SwRedlineHint(&rRedline,SWREDLINE_INSERTED));
                        const_cast<SwRedline&>(rRedline).Broadcast(SwRedlineHint(&rRedline,SWREDLINE_FOCUS));
                        */

                        sComment = rRedline.GetComment();
                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "Dialogdiet fail!");
                        ::DialogGetRanges fnGetRange = pFact->GetDialogGetRangesFunc( RID_SVXDLG_POSTIT );
                        DBG_ASSERT(fnGetRange, "Dialogdiet fail! GetRanges()");
                        SfxItemSet aSet( pSh->GetAttrPool(), fnGetRange() );

                        aSet.Put(SvxPostItTextItem(sComment.ConvertLineEnd(), SID_ATTR_POSTIT_TEXT));
                        aSet.Put(SvxPostItAuthorItem(rRedline.GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

                        aSet.Put(SvxPostItDateItem( GetAppLangDateTimeString(
                                    rRedline.GetRedlineData().GetTimeStamp() ),
                                    SID_ATTR_POSTIT_DATE ));

                        AbstractSvxPostItDialog* pDlg = pFact->CreateSvxPostItDialog( pParentDlg, aSet, FALSE );
                        DBG_ASSERT(pDlg, "Dialogdiet fail!");

                        pDlg->HideAuthor();

                        USHORT nResId = 0;
                        switch( rRedline.GetType() )
                        {
                        case nsRedlineType_t::REDLINE_INSERT:
                            nResId = STR_REDLINE_INSERTED;
                            break;
                        case nsRedlineType_t::REDLINE_DELETE:
                            nResId = STR_REDLINE_DELETED;
                            break;
                        case nsRedlineType_t::REDLINE_FORMAT:
                            nResId = STR_REDLINE_FORMATED;
                            break;
                        case nsRedlineType_t::REDLINE_TABLE:
                            nResId = STR_REDLINE_TABLECHG;
                            break;
                        default:;//prevent warning
                        }
                        String sTitle(SW_RES(STR_REDLINE_COMMENT));
                        if( nResId )
                            sTitle += SW_RESSTR( nResId );
                        pDlg->SetText(sTitle);

                        pSh->SetCareWin(pDlg->GetWindow());

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                            String sMsg(((const SvxPostItTextItem&)pOutSet->Get(SID_ATTR_POSTIT_TEXT)).GetValue());

                            // Kommentar einfuegen bzw aendern
                            pSh->SetRedlineComment(sMsg);
                            sMsg.SearchAndReplaceAll((sal_Unicode)_LF,(sal_Unicode)' ');
                            pTable->SetEntryText(sMsg, pEntry, 3);
                        }

                        delete pDlg;
                        pSh->SetCareWin(NULL);
                    }

                }
                break;

            case MN_SORT_ACTION:
            case MN_SORT_AUTHOR:
            case MN_SORT_DATE:
            case MN_SORT_COMMENT:
            case MN_SORT_POSITION:
                {
                    bSortDir = TRUE;
                    if (nRet - MN_SORT_ACTION == 4 && pTable->GetSortedCol() == 0xffff)
                        break;  // Haben wir schon

                    nSortMode = nRet - MN_SORT_ACTION;
                    if (nSortMode == 4)
                        nSortMode = 0xffff; // unsortiert bzw sortiert nach Position

                    if (pTable->GetSortedCol() == nSortMode)
                        bSortDir = !pTable->GetSortDirection();

                    SwWait aWait( *::GetActiveView()->GetDocShell(), FALSE );
                    pTable->SortByCol(nSortMode, bSortDir);
                    if (nSortMode == 0xffff)
                        Init();             // Alles neu fuellen
                }
                break;
            }
        }
        break;
    }

    return 0;
}

void SwRedlineAcceptDlg::Initialize(const String& rExtraData)
{
    if (rExtraData.Len())
    {
        USHORT nPos = rExtraData.Search(C2S("AcceptChgDat:"));

        // Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
        // er nicht vorhanden ist, liegt eine "altere Version vor
        if (nPos != STRING_NOTFOUND)
        {
            USHORT n1 = rExtraData.Search('(', nPos);
            if (n1 != STRING_NOTFOUND)
            {
                USHORT n2 = rExtraData.Search(')', n1);
                if (n2 != STRING_NOTFOUND)
                {
                    // Alignment-String herausschneiden
                    String aStr = rExtraData.Copy(nPos, n2 - nPos + 1);
                    aStr.Erase(0, n1 - nPos + 1);

                    if (aStr.Len())
                    {
                        USHORT nCount = static_cast< USHORT >(aStr.ToInt32());

                        for (USHORT i = 0; i < nCount; i++)
                        {
                            USHORT n3 = aStr.Search(';');
                            aStr.Erase(0, n3 + 1);
                            pTable->SetTab(i, aStr.ToInt32(), MAP_PIXEL);
                        }
                    }
                }
            }
        }
    }
}

void SwRedlineAcceptDlg::FillInfo(String &rExtraData) const
{
    rExtraData.AppendAscii("AcceptChgDat:(");

    USHORT  nCount = pTable->TabCount();

    rExtraData += String::CreateFromInt32(nCount);
    rExtraData += ';';
    for(USHORT i = 0; i < nCount; i++)
    {
        rExtraData += String::CreateFromInt32( pTable->GetTab(i) );
        rExtraData += ';';
    }
    rExtraData += ')';
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
