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


#ifndef _TEXTUNDO_HXX
#define _TEXTUNDO_HXX

#include <svl/undo.hxx>

class TextEngine;

class TextUndoManager : public SfxUndoManager
{
    TextEngine*     mpTextEngine;

protected:

    void            UndoRedoStart();
    void            UndoRedoEnd();

    TextView*       GetView() const { return mpTextEngine->GetActiveView(); }

public:
                    TextUndoManager( TextEngine* pTextEngine );
                    ~TextUndoManager();

    using SfxUndoManager::Undo;
    virtual sal_Bool Undo();
    using SfxUndoManager::Redo;
    virtual sal_Bool Redo();

};

class TextUndo : public SfxUndoAction
{
private:
    TextEngine*         mpTextEngine;

protected:

    TextView*           GetView() const { return mpTextEngine->GetActiveView(); }
    void                SetSelection( const TextSelection& rSel );

    TextDoc*            GetDoc() const { return mpTextEngine->mpDoc; }
    TEParaPortions*     GetTEParaPortions() const { return mpTextEngine->mpTEParaPortions; }

public:
                        TextUndo( TextEngine* pTextEngine );
    virtual             ~TextUndo();

    TextEngine*         GetTextEngine() const   { return mpTextEngine; }

    virtual void        Undo()      = 0;
    virtual void        Redo()      = 0;

    virtual XubString   GetComment() const;
};

#endif // _TEXTUNDO_HXX
