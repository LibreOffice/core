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
#ifndef _SWDRWTXTSH_HXX
#define _SWDRWTXTSH_HXX

#include <sfx2/shell.hxx>
#include "shellid.hxx"
#include <unotools/caserotate.hxx>

class SdrView;
class SwView;
class SwWrtShell;
class SfxModule;

class SwDrawTextShell: public SfxShell
{
    SwView      &rView;
    RotateTransliteration m_aRotateCase;

    SdrView     *pSdrView;

    sal_Bool        bRotate : 1;
    sal_Bool        bSelMove: 1;

    void        SetAttrToMarked(const SfxItemSet& rAttr);
    void        InsertSymbol(SfxRequest& rReq);
    sal_Bool        IsTextEdit();
public:
    SFX_DECL_INTERFACE(SW_DRWTXTSHELL)
    TYPEINFO();

    SwView     &GetView() { return rView; }
    SwWrtShell &GetShell();

                SwDrawTextShell(SwView &rView);
    virtual     ~SwDrawTextShell();

    virtual ::svl::IUndoManager*
                GetUndoManager();

    void        StateDisableItems(SfxItemSet &);

    void        Execute(SfxRequest &);
    void        ExecDraw(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        GetDrawTxtCtrlState(SfxItemSet&);

    void        ExecFontWork(SfxRequest& rReq);
    void        StateFontWork(SfxItemSet& rSet);
    void        ExecFormText(SfxRequest& rReq);
    void        GetFormTextState(SfxItemSet& rSet);
    void        ExecDrawLingu(SfxRequest &rReq);
    void        ExecUndo(SfxRequest &rReq);
    void        StateUndo(SfxItemSet &rSet);
    void        ExecClpbrd(SfxRequest &rReq);
    void        StateClpbrd(SfxItemSet &rSet);
    void        StateInsert(SfxItemSet &rSet);
    void        ExecTransliteration(SfxRequest &);
    void                ExecRotateTransliteration(SfxRequest &);

    void        Init();
    void        StateStatusline(SfxItemSet &rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
