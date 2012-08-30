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
#ifndef _TEXTUNDO_HXX
#define _TEXTUNDO_HXX

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
                        TYPEINFO();
                        TextUndo( TextEngine* pTextEngine );
    virtual             ~TextUndo();

    TextEngine*         GetTextEngine() const   { return mpTextEngine; }

    virtual void        Undo()      = 0;
    virtual void        Redo()      = 0;

    virtual rtl::OUString GetComment() const;
};

#endif // _TEXTUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
