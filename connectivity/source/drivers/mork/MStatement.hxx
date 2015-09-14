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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MSTATEMENT_HXX

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/CommonTools.hxx>
#include <list>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include <connectivity/FValue.hxx>
#include "TSortIndex.hxx"
#include "MConnection.hxx"
#include "MTable.hxx"

#include <memory>

namespace connectivity
{
    namespace mork
    {
        class OResultSet;

        typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::sdbc::XStatement,
                                                   ::com::sun::star::sdbc::XWarningsSupplier,
                                                   ::com::sun::star::sdbc::XCloseable> OCommonStatement_IBASE;


        //************ Class: OCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class OCommonStatement;
        typedef ::connectivity::OSubComponent< OCommonStatement, OCommonStatement_IBASE >  OCommonStatement_SBASE;

        class OCommonStatement  :public comphelper::OBaseMutex
                                ,public OCommonStatement_IBASE
                                ,public ::cppu::OPropertySetHelper
                                ,public ::comphelper::OPropertyArrayUsageHelper< OCommonStatement >
                                ,public OCommonStatement_SBASE
        {
            friend class ::connectivity::OSubComponent< OCommonStatement, OCommonStatement_IBASE >;

        private:
            ::com::sun::star::sdbc::SQLWarning                            m_aLastWarning;

        protected:
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet >   m_xResultSet;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> m_xDBMetaData;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>  m_xColNames; // table columns

            //  for this Statement

            ::std::list< OUString>               m_aBatchList;

            OTable*                                     m_pTable;
            OConnection*                                m_pConnection;  // The owning Connection object

            OValueRow                                   m_aRow;

            connectivity::OSQLParser                    m_aParser;
            std::shared_ptr< ::connectivity::OSQLParseTreeIterator >
                                                        m_pSQLIterator;

            connectivity::OSQLParseNode*                m_pParseTree;

            ::std::vector<sal_Int32>                    m_aColMapping;
            ::std::vector<sal_Int32>                    m_aOrderbyColumnNumber;
            ::std::vector<TAscendingOrder>              m_aOrderbyAscending;

            ::cppu::OBroadcastHelper&                   rBHelper;

        protected:

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                                                ::com::sun::star::uno::Any & rConvertedValue,
                                                                ::com::sun::star::uno::Any & rOldValue,
                                                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue )
                                                            throw (::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue)   throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;
            virtual void SAL_CALL getFastPropertyValue(
                                                                ::com::sun::star::uno::Any& rValue,
                                                                sal_Int32 nHandle) const SAL_OVERRIDE;
            virtual ~OCommonStatement();

        protected:

            // Driver Internal Methods

            enum StatementType { eSelect, eCreateTable };
            /** called to do the parsing of a to-be-executed SQL statement, and set all members as needed
            */
            virtual StatementType
                            parseSql( const OUString& sql , bool bAdjusted = false) throw ( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
            /** called to initialize a result set, according to a previously parsed SQL statement
            */
            virtual void    initializeResultSet( OResultSet* _pResult );
            /** called when a possible cached instance of our last result set should be cleared
            */
            virtual void    clearCachedResultSet();
            /** caches a result set which has just been created by an execution of an SQL statement
            */
            virtual void    cacheResultSet( const ::rtl::Reference< OResultSet >& _pResult );


            /** executes the current query (the one which has been passed to the last parseSql call)
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
                            impl_executeCurrentQuery();

            void         createColumnMapping();
            void         analyseSQL();
            void         setOrderbyColumn( connectivity::OSQLParseNode* pColumnRef,
                                           connectivity::OSQLParseNode* pAscendingDescending);

        public:
            // other methods
            OConnection* getOwnConnection() const { return m_pConnection;}

            explicit OCommonStatement(OConnection* _pConnection );
            using OCommonStatement_IBASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            // OComponentHelper
            virtual void SAL_CALL disposing() SAL_OVERRIDE;

            // XInterface
            virtual void SAL_CALL release() throw() SAL_OVERRIDE;
            virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE ;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };

        class OStatement :  public OCommonStatement,
                            public ::com::sun::star::lang::XServiceInfo
        {
        protected:
            virtual ~OStatement(){}
        public:
            // a constructor, for when the object needs to be returned:
            explicit OStatement( OConnection* _pConnection);
            DECLARE_SERVICE_INFO();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
            virtual void SAL_CALL release() throw() SAL_OVERRIDE;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
