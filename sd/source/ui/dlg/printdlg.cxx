/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printdlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:41:45 $
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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif


#include "sdresid.hxx"
#include "printdlg.hxx"
#include "printdlg.hrc"

/*************************************************************************
|*
|*    SdPrintDlg::SdPrintDlg()
|*
|*    Beschreibung  Dialog zum Einstellen von Printoptionen
|*
*************************************************************************/

SdPrintDlg::SdPrintDlg( Window* pWindow ) :
    ModalDialog     ( pWindow, SdResId( DLG_PRINT_WARNINGS ) ),
    aFtInfo         ( this, SdResId( FI_INFO ) ),
    aRbtScale       ( this, SdResId( RBT_SCALE ) ),
    aRbtPoster      ( this, SdResId( RBT_POSTER ) ),
    aRbtCut         ( this, SdResId( RBT_CUT ) ),
    aGrpOptions     ( this, SdResId( GRP_OPTIONS ) ),
    aBtnOK          ( this, SdResId( BTN_OK ) ),
    aBtnCancel      ( this, SdResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, SdResId( BTN_HELP ) )
{
    FreeResource();

    aRbtScale.Check();
}

/*************************************************************************
|*
|*    SdPrintDlg::GetAttr()
|*
|*    Beschreibung  Liefert eingestellte Option zurueck
|*
*************************************************************************/

USHORT SdPrintDlg::GetAttr()
{
    USHORT nOption = 0;

    if( aRbtScale.IsChecked() )
        nOption = 1;
    else if( aRbtPoster.IsChecked() )
        nOption = 2;
    else if( aRbtCut.IsChecked() )
        nOption = 3;

    return( nOption );
}




