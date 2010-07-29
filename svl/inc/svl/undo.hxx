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
    sal_Bool bLinked;
public:
                            TYPEINFO();
                            SfxUndoAction();
    virtual                 ~SfxUndoAction();

    virtual sal_Bool            IsLinked();
    virtual void            SetLinked( sal_Bool bIsLinked = sal_True );
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget&);
    virtual sal_Bool            CanRepeat(SfxRepeatTarget&) const;

    virtual sal_Bool            Merge( SfxUndoAction *pNextAction );

    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&) const;
    virtual sal_uInt16          GetId() const;

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
    sal_uInt16                  nMaxUndoActions;
    sal_uInt16                  nCurUndoAction;
    SfxUndoArray            *pFatherUndoArray;
                            SfxUndoArray(sal_uInt16 nMax=0):
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
                                const UniString rRepeatComment, sal_uInt16 Id, SfxUndoArray *pFather);
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget&);
    virtual sal_Bool            CanRepeat(SfxRepeatTarget&) const;

    virtual sal_Bool            Merge( SfxUndoAction *pNextAction );

    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&) const;
    virtual sal_uInt16          GetId() const;

    void SetComment( const UniString& rComment );

    private:

    sal_uInt16                  nId;
    UniString                   aComment, aRepeatComment;

};

//=========================================================================

class SVL_DLLPUBLIC SfxUndoManager
{
    friend class SfxLinkUndoAction;

    SfxUndoArray            *pUndoArray;
    SfxUndoArray            *pActUndoArray;
    SfxUndoArray            *pFatherUndoArray;

    bool                    mbUndoEnabled;
public:
                            SfxUndoManager( sal_uInt16 nMaxUndoActionCount = 20 );
    virtual                 ~SfxUndoManager();

    virtual void            SetMaxUndoActionCount( sal_uInt16 nMaxUndoActionCount );
    virtual sal_uInt16          GetMaxUndoActionCount() const;
    virtual void            Clear();

    virtual void            AddUndoAction( SfxUndoAction *pAction, sal_Bool bTryMerg=sal_False );

    virtual sal_uInt16          GetUndoActionCount() const;
    virtual sal_uInt16          GetUndoActionId(sal_uInt16 nNo=0) const;
    virtual UniString       GetUndoActionComment( sal_uInt16 nNo=0 ) const;
    /** returns the nNo'th undo action from the top */
    SfxUndoAction*          GetUndoAction( sal_uInt16 nNo=0 ) const;

    virtual sal_Bool            Undo( sal_uInt16 nCount=1 );
    virtual void            Undo( SfxUndoAction &rAction );

    virtual sal_uInt16          GetRedoActionCount() const;
    virtual sal_uInt16          GetRedoActionId(sal_uInt16 nNo=0) const;
    virtual UniString           GetRedoActionComment( sal_uInt16 nNo=0 ) const;

    virtual sal_Bool            Redo( sal_uInt16 nCount=1 );
    virtual void            Redo( SfxUndoAction &rAction );
    virtual void            ClearRedo();

    virtual sal_uInt16          GetRepeatActionCount() const;
    virtual UniString           GetRepeatActionComment( SfxRepeatTarget &rTarget, sal_uInt16 nNo = 0) const;
    virtual sal_Bool            Repeat( SfxRepeatTarget &rTarget, sal_uInt16 nFrom=0, sal_uInt16 nCount=1 );
    virtual void            Repeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction );
    virtual sal_Bool            CanRepeat( SfxRepeatTarget &rTarget, sal_uInt16 nNo = 0 ) const;
    virtual sal_Bool            CanRepeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction ) const;

    virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, sal_uInt16 nId=0);
    virtual void            LeaveListAction();

    /** clears the redo stack and removes the top undo action */
    void                    RemoveLastUndoAction();

    // enables (true) or disables (false) recording of undo actions
    // If undo actions are added while undo is disabled, they are deleted.
    // Disabling undo does not clear the current undo buffer!
    void                    EnableUndo( bool bEnable );

    // returns true if undo is currently enabled
    // This returns false if undo was disabled using EnableUndo( false ) and
    // also during the runtime of the Undo() and Redo() methods.
    bool                    IsUndoEnabled() const { return mbUndoEnabled; }
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
    virtual sal_Bool            CanRepeat(SfxRepeatTarget& r) const;

    virtual void            Repeat(SfxRepeatTarget&r);

    virtual UniString           GetComment() const;
    virtual UniString           GetRepeatComment(SfxRepeatTarget&r) const;
    virtual sal_uInt16          GetId() const;

    SfxUndoAction*          GetAction() const { return pAction; }

protected:
    SfxUndoManager          *pUndoManager;
    SfxUndoAction           *pAction;

};

#endif
