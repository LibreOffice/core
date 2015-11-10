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

#include <UndoManager.hxx>

#include <vcl/wrkwin.hxx>
#include <svx/svdmodel.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <ndarr.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <undo.hrc>
#include <editsh.hxx>
#include <unobaseclass.hxx>
#include <limits>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>

using namespace ::com::sun::star;

// the undo array should never grow beyond this limit:
#define UNDO_ACTION_LIMIT (USHRT_MAX - 1000)

namespace sw {

UndoManager::UndoManager(std::shared_ptr<SwNodes> xUndoNodes,
            IDocumentDrawModelAccess & rDrawModelAccess,
            IDocumentRedlineAccess & rRedlineAccess,
            IDocumentState & rState)
    :   m_rDrawModelAccess(rDrawModelAccess)
    ,   m_rRedlineAccess(rRedlineAccess)
    ,   m_rState(rState)
    ,   m_xUndoNodes(xUndoNodes)
    ,   m_bGroupUndo(true)
    ,   m_bDrawUndo(true)
    ,   m_bLockUndoNoModifiedPosition(false)
    ,   m_UndoSaveMark(MARK_INVALID)
{
    OSL_ASSERT(m_xUndoNodes.get());
    // writer expects it to be disabled initially
    // Undo is enabled by SwEditShell constructor
    SdrUndoManager::EnableUndo(false);
}

SwNodes const& UndoManager::GetUndoNodes() const
{
    return *m_xUndoNodes;
}

SwNodes      & UndoManager::GetUndoNodes()
{
    return *m_xUndoNodes;
}

bool UndoManager::IsUndoNodes(SwNodes const& rNodes) const
{
    return & rNodes == m_xUndoNodes.get();
}

size_t UndoManager::GetUndoActionCount(const bool bCurrentLevel) const
{
    return SdrUndoManager::GetUndoActionCount(bCurrentLevel);
}

void UndoManager::DoUndo(bool const bDoUndo)
{
    if(!isTextEditActive())
    {
        EnableUndo(bDoUndo);

        SwDrawModel*const pSdrModel = m_rDrawModelAccess.GetDrawModel();
        if( pSdrModel )
        {
            pSdrModel->EnableUndo(bDoUndo);
        }
    }
}

bool UndoManager::DoesUndo() const
{
    if(isTextEditActive())
    {
        return false;
    }
    else
    {
        return IsUndoEnabled();
    }
}

void UndoManager::DoGroupUndo(bool const bDoUndo)
{
    m_bGroupUndo = bDoUndo;
}

bool UndoManager::DoesGroupUndo() const
{
    return m_bGroupUndo;
}

void UndoManager::DoDrawUndo(bool const bDoUndo)
{
    m_bDrawUndo = bDoUndo;
}

bool UndoManager::DoesDrawUndo() const
{
    return m_bDrawUndo;
}

bool UndoManager::IsUndoNoResetModified() const
{
    return MARK_INVALID == m_UndoSaveMark;
}

void UndoManager::SetUndoNoResetModified()
{
    if (MARK_INVALID != m_UndoSaveMark)
    {
        RemoveMark(m_UndoSaveMark);
        m_UndoSaveMark = MARK_INVALID;
    }
}

void UndoManager::SetUndoNoModifiedPosition()
{
    if (!m_bLockUndoNoModifiedPosition)
    {
        m_UndoSaveMark = MarkTopUndoAction();
    }
}

void UndoManager::LockUndoNoModifiedPosition()
{
    m_bLockUndoNoModifiedPosition = true;
}

void UndoManager::UnLockUndoNoModifiedPosition()
{
    m_bLockUndoNoModifiedPosition = false;
}

SwUndo* UndoManager::GetLastUndo()
{
    if (!SdrUndoManager::GetUndoActionCount())
    {
        return nullptr;
    }
    SfxUndoAction *const pAction( SdrUndoManager::GetUndoAction() );
    return dynamic_cast<SwUndo*>(pAction);
}

void UndoManager::AppendUndo(SwUndo *const pUndo)
{
    AddUndoAction(pUndo);
}

void UndoManager::ClearRedo()
{
    return SdrUndoManager::ImplClearRedo_NoLock(TopLevel);
}

void UndoManager::DelAllUndoObj()
{
    ::sw::UndoGuard const undoGuard(*this);

    SdrUndoManager::ClearAllLevels();

    m_UndoSaveMark = MARK_INVALID;
}

SwUndoId
UndoManager::StartUndo(SwUndoId const i_eUndoId,
        SwRewriter const*const pRewriter)
{
    if (!IsUndoEnabled())
    {
        return UNDO_EMPTY;
    }

    SwUndoId const eUndoId( (i_eUndoId == UNDO_EMPTY) ? UNDO_START : i_eUndoId );

    OSL_ASSERT(UNDO_END != eUndoId);
    OUString comment( (UNDO_START == eUndoId)
        ?   OUString("??")
        :   OUString(SW_RES(UNDO_BASE + eUndoId)) );
    if (pRewriter)
    {
        OSL_ASSERT(UNDO_START != eUndoId);
        comment = pRewriter->Apply(comment);
    }

    SdrUndoManager::EnterListAction(comment, comment, eUndoId);

    return eUndoId;
}

SwUndoId
UndoManager::EndUndo(SwUndoId const i_eUndoId, SwRewriter const*const pRewriter)
{
    if (!IsUndoEnabled())
    {
        return UNDO_EMPTY;
    }

    SwUndoId const eUndoId( ((i_eUndoId == UNDO_EMPTY) || (UNDO_START == i_eUndoId))
            ? UNDO_END : i_eUndoId );
    OSL_ENSURE(!((UNDO_END == eUndoId) && pRewriter),
                "EndUndo(): no Undo ID, but rewriter given?");

    SfxUndoAction *const pLastUndo(
        (0 == SdrUndoManager::GetUndoActionCount())
            ? nullptr : SdrUndoManager::GetUndoAction() );

    int const nCount = LeaveListAction();

    if (nCount) // otherwise: empty list action not inserted!
    {
        OSL_ASSERT(pLastUndo);
        OSL_ASSERT(UNDO_START != eUndoId);
        SfxUndoAction *const pUndoAction(SdrUndoManager::GetUndoAction());
        SfxListUndoAction *const pListAction(
            dynamic_cast<SfxListUndoAction*>(pUndoAction));
        OSL_ASSERT(pListAction);
        if (pListAction)
        {
            if (UNDO_END != eUndoId)
            {
                OSL_ENSURE(pListAction->GetId() == eUndoId,
                        "EndUndo(): given ID different from StartUndo()");
                // comment set by caller of EndUndo
                OUString comment = SW_RES(UNDO_BASE + eUndoId);
                if (pRewriter)
                {
                    comment = pRewriter->Apply(comment);
                }
                pListAction->SetComment(comment);
            }
            else if ((UNDO_START != pListAction->GetId()))
            {
                // comment set by caller of StartUndo: nothing to do here
            }
            else if (pLastUndo)
            {
                // comment was not set at StartUndo or EndUndo:
                // take comment of last contained action
                // (note that this works recursively, i.e. the last contained
                // action may be a list action created by StartUndo/EndUndo)
                OUString const comment(pLastUndo->GetComment());
                pListAction->SetComment(comment);
            }
            else
            {
                OSL_ENSURE(false, "EndUndo(): no comment?");
            }
        }
    }

    return eUndoId;
}

bool
UndoManager::GetLastUndoInfo(
        OUString *const o_pStr, SwUndoId *const o_pId) const
{
    // this is actually expected to work on the current level,
    // but that was really not obvious from the previous implementation...
    if (!SdrUndoManager::GetUndoActionCount())
    {
        return false;
    }

    SfxUndoAction *const pAction( SdrUndoManager::GetUndoAction() );

    if (o_pStr)
    {
        *o_pStr = pAction->GetComment();
    }
    if (o_pId)
    {
        sal_uInt16 const nId(pAction->GetId());
        *o_pId = static_cast<SwUndoId>(nId);
    }

    return true;
}

SwUndoComments_t UndoManager::GetUndoComments() const
{
    OSL_ENSURE(!SdrUndoManager::IsInListAction(),
            "GetUndoComments() called while in list action?");

    SwUndoComments_t ret;
    const size_t nUndoCount(SdrUndoManager::GetUndoActionCount(TopLevel));
    for (size_t n = 0; n < nUndoCount; ++n)
    {
        OUString const comment(
                SdrUndoManager::GetUndoActionComment(n, TopLevel));
        ret.push_back(comment);
    }

    return ret;
}

bool UndoManager::GetFirstRedoInfo(OUString *const o_pStr,
                                   SwUndoId *const o_pId) const
{
    if (!SdrUndoManager::GetRedoActionCount())
    {
        return false;
    }

    SfxUndoAction *const pAction( SdrUndoManager::GetRedoAction() );
    if ( pAction == nullptr )
    {
        return false;
    }

    if (o_pStr)
    {
        *o_pStr = pAction->GetComment();
    }
    if (o_pId)
    {
        sal_uInt16 const nId(pAction->GetId());
        *o_pId = static_cast<SwUndoId>(nId);
    }

    return true;
}

SwUndoComments_t UndoManager::GetRedoComments() const
{
    OSL_ENSURE(!SdrUndoManager::IsInListAction(),
            "GetRedoComments() called while in list action?");

    SwUndoComments_t ret;
    const size_t nRedoCount(SdrUndoManager::GetRedoActionCount(TopLevel));
    for (size_t n = 0; n < nRedoCount; ++n)
    {
        OUString const comment(
                SdrUndoManager::GetRedoActionComment(n, TopLevel));
        ret.push_back(comment);
    }

    return ret;
}

SwUndoId UndoManager::GetRepeatInfo(OUString *const o_pStr) const
{
    SwUndoId nRepeatId(UNDO_EMPTY);
    GetLastUndoInfo(o_pStr, & nRepeatId);
    if( REPEAT_START <= nRepeatId && REPEAT_END > nRepeatId )
    {
        return nRepeatId;
    }
    if (o_pStr) // not repeatable -> clear comment
    {
        o_pStr->clear();
    }
    return UNDO_EMPTY;
}

SwUndo * UndoManager::RemoveLastUndo()
{
    if (SdrUndoManager::GetRedoActionCount() ||
        SdrUndoManager::GetRedoActionCount(TopLevel))
    {
        OSL_ENSURE(false, "RemoveLastUndoAction(): there are Redo actions?");
        return nullptr;
    }
    if (!SdrUndoManager::GetUndoActionCount())
    {
        OSL_ENSURE(false, "RemoveLastUndoAction(): no Undo actions");
        return nullptr;
    }
    SfxUndoAction *const pLastUndo(GetUndoAction());
    SdrUndoManager::RemoveLastUndoAction();
    return dynamic_cast<SwUndo *>(pLastUndo);
}

// svl::IUndoManager

void UndoManager::EnableUndo(bool bEnable)
{
    // SdrUndoManager does not have a counter anymore, but reverted to the old behavior of
    // having a simple boolean flag for locking. So, simply forward.
    SdrUndoManager::EnableUndo(bEnable);
}

void UndoManager::AddUndoAction(SfxUndoAction *pAction, bool bTryMerge)
{
    SwUndo *const pUndo( dynamic_cast<SwUndo *>(pAction) );
    if (pUndo)
    {
        if (nsRedlineMode_t::REDLINE_NONE == pUndo->GetRedlineMode())
        {
            pUndo->SetRedlineMode( m_rRedlineAccess.GetRedlineMode() );
        }
    }
    SdrUndoManager::AddUndoAction(pAction, bTryMerge);
    // if the undo nodes array is too large, delete some actions
    while (UNDO_ACTION_LIMIT < GetUndoNodes().Count())
    {
        RemoveOldestUndoActions(1);
    }
}

class CursorGuard
{
public:
    CursorGuard(SwEditShell & rShell, bool const bSave)
        : m_rShell(rShell)
        , m_bSaveCursor(bSave)
    {
        if (m_bSaveCursor)
        {
            m_rShell.Push(); // prevent modification of current cursor
        }
    }
    ~CursorGuard()
    {
        if (m_bSaveCursor)
        {
            m_rShell.Pop( false );
        }
    }
private:
    SwEditShell & m_rShell;
    bool const m_bSaveCursor;
};

bool UndoManager::impl_DoUndoRedo(UndoOrRedo_t const undoOrRedo)
{
    SwDoc & rDoc(*GetUndoNodes().GetDoc());

    UnoActionContext c(& rDoc); // exception-safe StartAllAction/EndAllAction

    SwEditShell *const pEditShell( rDoc.GetEditShell() );

    OSL_ENSURE(pEditShell, "sw::UndoManager needs a SwEditShell!");
    if (!pEditShell)
    {
        throw uno::RuntimeException();
    }

    // in case the model has controllers locked, the Undo should not
    // change the view cursors!
    bool const bSaveCursors(pEditShell->CursorsLocked());
    CursorGuard aCursorGuard(*pEditShell, bSaveCursors);
    if (!bSaveCursors)
    {
        // (in case Undo was called via API) clear the cursors:
        pEditShell->KillPams();
        pEditShell->SetMark();
        pEditShell->ClearMark();
    }

    bool bRet(false);

    ::sw::UndoRedoContext context(rDoc, *pEditShell);

    // N.B. these may throw!
    if (UNDO == undoOrRedo)
    {
        bRet = SdrUndoManager::UndoWithContext(context);
    }
    else
    {
        bRet = SdrUndoManager::RedoWithContext(context);
    }

    if (bRet)
    {
        // if we are at the "last save" position, the document is not modified
        if (SdrUndoManager::HasTopUndoActionMark(m_UndoSaveMark))
        {
            m_rState.ResetModified();
        }
        else
        {
            m_rState.SetModified();
        }
    }

    pEditShell->HandleUndoRedoContext(context);

    return bRet;
}

bool UndoManager::Undo()
{
    if(isTextEditActive())
    {
        return SdrUndoManager::Undo();
    }
    else
    {
        return impl_DoUndoRedo(UNDO);
    }
}

bool UndoManager::Redo()
{
    if(isTextEditActive())
    {
        return SdrUndoManager::Redo();
    }
    else
    {
        return impl_DoUndoRedo(REDO);
    }
}

/** N.B.: this does _not_ call SdrUndoManager::Repeat because it is not
          possible to wrap a list action around it:
          calling EnterListAction here will cause SdrUndoManager::Repeat
          to repeat the list action!
 */
bool UndoManager::Repeat(::sw::RepeatContext & rContext,
                         sal_uInt16 const nRepeatCount)
{
    if (SdrUndoManager::IsInListAction())
    {
        OSL_ENSURE(false, "repeat in open list action???");
        return false;
    }
    if (!SdrUndoManager::GetUndoActionCount(TopLevel))
    {
        return false;
    }
    SfxUndoAction *const pRepeatAction(GetUndoAction());
    OSL_ASSERT(pRepeatAction);
    if (!pRepeatAction || !pRepeatAction->CanRepeat(rContext))
    {
        return false;
    }

    OUString const comment(pRepeatAction->GetComment());
    OUString const rcomment(pRepeatAction->GetRepeatComment(rContext));
    sal_uInt16 const nId(pRepeatAction->GetId());
    if (DoesUndo())
    {
        EnterListAction(comment, rcomment, nId);
    }

    SwPaM* pTmp = rContext.m_pCurrentPaM;
    for(SwPaM& rPaM : rContext.GetRepeatPaM().GetRingContainer())
    {    // iterate over ring
        rContext.m_pCurrentPaM = &rPaM;
        for (sal_uInt16 nRptCnt = nRepeatCount; nRptCnt > 0; --nRptCnt)
        {
            pRepeatAction->Repeat(rContext);
        }
        rContext.m_bDeleteRepeated = false; // reset for next PaM
    }
    rContext.m_pCurrentPaM = pTmp;

    if (DoesUndo())
    {
        LeaveListAction();
    }
    return true;
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
