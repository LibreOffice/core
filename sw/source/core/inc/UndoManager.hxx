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


class SwUndos;
class SwDoc;


namespace sw {

class UndoManager
    : public IDocumentUndoRedo
{
public:
    /// max number of undo actions
    static sal_uInt16 GetUndoActionCount();
    static void SetUndoActionCount(sal_uInt16 nNew);

    UndoManager(SwDoc & rDoc);

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
    virtual bool Undo(SwUndoIter & rUndoIter);
    virtual SwUndoId StartUndo(SwUndoId const eUndoId,
                SwRewriter const*const pRewriter);
    virtual SwUndoId EndUndo(SwUndoId const eUndoId,
                SwRewriter const*const pRewriter);
    virtual void DelAllUndoObj();
    virtual SwUndoId GetUndoIds(String *const o_pStr,
                SwUndoIds *const o_pUndoIds) const;
    virtual SwUndo* RemoveLastUndo(SwUndoId const eUndoId);
    virtual bool HasTooManyUndos() const;
    virtual bool Redo(SwUndoIter & rUndoIter);
    virtual SwUndoId GetRedoIds(String *const o_pStr,
                SwUndoIds *const o_pRedoIds) const;
    virtual bool Repeat(SwUndoIter & rUndoIter, sal_uInt16 const nRepeatCnt);
    virtual SwUndoId GetRepeatIds(String *const o_pStr) const;
    virtual void AppendUndo(SwUndo *const pUndo);
    virtual void ClearRedo();
    virtual bool IsUndoNodes(SwNodes const& rNodes) const;

    SwUndo* GetLastUndo();

    sal_Bool RestoreInvisibleContent();
    SwNodes const& GetUndoNodes() const;
    SwNodes      & GetUndoNodes();

private:
    SwDoc & m_rDoc;

    /// Undo nodes array: content not currently in document
    ::std::auto_ptr<SwNodes> m_pUndoNodes;

    ::std::auto_ptr<SwUndos>    pUndos;         // Undo/Redo History

    sal_uInt16  nUndoPos;       // current Undo-InsertPosition (beyond: Redo)
    sal_uInt16  nUndoSavePos;   // position in Undo-Array at which Doc was saved
    sal_uInt16  nUndoCnt;       // number of Undo/Redo actions
    sal_uInt16  nUndoSttEnd;    // nesting depth: != 0 -> inside StartUndo()

    bool mbUndo             : 1;    // TRUE: Undo enabled
    bool mbGroupUndo        : 1;    // TRUE: Undo grouping enabled
    bool m_bDrawUndo        : 1;    // TRUE: Draw Undo enabled
    bool m_bLockUndoNoModifiedPosition : 1;

    /// delete all undo objects from 0 until nEnd
    bool DelUndoObj(sal_uInt16 nEnd);
    /** Is there an Undo action with the given Id, or a Start/End action
        with the given Id as UserId?
    */
    bool HasUndoId(SwUndoId const eId) const;
    /// max number of Undo actions
//    static sal_uInt16 nUndoActions;
};

} // namespace sw

#endif // SW_UNDO_MANAGER_HXX

