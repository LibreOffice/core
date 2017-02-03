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

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <config_gio.h>

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <osl/diagnose.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include "unx/gtk/gtkinst.hxx"
#include "gtk/fpicker/SalGtkFolderPicker.hxx"

#include <string.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

// constructor

SalGtkFolderPicker::SalGtkFolderPicker( const uno::Reference< uno::XComponentContext >& xContext ) :
    SalGtkPicker( xContext )
{
    m_pDialog = gtk_file_chooser_dialog_new(
        OUStringToOString( getResString( FOLDERPICKER_TITLE ), RTL_TEXTENCODING_UTF8 ).getStr(),
        nullptr, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
        GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, nullptr );

    gtk_dialog_set_default_response( GTK_DIALOG (m_pDialog), GTK_RESPONSE_ACCEPT );
#if ENABLE_GIO
    gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER( m_pDialog ), false );
#endif
    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( m_pDialog ), false );
}

void SAL_CALL SalGtkFolderPicker::setDisplayDirectory( const OUString& aDirectory )
{
    SolarMutexGuard g;

    assert( m_pDialog != nullptr );

    OString aTxt = unicodetouri( aDirectory );
    if( aTxt.isEmpty() ){
      aTxt = unicodetouri("file:///.");
    }

    if( aTxt.endsWith("/") )
        aTxt = aTxt.copy( 0, aTxt.getLength() - 1 );

    SAL_INFO( "vcl", "setting path to " << aTxt );

    gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ),
        aTxt.getStr() );
}

OUString SAL_CALL SalGtkFolderPicker::getDisplayDirectory()
{
    SolarMutexGuard g;

    assert( m_pDialog != nullptr );

    gchar* pCurrentFolder =
        gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ) );
    OUString aCurrentFolderName = uritounicode(pCurrentFolder);
    g_free( pCurrentFolder );

    return aCurrentFolderName;
}

OUString SAL_CALL SalGtkFolderPicker::getDirectory()
{
    SolarMutexGuard g;

    assert( m_pDialog != nullptr );

    gchar* pSelectedFolder =
        gtk_file_chooser_get_uri( GTK_FILE_CHOOSER( m_pDialog ) );
    OUString aSelectedFolderName = uritounicode(pSelectedFolder);
    g_free( pSelectedFolder );

    return aSelectedFolderName;
}

void SAL_CALL SalGtkFolderPicker::setDescription( const OUString& /*rDescription*/ )
{
}

// XExecutableDialog functions

void SAL_CALL SalGtkFolderPicker::setTitle( const OUString& aTitle )
{
    SolarMutexGuard g;

    assert( m_pDialog != nullptr );

    OString aWindowTitle = OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 );

    gtk_window_set_title( GTK_WINDOW( m_pDialog ), aWindowTitle.getStr() );
}

sal_Int16 SAL_CALL SalGtkFolderPicker::execute()
{
    SolarMutexGuard g;

    assert( m_pDialog != nullptr );

    sal_Int16 retVal = 0;

    uno::Reference< awt::XExtendedToolkit > xToolkit(
        awt::Toolkit::create(m_xContext),
        uno::UNO_QUERY);

    uno::Reference< frame::XDesktop > xDesktop( frame::Desktop::create(m_xContext), uno::UNO_QUERY);

    GtkWindow *pParent = RunDialog::GetTransientFor();
    if (pParent)
        gtk_window_set_transient_for(GTK_WINDOW(m_pDialog), pParent);
    RunDialog* pRunDialog = new RunDialog(m_pDialog, xToolkit);
    uno::Reference < awt::XTopWindowListener > xLifeCycle(pRunDialog);
    gint nStatus = pRunDialog->run();
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
    gtk_widget_hide(m_pDialog);

    return retVal;
}

// XCancellable

void SAL_CALL SalGtkFolderPicker::cancel()
{
    SolarMutexGuard g;

    assert( m_pDialog != nullptr );

    // TODO m_pImpl->cancel();
}

uno::Reference< ui::dialogs::XFolderPicker2 >
GtkInstance::createFolderPicker( const uno::Reference< uno::XComponentContext > &xMSF )
{
    return uno::Reference< ui::dialogs::XFolderPicker2 >(
                new SalGtkFolderPicker( xMSF ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
