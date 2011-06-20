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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "inscldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


static sal_uInt8 nInsItemChecked=0;

//==================================================================

ScInsertCellDlg::ScInsertCellDlg( Window* pParent,sal_Bool bDisallowCellMove) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_INSCELL ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aBtnCellsDown   ( this, ScResId( BTN_CELLSDOWN ) ),
    aBtnCellsRight  ( this, ScResId( BTN_CELLSRIGHT ) ),
    aBtnInsRows     ( this, ScResId( BTN_INSROWS ) ),
    aBtnInsCols     ( this, ScResId( BTN_INSCOLS ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    if (bDisallowCellMove)
    {
        aBtnCellsDown.Disable();
        aBtnCellsRight.Disable();
        aBtnInsRows.Check();

        switch(nInsItemChecked)
        {
            case 2: aBtnInsRows   .Check();break;
            case 3: aBtnInsCols   .Check();break;
            default:aBtnInsRows   .Check();break;
        }
    }
    else
    {
        switch(nInsItemChecked)
        {
            case 0: aBtnCellsDown .Check();break;
            case 1: aBtnCellsRight.Check();break;
            case 2: aBtnInsRows   .Check();break;
            case 3: aBtnInsCols   .Check();break;
        }
    }
    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

InsCellCmd ScInsertCellDlg::GetInsCellCmd() const
{
    InsCellCmd nReturn = INS_NONE;

    if ( aBtnCellsDown.IsChecked() )
    {
        nInsItemChecked=0;
        nReturn = INS_CELLSDOWN;
    }
    else if ( aBtnCellsRight.IsChecked())
    {
        nInsItemChecked=1;
        nReturn = INS_CELLSRIGHT;
    }
    else if ( aBtnInsRows.IsChecked()   )
    {
        nInsItemChecked=2;
        nReturn = INS_INSROWS;
    }
    else if ( aBtnInsCols.IsChecked()   )
    {
        nInsItemChecked=3;
        nReturn = INS_INSCOLS;
    }

    return nReturn;
}

ScInsertCellDlg::~ScInsertCellDlg()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
