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

#ifndef INCLUDED_SW_INC_IDOCUMENTUNDOREDO_HXX
#define INCLUDED_SW_INC_IDOCUMENTUNDOREDO_HXX

#include <sal/types.h>

#include <swundo.hxx>

class SwRewriter;
class SwNodes;
class SwUndo;

namespace sw {
    class RepeatContext;
}

class IDocumentUndoRedo
{
public:

    /** Enable/Disable Undo.
    */
    virtual void DoUndo(bool const bDoUndo) = 0;

    /** Is Undo enabled?
    */
    virtual bool DoesUndo() const = 0;

    /** Enable/Disable Group Undo.
        This determines whether successive Insert/Delete/Overwrite
        actions are combined.
    */
    virtual void DoGroupUndo(bool const bDoUndo) = 0;

    /** Is Group Undo enabled?
    */
    virtual bool DoesGroupUndo() const = 0;

    /** Enable/Disable Undo for Drawing objects.
     */
    virtual void DoDrawUndo(bool const bDoUndo) = 0;

    /** Is Undo for Drawing objects enabled?
        for Draw-Undo: writer wants to handle actions on Flys on its own.
     */
    virtual bool DoesDrawUndo() const = 0;

    /** Set the position at which the document is in the "unmodified" state
        to the current position in the Undo stack.
    */
    virtual void SetUndoNoModifiedPosition() = 0;

    /** Prevent updates to the "unmodified" state position
        via SetUndoNoResetModified().
    */
    virtual void LockUndoNoModifiedPosition() = 0;

    /** Allow updates to the "unmodified" state position
        via SetUndoNoResetModified().
    */
    virtual void UnLockUndoNoModifiedPosition() = 0;

    /** Disable (re)setting the document modified flag on Undo/Redo.
    */
    virtual void SetUndoNoResetModified() = 0;

    /** Is setting the document modified flag on Undo/Redo disabled?
    */
    virtual bool IsUndoNoResetModified() const = 0;

    /** Execute Undo.

        @return     true if executing the last Undo action was successful.
    */
    virtual bool Undo() = 0;

    /** Opens undo block.

        @remark     StartUndo() and EndUndo() do nothing if !DoesUndo().

        @param nUndoId        undo ID for the list action
        @param pRewriter      rewriter for comments @see SwUndo::GetComment

        If the given nUndoId is equal to zero an undo object with ID
        UNDO_START will be generated.

        @return the undo ID of the created object
    */
    virtual SwUndoId StartUndo(SwUndoId const eUndoId,
                SwRewriter const*const  pRewriter) = 0;

    /**
       Closes undo block.

       @remark     StartUndo() and EndUndo() do nothing if !DoesUndo().

       @param nUndoId         undo ID for the list action
       @param pRewriter       rewriter for comments @see SwUndo::GetComment

       If the given nUndoId is not UNDO_EMPTY or UNDO_END, the comment of
       the resulting list action will be set via the nUndoId, applying the
       given pRewriter (if not 0).  Otherwise the comment of the resulting
       list action is unchanged if it has an UndoId that is not UNDO_START
       set by StartUndo, and in case the UndoId is UNDO_START the comment
       of the list action defaults to the comment of the last action
       contained in the list action.
    */
    virtual SwUndoId EndUndo(SwUndoId const eUndoId,
                SwRewriter const*const  pRewriter) = 0;

    /**
        Delete all Undo actions.
        Of course Undo will be disabled during deletion.
    */
    virtual void DelAllUndoObj() = 0;

    /** Get Id and comment of last Undo action.
        @param o_pStr       if not 0, receives comment of last Undo action.
        @param o_pId        if not 0, receives Id of last Undo action.
        @return     true if there is a Undo action, false if none
    */
    virtual bool GetLastUndoInfo(OUString *const o_pStr,
                SwUndoId *const o_pId) const = 0;

    /** Get comments of Undo actions.
        @return     comments of all top-level Undo actions.
    */
    virtual SwUndoComments_t GetUndoComments() const = 0;

    /** Execute Redo.

        @return     true if executing the first Redo action was successful.
    */
    virtual bool Redo() = 0;

    /** Get Id and comment of first Redo action.
        @param o_pStr       if not 0, receives comment of first Redo action.
        @param o_pId        if not 0, receives Id of first Redo action.
        @return     true if there is a Redo action, false if none
    */
    virtual bool GetFirstRedoInfo(OUString *const o_pStr,
                                  SwUndoId *const o_pId = nullptr) const = 0;

    /** Get comments of Redo actions.
        @return     comments of all top-level Redo actions.
    */
    virtual SwUndoComments_t GetRedoComments() const = 0;

    /** Repeat the last Undo action.
        @return     true if repeating the last Undo Redo action was attempted.
    */
    virtual bool Repeat(::sw::RepeatContext & rContext,
                sal_uInt16 const nRepeatCnt) = 0;

    /** Get Id and comment of last Undo action, if it is Repeat capable.
        @param o_pStr       if not 0, receives comment of last Undo action
                            if it is Repeat capable.
        @return     Id of last Undo action if it is Repeat capable,
                    or UNDO_EMPTY if there is none or it is not Repeat capable.
    */
    virtual SwUndoId GetRepeatInfo(OUString *const o_pStr) const = 0;

    /** Add new Undo action.
        Takes over ownership of pUndo.
        @remark     calls ClearRedo(), except for UNDO_START/UNDO_END.
        @remark     does nothing if !DoesUndo().
    */
    virtual void AppendUndo(SwUndo *const pUndo) = 0;

    /** Delete all Redo actions.
    */
    virtual void ClearRedo() = 0;

    /* Is the given nodes array the Undo nodes array?
    */
    virtual bool IsUndoNodes(SwNodes const& rNodes) const = 0;

    /** Get the number of Undo actions.
    */
    virtual size_t GetUndoActionCount(const bool bCurrentLevel = true) const = 0;

protected:
    virtual ~IDocumentUndoRedo() {};
};

namespace sw {

class UndoGuard
{
public:

    UndoGuard(IDocumentUndoRedo & rUndoRedo)
        :   m_rUndoRedo(rUndoRedo)
        ,   m_bUndoWasEnabled(rUndoRedo.DoesUndo())
    {
        m_rUndoRedo.DoUndo(false);
    }
    ~UndoGuard()
    {
        m_rUndoRedo.DoUndo(m_bUndoWasEnabled);
    }

    bool UndoWasEnabled() const
    {
        return m_bUndoWasEnabled;
    }

private:
    IDocumentUndoRedo & m_rUndoRedo;
    bool const m_bUndoWasEnabled;
};

class GroupUndoGuard
{
public:

    GroupUndoGuard(IDocumentUndoRedo & rUndoRedo)
        :   m_rUndoRedo(rUndoRedo)
        ,   m_bGroupUndoWasEnabled(rUndoRedo.DoesGroupUndo())
    {
        m_rUndoRedo.DoGroupUndo(false);
    }
    ~GroupUndoGuard()
    {
        m_rUndoRedo.DoGroupUndo(m_bGroupUndoWasEnabled);
    }

private:
    IDocumentUndoRedo & m_rUndoRedo;
    bool const m_bGroupUndoWasEnabled;
};

class DrawUndoGuard
{
public:

    DrawUndoGuard(IDocumentUndoRedo & rUndoRedo)
        :   m_rUndoRedo(rUndoRedo)
        ,   m_bDrawUndoWasEnabled(rUndoRedo.DoesDrawUndo())
    {
        m_rUndoRedo.DoDrawUndo(false);
    }
    ~DrawUndoGuard()
    {
        m_rUndoRedo.DoDrawUndo(m_bDrawUndoWasEnabled);
    }

private:
    IDocumentUndoRedo & m_rUndoRedo;
    bool const m_bDrawUndoWasEnabled;
};

} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
