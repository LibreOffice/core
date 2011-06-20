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


#ifndef ADABASUI_NEWDB_HXX
#define ADABASUI_NEWDB_HXX

#include <svtools/genericunodialog.hxx>
#include "Acomponentmodule.hxx"
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>



//.........................................................................
namespace adabasui
{
//.........................................................................
    class OAdabasNewDbDlg;
    typedef ::svt::OGenericUnoDialog OAdabasCreateDialogBase;
    class OAdabasCreateDialog
            :public OAdabasCreateDialogBase
            ,public ::comphelper::OPropertyArrayUsageHelper< OAdabasCreateDialog >
            ,public OModuleResourceClient
    {
    protected:
        OAdabasNewDbDlg* m_pDialog; // just to avoid a cast
        // <properties>
        ::rtl::OUString m_sDatabaseName;
        ::rtl::OUString m_sControlUser;
        ::rtl::OUString m_sControlPassword;
        ::rtl::OUString m_sUser;
        ::rtl::OUString m_sUserPassword;
        sal_Int32       m_nCacheSize;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XCreateCatalog> m_xCreateCatalog;
        // </properties>
    protected:
        OAdabasCreateDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

    public:
        // XTypeProvider
        virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

        // XServiceInfo - static methods
        static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( com::sun::star::uno::RuntimeException );
        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
                SAL_CALL Create(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
        virtual void executedDialog(sal_Int16 _nExecutionResult);
    };
//.........................................................................
}   // namespace adabasui
//.........................................................................

#endif // ADABASUI_NEWDB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
