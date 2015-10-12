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
    virtual void DoUndo(bool const bDoUndo) override;
    virtual bool DoesUndo() const override;
    virtual void DoGroupUndo(bool const bDoUndo) override;
    virtual bool DoesGroupUndo() const override;
    virtual void DoDrawUndo(bool const bDoUndo) override;
    virtual bool DoesDrawUndo() const override;
    virtual void SetUndoNoModifiedPosition() override;
    virtual void LockUndoNoModifiedPosition() override;
    virtual void UnLockUndoNoModifiedPosition() override;
    virtual void SetUndoNoResetModified() override;
    virtual bool IsUndoNoResetModified() const override;

    virtual SwUndoId StartUndo(SwUndoId const eUndoId,
                               SwRewriter const*const pRewriter) override;
    virtual SwUndoId EndUndo(SwUndoId const eUndoId,
                             SwRewriter const*const pRewriter) override;
    virtual void DelAllUndoObj() override;
    virtual bool GetLastUndoInfo(OUString *const o_pStr,
                                 SwUndoId *const o_pId) const override;
    virtual SwUndoComments_t GetUndoComments() const override;
    virtual bool GetFirstRedoInfo(OUString *const o_pStr,
                                  SwUndoId *const o_pId = 0) const override;
    virtual SwUndoComments_t GetRedoComments() const override;
    virtual bool Repeat(::sw::RepeatContext & rContext,
                        sal_uInt16 const nRepeatCnt) override;
    virtual SwUndoId GetRepeatInfo(OUString *const o_pStr) const override;
    virtual void AppendUndo(SwUndo *const pUndo) override;
    virtual void ClearRedo() override;
    virtual bool IsUndoNodes(SwNodes const& rNodes) const override;
    virtual size_t GetUndoActionCount(const bool bCurrentLevel = true) const override;

    // ::svl::IUndoManager
    virtual void AddUndoAction(SfxUndoAction *pAction,
                                   bool bTryMerg = false) override;
    virtual bool Undo() override;
    virtual bool Redo() override;
    virtual void EnableUndo(bool bEnable) override;

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
