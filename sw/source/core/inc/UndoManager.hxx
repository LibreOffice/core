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

#ifndef SW_UNDO_MANAGER_HXX
#define SW_UNDO_MANAGER_HXX

#include <IDocumentUndoRedo.hxx>

#include <memory>

#include <svl/undo.hxx>


class IDocumentDrawModelAccess;
class IDocumentRedlineAccess;
class IDocumentState;


namespace sw {

class UndoManager
    : public IDocumentUndoRedo
    , public SfxUndoManager
{
public:

    UndoManager(::std::auto_ptr<SwNodes> pUndoNodes,
        IDocumentDrawModelAccess & rDrawModelAccess,
        IDocumentRedlineAccess & rRedlineAccess,
        IDocumentState & rState);

    /** IDocumentUndoRedo */
    virtual void DoUndo(bool const bDoUndo);
    virtual bool DoesUndo() const;
    virtual void DoGroupUndo(bool const bDoUndo);
    virtual bool DoesGroupUndo() const;
    virtual void DoDrawUndo(bool const bDoUndo);
    virtual bool DoesDrawUndo() const;
    virtual void SetUndoNoModifiedPosition();
    virtual void LockUndoNoModifiedPosition();
    virtual void UnLockUndoNoModifiedPosition();
    virtual void SetUndoNoResetModified();
    virtual bool IsUndoNoResetModified() const;
//    virtual bool Undo();
    virtual SwUndoId StartUndo(SwUndoId const eUndoId,
                SwRewriter const*const pRewriter);
    virtual SwUndoId EndUndo(SwUndoId const eUndoId,
                SwRewriter const*const pRewriter);
    virtual void DelAllUndoObj();
    virtual bool GetLastUndoInfo(::rtl::OUString *const o_pStr,
                SwUndoId *const o_pId) const;
    virtual SwUndoComments_t GetUndoComments() const;
//    virtual bool Redo();
    virtual bool GetFirstRedoInfo(::rtl::OUString *const o_pStr) const;
    virtual SwUndoComments_t GetRedoComments() const;
    virtual bool Repeat(::sw::RepeatContext & rContext,
                sal_uInt16 const nRepeatCnt);
    virtual SwUndoId GetRepeatInfo(::rtl::OUString *const o_pStr) const;
    virtual void AppendUndo(SwUndo *const pUndo);
    virtual void ClearRedo();
    virtual bool IsUndoNodes(SwNodes const& rNodes) const;

    // ::svl::IUndoManager
    virtual void     AddUndoAction(SfxUndoAction *pAction,
                                   sal_Bool bTryMerg = sal_False);
    virtual sal_Bool Undo();
    virtual sal_Bool Redo();
    virtual void     EnableUndo(bool bEnable);

    SwUndo * RemoveLastUndo();
    SwUndo * GetLastUndo();

    SwNodes const& GetUndoNodes() const;
    SwNodes      & GetUndoNodes();

private:
    IDocumentDrawModelAccess & m_rDrawModelAccess;
    IDocumentRedlineAccess & m_rRedlineAccess;
    IDocumentState & m_rState;

    /// Undo nodes array: content not currently in document
    ::std::auto_ptr<SwNodes> m_pUndoNodes;

    bool m_bGroupUndo       : 1;    // TRUE: Undo grouping enabled
    bool m_bDrawUndo        : 1;    // TRUE: Draw Undo enabled
    bool m_bLockUndoNoModifiedPosition : 1;
    /// position in Undo-Array at which Doc was saved (and is not modified)
    UndoStackMark m_UndoSaveMark;

    typedef enum { UNDO = true, REDO = false } UndoOrRedo_t;
    bool impl_DoUndoRedo(UndoOrRedo_t const undoOrRedo);

    // UGLY: should not be called
    using SfxUndoManager::Repeat;
};

} // namespace sw

#endif // SW_UNDO_MANAGER_HXX

