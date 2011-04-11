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
        // if no drawing layer existed when the action was created,
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
    // DoSdrUndoAction/RedoSdrUndoAction is called even if the pointer is null

    if ( pUndoAction )
        pUndoAction->Redo();
}

void DeleteSdrUndoAction( SdrUndoAction* pUndoAction )
{
    delete pUndoAction;
}

void EnableDrawAdjust( ScDocument* pDoc, sal_Bool bEnable )
{
    ScDrawLayer* pLayer = pDoc->GetDrawLayer();
    if (pLayer)
        pLayer->EnableAdjust(bEnable);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
