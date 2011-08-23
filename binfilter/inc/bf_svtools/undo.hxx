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
#ifndef _UNDO_HXX
#define _UNDO_HXX

#include "bf_svtools/svtdllapi.h"

#include <tools/rtti.hxx>

#include <tools/string.hxx>

#include <bf_svtools/svarray.hxx>

namespace binfilter
{

//====================================================================

class  SfxRepeatTarget
{
public:
                        TYPEINFO();
    virtual 			~SfxRepeatTarget() = 0;
};

//====================================================================

class  SfxUndoAction
{
    BOOL bLinked;
public:
                            TYPEINFO();
                            SfxUndoAction();
    virtual 				~SfxUndoAction();

    virtual BOOL            IsLinked();
    virtual void            SetLinked( BOOL bIsLinked = TRUE );
    virtual void			Undo();
    virtual void			Redo();
    virtual void			Repeat(SfxRepeatTarget&);
    virtual BOOL			CanRepeat(SfxRepeatTarget&) const;

    virtual BOOL			Merge( SfxUndoAction *pNextAction );

    virtual UniString			GetComment() const;
    virtual UniString			GetRepeatComment(SfxRepeatTarget&) const;
    virtual USHORT			GetId() const;

private:
    SfxUndoAction&			operator=( const SfxUndoAction& );	  // n.i.!!
};

//========================================================================

SV_DECL_PTRARR( SfxUndoActions, SfxUndoAction*, 20, 8 )

//====================================================================

/** do not make use of this implementation details, unless you
    really really have to! */
struct  SfxUndoArray
{
    SfxUndoActions          aUndoActions;
    USHORT					nMaxUndoActions;
    USHORT					nCurUndoAction;
    SfxUndoArray 			*pFatherUndoArray;
                            SfxUndoArray(USHORT nMax=0):
                                nMaxUndoActions(nMax), nCurUndoAction(0),
                                pFatherUndoArray(0) {}
                           ~SfxUndoArray();
};

//=========================================================================

/** do not make use of this implementation details, unless you
    really really have to! */
class  SfxListUndoAction : public SfxUndoAction, public SfxUndoArray

/*	[Beschreibung]

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
    virtual void			Undo();
    virtual void			Redo();
    virtual void			Repeat(SfxRepeatTarget&);
    virtual BOOL			CanRepeat(SfxRepeatTarget&) const;

    virtual BOOL			Merge( SfxUndoAction *pNextAction );

    virtual UniString			GetComment() const;
    virtual UniString			GetRepeatComment(SfxRepeatTarget&) const;
    virtual USHORT			GetId() const;

    void SetComment( const UniString& rComment );

    private:

    USHORT					nId;
    UniString					aComment, aRepeatComment;

};

//=========================================================================

class  SfxUndoManager
{
    friend class SfxLinkUndoAction;

    SfxUndoArray 			*pUndoArray;
    SfxUndoArray			*pActUndoArray;
    SfxUndoArray			*pFatherUndoArray;

public:
                            SfxUndoManager( USHORT nMaxUndoActionCount = 20 );
    virtual 				~SfxUndoManager();

    virtual void			SetMaxUndoActionCount( USHORT nMaxUndoActionCount );
    virtual USHORT			GetMaxUndoActionCount() const;
    virtual void			Clear();

    virtual void			AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg=FALSE );

    virtual USHORT			GetUndoActionCount() const;
    virtual USHORT			GetUndoActionId(USHORT nNo=0) const;
    virtual UniString		GetUndoActionComment( USHORT nNo=0 ) const;

    virtual BOOL			Undo( USHORT nCount=1 );
    virtual void			Undo( SfxUndoAction &rAction );

    virtual USHORT			GetRedoActionCount() const;
    virtual USHORT			GetRedoActionId(USHORT nNo=0) const;
    virtual UniString			GetRedoActionComment( USHORT nNo=0 ) const;

    virtual BOOL			Redo( USHORT nCount=1 );
    virtual void			Redo( SfxUndoAction &rAction );
    virtual	void			ClearRedo();

    virtual USHORT			GetRepeatActionCount() const;
    virtual UniString			GetRepeatActionComment( SfxRepeatTarget &rTarget, USHORT nNo = 0) const;
    virtual BOOL			Repeat( SfxRepeatTarget &rTarget, USHORT nFrom=0, USHORT nCount=1 );
    virtual void			Repeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction );
    virtual BOOL			CanRepeat( SfxRepeatTarget &rTarget, USHORT nNo = 0 ) const;
    virtual BOOL			CanRepeat( SfxRepeatTarget &rTarget, SfxUndoAction &rAction ) const;

    virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, USHORT nId=0);
    virtual void 			LeaveListAction();
};

//=========================================================================

class  SfxLinkUndoAction : public SfxUndoAction

/*	[Beschreibung]

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
                            ~SfxLinkUndoAction();

    virtual void			Undo();
    virtual void			Redo();
    virtual BOOL			CanRepeat(SfxRepeatTarget& r) const;

    virtual void			Repeat(SfxRepeatTarget&r);

    virtual UniString			GetComment() const;
    virtual UniString			GetRepeatComment(SfxRepeatTarget&r) const;
    virtual USHORT			GetId() const;

    SfxUndoAction*			GetAction() const { return pAction; }

protected:
    SfxUndoManager			*pUndoManager;
    SfxUndoAction			*pAction;

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
