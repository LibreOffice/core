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

#pragma once

#include <svl/undo.hxx>
#include <address.hxx>
#include "docsh.hxx"

#include <memory>
#include <map>
#include <optional>

class SdrUndoAction;
class ScRefUndoData;
class ScDBData;

class SAL_DLLPUBLIC_RTTI ScSimpleUndo: public SfxUndoAction
{
    ScSimpleUndo(const ScSimpleUndo&) = delete;

public:
    typedef std::map<SCTAB, std::unique_ptr<sc::ColumnSpanSet>> DataSpansType;

                    ScSimpleUndo( ScDocShell* pDocSh );

    virtual bool    Merge( SfxUndoAction *pNextAction ) override;
    /// See SfxUndoAction::GetViewShellId().
    ViewShellId GetViewShellId() const override;

    virtual std::optional<ScRange> getAffectedRange() const { return std::nullopt; }

protected:
    ScDocShell*     pDocShell;
    std::unique_ptr<SfxUndoAction>
                    pDetectiveUndo;
    ViewShellId     mnViewShellId;

    bool            IsPaintLocked() const { return pDocShell->IsPaintLocked(); }

    bool            SetViewMarkData( const ScMarkData& rMarkData );

    void            BeginUndo();
    void            EndUndo();
    void            BeginRedo();
    void            EndRedo();

    void BroadcastChanges( const ScRange& rRange );

    /**
     * Broadcast changes on specified spans.
     *
     * @param rSpans container that specifies all spans whose changes need to
     *               be broadcasted.
     */
    void BroadcastChanges( const DataSpansType& rSpans );

    static void     ShowTable( SCTAB nTab );
    static void     ShowTable( const ScRange& rRange );
};

enum ScBlockUndoMode { SC_UNDO_SIMPLE, SC_UNDO_MANUALHEIGHT, SC_UNDO_AUTOHEIGHT };

class ScBlockUndo: public ScSimpleUndo
{
public:
                    ScBlockUndo( ScDocShell* pDocSh, const ScRange& rRange,
                                 ScBlockUndoMode eBlockMode );
    virtual         ~ScBlockUndo() override;

    virtual std::optional<ScRange> getAffectedRange() const override { return aBlockRange; }

protected:
    ScRange         aBlockRange;
    std::unique_ptr<SdrUndoAction> pDrawUndo;
    ScBlockUndoMode eMode;

    void            BeginUndo();
    void            EndUndo();
//  void            BeginRedo();
    void            EndRedo();

    bool            AdjustHeight();
    void            ShowBlock();
};

class SC_DLLPUBLIC ScMultiBlockUndo: public ScSimpleUndo
{
public:
    ScMultiBlockUndo(ScDocShell* pDocSh, ScRangeList aRanges);
    virtual ~ScMultiBlockUndo() override;

protected:
    ScRangeList     maBlockRanges;
    std::unique_ptr<SdrUndoAction> mpDrawUndo;

    void BeginUndo();
    void EndUndo();
    void EndRedo();

    void ShowBlock();
};

// for functions that act on a database range - takes care of the unnamed database range
// (collected separately, before the undo action, for showing dialogs etc.)

class ScDBFuncUndo: public ScSimpleUndo
{
protected:
    std::unique_ptr<ScDBData> pAutoDBRange;
    ScRange         aOriginalRange;

public:
                    ScDBFuncUndo( ScDocShell* pDocSh, const ScRange& rOriginal );
    virtual         ~ScDBFuncUndo() override;

    void            BeginUndo();
    void            EndUndo();
    void            BeginRedo();
    void            EndRedo();
};

class ScMoveUndo: public ScSimpleUndo               // with references
{
public:
                    ScMoveUndo( ScDocShell* pDocSh,
                                ScDocumentUniquePtr pRefDoc, std::unique_ptr<ScRefUndoData> pRefData );
    virtual         ~ScMoveUndo() override;

protected:
    std::unique_ptr<SdrUndoAction>  pDrawUndo;
    ScDocumentUniquePtr             pRefUndoDoc;
    std::unique_ptr<ScRefUndoData>  pRefUndoData;

    void            BeginUndo();
    void            EndUndo();
//  void            BeginRedo();
//  void            EndRedo();

private:
    void            UndoRef();
};

class ScUndoWrapper: public SfxUndoAction           // for manual merging of actions
{
    std::unique_ptr<SfxUndoAction>  pWrappedUndo;
    ViewShellId                     mnViewShellId;

public:
                            ScUndoWrapper( std::unique_ptr<SfxUndoAction> pUndo );
    virtual                 ~ScUndoWrapper() override;

    SfxUndoAction*          GetWrappedUndo()        { return pWrappedUndo.get(); }
    void                    ForgetWrappedUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;
    virtual void            Repeat(SfxRepeatTarget& rTarget) override;
    virtual bool            CanRepeat(SfxRepeatTarget& rTarget) const override;
    virtual bool            Merge( SfxUndoAction *pNextAction ) override;
    virtual OUString        GetComment() const override;
    virtual OUString        GetRepeatComment(SfxRepeatTarget&) const override;
    /// See SfxUndoAction::GetViewShellId().
    ViewShellId GetViewShellId() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
