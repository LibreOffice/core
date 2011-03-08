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

#ifndef _CONNECTIVITY_FILE_OPREPAREDSTATEMENT_HXX_
#define _CONNECTIVITY_FILE_OPREPAREDSTATEMENT_HXX_

#include "file/filedllapi.hxx"
#include "file/FStatement.hxx"
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include "file/FResultSet.hxx"

namespace connectivity
{
    namespace file
    {

        class OOO_DLLPUBLIC_FILE SAL_NO_VTABLE OPreparedStatement : public  OStatement_BASE2,
                                    public  ::com::sun::star::sdbc::XPreparedStatement,
                                    public  ::com::sun::star::sdbc::XParameters,
                                    public  ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                    public  ::com::sun::star::lang::XServiceInfo

        {
        protected:
            //====================================================================
            // Data attributes
            //====================================================================
            ::rtl::OUString                                     m_aSql;
            OValueRefRow                                        m_aParameterRow;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>   m_xMetaData;

            OResultSet*                                         m_pResultSet;
            ::rtl::Reference<connectivity::OSQLColumns>             m_xParamColumns;    // the parameter columns

            // factory method for resultset's
            virtual OResultSet* createResultSet();
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> initResultSet();

            void checkAndResizeParameters(sal_Int32 parameterIndex);
            void setParameter(sal_Int32 parameterIndex, const ORowSetValue& x);

            UINT32 AddParameter(connectivity::OSQLParseNode * pParameter,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xCol);
            void scanParameter(OSQLParseNode* pParseNode,::std::vector< OSQLParseNode*>& _rParaNodes);
            void describeColumn(OSQLParseNode* _pParameter,OSQLParseNode* _pNode,const OSQLTable& _xTable);
            void describeParameter();

            virtual void parseParamterElem(const String& _sColumnName,OSQLParseNode* pRow_Value_Constructor_Elem);
            virtual void initializeResultSet(OResultSet* _pResult);

            virtual ~OPreparedStatement();
        public:
            DECLARE_SERVICE_INFO();
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OPreparedStatement( OConnection* _pConnection);

            virtual void construct(const ::rtl::OUString& sql)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // XPreparedStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL executeUpdate(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XParameters
            virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setBytes( sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setBinaryStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setCharacterStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setObjectWithInfo( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setRef( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setBlob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setClob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setArray( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearParameters(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XCloseable
                        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_FILE_OPREPAREDSTATEMENT_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
