/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SalGtkPicker.hxx,v $
 * $Revision: 1.10 $
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

//_____________________________________________________________________________
//  includes of other projects
//_____________________________________________________________________________

#include <osl/mutex.hxx>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>

#ifndef _COM_SUN_STAR_UI_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalGtkPicker
{
    public:
        SalGtkPicker() : m_pDialog(0) {}
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
        static rtl::OUString uritounicode(const gchar *pIn);
        static rtl::OString unicodetouri(const rtl::OUString &rURL);
};

class RunDialog
{
private:
    bool mbFinished;
    osl::Mutex maLock;
    gint mnStatus;
    GtkWidget *m_pDialog;
public:
    void run();
    RunDialog(GtkWidget *pDialog) : mbFinished(false), m_pDialog(pDialog) { }
    gint runandwaitforresult();
};

#endif
