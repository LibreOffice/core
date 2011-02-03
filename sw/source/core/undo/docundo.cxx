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

#include <UndoManager.hxx>

#include <unotools/undoopt.hxx>

#include <vcl/wrkwin.hxx>

#include <svx/svdmodel.hxx>

#include <swmodule.hxx>
#include <doc.hxx>
#include <ndarr.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <undo.hrc>
#include <editsh.hxx>
#include <unobaseclass.hxx>


using namespace ::com::sun::star;


// the undo array should never grow beyond this limit:
#define UNDO_ACTION_LIMIT (USHRT_MAX - 1000)


// UndoManager ///////////////////////////////////////////////////////////

namespace sw {

UndoManager::UndoManager(::std::auto_ptr<SwNodes> pUndoNodes,
            IDocumentDrawModelAccess & rDrawModelAccess,
            IDocumentRedlineAccess & rRedlineAccess,
            IDocumentState & rState)
    :   m_rDrawModelAccess(rDrawModelAccess)
    ,   m_rRedlineAccess(rRedlineAccess)
    ,   m_rState(rState)
    ,   m_pUndoNodes(pUndoNodes)
    ,   m_bGroupUndo(true)
    ,   m_bDrawUndo(true)
    ,   m_bLockUndoNoModifiedPosition(false)
    ,   m_UndoSaveMark(MARK_INVALID)
{
    OSL_ASSERT(m_pUndoNodes.get());
    // writer expects it to be disabled initially
    // Undo is enabled by SwEditShell constructor
    SfxUndoManager::EnableUndo(false);
}

SwNodes const& UndoManager::GetUndoNodes() const
{
    return *m_pUndoNodes;
}

SwNodes      & UndoManager::GetUndoNodes()
{
    return *m_pUndoNodes;
}

bool UndoManager::IsUndoNodes(SwNodes const& rNodes) const
{
    return & rNodes == m_pUndoNodes.get();
}

void UndoManager::DoUndo(bool const bDoUndo)
{
    EnableUndo(bDoUndo);

    SdrModel *const pSdrModel = m_rDrawModelAccess.GetDrawModel();
    if( pSdrModel )
    {
        pSdrModel->EnableUndo(bDoUndo);
    }
}

bool UndoManager::DoesUndo() const
{
    return IsUndoEnabled();
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
    if (!SfxUndoManager::GetUndoActionCount(CurrentLevel))
    {
        return 0;
    }
    SfxUndoAction *const pAction( SfxUndoManager::GetUndoAction(0) );
    return dynamic_cast<SwUndo*>(pAction);
}

void UndoManager::AppendUndo(SwUndo *const pUndo)
{
    AddUndoAction(pUndo);
}

void UndoManager::ClearRedo()
{
    return SfxUndoManager::ImplClearRedo_NoLock(TopLevel);
}

void UndoManager::DelAllUndoObj()
{
    ::sw::UndoGuard const undoGuard(*this);

    SfxUndoManager::ClearAllLevels();

    m_UndoSaveMark = MARK_INVALID;
}


/**************** UNDO ******************/

SwUndoId
UndoManager::StartUndo(SwUndoId const i_eUndoId,
        SwRewriter const*const pRewriter)
{
    if (!IsUndoEnabled())
    {
        return UNDO_EMPTY;
    }

    SwUndoId const eUndoId( (0 == i_eUndoId) ? UNDO_START : i_eUndoId );

    OSL_ASSERT(UNDO_END != eUndoId);
    String comment( (UNDO_START == eUndoId)
        ?   String("??", RTL_TEXTENCODING_ASCII_US)
        :   String(SW_RES(UNDO_BASE + eUndoId)) );
    if (pRewriter)
    {
        OSL_ASSERT(UNDO_START != eUndoId);
        comment = pRewriter->Apply(comment);
    }

    SfxUndoManager::EnterListAction(comment, comment, eUndoId);

    return eUndoId;
}


SwUndoId
UndoManager::EndUndo(SwUndoId const i_eUndoId, SwRewriter const*const pRewriter)
{
    if (!IsUndoEnabled())
    {
        return UNDO_EMPTY;
    }

    SwUndoId const eUndoId( ((0 == i_eUndoId) || (UNDO_START == i_eUndoId))
            ? UNDO_END : i_eUndoId );
    OSL_ENSURE(!((UNDO_END == eUndoId) && pRewriter),
                "EndUndo(): no Undo ID, but rewriter given?");

    SfxUndoAction *const pLastUndo(
        (0 == SfxUndoManager::GetUndoActionCount(CurrentLevel))
            ? 0 : SfxUndoManager::GetUndoAction(0) );

    int const nCount = LeaveListAction();

    if (nCount) // otherwise: empty list action not inserted!
    {
        OSL_ASSERT(pLastUndo);
        OSL_ASSERT(UNDO_START != eUndoId);
        SfxUndoAction *const pUndoAction(SfxUndoManager::GetUndoAction(0));
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
                String comment = String(SW_RES(UNDO_BASE + eUndoId));
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
                String const comment(pLastUndo->GetComment());
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
        ::rtl::OUString *const o_pStr, SwUndoId *const o_pId) const
{
    // this is actually expected to work on the current level,
    // but that was really not obvious from the previous implementation...
    if (!SfxUndoManager::GetUndoActionCount(CurrentLevel))
    {
        return false;
    }

    SfxUndoAction *const pAction( SfxUndoManager::GetUndoAction(0) );

    if (o_pStr)
    {
        *o_pStr = pAction->GetComment();
    }
    if (o_pId)
    {
        USHORT const nId(pAction->GetId());
        *o_pId = static_cast<SwUndoId>(nId);
    }

    return true;
}

SwUndoComments_t UndoManager::GetUndoComments() const
{
    OSL_ENSURE(!SfxUndoManager::IsInListAction(),
            "GetUndoComments() called while in list action?");

    SwUndoComments_t ret;
    USHORT const nUndoCount(SfxUndoManager::GetUndoActionCount(TopLevel));
    for (USHORT n = 0; n < nUndoCount; ++n)
    {
        ::rtl::OUString const comment(
                SfxUndoManager::GetUndoActionComment(n, TopLevel));
        ret.push_back(comment);
    }

    return ret;
}


/**************** REDO ******************/

bool UndoManager::GetFirstRedoInfo(::rtl::OUString *const o_pStr) const
{
    if (!SfxUndoManager::GetRedoActionCount(CurrentLevel))
    {
        return false;
    }

    if (o_pStr)
    {
        *o_pStr = SfxUndoManager::GetRedoActionComment(0, CurrentLevel);
    }

    return true;
}


SwUndoComments_t UndoManager::GetRedoComments() const
{
    OSL_ENSURE(!SfxUndoManager::IsInListAction(),
            "GetRedoComments() called while in list action?");

    SwUndoComments_t ret;
    USHORT const nRedoCount(SfxUndoManager::GetRedoActionCount(TopLevel));
    for (USHORT n = 0; n < nRedoCount; ++n)
    {
        ::rtl::OUString const comment(
                SfxUndoManager::GetRedoActionComment(n, TopLevel));
        ret.push_back(comment);
    }

    return ret;
}

/**************** REPEAT ******************/

SwUndoId UndoManager::GetRepeatInfo(::rtl::OUString *const o_pStr) const
{
    SwUndoId nRepeatId(UNDO_EMPTY);
    GetLastUndoInfo(o_pStr, & nRepeatId);
    if( REPEAT_START <= nRepeatId && REPEAT_END > nRepeatId )
    {
        return nRepeatId;
    }
    if (o_pStr) // not repeatable -> clear comment
    {
        *o_pStr = String();
    }
    return UNDO_EMPTY;
}

SwUndo * UndoManager::RemoveLastUndo()
{
    if (SfxUndoManager::GetRedoActionCount(CurrentLevel) ||
        SfxUndoManager::GetRedoActionCount(TopLevel))
    {
        OSL_ENSURE(false, "RemoveLastUndoAction(): there are Redo actions?");
        return 0;
    }
    if (!SfxUndoManager::GetUndoActionCount(CurrentLevel))
    {
        OSL_ENSURE(false, "RemoveLastUndoAction(): no Undo actions");
        return 0;
    }
    SfxUndoAction *const pLastUndo(GetUndoAction(0));
    SfxUndoManager::RemoveLastUndoAction();
    return dynamic_cast<SwUndo *>(pLastUndo);
}

// svl::IUndoManager /////////////////////////////////////////////////////

void UndoManager::EnableUndo(bool bEnable)
{
    // UGLY: SfxUndoManager has a counter to match enable/disable calls
    //       but the writer code expects that a single call switches
    while (IsUndoEnabled() != bEnable)
    {
        SfxUndoManager::EnableUndo(bEnable);
    }
}

void UndoManager::AddUndoAction(SfxUndoAction *pAction, sal_Bool bTryMerge)
{
    SwUndo *const pUndo( dynamic_cast<SwUndo *>(pAction) );
    if (pUndo)
    {
        if (nsRedlineMode_t::REDLINE_NONE == pUndo->GetRedlineMode())
        {
            pUndo->SetRedlineMode( m_rRedlineAccess.GetRedlineMode() );
        }
    }
    SfxUndoManager::AddUndoAction(pAction, bTryMerge);
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
            m_rShell.Pop();
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
    CursorGuard(*pEditShell, bSaveCursors);
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
        bRet = SfxUndoManager::UndoWithContext(context);
    }
    else
    {
        bRet = SfxUndoManager::RedoWithContext(context);
    }

    if (bRet)
    {
        // if we are at the "last save" position, the document is not modified
        if (SfxUndoManager::HasTopUndoActionMark(m_UndoSaveMark))
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

sal_Bool UndoManager::Undo()
{
    bool const bRet = impl_DoUndoRedo(UNDO);
    return bRet;
}

sal_Bool UndoManager::Redo()
{
    bool const bRet = impl_DoUndoRedo(REDO);
    return bRet;
}

/** N.B.: this does _not_ call SfxUndoManager::Repeat because it is not
          possible to wrap a list action around it:
          calling EnterListAction here will cause SfxUndoManager::Repeat
          to repeat the list action!
 */
bool
UndoManager::Repeat(::sw::RepeatContext & rContext,
        sal_uInt16 const nRepeatCount)
{
    if (SfxUndoManager::IsInListAction())
    {
        OSL_ENSURE(false, "repeat in open list action???");
        return false;
    }
    if (!SfxUndoManager::GetUndoActionCount(TopLevel))
    {
        return false;
    }
    SfxUndoAction *const pRepeatAction(GetUndoAction(0));
    OSL_ASSERT(pRepeatAction);
    if (!pRepeatAction || !pRepeatAction->CanRepeat(rContext))
    {
        return false;
    }

    ::rtl::OUString const comment(pRepeatAction->GetComment());
    ::rtl::OUString const rcomment(pRepeatAction->GetRepeatComment(rContext));
    USHORT const nId(pRepeatAction->GetId());
    if (DoesUndo())
    {
        EnterListAction(comment, rcomment, nId);
    }

    SwPaM *const pFirstCursor(& rContext.GetRepeatPaM());
    do {    // iterate over ring
        for (USHORT nRptCnt = nRepeatCount; nRptCnt > 0; --nRptCnt)
        {
            pRepeatAction->Repeat(rContext);
        }
        rContext.m_bDeleteRepeated = false; // reset for next PaM
        rContext.m_pCurrentPaM =
            static_cast<SwPaM*>(rContext.m_pCurrentPaM->GetNext());
    } while (pFirstCursor != & rContext.GetRepeatPaM());

    if (DoesUndo())
    {
        LeaveListAction();
    }
    return true;
}

} // namespace sw

