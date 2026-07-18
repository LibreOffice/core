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

#include <odbc/odbcbasedllapi.hxx>
#include <odbc/OStatement.hxx>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase5.hxx>

namespace connectivity::odbc
{
    class OResultSetMetaData;
    class OBoundParam;
    typedef ::cppu::ImplHelper5<    css::sdbc::XPreparedStatement,
                                    css::sdbc::XParameters,
                                    css::sdbc::XPreparedBatchExecution,
                                    css::sdbc::XResultSetMetaDataSupplier,
                                    css::lang::XServiceInfo> OPreparedStatement_BASE;

    class OPreparedStatement final :
                                public  OStatement_BASE2,
                                public  OPreparedStatement_BASE
    {
        static const short invalid_scale = -1;

        // Data attributes

        SQLSMALLINT     numParams;      // Number of parameter markers for the prepared statement

        std::unique_ptr<OBoundParam[]>  boundParams;
                        // Array of bound parameter objects. Each parameter marker will have a
                        // corresponding object to hold bind information, and resulting data.
        rtl::Reference<OResultSetMetaData>  m_xMetaData;
        bool                                                                        m_bPrepared;

        void FreeParams();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void putParamData (sal_Int32 index);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setStream (sal_Int32 ParameterIndex,const css::uno::Reference< css::io::XInputStream>& x,
                                                    SQLLEN length,sal_Int32 SQLtype);
        SQLLEN* getLengthBuf (sal_Int32 index);
        void* allocBindBuf (    sal_Int32 index,    sal_Int32 bufLen);
        /// @throws css::sdbc::SQLException
        void initBoundParam ();
        void setParameterPre(sal_Int32 parameterIndex);
        template <typename T> void setScalarParameter(sal_Int32 parameterIndex, sal_Int32 _nType, SQLULEN _nColumnSize, const T i_Value);
        template <typename T> void setScalarParameter(sal_Int32 parameterIndex, sal_Int32 _nType, SQLULEN _nColumnSize, sal_Int32 _nScale, const T i_Value);
        void setParameter(sal_Int32 parameterIndex, sal_Int32 _nType, SQLULEN _nColumnSize, sal_Int32 _nScale, const void* _pData, SQLULEN _nDataLen, SQLLEN _nDataAllocLen);
        // Wrappers for special cases
        void setParameter(sal_Int32 parameterIndex, sal_Int32 _nType, sal_Int16 _nScale, const OUString &_sData);
        void setParameter(sal_Int32 parameterIndex, sal_Int32 _nType, const cpo::uno::Sequence< sal_Int8 > &Data);

        bool isPrepared() const { return m_bPrepared;}
        void prepareStatement();
        void checkParameterIndex(sal_Int32 _parameterIndex);

        /**
            creates the driver specific resultset (factory)
        */
        virtual rtl::Reference<OResultSet> createResultSet() override;

        virtual void setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                               const cpo::uno::Any& rValue) override;
    public:
        DECLARE_SERVICE_INFO();
        // A ctor, needed to return the object
        OPreparedStatement( OConnection* _pConnection,const OUString& sql);
        virtual ~OPreparedStatement() override;
        OPreparedStatement& operator=( OPreparedStatement const & ) = delete; // MSVC2015 workaround
        OPreparedStatement( OPreparedStatement const & ) = delete; // MSVC2015 workaround

        //XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;
        //XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;

        // XPreparedStatement
        virtual css::uno::Reference< css::sdbc::XResultSet > executeQuery(  ) override;
        virtual sal_Int32 executeUpdate(  ) override;
        virtual bool execute(  ) override;
        virtual css::uno::Reference< css::sdbc::XConnection > getConnection(  ) override;
        // XParameters
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
        // XPreparedBatchExecution
        virtual void addBatch(  ) override;
        virtual void clearBatch(  ) override;
        virtual cpo::uno::Sequence< sal_Int32 > executeBatch(  ) override;
        // XCloseable
        virtual void close(  ) override;
        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData(  ) override;
        const rtl::Reference< OResultSetMetaData > & getMetaDataImpl();

    public:
        using OStatement_Base::executeQuery;
        using OStatement_Base::executeUpdate;
        using OStatement_Base::execute;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
