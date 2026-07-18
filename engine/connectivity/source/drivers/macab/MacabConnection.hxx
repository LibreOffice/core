/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <connectivity/CommonTools.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <cppuhelper/compbase.hxx>
#include <TConnection.hxx>

namespace connectivity::macab
{

    typedef ::cppu::WeakComponentImplHelper<css::sdbc::XConnection,
                                            css::sdbc::XWarningsSupplier,
                                            css::lang::XServiceInfo
                                        > OMetaConnection_BASE;

    class MacabDriver;
    class MacabAddressBook;

    typedef connectivity::OMetaConnection MacabConnection_BASE;

    class MacabConnection : public MacabConnection_BASE
    {
    protected:

        // Data attributes

        MacabAddressBook*                       m_pAddressBook; // the address book
        MacabDriver*                            m_pDriver;      // pointer to the owning driver object
        css::uno::Reference< css::sdbcx::XTablesSupplier>
                                                m_xCatalog;     // needed for the SQL interpreter

    private:
        bool doIsClosed();

        void doClose();

    public:
        /// @throws css::sdbc::SQLException
        virtual void construct( const OUString& url,const cpo::uno::Sequence< css::beans::PropertyValue >& info);

        explicit MacabConnection(MacabDriver* _pDriver);
        virtual ~MacabConnection() override;

        // OComponentHelper
        virtual void disposing() override;

        // XServiceInfo
        DECLARE_SERVICE_INFO();

        // XConnection
        virtual css::uno::Reference< css::sdbc::XStatement > createStatement(  ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareStatement( const OUString& sql ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareCall( const OUString& sql ) override;
        virtual OUString nativeSQL( const OUString& sql ) override;
        virtual void setAutoCommit( bool autoCommit ) override;
        virtual bool getAutoCommit(  ) override;
        virtual void commit(  ) override;
        virtual void rollback(  ) override;
        virtual bool isClosed(  ) override;
        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > getMetaData(  ) override;
        virtual void setReadOnly( bool readOnly ) override;
        virtual bool isReadOnly(  ) override;
        virtual void setCatalog( const OUString& catalog ) override;
        virtual OUString getCatalog(  ) override;
        virtual void setTransactionIsolation( sal_Int32 level ) override;
        virtual sal_Int32 getTransactionIsolation(  ) override;
        virtual css::uno::Reference< css::container::XNameAccess > getTypeMap(  ) override;
        virtual void setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

        // XCloseable
        virtual void close(  ) override;

        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;

        // needed for the SQL interpreter
        css::uno::Reference< css::sdbcx::XTablesSupplier > createCatalog();

        // accessors
        MacabDriver*         getDriver()         const { return m_pDriver;}
               MacabAddressBook* getAddressBook()   const;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
