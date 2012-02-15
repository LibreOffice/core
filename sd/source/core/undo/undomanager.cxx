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
#include <tools/debug.hxx>
#include "undo/undomanager.hxx"

using namespace sd;

UndoManager::UndoManager( sal_uInt16 nMaxUndoActionCount /* = 20 */ )
: SfxUndoManager( nMaxUndoActionCount )
, mpLinkedUndoManager(NULL)
{
}

void UndoManager::EnterListAction(const UniString &rComment, const UniString& rRepeatComment, sal_uInt16 nId /* =0 */)
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SfxUndoManager::EnterListAction( rComment, rRepeatComment, nId );
    }
}

void UndoManager::AddUndoAction( SfxUndoAction *pAction, sal_Bool bTryMerg /* = sal_False */ )
{
    if( !IsDoing() )
    {
        ClearLinkedRedoActions();
        SfxUndoManager::AddUndoAction( pAction, bTryMerg );
    }
    else
    {
        delete pAction;
    }
}


void UndoManager::SetLinkedUndoManager (::svl::IUndoManager* pLinkedUndoManager)
{
    mpLinkedUndoManager = pLinkedUndoManager;
}




void UndoManager::ClearLinkedRedoActions (void)
{
    if (mpLinkedUndoManager != NULL)
        mpLinkedUndoManager->ClearRedo();
}
