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

#ifndef INCLUDED_SC_SOURCE_UI_INC_UNDOBASE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_UNDOBASE_HXX

#include <svl/undo.hxx>
#include <global.hxx>
#include <address.hxx>
#include "docsh.hxx"
#include <columnspanset.hxx>

#include <memory>
#include <map>

class ScDocument;
class ScDocShell;
class SdrUndoAction;
class ScRefUndoData;
class ScDBData;

class ScSimpleUndo: public SfxUndoAction
{
    ScSimpleUndo(const ScSimpleUndo&) = delete;

public:
    typedef std::map<SCTAB, std::unique_ptr<sc::ColumnSpanSet>> DataSpansType;

                    ScSimpleUndo( ScDocShell* pDocSh );
    virtual         ~ScSimpleUndo() override;

    virtual bool    Merge( SfxUndoAction *pNextAction ) override;
    /// See SfxUndoAction::GetViewShellId().
    ViewShellId GetViewShellId() const override;

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

protected:
    ScRange         aBlockRange;
    SdrUndoAction*  pDrawUndo;
    ScBlockUndoMode eMode;

    void            BeginUndo();
    void            EndUndo();
//  void            BeginRedo();
    void            EndRedo();

    bool            AdjustHeight();
    void            ShowBlock();
};

class ScMultiBlockUndo: public ScSimpleUndo
{
public:
    ScMultiBlockUndo(ScDocShell* pDocSh, const ScRangeList& rRanges);
    virtual ~ScMultiBlockUndo() override;

protected:
    ScRangeList     maBlockRanges;
    SdrUndoAction*  mpDrawUndo;

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
    ScDBData*       pAutoDBRange;
    ScRange         aOriginalRange;

public:
                    ScDBFuncUndo( ScDocShell* pDocSh, const ScRange& rOriginal );
    virtual         ~ScDBFuncUndo() override;

    void            BeginUndo();
    void            EndUndo();
    void            BeginRedo();
    void            EndRedo();
};

enum ScMoveUndoMode { SC_UNDO_REFFIRST, SC_UNDO_REFLAST };

class ScMoveUndo: public ScSimpleUndo               // with references
{
public:
                    ScMoveUndo( ScDocShell* pDocSh,
                                ScDocument* pRefDoc, ScRefUndoData* pRefData,
                                ScMoveUndoMode eRefMode );
    virtual         ~ScMoveUndo() override;

protected:
    SdrUndoAction*  pDrawUndo;
    ScDocument*     pRefUndoDoc;
    ScRefUndoData*  pRefUndoData;
    ScMoveUndoMode  eMode;

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
                            ScUndoWrapper( SfxUndoAction* pUndo );
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
