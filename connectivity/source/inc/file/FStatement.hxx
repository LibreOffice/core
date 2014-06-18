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
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/uno3.hxx>
#include "connectivity/CommonTools.hxx"
#include "file/FConnection.hxx"
#include "file/filedllapi.hxx"
#include <list>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/propertycontainer.hxx>
#include "file/fanalyzer.hxx"
#include <comphelper/broadcasthelper.hxx>
#include "TSortIndex.hxx"

namespace connectivity
{
    namespace file
    {
        class OResultSet;
        class OFileTable;
        typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::util::XCancellable,
                                                    ::com::sun::star::sdbc::XCloseable> OStatement_BASE;


        //************ Class: java.sql.Statement

        class OOO_DLLPUBLIC_FILE OStatement_Base :
                                        public  comphelper::OBaseMutex,
                                        public  OStatement_BASE,
                                        public  ::comphelper::OPropertyContainer,
                                        public  ::comphelper::OPropertyArrayUsageHelper<OStatement_Base>

        {
        protected:
            ::std::vector<sal_Int32>                    m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time
            ::std::vector<sal_Int32>                    m_aParameterIndexes; // maps the parameter index to column index
            ::std::vector<sal_Int32>                    m_aOrderbyColumnNumber;
            ::std::vector<TAscendingOrder>              m_aOrderbyAscending;

            ::com::sun::star::sdbc::SQLWarning                                           m_aLastWarning;
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> m_xDBMetaData;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>  m_xColNames; // table columns                                                          //  for this Statement


            connectivity::OSQLParser                    m_aParser;
            connectivity::OSQLParseTreeIterator         m_aSQLIterator;

            OConnection*                                m_pConnection;// The owning Connection object
            connectivity::OSQLParseNode*                m_pParseTree;
            OSQLAnalyzer*                               m_pSQLAnalyzer; //the sql analyzer used by the resultset

            ::std::vector<sal_Int32>*                   m_pEvaluationKeySet;

            OFileTable*                                 m_pTable;       // the current table
            OValueRefRow                                m_aSelectRow;
            OValueRefRow                                m_aRow;
            OValueRefRow                                m_aEvaluateRow; // contains all values of a row
            ORefAssignValues                            m_aAssignValues; // needed for insert,update and parameters
                                                                    // to compare with the restrictions

            OUString                             m_aCursorName;
            sal_Int32                                   m_nMaxFieldSize;
            sal_Int32                                   m_nMaxRows;
            sal_Int32                                   m_nQueryTimeOut;
            sal_Int32                                   m_nFetchSize;
            sal_Int32                                   m_nResultSetType;
            sal_Int32                                   m_nFetchDirection;
            sal_Int32                                   m_nResultSetConcurrency;
            bool                                    m_bEscapeProcessing;

            ::cppu::OBroadcastHelper&                   rBHelper;

        protected:
            // initialize the column index map (mapping select columns to table columns)
            void createColumnMapping();
            // searches the statement for sort criteria
            void anylizeSQL();
            void setOrderbyColumn( connectivity::OSQLParseNode* pColumnRef,
                                     connectivity::OSQLParseNode* pAscendingDescending);

            virtual void initializeResultSet(OResultSet* _pResult);
            // create the analyzer
            virtual OSQLAnalyzer* createAnalyzer();

            void closeResultSet () throw( ::com::sun::star::sdbc::SQLException);
            sal_Int32 getPrecision ( sal_Int32 sqlType);

            void disposeResultSet();
            void GetAssignValues();
            void SetAssignValue(const OUString& aColumnName,
                                   const OUString& aValue,
                                   bool bSetNull = false,
                                   sal_uInt32 nParameter=SQL_NO_PARAMETER);
            void ParseAssignValues( const ::std::vector< OUString>& aColumnNameList,
                                    connectivity::OSQLParseNode* pRow_Value_Constructor_Elem, sal_Int32 nIndex);

            virtual void parseParamterElem(const OUString& _sColumnName,OSQLParseNode* pRow_Value_Constructor_Elem);
            // factory method for resultset's
            virtual OResultSet* createResultSet() = 0;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() SAL_OVERRIDE;
            virtual ~OStatement_Base();
        public:
            connectivity::OSQLParseNode* getParseTree() const { return m_pParseTree;}

            OStatement_Base(OConnection* _pConnection );

            OConnection* getOwnConnection() const { return m_pConnection;}

            using OStatement_BASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            virtual void construct(const OUString& sql)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // OComponentHelper
            virtual void SAL_CALL disposing(void) SAL_OVERRIDE;
            // XInterface
            //      virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException) = 0;
            virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        };

        class OOO_DLLPUBLIC_FILE OStatement_BASE2 :
                                    public OStatement_Base,
                                    public connectivity::OSubComponent<OStatement_BASE2, OStatement_BASE>

        {
            friend class connectivity::OSubComponent<OStatement_BASE2, OStatement_BASE>;
        public:
            OStatement_BASE2(OConnection* _pConnection ) :  OStatement_Base(_pConnection ),
                                    connectivity::OSubComponent<OStatement_BASE2, OStatement_BASE>((::cppu::OWeakObject*)_pConnection, this){}
            // OComponentHelper
            virtual void SAL_CALL disposing(void) SAL_OVERRIDE;
            // XInterface
            virtual void SAL_CALL release() throw() SAL_OVERRIDE;
        };

        typedef ::cppu::ImplHelper2< ::com::sun::star::sdbc::XStatement,::com::sun::star::lang::XServiceInfo > OStatement_XStatement;
        class OOO_DLLPUBLIC_FILE OStatement :
                            public OStatement_BASE2,
                            public OStatement_XStatement
        {
        protected:
            // factory method for resultset's
            virtual OResultSet* createResultSet() SAL_OVERRIDE;
        public:
            // a Constructor, that is needed for when Returning the Object is needed:
            OStatement( OConnection* _pConnection) : OStatement_BASE2( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
            virtual void SAL_CALL release() throw() SAL_OVERRIDE;

            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
