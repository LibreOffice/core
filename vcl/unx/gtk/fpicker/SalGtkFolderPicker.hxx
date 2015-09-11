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

#ifndef INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKFOLDERPICKER_HXX
#define INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKFOLDERPICKER_HXX

#include <list>
#include <memory>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include "gtk/fpicker/SalGtkPicker.hxx"

class SalGtkFolderPicker :
        public SalGtkPicker,
    public cppu::WeakImplHelper<
        ::com::sun::star::ui::dialogs::XFolderPicker2 >
{
    public:

        // constructor
        SalGtkFolderPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xServiceMgr );

        // XExecutableDialog functions

        virtual void SAL_CALL setTitle( const OUString& aTitle )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual sal_Int16 SAL_CALL execute(  )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XFolderPicker functions

        virtual void SAL_CALL setDisplayDirectory( const OUString& rDirectory )
            throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual OUString SAL_CALL getDisplayDirectory(  )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual OUString SAL_CALL getDirectory( )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        virtual void SAL_CALL setDescription( const OUString& rDescription )
            throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XCancellable

        virtual void SAL_CALL cancel( )
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    private:
        SalGtkFolderPicker( const SalGtkFolderPicker& ) SAL_DELETED_FUNCTION;
        SalGtkFolderPicker& operator=( const SalGtkFolderPicker& ) SAL_DELETED_FUNCTION;
};

#endif // INCLUDED_VCL_UNX_GTK_FPICKER_SALGTKFOLDERPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
