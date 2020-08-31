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
#include <tools/lineend.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <vcl/commandevent.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <redlndlg.hxx>
#include <swwait.hxx>
#include <uitool.hxx>

#include <helpids.h>
#include <cmdid.h>
#include <strings.hrc>

// -> #111827#
#include <swundo.hxx>
#include <SwRewriter.hxx>
// <- #111827#

#include <vector>
#include <svx/svxdlg.hxx>
#include <bitmaps.hlst>

#include <docsh.hxx>

#include <IDocumentRedlineAccess.hxx>
#include <memory>

SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(SwRedlineAcceptChild, FN_REDLINE_ACCEPT)

SwRedlineAcceptChild::SwRedlineAcceptChild(vcl::Window* _pParent,
                                           sal_uInt16 nId,
                                           SfxBindings* pBindings,
                                           SfxChildWinInfo* pInfo)
    : SwChildWinWrapper(_pParent, nId)
{
    auto xDlg = std::make_shared<SwModelessRedlineAcceptDlg>(pBindings, this, _pParent->GetFrameWeld());
    SetController(xDlg);
    xDlg->Initialize(pInfo);
}

// newly initialise dialog after document switch
bool SwRedlineAcceptChild::ReInitDlg(SwDocShell *pDocSh)
{
    bool bRet = SwChildWinWrapper::ReInitDlg(pDocSh);
    if (bRet)  // update immediately, doc switch!
        static_cast<SwModelessRedlineAcceptDlg*>(GetController().get())->Activate();

    return bRet;
}

SwModelessRedlineAcceptDlg::SwModelessRedlineAcceptDlg(
    SfxBindings* _pBindings, SwChildWinWrapper* pChild, weld::Window *pParent)
    : SfxModelessDialogController(_pBindings, pChild, pParent,
        "svx/ui/acceptrejectchangesdialog.ui", "AcceptRejectChangesDialog")
    , m_xContentArea(m_xDialog->weld_content_area())
    , pChildWin(pChild)
{
    m_xImplDlg.reset(new SwRedlineAcceptDlg(m_xDialog, m_xBuilder.get(), m_xContentArea.get()));
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
        m_xImplDlg->Init();
        SfxModelessDialogController::Activate();

        return;
    }

    SfxModelessDialogController::Activate();
    m_xImplDlg->Activate();
}

void SwModelessRedlineAcceptDlg::Initialize(SfxChildWinInfo* pInfo)
{
    if (pInfo != nullptr)
        m_xImplDlg->Initialize(pInfo->aExtraString);

    SfxModelessDialogController::Initialize(pInfo);
}

void SwModelessRedlineAcceptDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxModelessDialogController::FillInfo(rInfo);
    m_xImplDlg->FillInfo(rInfo.aExtraString);
}

SwModelessRedlineAcceptDlg::~SwModelessRedlineAcceptDlg()
{
}

SwRedlineAcceptDlg::SwRedlineAcceptDlg(const std::shared_ptr<weld::Window>& rParent, weld::Builder *pBuilder,
                                       weld::Container *pContentArea, bool bAutoFormat)
    : m_xParentDlg(rParent)
    , m_sInserted(SwResId(STR_REDLINE_INSERTED))
    , m_sDeleted(SwResId(STR_REDLINE_DELETED))
    , m_sFormated(SwResId(STR_REDLINE_FORMATTED))
    , m_sTableChgd(SwResId(STR_REDLINE_TABLECHG))
    , m_sFormatCollSet(SwResId(STR_REDLINE_FMTCOLLSET))
    , m_sAutoFormat(SwResId(STR_REDLINE_AUTOFMT))
    , m_bOnlyFormatedRedlines(false)
    , m_bRedlnAutoFormat(bAutoFormat)
    , m_bInhibitActivate(false)
    , m_xTabPagesCTRL(new SvxAcceptChgCtr(pContentArea, m_xParentDlg.get(), pBuilder))
    , m_xPopup(pBuilder->weld_menu("writermenu"))
{
    m_xTabPagesCTRL->set_help_id(HID_REDLINE_CTRL);
    m_pTPView = m_xTabPagesCTRL->GetViewPage();

    m_pTable = m_pTPView->GetTableControl();
    m_pTable->SetWriterView();

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

    m_xTabPagesCTRL->GetFilterPage()->SetReadyHdl(LINK(this, SwRedlineAcceptDlg, FilterChangedHdl));

    weld::ComboBox* pActLB = m_xTabPagesCTRL->GetFilterPage()->GetLbAction();
    pActLB->append_text(m_sInserted);
    pActLB->append_text(m_sDeleted);
    pActLB->append_text(m_sFormated);
    pActLB->append_text(m_sTableChgd);

    if (HasRedlineAutoFormat())
    {
        pActLB->append_text(m_sFormatCollSet);
        pActLB->append_text(m_sAutoFormat);
        m_pTPView->ShowUndo();
        m_pTPView->DisableUndo();     // no UNDO events yet
    }

    pActLB->set_active(0);

    weld::TreeView& rTreeView = m_pTable->GetWidget();
    rTreeView.set_selection_mode(SelectionMode::Multiple);

    rTreeView.connect_changed(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    rTreeView.connect_popup_menu(LINK(this, SwRedlineAcceptDlg, CommandHdl));

    // avoid multiple selection of the same texts:
    m_aSelectTimer.SetTimeout(100);
    m_aSelectTimer.SetInvokeHandler(LINK(this, SwRedlineAcceptDlg, GotoHdl));
}

SwRedlineAcceptDlg::~SwRedlineAcceptDlg()
{
}

void SwRedlineAcceptDlg::Init(SwRedlineTable::size_type nStart)
{
    SwWait aWait( *::GetActiveView()->GetDocShell(), false );
    weld::TreeView& rTreeView = m_pTable->GetWidget();
    m_aUsedSeqNo.clear();

    rTreeView.freeze();
    if (nStart)
        RemoveParents(nStart, m_RedlineParents.size() - 1);
    else
    {
        rTreeView.clear();
        m_RedlineChildren.clear();
        m_RedlineParents.erase(m_RedlineParents.begin() + nStart, m_RedlineParents.end());
    }
    rTreeView.thaw();

    // insert parents
    InsertParents(nStart);
    InitAuthors();

    // #i69618# this moves the list box to the right position, visually
    std::unique_ptr<weld::TreeIter> xSelEntry(rTreeView.make_iterator());
    if (rTreeView.get_selected(xSelEntry.get()))
        rTreeView.scroll_to_row(*xSelEntry); //#i70937#, force the scroll
}

void SwRedlineAcceptDlg::InitAuthors()
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();

    if (!m_xTabPagesCTRL)
        return;

    SvxTPFilter *pFilterPage = m_xTabPagesCTRL->GetFilterPage();

    std::vector<OUString> aStrings;
    OUString sOldAuthor(pFilterPage->GetSelectedAuthor());
    pFilterPage->ClearAuthors();

    SwRedlineTable::size_type nCount = pSh->GetRedlineCount();

    m_bOnlyFormatedRedlines = true;
    bool bIsNotFormated = false;

    // determine authors
    for ( SwRedlineTable::size_type i = 0; i < nCount; i++)
    {
        const SwRangeRedline& rRedln = pSh->GetRedline(i);

        if( m_bOnlyFormatedRedlines && RedlineType::Format != rRedln.GetType() )
            m_bOnlyFormatedRedlines = false;

        aStrings.push_back(rRedln.GetAuthorString());

        for (sal_uInt16 nStack = 1; nStack < rRedln.GetStackCount(); nStack++)
        {
            aStrings.push_back(rRedln.GetAuthorString(nStack));
        }
    }

    std::sort(aStrings.begin(), aStrings.end());
    aStrings.erase(std::unique(aStrings.begin(), aStrings.end()), aStrings.end());

    for (auto const & i: aStrings)
        pFilterPage->InsertAuthor(i);

    if (pFilterPage->SelectAuthor(sOldAuthor) == -1 && !aStrings.empty())
        pFilterPage->SelectAuthor(aStrings[0]);

    weld::TreeView& rTreeView = m_pTable->GetWidget();
    bool bEnable = rTreeView.n_children() != 0 && !pSh->getIDocumentRedlineAccess().GetRedlinePassword().hasElements();
    bool bSel = rTreeView.get_selected(nullptr);

    rTreeView.selected_foreach([this, pSh, &bIsNotFormated](weld::TreeIter& rEntry){
        // find the selected redline
        // (fdo#57874: ignore, if the redline is already gone)
        SwRedlineTable::size_type nPos = GetRedlinePos(rEntry);
        if( nPos != SwRedlineTable::npos )
        {
            const SwRangeRedline& rRedln = pSh->GetRedline( nPos );

            bIsNotFormated |= RedlineType::Format != rRedln.GetType();
        }
        return false;
    });

    m_pTPView->EnableAccept( bEnable && bSel );
    m_pTPView->EnableReject( bEnable && bSel );
    m_pTPView->EnableClearFormat( bEnable && !bIsNotFormated && bSel );
    m_pTPView->EnableAcceptAll( bEnable );
    m_pTPView->EnableRejectAll( bEnable );
    m_pTPView->EnableClearFormatAll( bEnable &&
                                m_bOnlyFormatedRedlines );
}

OUString SwRedlineAcceptDlg::GetActionImage(const SwRangeRedline& rRedln, sal_uInt16 nStack)
{
    switch (rRedln.GetType(nStack))
    {
        case RedlineType::Insert:  return BMP_REDLINE_INSERTED;
        case RedlineType::Delete:  return BMP_REDLINE_DELETED;
        case RedlineType::Format:  return BMP_REDLINE_FORMATTED;
        case RedlineType::ParagraphFormat: return BMP_REDLINE_FORMATTED;
        case RedlineType::Table:   return BMP_REDLINE_TABLECHG;
        case RedlineType::FmtColl: return BMP_REDLINE_FMTCOLLSET;
        default: break;
    }

    return OUString();
}

OUString SwRedlineAcceptDlg::GetActionText(const SwRangeRedline& rRedln, sal_uInt16 nStack)
{
    switch( rRedln.GetType(nStack) )
    {
        case RedlineType::Insert:   return m_sInserted;
        case RedlineType::Delete:   return m_sDeleted;
        case RedlineType::Format:   return m_sFormated;
        case RedlineType::ParagraphFormat:   return m_sFormated;
        case RedlineType::Table:    return m_sTableChgd;
        case RedlineType::FmtColl:  return m_sFormatCollSet;
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
    SwRedlineTable::size_type nCount = pSh->GetRedlineCount();

    // check the number of pointers
    for ( SwRedlineTable::size_type i = 0; i < nCount; i++)
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
            if ((i = CalcDiff(i, false)) == SwRedlineTable::npos)
                return;
            continue;
        }

        const SwRedlineData *pRedlineData = rRedln.GetRedlineData().Next();
        const SwRedlineDataChild *pBackupData = pParent->pNext;

        if (!pRedlineData && pBackupData)
        {
            // Redline-Children were deleted
            if ((i = CalcDiff(i, true)) == SwRedlineTable::npos)
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
                    if ((i = CalcDiff(i, true)) == SwRedlineTable::npos)
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
    weld::TreeView& rTreeView = m_pTable->GetWidget();
    for (SwRedlineTable::size_type i = 0; i < nCount; i++)
    {
        const SwRangeRedline& rRedln = pSh->GetRedline(i);
        SwRedlineDataParent *const pParent = m_RedlineParents[i].get();

        if(rRedln.GetComment() != pParent->sComment)
        {
            if (pParent->xTLBParent)
            {
                // update only comment
                const OUString& sComment(rRedln.GetComment());
                rTreeView.set_text(*pParent->xTLBParent, sComment.replace('\n', ' '), 3);
            }
            pParent->sComment = rRedln.GetComment();
        }
    }

    InitAuthors();
}

SwRedlineTable::size_type SwRedlineAcceptDlg::CalcDiff(SwRedlineTable::size_type nStart, bool bChild)
{
    if (!nStart)
    {
        Init();
        return SwRedlineTable::npos;
    }

    weld::TreeView& rTreeView = m_pTable->GetWidget();
    rTreeView.freeze();
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    bool bHasRedlineAutoFormat = HasRedlineAutoFormat();
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
            if (pBackupData->xTLBChild)
                rTreeView.remove(*pBackupData->xTLBChild);

            auto it = std::find_if(m_RedlineChildren.begin(), m_RedlineChildren.end(),
                [&pBackupData](const std::unique_ptr<SwRedlineDataChild>& rChildPtr) { return rChildPtr.get() == pBackupData; });
            if (it != m_RedlineChildren.end())
                m_RedlineChildren.erase(it);

            pBackupData = pNext;
        }
        pParent->pNext = nullptr;

        // insert new children
        InsertChildren(pParent, rRedln, bHasRedlineAutoFormat);

        rTreeView.thaw();
        return nStart;
    }

    // have entries been deleted?
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    for (SwRedlineTable::size_type i = nStart + 1; i < m_RedlineParents.size(); i++)
    {
        if (m_RedlineParents[i]->pData == pRedlineData)
        {
            // remove entries from nStart to i-1
            RemoveParents(nStart, i - 1);
            rTreeView.thaw();
            return nStart - 1;
        }
    }

    // entries been inserted?
    SwRedlineTable::size_type nCount = pSh->GetRedlineCount();
    pRedlineData = m_RedlineParents[nStart]->pData;

    for (SwRedlineTable::size_type i = nStart + 1; i < nCount; i++)
    {
        if (&pSh->GetRedline(i).GetRedlineData() == pRedlineData)
        {
            // insert entries from nStart to i-1
            InsertParents(nStart, i - 1);
            rTreeView.thaw();
            return nStart - 1;
        }
    }

    rTreeView.thaw();
    Init(nStart);   // adjust all entries until the end
    return SwRedlineTable::npos;
}

void SwRedlineAcceptDlg::InsertChildren(SwRedlineDataParent *pParent, const SwRangeRedline& rRedln, bool bHasRedlineAutoFormat)
{
    SwRedlineDataChild *pLastRedlineChild = nullptr;
    const SwRedlineData *pRedlineData = &rRedln.GetRedlineData();
    bool bAutoFormatRedline = rRedln.IsAutoFormat();

    weld::TreeView& rTreeView = m_pTable->GetWidget();

    OUString sAction = GetActionText(rRedln);
    bool bValidParent = m_sFilterAction.isEmpty() || m_sFilterAction == sAction;
    bValidParent = bValidParent && m_pTable->IsValidEntry(rRedln.GetAuthorString(), rRedln.GetTimeStamp(), rRedln.GetComment());
    if (bHasRedlineAutoFormat)
    {

        if (pParent->pData->GetSeqNo())
        {
            std::pair<SwRedlineDataParentSortArr::const_iterator,bool> const ret
                = m_aUsedSeqNo.insert(pParent);
            if (ret.second) // already there
            {
                if (pParent->xTLBParent)
                {
                    rTreeView.set_text(*(*ret.first)->xTLBParent, m_sAutoFormat, 0);
                    rTreeView.remove(*pParent->xTLBParent);
                    pParent->xTLBParent.reset();
                }
                return;
            }
        }
        bValidParent = bValidParent && bAutoFormatRedline;
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
        if (bHasRedlineAutoFormat)
            bValidChild = bValidChild && bAutoFormatRedline;
        bValidTree |= bValidChild;

        if (bValidChild)
        {
            std::unique_ptr<RedlinData> pData(new RedlinData);
            pData->pData = pRedlineChild;
            pData->bDisabled = true;

            OUString sImage(GetActionImage(rRedln, nStack));
            OUString sAuthor = rRedln.GetAuthorString(nStack);
            pData->aDateTime = rRedln.GetTimeStamp(nStack);
            pData->eType = rRedln.GetType(nStack);
            OUString sDateEntry = GetAppLangDateTimeString(pData->aDateTime);
            OUString sComment = rRedln.GetComment(nStack);

            std::unique_ptr<weld::TreeIter> xChild(rTreeView.make_iterator());
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pData.release())));
            rTreeView.insert(pParent->xTLBParent.get(), -1, nullptr, &sId, nullptr, nullptr,
                             nullptr, false, xChild.get());

            rTreeView.set_image(*xChild, sImage, -1);
            rTreeView.set_text(*xChild, sAuthor, 1);
            rTreeView.set_text(*xChild, sDateEntry, 2);
            rTreeView.set_text(*xChild, sComment, 3);

            pRedlineChild->xTLBChild = std::move(xChild);
            if (!bValidParent)
                rTreeView.expand_row(*pParent->xTLBParent);
        }
        else
            pRedlineChild->xTLBChild.reset();

        pLastRedlineChild = pRedlineChild;
    }

    if (pLastRedlineChild)
        pLastRedlineChild->pNext = nullptr;

    if (!bValidTree && pParent->xTLBParent)
    {
        rTreeView.remove(*pParent->xTLBParent);
        pParent->xTLBParent.reset();
        if (bHasRedlineAutoFormat)
            m_aUsedSeqNo.erase(pParent);
    }
}

void SwRedlineAcceptDlg::RemoveParents(SwRedlineTable::size_type nStart, SwRedlineTable::size_type nEnd)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    SwRedlineTable::size_type nCount = pSh->GetRedlineCount();

    std::vector<const weld::TreeIter*> aLBoxArr;

    weld::TreeView& rTreeView = m_pTable->GetWidget();

    // because of Bug of TLB that ALWAYS calls the SelectHandler at Remove:
    rTreeView.connect_changed(Link<weld::TreeView&,void>());

    bool bChildrenRemoved = false;
    rTreeView.thaw();
    rTreeView.unselect_all();

    // set the cursor after the last entry because otherwise performance problem in TLB.
    // TLB would otherwise reset the cursor at every Remove (expensive)
    SwRedlineTable::size_type nPos = std::min(nCount, m_RedlineParents.size());
    weld::TreeIter *pCurEntry = nullptr;
    while( ( pCurEntry == nullptr ) && ( nPos > 0 ) )
    {
        --nPos;
        pCurEntry = m_RedlineParents[nPos]->xTLBParent.get();
    }

    if (pCurEntry)
        rTreeView.set_cursor(*pCurEntry);

    rTreeView.freeze();

    for (SwRedlineTable::size_type i = nStart; i <= nEnd; i++)
    {
        if (!bChildrenRemoved && m_RedlineParents[i]->pNext)
        {
            SwRedlineDataChild * pChildPtr =
                const_cast<SwRedlineDataChild*>(m_RedlineParents[i]->pNext);
            auto it = std::find_if(m_RedlineChildren.begin(), m_RedlineChildren.end(),
                [&pChildPtr](const std::unique_ptr<SwRedlineDataChild>& rChildPtr) { return rChildPtr.get() == pChildPtr; });
            if (it != m_RedlineChildren.end())
            {
                sal_uInt16 nChildren = 0;
                while (pChildPtr)
                {
                    pChildPtr = const_cast<SwRedlineDataChild*>(pChildPtr->pNext);
                    nChildren++;
                }

                m_RedlineChildren.erase(it, it + nChildren);
                bChildrenRemoved = true;
            }
        }
        weld::TreeIter *const pEntry = m_RedlineParents[i]->xTLBParent.get();
        if (pEntry)
            aLBoxArr.push_back(pEntry);
    }

    std::sort(aLBoxArr.begin(), aLBoxArr.end(), [&rTreeView](const weld::TreeIter* a, const weld::TreeIter* b) {
        return rTreeView.iter_compare(*a, *b) == -1;
    });
    // clear TLB from behind
    for (auto it = aLBoxArr.rbegin(); it != aLBoxArr.rend(); ++it)
    {
        const weld::TreeIter* pIter = *it;
        rTreeView.remove(*pIter);
    }

    rTreeView.thaw();
    rTreeView.connect_changed(LINK(this, SwRedlineAcceptDlg, SelectHdl));
    // unfortunately by Remove it was selected from the TLB always again ...
    rTreeView.unselect_all();
    rTreeView.freeze();

    m_RedlineParents.erase(m_RedlineParents.begin() + nStart, m_RedlineParents.begin() + nEnd + 1);
}

void SwRedlineAcceptDlg::InsertParents(SwRedlineTable::size_type nStart, SwRedlineTable::size_type nEnd)
{
    SwView *pView   = ::GetActiveView();
    SwWrtShell* pSh = pView->GetWrtShellPtr();
    bool bHasRedlineAutoFormat = HasRedlineAutoFormat();

    SwRedlineTable::size_type nCount = pSh->GetRedlineCount();
    nEnd = std::min(nEnd, (nCount - 1)); // also treats nEnd=SwRedlineTable::npos (until the end)

    if (nEnd == SwRedlineTable::npos)
        return;     // no redlines in the document

    weld::TreeView& rTreeView = m_pTable->GetWidget();

    SwRedlineDataParent* pRedlineParent;
    const SwRangeRedline* pCurrRedline;
    if (!nStart && !rTreeView.get_selected(nullptr))
    {
        pCurrRedline = pSh->GetCurrRedline();
        if( !pCurrRedline )
        {
            pSh->SwCursorShell::Push();
            if( nullptr == (pCurrRedline = pSh->SelNextRedline()))
                pCurrRedline = pSh->SelPrevRedline();
            pSh->SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
        }
    }
    else
        pCurrRedline = nullptr;

    rTreeView.freeze();
    if (m_pTable->IsSorted())
        rTreeView.make_unsorted();
    for (SwRedlineTable::size_type i = nStart; i <= nEnd; i++)
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

        std::unique_ptr<RedlinData> pData(new RedlinData);
        pData->pData = pRedlineParent;
        pData->bDisabled = false;

        OUString sImage = GetActionImage(rRedln);
        OUString sAuthor = rRedln.GetAuthorString(0);
        pData->aDateTime = rRedln.GetTimeStamp(0);
        pData->eType = rRedln.GetType(0);
        OUString sDateEntry = GetAppLangDateTimeString(pData->aDateTime);

        OUString sId = OUString::number(reinterpret_cast<sal_Int64>(pData.release()));
        std::unique_ptr<weld::TreeIter> xParent(rTreeView.make_iterator());
        rTreeView.insert(nullptr, i, nullptr, &sId, nullptr, nullptr, nullptr, false, xParent.get());

        rTreeView.set_image(*xParent, sImage, -1);
        rTreeView.set_text(*xParent, sAuthor, 1);
        rTreeView.set_text(*xParent, sDateEntry, 2);
        rTreeView.set_text(*xParent, sComment, 3);

        if (pCurrRedline == &rRedln)
        {
            rTreeView.thaw();
            rTreeView.set_cursor(*xParent);
            rTreeView.select(*xParent);
            rTreeView.scroll_to_row(*xParent);
            rTreeView.freeze();
        }

        pRedlineParent->xTLBParent = std::move(xParent);

        InsertChildren(pRedlineParent, rRedln, bHasRedlineAutoFormat);
    }
    rTreeView.thaw();
    if (m_pTable->IsSorted())
        rTreeView.make_sorted();
}

void SwRedlineAcceptDlg::CallAcceptReject( bool bSelect, bool bAccept )
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    int nPos = -1;

    typedef std::vector<std::unique_ptr<weld::TreeIter>> ListBoxEntries_t;
    ListBoxEntries_t aRedlines;

    // don't activate
    OSL_ENSURE( !m_bInhibitActivate,
                "recursive call of CallAcceptReject?");
    m_bInhibitActivate = true;

    weld::TreeView& rTreeView = m_pTable->GetWidget();

    auto lambda = [this, pSh, bSelect, bAccept, &rTreeView, &nPos, &aRedlines](weld::TreeIter& rEntry) {
        if (!rTreeView.get_iter_depth(rEntry))
        {
            if (bSelect && nPos == -1)
                nPos = rTreeView.get_iter_index_in_parent(rEntry);

            RedlinData *pData = reinterpret_cast<RedlinData*>(rTreeView.get_id(rEntry).toInt64());

            bool bIsNotFormatted = true;

            // first remove only changes with insertion/deletion, if they exist
            // (format-only changes haven't had real rejection yet, only an
            // approximation: clear direct formatting, so try to warn
            // with the extended button label "Reject All/Clear formatting")
            if ( !bSelect && !bAccept && !m_bOnlyFormatedRedlines )
            {
                SwRedlineTable::size_type nPosition = GetRedlinePos(rEntry);
                const SwRangeRedline& rRedln = pSh->GetRedline(nPosition);

                if( RedlineType::Format == rRedln.GetType() )
                    bIsNotFormatted = false;
            }

            if (!pData->bDisabled && bIsNotFormatted)
                aRedlines.emplace_back(rTreeView.make_iterator(&rEntry));
        }
        return false;
    };

    // collect redlines-to-be-accepted/rejected in aRedlines vector
    if (bSelect)
        rTreeView.selected_foreach(lambda);
    else
        rTreeView.all_foreach(lambda);

    bool (SwEditShell:: *FnAccRej)( SwRedlineTable::size_type ) = &SwEditShell::AcceptRedline;
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
            aTmpStr = aRewriter.Apply(SwResId(STR_N_REDLINES));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        pSh->StartUndo(bAccept? SwUndoId::ACCEPT_REDLINE : SwUndoId::REJECT_REDLINE,
                       &aRewriter);
    }

    // accept/reject the redlines in aRedlines. The absolute
    // position may change during the process (e.g. when two redlines
    // are merged in result of another one being deleted), so the
    // position must be resolved late and checked before using it.
    // (cf #102547#)
    for (const auto& rRedLine : aRedlines)
    {
        SwRedlineTable::size_type nPosition = GetRedlinePos( *rRedLine );
        if( nPosition != SwRedlineTable::npos )
            (pSh->*FnAccRej)( nPosition );
    }

    if (aRedlines.size() > 1)
    {
        pSh->EndUndo();
    }

    pSh->EndAction();

    m_bInhibitActivate = false;
    Activate();

    if (nPos != -1 && rTreeView.n_children())
    {
        if (nPos >= rTreeView.n_children())
            nPos = rTreeView.n_children() - 1;
        rTreeView.select(nPos);
        rTreeView.scroll_to_row(nPos);
        rTreeView.set_cursor(nPos);
        SelectHdl(rTreeView);
    }
    m_pTPView->EnableUndo();
}

SwRedlineTable::size_type SwRedlineAcceptDlg::GetRedlinePos(const weld::TreeIter& rEntry)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    weld::TreeView& rTreeView = m_pTable->GetWidget();
    return pSh->FindRedlineOfData( *static_cast<SwRedlineDataParent*>(reinterpret_cast<RedlinData*>(
                                    rTreeView.get_id(rEntry).toInt64())->pData)->pData );
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, AcceptHdl, SvxTPView*, void)
{
    CallAcceptReject( true, true );
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, AcceptAllHdl, SvxTPView*, void)
{
    CallAcceptReject( false, true );
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, RejectHdl, SvxTPView*, void)
{
    CallAcceptReject( true, false );
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, RejectAllHdl, SvxTPView*, void)
{
    CallAcceptReject( false, false );
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, UndoHdl, SvxTPView*, void)
{
    SwView * pView = ::GetActiveView();
    pView->GetViewFrame()->GetDispatcher()->
                Execute(SID_UNDO, SfxCallMode::SYNCHRON);
    m_pTPView->EnableUndo(pView->GetSlotState(SID_UNDO) != nullptr);

    Activate();
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, FilterChangedHdl, SvxTPFilter*, void)
{
    SvxTPFilter *pFilterTP = m_xTabPagesCTRL->GetFilterPage();

    if (pFilterTP->IsAction())
        m_sFilterAction = pFilterTP->GetLbAction()->get_active_text();
    else
        m_sFilterAction.clear();

    Init();
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, SelectHdl, weld::TreeView&, void)
{
    m_aSelectTimer.Start();
}

IMPL_LINK_NOARG(SwRedlineAcceptDlg, GotoHdl, Timer *, void)
{
    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    m_aSelectTimer.Stop();

    bool bIsNotFormated = false;
    bool bSel = false;

    //#98883# don't select redlines while the dialog is not focused
    //#107938# But not only ask pTable if it has the focus. To move
    //         the selection to the selected redline any child of pParentDlg
    //         may the focus.
    if (!m_xParentDlg || m_xParentDlg->has_toplevel_focus())
    {
        weld::TreeView& rTreeView = m_pTable->GetWidget();
        std::unique_ptr<weld::TreeIter> xActEntry(rTreeView.make_iterator());
        if (rTreeView.get_selected(xActEntry.get()))
        {
            pSh->StartAction();
            pSh->EnterStdMode();
            SwViewShell::SetCareDialog(m_xParentDlg);

            rTreeView.selected_foreach([this, pSh, &rTreeView, &xActEntry, &bIsNotFormated, &bSel](weld::TreeIter& rEntry){
                rTreeView.copy_iterator(rEntry, *xActEntry);
                if (rTreeView.get_iter_depth(rEntry))
                {
                    rTreeView.iter_parent(*xActEntry);
                    if (rTreeView.is_selected(*xActEntry))
                        return false;   // don't select twice
                }
                else
                    bSel = true;

                // #98864# find the selected redline (ignore, if the redline is already gone)
                SwRedlineTable::size_type nPos = GetRedlinePos(*xActEntry);
                if (nPos != SwRedlineTable::npos)
                {

                    const SwRangeRedline& rRedln = pSh->GetRedline( nPos );
                    bIsNotFormated |= RedlineType::Format != rRedln.GetType();

                    if (pSh->GotoRedline(nPos, true))
                    {
                        pSh->SetInSelect();
                        pSh->EnterAddMode();
                    }
                }
                return false;
            });

            pSh->LeaveAddMode();
            pSh->EndAction();
            SwViewShell::SetCareDialog(nullptr);
        }
    }

    bool bEnable = !pSh->getIDocumentRedlineAccess().GetRedlinePassword().hasElements();
    m_pTPView->EnableAccept( bEnable && bSel /*&& !bReadonlySel*/ );
    m_pTPView->EnableReject( bEnable && bSel /*&& !bReadonlySel*/ );
    m_pTPView->EnableClearFormat( bEnable && bSel && !bIsNotFormated /*&& !bReadonlySel*/ );
    m_pTPView->EnableRejectAll( bEnable );
    m_pTPView->EnableClearFormatAll( bEnable && m_bOnlyFormatedRedlines );
}

IMPL_LINK(SwRedlineAcceptDlg, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    SwWrtShell* pSh = ::GetActiveView()->GetWrtShellPtr();
    const SwRangeRedline *pRed = nullptr;

    weld::TreeView& rTreeView = m_pTable->GetWidget();
    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    bool bEntry = rTreeView.get_selected(xEntry.get());
    if (bEntry)
    {
        std::unique_ptr<weld::TreeIter> xTopEntry(rTreeView.make_iterator(xEntry.get()));

        if (rTreeView.get_iter_depth(*xTopEntry))
            rTreeView.iter_parent(*xTopEntry);

        SwRedlineTable::size_type nPos = GetRedlinePos(*xTopEntry);

        // disable commenting for protected areas
        if (nPos != SwRedlineTable::npos && (pRed = pSh->GotoRedline(nPos, true)) != nullptr)
        {
            if( pSh->IsCursorPtAtEnd() )
                pSh->SwapPam();
            pSh->SetInSelect();
        }
    }

    m_xPopup->set_sensitive("writeredit", bEntry && pRed &&
                                          !rTreeView.get_iter_depth(*xEntry) &&
                                          rTreeView.count_selected_rows() == 1);
    m_xPopup->set_sensitive("writersort", rTreeView.n_children() != 0);
    int nColumn = rTreeView.get_sort_column();
    if (nColumn == -1)
        nColumn = 4;
    for (sal_Int32 i = 0; i < 5; ++i)
        m_xPopup->set_active("writersort" + OString::number(i), i == nColumn);

    OString sCommand = m_xPopup->popup_at_rect(&rTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));

    if (sCommand == "writeredit")
    {
        if (bEntry)
        {
            if (rTreeView.get_iter_depth(*xEntry))
                rTreeView.iter_parent(*xEntry);

            SwRedlineTable::size_type nPos = GetRedlinePos(*xEntry);

            if (nPos == SwRedlineTable::npos)
                return true;

            const SwRangeRedline &rRedline = pSh->GetRedline(nPos);

            /* enable again once we have redline comments in the margin
            sComment = rRedline.GetComment();
            if ( !sComment.Len() )
                GetActiveView()->GetDocShell()->Broadcast(SwRedlineHint(&rRedline,SWREDLINE_INSERTED));
            const_cast<SwRangeRedline&>(rRedline).Broadcast(SwRedlineHint(&rRedline,SWREDLINE_FOCUS));
            */

            OUString sComment = convertLineEnd(rRedline.GetComment(), GetSystemLineEnd());
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ::DialogGetRanges fnGetRange = pFact->GetDialogGetRangesFunc();
            SfxItemSet aSet( pSh->GetAttrPool(), fnGetRange() );

            aSet.Put(SvxPostItTextItem(sComment, SID_ATTR_POSTIT_TEXT));
            aSet.Put(SvxPostItAuthorItem(rRedline.GetAuthorString(), SID_ATTR_POSTIT_AUTHOR));

            aSet.Put(SvxPostItDateItem( GetAppLangDateTimeString(
                        rRedline.GetRedlineData().GetTimeStamp() ),
                        SID_ATTR_POSTIT_DATE ));

            ScopedVclPtr<AbstractSvxPostItDialog> pDlg(pFact->CreateSvxPostItDialog(&rTreeView, aSet));

            pDlg->HideAuthor();

            const char* pResId = nullptr;
            switch( rRedline.GetType() )
            {
                case RedlineType::Insert:
                    pResId = STR_REDLINE_INSERTED;
                    break;
                case RedlineType::Delete:
                    pResId = STR_REDLINE_DELETED;
                    break;
                case RedlineType::Format:
                case RedlineType::ParagraphFormat:
                    pResId = STR_REDLINE_FORMATTED;
                    break;
                case RedlineType::Table:
                    pResId = STR_REDLINE_TABLECHG;
                    break;
                default:;//prevent warning
            }
            OUString sTitle(SwResId(STR_REDLINE_COMMENT));
            if (pResId)
                sTitle += SwResId(pResId);
            pDlg->SetText(sTitle);

            SwViewShell::SetCareDialog(pDlg->GetDialog());

            if ( pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                OUString sMsg(pOutSet->Get(SID_ATTR_POSTIT_TEXT).GetValue());

                // insert / change comment
                pSh->SetRedlineComment(sMsg);
                rTreeView.set_text(*xEntry, sMsg.replace('\n', ' '), 3);
                Init();
            }

            SwViewShell::SetCareDialog(nullptr);
            pDlg.disposeAndClear();
        }
    }
    else if (!sCommand.isEmpty())
    {
        int nSortMode = sCommand.copy(10).toInt32();

        if (nSortMode == 4 && nColumn == 4)
            return true;  // we already have it
        if (nSortMode == 4)
            nSortMode = -1; // unsorted / sorted by position

        SwWait aWait( *::GetActiveView()->GetDocShell(), false );
        m_pTable->HeaderBarClick(nSortMode);
        if (nSortMode == -1)
            Init();             // newly fill everything
    }
    return true;
}

namespace
{
    OUString lcl_StripAcceptChgDat(OUString &rExtraString)
    {
        OUString aStr;
        while(true)
        {
            sal_Int32 nPos = rExtraString.indexOf("AcceptChgDat:");
            if (nPos == -1)
                break;
            // try to read the alignment string "ALIGN:(...)"; if none existing,
            // it's an old version
            sal_Int32 n1 = rExtraString.indexOf('(', nPos);
            if (n1 != -1)
            {
                sal_Int32 n2 = rExtraString.indexOf(')', n1);
                if (n2 != -1)
                {
                    // cut out the alignment string
                    aStr = rExtraString.copy(nPos, n2 - nPos + 1);
                    rExtraString = rExtraString.replaceAt(nPos, n2 - nPos + 1, "");
                    aStr = aStr.copy(n1 - nPos + 1);
                }
            }
        }
        return aStr;
    }
}

void SwRedlineAcceptDlg::Initialize(OUString& rExtraString)
{
    if (!rExtraString.isEmpty())
    {
        OUString aStr = lcl_StripAcceptChgDat(rExtraString);
        if (!aStr.isEmpty())
        {
            int nCount = aStr.toInt32();
            if (nCount > 2)
            {
                std::vector<int> aEndPos;

                for (int i = 0; i < nCount; ++i)
                {
                    sal_Int32 n1 = aStr.indexOf(';');
                    aStr = aStr.copy( n1+1 );
                    aEndPos.push_back(aStr.toInt32());
                }

                bool bUseless = false;

                std::vector<int> aWidths;
                for (int i = 1; i < nCount; ++i)
                {
                    aWidths.push_back(aEndPos[i] - aEndPos[i - 1]);
                    if (aWidths.back() <= 0)
                        bUseless = true;
                }

                if (!bUseless)
                {
                    // turn column end points back to column widths, ignoring the small
                    // value used for the expander column
                    weld::TreeView& rTreeView = m_pTable->GetWidget();
                    rTreeView.set_column_fixed_widths(aWidths);
                }
            }
        }
    }
}

void SwRedlineAcceptDlg::FillInfo(OUString &rExtraData) const
{
    //remove any old one before adding a new one
    lcl_StripAcceptChgDat(rExtraData);
    rExtraData += "AcceptChgDat:(";

    const int nTabCount = 4;

    rExtraData += OUString::number(nTabCount);
    rExtraData += ";";

    weld::TreeView& rTreeView = m_pTable->GetWidget();
    std::vector<int> aWidths;
    // turn column widths back into column end points for compatibility
    // with how they used to be stored, including a small value for the
    // expander column
    aWidths.push_back(rTreeView.get_checkbox_column_width());
    for (int i = 0; i < nTabCount - 1; ++i)
    {
        int nWidth = rTreeView.get_column_width(i);
        assert(nWidth > 0 && "suspicious to get a value like this");
        aWidths.push_back(aWidths.back() + nWidth);
    }

    for (auto a : aWidths)
    {
        rExtraData += OUString::number(a);
        rExtraData += ";";
    }
    rExtraData += ")";
}

SwRedlineAcceptPanel::SwRedlineAcceptPanel(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame)
    : PanelLayout(pParent, "ManageChangesPanel", "modules/swriter/ui/managechangessidebar.ui", rFrame)
    , mxContentArea(m_xBuilder->weld_container("content_area"))
{
    mpImplDlg.reset(new SwRedlineAcceptDlg(nullptr, m_xBuilder.get(), mxContentArea.get()));

    mpImplDlg->Init();

    // we want to receive SfxHintId::DocChanged
    StartListening(*(SW_MOD()->GetView()->GetDocShell()));
}

SwRedlineAcceptPanel::~SwRedlineAcceptPanel()
{
    disposeOnce();
}

void SwRedlineAcceptPanel::dispose()
{
    mpImplDlg.reset();
    mxContentArea.reset();
    PanelLayout::dispose();
}

void SwRedlineAcceptPanel::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (mpImplDlg && rHint.GetId() == SfxHintId::DocChanged)
        mpImplDlg->Activate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
