/*************************************************************************
 *
 *  $RCSfile: inscldlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "inscldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


static BYTE nInsItemChecked=0;

//==================================================================

ScInsertCellDlg::ScInsertCellDlg( Window* pParent,BOOL bDisallowCellMove) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_INSCELL ) ),
    //
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aBtnCellsDown   ( this, ScResId( BTN_CELLSDOWN ) ),
    aBtnCellsRight  ( this, ScResId( BTN_CELLSRIGHT ) ),
    aBtnInsRows     ( this, ScResId( BTN_INSROWS ) ),
    aBtnInsCols     ( this, ScResId( BTN_INSCOLS ) ),
    aGbFrame        ( this, ScResId( GB_FRAME ) )
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

__EXPORT ScInsertCellDlg::~ScInsertCellDlg()
{
}



