/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/svdundo.hxx>

#include "document.hxx"
#include "drwlayer.hxx"

SdrUndoAction* GetSdrUndoAction( ScDocument* pDoc )
{
    ScDrawLayer* pLayer = pDoc->GetDrawLayer();
    if (pLayer)
        return pLayer->GetCalcUndo();               // must exist
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
