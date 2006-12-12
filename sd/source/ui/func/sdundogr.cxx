/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdundogr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:27:57 $
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


#include "sdundogr.hxx"


TYPEINIT1(SdUndoGroup, SdUndoAction);

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdUndoGroup::~SdUndoGroup()
{
    ULONG nLast = aCtn.Count();
    for (ULONG nAction = 0; nAction < nLast; nAction++)
    {
        delete (SdUndoAction*) aCtn.GetObject(nAction);
    }
    aCtn.Clear();
}

/*************************************************************************
|*
|* Merge
|*
\************************************************************************/

BOOL SdUndoGroup::Merge( SfxUndoAction* pNextAction )
{
    BOOL bRet = FALSE;

    if( pNextAction && pNextAction->ISA( SdUndoAction ) )
    {
        SdUndoAction* pClone = static_cast< SdUndoAction* >( pNextAction )->Clone();

        if( pClone )
        {
            AddAction( pClone );
            bRet = TRUE;
        }
    }

    return bRet;
}

/*************************************************************************
|*
|* Undo, umgekehrte Reihenfolge der Ausfuehrung
|*
\************************************************************************/

void SdUndoGroup::Undo()
{
    long nLast = aCtn.Count();
    for (long nAction = nLast - 1; nAction >= 0; nAction--)
    {
        ((SdUndoAction*)aCtn.GetObject((ULONG)nAction))->Undo();
    }

}

/*************************************************************************
|*
|* Redo
|*
\************************************************************************/

void SdUndoGroup::Redo()
{
    ULONG nLast = aCtn.Count();
    for (ULONG nAction = 0; nAction < nLast; nAction++)
    {
        ((SdUndoAction*)aCtn.GetObject(nAction))->Redo();
    }

}

/*************************************************************************
|*
|* eine Aktion hinzufuegen
|*
\************************************************************************/

void SdUndoGroup::AddAction(SdUndoAction* pAction)
{
    aCtn.Insert(pAction, CONTAINER_APPEND);
}

/*************************************************************************
|*
|* Zeiger auf eine Aktion liefern
|*
\************************************************************************/

SdUndoAction* SdUndoGroup::GetAction(ULONG nAction) const
{
    return (SdUndoAction*)aCtn.GetObject(nAction);
}
