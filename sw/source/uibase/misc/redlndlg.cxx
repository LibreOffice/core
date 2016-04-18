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
#include <memory>

SFX_IMPL_MODELESSDIALOG_WITHID( SwRedlineAcceptChild, FN_REDLINE_ACCEPT )

static sal_uInt16 nSortMode = 0xffff;
static bool       bSortDir = true;

SwRedlineAcceptChild::SwRedlineAcceptChild( vcl::Window* _pParent,
                                            sal_uInt16 nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
    SwChildWinWrapper( _pParent, nId )
{
    SetWindow( VclPtr<SwModelessRedlineAcceptDlg>::Create( pBindings, this, _pParent) );

    static_cast<SwModelessRedlineAcceptDlg *>(GetWindow())->Initialize(pInfo);
}

// newly initialise dialog after document switch
bool SwRedlineAcceptChild::ReInitDlg(SwDocShell *pDocSh)
{
    bool bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)))  // update immediately, doc switch!
        static_cast<SwModelessRedlineAcceptDlg*>(GetWindow())->Activate();

    return bRet;
}

SwModelessRedlineAcceptDlg::SwModelessRedlineAcceptDlg(
    SfxBindings* _pBindings, SwChildWinWrapper* pChild, vcl::Window *_pParent)
    : SfxModelessDialog(_pBindings, pChild, _pParent,
        "AcceptRejectChangesDialog", "svx/ui/acceptrejectchangesdialog.ui")
    , pChildWin       (pChild)
{
    pImplDlg = new SwRedlineAcceptDlg(this, this, get_content_area());
}

void SwModelessRedlineAcceptDlg::Activate()
{
    SwView *pView = ::GetActiveView();
    if (!pView) // can happen when switching to another app, when a Listbox in dialog
        return; // had the focus previously (actually THs Bug)

    SwDocShell *pDocSh = pView->GetDocShell();

    if (pChildWin->GetOldDocShell() != pDocSh)
    {   // doc-switch
        SwWait aWait( *pDocSh, false );
        SwWrtShell* pSh = pView->GetWrtShellPtr();

        pChildWin->SetOldDocShell(pDocSh);  // avoid recursion (using modified-Hdl)

        bool bMod = pSh->IsModified();
        SfxBoolItem aShow(FN_REDLINE_SHOW, true);
        pSh->GetView().GetViewFrame()->GetDispatcher()->ExecuteList(
            FN_REDLINE_SHOW, SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
            { &aShow });
        if (!bMod)
            pSh->ResetModified();
        pImplDlg->Init();

        return;
    }

    pImplDlg->Activate();
}

void SwModelessRedlineAcceptDlg::Initialize(SfxChildWinInfo *pInfo)
{
    if (pInfo != nullptr)
        pImplDlg->Initialize(pInfo->aExtraString);

    SfxModelessDialog::Initialize(pInfo);
}

void SwModelessRedlineAcceptDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxModelessDialog::FillInfo(rInfo);
    pImplDlg->FillInfo(rInfo.aExtraString);
}

SwModelessRedlineAcceptDlg::~SwModelessRedlineAcceptDlg()
{
    disposeOnce();
}

void SwModelessRedlineAcceptDlg::dispose()
{
    delete pImplDlg;
    SfxModelessDialog::dispose();
}

SwRedlineAcceptDlg::SwRedlineAcceptDlg(vcl::Window *pParent, VclBuilderContainer *pBuilder,
                                       vcl::Window *pContentArea, bool bAutoFormat) :
    m_pParentDlg      (pParent),
    m_aTabPagesCTRL   (VclPtr<SvxAcceptChgCtr>::Create(pContentArea, pBuilder)),
    m_aPopup          (SW_RES(MN_REDLINE_POPUP)),
    m_sInserted       (SW_RES(STR_REDLINE_INSERTED)),
    m_sDeleted        (SW_RES(STR_REDLINE_DELETED)),
    m_sFormated       (SW_RES(STR_REDLINE_FORMATED)),
    m_sTableChgd      (SW_RES(STR_REDLINE_TABLECHG)),
    m_sFormatCollSet     (SW_RES(STR_REDLINE_FMTCOLLSET)),
    m_sAutoFormat     (SW_RES(STR_REDLINE_AUTOFMT)),
    m_bOnlyFormatedRedlines( false ),
    m_bHasReadonlySel ( false ),
    m_bRedlnAutoFormat   (bAutoFormat),
    m_bInhibitActivate( false ),
    m_aInserted       (SW_RES(IMG_REDLINE_INSERTED)),
    m_aDeleted        (SW_RES(IMG_REDLINE_DELETED)),
    m_aFormated       (SW_RES(IMG_REDLINE_FORMATED)),
    m_aTableChgd      (SW_RES(IMG_REDLINE_TABLECHG)),
    m_aFormatCollSet  (SW_RES(IMG_REDLINE_FMTCOLLSET))

{
    m_aTabPagesCTRL->SetHelpId(HID_REDLINE_CTRL);
    m_pTPView = m_aTabPagesCTRL->GetViewPage();

    m_pTable = m_pTPView->GetTableControl();

    m_pTPView->InsertWriterHeader();
    m_pTPView->SetAcceptClickHdl(LINK(this, SwRedlineAcceptDlg, AcceptHdl));
    m_pTPView->SetAcceptAllClickHdl(LINK(this, SwRedlineAcceptDlg, AcceptAllHdl));
    m_pTPView->SetRejectClickHdl(LINK(this, SwRedlineAcceptDlg, RejectHdl));
    m_pTPView->SetRejectAllClickHdl(LINK(this, SwRedlineAcceptDlg, RejectAllHdl));
    m_pTPView->SetUndoClickHdl(LINK(this, SwRedlineAcceptDlg, UndoHdl));
    //tdf#89227 default to disabled, and only enable if possible to accept/reject
    m_pTPView->EnableAccept(false);
    m_pTPView->EnableReject(false);
    m_pTPView->EnableAcceptAll(false);
    m_pTPView->EnableRejectAll(false);

    m_aTabPagesCTRL->GetFilterPage()->SetReadyHdl(LINK(this, SwRedlineAcceptDlg, FilterChangedHdl));

    ListBox *pActLB = m_aTabPagesCTRL->GetFilterPage()->GetLbAction();
    pActLB->InsertEntry(m_sInserted);
    pActLB->InsertEntry(m_sDeleted);
    pActLB->InsertEntry(m_sFormated);
    pActLB->InsertEntry(m_sTableChgd);

    if (HasRedlineAutoFormat())
    {
        pActLB->InsertEntry(m_sFormatCollSet);
        pActLB->InsertEntry(m_sAutoFormat);
        m_pTPView->ShowUndo();
        m_pTPView->DisableUndo();     // no UNDO events yet
    }

    pActLB->SelectEntryPos(0);

    m_pTable->SetStyle(m_pTable->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    m_pTable->SetNodeDefaultImages();
    m_pTable->SetSelectionMode(MULTIPLE_SELECTION);
    m_pTable->SetHighlightRange(1);

    m_pTable->SortByCol(nSortMode, bSortDir);

    m_aOldSelectHdl = m_pTable->GetSelectHdl();
    m_aOldDeselectHdl = m_pTable->GetDeselectHdl();
    m_pTable->SetSelectHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    m_pTable->SetDeselectHdl(LINK(this, SwRedlineAcceptDlg, DeselectHdl));
    m_pTable->SetCommandHdl(LINK(this, SwRedlineAcceptDlg, CommandHdl));

    // avoid flickering of buttons:
    m_aDeselectTimer.SetTimeout(100);
    m_aDeselectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, SelectTimerHdl));

    // avoid multiple selection of the same texts:
    m_aSelectTimer.SetTimeout(100);
    m_aSelectTimer.SetTimeoutHdl(LINK(this, SwRedlineAcceptDlg, GotoHdl));
}

SwRedlineAcceptDlg::~SwRedlineAcceptDlg()
{
    m_aTabPagesCTRL.disposeAndClear();
}

void SwRedlineAcceptDlg::Init(sal_uInt16 nStart)
{
    SwWait aWait( *::GetActiveView()->GetDocShell(), false );
    m_pTable->SetUpdateMode(false);
    m_aUsedSeqNo.clear();

    if (nStart)
        RemoveParents(nStart, m_RedlineParents.size() - 1);
    else
    {
        m_pTable->Clear();
        m_RedlineChildren.clear();
        m_RedlineParents.erase(m_RedlineParents.begin() + nStart, m_RedlineParents.end());
    }

    // insert parents
    InsertParents(nStart);
    InitAuthors();

    m_pTable->SetUpdateMode(true);
    // #i69618# this moves the list box to the right position, visually
    SvTreeListEntry* pSelEntry = m_pTable->FirstSelected();
    if( pSelEntry )
        m_pTable->MakeVisible( pSelEntry, true ); //#i70937#, force the scroll
}

void SwRedlineAcceptDlg::InitAuthors()
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();

    if (!m_aTabPagesCTRL)
        return;

    SvxTPFilter *pFilterPage = m_aTabPagesCTRL->GetFilterPage();

    std::vector<OUString> aStrings;
    OUString sOldAuthor(pFilterPage->GetSelectedAuthor());
    pFilterPage->ClearAuthors();

    sal_uInt16 nCount = pSh->GetRedlineCount();

    m_bOnlyFormatedRedlines = true;
    m_bHasReadonlySel = false;
    bool bIsNotFormated = false;
    sal_uInt16 i;

    // determine authors
    for ( i = 0; i < nCount; i++)
    {
        const SwRangeRedline& rRedln = pSh->GetRedline(i);

        if( m_bOnlyFormatedRedlines && nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType() )
            m_bOnlyFormatedRedlines = false;

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

    bool bEnable = m_pTable->GetEntryCount() != 0 && !pSh->getIDocumentRedlineAccess().GetRedlinePassword().getLength();
    bool bSel = m_pTable->FirstSelected() != nullptr;

    SvTreeListEntry* pSelEntry = m_pTable->FirstSelected();
    while (pSelEntry)
    {
        // find the selected redline
        // (fdo#57874: ignore, if the redline is already gone)
        sal_uInt16 nPos = GetRedlinePos(*pSelEntry);
        if( nPos != USHRT_MAX )
        {
            const SwRangeRedline& rRedln = pSh->GetRedline( nPos );

            bIsNotFormated |= nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType();
        }
        pSelEntry = m_pTable->NextSelected(pSelEntry);
    }

    m_pTPView->EnableAccept( bEnable && bSel );
    m_pTPView->EnableReject( bEnable && bIsNotFormated && bSel );
    m_pTPView->EnableAcceptAll( bEnable && !m_bHasReadonlySel );
    m_pTPView->EnableRejectAll( bEnable && !m_bHasReadonlySel &&
                                !m_bOnlyFormatedRedlines );
}

OUString SwRedlineAcceptDlg::GetRedlineText(const SwRangeRedline& rRedln, DateTime &rDateTime, sal_uInt16 nStack)
{
    OUString sEntry(rRedln.GetAuthorString(nStack));
    sEntry += "\t";

    const DateTime &rDT = rRedln.GetTimeStamp(nStack);
    rDateTime = rDT;

    sEntry += GetAppLangDateTimeString( rDT );
    sEntry += "\t";

    sEntry += rRedln.GetComment(nStack);

    return sEntry;
}

Image SwRedlineAcceptDlg::GetActionImage(const SwRangeRedline& rRedln, sal_uInt16 nStack)
{
    switch (rRedln.GetType(nStack))
    {
        case nsRedlineType_t::REDLINE_INSERT:  return m_aInserted;
        case nsRedlineType_t::REDLINE_DELETE:  return m_aDeleted;
        case nsRedlineType_t::REDLINE_FORMAT:  return m_aFormated;
        case nsRedlineType_t::REDLINE_PARAGRAPH_FORMAT: return m_aFormated;
        case nsRedlineType_t::REDLINE_TABLE:   return m_aTableChgd;
        case nsRedlineType_t::REDLINE_FMTCOLL: return m_aFormatCollSet;
    }

    return Image();
}

OUString SwRedlineAcceptDlg::GetActionText(const SwRangeRedline& rRedln, sal_uInt16 nStack)
{
    switch( rRedln.GetType(nStack) )
    {
        case nsRedlineType_t::REDLINE_INSERT:   return m_sInserted;
        case nsRedlineType_t::REDLINE_DELETE:   return m_sDeleted;
        case nsRedlineType_t::REDLINE_FORMAT:   return m_sFormated;
        case nsRedlineType_t::REDLINE_PARAGRAPH_FORMAT:   return m_sFormated;
        case nsRedlineType_t::REDLINE_TABLE:    return m_sTableChgd;
        case nsRedlineType_t::REDLINE_FMTCOLL:  return m_sFormatCollSet;
        default:;//prevent warning
    }

    return OUString();
}

// newly initialise after activation
void SwRedlineAcceptDlg::Activate()
{
    // prevent update if flag is set (#102547#)
    if( m_bInhibitActivate )
        return;

    SwView *pView = ::GetActiveView();
    if (!pView) // can happen when switching to another app
    {
        m_pTPView->EnableAccept(false);
        m_pTPView->EnableReject(false);
        m_pTPView->EnableAcceptAll(false);
        m_pTPView->EnableRejectAll(false);
        return; // had the focus previously
    }

    SwWait aWait( *pView->GetDocShell(), false );

    m_aUsedSeqNo.clear();

    // did something change?
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    sal_uInt16 nCount = pSh->GetRedlineCount();

    // check the number of pointers
    sal_uInt16 i;

    for ( i = 0; i < nCount; i++)
    {
        const SwRangeRedline& rRedln = pSh->GetRedline(i);

        if (i >= m_RedlineParents.size())
        {
            // new entries have been appended
            Init(i);
            return;
        }

        SwRedlineDataParent *const pParent = m_RedlineParents[i].get();
        if (&rRedln.GetRedlineData() != pParent->pData)
        {
            // Redline-Parents were inserted, changed or deleted
            if ((i = CalcDiff(i, false)) == USHRT_MAX)
                return;
            continue;
        }

        const SwRedlineData *pRedlineData = rRedln.GetRedlineData().Next();
        const SwRedlineDataChild *pBackupData = pParent->pNext;

        if (!pRedlineData && pBackupData)
        {
            // Redline-Children were deleted
            if ((i = CalcDiff(i, true)) == USHRT_MAX)
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
                    if ((i = CalcDiff(i, true)) == USHRT_MAX)
                        return;
                    continue;
                }
                pBackupData = pBackupData->pNext;
                pRedlineData = pRedlineData->Next();
            }
        }
    }

    if (nCount != m_RedlineParents.size())
    {
        // Redlines were deleted at the end
        Init(nCount);
        return;
    }

    // check comment
    for (i = 0; i < nCount; i++)
    {
        const SwRangeRedline& rRedln = pSh->GetRedline(i);
        SwRedlineDataParent *const pParent = m_RedlineParents[i].get();

        if(rRedln.GetComment() != pParent->sComment)
        {
            if (pParent->pTLBParent)
            {
                // update only comment
                const OUString& sComment(rRedln.GetComment());
                m_pTable->SetEntryText(sComment.replace('\n', ' '), pParent->pTLBParent, 3);
            }
            pParent->sComment = rRedln.GetComment();
        }
    }

    InitAuthors();
}

sal_uInt16 SwRedlineAcceptDlg::CalcDiff(sal_uInt16 nStart, bool bChild)
{
    if (!nStart)
    {
        Init();
        return USHRT_MAX;
    }

    m_pTable->SetUpdateMode(false);
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    sal_uInt16 nAutoFormat = HasRedlineAutoFormat() ? nsRedlineType_t::REDLINE_FORM_AUTOFMT : 0;
    SwRedlineDataParent *const pParent = m_RedlineParents[nStart].get();
    const SwRangeRedline& rRedln = pSh->GetRedline(nStart);

    if (bChild)     // should actually never happen, but just in case...
    {
        // throw away all entry's children and initialise newly
        SwRedlineDataChild* pBackupData = const_cast<SwRedlineDataChild*>(pParent->pNext);
        SwRedlineDataChild* pNext;

        while (pBackupData)
        {
            pNext = const_cast<SwRedlineDataChild*>(pBackupData->pNext);
            if (pBackupData->pTLBChild)
                m_pTable->RemoveEntry(pBackupData->pTLBChild);

            for (SwRedlineDataChildArr::iterator it = m_RedlineChildren.begin();
                 it != m_RedlineChildren.end(); ++it)
            {
                if (it->get() == pBackupData)
                {
                    m_RedlineChildren.erase(it);
                    break;
                }
            }
            pBackupData = pNext;
        }
        pParent->pNext = nullptr;

        // insert new children
        InsertChildren(pParent, rRedln, nAutoFormat);

        m_pTable->SetUpdateMode(true);
        return nStart;
    }

    // have entries been deleted?
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    for (size_t i = nStart + 1; i < m_RedlineParents.size(); i++)
    {
        if (m_RedlineParents[i]->pData == pRedlineData)
        {
            // remove entries from nStart to i-1
            RemoveParents(nStart, i - 1);
            m_pTable->SetUpdateMode(true);
            return nStart - 1;
        }
    }

    // entries been inserted?
    sal_uInt16 nCount = pSh->GetRedlineCount();
    pRedlineData = m_RedlineParents[nStart]->pData;

    for (sal_uInt16 i = nStart + 1; i < nCount; i++)
    {
        if (&pSh->GetRedline(i).GetRedlineData() == pRedlineData)
        {
            // insert entries from nStart to i-1
            InsertParents(nStart, i - 1);
            m_pTable->SetUpdateMode(true);
            return nStart - 1;
        }
    }

    m_pTable->SetUpdateMode(true);
    Init(nStart);   // adjust all entries until the end
    return USHRT_MAX;
}

void SwRedlineAcceptDlg::InsertChildren(SwRedlineDataParent *pParent, const SwRangeRedline& rRedln, const sal_uInt16 nAutoFormat)
{
    OUString sChild;
    SwRedlineDataChild *pLastRedlineChild = nullptr;
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    bool bAutoFormat = (rRedln.GetRealType() & nAutoFormat) != 0;

    OUString sAction = GetActionText(rRedln);
    bool bValidParent = m_sFilterAction.isEmpty() || m_sFilterAction == sAction;
    bValidParent = bValidParent && m_pTable->IsValidEntry(rRedln.GetAuthorString(), rRedln.GetTimeStamp(), rRedln.GetComment());
    if (nAutoFormat)
    {

        if (pParent->pData->GetSeqNo())
        {
            std::pair<SwRedlineDataParentSortArr::const_iterator,bool> const ret
                = m_aUsedSeqNo.insert(pParent);
            if (ret.second) // already there
            {
                if (pParent->pTLBParent)
                {
                    m_pTable->SetEntryText(
                            m_sAutoFormat, (*ret.first)->pTLBParent, 0);
                    m_pTable->RemoveEntry(pParent->pTLBParent);
                    pParent->pTLBParent = nullptr;
                }
                return;
            }
        }
        bValidParent = bValidParent && bAutoFormat;
    }
    bool bValidTree = bValidParent;

    for (sal_uInt16 nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
    {
        pRedlineData = pRedlineData->Next();

        SwRedlineDataChild* pRedlineChild = new SwRedlineDataChild;
        pRedlineChild->pChild = pRedlineData;
        m_RedlineChildren.push_back(std::unique_ptr<SwRedlineDataChild>(pRedlineChild));

        if ( pLastRedlineChild )
            pLastRedlineChild->pNext = pRedlineChild;
        else
            pParent->pNext = pRedlineChild;

        sAction = GetActionText(rRedln, nStack);
        bool bValidChild = m_sFilterAction.isEmpty() || m_sFilterAction == sAction;
        bValidChild = bValidChild && m_pTable->IsValidEntry(rRedln.GetAuthorString(nStack), rRedln.GetTimeStamp(nStack), rRedln.GetComment());
        if (nAutoFormat)
            bValidChild = bValidChild && bAutoFormat;
        bValidTree |= bValidChild;

        if (bValidChild)
        {
            RedlinData *pData = new RedlinData;
            pData->pData = pRedlineChild;
            pData->bDisabled = true;
            sChild = GetRedlineText(rRedln, pData->aDateTime, nStack);

            SvTreeListEntry* pChild = m_pTable->InsertEntry(GetActionImage(rRedln, nStack),
                    sChild, pData, pParent->pTLBParent);

            pRedlineChild->pTLBChild = pChild;
            if (!bValidParent)
                m_pTable->Expand(pParent->pTLBParent);
        }
        else
            pRedlineChild->pTLBChild = nullptr;

        pLastRedlineChild = pRedlineChild;
    }

    if (pLastRedlineChild)
        pLastRedlineChild->pNext = nullptr;

    if (!bValidTree && pParent->pTLBParent)
    {
        m_pTable->RemoveEntry(pParent->pTLBParent);
        pParent->pTLBParent = nullptr;
        if (nAutoFormat)
            m_aUsedSeqNo.erase(pParent);
    }
}

void SwRedlineAcceptDlg::RemoveParents(sal_uInt16 nStart, sal_uInt16 nEnd)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    sal_uInt16 nCount = pSh->GetRedlineCount();

    std::vector<SvTreeListEntry*> aLBoxArr;

    // because of Bug of TLB that ALWAYS calls the SelectHandler at Remove:
    m_pTable->SetSelectHdl(m_aOldSelectHdl);
    m_pTable->SetDeselectHdl(m_aOldDeselectHdl);
    bool bChildrenRemoved = false;
    m_pTable->SelectAll(false);

    // set the cursor after the last entry because otherwise performance problem in TLB.
    // TLB would otherwise reset the cursor at every Remove (expensive)
    sal_uInt16 nPos = std::min((sal_uInt16)nCount, (sal_uInt16)m_RedlineParents.size());
    SvTreeListEntry *pCurEntry = nullptr;
    while( ( pCurEntry == nullptr ) && ( nPos > 0 ) )
    {
        --nPos;
        pCurEntry = m_RedlineParents[nPos]->pTLBParent;
    }

    if (pCurEntry)
        m_pTable->SetCurEntry(pCurEntry);

    SvTreeList* pModel = m_pTable->GetModel();

    for (sal_uInt16 i = nStart; i <= nEnd; i++)
    {
        if (!bChildrenRemoved && m_RedlineParents[i]->pNext)
        {
            SwRedlineDataChild * pChildPtr =
                const_cast<SwRedlineDataChild*>(m_RedlineParents[i]->pNext);
            for (SwRedlineDataChildArr::iterator it = m_RedlineChildren.begin();
                 it != m_RedlineChildren.end(); ++it)
            {
                if (it->get() == pChildPtr)
                {
                    sal_uInt16 nChildren = 0;
                    while (pChildPtr)
                    {
                        pChildPtr = const_cast<SwRedlineDataChild*>(pChildPtr->pNext);
                        nChildren++;
                    }

                    m_RedlineChildren.erase(it, it + nChildren);
                    bChildrenRemoved = true;
                    break;
                }
            }
        }
        SvTreeListEntry *const pEntry = m_RedlineParents[i]->pTLBParent;
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
        m_pTable->RemoveEntry(aLBoxArr[ static_cast< sal_uInt16 >(nIdx--) ]);

    m_pTable->SetSelectHdl(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    m_pTable->SetDeselectHdl(LINK(this, SwRedlineAcceptDlg, DeselectHdl));
    // unfortunately by Remove it was selected from the TLB always again ...
    m_pTable->SelectAll(false);

    m_RedlineParents.erase(m_RedlineParents.begin() + nStart, m_RedlineParents.begin() + nEnd + 1);
}

void SwRedlineAcceptDlg::InsertParents(sal_uInt16 nStart, sal_uInt16 nEnd)
{
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    sal_uInt16 nAutoFormat = HasRedlineAutoFormat() ? nsRedlineType_t::REDLINE_FORM_AUTOFMT : 0;

    OUString sParent;
    sal_uInt16 nCount = pSh->GetRedlineCount();
    nEnd = std::min((sal_uInt16)nEnd, (sal_uInt16)(nCount - 1)); // also treats nEnd=USHRT_MAX (until the end)

    if (nEnd == USHRT_MAX)
        return;     // no redlines in the document

    SvTreeListEntry *pParent;
    SwRedlineDataParent* pRedlineParent;
    const SwRangeRedline* pCurrRedline;
    if( !nStart && !m_pTable->FirstSelected() )
    {
        pCurrRedline = pSh->GetCurrRedline();
        if( !pCurrRedline )
        {
            pSh->SwCursorShell::Push();
            if( nullptr == (pCurrRedline = pSh->SelNextRedline()))
                pCurrRedline = pSh->SelPrevRedline();
            pSh->SwCursorShell::Pop( false );
        }
    }
    else
        pCurrRedline = nullptr;

    for (sal_uInt16 i = nStart; i <= nEnd; i++)
    {
        const SwRangeRedline& rRedln = pSh->GetRedline(i);
        const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();

        pRedlineParent = new SwRedlineDataParent;
        pRedlineParent->pData    = pRedlineData;
        pRedlineParent->pNext    = nullptr;
        const OUString& sComment(rRedln.GetComment());
        pRedlineParent->sComment = sComment.replace('\n', ' ');
        m_RedlineParents.insert(m_RedlineParents.begin() + i,
                std::unique_ptr<SwRedlineDataParent>(pRedlineParent));

        RedlinData *pData = new RedlinData;
        pData->pData = pRedlineParent;
        pData->bDisabled = false;

        sParent = GetRedlineText(rRedln, pData->aDateTime);
        pParent = m_pTable->InsertEntry(GetActionImage(rRedln), sParent, pData, nullptr, i);
        if( pCurrRedline == &rRedln )
        {
            m_pTable->SetCurEntry( pParent );
            m_pTable->Select( pParent );
            m_pTable->MakeVisible( pParent );
        }

        pRedlineParent->pTLBParent = pParent;

        InsertChildren(pRedlineParent, rRedln, nAutoFormat);
    }
}

void SwRedlineAcceptDlg::CallAcceptReject( bool bSelect, bool bAccept )
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    SvTreeListEntry* pEntry = bSelect ? m_pTable->FirstSelected() : m_pTable->First();
    sal_uLong nPos = LONG_MAX;

    typedef std::vector<SvTreeListEntry*> ListBoxEntries_t;
    ListBoxEntries_t aRedlines;

    // don't activate
    OSL_ENSURE( !m_bInhibitActivate,
                "recursive call of CallAcceptReject?");
    m_bInhibitActivate = true;

    // collect redlines-to-be-accepted/rejected in aRedlines vector
    while( pEntry )
    {
        if( !m_pTable->GetParent( pEntry ) )
        {
            if( bSelect && LONG_MAX == nPos )
                nPos = m_pTable->GetModel()->GetAbsPos( pEntry );

            RedlinData *pData = static_cast<RedlinData *>(pEntry->GetUserData());

            if( !pData->bDisabled )
                aRedlines.push_back( pEntry );
        }

        pEntry = bSelect ? m_pTable->NextSelected(pEntry) : m_pTable->Next(pEntry);
    }

    bool (SwEditShell:: *FnAccRej)( sal_uInt16 ) = &SwEditShell::AcceptRedline;
    if( !bAccept )
        FnAccRej = &SwEditShell::RejectRedline;

    SwWait aWait( *pSh->GetView().GetDocShell(), true );
    pSh->StartAction();

    if (aRedlines.size() > 1)
    {
        OUString aTmpStr;
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1,
                              OUString::number(aRedlines.size()));
            aTmpStr = aRewriter.Apply(OUString(SW_RES(STR_N_REDLINES)));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        pSh->StartUndo(bAccept? UNDO_ACCEPT_REDLINE : UNDO_REJECT_REDLINE,
                       &aRewriter);
    }

    // accept/reject the redlines in aRedlines. The absolute
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

    if (aRedlines.size() > 1)
    {
        pSh->EndUndo();
    }

    pSh->EndAction();

    m_bInhibitActivate = false;
    Activate();

    if( ULONG_MAX != nPos && m_pTable->GetEntryCount() )
    {
        if( nPos >= m_pTable->GetEntryCount() )
            nPos = m_pTable->GetEntryCount() - 1;
        pEntry = m_pTable->GetEntryAtAbsPos( nPos );
        if( !pEntry && nPos-- )
            pEntry = m_pTable->GetEntryAtAbsPos( nPos );
        if( pEntry )
        {
            m_pTable->Select( pEntry );
            m_pTable->MakeVisible( pEntry );
            m_pTable->SetCurEntry(pEntry);
        }
    }
    m_pTPView->EnableUndo();
}

sal_uInt16 SwRedlineAcceptDlg::GetRedlinePos( const SvTreeListEntry& rEntry ) const
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    return pSh->FindRedlineOfData( *static_cast<SwRedlineDataParent*>(static_cast<RedlinData *>(
                                    rEntry.GetUserData())->pData)->pData );
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, AcceptHdl, SvxTPView*, void)
{
    CallAcceptReject( true, true );
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, AcceptAllHdl, SvxTPView*, void)
{
    CallAcceptReject( false, true );
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, RejectHdl, SvxTPView*, void)
{
    CallAcceptReject( true, false );
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, RejectAllHdl, SvxTPView*, void)
{
    CallAcceptReject( false, false );
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, UndoHdl, SvxTPView*, void)
{
    SwView * pView = ::GetActiveView();
    pView->GetViewFrame()->GetDispatcher()->
                Execute(SID_UNDO, SfxCallMode::SYNCHRON);
    m_pTPView->EnableUndo(pView->GetSlotState(SID_UNDO) != nullptr);

    Activate();
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, FilterChangedHdl, SvxTPFilter*, void)
{
    SvxTPFilter *pFilterTP = m_aTabPagesCTRL->GetFilterPage();

    if (pFilterTP->IsAction())
        m_sFilterAction = pFilterTP->GetLbAction()->GetSelectEntry();
    else
        m_sFilterAction = aEmptyOUStr;

    Init();
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, DeselectHdl, SvTreeListBox*, void)
{
    // avoid flickering of buttons:
    m_aDeselectTimer.Start();
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, SelectHdl, SvTreeListBox*, void)
{
    SelectTimerHdl(nullptr);
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, SelectTimerHdl, Timer *, void)
{
    m_aDeselectTimer.Stop();
    m_aSelectTimer.Start();
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, GotoHdl, Timer *, void)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    m_aSelectTimer.Stop();

    bool bIsNotFormated = false;
    bool bSel = false;

    //#98883# don't select redlines while the dialog is not focussed
    //#107938# But not only ask pTable if it has the focus. To move
    //         the selection to the selected redline any child of pParentDlg
    //         may the focus.
    SvTreeListEntry* pSelEntry = nullptr;

    if (m_pParentDlg->HasChildPathFocus())
        pSelEntry = m_pTable->FirstSelected();

    if( pSelEntry )
    {
        SvTreeListEntry* pActEntry = pSelEntry;
        pSh->StartAction();
        pSh->EnterStdMode();
        SwViewShell::SetCareWin(m_pParentDlg);

        while (pSelEntry)
        {
            if (m_pTable->GetParent(pSelEntry))
            {
                pActEntry = m_pTable->GetParent(pSelEntry);

                if (m_pTable->IsSelected(pActEntry))
                {
                    pSelEntry = pActEntry = m_pTable->NextSelected(pSelEntry);
                    continue;   // don't select twice
                }
            }
            else
                bSel = true;

            // #98864# find the selected redline (ignore, if the redline is already gone)
            sal_uInt16 nPos = GetRedlinePos(*pActEntry);
            if( nPos != USHRT_MAX )
            {

                const SwRangeRedline& rRedln = pSh->GetRedline( nPos );
                bIsNotFormated |= nsRedlineType_t::REDLINE_FORMAT != rRedln.GetType();

                if (pSh->GotoRedline(nPos, true))
                {
                    pSh->SetInSelect();
                    pSh->EnterAddMode();
                }
            }

            pSelEntry = pActEntry = m_pTable->NextSelected(pSelEntry);
        }

        pSh->LeaveAddMode();
        pSh->EndAction();
        SwViewShell::SetCareWin(nullptr);
    }
    bool bEnable = !pSh->getIDocumentRedlineAccess().GetRedlinePassword().getLength();
    m_pTPView->EnableAccept( bEnable && bSel /*&& !bReadonlySel*/ );
    m_pTPView->EnableReject( bEnable && bSel && bIsNotFormated /*&& !bReadonlySel*/ );
    m_pTPView->EnableRejectAll( bEnable && !m_bOnlyFormatedRedlines && !m_bHasReadonlySel );
}

IMPL_LINK_NOARG_TYPED(SwRedlineAcceptDlg, CommandHdl, SvSimpleTable*, void)
{
    const CommandEvent aCEvt(m_pTable->GetCommandEvent());

    switch ( aCEvt.GetCommand() )
    {
        case CommandEventId::ContextMenu:
        {
            SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
            SvTreeListEntry* pEntry = m_pTable->FirstSelected();
            const SwRangeRedline *pRed = nullptr;

            if (pEntry)
            {
                SvTreeListEntry* pTopEntry = pEntry;

                if (m_pTable->GetParent(pEntry))
                    pTopEntry = m_pTable->GetParent(pEntry);

                sal_uInt16 nPos = GetRedlinePos(*pTopEntry);

                // disable commenting for protected areas
                if (nPos != USHRT_MAX && (pRed = pSh->GotoRedline(nPos, true)) != nullptr)
                {
                    if( pSh->IsCursorPtAtEnd() )
                        pSh->SwapPam();
                    pSh->SetInSelect();
                }
            }

            m_aPopup.EnableItem( MN_EDIT_COMMENT, pEntry && pRed &&
                                            !m_pTable->GetParent(pEntry) &&
                                            !m_pTable->NextSelected(pEntry)
//JP 27.9.2001: make no sense if we handle readonly sections
//                                          && pRed->HasReadonlySel()
                                            );

            m_aPopup.EnableItem( MN_SUB_SORT, m_pTable->First() != nullptr );
            sal_uInt16 nColumn = m_pTable->GetSortedCol();
            if (nColumn == 0xffff)
                nColumn = 4;

            PopupMenu *pSubMenu = m_aPopup.GetPopupMenu(MN_SUB_SORT);
            if (pSubMenu)
            {
                for (sal_uInt16 i = MN_SORT_ACTION; i < MN_SORT_ACTION + 5; i++)
                    pSubMenu->CheckItem(i, false);

                pSubMenu->CheckItem(nColumn + MN_SORT_ACTION);
            }

            sal_uInt16 nRet = m_aPopup.Execute(m_pTable, aCEvt.GetMousePosPixel());

            switch( nRet )
            {
                case MN_EDIT_COMMENT:
                {
                    if (pEntry)
                    {
                        if (m_pTable->GetParent(pEntry))
                            pEntry = m_pTable->GetParent(pEntry);

                        sal_uInt16 nPos = GetRedlinePos(*pEntry);

                        if (nPos == USHRT_MAX)
                            break;

                        const SwRangeRedline &rRedline = pSh->GetRedline(nPos);

                        /* enable again once we have redline comments in the margin
                        sComment = rRedline.GetComment();
                        if ( !sComment.Len() )
                            GetActiveView()->GetDocShell()->Broadcast(SwRedlineHint(&rRedline,SWREDLINE_INSERTED));
                        const_cast<SwRangeRedline&>(rRedline).Broadcast(SwRedlineHint(&rRedline,SWREDLINE_FOCUS));
                        */

                        OUString sComment = convertLineEnd(rRedline.GetComment(), GetSystemLineEnd());
                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "Dialog creation failed!");
                        ::DialogGetRanges fnGetRange = pFact->GetDialogGetRangesFunc();
                        OSL_ENSURE(fnGetRange, "Dialog creation failed! GetRanges()");
                        SfxItemSet aSet( pSh->GetAttrPool(), fnGetRange() );

                        aSet.Put(SvxPostItTextItem(sComment, SID_ATTR_POSTIT_TEXT));
                        aSet.Put(SvxPostItAuthorItem(rRedline.GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

                        aSet.Put(SvxPostItDateItem( GetAppLangDateTimeString(
                                    rRedline.GetRedlineData().GetTimeStamp() ),
                                    SID_ATTR_POSTIT_DATE ));

                        std::unique_ptr<AbstractSvxPostItDialog> pDlg(pFact->CreateSvxPostItDialog( m_pParentDlg, aSet ));
                        OSL_ENSURE(pDlg, "Dialog creation failed!");

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
                        case nsRedlineType_t::REDLINE_PARAGRAPH_FORMAT:
                            nResId = STR_REDLINE_FORMATED;
                            break;
                        case nsRedlineType_t::REDLINE_TABLE:
                            nResId = STR_REDLINE_TABLECHG;
                            break;
                        default:;//prevent warning
                        }
                        OUString sTitle(SW_RES(STR_REDLINE_COMMENT));
                        if( nResId )
                            sTitle += SW_RESSTR( nResId );
                        pDlg->SetText(sTitle);

                        SwViewShell::SetCareWin(pDlg->GetWindow());

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                            OUString sMsg(static_cast<const SvxPostItTextItem&>(pOutSet->Get(SID_ATTR_POSTIT_TEXT)).GetValue());

                            // insert / change comment
                            pSh->SetRedlineComment(sMsg);
                            m_pTable->SetEntryText(sMsg.replace('\n', ' '), pEntry, 3);
                        }

                        pDlg.reset();
                        SwViewShell::SetCareWin(nullptr);
                    }

                }
                break;

            case MN_SORT_ACTION:
            case MN_SORT_AUTHOR:
            case MN_SORT_DATE:
            case MN_SORT_COMMENT:
            case MN_SORT_POSITION:
                {
                    bSortDir = true;
                    if (nRet - MN_SORT_ACTION == 4 && m_pTable->GetSortedCol() == 0xffff)
                        break;  // we already have it

                    nSortMode = nRet - MN_SORT_ACTION;
                    if (nSortMode == 4)
                        nSortMode = 0xffff; // unsorted / sorted by position

                    if (m_pTable->GetSortedCol() == nSortMode)
                        bSortDir = !m_pTable->GetSortDirection();

                    SwWait aWait( *::GetActiveView()->GetDocShell(), false );
                    m_pTable->SortByCol(nSortMode, bSortDir);
                    if (nSortMode == 0xffff)
                        Init();             // newly fill everything
                }
                break;
            }
        }
        break;
        default: break;
    }
}

void SwRedlineAcceptDlg::Initialize(const OUString& rExtraData)
{
    if (!rExtraData.isEmpty())
    {
        sal_Int32 nPos = rExtraData.indexOf("AcceptChgDat:");

        // try to read the alignment string "ALIGN:(...)"; if none existing,
        // it's an old version
        if (nPos != -1)
        {
            sal_Int32 n1 = rExtraData.indexOf('(', nPos);
            if (n1 != -1)
            {
                sal_Int32 n2 = rExtraData.indexOf(')', n1);
                if (n2 != -1)
                {
                    // cut out the alignment string
                    OUString aStr = rExtraData.copy(nPos, n2 - nPos + 1);
                    aStr = aStr.copy(n1 - nPos + 1);

                    if (!aStr.isEmpty())
                    {
                        sal_uInt16 nCount = static_cast< sal_uInt16 >(aStr.toInt32());

                        for (sal_uInt16 i = 0; i < nCount; i++)
                        {
                            sal_Int32 n3 = aStr.indexOf(';');
                            aStr = aStr.copy(n3 + 1);
                            m_pTable->SetTab(i, aStr.toInt32(), MAP_PIXEL);
                        }
                    }
                }
            }
        }
    }
}

void SwRedlineAcceptDlg::FillInfo(OUString &rExtraData) const
{
    rExtraData += "AcceptChgDat:(";

    sal_uInt16  nCount = m_pTable->TabCount();

    rExtraData += OUString::number(nCount);
    rExtraData += ";";
    for(sal_uInt16 i = 0; i < nCount; i++)
    {
        rExtraData += OUString::number( m_pTable->GetTab(i) );
        rExtraData += ";";
    }
    rExtraData += ")";
}

SwRedlineAcceptPanel::SwRedlineAcceptPanel(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame)
    : PanelLayout(pParent, "ManageChangesPanel", "modules/swriter/ui/managechangessidebar.ui", rFrame)
{
    mpImplDlg = new SwRedlineAcceptDlg(this, this, get<VclGrid>("content_area"));

    mpImplDlg->Init();

    // we want to receive SFX_HINT_DOCCHANGED
    StartListening(*(SW_MOD()->GetView()->GetDocShell()));
}

SwRedlineAcceptPanel::~SwRedlineAcceptPanel()
{
    disposeOnce();
}

void SwRedlineAcceptPanel::dispose()
{
    delete mpImplDlg;
    mpImplDlg = nullptr;
    PanelLayout::dispose();
}

void SwRedlineAcceptPanel::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SfxSimpleHint *pHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (mpImplDlg && pHint && pHint->GetId() == SFX_HINT_DOCCHANGED)
        mpImplDlg->Activate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
