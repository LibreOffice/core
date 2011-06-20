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
#ifndef _SWANNOTSH_HXX
#define _SWANNOTSH_HXX

#include <sfx2/shell.hxx>
#include "shellid.hxx"
#include "swmodule.hxx"
#include <unotools/caserotate.hxx>

class SwView;
class SwAnnotationShell: public SfxShell
{
    SwView&     rView;
    RotateTransliteration m_aRotateCase;

public:
    SFX_DECL_INTERFACE(SW_ANNOTATIONSHELL)
    TYPEINFO();

                SwAnnotationShell(SwView&);
    virtual     ~SwAnnotationShell();

    void        StateDisableItems(SfxItemSet &);
    void        Exec(SfxRequest &);

    void        GetState(SfxItemSet &);
    void        StateInsert(SfxItemSet &rSet);

    void        NoteExec(SfxRequest &);
    void        GetNoteState(SfxItemSet &);

    void        ExecLingu(SfxRequest &rReq);
    void        GetLinguState(SfxItemSet &);

    void        ExecClpbrd(SfxRequest &rReq);
    void        StateClpbrd(SfxItemSet &rSet);

    void        ExecTransliteration(SfxRequest &);
    void                ExecRotateTransliteration(SfxRequest &);

    void        ExecUndo(SfxRequest &rReq);
    void        StateUndo(SfxItemSet &rSet);

    void        StateStatusLine(SfxItemSet &rSet);

    void        InsertSymbol(SfxRequest& rReq);

    void        ExecSearch(SfxRequest&, sal_Bool bNoMessage = sal_False);
    void            StateSearch(SfxItemSet &);


    virtual ::svl::IUndoManager*
                GetUndoManager();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
