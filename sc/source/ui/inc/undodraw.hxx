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



#ifndef SC_UNDODRAW_HXX
#define SC_UNDODRAW_HXX

#include <svl/undo.hxx>

class ScDocShell;

class ScUndoDraw: public SfxUndoAction
{
    SfxUndoAction*  pDrawUndo;
    ScDocShell*     pDocShell;

    void            UpdateSubShell();

public:
                            ScUndoDraw( SfxUndoAction* pUndo, ScDocShell* pDocSh );
    virtual                 ~ScUndoDraw();

    SfxUndoAction*          GetDrawUndo()       { return pDrawUndo; }
    void                    ForgetDrawUndo();

    virtual sal_Bool            IsLinked();
    virtual void            SetLinked( sal_Bool bIsLinked );
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool            CanRepeat(SfxRepeatTarget& rTarget) const;
    virtual sal_Bool            Merge( SfxUndoAction *pNextAction );
    virtual String          GetComment() const;
    virtual String          GetRepeatComment(SfxRepeatTarget&) const;
    virtual sal_uInt16          GetId() const;
};



#endif

