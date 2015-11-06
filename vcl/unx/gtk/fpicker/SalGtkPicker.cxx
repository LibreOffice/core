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

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/process.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

#include "vcl/window.hxx"
#include "unx/gtk/gtkframe.hxx"
#include "gtk/fpicker/SalGtkPicker.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

OUString SalGtkPicker::uritounicode(const gchar* pIn)
{
    if (!pIn)
        return OUString();

    OUString sURL( pIn, strlen(pIn),
        RTL_TEXTENCODING_UTF8 );

    INetURLObject aURL(sURL);
    if (INetProtocol::File == aURL.GetProtocol())
    {
        // all the URLs are handled by office in UTF-8
        // so the Gnome FP related URLs should be converted accordingly
        gchar *pEncodedFileName = g_filename_from_uri(pIn, NULL, NULL);
        if ( pEncodedFileName )
        {
            OUString sEncoded(pEncodedFileName, strlen(pEncodedFileName),
                osl_getThreadTextEncoding());
            g_free (pEncodedFileName);
            INetURLObject aCurrentURL(sEncoded, INetURLObject::FSYS_UNX);
            aCurrentURL.SetHost(aURL.GetHost());
            sURL = aCurrentURL.getExternalURL();
        }
        else
        {
            OUString aNewURL = uri::ExternalUriReferenceTranslator::create( m_xContext )->translateToInternal(sURL);
            if( !aNewURL.isEmpty() )
                sURL = aNewURL;
        }
    }
    return sURL;
}

OString SalGtkPicker::unicodetouri(const OUString &rURL)
{
    // all the URLs are handled by office in UTF-8 ( and encoded with "%xx" codes based on UTF-8 )
    // so the Gnome FP related URLs should be converted accordingly
    OString sURL = OUStringToOString(rURL, RTL_TEXTENCODING_UTF8);
    INetURLObject aURL(rURL);
    if (INetProtocol::File == aURL.GetProtocol())
    {
        OUString aNewURL = uri::ExternalUriReferenceTranslator::create( m_xContext )->translateToInternal(rURL);

        if( !aNewURL.isEmpty() )
        {
            // At this point the URL should contain ascii characters only actually
            sURL = OUStringToOString( aNewURL, osl_getThreadTextEncoding() );
        }
    }
    return sURL;
}

extern "C"
{
    static gboolean canceldialog(RunDialog *pDialog)
    {
        SolarMutexGuard g;
        pDialog->cancel();
        return false;
    }
}

GtkWindow* RunDialog::GetTransientFor()
{
    GtkWindow *pParent = NULL;

    vcl::Window * pWindow = ::Application::GetActiveTopWindow();
    if( pWindow )
    {
        GtkSalFrame *pFrame = dynamic_cast<GtkSalFrame *>( pWindow->ImplGetFrame() );
        if( pFrame )
            pParent = GTK_WINDOW( pFrame->getWindow() );
    }

    return pParent;
}

RunDialog::RunDialog( GtkWidget *pDialog, uno::Reference< awt::XExtendedToolkit >& rToolkit,
    uno::Reference< frame::XDesktop >& rDesktop ) :
    cppu::WeakComponentImplHelper< awt::XTopWindowListener, frame::XTerminateListener >( maLock ),
    mpDialog(pDialog), mxToolkit(rToolkit), mxDesktop(rDesktop)
{
}

RunDialog::~RunDialog()
{
    SolarMutexGuard g;

    g_source_remove_by_user_data (this);
}

void SAL_CALL RunDialog::windowOpened(const css::lang::EventObject& e)
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    //Don't popdown dialogs if a tooltip appears elsewhere, that's ok, but do pop down
    //if another dialog/frame is launched.
    css::uno::Reference<css::accessibility::XAccessible> xAccessible(e.Source, css::uno::UNO_QUERY);
    if (xAccessible.is())
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext(xAccessible->getAccessibleContext());
        if (xContext.is() && xContext->getAccessibleRole() == css::accessibility::AccessibleRole::TOOL_TIP)
        {
            return;
        }
    }

    g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, reinterpret_cast<GSourceFunc>(canceldialog), this, NULL);
}

void SAL_CALL RunDialog::queryTermination( const ::com::sun::star::lang::EventObject& )
        throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException, std::exception)
{
}

void SAL_CALL RunDialog::notifyTermination( const ::com::sun::star::lang::EventObject& )
        throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, reinterpret_cast<GSourceFunc>(canceldialog), this, NULL);
}

void RunDialog::cancel()
{
    gtk_dialog_response( GTK_DIALOG( mpDialog ), GTK_RESPONSE_CANCEL );
    gtk_widget_hide( mpDialog );
}

gint RunDialog::run()
{
    if (mxToolkit.is())
        mxToolkit->addTopWindowListener(this);

    gint nStatus = gtk_dialog_run( GTK_DIALOG( mpDialog ) );

    if (mxToolkit.is())
        mxToolkit->removeTopWindowListener(this);

    if (nStatus != 1)   //PLAY
        gtk_widget_hide( mpDialog );

    return nStatus;
}

SalGtkPicker::SalGtkPicker( const uno::Reference<uno::XComponentContext>& xContext )
    : m_pDialog( 0 ), m_xContext( xContext )
{
}

SalGtkPicker::~SalGtkPicker()
{
    SolarMutexGuard g;

    if (m_pDialog)
    {
        gtk_widget_destroy(m_pDialog);
    }
}

void SAL_CALL SalGtkPicker::implsetDisplayDirectory( const OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );

    OString aTxt = unicodetouri(aDirectory);
    if( aTxt.isEmpty() ){
      aTxt = unicodetouri("file:///.");
    }

    if( aTxt.endsWith("/") )
        aTxt = aTxt.copy( 0, aTxt.getLength() - 1 );

    OSL_TRACE( "setting path to %s", aTxt.getStr() );

    gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ),
        aTxt.getStr() );
}

OUString SAL_CALL SalGtkPicker::implgetDisplayDirectory() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );

    gchar* pCurrentFolder =
        gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ) );
    OUString aCurrentFolderName = uritounicode(pCurrentFolder);
    g_free( pCurrentFolder );

    return aCurrentFolderName;
}

void SAL_CALL SalGtkPicker::implsetTitle( const OUString& aTitle ) throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );

    OString aWindowTitle = OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 );

    gtk_window_set_title( GTK_WINDOW( m_pDialog ), aWindowTitle.getStr() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
