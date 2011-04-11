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
#ifndef CONNECTIVITY_SSTATEMENT_HXX
#define CONNECTIVITY_SSTATEMENT_HXX

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase3.hxx>
#include <comphelper/uno3.hxx>
#include "connectivity/CommonTools.hxx"
#ifndef INCLUDED_LIST
#include <list>
#define INCLUDED_LIST
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sqliterator.hxx"
#include "connectivity/sqlparse.hxx"
#include <connectivity/FValue.hxx>
#include "TSortIndex.hxx"
#include "MConnection.hxx"
#include "MTable.hxx"

#include <boost/shared_ptr.hpp>

namespace connectivity
{
    namespace mozab
    {
        class OResultSet;

        typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::sdbc::XStatement,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::sdbc::XCloseable> OCommonStatement_IBASE;

        //**************************************************************
        //************ Class: OCommonStatement
        // is a base class for the normal statement and for the prepared statement
        //**************************************************************
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

            ::std::list< ::rtl::OUString>               m_aBatchList;

            OTable*                                     m_pTable;
            OConnection*                                m_pConnection;  // The owning Connection object

            OValueRow                                   m_aRow;

            connectivity::OSQLParser                    m_aParser;
            ::boost::shared_ptr< ::connectivity::OSQLParseTreeIterator >
                                                        m_pSQLIterator;

            connectivity::OSQLParseNode*                m_pParseTree;

            ::std::vector<sal_Int32>                    m_aColMapping;
            ::std::vector<sal_Int32>                    m_aOrderbyColumnNumber;
            ::std::vector<TAscendingOrder>              m_aOrderbyAscending;

            ::cppu::OBroadcastHelper&                   rBHelper;

        protected:

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                                                ::com::sun::star::uno::Any & rConvertedValue,
                                                                ::com::sun::star::uno::Any & rOldValue,
                                                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue )
                                                            throw (::com::sun::star::lang::IllegalArgumentException);
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue)   throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                                                ::com::sun::star::uno::Any& rValue,
                                                                sal_Int32 nHandle) const;
            virtual ~OCommonStatement();

        protected:
            //
            // Driver Internal Methods
            //
            enum StatementType { eSelect, eCreateTable };
            /** called to do the parsing of a to-be-executed SQL statement, and set all members as needed
            */
            virtual StatementType
                            parseSql( const ::rtl::OUString& sql , sal_Bool bAdjusted = sal_False) throw ( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
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
            virtual void createTable(  ) throw (
                        ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );

        public:
            // other methods
            OConnection* getOwnConnection() const { return m_pConnection;}

            OCommonStatement(OConnection* _pConnection );
            using OCommonStatement_IBASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            // OComponentHelper
            virtual void SAL_CALL disposing(void);

            // XInterface
            virtual void SAL_CALL release() throw();
            virtual void SAL_CALL acquire() throw();
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            virtual sal_Int32 SAL_CALL executeUpdate( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            virtual sal_Bool SAL_CALL execute( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };

        class OStatement :  public OCommonStatement,
                            public ::com::sun::star::lang::XServiceInfo
        {
        protected:
            ~OStatement(){}
        public:
            // a constructor, for when the object needs to be returned:
            OStatement( OConnection* _pConnection) : OCommonStatement( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
        };
    }
}
#endif // CONNECTIVITY_SSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
