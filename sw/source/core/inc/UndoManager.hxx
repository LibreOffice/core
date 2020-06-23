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
class SwDocShell;
class SwView;

namespace sw {

class SAL_DLLPUBLIC_RTTI UndoManager
    : public IDocumentUndoRedo
    , public SdrUndoManager
{
public:
    UndoManager(std::shared_ptr<SwNodes> const & pUndoNodes,
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
    void DoRepair(bool bRepair) override;
    bool DoesRepair() const override;
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
                                 SwUndoId *const o_pId,
                                 const SwView* pView = nullptr) const override;
    virtual SwUndoComments_t GetUndoComments() const override;
    virtual bool GetFirstRedoInfo(OUString *const o_pStr,
                                  SwUndoId *const o_pId,
                                  const SwView* pView = nullptr) const override;
    virtual SwUndoComments_t GetRedoComments() const override;
    virtual bool Repeat(::sw::RepeatContext & rContext,
                        sal_uInt16 const nRepeatCnt) override;
    virtual SwUndoId GetRepeatInfo(OUString *const o_pStr) const override;
    virtual void AppendUndo(std::unique_ptr<SwUndo> pUndo) override;
    virtual void ClearRedo() override;
    virtual bool IsUndoNodes(SwNodes const& rNodes) const override;
    virtual size_t GetUndoActionCount(const bool bCurrentLevel = true) const override;
    size_t GetRedoActionCount(const bool bCurrentLevel = true) const override;
    void SetView(SwView* pView) override;

    // SfxUndoManager
    virtual void AddUndoAction(std::unique_ptr<SfxUndoAction> pAction,
                                   bool bTryMerg = false) override;
    virtual bool Undo() override;
    virtual bool Redo() override;
    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    SwUndo * RemoveLastUndo();
    SwUndo * GetLastUndo();

    SwNodes const& GetUndoNodes() const;
    SwNodes      & GetUndoNodes();
    void SetDocShell(SwDocShell* pDocShell);

protected:
    virtual void EmptyActionsChanged() override;

private:
    IDocumentDrawModelAccess & m_rDrawModelAccess;
    IDocumentRedlineAccess & m_rRedlineAccess;
    IDocumentState & m_rState;

    /// Undo nodes array: content not currently in document
    std::shared_ptr<SwNodes> m_xUndoNodes;

    bool m_bGroupUndo       : 1;    // TRUE: Undo grouping enabled
    bool m_bDrawUndo        : 1;    // TRUE: Draw Undo enabled
    /// If true, then repair mode is enabled.
    bool m_bRepair;
    bool m_bLockUndoNoModifiedPosition : 1;
    /// set the IgnoreRepeat flag on every added action
    bool m_isAddWithIgnoreRepeat;
    /// position in Undo-Array at which Doc was saved (and is not modified)
    UndoStackMark m_UndoSaveMark;
    SwDocShell* m_pDocShell;
    SwView* m_pView;

    enum class UndoOrRedoType { Undo, Redo };
    bool impl_DoUndoRedo(UndoOrRedoType undoOrRedo);

    // UGLY: should not be called
    using SdrUndoManager::Repeat;
};

} // namespace sw

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
