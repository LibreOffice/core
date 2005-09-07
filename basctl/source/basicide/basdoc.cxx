/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basdoc.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:54:15 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#define GLOBALOVERFLOW2

#include <sfx2/docfac.hxx>

#ifndef _SV_STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif

#include <svx/xmlsecctrl.hxx>

#include <basdoc.hxx>

#define BasicDocShell
#include <basslots.hxx>

#include "basicmod.hxx"
#include "unomodel.hxx"

TYPEINIT1(BasicDocShell, SfxObjectShell);
DBG_NAME(BasicDocShell);

SFX_IMPL_OBJECTFACTORY( BasicDocShell, SvGlobalName(), SFXOBJECTSHELL_STD_NORMAL, "sbasic" )

SFX_IMPL_INTERFACE( BasicDocShell, SfxObjectShell, IDEResId( 0 ) )
{
    SFX_STATUSBAR_REGISTRATION( SID_BASICIDE_STATUSBAR );
}

BasicDocShell::BasicDocShell( SfxObjectCreateMode eMode ) : SfxObjectShell( eMode )
{
    pPrinter = 0;
    SetPool( &SFX_APP()->GetPool() );
    SetModel( new SIDEModel(this) );
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

    // signatures
    rStatusBar.InsertItem( SID_SIGNATURE, XmlSecStatusBarControl::GetDefItemWidth( rStatusBar ), SIB_USERDRAW );
    rStatusBar.SetHelpId(SID_SIGNATURE, SID_SIGNATURE);

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

void BasicDocShell::FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               sal_Int32 nVersion ) const
{}

void BasicDocShell::Draw( OutputDevice *, const JobSetup & rSetup,
                          USHORT nAspect )
{}

