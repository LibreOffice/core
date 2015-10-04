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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OPREPAREDSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OPREPAREDSTATEMENT_HXX

#include "odbc/odbcbasedllapi.hxx"
#include "odbc/OStatement.hxx"
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase.hxx>

namespace connectivity
{
    namespace odbc
    {

        class OBoundParam;
        typedef ::cppu::ImplHelper <    ::com::sun::star::sdbc::XPreparedStatement,
                                        ::com::sun::star::sdbc::XParameters,
                                        ::com::sun::star::sdbc::XPreparedBatchExecution,
                                        ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                        ::com::sun::star::lang::XServiceInfo> OPreparedStatement_BASE;

        class OOO_DLLPUBLIC_ODBCBASE OPreparedStatement :
                                    public  OStatement_BASE2,
                                    public  OPreparedStatement_BASE
        {
        protected:
            static const short invalid_scale = -1;
            struct Parameter
            {
                ::com::sun::star::uno::Any  aValue;
                sal_Int32                   nDataType;

                Parameter(const ::com::sun::star::uno::Any& rValue,
                          sal_Int32                         rDataType) : aValue(rValue),nDataType(rDataType)
                {
                }

            };

            ::std::vector< Parameter>                   m_aParameters;

            // Data attributes

            SQLSMALLINT     numParams;      // Number of parameter markers for the prepared statement

            OBoundParam*    boundParams;
                            // Array of bound parameter objects. Each parameter marker will have a
                            // corresponding object to hold bind information, and resulting data.
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xMetaData;
            bool                                                                        m_bPrepared;

            void FreeParams();
            void putParamData (sal_Int32 index)
                throw (css::sdbc::SQLException, css::uno::RuntimeException);
            void setStream (sal_Int32 ParameterIndex,const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x,
                                                        SQLLEN length,sal_Int32 SQLtype) throw (css::sdbc::SQLException, css::uno::RuntimeException);
            SQLLEN* getLengthBuf (sal_Int32 index);
            void* allocBindBuf (    sal_Int32 index,    sal_Int32 bufLen);
            void initBoundParam () throw(::com::sun::star::sdbc::SQLException);
            void setParameterPre(sal_Int32 parameterIndex);
            template <typename T> void setScalarParameter(sal_Int32 parameterIndex, sal_Int32 _nType, SQLULEN _nColumnSize, const T i_Value);
            void setParameter(sal_Int32 parameterIndex, sal_Int32 _nType, SQLULEN _nColumnSize, sal_Int32 _nScale, const void* _pData, SQLULEN _nDataLen, SQLLEN _nDataAllocLen);
            // Wrappers for special cases
            void setParameter(sal_Int32 parameterIndex, sal_Int32 _nType, sal_Int16 _nScale, const OUString &_sData);
            void setParameter(sal_Int32 parameterIndex, sal_Int32 _nType, const com::sun::star::uno::Sequence< sal_Int8 > &_Data);

            bool isPrepared() const { return m_bPrepared;}
            void prepareStatement();
            void checkParameterIndex(sal_Int32 _parameterIndex);

            /**
                creates the driver specific resultset (factory)
            */
            virtual OResultSet* createResulSet() override;

        protected:
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                                   const ::com::sun::star::uno::Any& rValue)
                                                                        throw (::com::sun::star::uno::Exception, std::exception) override;
        public:
            DECLARE_SERVICE_INFO();
            // A ctor, needed to return the object
            OPreparedStatement( OConnection* _pConnection,const OUString& sql);

            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

            // XPreparedStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL executeUpdate(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XParameters
            virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setString( sal_Int32 parameterIndex, const OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
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
            // XPreparedBatchExecution
            virtual void SAL_CALL addBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        public:
            using OStatement_Base::executeQuery;
            using OStatement_Base::executeUpdate;
            using OStatement_Base::execute;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OPREPAREDSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
