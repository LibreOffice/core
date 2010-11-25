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

#ifndef IDOCUMENTUNDOREDO_HXX_INCLUDED
#define IDOCUMENTUNDOREDO_HXX_INCLUDED

#include <sal/types.h>
#include <swundo.hxx>


class SwUndoIter;
class SwRewriter;
class String;
class SwUndoIds;
class SwNodes;
class SwUndo;


/** IDocumentUndoRedo
*/
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

        @postcondition rUndoIter.pAktPam will contain the affected range.
        @return     true if executing the last Undo action was successful.
    */
    virtual bool Undo(SwUndoIter & rUndoIter) = 0; // -> #111827#

    /** Opens undo block.

        @remark     StartUndo() and EndUndo() do nothing if !DoesUndo().

        @param nUndoId        undo ID for the start object
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

       @param nUndoId         undo ID for the closure object
       @param pRewriter       rewriter for comments @see SwUndo::GetComment

       If the given nUndoId is equal to zero an undo object with ID
       UNDO_START will be generated.

       If pRewriter is not equal to zero the given rewriter will be
       set for the generated closure object and the corresponding
       start object. Otherwise an existent rewriter in the
       corresponding start object will be propagated to the generated
       closure object.
    */
    virtual SwUndoId EndUndo(SwUndoId const eUndoId,
                SwRewriter const*const  pRewriter) = 0;

    /** <- #111827#
        Delete all Undo actions.
        Of course Undo will be disabled during deletion.
    */
    virtual void DelAllUndoObj() = 0;

    /** Get Ids and comments of Undo actions.
        @param o_pStr       if not 0, receives comment of last Undo action.
        @param o_pUndoIds   if not 0, receives SwUndoIdAndName objects for all
                            top-level Undo actions.
        @return     Id of last Undo action, or UNDO_EMPTY if there is none.
    */
    virtual SwUndoId GetUndoIds(String *const o_pStr,
                SwUndoIds *const o_pUndoIds) const = 0;

    virtual SwUndo* RemoveLastUndo(SwUndoId const eUndoId) = 0;

    /** 2002-05-31 dvo, #95884#: To prevent an undo array overflow when
        doing nested undos, undo may have to be disabled. Undo-intensive
        actions (like auto-format) should check this manually.
    */
    virtual bool HasTooManyUndos() const = 0;

    /** Execute Redo.

        @postcondition rUndoIter.pAktPam will contain the affected range.
        @return     true if executing the first Redo action was successful.
    */
    virtual bool Redo(SwUndoIter & rUndoIter) = 0;

    /** Get Ids and comments of Redo actions.
        @param o_pStr       if not 0, receives comment of first Redo action.
        @param o_pUndoIds   if not 0, receives SwUndoIdAndName objects for all
                            top-level Redo actions.
        @return     Id of first Redo action, or UNDO_EMPTY if there is none.
    */
    virtual SwUndoId GetRedoIds(String *const o_pStr,
                SwUndoIds *const o_pRedoIds) const = 0;

    /** Repeat the last Undo action.
        @return     true if repeating the last Undo Redo action was attempted.
    */
    virtual bool Repeat(SwUndoIter & rUndoIter, sal_uInt16 const nRepeatCnt) = 0;

    /** Get Id and comment of last Undo action, if it is Repeat capable.
        @param o_pStr       if not 0, receives comment of last Undo action
                            if it is Repeat capable.
        @return     Id of last Undo action if it is Repeat capable,
                    or UNDO_EMPTY if there is none or it is not Repeat capable.
    */
    virtual SwUndoId GetRepeatIds(String *const o_pStr) const = 0;

    /** Add new Undo action.
        Takes over ownership of pUndo.
        @remark     calls ClearRedo(), except for UNDO_START/UNDO_END.
        @remark     does intentionally not check DoesUndo();
                    that is caller's responsibility.
    */
    virtual void AppendUndo(SwUndo *const pUndo) = 0;

    /** Delete all Redo actions.
    */
    virtual void ClearRedo() = 0;

    /* Is the given nodes array the Undo nodes array?
    */
    virtual bool IsUndoNodes(SwNodes const& rNodes) const = 0;

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

} // namespace sw

#endif

