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

#ifndef DBACCESS_CONNECTIONTOOLS_HXX
#define DBACCESS_CONNECTIONTOOLS_HXX

#include "module_sdbt.hxx"

#include "connectiondependent.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase3.hxx>

#include <comphelper/componentcontext.hxx>

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= ConnectionTools
    //====================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::sdb::tools::XConnectionTools
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::lang::XInitialization
                                    >   ConnectionTools_Base;
    /** implements the com::sun::star::sdb::tools::XConnectionTools functionality
    */
    class ConnectionTools   :public ConnectionTools_Base
                            ,public ConnectionDependentComponent
    {
    private:
        SdbtClient                      m_aModuleClient;

    public:
        /** constructs a ConnectionTools instance

            @param _rxContext
                the context of the component
        */
        ConnectionTools( const ::comphelper::ComponentContext& _rContext );

        // XConnectionTools
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XTableName > SAL_CALL createTableName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XObjectNames > SAL_CALL getObjectNames() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const ::rtl::OUString& command, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& keepFieldsAlive ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const ::rtl::OUString& command ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static ::rtl::OUString SAL_CALL getImplementationName_static();
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

        // XInitialization
        virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception);

    protected:
        ~ConnectionTools();

    private:
        ConnectionTools();                                      // never implemented
        ConnectionTools( const ConnectionTools& );              // never implemented
        ConnectionTools& operator=( const ConnectionTools& );   // never implemented
    };

//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_CONNECTIONTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
