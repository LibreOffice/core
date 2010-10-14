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
#ifndef _UNDO_HXX
#define _UNDO_HXX

#include "svl/svldllapi.h"
#include <tools/rtti.hxx>
#include <tools/string.hxx>
#include <svl/svarray.hxx>

#include <boost/scoped_ptr.hpp>

//====================================================================

class SVL_DLLPUBLIC SfxRepeatTarget
{
public:
                        TYPEINFO();
    virtual             ~SfxRepeatTarget() = 0;
};

//====================================================================

class SVL_DLLPUBLIC SfxUndoAction
{
    BOOL bLinked;
public:
                            TYPEINFO();
                            SfxUndoAction();
    virtual                 ~SfxUndoAction();

    virtual BOOL            IsLinked();
    virtual void            SetLinked( BOOL bIsLinked = TRUE );
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget&);
    virtual BOOL            CanRepeat(SfxRepeatTarget&) const;

    virtual BOOL            Merge( SfxUndoAction *pNextAction );

    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&) const;
    virtual USHORT          GetId() const;

private:
    SfxUndoAction&          operator=( const SfxUndoAction& );    // n.i.!!
};

//========================================================================

SV_DECL_PTRARR( SfxUndoActions, SfxUndoAction*, 20, 8 )

//====================================================================

/** do not make use of this implementation details, unless you
    really really have to! */
struct SVL_DLLPUBLIC SfxUndoArray
{
    SfxUndoActions          aUndoActions;
    USHORT                  nMaxUndoActions;
    USHORT                  nCurUndoAction;
    SfxUndoArray            *pFatherUndoArray;
                            SfxUndoArray(USHORT nMax=0):
                                nMaxUndoActions(nMax), nCurUndoAction(0),
                                pFatherUndoArray(0) {}
                           ~SfxUndoArray();
};

//=========================================================================

/** do not make use of this implementation details, unless you
    really really have to! */
class SVL_DLLPUBLIC SfxListUndoAction : public SfxUndoAction, public SfxUndoArray

/*  [Beschreibung]

    UndoAction zur Klammerung mehrerer Undos in einer UndoAction.
    Diese Actions werden vom SfxUndoManager verwendet. Dort
    wird mit < SfxUndoManager::EnterListAction > eine Klammerebene
    geoeffnet und mit <SfxUndoManager::LeaveListAction > wieder
    geschlossen. Redo und Undo auf SfxListUndoActions wirken
    Elementweise.

*/
{
    public:
                            TYPEINFO();

                            SfxListUndoAction( const UniString &rComment,
                                const UniString rRepeatComment, USHORT Id, SfxUndoArray *pFather);
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget&);
    virtual BOOL            CanRepeat(SfxRepeatTarget&) const;

    virtual BOOL            Merge( SfxUndoAction *pNextAction );

    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&) const;
    virtual USHORT          GetId() const;

    void SetComment( const UniString& rComment );

    private:

    USHORT                  nId;
    UniString                   aComment, aRepeatComment;

};

//=========================================================================

/**  is a callback interface for notifications about state changes of an SfxUndoManager
*/
class SAL_NO_VTABLE SfxUndoListener
{
public:
    virtual void actionUndone( SfxUndoAction& i_action ) = 0;
    virtual void actionRedone( SfxUndoAction& i_action ) = 0;
    virtual void undoActionAdded( SfxUndoAction& i_action ) = 0;
    virtual void cleared() = 0;
    virtual void clearedRedo() = 0;
    virtual void listActionEntered( const String& i_comment ) = 0;
    virtual void listActionLeft() = 0;
    virtual void undoManagerDying() = 0;
};

//=========================================================================

struct SfxUndoManager_Data;
class SVL_DLLPUBLIC SfxUndoManager
{
    friend class SfxLinkUndoAction;

    ::boost::scoped_ptr< SfxUndoManager_Data >
                            m_pData;
public:
                            SfxUndoManager( USHORT nMaxUndoActionCount = 20 );
    virtual                 ~SfxUndoManager();

    virtual void            SetMaxUndoActionCount( USHORT nMaxUndoActionCount );
    virtual USHORT          GetMaxUndoActionCount() const;
    virtual void            Clear();

    virtual void            AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg=FALSE );

    virtual USHORT          GetUndoActionCount() const;
    virtual USHORT          GetUndoActionId(USHORT nNo=0) const;
    virtual UniString       GetUndoActionComment( USHORT nNo=0 ) const;
    /** returns the nNo'th undo action from the top */
    SfxUndoAction*          GetUndoAction( USHORT nNo=0 ) const;

    virtual BOOL            Undo();

    virtual USHORT          GetRedoActionCount() const;
    virtual USHORT          GetRedoActionId(USHORT nNo=0) const;
    virtual UniString           GetRedoActionComment( USHORT nNo=0 ) const;

    virtual BOOL            Redo();
    virtual void            ClearRedo();

    virtual USHORT          GetRepeatActionCount() const;
    virtual UniString           GetRepeatActionComment( SfxRepeatTarget &rTarget, USHORT nNo = 0) const;
    virtual BOOL            Repeat( SfxRepeatTarget &rTarget, USHORT nFrom=0, USHORT nCount=1 );
    virtual void            Repeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction );
    virtual BOOL            CanRepeat( SfxRepeatTarget &rTarget, USHORT nNo = 0 ) const;
    virtual BOOL            CanRepeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction ) const;

    virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, USHORT nId=0);
    virtual void            LeaveListAction();

    /// determines whether we're within a ListAction context, i.e. a LeaveListAction call is pending
    bool                    IsInListAction() const;

    USHORT                  GetListActionDepth() const;

    /** clears the redo stack and removes the top undo action */
    void                    RemoveLastUndoAction();

    // enables (true) or disables (false) recording of undo actions
    // If undo actions are added while undo is disabled, they are deleted.
    // Disabling undo does not clear the current undo buffer!
    void                    EnableUndo( bool bEnable );

    // returns true if undo is currently enabled
    // This returns false if undo was disabled using EnableUndo( false ) and
    // also during the runtime of the Undo() and Redo() methods.
    bool                    IsUndoEnabled() const;

    /// adds a new listener to be notified about changes in the UndoManager's state
    void                    AddUndoListener( SfxUndoListener& i_listener );
    void                    RemoveUndoListener( SfxUndoListener& i_listener );

protected:
    void    ImplUndo( SfxUndoAction &rAction );
    void    ImplRedo( SfxUndoAction &rAction );
};

//=========================================================================

class SVL_DLLPUBLIC SfxLinkUndoAction : public SfxUndoAction

/*  [Beschreibung]

    Die SfxLinkUndoAction dient zur Verbindung zweier SfxUndoManager. Die
    im ersten SfxUndoManager eingefuegten SfxUndoAction leiten ihr Undo und Redo
    an den zweiten weiter, so dass ein Undo und Redo am ersten
    SfxUndoManager wie eine am zweiten wirkt.

    Die SfxLinkUndoAction ist nach dem Einfuegen der SfxUndoAction am
    zweiten SfxUndoManager einzufuegen. Waehrend der zweite SfxUndoManager
    vom ersten ferngesteuert wird, duerfen an ihm weder Actions eingefuegt werden,
    noch darf Undo/Redo aufgerufen werden.

*/

{
public:
                            TYPEINFO();
                            SfxLinkUndoAction(SfxUndoManager *pManager);
                            ~SfxLinkUndoAction();

    virtual void            Undo();
    virtual void            Redo();
    virtual BOOL            CanRepeat(SfxRepeatTarget& r) const;

    virtual void            Repeat(SfxRepeatTarget&r);

    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&r) const;
    virtual USHORT          GetId() const;

    SfxUndoAction*          GetAction() const { return pAction; }

protected:
    SfxUndoManager          *pUndoManager;
    SfxUndoAction           *pAction;

};

#endif
