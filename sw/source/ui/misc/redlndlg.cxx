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

#include <redline.hxx>
#include <tools/datetime.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <redlndlg.hxx>
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
#include <SwRewriter.hxx>
// <- #111827#

#include <vector>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <unomid.h>

#include <docsh.hxx>

#include <IDocumentRedlineAccess.hxx>

SFX_IMPL_MODELESSDIALOG_WITHID( SwRedlineAcceptChild, FN_REDLINE_ACCEPT )

static sal_uInt16 nSortMode = 0xffff;
static sal_Bool   bSortDir = sal_True;

SwRedlineAcceptChild::SwRedlineAcceptChild( Window* _pParent,
                                            sal_uInt16 nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
    SwChildWinWrapper( _pParent, nId )
{
    pWindow = new SwModelessRedlineAcceptDlg( pBindings, this, _pParent);

    ((SwModelessRedlineAcceptDlg *)pWindow)->Initialize(pInfo);
}

/*--------------------------------------------------------------------
    Description: newly initialise dialog after document switch
 --------------------------------------------------------------------*/
sal_Bool SwRedlineAcceptChild::ReInitDlg(SwDocShell *pDocSh)
{
    sal_Bool bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == sal_True)  // update immediately, doc switch!
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

    if (!pView) // can happen when switching to another app, when a Listbox in dialog
        return; // had the focus previously (actually THs Bug)

    SwDocShell *pDocSh = pView->GetDocShell();

    if (pChildWin->GetOldDocShell() != pDocSh)
    {   // doc-switch
        SwWait aWait( *pDocSh, sal_False );
        SwWrtShell* pSh = pView->GetWrtShellPtr();

        pChildWin->SetOldDocShell(pDocSh);  // avoid recursion (using modified-Hdl)

        sal_Bool bMod = pSh->IsModified();
        SfxBoolItem aShow(FN_REDLINE_SHOW, sal_True);
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

SwRedlineAcceptDlg::SwRedlineAcceptDlg(Dialog *pParent, sal_Bool bAutoFmt) :
    pParentDlg      (pParent),
    aTabPagesCTRL   (pParent, SW_RES(CTRL_TABPAGES)),
    aPopup          (SW_RES(MN_REDLINE_POPUP)),
    sInserted       (SW_RES(STR_REDLINE_INSERTED)),
    sDeleted        (SW_RES(STR_REDLINE_DELETED)),
    sFormated       (SW_RES(STR_REDLINE_FORMATED)),
    sTableChgd      (SW_RES(STR_REDLINE_TABLECHG)),
    sFmtCollSet     (SW_RES(STR_REDLINE_FMTCOLLSET)),
    sAutoFormat     (SW_RES(STR_REDLINE_AUTOFMT)),
    bOnlyFormatedRedlines( sal_False ),
    bHasReadonlySel ( sal_False ),
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
        pTPView->ShowUndo(sal_True);
        pTPView->DisableUndo();     // no UNDO events yet
    }

    pActLB->SelectEntryPos(0);

    pTable->SetStyle(pTable->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    pTable->SetNodeDefaultImages();
    pTable->SetSelectionMode(MULTIPLE_SELECTION);
    pTable->SetHighlightRange(1);

    static long aStaticTabs[]=
    {
        4,10,70,120,170
    };

    pTable->SetTabs(aStaticTabs);

    // set minimum size
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

    // avoid flickering of buttons:
    aDeselectTimer.SetTimeout(100);
    aDeselectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));

    // avoid multiple selection of the same texts:
    aSelectTimer.SetTimeout(100);
    aSelectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, GotoHdl));
}

SwRedlineAcceptDlg::~SwRedlineAcceptDlg()
{
}

void SwRedlineAcceptDlg::Init(sal_uInt16 nStart)
{
    SwWait aWait( *::GetActiveView()->GetDocShell(), sal_False );
    pTable->SetUpdateMode(sal_False);
    aUsedSeqNo.clear();

    if (nStart)
        RemoveParents(nStart, aRedlineParents.size() - 1);
    else
    {
        pTable->Clear();
        aRedlineChildren.clear();
        aRedlineParents.erase(aRedlineParents.begin() + nStart, aRedlineParents.end());
    }

    // insert parents
    InsertParents(nStart);
    InitAuthors();

    pTable->SetUpdateMode(sal_True);
    // #i69618# this moves the list box to the right position, visually
    SvTreeListEntry* pSelEntry = pTable->FirstSelected();
    if( pSelEntry )
        pTable->MakeVisible( pSelEntry, sal_True ); //#i70937#, force the scroll
}

void SwRedlineAcceptDlg::InitAuthors()
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();

    SvxTPFilter *pFilterPage = aTabPagesCTRL.GetFilterPage();

    String sAuthor;
    std::vector<String> aStrings;
    String sOldAuthor(pFilterPage->GetSelectedAuthor());
    pFilterPage->ClearAuthors();

    String sParent;
    sal_uInt16 nCount = pSh->GetRedlineCount();

    bOnlyFormatedRedlines = sal_True;
    bHasReadonlySel = sal_False;
    sal_Bool bIsNotFormated = sal_False;
    sal_uInt16 i;

    // determine authors
    for ( i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);

        if( bOnlyFormatedRedlines && nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType() )
            bOnlyFormatedRedlines = sal_False;

        aStrings.push_back(rRedln.GetAuthorString());

        for (sal_uInt16 nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
        {
            aStrings.push_back(rRedln.GetAuthorString(nStack));
        }
    }

    std::sort(aStrings.begin(), aStrings.end());
    aStrings.erase(std::unique(aStrings.begin(), aStrings.end()), aStrings.end());

    for (i = 0; i < aStrings.size(); i++)
        pFilterPage->InsertAuthor(aStrings[i]);

    if (pFilterPage->SelectAuthor(sOldAuthor) == LISTBOX_ENTRY_NOTFOUND && !aStrings.empty())
        pFilterPage->SelectAuthor(aStrings[0]);

    sal_Bool bEnable = pTable->GetEntryCount() != 0 && !pSh->getIDocumentRedlineAccess()->GetRedlinePassword().getLength();
    sal_Bool bSel = pTable->FirstSelected() != 0;

    SvTreeListEntry* pSelEntry = pTable->FirstSelected();
    while (pSelEntry)
    {
        sal_uInt16 nPos = GetRedlinePos(*pSelEntry);
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
                                        DateTime &rDateTime, sal_uInt16 nStack)
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

const String &SwRedlineAcceptDlg::GetActionText(const SwRedline& rRedln, sal_uInt16 nStack)
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
    Description: newly initialise after activation
 --------------------------------------------------------------------*/
void SwRedlineAcceptDlg::Activate()
{
    // prevent update if flag is set (#102547#)
    if( bInhibitActivate )
        return;

    SwView *pView = ::GetActiveView();

    if (!pView) // can happen when switching to another app, when a Listbox in the dialog
        return; // had the focus previously (actually THs Bug)

    SwWait aWait( *pView->GetDocShell(), sal_False );

    aUsedSeqNo.clear();

    // did something change?
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    sal_uInt16 nCount = pSh->GetRedlineCount();

    // check the number of pointers
    SwRedlineDataParent *pParent = 0;
    sal_uInt16 i;

    for ( i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);

        if (i >= aRedlineParents.size())
        {
            // new entries have been appended
            Init(i);
            return;
        }

        pParent = &aRedlineParents[i];
        if (&rRedln.GetRedlineData() != pParent->pData)
        {
            // Redline-Parents were inserted, changed or deleted
            if ((i = CalcDiff(i, sal_False)) == USHRT_MAX)
                return;
            continue;
        }

        const SwRedlineData *pRedlineData = rRedln.GetRedlineData().Next();
        const SwRedlineDataChild *pBackupData = pParent->pNext;

        if (!pRedlineData && pBackupData)
        {
            // Redline-Children were deleted
            if ((i = CalcDiff(i, sal_True)) == USHRT_MAX)
                return;
            continue;
        }
        else
        {
            while (pRedlineData)
            {
                if (pRedlineData != pBackupData->pChild)
                {
                    // Redline-Children were inserted, changed or deleted
                    if ((i = CalcDiff(i, sal_True)) == USHRT_MAX)
                        return;
                    continue;
                }
                if (pBackupData)
                    pBackupData = pBackupData->pNext;
                pRedlineData = pRedlineData->Next();
            }
        }
    }

    if (nCount != aRedlineParents.size())
    {
        // Redlines were deleted at the end
        Init(nCount);
        return;
    }

    // check comment
    for (i = 0; i < nCount; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);
        pParent = &aRedlineParents[i];

        if(!rRedln.GetComment().Equals(pParent->sComment))
        {
            if (pParent->pTLBParent)
            {
                // update only comment
                String sComment(rRedln.GetComment());
                sComment.SearchAndReplaceAll((sal_Unicode)_LF,(sal_Unicode)' ');
                pTable->SetEntryText(sComment, pParent->pTLBParent, 3);
            }
            pParent->sComment = rRedln.GetComment();
        }
    }

    InitAuthors();
}

sal_uInt16 SwRedlineAcceptDlg::CalcDiff(sal_uInt16 nStart, sal_Bool bChild)
{
    if (!nStart)
    {
        Init();
        return USHRT_MAX;
    }

    pTable->SetUpdateMode(sal_False);
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    sal_uInt16 nAutoFmt = HasRedlineAutoFmt() ? nsRedlineType_t::REDLINE_FORM_AUTOFMT : 0;
    SwRedlineDataParent *pParent = &aRedlineParents[nStart];
    const SwRedline& rRedln = pSh->GetRedline(nStart);

    if (bChild)     // should actually never happen, but just in case...
    {
        // throw away all entry's children and initialise newly
        SwRedlineDataChild* pBackupData = (SwRedlineDataChild*)pParent->pNext;
        SwRedlineDataChild* pNext;

        while (pBackupData)
        {
            pNext = (SwRedlineDataChild*)pBackupData->pNext;
            if (pBackupData->pTLBChild)
                pTable->RemoveEntry(pBackupData->pTLBChild);

            for( SwRedlineDataChildArr::iterator it = aRedlineChildren.begin();
                 it != aRedlineChildren.end(); ++it)
                if (&*it == pBackupData)
                {
                    aRedlineChildren.erase(it);
                    break;
                }
            pBackupData = pNext;
        }
        pParent->pNext = 0;

        // insert new children
        InsertChildren(pParent, rRedln, nAutoFmt);

        pTable->SetUpdateMode(sal_True);
        return nStart;
    }

    // have entries been deleted?
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    sal_uInt16 i;
    for ( i = nStart + 1; i < aRedlineParents.size(); i++)
    {
        if (aRedlineParents[i].pData == pRedlineData)
        {
            // remove entries from nStart to i-1
            RemoveParents(nStart, i - 1);
            pTable->SetUpdateMode(sal_True);
            return nStart - 1;
        }
    }

    // entries been inserted?
    sal_uInt16 nCount = pSh->GetRedlineCount();
    pRedlineData = aRedlineParents[nStart].pData;

    for (i = nStart + 1; i < nCount; i++)
    {
        if (&pSh->GetRedline(i).GetRedlineData() == pRedlineData)
        {
            // insert entries from nStart to i-1
            InsertParents(nStart, i - 1);
            pTable->SetUpdateMode(sal_True);
            return nStart - 1;
        }
    }

    pTable->SetUpdateMode(sal_True);
    Init(nStart);   // adjust all entries until the end
    return USHRT_MAX;
}

void SwRedlineAcceptDlg::InsertChildren(SwRedlineDataParent *pParent, const SwRedline& rRedln, const sal_uInt16 nAutoFmt)
{
    String sChild;
    SwRedlineDataChild *pLastRedlineChild = 0;
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    sal_Bool bAutoFmt = (rRedln.GetRealType() & nAutoFmt) != 0;

    const String *pAction = &GetActionText(rRedln);
    sal_Bool bValidParent = !sFilterAction.Len() || sFilterAction == *pAction;
    bValidParent = bValidParent && pTable->IsValidEntry(&rRedln.GetAuthorString(), &rRedln.GetTimeStamp(), &rRedln.GetComment());
    if (nAutoFmt)
    {

        if (pParent->pData->GetSeqNo())
        {
            std::pair<SwRedlineDataParentSortArr::const_iterator,bool> const ret
                = aUsedSeqNo.insert(pParent);
            if (ret.second) // already there
            {
                if (pParent->pTLBParent)
                {
                    pTable->SetEntryText(
                            sAutoFormat, (*ret.first)->pTLBParent, 0);
                    pTable->RemoveEntry(pParent->pTLBParent);
                    pParent->pTLBParent = 0;
                }
                return;
            }
        }
        bValidParent = bValidParent && bAutoFmt;
    }
    sal_Bool bValidTree = bValidParent;

    for (sal_uInt16 nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
    {
        pRedlineData = pRedlineData->Next();

        SwRedlineDataChildPtr pRedlineChild = new SwRedlineDataChild;
        pRedlineChild->pChild = pRedlineData;
        aRedlineChildren.push_back(pRedlineChild);

        if ( pLastRedlineChild )
            pLastRedlineChild->pNext = pRedlineChild;
        else
            pParent->pNext = pRedlineChild;

        pAction = &GetActionText(rRedln, nStack);
        sal_Bool bValidChild = !sFilterAction.Len() || sFilterAction == *pAction;
        bValidChild = bValidChild && pTable->IsValidEntry(&rRedln.GetAuthorString(nStack), &rRedln.GetTimeStamp(nStack), &rRedln.GetComment());
        if (nAutoFmt)
            bValidChild = bValidChild && bAutoFmt;
        bValidTree |= bValidChild;

        if (bValidChild)
        {
            RedlinData *pData = new RedlinData;
            pData->pData = pRedlineChild;
            pData->bDisabled = sal_True;
            sChild = GetRedlineText(rRedln, pData->aDateTime, nStack);

            SvTreeListEntry* pChild = pTable->InsertEntry(sChild, pData, pParent->pTLBParent);

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
            aUsedSeqNo.erase(pParent);
    }
}

void SwRedlineAcceptDlg::RemoveParents(sal_uInt16 nStart, sal_uInt16 nEnd)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    sal_uInt16 nCount = pSh->GetRedlineCount();

    SvLBoxEntryArr aLBoxArr;

    // because of Bug of TLB that ALWAYS calls the SelectHandler at Remove:
    pTable->SetSelectHdl(aOldSelectHdl);
    pTable->SetDeselectHdl(aOldDeselectHdl);
    sal_Bool bChildrenRemoved = sal_False;
    pTable->SelectAll(sal_False);

    // set the cursor after the last entry because otherwise performance problem in TLB.
    // TLB would otherwise reset the cursor at every Remove (expensive)
    sal_uInt16 nPos = Min((sal_uInt16)nCount, (sal_uInt16)aRedlineParents.size());
    SvTreeListEntry *pCurEntry = NULL;
    while( ( pCurEntry == NULL ) && ( nPos > 0 ) )
    {
        --nPos;
        pCurEntry = aRedlineParents[nPos].pTLBParent;
    }

    if (pCurEntry)
        pTable->SetCurEntry(pCurEntry);

    SvTreeList* pModel = pTable->GetModel();

    for (sal_uInt16 i = nStart; i <= nEnd; i++)
    {
        if (!bChildrenRemoved && aRedlineParents[i].pNext)
        {
            SwRedlineDataChildPtr pChildPtr = (SwRedlineDataChildPtr)aRedlineParents[i].pNext;
            for( SwRedlineDataChildArr::iterator it = aRedlineChildren.begin();
                 it != aRedlineChildren.end(); ++it)
                if (&*it == pChildPtr)
                {
                    sal_uInt16 nChildren = 0;
                    while (pChildPtr)
                    {
                        pChildPtr = (SwRedlineDataChildPtr)pChildPtr->pNext;
                        nChildren++;
                    }

                    aRedlineChildren.erase(it, it + nChildren);
                    bChildrenRemoved = sal_True;
                    break;
                }
        }
        SvTreeListEntry *pEntry = aRedlineParents[i].pTLBParent;
        if (pEntry)
        {
            long nIdx = aLBoxArr.size() - 1L;
            sal_uLong nAbsPos = pModel->GetAbsPos(pEntry);
            while (nIdx >= 0 &&
                    pModel->GetAbsPos(aLBoxArr[ static_cast< sal_uInt16 >(nIdx) ]) > nAbsPos)
                nIdx--;
            aLBoxArr.insert( aLBoxArr.begin() + static_cast< sal_uInt16 >(++nIdx) , pEntry);
        }
    }

    // clear TLB from behind
    long nIdx = (long)aLBoxArr.size() - 1L;
    while (nIdx >= 0)
        pTable->RemoveEntry(aLBoxArr[ static_cast< sal_uInt16 >(nIdx--) ]);

    pTable->SetSelectHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    pTable->SetDeselectHdl(LINK(this, SwRedlineAcceptDlg, DeselectHdl));
    // unfortunately by Remove it was selected from the TLB always again ...
    pTable->SelectAll(sal_False);

    aRedlineParents.erase( aRedlineParents.begin() + nStart, aRedlineParents.begin() + nEnd + 1);
}

void SwRedlineAcceptDlg::InsertParents(sal_uInt16 nStart, sal_uInt16 nEnd)
{
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    sal_uInt16 nAutoFmt = HasRedlineAutoFmt() ? nsRedlineType_t::REDLINE_FORM_AUTOFMT : 0;

    String sParent;
    sal_uInt16 nCount = pSh->GetRedlineCount();
    nEnd = Min((sal_uInt16)nEnd, (sal_uInt16)(nCount - 1)); // also treats nEnd=USHRT_MAX (until the end)

    if (nEnd == USHRT_MAX)
        return;     // no redlines in the document

    RedlinData *pData;
    SvTreeListEntry *pParent;
    SwRedlineDataParent* pRedlineParent;
    const SwRedline* pCurrRedline;
    if( !nStart && !pTable->FirstSelected() )
    {
        pCurrRedline = pSh->GetCurrRedline();
        if( !pCurrRedline )
        {
            pSh->SwCrsrShell::Push();
            if( 0 == (pCurrRedline = pSh->SelNextRedline()))
                pCurrRedline = pSh->SelPrevRedline();
            pSh->SwCrsrShell::Pop( sal_False );
        }
    }
    else
        pCurrRedline = 0;

    for (sal_uInt16 i = nStart; i <= nEnd; i++)
    {
        const SwRedline& rRedln = pSh->GetRedline(i);
        const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();

        pRedlineParent = new SwRedlineDataParent;
        pRedlineParent->pData    = pRedlineData;
        pRedlineParent->pNext    = 0;
        String sComment(rRedln.GetComment());
        sComment.SearchAndReplaceAll((sal_Unicode)_LF,(sal_Unicode)' ');
        pRedlineParent->sComment = sComment;
        aRedlineParents.insert(aRedlineParents.begin() + i, pRedlineParent);

        pData = new RedlinData;
        pData->pData = pRedlineParent;
        pData->bDisabled = sal_False;

        sParent = GetRedlineText(rRedln, pData->aDateTime);
        pParent = pTable->InsertEntry(sParent, pData, 0, i);
        if( pCurrRedline == &rRedln )
        {
            pTable->SetCurEntry( pParent );
            pTable->Select( pParent );
            pTable->MakeVisible( pParent );
        }

        pRedlineParent->pTLBParent = pParent;

        InsertChildren(pRedlineParent, rRedln, nAutoFmt);
    }
}

void SwRedlineAcceptDlg::CallAcceptReject( sal_Bool bSelect, sal_Bool bAccept )
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    SvTreeListEntry* pEntry = bSelect ? pTable->FirstSelected() : pTable->First();
    sal_uLong nPos = LONG_MAX;

    typedef std::vector<SvTreeListEntry*> ListBoxEntries_t;
    ListBoxEntries_t aRedlines;

    // don't activate
    OSL_ENSURE( bInhibitActivate == false,
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

    sal_Bool (SwEditShell:: *FnAccRej)( sal_uInt16 ) = &SwEditShell::AcceptRedline;
    if( !bAccept )
        FnAccRej = &SwEditShell::RejectRedline;

    SwWait aWait( *pSh->GetView().GetDocShell(), sal_True );
    pSh->StartAction();

    // #111827#
    if (aRedlines.size() > 1)
    {
        String aTmpStr;
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1,
                              String::CreateFromInt32(aRedlines.size()));
            aTmpStr = aRewriter.Apply(String(SW_RES(STR_N_REDLINES)));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

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
         ++aIter )
    {
        sal_uInt16 nPosition = GetRedlinePos( **aIter );
        if( nPosition != USHRT_MAX )
            (pSh->*FnAccRej)( nPosition );
    }

    // #111827#
    if (aRedlines.size() > 1)
    {
        pSh->EndUndo();
    }

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

sal_uInt16 SwRedlineAcceptDlg::GetRedlinePos( const SvTreeListEntry& rEntry ) const
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    return pSh->FindRedlineOfData( *((SwRedlineDataParent*)((RedlinData *)
                                    rEntry.GetUserData())->pData)->pData );
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, AcceptHdl)
{
    CallAcceptReject( sal_True, sal_True );
    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, AcceptAllHdl)
{
    CallAcceptReject( sal_False, sal_True );
    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, RejectHdl)
{
    CallAcceptReject( sal_True, sal_False );
    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, RejectAllHdl)
{
    CallAcceptReject( sal_False, sal_False );
    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, UndoHdl)
{
    SwView * pView = ::GetActiveView();
    pView->GetViewFrame()->GetDispatcher()->
                Execute(SID_UNDO, SFX_CALLMODE_SYNCHRON);
    pTPView->EnableUndo(pView->GetSlotState(SID_UNDO) != 0);

    Activate();

    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, FilterChangedHdl)
{
    SvxTPFilter *pFilterTP = aTabPagesCTRL.GetFilterPage();

    if (pFilterTP->IsAction())
        sFilterAction = pFilterTP->GetLbAction()->GetSelectEntry();
    else
        sFilterAction = aEmptyStr;

    Init();

    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, DeselectHdl)
{
    // avoid flickering of buttons:
    aDeselectTimer.Start();

    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, SelectHdl)
{
    aDeselectTimer.Stop();
    aSelectTimer.Start();

    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, GotoHdl)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    aSelectTimer.Stop();

    sal_Bool bIsNotFormated = sal_False;
    sal_Bool bSel = sal_False;

    //#98883# don't select redlines while the dialog is not focussed
    //#107938# But not only ask pTable if it has the focus. To move
    //         the selection to the selected redline any child of pParentDlg
    //         may the focus.
    SvTreeListEntry* pSelEntry = 0;

    if (pParentDlg->HasChildPathFocus())
        pSelEntry = pTable->FirstSelected();

    if( pSelEntry )
    {
        SvTreeListEntry* pActEntry = pSelEntry;
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
                    continue;   // don't select twice
                }
            }
            else
                bSel = sal_True;

            // #98864# find the selected redline (ignore, if the redline is already gone)
            sal_uInt16 nPos = GetRedlinePos(*pActEntry);
            if( nPos != USHRT_MAX )
            {

                const SwRedline& rRedln = pSh->GetRedline( nPos );
                bIsNotFormated |= nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType();

                if (pSh->GotoRedline(nPos, sal_True))
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
    sal_Bool bEnable = !pSh->getIDocumentRedlineAccess()->GetRedlinePassword().getLength();
    pTPView->EnableAccept( bEnable && bSel /*&& !bReadonlySel*/ );
    pTPView->EnableReject( bEnable && bSel && bIsNotFormated /*&& !bReadonlySel*/ );
    pTPView->EnableRejectAll( bEnable && !bOnlyFormatedRedlines && !bHasReadonlySel );

    return 0;
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, CommandHdl)
{
    const CommandEvent aCEvt(pTable->GetCommandEvent());

    switch ( aCEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
            SvTreeListEntry* pEntry = pTable->FirstSelected();
            const SwRedline *pRed = 0;

            if (pEntry)
            {
                SvTreeListEntry* pTopEntry = pEntry;

                if (pTable->GetParent(pEntry))
                    pTopEntry = pTable->GetParent(pEntry);

                sal_uInt16 nPos = GetRedlinePos(*pTopEntry);

                // disable commenting for protected areas
                if ((pRed = pSh->GotoRedline(nPos, sal_True)) != 0)
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
            sal_uInt16 nColumn = pTable->GetSortedCol();
            if (nColumn == 0xffff)
                nColumn = 4;

            PopupMenu *pSubMenu = aPopup.GetPopupMenu(MN_SUB_SORT);
            if (pSubMenu)
            {
                for (sal_uInt16 i = MN_SORT_ACTION; i < MN_SORT_ACTION + 5; i++)
                    pSubMenu->CheckItem(i, sal_False);

                pSubMenu->CheckItem(nColumn + MN_SORT_ACTION);
            }

            sal_uInt16 nRet = aPopup.Execute(pTable, aCEvt.GetMousePosPixel());

            switch( nRet )
            {
                case MN_EDIT_COMMENT:
                {
                    if (pEntry)
                    {
                        if (pTable->GetParent(pEntry))
                            pEntry = pTable->GetParent(pEntry);

                        sal_uInt16 nPos = GetRedlinePos(*pEntry);
                        const SwRedline &rRedline = pSh->GetRedline(nPos);


                        /* enable again once we have redline comments in the margin
                        sComment = rRedline.GetComment();
                        if ( !sComment.Len() )
                            GetActiveView()->GetDocShell()->Broadcast(SwRedlineHint(&rRedline,SWREDLINE_INSERTED));
                        const_cast<SwRedline&>(rRedline).Broadcast(SwRedlineHint(&rRedline,SWREDLINE_FOCUS));
                        */

                        rtl::OUString sComment = convertLineEnd(rRedline.GetComment(), GetSystemLineEnd());
                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "Dialogdiet fail!");
                        ::DialogGetRanges fnGetRange = pFact->GetDialogGetRangesFunc( RID_SVXDLG_POSTIT );
                        OSL_ENSURE(fnGetRange, "Dialogdiet fail! GetRanges()");
                        SfxItemSet aSet( pSh->GetAttrPool(), fnGetRange() );

                        aSet.Put(SvxPostItTextItem(sComment, SID_ATTR_POSTIT_TEXT));
                        aSet.Put(SvxPostItAuthorItem(rRedline.GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

                        aSet.Put(SvxPostItDateItem( GetAppLangDateTimeString(
                                    rRedline.GetRedlineData().GetTimeStamp() ),
                                    SID_ATTR_POSTIT_DATE ));

                        AbstractSvxPostItDialog* pDlg = pFact->CreateSvxPostItDialog( pParentDlg, aSet, sal_False );
                        OSL_ENSURE(pDlg, "Dialogdiet fail!");

                        pDlg->HideAuthor();

                        sal_uInt16 nResId = 0;
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

                            // insert / change comment
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
                    bSortDir = sal_True;
                    if (nRet - MN_SORT_ACTION == 4 && pTable->GetSortedCol() == 0xffff)
                        break;  // we already have it

                    nSortMode = nRet - MN_SORT_ACTION;
                    if (nSortMode == 4)
                        nSortMode = 0xffff; // unsorted / sorted by position

                    if (pTable->GetSortedCol() == nSortMode)
                        bSortDir = !pTable->GetSortDirection();

                    SwWait aWait( *::GetActiveView()->GetDocShell(), sal_False );
                    pTable->SortByCol(nSortMode, bSortDir);
                    if (nSortMode == 0xffff)
                        Init();             // newly fill everything
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
        sal_uInt16 nPos = rExtraData.Search(rtl::OUString("AcceptChgDat:"));

        // try to read the alignment string "ALIGN:(...)"; if none existing,
        // it's an old version
        if (nPos != STRING_NOTFOUND)
        {
            sal_uInt16 n1 = rExtraData.Search('(', nPos);
            if (n1 != STRING_NOTFOUND)
            {
                sal_uInt16 n2 = rExtraData.Search(')', n1);
                if (n2 != STRING_NOTFOUND)
                {
                    // cut out the alignment string
                    String aStr = rExtraData.Copy(nPos, n2 - nPos + 1);
                    aStr.Erase(0, n1 - nPos + 1);

                    if (aStr.Len())
                    {
                        sal_uInt16 nCount = static_cast< sal_uInt16 >(aStr.ToInt32());

                        for (sal_uInt16 i = 0; i < nCount; i++)
                        {
                            sal_uInt16 n3 = aStr.Search(';');
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

    sal_uInt16  nCount = pTable->TabCount();

    rExtraData += String::CreateFromInt32(nCount);
    rExtraData += ';';
    for(sal_uInt16 i = 0; i < nCount; i++)
    {
        rExtraData += String::CreateFromInt32( pTable->GetTab(i) );
        rExtraData += ';';
    }
    rExtraData += ')';
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
