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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOMANAGER_HXX

#include <IDocumentUndoRedo.hxx>
#include <svx/sdrundomanager.hxx>
#include <ndarr.hxx>
#include <memory>

class IDocumentDrawModelAccess;
class IDocumentRedlineAccess;
class IDocumentState;

namespace sw {

class UndoManager
    : public IDocumentUndoRedo
    , public SdrUndoManager
{
public:
    UndoManager(std::shared_ptr<SwNodes> pUndoNodes,
        IDocumentDrawModelAccess & rDrawModelAccess,
        IDocumentRedlineAccess & rRedlineAccess,
        IDocumentState & rState);

    /** IDocumentUndoRedo */
    virtual void DoUndo(bool const bDoUndo) SAL_OVERRIDE;
    virtual bool DoesUndo() const SAL_OVERRIDE;
    virtual void DoGroupUndo(bool const bDoUndo) SAL_OVERRIDE;
    virtual bool DoesGroupUndo() const SAL_OVERRIDE;
    virtual void DoDrawUndo(bool const bDoUndo) SAL_OVERRIDE;
    virtual bool DoesDrawUndo() const SAL_OVERRIDE;
    virtual void SetUndoNoModifiedPosition() SAL_OVERRIDE;
    virtual void LockUndoNoModifiedPosition() SAL_OVERRIDE;
    virtual void UnLockUndoNoModifiedPosition() SAL_OVERRIDE;
    virtual void SetUndoNoResetModified() SAL_OVERRIDE;
    virtual bool IsUndoNoResetModified() const SAL_OVERRIDE;

    virtual SwUndoId StartUndo(SwUndoId const eUndoId,
                               SwRewriter const*const pRewriter) SAL_OVERRIDE;
    virtual SwUndoId EndUndo(SwUndoId const eUndoId,
                             SwRewriter const*const pRewriter) SAL_OVERRIDE;
    virtual void DelAllUndoObj() SAL_OVERRIDE;
    virtual bool GetLastUndoInfo(OUString *const o_pStr,
                                 SwUndoId *const o_pId) const SAL_OVERRIDE;
    virtual SwUndoComments_t GetUndoComments() const SAL_OVERRIDE;
    virtual bool GetFirstRedoInfo(OUString *const o_pStr,
                                  SwUndoId *const o_pId = 0) const SAL_OVERRIDE;
    virtual SwUndoComments_t GetRedoComments() const SAL_OVERRIDE;
    virtual bool Repeat(::sw::RepeatContext & rContext,
                        sal_uInt16 const nRepeatCnt) SAL_OVERRIDE;
    virtual SwUndoId GetRepeatInfo(OUString *const o_pStr) const SAL_OVERRIDE;
    virtual void AppendUndo(SwUndo *const pUndo) SAL_OVERRIDE;
    virtual void ClearRedo() SAL_OVERRIDE;
    virtual bool IsUndoNodes(SwNodes const& rNodes) const SAL_OVERRIDE;
    virtual size_t GetUndoActionCount(const bool bCurrentLevel = true) const SAL_OVERRIDE;

    // ::svl::IUndoManager
    virtual void AddUndoAction(SfxUndoAction *pAction,
                                   bool bTryMerg = false) SAL_OVERRIDE;
    virtual bool Undo() SAL_OVERRIDE;
    virtual bool Redo() SAL_OVERRIDE;
    virtual void EnableUndo(bool bEnable) SAL_OVERRIDE;

    SwUndo * RemoveLastUndo();
    SwUndo * GetLastUndo();

    SwNodes const& GetUndoNodes() const;
    SwNodes      & GetUndoNodes();

private:
    IDocumentDrawModelAccess & m_rDrawModelAccess;
    IDocumentRedlineAccess & m_rRedlineAccess;
    IDocumentState & m_rState;

    /// Undo nodes array: content not currently in document
    std::shared_ptr<SwNodes> m_xUndoNodes;

    bool m_bGroupUndo       : 1;    // TRUE: Undo grouping enabled
    bool m_bDrawUndo        : 1;    // TRUE: Draw Undo enabled
    bool m_bLockUndoNoModifiedPosition : 1;
    /// position in Undo-Array at which Doc was saved (and is not modified)
    UndoStackMark m_UndoSaveMark;

    typedef enum { UNDO = int(true), REDO = int(false) } UndoOrRedo_t;
    bool impl_DoUndoRedo(UndoOrRedo_t const undoOrRedo);

    // UGLY: should not be called
    using SdrUndoManager::Repeat;
};

} // namespace sw

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
