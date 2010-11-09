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
    virtual void actionUndone( const String& i_actionComment ) = 0;
    virtual void actionRedone( const String& i_actionComment ) = 0;
    virtual void undoActionAdded( const String& i_actionComment ) = 0;
    virtual void cleared() = 0;
    virtual void clearedRedo() = 0;
    virtual void listActionEntered( const String& i_comment ) = 0;
    virtual void listActionLeft() = 0;
    virtual void listActionLeftAndMerged() = 0;
    virtual void listActionCancelled() = 0;
    virtual void undoManagerDying() = 0;
};

//=========================================================================

namespace svl
{
    class SAL_NO_VTABLE IUndoManager
    {
    public:
        enum
        {
            CurrentLevel = true,
            TopLevel = false
        };

        virtual                 ~IUndoManager() { };

        virtual void            SetMaxUndoActionCount( USHORT nMaxUndoActionCount ) = 0;
        virtual USHORT          GetMaxUndoActionCount() const = 0;

        virtual void            AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg=FALSE ) = 0;

        virtual USHORT          GetUndoActionCount( bool const i_currentLevel = CurrentLevel ) const = 0;
        virtual USHORT          GetUndoActionId() const = 0;
        virtual UniString       GetUndoActionComment( USHORT nNo=0, bool const i_currentLevel = CurrentLevel ) const = 0;
        virtual SfxUndoAction*  GetUndoAction( USHORT nNo=0 ) const = 0;

        virtual USHORT          GetRedoActionCount( bool const i_currentLevel = CurrentLevel ) const = 0;
        virtual UniString       GetRedoActionComment( USHORT nNo=0, bool const i_currentLevel = CurrentLevel ) const = 0;

        virtual BOOL            Undo() = 0;
        virtual BOOL            Redo() = 0;

        virtual void            Clear() = 0;
        virtual void            ClearRedo() = 0;

        /** determines whether an Undo or Redo is currently running
        */
        virtual bool            IsDoing() const = 0;

        virtual USHORT          GetRepeatActionCount() const = 0;
        virtual UniString       GetRepeatActionComment( SfxRepeatTarget &rTarget) const = 0;
        virtual BOOL            Repeat( SfxRepeatTarget &rTarget ) = 0;
        virtual BOOL            CanRepeat( SfxRepeatTarget &rTarget ) const = 0;

        virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, USHORT nId=0) = 0;

        /** leaves the list action entered with EnterListAction
            @return the number of the sub actions in the list which has just been left. Note that in case no such
                actions exist, the list action does not contribute to the Undo stack, but is silently removed.
        */
        virtual USHORT          LeaveListAction() = 0;

        /** leaves the list action entered with EnterListAction, and forcefully merges the previous
            action on the stack into the newly created list action.

            Say you have an Undo action A on the stack, then call EnterListAction, followed by one or more calls to
            AddUndoAction, followed by a call to LeaveAndMergeListAction. In opposite to LeaveListAction, your Undo
            stack will now still contain one undo action: the newly created list action, whose first child is the
            original A, whose other children are those you added via AddUndoAction, and whose comment is the same as
            the comment of A.

            Effectively, this means that all actions added between EnterListAction and LeaveAndMergeListAction are
            hidden from the user.

            @return the number of the sub actions in the list which has just been left. Note that in case no such
                actions exist, the list action does not contribute to the Undo stack, but is silently removed.
        */
        virtual USHORT          LeaveAndMergeListAction() = 0;

        /// determines whether we're within a ListAction context, i.e. a LeaveListAction/LeaveAndMergeListAction call is pending
        virtual bool            IsInListAction() const = 0;

        /// determines how many nested list actions are currently open
        virtual USHORT          GetListActionDepth() const = 0;

        /** clears the redo stack and removes the top undo action */
        virtual void            RemoveLastUndoAction() = 0;

        // enables (true) or disables (false) recording of undo actions
        // If undo actions are added while undo is disabled, they are deleted.
        // Disabling undo does not clear the current undo buffer!
        virtual void            EnableUndo( bool bEnable ) = 0;

        // returns true if undo is currently enabled
        // This returns false if undo was disabled using EnableUndo( false ) and
        // also during the runtime of the Undo() and Redo() methods.
        virtual bool            IsUndoEnabled() const = 0;

        /// adds a new listener to be notified about changes in the UndoManager's state
        virtual void            AddUndoListener( SfxUndoListener& i_listener ) = 0;
        virtual void            RemoveUndoListener( SfxUndoListener& i_listener ) = 0;
   };
}

//=========================================================================

namespace svl { namespace undo { namespace impl
{
    class UndoManagerGuard;
    class LockGuard;
} } }

struct SfxUndoManager_Data;
class SVL_DLLPUBLIC SfxUndoManager : public ::svl::IUndoManager
{
    friend class SfxLinkUndoAction;

    ::boost::scoped_ptr< SfxUndoManager_Data >
                            m_pData;
public:
                            SfxUndoManager( USHORT nMaxUndoActionCount = 20 );
    virtual                 ~SfxUndoManager();

    // IUndoManager overridables
    virtual void            SetMaxUndoActionCount( USHORT nMaxUndoActionCount );
    virtual USHORT          GetMaxUndoActionCount() const;
    virtual void            AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg=FALSE );
    virtual USHORT          GetUndoActionCount( bool const i_currentLevel = CurrentLevel ) const;
    virtual USHORT          GetUndoActionId() const;
    virtual UniString       GetUndoActionComment( USHORT nNo=0, bool const i_currentLevel = CurrentLevel ) const;
    virtual SfxUndoAction*  GetUndoAction( USHORT nNo=0 ) const;
    virtual USHORT          GetRedoActionCount( bool const i_currentLevel = CurrentLevel ) const;
    virtual UniString       GetRedoActionComment( USHORT nNo=0, bool const i_currentLevel = CurrentLevel ) const;
    virtual BOOL            Undo();
    virtual BOOL            Redo();
    virtual void            Clear();
    virtual void            ClearRedo();
    virtual bool            IsDoing() const;
    virtual USHORT          GetRepeatActionCount() const;
    virtual UniString       GetRepeatActionComment( SfxRepeatTarget &rTarget) const;
    virtual BOOL            Repeat( SfxRepeatTarget &rTarget );
    virtual BOOL            CanRepeat( SfxRepeatTarget &rTarget ) const;
    virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, USHORT nId=0);
    virtual USHORT          LeaveListAction();
    virtual USHORT          LeaveAndMergeListAction();
    virtual bool            IsInListAction() const;
    virtual USHORT          GetListActionDepth() const;
    virtual void            RemoveLastUndoAction();
    virtual void            EnableUndo( bool bEnable );
    virtual bool            IsUndoEnabled() const;
    virtual void            AddUndoListener( SfxUndoListener& i_listener );
    virtual void            RemoveUndoListener( SfxUndoListener& i_listener );

private:
    USHORT  ImplLeaveListAction( const bool i_merge );
    bool    ImplAddUndoAction_NoNotify( SfxUndoAction* pAction, BOOL bTryMerge, ::svl::undo::impl::UndoManagerGuard& i_guard );
    void    ImplClearRedo( ::svl::undo::impl::UndoManagerGuard& i_guard );
    void    ImplClearUndo( ::svl::undo::impl::UndoManagerGuard& i_guard );
    USHORT  ImplGetRedoActionCount_Lock( bool const i_currentLevel = CurrentLevel ) const;
    bool    ImplIsUndoEnabled_Lock() const;
    bool    ImplIsInListAction_Lock() const;
    void    ImplEnableUndo_Lock( bool const i_enable );

    friend ::svl::undo::impl::LockGuard;
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
                            SfxLinkUndoAction(::svl::IUndoManager *pManager);
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
    ::svl::IUndoManager     *pUndoManager;
    SfxUndoAction           *pAction;

};

#endif
