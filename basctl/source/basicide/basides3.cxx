/*************************************************************************
 *
 *  $RCSfile: basides3.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:02:38 $
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

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#include <basidesh.hrc>
#include <ide_pch.hxx>

#pragma hdrstop

#define _SOLAR__PRIVATE 1

#include <basidesh.hxx>
#include <baside3.hxx>
#include <basobj.hxx>

#ifndef _BASCTL_DLGEDVIEW_HXX
#include <dlgedview.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#include <xmlscript/xmldlg_imexp.hxx>
#endif

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;


DialogWindow* BasicIDEShell::CreateDlgWin( SfxObjectShell* pShell, const String& rLibName, const String& rDlgName )
{
    bCreatingWindow = TRUE;

    ULONG nKey = 0;
    DialogWindow* pWin = 0;
    String aLibName( rLibName );
    String aDlgName( rDlgName );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );

    if ( !BasicIDE::HasDialogLibrary( pShell, aLibName ) )
        BasicIDE::CreateDialogLibrary( pShell, aLibName );

    if ( !aDlgName.Len() )
        aDlgName = BasicIDE::CreateDialogName( pShell, aLibName );

    // Vielleicht gibt es ein suspendiertes?
    pWin = FindDlgWin( pShell, aLibName, aDlgName, FALSE, TRUE );

    if ( !pWin )
    {
        try
        {
            Reference< io::XInputStreamProvider > xISP;
            if ( BasicIDE::HasDialog( pShell, aLibName, aDlgName ) )
            {
                // get dialog
                xISP = BasicIDE::GetDialog( pShell, aLibName, aDlgName );
            }
            else
            {
                // create dialog
                xISP = BasicIDE::CreateDialog( pShell, aLibName, aDlgName );
            }

            if ( xISP.is() )
            {
                // create dialog model
                Reference< lang::XMultiServiceFactory > xMSF = getProcessServiceFactory();
                Reference< container::XNameContainer > xDialogModel( xMSF->createInstance
                    ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), UNO_QUERY );
                Reference< XInputStream > xInput( xISP->createInputStream() );
                Reference< XComponentContext > xContext;
                Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
                OSL_ASSERT( xProps.is() );
                OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
                ::xmlscript::importDialogModel( xInput, xDialogModel, xContext );

                // new dialog window
                pWin = new DialogWindow( &GetViewFrame()->GetWindow(), pShell, aLibName, aDlgName, xDialogModel );
                nKey = InsertWindowInTable( pWin );
            }
        }
        catch ( container::ElementExistException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }
        catch ( container::NoSuchElementException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }
    }
    else
    {
        pWin->SetStatus( pWin->GetStatus() & ~BASWIN_SUSPENDED );
        IDEBaseWindow* pTmp = aIDEWindowTable.First();
        while ( pTmp && !nKey )
        {
            if ( pTmp == pWin )
                nKey = aIDEWindowTable.GetCurKey();
            pTmp = aIDEWindowTable.Next();
        }
        DBG_ASSERT( nKey, "CreateDlgWin: Kein Key - Fenster nicht gefunden!" );
    }

    if( pWin )
    {
        pWin->GrabScrollBars( &aHScrollBar, &aVScrollBar );
        pTabBar->InsertPage( (USHORT)nKey, aDlgName );
        pTabBar->Sort();
        if ( !pCurWin )
            SetCurWindow( pWin, FALSE, FALSE );
    }

    bCreatingWindow = FALSE;
    return pWin;
}

DialogWindow* BasicIDEShell::FindDlgWin( SfxObjectShell* pShell, const String& rLibName, const String& rDlgName, BOOL bCreateIfNotExist, BOOL bFindSuspended )
{
    DialogWindow* pDlgWin = 0;
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin && !pDlgWin )
    {
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( DialogWindow ) ) )
        {
            if ( !rLibName.Len() )  // nur irgendeins finden...
                pDlgWin = (DialogWindow*)pWin;
            else if ( pWin->GetShell() == pShell && pWin->GetLibName() == rLibName && pWin->GetName() == rDlgName )
                pDlgWin = (DialogWindow*)pWin;
        }
        pWin = aIDEWindowTable.Next();
    }
    if ( !pDlgWin && bCreateIfNotExist )
        pDlgWin = CreateDlgWin( pShell, rLibName, rDlgName );

    return pDlgWin;
}

SdrView* BasicIDEShell::GetCurDlgView()
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( DialogWindow ) ) )
        return NULL;

    DialogWindow* pWin = (DialogWindow*)pCurWin;
    return pWin->GetView();
}

// Nur wenn Dialogfenster oben:
void __EXPORT BasicIDEShell::ExecuteDialog( SfxRequest& rReq )
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( DialogWindow ) ) )
        return;

    pCurWin->ExecuteCommand( rReq );
}

