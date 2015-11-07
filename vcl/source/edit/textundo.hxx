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
#ifndef INCLUDED_VCL_SOURCE_EDIT_TEXTUNDO_HXX
#define INCLUDED_VCL_SOURCE_EDIT_TEXTUNDO_HXX

#include <svl/undo.hxx>
#include <vcl/texteng.hxx>

class TextEngine;
class TextView;
class TextSelection;
class TextDoc;
class TEParaPortions;

class TextUndoManager : public SfxUndoManager
{
    TextEngine*     mpTextEngine;

protected:

    void            UndoRedoStart();
    void            UndoRedoEnd();

    TextView*       GetView() const { return mpTextEngine->GetActiveView(); }

public:
    explicit TextUndoManager( TextEngine* pTextEngine );
    virtual ~TextUndoManager();

    using SfxUndoManager::Undo;
    virtual bool Undo() override;
    using SfxUndoManager::Redo;
    virtual bool Redo() override;

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
    explicit            TextUndo( TextEngine* pTextEngine );
    virtual             ~TextUndo();

    TextEngine*         GetTextEngine() const   { return mpTextEngine; }

    virtual void        Undo() override      = 0;
    virtual void        Redo() override      = 0;

    virtual OUString GetComment() const override;
};

#endif // INCLUDED_VCL_SOURCE_EDIT_TEXTUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
