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

#include "delcldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


static sal_uInt8 nDelItemChecked=0;

//==================================================================

ScDeleteCellDlg::ScDeleteCellDlg( Window* pParent, sal_Bool bDisallowCellMove ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DELCELL ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aBtnCellsUp     ( this, ScResId( BTN_CELLSUP ) ),
    aBtnCellsLeft   ( this, ScResId( BTN_CELLSLEFT ) ),
    aBtnDelRows     ( this, ScResId( BTN_DELROWS ) ),
    aBtnDelCols     ( this, ScResId( BTN_DELCOLS ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{

    if (bDisallowCellMove)
    {
        aBtnCellsUp.Disable();
        aBtnCellsLeft.Disable();

        switch(nDelItemChecked)
        {
            case 2: aBtnDelRows.Check();break;
            case 3: aBtnDelCols.Check();break;
            default:aBtnDelRows.Check();break;
        }
    }
    else
    {
        switch(nDelItemChecked)
        {
            case 0: aBtnCellsUp.Check();break;
            case 1: aBtnCellsLeft.Check();break;
            case 2: aBtnDelRows.Check();break;
            case 3: aBtnDelCols.Check();break;
        }
    }

    FreeResource();
}

//------------------------------------------------------------------------

DelCellCmd ScDeleteCellDlg::GetDelCellCmd() const
{
    DelCellCmd nReturn = DEL_NONE;

    if ( aBtnCellsUp.IsChecked()   )
    {
        nDelItemChecked=0;
        nReturn = DEL_CELLSUP;
    }
    else if ( aBtnCellsLeft.IsChecked() )
    {
        nDelItemChecked=1;
        nReturn = DEL_CELLSLEFT;
    }
    else if ( aBtnDelRows.IsChecked()   )
    {
        nDelItemChecked=2;
        nReturn = DEL_DELROWS;
    }
    else if ( aBtnDelCols.IsChecked()   )
    {
        nDelItemChecked=3;
        nReturn = DEL_DELCOLS;
    }

    return nReturn;
}

ScDeleteCellDlg::~ScDeleteCellDlg()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
