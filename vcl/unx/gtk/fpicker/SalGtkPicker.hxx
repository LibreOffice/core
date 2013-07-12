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

#ifndef _SALGTKFPICKER_HXX_
#define _SALGTKFPICKER_HXX_

#include <osl/mutex.hxx>
#include <cppuhelper/compbase2.hxx>

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
                 SalGtkPicker( const ::com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xContext );
        virtual ~SalGtkPicker();
    protected:
        osl::Mutex m_rbHelperMtx;
        GtkWidget  *m_pDialog;
    protected:
        virtual void SAL_CALL implsetTitle( const OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL implsetDisplayDirectory( const OUString& rDirectory )
            throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

        virtual OUString SAL_CALL implgetDisplayDirectory(  )
            throw( com::sun::star::uno::RuntimeException );
        OUString uritounicode(const gchar *pIn);
        OString unicodetouri(const OUString &rURL);

        // to instantiate own services
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

        OUString getResString( sal_Int32 aId );
    private:
        void setGtkLanguage();
};

class GdkThreadLock
{
public:
    GdkThreadLock() { gdk_threads_enter(); }
    ~GdkThreadLock() { gdk_threads_leave(); }
};

//Run the Gtk Dialog. Watch for any "new windows" created while we're
//executing and consider that a CANCEL event to avoid e.g. "file cannot be opened"
//modal dialogs and this one getting locked if some other API call causes this
//to happen while we're opened waiting for user input, e.g.
//https://bugzilla.redhat.com/show_bug.cgi?id=441108
class RunDialog :
    public cppu::WeakComponentImplHelper2<
        ::com::sun::star::awt::XTopWindowListener,
        ::com::sun::star::frame::XTerminateListener >
{
private:
    osl::Mutex maLock;
    GtkWidget *mpDialog;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XExtendedToolkit>  mxToolkit;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop >  mxDesktop;
public:

    // XTopWindowListener
    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& )
        throw(::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
        throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
        throw(::com::sun::star::uno::RuntimeException);
public:
    RunDialog(GtkWidget *pDialog,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XExtendedToolkit > &rToolkit,
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop > &rDesktop
        );
    virtual ~RunDialog();
    gint run();
    void cancel();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
