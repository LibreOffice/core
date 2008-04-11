/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SalGtkFolderPicker.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <osl/diagnose.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <FPServiceInfo.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include "SalGtkFolderPicker.hxx"

#include <tools/urlobj.hxx>

#include <iostream>
#include "resourceprovider.hxx"
#ifndef _SV_RC_H
#include <tools/rc.hxx>
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controling event notifications
    uno::Sequence<rtl::OUString> SAL_CALL FolderPicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(2);
        aRet[0] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.SystemFolderPicker" );
        aRet[1] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.GtkFolderPicker" );
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------------------
SalGtkFolderPicker::SalGtkFolderPicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr ) :
    m_xServiceMgr( xServiceMgr )
{
    CResourceProvider aResProvider;
    m_pDialog = gtk_file_chooser_dialog_new(
        OUStringToOString( aResProvider.getResString( FOLDERPICKER_TITLE ), RTL_TEXTENCODING_UTF8 ).getStr(),
        NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, (char *)NULL );

    gtk_dialog_set_default_response( GTK_DIALOG (m_pDialog), GTK_RESPONSE_ACCEPT );
    gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER( m_pDialog ), FALSE );
    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( m_pDialog ), FALSE );
}

// -------------------------------------------------
// XEventListener
// -------------------------------------------------

void SAL_CALL SalGtkFolderPicker::disposing( const lang::EventObject& )
    throw( uno::RuntimeException )
{
}

void SAL_CALL SalGtkFolderPicker::setDisplayDirectory( const rtl::OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OString aTxt = unicodetouri( aDirectory );

    if( aTxt.lastIndexOf('/') == aTxt.getLength() - 1 )
        aTxt = aTxt.copy( 0, aTxt.getLength() - 1 );

    OSL_TRACE( "setting path to %s\n", aTxt.getStr() );

    gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ),
                         aTxt.getStr() );
}

rtl::OUString SAL_CALL SalGtkFolderPicker::getDisplayDirectory() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    gchar* pCurrentFolder =
        gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ) );
    ::rtl::OUString aCurrentFolderName = uritounicode(pCurrentFolder);
    g_free( pCurrentFolder );

    return aCurrentFolderName;
}

rtl::OUString SAL_CALL SalGtkFolderPicker::getDirectory() throw( uno::RuntimeException )
{
    return getDisplayDirectory();
}

void SAL_CALL SalGtkFolderPicker::setDescription( const rtl::OUString& rDescription )
    throw( uno::RuntimeException )
{
    ::rtl::OString aDescription = OUStringToOString( rDescription, RTL_TEXTENCODING_UTF8 );
}



//-----------------------------------------------------------------------------------------
// XExecutableDialog functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFolderPicker::setTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ::rtl::OString aWindowTitle = OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 );
    gtk_window_set_title( GTK_WINDOW( m_pDialog ), aWindowTitle.getStr() );
}

sal_Int16 SAL_CALL SalGtkFolderPicker::execute() throw( uno::RuntimeException )
{
    OSL_TRACE( "1: HERE WE ARE\n");
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_Int16 retVal = 0;

        RunDialog aRunInMain(m_pDialog);
        gint nStatus = aRunInMain.runandwaitforresult();
    switch( nStatus )
    {
        case GTK_RESPONSE_ACCEPT:
            retVal = ExecutableDialogResults::OK;
            break;
        case GTK_RESPONSE_CANCEL:
            retVal = ExecutableDialogResults::CANCEL;
            break;
        default:
            retVal = 0;
            break;
    }

    return retVal;
}

//------------------------------------------------------------------------------------
// XCancellable
//------------------------------------------------------------------------------------

void SAL_CALL SalGtkFolderPicker::cancel() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO m_pImpl->cancel();
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

rtl::OUString SAL_CALL SalGtkFolderPicker::getImplementationName()
    throw( uno::RuntimeException )
{
    return rtl::OUString::createFromAscii( FOLDER_PICKER_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL SalGtkFolderPicker::supportsService( const rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence <rtl::OUString> SupportedServicesNames = FolderPicker_getSupportedServiceNames();

    for( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if( SupportedServicesNames[n].compareTo( ServiceName ) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL SalGtkFolderPicker::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return FolderPicker_getSupportedServiceNames();
}
