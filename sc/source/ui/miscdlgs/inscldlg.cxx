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
