/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:41:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _UNDO_HXX
#define _UNDO_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

//====================================================================

class SVT_DLLPUBLIC SfxRepeatTarget
{
public:
                        TYPEINFO();
    virtual             ~SfxRepeatTarget() = 0;
};

//====================================================================

class SVT_DLLPUBLIC SfxUndoAction
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
struct SVT_DLLPUBLIC SfxUndoArray
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
class SVT_DLLPUBLIC SfxListUndoAction : public SfxUndoAction, public SfxUndoArray

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

class SVT_DLLPUBLIC SfxUndoManager
{
    friend class SfxLinkUndoAction;

    SfxUndoArray            *pUndoArray;
    SfxUndoArray            *pActUndoArray;
    SfxUndoArray            *pFatherUndoArray;

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

    virtual BOOL            Undo( USHORT nCount=1 );
    virtual void            Undo( SfxUndoAction &rAction );

    virtual USHORT          GetRedoActionCount() const;
    virtual USHORT          GetRedoActionId(USHORT nNo=0) const;
    virtual UniString           GetRedoActionComment( USHORT nNo=0 ) const;

    virtual BOOL            Redo( USHORT nCount=1 );
    virtual void            Redo( SfxUndoAction &rAction );
    virtual void            ClearRedo();

    virtual USHORT          GetRepeatActionCount() const;
    virtual UniString           GetRepeatActionComment( SfxRepeatTarget &rTarget, USHORT nNo = 0) const;
    virtual BOOL            Repeat( SfxRepeatTarget &rTarget, USHORT nFrom=0, USHORT nCount=1 );
    virtual void            Repeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction );
    virtual BOOL            CanRepeat( SfxRepeatTarget &rTarget, USHORT nNo = 0 ) const;
    virtual BOOL            CanRepeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction ) const;

    virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, USHORT nId=0);
    virtual void            LeaveListAction();

    /** clears the redo stack and removes the top undo action */
    void                    RemoveLastUndoAction();
};

//=========================================================================

class SVT_DLLPUBLIC SfxLinkUndoAction : public SfxUndoAction

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
