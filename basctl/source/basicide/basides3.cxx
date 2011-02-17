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
#include "precompiled_basctl.hxx"

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#include <basidesh.hrc>
#include <ide_pch.hxx>


#define _SOLAR__PRIVATE 1

#include <basidesh.hxx>
#include <baside2.hxx>
#include <baside3.hxx>
#include <basobj.hxx>
#include <localizationmgr.hxx>
#include <dlgedview.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <xmlscript/xmldlg_imexp.hxx>
#include <tools/diagnose_ex.h>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;


DialogWindow* BasicIDEShell::CreateDlgWin( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName )
{
    bCreatingWindow = sal_True;

    sal_uLong nKey = 0;
    DialogWindow* pWin = 0;
    String aLibName( rLibName );
    String aDlgName( rDlgName );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );

    rDocument.getOrCreateLibrary( E_DIALOGS, aLibName );

    if ( !aDlgName.Len() )
        aDlgName = rDocument.createObjectName( E_DIALOGS, aLibName );

    // Vielleicht gibt es ein suspendiertes?
    pWin = FindDlgWin( rDocument, aLibName, aDlgName, sal_False, sal_True );

    if ( !pWin )
    {
        try
        {
            Reference< io::XInputStreamProvider > xISP;
            if ( rDocument.hasDialog( aLibName, aDlgName ) )
                rDocument.getDialog( aLibName, aDlgName, xISP );
            else
                rDocument.createDialog( aLibName, aDlgName, xISP );

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
                LocalizationMgr::setStringResourceAtDialog( rDocument, rLibName, aDlgName, xDialogModel );

                // new dialog window
                pWin = new DialogWindow( &GetViewFrame()->GetWindow(), rDocument, aLibName, aDlgName, xDialogModel );
                nKey = InsertWindowInTable( pWin );
            }
        }
        catch ( uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
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
        pTabBar->InsertPage( (sal_uInt16)nKey, aDlgName );
        pTabBar->Sort();
        if ( !pCurWin )
            SetCurWindow( pWin, sal_False, sal_False );
    }

    bCreatingWindow = sal_False;
    return pWin;
}

DialogWindow* BasicIDEShell::FindDlgWin( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName, sal_Bool bCreateIfNotExist, sal_Bool bFindSuspended )
{
    DialogWindow* pDlgWin = 0;
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin && !pDlgWin )
    {
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( DialogWindow ) ) )
        {
            if ( !rLibName.Len() )  // nur irgendeins finden...
                pDlgWin = (DialogWindow*)pWin;
            else if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName && pWin->GetName() == rDlgName )
                pDlgWin = (DialogWindow*)pWin;
        }
        pWin = aIDEWindowTable.Next();
    }
    if ( !pDlgWin && bCreateIfNotExist )
        pDlgWin = CreateDlgWin( rDocument, rLibName, rDlgName );

    return pDlgWin;
}

SdrView* BasicIDEShell::GetCurDlgView() const
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( DialogWindow ) ) )
        return NULL;

    DialogWindow* pWin = (DialogWindow*)pCurWin;
    return pWin->GetView();
}

// Nur wenn Dialogfenster oben:
void __EXPORT BasicIDEShell::ExecuteDialog( SfxRequest& rReq )
{
    if ( pCurWin && ( pCurWin->IsA( TYPE( DialogWindow) ) ||
        (rReq.GetSlot() == SID_IMPORT_DIALOG &&pCurWin->IsA( TYPE( ModulWindow) ) ) ) )
    {
        pCurWin->ExecuteCommand( rReq );
    }
}

