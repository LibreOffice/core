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

#include <com/sun/star/sdbc/XConnection.hpp>
#include <connectivity/ConnectionWrapper.hxx>

namespace connectivity
{


    // OConnectionWeakWrapper - wraps all methods to the real connection from the driver
    // but when disposed it doesn't dispose the real connection

    typedef ::cppu::ImplInheritanceHelper<OConnectionWrapper, css::sdbc::XConnection> OConnectionWeakWrapper_BASE;

    class OConnectionWeakWrapper : public OConnectionWeakWrapper_BASE
    {
    protected:
        // OComponentHelper
        virtual ~OConnectionWeakWrapper() override;
    public:
        explicit OConnectionWeakWrapper(css::uno::Reference< css::uno::XAggregation >& _xConnection);

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
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
