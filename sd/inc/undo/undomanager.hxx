/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undomanager.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:27:29 $
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

#ifndef _SD_UNDOMANAGER_HXX
#define _SD_UNDOMANAGER_HXX

#ifndef _SD_SCOPELOCK_HXX
#include "misc/scopelock.hxx"
#endif

#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif

namespace sd
{

class UndoManager : public SfxUndoManager
{
public:
    UndoManager( USHORT nMaxUndoActionCount = 20 );

    virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, USHORT nId=0);
    virtual void            LeaveListAction();

    virtual void            AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg=FALSE );

    bool                    isInListAction() const { return mnListLevel != 0; }
    bool                    isInUndo() const { return maIsInUndoLock.isLocked(); }

    virtual BOOL            Undo( USHORT nCount=1 );
    virtual BOOL            Redo( USHORT nCount=1 );

private:
    using SfxUndoManager::Undo;
    using SfxUndoManager::Redo;

    int mnListLevel;
    ScopeLock maIsInUndoLock;
};

}

#endif     // _SD_UNDOMANAGER_HXX
