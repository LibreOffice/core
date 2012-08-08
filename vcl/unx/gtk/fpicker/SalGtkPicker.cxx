/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <rtl/process.h>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <stdio.h>

#include "vcl/window.hxx"
#include "unx/gtk/gtkframe.hxx"
#include "gtk/fpicker/SalGtkPicker.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

rtl::OUString SalGtkPicker::uritounicode(const gchar* pIn)
{
    if (!pIn)
        return rtl::OUString();

    rtl::OUString sURL( const_cast<const sal_Char *>(pIn), strlen(pIn),
        RTL_TEXTENCODING_UTF8 );

    INetURLObject aURL(sURL);
    if (INET_PROT_FILE == aURL.GetProtocol())
    {
        // all the URLs are handled by office in UTF-8
        // so the Gnome FP related URLs should be converted accordingly
        gchar *pEncodedFileName = g_filename_from_uri(pIn, NULL, NULL);
        if ( pEncodedFileName )
        {
            rtl::OUString sEncoded(pEncodedFileName, strlen(pEncodedFileName),
                osl_getThreadTextEncoding());
            INetURLObject aCurrentURL(sEncoded, INetURLObject::FSYS_UNX);
            aCurrentURL.SetHost(aURL.GetHost());
            sURL = aCurrentURL.getExternalURL();
        }
        else
        {
            OUString aNewURL = uno::Reference<uri::XExternalUriReferenceTranslator>(uno::Reference<XMultiComponentFactory>(comphelper::getProcessServiceFactory(), UNO_QUERY_THROW)->createInstanceWithContext(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uri.ExternalUriReferenceTranslator")), m_xContext), UNO_QUERY_THROW)->translateToInternal(sURL);
            if( !aNewURL.isEmpty() )
                sURL = aNewURL;
        }
    }
    return sURL;
}

rtl::OString SalGtkPicker::unicodetouri(const rtl::OUString &rURL)
{
    // all the URLs are handled by office in UTF-8 ( and encoded with "%xx" codes based on UTF-8 )
    // so the Gnome FP related URLs should be converted accordingly
    OString sURL = OUStringToOString(rURL, RTL_TEXTENCODING_UTF8);
    INetURLObject aURL(rURL);
    if (INET_PROT_FILE == aURL.GetProtocol())
    {
        OUString aNewURL = uno::Reference<uri::XExternalUriReferenceTranslator>(uno::Reference<XMultiComponentFactory>(comphelper::getProcessServiceFactory(), UNO_QUERY_THROW)->createInstanceWithContext(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uri.ExternalUriReferenceTranslator")), m_xContext ), UNO_QUERY_THROW)->translateToExternal( rURL );

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
        GdkThreadLock lock;
        pDialog->cancel();
        return false;
    }
}

RunDialog::RunDialog( GtkWidget *pDialog, uno::Reference< awt::XExtendedToolkit >& rToolkit,
    uno::Reference< frame::XDesktop >& rDesktop ) :
    cppu::WeakComponentImplHelper2< awt::XTopWindowListener, frame::XTerminateListener >( maLock ),
    mpDialog(pDialog), mxToolkit(rToolkit), mxDesktop(rDesktop)
{
    GtkWindow *pParent = NULL;

    ::Window * pWindow = ::Application::GetActiveTopWindow();
    if( pWindow )
    {
        GtkSalFrame *pFrame = dynamic_cast<GtkSalFrame *>( pWindow->ImplGetFrame() );
        if( pFrame )
            pParent = GTK_WINDOW( pFrame->getWindow() );
    }
    if (pParent)
        gtk_window_set_transient_for( GTK_WINDOW( mpDialog ), pParent );
}

RunDialog::~RunDialog()
{
    SolarMutexGuard g;

    g_source_remove_by_user_data (this);
}

void SAL_CALL RunDialog::windowOpened( const ::com::sun::star::lang::EventObject& )
    throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard g;

    g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, (GSourceFunc)canceldialog, this, NULL);
}

void SAL_CALL RunDialog::queryTermination( const ::com::sun::star::lang::EventObject& )
        throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL RunDialog::notifyTermination( const ::com::sun::star::lang::EventObject& )
        throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard g;

    g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, (GSourceFunc)canceldialog, this, NULL);
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

// FIXME: this is pretty nasty ... we try to tell gtk+'s
// gettext the locale it should use via the environment
void SalGtkPicker::setGtkLanguage()
{
    static bool bSet = false;
    if (bSet)
        return;

    ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
    rtl::OUStringBuffer aBuffer;
    aBuffer.append( aLocale.Language );
    aBuffer.appendAscii( "_" );
    aBuffer.append( aLocale.Country );
    aBuffer.appendAscii( ".UTF-8" );

    if (aBuffer.getLength() > 8)
    {
        rtl::OUString envVar( RTL_CONSTASCII_USTRINGPARAM( "LANGUAGE" ) );
        osl_setEnvironment( envVar.pData, aBuffer.makeStringAndClear().pData );
    }
    bSet = true;
}

SalGtkPicker::SalGtkPicker( const uno::Reference<uno::XComponentContext>& xContext )
    : m_pDialog( 0 ), m_xContext( xContext )
{
    setGtkLanguage();
}

SalGtkPicker::~SalGtkPicker()
{
    SolarMutexGuard g;

    if (m_pDialog)
    {
        gtk_widget_destroy(m_pDialog);
    }
}

void SAL_CALL SalGtkPicker::implsetDisplayDirectory( const rtl::OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );

    OString aTxt = unicodetouri(aDirectory);

    if( aTxt.lastIndexOf('/') == aTxt.getLength() - 1 )
        aTxt = aTxt.copy( 0, aTxt.getLength() - 1 );

    OSL_TRACE( "setting path to %s", aTxt.getStr() );

    gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ),
        aTxt.getStr() );
}

rtl::OUString SAL_CALL SalGtkPicker::implgetDisplayDirectory() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );

    gchar* pCurrentFolder =
        gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ) );
    ::rtl::OUString aCurrentFolderName = uritounicode(pCurrentFolder);
    g_free( pCurrentFolder );

    return aCurrentFolderName;
}

void SAL_CALL SalGtkPicker::implsetTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );

    ::rtl::OString aWindowTitle = OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 );

    gtk_window_set_title( GTK_WINDOW( m_pDialog ), aWindowTitle.getStr() );
}

uno::Reference< uno::XInterface > SalGtkPicker::createInstance( const rtl::OUString &rName )
{
    return m_xContext->getServiceManager()->createInstanceWithContext( rName, m_xContext );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
