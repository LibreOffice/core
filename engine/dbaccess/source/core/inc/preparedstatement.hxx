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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include "statement.hxx"
#include "column.hxx"

namespace dbaccess
{

    //  OPreparedStatement

    class OPreparedStatement : public OStatementBase,
                               public css::sdbc::XPreparedStatement,
                               public css::sdbc::XParameters,
                               public css::sdbc::XResultSetMetaDataSupplier,
                               public css::sdbcx::XColumnsSupplier,
                               public css::lang::XServiceInfo
    {
        std::unique_ptr<OColumns>  m_pColumns;
        css::uno::Reference< css::sdbc::XParameters > m_xAggregateAsParameters;

    public:
        OPreparedStatement(const rtl::Reference< OConnection > & _xConn,
                           const css::uno::Reference< css::uno::XInterface > & _xStatement);
        virtual ~OPreparedStatement() override;

    // css::lang::XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
        virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // css::uno::XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

    // css::lang::XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // OComponentHelper
        virtual void disposing() override;

    // css::sdbc::XPreparedStatement
        virtual css::uno::Reference< css::sdbc::XResultSet > executeQuery(  ) override;
        virtual sal_Int32 executeUpdate(  ) override;
        virtual bool execute(  ) override;
        virtual css::uno::Reference< css::sdbc::XConnection > getConnection(  ) override;

    // css::sdbcx::XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > getColumns(  ) override;

    // css::sdbc::XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData(  ) override;

    // css::sdbc::XParameters
        virtual void setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) override;
        virtual void setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
        virtual void setBoolean( sal_Int32 parameterIndex, bool x ) override;
        virtual void setByte( sal_Int32 parameterIndex, sal_Int8 x ) override;
        virtual void setShort( sal_Int32 parameterIndex, sal_Int16 x ) override;
        virtual void setInt( sal_Int32 parameterIndex, sal_Int32 x ) override;
        virtual void setLong( sal_Int32 parameterIndex, sal_Int64 x ) override;
        virtual void setFloat( sal_Int32 parameterIndex, float x ) override;
        virtual void setDouble( sal_Int32 parameterIndex, double x ) override;
        virtual void setString( sal_Int32 parameterIndex, const OUString& x ) override;
        virtual void setBytes( sal_Int32 parameterIndex, const cpo::uno::Sequence< sal_Int8 >& x ) override;
        virtual void setDate( sal_Int32 parameterIndex, const css::util::Date& x ) override;
        virtual void setTime( sal_Int32 parameterIndex, const css::util::Time& x ) override;
        virtual void setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x ) override;
        virtual void setBinaryStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void setCharacterStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void setObject( sal_Int32 parameterIndex, const cpo::uno::Any& x ) override;
        virtual void setObjectWithInfo( sal_Int32 parameterIndex, const cpo::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) override;
        virtual void setRef( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef >& x ) override;
        virtual void setBlob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob >& x ) override;
        virtual void setClob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob >& x ) override;
        virtual void setArray( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray >& x ) override;
        virtual void clearParameters(  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
