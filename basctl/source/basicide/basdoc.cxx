/*************************************************************************
 *
 *  $RCSfile: basdoc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:35 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#define GLOBALOVERFLOW2

#include <sfx2/docfac.hxx>

#ifndef _SV_STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif

#include <basdoc.hxx>
#include <basidesh.hrc>

#define BasicDocShell
#include <baside.hxx>


SFX_IMPL_SIMPLE_OBJECTFACTORY( BasicDocShell, SFXOBJECTSHELL_STD_SPECIAL | SFXOBJECTSHELL_DONTREPLACE, side3 )
{
    Factory().SetExplorerImageId( IMG_SFX_EXP_MACROLIB );
    Factory().RegisterHelpFile( String( RTL_CONSTASCII_USTRINGPARAM( "sbasic" ) ) );
}


SFX_IMPL_INTERFACE( BasicDocShell, SfxObjectShell, IDEResId( 0 ) )
{
    SFX_STATUSBAR_REGISTRATION( SID_BASICIDE_STATUSBAR );
//  SFX_CHILDWINDOW_REGISTRATION( SID_SHOW_BROWSER );
}



BasicDocShell::BasicDocShell( SfxObjectCreateMode eMode ) : SfxObjectShell( eMode )
{
    pPrinter = 0;
    SetPool( &SFX_APP()->GetPool() );
}




__EXPORT BasicDocShell::~BasicDocShell()
{
    delete pPrinter;
}




void __EXPORT BasicDocShell::Execute( SfxRequest& rReq )
{
}




void __EXPORT BasicDocShell::GetState(SfxItemSet &rSet)
{
}




SfxPrinter* BasicDocShell::GetPrinter( BOOL bCreate )
{
    if ( !pPrinter && bCreate )
        pPrinter = new SfxPrinter( new SfxItemSet( GetPool(), SID_PRINTER_NOTFOUND_WARN , SID_PRINTER_NOTFOUND_WARN ) );

    return pPrinter;
}




void BasicDocShell::SetPrinter( SfxPrinter* pPr )
{
    if ( pPr != pPrinter )
    {
        delete pPrinter;
        pPrinter = pPr;
    }
}


void BasicDocShell::FillStatusBar( StatusBar& rStatusBar )
{
    String aTmp;

    // Titel
    aTmp.Fill( 30, 'X' );
    rStatusBar.InsertItem( SID_BASICIDE_STAT_TITLE,
        rStatusBar.GetTextWidth( aTmp ), SIB_AUTOSIZE | SIB_LEFT);

    // Modify
    rStatusBar.InsertItem( SID_DOC_MODIFIED,
        rStatusBar.GetTextWidth( '*' ) );

    // Position
    aTmp.Erase();
    aTmp.Fill( 15, 'X' );
    rStatusBar.InsertItem( SID_BASICIDE_STAT_POS,
        rStatusBar.GetTextWidth( aTmp ), SIB_LEFT);

    // Insert/Overwrite
    rStatusBar.InsertItem( SID_ATTR_INSERT,
        rStatusBar.GetTextWidth( String( RTL_CONSTASCII_USTRINGPARAM( "XXXXX" /* "EINFG" */ ) ) ) );

    // Uhrzeit
    aTmp.Fill( 20, 'X' );
    rStatusBar.InsertItem( SID_ATTR_SIZE,
        rStatusBar.GetTextWidth( aTmp ), SIB_AUTOSIZE | SIB_LEFT | SIB_USERDRAW );

//  return pStatusBar;

}


