/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef CONNECTIVITY_SRESULSETMETADATA_HXX
#define CONNECTIVITY_SRESULSETMETADATA_HXX

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>

#include <cppuhelper/implbase1.hxx>
#include <preextstl.h>
#include <cppconn/resultset_metadata.h>
#include <postextstl.h>

namespace connectivity
{
    namespace mysqlc
    {
        using ::com::sun::star::sdbc::SQLException;
        using ::com::sun::star::uno::RuntimeException;
        using ::rtl::OUString;
        //**************************************************************
        //************ Class: ResultSetMetaData
        //**************************************************************
        typedef ::cppu::WeakImplHelper1< ::com::sun::star::sdbc::XResultSetMetaData> OResultSetMetaData_BASE;

        class OResultSetMetaData : public OResultSetMetaData_BASE
        {
            sql::ResultSetMetaData * meta;
            rtl_TextEncoding        m_encoding;
        protected:
            virtual ~OResultSetMetaData();
        public:
            OResultSetMetaData( sql::ResultSetMetaData * _meta, rtl_TextEncoding _encoding )
                :meta(_meta)
                ,m_encoding( _encoding )
            {
            }

            inline ::rtl::OUString convert( const ::ext_std::string& _string ) const
            {
                return ::rtl::OUString( _string.c_str(), _string.size(), m_encoding );
            }

            /// Avoid ambigous cast error from the compiler.
            inline operator ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > () throw()
            { return this; }

            sal_Int32 SAL_CALL getColumnCount()                     throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isAutoIncrement(sal_Int32 column)     throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isCaseSensitive(sal_Int32 column)     throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isSearchable(sal_Int32 column)        throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isCurrency(sal_Int32 column)          throw(SQLException, RuntimeException);

            sal_Int32 SAL_CALL isNullable(sal_Int32 column)         throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isSigned(sal_Int32 column)            throw(SQLException, RuntimeException);

            sal_Int32 SAL_CALL getColumnDisplaySize(sal_Int32 column)throw(SQLException, RuntimeException);

            OUString SAL_CALL getColumnLabel(sal_Int32 column)      throw(SQLException, RuntimeException);

            OUString SAL_CALL getColumnName(sal_Int32 column)       throw(SQLException, RuntimeException);

            OUString SAL_CALL getSchemaName(sal_Int32 column)       throw(SQLException, RuntimeException);

            sal_Int32 SAL_CALL getPrecision(sal_Int32 column)       throw(SQLException, RuntimeException);

            sal_Int32 SAL_CALL getScale(sal_Int32 column)           throw(SQLException, RuntimeException);

            OUString SAL_CALL getTableName(sal_Int32 column)        throw(SQLException, RuntimeException);

            OUString SAL_CALL getCatalogName(sal_Int32 column)      throw(SQLException, RuntimeException);

            sal_Int32 SAL_CALL getColumnType(sal_Int32 column)      throw(SQLException, RuntimeException);

            OUString SAL_CALL getColumnTypeName(sal_Int32 column)   throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isReadOnly(sal_Int32 column)          throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isWritable(sal_Int32 column)          throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL isDefinitelyWritable(sal_Int32 column)throw(SQLException, RuntimeException);

            OUString SAL_CALL getColumnServiceName(sal_Int32 column)throw(SQLException, RuntimeException);

            void checkColumnIndex(sal_Int32 columnIndex)            throw (SQLException, RuntimeException);
        };
    }
}

#endif // CONNECTIVITY_SRESULSETMETADATA_HXX
