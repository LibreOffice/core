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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FSTATEMENT_HXX

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/sqlparse.hxx>
#include <file/FConnection.hxx>
#include <file/filedllapi.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/propertycontainer.hxx>
#include <file/fanalyzer.hxx>
#include <TSortIndex.hxx>

namespace connectivity::file
    {
        class OResultSet;
        class OFileTable;
        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XWarningsSupplier,
                                                   css::util::XCancellable,
                                                   css::sdbc::XCloseable> OStatement_BASE;


        //************ Class: java.sql.Statement

        class OOO_DLLPUBLIC_FILE OStatement_Base :
                                        public  cppu::BaseMutex,
                                        public  OStatement_BASE,
                                        public  ::comphelper::OPropertyContainer,
                                        public  ::comphelper::OPropertyArrayUsageHelper<OStatement_Base>

        {
        protected:
            std::vector<sal_Int32>                    m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time
            std::vector<sal_Int32>                    m_aParameterIndexes; // maps the parameter index to column index
            std::vector<sal_Int32>                    m_aOrderbyColumnNumber;
            std::vector<TAscendingOrder>              m_aOrderbyAscending;

            css::sdbc::SQLWarning                              m_aLastWarning;
            css::uno::WeakReference< css::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
            css::uno::Reference< css::sdbc::XDatabaseMetaData> m_xDBMetaData;
            css::uno::Reference< css::container::XNameAccess>  m_xColNames; // table columns                                                          //  for this Statement


            connectivity::OSQLParser                    m_aParser;
            connectivity::OSQLParseTreeIterator         m_aSQLIterator;

            rtl::Reference<OConnection>                 m_pConnection;// The owning Connection object
            connectivity::OSQLParseNode*                m_pParseTree;
            std::unique_ptr<OSQLAnalyzer>               m_pSQLAnalyzer; //the sql analyzer used by the resultset

            rtl::Reference<OFileTable>                  m_pTable;       // the current table
            OValueRefRow                                m_aSelectRow;
            OValueRefRow                                m_aRow;
            OValueRefRow                                m_aEvaluateRow; // contains all values of a row
            ORefAssignValues                            m_aAssignValues; // needed for insert,update and parameters
                                                                    // to compare with the restrictions

            OUString                                    m_aCursorName;
            sal_Int32                                   m_nMaxFieldSize;
            sal_Int32                                   m_nMaxRows;
            sal_Int32                                   m_nQueryTimeOut;
            sal_Int32                                   m_nFetchSize;
            sal_Int32                                   m_nResultSetType;
            sal_Int32                                   m_nFetchDirection;
            sal_Int32                                   m_nResultSetConcurrency;
            bool                                        m_bEscapeProcessing;

        protected:
            // initialize the column index map (mapping select columns to table columns)
            void createColumnMapping();
            // searches the statement for sort criteria
            void anylizeSQL();
            void setOrderbyColumn( connectivity::OSQLParseNode const * pColumnRef,
                                   connectivity::OSQLParseNode const * pAscendingDescending);

            virtual void initializeResultSet(OResultSet* _pResult);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void closeResultSet();

            void disposeResultSet();
            void GetAssignValues();
            void SetAssignValue(const OUString& aColumnName,
                                   const OUString& aValue,
                                   bool bSetNull = false,
                                   sal_uInt32 nParameter=SQL_NO_PARAMETER);
            void ParseAssignValues( const std::vector< OUString>& aColumnNameList,
                                    connectivity::OSQLParseNode* pRow_Value_Constructor_Elem, sal_Int32 nIndex);

            virtual void parseParamterElem(const OUString& _sColumnName,OSQLParseNode* pRow_Value_Constructor_Elem);
            // factory method for resultset's
            virtual rtl::Reference<OResultSet> createResultSet() = 0;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
            virtual ~OStatement_Base() override;
        public:
            connectivity::OSQLParseNode* getParseTree() const { return m_pParseTree;}

            OStatement_Base(OConnection* _pConnection );

            OConnection* getOwnConnection() const { return m_pConnection.get(); }

            using OStatement_BASE::operator css::uno::Reference< css::uno::XInterface >;

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            virtual void construct(const OUString& sql);

            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            //      virtual void SAL_CALL release() throw(css::uno::RuntimeException) = 0;
            virtual void SAL_CALL acquire() throw() override;
            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) override;
            // XCloseable
            virtual void SAL_CALL close(  ) override;
        };

        class OOO_DLLPUBLIC_FILE OStatement_BASE2 : public OStatement_Base

        {
        public:
            OStatement_BASE2(OConnection* _pConnection ) : OStatement_Base(_pConnection ) {}
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;
        };

        typedef ::cppu::ImplHelper2< css::sdbc::XStatement,css::lang::XServiceInfo > OStatement_XStatement;
        class OOO_DLLPUBLIC_FILE OStatement :
                            public OStatement_BASE2,
                            public OStatement_XStatement
        {
        protected:
            // factory method for resultset's
            virtual rtl::Reference<OResultSet> createResultSet() override;
        public:
            // a Constructor, that is needed for when Returning the Object is needed:
            OStatement( OConnection* _pConnection) : OStatement_BASE2( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;

            // XStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) override ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) override ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) override ;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override ;
        };

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
