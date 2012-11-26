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



#ifndef _EDITUND2_HXX
#define _EDITUND2_HXX

#include "editeng/editengdllapi.h"
#include <svl/undo.hxx>

class ImpEditEngine;

class EDITENG_DLLPUBLIC EditUndoManager : public SfxUndoManager
{
private:
    using SfxUndoManager::Undo;
    using SfxUndoManager::Redo;
    friend class ImpEditEngine;

    ImpEditEngine*  mpImpEE;
    void SetImpEditEngine(ImpEditEngine* pNew);

public:
    EditUndoManager(sal_uInt16 nMaxUndoActionCount = 20);

    virtual sal_Bool Undo();
    virtual sal_Bool Redo();
};

// -----------------------------------------------------------------------
// EditUndo
// ------------------------------------------------------------------------
class EDITENG_DLLPUBLIC EditUndo : public SfxUndoAction
{
private:
    sal_uInt16          nId;
    ImpEditEngine*  pImpEE;

public:
                    EditUndo( sal_uInt16 nI, ImpEditEngine* pImpEE );
    virtual         ~EditUndo();

    ImpEditEngine*  GetImpEditEngine() const    { return pImpEE; }

    virtual void    Undo()      = 0;
    virtual void    Redo()      = 0;

    virtual sal_Bool    CanRepeat(SfxRepeatTarget&) const;
    virtual String  GetComment() const;
    virtual sal_uInt16  GetId() const;
};

#endif  // _EDITUND2_HXX


