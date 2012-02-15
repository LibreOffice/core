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



#ifndef _SD_UNDOMANAGER_HXX
#define _SD_UNDOMANAGER_HXX

#include "misc/scopelock.hxx"
#include <svl/undo.hxx>

namespace sd
{

class UndoManager : public SfxUndoManager
{
public:
    UndoManager( sal_uInt16 nMaxUndoActionCount = 20 );

    virtual void            EnterListAction(const UniString &rComment, const UniString& rRepeatComment, sal_uInt16 nId=0);

    virtual void            AddUndoAction( SfxUndoAction *pAction, sal_Bool bTryMerg=sal_False );

    /** Set or reset the undo manager linked with the called undo manager.
    */
    void SetLinkedUndoManager (::svl::IUndoManager* pLinkedUndoManager);

private:
    using SfxUndoManager::Undo;
    using SfxUndoManager::Redo;

    /** Used when the outline view is visible as a last resort to
        synchronize the undo managers.
    */
    ::svl::IUndoManager* mpLinkedUndoManager;

    /** Call ClearRedo() at the linked undo manager, when present.

        It should not be necessary to call ClearRedo() explicitly, but the
        synchronization between the under managers of the document and the
        outline view seems to have a bug.  Therefore this method is called
        whenever a new undo action is added.
    */
    void ClearLinkedRedoActions (void);
};

}

#endif     // _SD_UNDOMANAGER_HXX
