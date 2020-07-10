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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MPREPAREDSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MPREPAREDSTATEMENT_HXX

#include "MResultSet.hxx"
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <cppuhelper/implbase5.hxx>

namespace connectivity::mork
    {

        typedef ::cppu::ImplHelper5<    css::sdbc::XPreparedStatement,
                                        css::sdbc::XParameters,
                                        css::sdbc::XResultSetMetaDataSupplier,
                                        css::sdbc::XMultipleResults,
                                        css::lang::XServiceInfo> OPreparedStatement_BASE;

        class OPreparedStatement final : public  OCommonStatement,
                                    public  OPreparedStatement_BASE
        {
            // Data attributes

            OUString                                    m_sSqlStatement;
            css::uno::Reference< css::sdbc::XResultSetMetaData >  m_xMetaData;
            ::rtl::Reference< OResultSet >              m_pResultSet;
            ::rtl::Reference<connectivity::OSQLColumns> m_xParamColumns;    // the parameter columns
            OValueRow                                   m_aParameterRow;

            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                                   const css::uno::Any& rValue) override;
            virtual ~OPreparedStatement() override;

            virtual void SAL_CALL disposing() override;

            // OCommonStatement overridables
            virtual StatementType
                            parseSql( const OUString& sql , bool bAdjusted = false) override;
            virtual void    initializeResultSet( OResultSet* _pResult ) override;
            virtual void    clearCachedResultSet() override;
            virtual void    cacheResultSet( const ::rtl::Reference< OResultSet >& _pResult ) override;


            void checkAndResizeParameters(sal_Int32 parameterIndex);
            void setParameter(sal_Int32 parameterIndex, const ORowSetValue& x);

            void AddParameter(connectivity::OSQLParseNode const * pParameter,
                                const css::uno::Reference< css::beans::XPropertySet>& _xCol);
            void scanParameter(OSQLParseNode* pParseNode,std::vector< OSQLParseNode*>& _rParaNodes);
            void describeColumn(OSQLParseNode const * _pParameter, OSQLParseNode const * _pNode, const OSQLTable& _xTable);
            void describeParameter();

        public:
            DECLARE_SERVICE_INFO();
            // A ctor need for returning the object
            OPreparedStatement( OConnection* _pConnection,const OUString& sql);
            void lateInit();

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
            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;
            // XMultipleResults
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getResultSet(  ) override;
            virtual sal_Int32 SAL_CALL getUpdateCount(  ) override;
            virtual sal_Bool SAL_CALL getMoreResults(  ) override;

        public:
            using OCommonStatement::executeQuery;
            using OCommonStatement::executeUpdate;
            using OCommonStatement::execute;
        private:
            using OPropertySetHelper::getFastPropertyValue;
        };

}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MPREPAREDSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
