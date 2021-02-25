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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FPREPAREDSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FPREPAREDSTATEMENT_HXX

#include <file/filedllapi.hxx>
#include <file/FStatement.hxx>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <file/FResultSet.hxx>

namespace connectivity::file
    {

        class OOO_DLLPUBLIC_FILE SAL_NO_VTABLE OPreparedStatement : public  OStatement_BASE2,
                                    public  css::sdbc::XPreparedStatement,
                                    public  css::sdbc::XParameters,
                                    public  css::sdbc::XResultSetMetaDataSupplier,
                                    public  css::lang::XServiceInfo

        {
        protected:

            // Data attributes

            OValueRefRow                                          m_aParameterRow;
            css::uno::Reference< css::sdbc::XResultSetMetaData>   m_xMetaData;

            ::rtl::Reference<connectivity::OSQLColumns>           m_xParamColumns;    // the parameter columns

            // factory method for resultset's
            virtual rtl::Reference<OResultSet> createResultSet() override;
            ::rtl::Reference< OResultSet > makeResultSet();
            void initResultSet(OResultSet*);

            void checkAndResizeParameters(sal_Int32 parameterIndex);
            void setParameter(sal_Int32 parameterIndex, const ORowSetValue& x);

            sal_uInt32 AddParameter(connectivity::OSQLParseNode const * pParameter,
                                const css::uno::Reference< css::beans::XPropertySet>& _xCol);
            void scanParameter(OSQLParseNode* pParseNode,std::vector< OSQLParseNode*>& _rParaNodes);
            void describeColumn(OSQLParseNode const * _pParameter, OSQLParseNode const * _pNode, const OSQLTable& _xTable);
            void describeParameter();

            virtual void parseParamterElem(const OUString& _sColumnName,OSQLParseNode* pRow_Value_Constructor_Elem) override;
            virtual void initializeResultSet(OResultSet* _pResult) override;

            virtual ~OPreparedStatement() override;
        public:
            DECLARE_SERVICE_INFO();
            // a Constructor, that is needed for when Returning the Object is needed:
            OPreparedStatement( OConnection* _pConnection);

            virtual void construct(const OUString& sql) override;

            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            //XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

            // XPreparedStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery(  ) override;
            virtual sal_Int32 SAL_CALL executeUpdate(  ) override;
            virtual sal_Bool SAL_CALL execute(  ) override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override;
            // XParameters
            virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) override;
            virtual void SAL_CALL setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
            virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) override;
            virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) override;
            virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) override;
            virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) override;
            virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) override;
            virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) override;
            virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) override;
            virtual void SAL_CALL setString( sal_Int32 parameterIndex, const OUString& x ) override;
            virtual void SAL_CALL setBytes( sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x ) override;
            virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const css::util::Date& x ) override;
            virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const css::util::Time& x ) override;
            virtual void SAL_CALL setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x ) override;
            virtual void SAL_CALL setBinaryStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
            virtual void SAL_CALL setCharacterStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
            virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const css::uno::Any& x ) override;
            virtual void SAL_CALL setObjectWithInfo( sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) override;
            virtual void SAL_CALL setRef( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef >& x ) override;
            virtual void SAL_CALL setBlob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob >& x ) override;
            virtual void SAL_CALL setClob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob >& x ) override;
            virtual void SAL_CALL setArray( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray >& x ) override;
            virtual void SAL_CALL clearParameters(  ) override;
            // XCloseable
                        virtual void SAL_CALL close(  ) override;
            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;
        };

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FPREPAREDSTATEMENT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
