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

#ifndef _SALGTKFPICKER_HXX_
#define _SALGTKFPICKER_HXX_

#include <osl/mutex.hxx>
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#define FOLDERPICKER_TITLE            500
#define FOLDER_PICKER_DEF_DESCRIPTION 501
#define FILE_PICKER_TITLE_OPEN        502
#define FILE_PICKER_TITLE_SAVE        503
#define FILE_PICKER_FILE_TYPE         504
#define FILE_PICKER_OVERWRITE         505
#define FILE_PICKER_ALLFORMATS        506

class SalGtkPicker
{
    public:
                 SalGtkPicker( const ::com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& xContext );
        virtual ~SalGtkPicker();
    protected:
        osl::Mutex m_rbHelperMtx;
        GtkWidget  *m_pDialog;
    protected:
        virtual void SAL_CALL implsetTitle( const ::rtl::OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL implsetDisplayDirectory( const rtl::OUString& rDirectory )
            throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL implgetDisplayDirectory(  )
            throw( com::sun::star::uno::RuntimeException );
        rtl::OUString uritounicode(const gchar *pIn);
        rtl::OString unicodetouri(const rtl::OUString &rURL);

        // to instantiate own services
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > createInstance( const rtl::OUString &rName );

        rtl::OUString getResString( sal_Int32 aId );
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
