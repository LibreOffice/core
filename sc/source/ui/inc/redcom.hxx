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

#ifndef SC_REDCOM_HXX
#define SC_REDCOM_HXX

#include "chgtrack.hxx"

class ScDocShell;
class AbstractSvxPostItDialog;

class ScRedComDialog
{
private:

    ScChangeAction  *pChangeAction;
    ScDocShell      *pDocShell;
    String          aComment;
    AbstractSvxPostItDialog* pDlg;

    DECL_LINK( PrevHdl, AbstractSvxPostItDialog* );
    DECL_LINK( NextHdl, AbstractSvxPostItDialog* );

protected:

    void    ReInit(ScChangeAction *);
    void    SelectCell();

    ScChangeAction *FindPrev(ScChangeAction *pAction);
    ScChangeAction *FindNext(ScChangeAction *pAction);

public:

    ScRedComDialog( Window* pParent, const SfxItemSet& rCoreSet,
                    ScDocShell *,ScChangeAction *,sal_Bool bPrevNext = false);
    ~ScRedComDialog();

    short Execute();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
