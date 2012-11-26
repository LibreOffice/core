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
#include "precompiled_sd.hxx"


#include "sdundogr.hxx"


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
    bool bRet = false;
    SdUndoAction* pSdUndoAction = dynamic_cast< SdUndoAction* >(pNextAction);

    if( pSdUndoAction )
    {
        SdUndoAction* pClone = pSdUndoAction->Clone();

        if( pClone )
        {
            AddAction( pClone );
            bRet = true;
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
