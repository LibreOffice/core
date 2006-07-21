/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: teamdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:11:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

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
    Point aPos = GetPosPixel();
    if (aPos.Y() < 0)
    {
        //  #87164# title bar must be visible
        aPos.Y() = 0;
        SetPosPixel(aPos);
    }
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



