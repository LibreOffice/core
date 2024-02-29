/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <svx/sdrundomanager.hxx>

class SfxViewShell;
class ScSimpleUndo;

class SC_DLLPUBLIC ScUndoManager : public SdrUndoManager
{
public:
    ~ScUndoManager();

    /**
     * Checks if the topmost undo action owned by pView is independent from the topmost action undo
     * action. Sets rOffset to the offset of that independent undo action on success.
     */
    bool IsViewUndoActionIndependent(const SfxViewShell* pView, sal_uInt16& rOffset) const;

    /// Make these public
    using SdrUndoManager::UndoWithContext;
    using SdrUndoManager::RedoWithContext;
    using SdrUndoManager::UndoMark;

private:
    static std::optional<ScRange> getAffectedRangeFromUndo(const SfxUndoAction*);
    static const ScSimpleUndo* getScSimpleUndo(const SfxUndoAction*);
};

class ScUndoRedoContext final : public SfxUndoContext
{
public:
    void SetUndoOffset(size_t nUndoOffset) { m_nUndoOffset = nUndoOffset; }

    size_t GetUndoOffset() override { return m_nUndoOffset; }

private:
    size_t m_nUndoOffset = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
