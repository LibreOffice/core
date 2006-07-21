/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoolk.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:27:30 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svx/svdundo.hxx>

#include "document.hxx"
#include "drwlayer.hxx"


// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

SdrUndoAction* GetSdrUndoAction( ScDocument* pDoc )
{
    ScDrawLayer* pLayer = pDoc->GetDrawLayer();
    if (pLayer)
        return pLayer->GetCalcUndo();               // muss vorhanden sein
    else
        return NULL;
}

void DoSdrUndoAction( SdrUndoAction* pUndoAction, ScDocument* pDoc )
{
    if ( pUndoAction )
        pUndoAction->Undo();
    else
    {
        // #125875# if no drawing layer existed when the action was created,
        // but it was created after that, there is no draw undo action,
        // and after undo there might be a drawing layer with a wrong page count.
        // The drawing layer must have been empty in that case, so any missing
        // pages can just be created now.

        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if ( pDrawLayer )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            SCTAB nPages = static_cast<SCTAB>(pDrawLayer->GetPageCount());
            while ( nPages < nTabCount )
            {
                pDrawLayer->ScAddPage( nPages );
                ++nPages;
            }
        }
    }
}


void RedoSdrUndoAction( SdrUndoAction* pUndoAction )
{
    // #125875# DoSdrUndoAction/RedoSdrUndoAction is called even if the pointer is null

    if ( pUndoAction )
        pUndoAction->Redo();
}

void DeleteSdrUndoAction( SdrUndoAction* pUndoAction )
{
    delete pUndoAction;
}

void EnableDrawAdjust( ScDocument* pDoc, BOOL bEnable )
{
    ScDrawLayer* pLayer = pDoc->GetDrawLayer();
    if (pLayer)
        pLayer->EnableAdjust(bEnable);
}



