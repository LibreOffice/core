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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OSTATEMENT_HXX

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/CommonTools.hxx>
#include "odbc/OFunctions.hxx"
#include "odbc/OConnection.hxx"
#include "odbc/odbcbasedllapi.hxx"
#include <list>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/broadcasthelper.hxx>

namespace connectivity
{
    namespace odbc
    {

        typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::sdbc::XStatement,
                                                   ::com::sun::star::sdbc::XWarningsSupplier,
                                                   ::com::sun::star::util::XCancellable,
                                                   ::com::sun::star::sdbc::XCloseable,
                                                   ::com::sun::star::sdbc::XGeneratedResultSet,
                                                   ::com::sun::star::sdbc::XMultipleResults> OStatement_BASE;

        class OResultSet;

        //************ Class: java.sql.Statement

        class OOO_DLLPUBLIC_ODBCBASE OStatement_Base :
                                        public comphelper::OBaseMutex,
                                        public  OStatement_BASE,
                                        public  ::cppu::OPropertySetHelper,
                                        public  ::comphelper::OPropertyArrayUsageHelper<OStatement_Base>

        {
        ::com::sun::star::sdbc::SQLWarning                                           m_aLastWarning;
        protected:
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>   m_xResultSet;   // The last ResultSet created
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement>       m_xGeneratedStatement;
            //  for this Statement

            ::std::list< OUString>   m_aBatchList;
            OUString                 m_sSqlStatement;

            OConnection*                    m_pConnection;// The owning Connection object
            SQLHANDLE                       m_aStatementHandle;
            SQLUSMALLINT*                   m_pRowStatusArray;
            ::cppu::OBroadcastHelper&       rBHelper;

        protected:

            sal_Int64 getQueryTimeOut()         const;
            sal_Int64 getMaxFieldSize()         const;
            sal_Int64 getMaxRows()              const;
            sal_Int32 getResultSetConcurrency() const;
            sal_Int32 getResultSetType()        const;
            sal_Int32 getFetchDirection()       const;
            sal_Int32 getFetchSize()            const;
            OUString getCursorName()     const;
            bool isUsingBookmarks()         const;
            bool getEscapeProcessing()      const;
            template < typename T, SQLINTEGER BufferLength > T getStmtOption (SQLINTEGER fOption, T dflt = 0) const;

            void setQueryTimeOut(sal_Int64 _par0)           ;
            void setMaxFieldSize(sal_Int64 _par0)           ;
            void setMaxRows(sal_Int64 _par0)                ;
            void setFetchDirection(sal_Int32 _par0)         ;
            void setFetchSize(sal_Int32 _par0)              ;
            void setCursorName(const OUString &_par0);
            void setEscapeProcessing( const bool _bEscapeProc );
            template < typename T, SQLINTEGER BufferLength > SQLRETURN setStmtOption (SQLINTEGER fOption, T value) const;

            void setResultSetConcurrency(sal_Int32 _par0)   ;
            void setResultSetType(sal_Int32 _par0)          ;
            void setUsingBookmarks(bool _bUseBookmark)  ;

            void reset() throw (css::sdbc::SQLException, css::uno::RuntimeException);
            void clearMyResultSet() throw (css::sdbc::SQLException, css::uno::RuntimeException);
            void setWarning (const ::com::sun::star::sdbc::SQLWarning &ex)
                throw (css::sdbc::SQLException, css::uno::RuntimeException);
            bool lockIfNecessary (const OUString& sql) throw (css::sdbc::SQLException, css::uno::RuntimeException);
            sal_Int32 getColumnCount() throw (css::sdbc::SQLException, css::uno::RuntimeException);


            // getResultSet
            // getResultSet returns the current result as a ResultSet.  It
            // returns NULL if the current result is not a ResultSet.

            css::uno::Reference<css::sdbc::XResultSet> getResultSet(bool checkCount)
                throw (css::sdbc::SQLException, css::uno::RuntimeException);
            /**
                creates the driver specific resultset (factory)
            */
            virtual OResultSet* createResulSet();

            SQLLEN getRowCount() throw (css::sdbc::SQLException, css::uno::RuntimeException);


            void disposeResultSet();

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                            throw (::com::sun::star::lang::IllegalArgumentException) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                 throw (::com::sun::star::uno::Exception, std::exception) override;
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const override;
            virtual ~OStatement_Base();

        public:
            OStatement_Base(OConnection* _pConnection );
            using OStatement_BASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            inline oslGenericFunction getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const
            {
                return m_pConnection->getOdbcFunction(_nIndex);
            }
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;
            virtual void SAL_CALL acquire() throw() override;
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XMultipleResults
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            //XGeneratedResultSet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getGeneratedValues(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // other methods
            SQLHANDLE getConnectionHandle() { return m_pConnection->getConnection(); }
            OConnection* getOwnConnection() const { return m_pConnection;}
            /** getCursorProperties return the properties for a specific cursor type
                @param _nCursorType     the CursorType
                @param bFirst           when true the first property set is returned

                @return the cursor properties
            */
            SQLUINTEGER getCursorProperties(SQLINTEGER _nCursorType, bool bFirst);

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };

        class OOO_DLLPUBLIC_ODBCBASE OStatement_BASE2 :
                                 public OStatement_Base
                                ,public ::connectivity::OSubComponent<OStatement_BASE2, OStatement_BASE>

        {
            friend class OSubComponent<OStatement_BASE2, OStatement_BASE>;
        public:
            OStatement_BASE2(OConnection* _pConnection ) :  OStatement_Base(_pConnection ),
                                    ::connectivity::OSubComponent<OStatement_BASE2, OStatement_BASE>(static_cast<cppu::OWeakObject*>(_pConnection), this){}
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;
        };

        class OOO_DLLPUBLIC_ODBCBASE OStatement :
                            public OStatement_BASE2,
                            public ::com::sun::star::sdbc::XBatchExecution,
                            public ::com::sun::star::lang::XServiceInfo
        {
        protected:
            virtual ~OStatement(){}
        public:
            // A ctor that is needed for returning the object
            OStatement( OConnection* _pConnection) : OStatement_BASE2( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            // XBatchExecution
            virtual void SAL_CALL addBatch( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
