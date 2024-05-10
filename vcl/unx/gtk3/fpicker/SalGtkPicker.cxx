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

#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

#include <vcl/window.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkframe.hxx>
#include "SalGtkPicker.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

OUString SalGtkPicker::uritounicode(const gchar* pIn) const
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
        OUString aNewURL = uri::ExternalUriReferenceTranslator::create( m_xContext )->translateToInternal(sURL);
        if( !aNewURL.isEmpty() )
            sURL = aNewURL;
    }
    return sURL;
}

OString SalGtkPicker::unicodetouri(const OUString &rURL) const
{
    // all the URLs are handled by office in UTF-8 ( and encoded with "%xx" codes based on UTF-8 )
    // so the Gnome FP related URLs should be converted accordingly
    OString sURL = OUStringToOString(rURL, RTL_TEXTENCODING_UTF8);
    INetURLObject aURL(rURL);
    if (INetProtocol::File == aURL.GetProtocol())
    {
        OUString aNewURL = uri::ExternalUriReferenceTranslator::create( m_xContext )->translateToExternal(rURL);

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
    vcl::Window * pWindow = ::Application::GetActiveTopWindow();
    if (!pWindow)
        return nullptr;
    GtkSalFrame *pFrame = dynamic_cast<GtkSalFrame*>(pWindow->ImplGetFrame());
    if (!pFrame)
        return nullptr;
    return GTK_WINDOW(widget_get_toplevel(pFrame->getWindow()));
}

RunDialog::RunDialog(GtkWidget *pDialog, uno::Reference<awt::XExtendedToolkit> xToolkit,
                                         uno::Reference<frame::XDesktop> xDesktop)
    : cppu::WeakComponentImplHelper<awt::XTopWindowListener, frame::XTerminateListener>(maLock)
    , mpDialog(pDialog)
    , mbTerminateDesktop(false)
    , mxToolkit(std::move(xToolkit))
    , mxDesktop(std::move(xDesktop))
{
}

RunDialog::~RunDialog()
{
    SolarMutexGuard g;

    g_source_remove_by_user_data (this);
}

void SAL_CALL RunDialog::windowOpened(const css::lang::EventObject& e)
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

    g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, reinterpret_cast<GSourceFunc>(canceldialog), this, nullptr);
}

void SAL_CALL RunDialog::queryTermination( const css::lang::EventObject& )
{
    SolarMutexGuard g;

    g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, reinterpret_cast<GSourceFunc>(canceldialog), this, nullptr);

    mbTerminateDesktop = true;

    throw css::frame::TerminationVetoException();
}

void SAL_CALL RunDialog::notifyTermination( const css::lang::EventObject& )
{
}

void RunDialog::cancel()
{
    gtk_dialog_response( GTK_DIALOG( mpDialog ), GTK_RESPONSE_CANCEL );
    gtk_widget_hide( mpDialog );
}

namespace
{
    class ExecuteInfo
    {
    private:
        css::uno::Reference<css::frame::XDesktop> mxDesktop;
    public:
        ExecuteInfo(css::uno::Reference<css::frame::XDesktop> xDesktop)
            : mxDesktop(std::move(xDesktop))
        {
        }
        void terminate()
        {
            mxDesktop->terminate();
        }
    };
}

gint RunDialog::run()
{
    if (mxToolkit.is())
        mxToolkit->addTopWindowListener(this);

    mxDesktop->addTerminateListener(this);

    // [Inc/Dec]ModalCount on parent frame so it knows it is in modal mode
    GtkWindow* pParent = gtk_window_get_transient_for(GTK_WINDOW(mpDialog));
    GtkSalFrame* pFrame = pParent ? GtkSalFrame::getFromWindow(GTK_WIDGET(pParent)) : nullptr;
    VclPtr<vcl::Window> xFrameWindow = pFrame ? pFrame->GetWindow() : nullptr;
    if (xFrameWindow)
    {
        xFrameWindow->IncModalCount();
        xFrameWindow->ImplGetFrame()->NotifyModalHierarchy(true);
    }

    gint nStatus = gtk_dialog_run(GTK_DIALOG(mpDialog));

    if (xFrameWindow)
    {
        xFrameWindow->DecModalCount();
        xFrameWindow->ImplGetFrame()->NotifyModalHierarchy(false);
    }

    mxDesktop->removeTerminateListener(this);

    if (mxToolkit.is())
        mxToolkit->removeTopWindowListener(this);

    if (mbTerminateDesktop)
    {
        ExecuteInfo* pExecuteInfo = new ExecuteInfo(mxDesktop);
        Application::PostUserEvent(LINK(nullptr, RunDialog, TerminateDesktop), pExecuteInfo);
    }

    return nStatus;
}

IMPL_STATIC_LINK(RunDialog, TerminateDesktop, void*, p, void)
{
    ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
    pExecuteInfo->terminate();
    delete pExecuteInfo;
}

SalGtkPicker::SalGtkPicker( uno::Reference<uno::XComponentContext> xContext )
    : m_pParentWidget(nullptr)
    , m_pDialog(nullptr)
    , m_xContext(std::move(xContext))
{
}

SalGtkPicker::~SalGtkPicker()
{
    SolarMutexGuard g;

    if (m_pDialog)
    {
#if !GTK_CHECK_VERSION(4, 0, 0)
        gtk_widget_destroy(m_pDialog);
#else
        gtk_window_destroy(GTK_WINDOW(m_pDialog));
#endif
    }
}

void SalGtkPicker::implsetDisplayDirectory( const OUString& aDirectory )
{
    OSL_ASSERT( m_pDialog != nullptr );

    OString aTxt = unicodetouri(aDirectory);
    if( aTxt.isEmpty() ){
      aTxt = unicodetouri(u"file:///."_ustr);
    }

    if( aTxt.endsWith("/") )
        aTxt = aTxt.copy( 0, aTxt.getLength() - 1 );

    SAL_INFO( "vcl", "setting path to " << aTxt );

#if GTK_CHECK_VERSION(4, 0, 0)
    GFile* pPath = g_file_new_for_uri(aTxt.getStr());
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_pDialog), pPath, nullptr);
    g_object_unref(pPath);
#else
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(m_pDialog), aTxt.getStr());
#endif
}

OUString SalGtkPicker::implgetDisplayDirectory()
{
    OSL_ASSERT( m_pDialog != nullptr );

#if GTK_CHECK_VERSION(4, 0, 0)
    GFile* pPath =
        gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(m_pDialog));
    gchar* pCurrentFolder = g_file_get_uri(pPath);
    g_object_unref(pPath);
#else
    gchar* pCurrentFolder =
        gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(m_pDialog));
#endif
    OUString aCurrentFolderName = uritounicode(pCurrentFolder);
    g_free( pCurrentFolder );

    return aCurrentFolderName;
}

void SalGtkPicker::implsetTitle( std::u16string_view aTitle )
{
    OSL_ASSERT( m_pDialog != nullptr );

    OString aWindowTitle = OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 );

    gtk_window_set_title( GTK_WINDOW( m_pDialog ), aWindowTitle.getStr() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
