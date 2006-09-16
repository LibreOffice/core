/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undomanager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:18:17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SD_UNDOMANAGER_HXX
#include "undo/undomanager.hxx"
#endif

using namespace sd;

UndoManager::UndoManager( USHORT nMaxUndoActionCount /* = 20 */ )
: SfxUndoManager( nMaxUndoActionCount )
, mnListLevel( 0 )
{
}

void UndoManager::EnterListAction(const UniString &rComment, const UniString& rRepeatComment, USHORT nId /* =0 */)
{
    DBG_ASSERT( !isInUndo(), "sd::UndoManager::EnterListAction(), calling me during Undo, are you nuts!?" );

    mnListLevel++;
    SfxUndoManager::EnterListAction( rComment, rRepeatComment, nId );
}

void UndoManager::LeaveListAction()
{
    DBG_ASSERT( !isInUndo(), "sd::UndoManager::LeaveListAction(), calling me during Undo, are you nuts!?" );

    SfxUndoManager::LeaveListAction();
    if( mnListLevel )
    {
        mnListLevel--;
    }
    else
    {
        DBG_ERROR("sd::UndoManager::LeaveListAction(), no open list action!" );
    }
}

void UndoManager::AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg /* = FALSE */ )
{
    DBG_ASSERT( !isInUndo(), "sd::UndoManager::AddUndoAction(), calling me during Undo, are you nuts!?" );
    SfxUndoManager::AddUndoAction( pAction, bTryMerg );
}


BOOL UndoManager::Undo( USHORT nCount )
{
    ScopeLockGuard aGuard( maIsInUndoLock );
    return SfxUndoManager::Undo( nCount );
}

BOOL UndoManager::Redo( USHORT nCount )
{
    ScopeLockGuard aGuard( maIsInUndoLock );
    return SfxUndoManager::Redo( nCount );
}

