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
    sal_uLong nLast = aCtn.Count();
    for (sal_uLong nAction = 0; nAction < nLast; nAction++)
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

sal_Bool SdUndoGroup::Merge( SfxUndoAction* pNextAction )
{
    sal_Bool bRet = sal_False;

    if( pNextAction && pNextAction->ISA( SdUndoAction ) )
    {
        SdUndoAction* pClone = static_cast< SdUndoAction* >( pNextAction )->Clone();

        if( pClone )
        {
            AddAction( pClone );
            bRet = sal_True;
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
        ((SdUndoAction*)aCtn.GetObject((sal_uLong)nAction))->Undo();
    }

}

/*************************************************************************
|*
|* Redo
|*
\************************************************************************/

void SdUndoGroup::Redo()
{
    sal_uLong nLast = aCtn.Count();
    for (sal_uLong nAction = 0; nAction < nLast; nAction++)
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
