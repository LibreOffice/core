/*************************************************************************
 *
 *  $RCSfile: teamdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
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

#include <vcl/system.hxx>
#include <tools/shl.hxx>

#include "teamdlg.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "sc.hrc"


//------------------------------------------------------------------

void ShowTheTeam()
{
    SC_MOD()->OpenTeamDlg();
}

//========================================================================
//  ScTeamDlg
//========================================================================

ScTeamDlg::ScTeamDlg( Window* pParent )
    : FloatingWindow( pParent, ScResId( RID_SCDLG_TEAM ) ),
      aBmpTeam      ( this, ScResId( 1 ) )
{
    FreeResource();
    SC_MOD()->SetTeamDlg( this );

    Bitmap aBmp( ScResId( RID_SCTEAMDLGBMP1 ) );

    Size    aSize   = aBmp.GetSizePixel();
    USHORT  nOff    = (USHORT)aBmpTeam.GetPosPixel().X();

    aBmpTeam.SetSizePixel( aSize );
    aBmpTeam.SetBitmap( aBmp );
    aSize.Width()  += (2*nOff);
    aSize.Height() += (2*nOff);
    SetOutputSizePixel( aSize );
    Center();
    Show();
}

//------------------------------------------------------------------------

__EXPORT ScTeamDlg::~ScTeamDlg()
{
}

//------------------------------------------------------------------------

BOOL __EXPORT ScTeamDlg::Close()
{
    BOOL bReturn = FloatingWindow::Close();

    SC_MOD()->SetTeamDlg( NULL );
    delete this;

    return bReturn;
}

//------------------------------------------------------------------------

void ScTeamDlg::Center()
{
    if ( IsRollUp() ) RollDown();

    Size  aSizeDesktop  = GetParent()->GetSizePixel();
    Size  aSize         = GetSizePixel();
    Point aNewPos;

    aNewPos.X() = (aSizeDesktop.Width()  - aSize.Width())  / 2;
    aNewPos.Y() = (aSizeDesktop.Height() - aSize.Height()) / 2;
    SetPosPixel( aNewPos );
}



