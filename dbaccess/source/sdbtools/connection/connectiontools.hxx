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

#ifndef DBACCESS_CONNECTIONTOOLS_HXX
#define DBACCESS_CONNECTIONTOOLS_HXX

#include "module_sdbt.hxx"

#include "connectiondependent.hxx"

#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase3.hxx>

namespace sdbtools
{

    // ConnectionTools
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
        ConnectionTools( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rContext );

        // XConnectionTools
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XTableName > SAL_CALL createTableName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XObjectNames > SAL_CALL getObjectNames() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& keepFieldsAlive ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const OUString& command ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService(const OUString & ServiceName) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static OUString SAL_CALL getImplementationName_static();
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
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

} // namespace sdbtools

#endif // DBACCESS_CONNECTIONTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
