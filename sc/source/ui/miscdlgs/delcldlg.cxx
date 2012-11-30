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
