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

#ifndef INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_CONNECTIONTOOLS_HXX
#define INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_CONNECTIONTOOLS_HXX

#include "module_sdbt.hxx"

#include "connectiondependent.hxx"

#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>

namespace sdbtools
{

    // ConnectionTools
    typedef ::cppu::WeakImplHelper<   css::sdb::tools::XConnectionTools
                                  ,   css::lang::XServiceInfo
                                  ,   css::lang::XInitialization
                                  >   ConnectionTools_Base;
    /** implements the css::sdb::tools::XConnectionTools functionality
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
        explicit ConnectionTools( const css::uno::Reference< css::uno::XComponentContext >& _rContext );

        // XConnectionTools
        virtual css::uno::Reference< css::sdb::tools::XTableName > SAL_CALL createTableName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Reference< css::sdb::tools::XObjectNames > SAL_CALL getObjectNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Reference< css::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, css::uno::Reference< css::lang::XComponent >& keepFieldsAlive ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Reference< css::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const OUString& command ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo - static versions
        static OUString SAL_CALL getImplementationName_static();
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
                        Create(const css::uno::Reference< css::uno::XComponentContext >&);

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception, std::exception) SAL_OVERRIDE;

    protected:
        virtual ~ConnectionTools();

    private:
        ConnectionTools( const ConnectionTools& ) = delete;
        ConnectionTools& operator=( const ConnectionTools& ) = delete;
    };

} // namespace sdbtools

#endif // INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_CONNECTIONTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
