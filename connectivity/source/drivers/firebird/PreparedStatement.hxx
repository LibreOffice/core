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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_PREPAREDSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_PREPAREDSTATEMENT_HXX

#include "Statement.hxx"

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <ibase.h>

namespace connectivity
{
    namespace firebird
    {

        class OBoundParam;
        typedef ::cppu::ImplHelper <    ::com::sun::star::sdbc::XPreparedStatement,
                                        ::com::sun::star::sdbc::XParameters,
                                        ::com::sun::star::sdbc::XPreparedBatchExecution,
                                        ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                        ::com::sun::star::lang::XServiceInfo> OPreparedStatement_Base;

        class OPreparedStatement :  public  OStatementCommonBase,
                                    public  OPreparedStatement_Base
        {
        protected:
            struct Parameter
            {
                ::com::sun::star::uno::Any  aValue;
                sal_Int32                   nDataType;

                Parameter(const ::com::sun::star::uno::Any& rValue,
                          sal_Int32                         rDataType) : aValue(rValue),nDataType(rDataType)
                {
                }

            };

            ::std::vector< Parameter>       m_aParameters;

            TTypeInfoVector                 m_aTypeInfo;    // Hashtable containing an entry
                                                                        //  for each row returned by
                                                                        //  DatabaseMetaData.getTypeInfo.

            ::rtl::OUString                                                                 m_sSqlStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xMetaData;

            XSQLDA*         m_pOutSqlda;
            XSQLDA*         m_pInSqlda;
            void checkParameterIndex(sal_Int32 nParameterIndex)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);

            /**
             * Set a numeric value in the input SQLDA. If the destination
             * parameter is not of nType then an Exception will be thrown.
             */
            template <typename T> void setValue(sal_Int32 nIndex, T& nValue, ISC_SHORT nType)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);
            void setParameterNull(sal_Int32 nParameterIndex, bool bSetNull = true);

            void ensurePrepared()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);
            /**
             * Assumes that all necessary mutexes have been taken.
             */
            void openBlobForWriting(isc_blob_handle& rBlobHandle, ISC_QUAD& rBlobId);
            /**
             * Assumes that all necessary mutexes have been taken.
             */
            void closeBlobAfterWriting(isc_blob_handle& rBlobHandle);

        protected:
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                                   const ::com::sun::star::uno::Any& rValue)
                                                                        throw (::com::sun::star::uno::Exception, std::exception) override;
            virtual ~OPreparedStatement();
        public:
            DECLARE_SERVICE_INFO();
            // a constructor, which is required for returning objects:
            OPreparedStatement( Connection* _pConnection,
                                const TTypeInfoVector& _TypeInfo,
                                const ::rtl::OUString& sql);

            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

            // XPreparedStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL
                executeQuery()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL
                executeUpdate()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL
                execute()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL
                getConnection()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XParameters
            virtual void SAL_CALL setNull(sal_Int32 nIndex, sal_Int32 nValue)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setBoolean( sal_Int32 nIndex, sal_Bool nValue)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setByte(sal_Int32 nIndex, sal_Int8 nValue)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setShort(sal_Int32 nIndex, sal_Int16 nValue)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setInt(sal_Int32 nIndex, sal_Int32 nValue)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setLong(sal_Int32 nIndex, sal_Int64 nValue)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setBytes( sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setBinaryStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setCharacterStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setObjectWithInfo( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setRef( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setBlob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setClob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setArray( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearParameters(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XPreparedBatchExecution -- UNSUPPORTED by firebird
            virtual void SAL_CALL
                addBatch()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL
                clearBatch()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
                executeBatch()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XCloseable
            virtual void SAL_CALL close()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_PREPAREDSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
