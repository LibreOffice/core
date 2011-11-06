/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

void EnableDrawAdjust( ScDocument* pDoc, sal_Bool bEnable )
{
    ScDrawLayer* pLayer = pDoc->GetDrawLayer();
    if (pLayer)
        pLayer->EnableAdjust(bEnable);
}



