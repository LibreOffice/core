/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


typedef sal_uInt16 SwUndoNoModifiedPosition;

/** IDocumentUndoRedo
*/
class IDocumentUndoRedo
{
public:
    /**
    */
    virtual void SetUndoNoResetModified() = 0;

    /**
    */
    virtual bool IsUndoNoResetModified() const = 0;

    /** UndoHistory am Dokument pflegen
        bei Save, SaveAs, Create wird UndoHistory zurueckgesetzt ???
    */
    virtual void DoUndo(bool bUn) = 0;

    /**
    */
    virtual bool DoesUndo() const = 0;

    /** Zusammenfassen von Kontinuierlichen Insert/Delete/Overwrite von
        Charaktern. Default ist ::com::sun::star::sdbcx::Group-Undo.
    */
    virtual void DoGroupUndo(bool bUn) = 0;

    /**
    */
    virtual bool DoesGroupUndo() const = 0;

    /** macht rueckgaengig:
        0 letzte Aktion, sonst Aktionen bis zum Start der Klammerung nUndoId
        In rUndoRange wird der restaurierte Bereich gesetzt.
    */
    virtual bool Undo( SwUndoIter& ) = 0; // -> #111827#

    /** Opens undo block.

        @param nUndoId        undo ID for the start object
        @param pRewriter      rewriter for comments @see SwUndo::GetComment

        If the given nUndoId is equal to zero an undo object with ID
        UNDO_START will be generated.

        @return the undo ID of the created object
    */
    virtual SwUndoId StartUndo( SwUndoId eUndoId, const SwRewriter * pRewriter) = 0;

    /**
       Closes undo block.

       @param nUndoId         undo ID for the closure object
       @param pRewriter       rewriter for comments @see SwUndo::GetComment

       If the given nUndoId is equal to zero an undo object with ID
       UNDO_START will be generated.

       If pRewriter is not equal to zero the given rewriter will be
       set for the generated closure object and the corresponding
       start object. Otherwise an existent rewriter in theIDocumentRedlineAccess
       corresponding start object will be propagated to the generated
       closure object.
    */
    virtual SwUndoId EndUndo( SwUndoId eUndoId, const SwRewriter * pRewriter) = 0;

    /** <- #111827#
        loescht die gesamten UndoObjecte ( fuer Methoden die am Nodes
        Array drehen ohne entsprechendes Undo !!)
    */
    virtual void DelAllUndoObj() = 0;

    /** liefert die Id der letzten undofaehigen Aktion zurueck
        oder USHRT_MAX fuellt ggf. VARARR mit ::com::sun::star::sdbcx::User-UndoIds
    */
    virtual SwUndoId GetUndoIds(String* pStr, SwUndoIds *pUndoIds) const = 0;

    /**
    */
    virtual String GetUndoIdsStr(String* pStr, SwUndoIds *pUndoIds) const = 0;

    /** gibt es Klammerung mit der Id?
    */
    virtual bool HasUndoId(SwUndoId eId) const = 0;

    /* @@@MAINTAINABILITY-HORROR@@@
       Implementation details made public.
       die drei folgenden Methoden werden beim Undo und nur dort
       benoetigt. Sollten sonst nicht aufgerufen werden.
    */
    virtual const SwNodes* GetUndoNds() const = 0;

    virtual SwUndo* RemoveLastUndo(SwUndoId eUndoId) = 0;

    /** 2002-05-31 dvo, #95884#: To prevent an undo array overflow when
        doing nested undos, undo may have to be disabled. Undo-intensive
        actions (like auto-format) should check this manually.
    */
    virtual bool HasTooManyUndos() const = 0;

    /**
    */
    virtual bool Redo( SwUndoIter& ) = 0;

    /** liefert die Id der letzten Redofaehigen Aktion zurueck
        fuellt ggf. VARARR mit RedoIds
    */
    virtual SwUndoId GetRedoIds( String* pStr, SwUndoIds *pRedoIds) const = 0;

    /**
    */
    virtual String GetRedoIdsStr( String* pStr, SwUndoIds *pRedoIds) const = 0;

    /**
    */
    virtual bool Repeat( SwUndoIter&, sal_uInt16 nRepeatCnt) = 0;

    /** liefert die Id der letzten Repeatfaehigen Aktion zurueck
        fuellt ggf. VARARR mit RedoIds
    */
    virtual SwUndoId GetRepeatIds( String* pStr, SwUndoIds *pRedoIds) const = 0;

    /**
    */
    virtual String GetRepeatIdsStr( String* pStr, SwUndoIds *pRedoIds) const = 0;

    /** interne Verkuerzung fuer Insert am Ende
    */
    virtual void AppendUndo(SwUndo*) = 0;

    /** loescht alle UndoObjecte von nUndoPos
        bis zum Ende des Undo-Arrays
    */
    virtual void ClearRedo() = 0;

    /** Manipulates the position of the undo stack which reset the modified flag
    */
    virtual void setUndoNoModifiedPosition( SwUndoNoModifiedPosition ) = 0;

    /** Gets the position of the undo stack which reset the modified flag
    */
    virtual SwUndoNoModifiedPosition getUndoNoModifiedPosition() const = 0;

protected:
     virtual ~IDocumentUndoRedo() {};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
