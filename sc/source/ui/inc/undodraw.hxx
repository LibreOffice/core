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
                            TYPEINFO();
                            ScUndoDraw( SfxUndoAction* pUndo, ScDocShell* pDocSh );
    virtual                 ~ScUndoDraw();

    SfxUndoAction*          GetDrawUndo()       { return pDrawUndo; }
    void                    ForgetDrawUndo();

    virtual sal_Bool            IsLinked();
    virtual void            SetLinked( sal_Bool bIsLinked );
    virtual void            Undo();
    virtual void            Redo();
    virtual void            Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool        CanRepeat(SfxRepeatTarget& rTarget) const;
    virtual sal_Bool        Merge( SfxUndoAction *pNextAction );
    virtual OUString        GetComment() const;
    virtual OUString        GetRepeatComment(SfxRepeatTarget&) const;
    virtual sal_uInt16      GetId() const;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
