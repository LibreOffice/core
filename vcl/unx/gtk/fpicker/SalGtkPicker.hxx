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

#ifndef INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKPICKER_HXX
#define INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKPICKER_HXX

#include <osl/mutex.hxx>
#include <cppuhelper/compbase.hxx>

#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#define FOLDERPICKER_TITLE              500
#define FOLDER_PICKER_DEF_DESCRIPTION   501
#define FILE_PICKER_TITLE_OPEN          502
#define FILE_PICKER_TITLE_SAVE          503
#define FILE_PICKER_FILE_TYPE           504
#define FILE_PICKER_OVERWRITE_PRIMARY   505
#define FILE_PICKER_OVERWRITE_SECONDARY 506
#define FILE_PICKER_ALLFORMATS          507

class SalGtkPicker
{
    public:
                 SalGtkPicker( const css::uno::Reference<css::uno::XComponentContext>& xContext );
        virtual ~SalGtkPicker();
    protected:
        osl::Mutex m_rbHelperMtx;
        GtkWidget  *m_pDialog;
    protected:
        void SAL_CALL implsetTitle( const OUString& aTitle )
            throw( css::uno::RuntimeException );

        void SAL_CALL implsetDisplayDirectory( const OUString& rDirectory )
            throw( css::lang::IllegalArgumentException, css::uno::RuntimeException );

        OUString SAL_CALL implgetDisplayDirectory(  )
            throw( css::uno::RuntimeException );
        OUString uritounicode(const gchar *pIn);
        OString unicodetouri(const OUString &rURL);

        // to instantiate own services
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        static OUString getResString( sal_Int32 aId );
};

//Run the Gtk Dialog. Watch for any "new windows" created while we're
//executing and consider that a CANCEL event to avoid e.g. "file cannot be opened"
//modal dialogs and this one getting locked if some other API call causes this
//to happen while we're opened waiting for user input, e.g.
//https://bugzilla.redhat.com/show_bug.cgi?id=441108
class RunDialog :
    public cppu::WeakComponentImplHelper<
        css::awt::XTopWindowListener,
        css::frame::XTerminateListener >
{
private:
    osl::Mutex maLock;
    GtkWidget *mpDialog;
    css::uno::Reference< css::awt::XExtendedToolkit>  mxToolkit;
    css::uno::Reference< css::frame::XDesktop >  mxDesktop;
public:

    // XTopWindowListener
    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const css::lang::EventObject& )
        throw(css::uno::RuntimeException, std::exception) override {}
    virtual void SAL_CALL windowOpened( const css::lang::EventObject& e )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowClosing( const css::lang::EventObject& )
        throw (css::uno::RuntimeException, std::exception) override {}
    virtual void SAL_CALL windowClosed( const css::lang::EventObject& )
        throw (css::uno::RuntimeException, std::exception) override {}
    virtual void SAL_CALL windowMinimized( const css::lang::EventObject& )
        throw (css::uno::RuntimeException, std::exception) override {}
    virtual void SAL_CALL windowNormalized( const css::lang::EventObject& )
        throw (css::uno::RuntimeException, std::exception) override {}
    virtual void SAL_CALL windowActivated( const css::lang::EventObject& )
        throw (css::uno::RuntimeException, std::exception) override {}
    virtual void SAL_CALL windowDeactivated( const css::lang::EventObject& )
        throw (css::uno::RuntimeException, std::exception) override {}

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent )
        throw(css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent )
        throw(css::uno::RuntimeException, std::exception) override;
public:
    RunDialog(GtkWidget *pDialog,
        css::uno::Reference< css::awt::XExtendedToolkit > &rToolkit,
        css::uno::Reference< css::frame::XDesktop > &rDesktop
        );
    virtual ~RunDialog();
    gint run();
    void cancel();
    static GtkWindow* GetTransientFor();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
