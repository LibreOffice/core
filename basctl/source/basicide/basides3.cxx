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

#include <basidesh.hrc>

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
#include <sfx2/dispatch.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/request.hxx>
#include <tools/diagnose_ex.h>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

DialogWindow* BasicIDEShell::CreateDlgWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rDlgName )
{
    bCreatingWindow = true;

    sal_uLong nKey = 0;
    DialogWindow* pWin = 0;
    ::rtl::OUString aLibName( rLibName );
    ::rtl::OUString aDlgName( rDlgName );

    if ( aLibName.isEmpty() )
        aLibName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Standard"));

    rDocument.getOrCreateLibrary( E_DIALOGS, aLibName );

    if ( aDlgName.isEmpty() )
        aDlgName = rDocument.createObjectName( E_DIALOGS, aLibName );

    // maybe there's a suspended one?
    pWin = FindDlgWin( rDocument, aLibName, aDlgName, false, true );

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
                ::xmlscript::importDialogModel( xInput, xDialogModel, xContext, rDocument.isDocument() ? rDocument.getDocument() : Reference< frame::XModel >() );
                LocalizationMgr::setStringResourceAtDialog( rDocument, rLibName, aDlgName, xDialogModel );

                // new dialog window
                pWin = new DialogWindow( &GetViewFrame()->GetWindow(), rDocument, aLibName, aDlgName, xDialogModel );
                nKey = InsertWindowInTable( pWin );
            }
        }
        catch (const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    else
    {
        pWin->SetStatus( pWin->GetStatus() & ~BASWIN_SUSPENDED );
        nKey = GetIDEWindowId( pWin );
        DBG_ASSERT( nKey, "CreateDlgWin: Kein Key - Fenster nicht gefunden!" );
    }

    if( pWin )
    {
        pWin->GrabScrollBars( &aHScrollBar, &aVScrollBar );
        pTabBar->InsertPage( (sal_uInt16)nKey, aDlgName );
        pTabBar->Sort();
        if ( !pCurWin )
            SetCurWindow( pWin, false, false );
    }

    bCreatingWindow = false;
    return pWin;
}

DialogWindow* BasicIDEShell::FindDlgWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rDlgName, bool bCreateIfNotExist, bool bFindSuspended )
{
    DialogWindow* pDlgWin = 0;
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
    {
        IDEBaseWindow* pWin = it->second;
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( DialogWindow ) ) )
        {
            if ( rLibName.isEmpty() )
            {
                pDlgWin = (DialogWindow*)pWin;
                break;
            }
            else if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName && pWin->GetName() == rDlgName )
            {
                pDlgWin = (DialogWindow*)pWin;
                break;
            }
        }
    }
    if ( !pDlgWin && bCreateIfNotExist )
        pDlgWin = CreateDlgWin( rDocument, rLibName, rDlgName );

    return pDlgWin;
}

sal_uInt16 BasicIDEShell::GetIDEWindowId(const IDEBaseWindow* pWin) const
{
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
        if ( it->second == pWin )
            return it->first;
    return 0;
}

SdrView* BasicIDEShell::GetCurDlgView() const
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( DialogWindow ) ) )
        return NULL;

    DialogWindow* pWin = (DialogWindow*)pCurWin;
    return pWin->GetView();
}

// only if dialogue window above:
void BasicIDEShell::ExecuteDialog( SfxRequest& rReq )
{
    if ( pCurWin && ( pCurWin->IsA( TYPE( DialogWindow) ) ||
        (rReq.GetSlot() == SID_IMPORT_DIALOG &&pCurWin->IsA( TYPE( ModulWindow) ) ) ) )
    {
        pCurWin->ExecuteCommand( rReq );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
