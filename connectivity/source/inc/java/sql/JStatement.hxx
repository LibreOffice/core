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
#ifndef _CONNECTIVITY_JAVA_SQL_STATEMENT_HXX_
#define _CONNECTIVITY_JAVA_SQL_STATEMENT_HXX_

#include "java/lang/Object.hxx"
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase6.hxx>
#include <comphelper/uno3.hxx>
#include "connectivity/CommonTools.hxx"
#include "OSubComponent.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/broadcasthelper.hxx>

#include "java/sql/ConnectionLog.hxx"

namespace connectivity
{

        typedef ::cppu::WeakComponentImplHelper6<       ::com::sun::star::sdbc::XStatement,
                                                        ::com::sun::star::sdbc::XWarningsSupplier,
                                                        ::com::sun::star::util::XCancellable,
                                                        ::com::sun::star::sdbc::XCloseable,
                                                        ::com::sun::star::sdbc::XGeneratedResultSet,
                                                        ::com::sun::star::sdbc::XMultipleResults> java_sql_Statement_BASE;

    class java_sql_Connection;

    //**************************************************************
    //************ Class: java.sql.Statement
    //**************************************************************
    class java_sql_Statement_Base : public comphelper::OBaseMutex,
                                    public  java_sql_Statement_BASE,
                                    public  java_lang_Object,
                                    public  ::cppu::OPropertySetHelper,
                                    public  ::comphelper::OPropertyArrayUsageHelper<java_sql_Statement_Base>

    {

        sal_Int32 getQueryTimeOut()             throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 getMaxFieldSize()             throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 getMaxRows()                  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 getResultSetConcurrency()     throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 getResultSetType()            throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 getFetchDirection()           throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        sal_Int32 getFetchSize()                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        ::rtl::OUString getCursorName()         throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        void setQueryTimeOut(sal_Int32 _par0)                   throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setMaxFieldSize(sal_Int32 _par0)                   throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setMaxRows(sal_Int32 _par0)                                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setResultSetConcurrency(sal_Int32 _par0)   throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setResultSetType(sal_Int32 _par0)                  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setFetchDirection(sal_Int32 _par0)                 throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setFetchSize(sal_Int32 _par0)                              throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setCursorName(const ::rtl::OUString &_par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        void setEscapeProcessing(sal_Bool _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement>       m_xGeneratedStatement;
        java_sql_Connection*        m_pConnection;
        java::sql::ConnectionLog    m_aLogger;
        ::rtl::OUString             m_sSqlStatement;
        // Properties
        sal_Int32                   m_nResultSetConcurrency;
        sal_Int32                   m_nResultSetType;
        sal_Bool                    m_bEscapeProcessing;
        ::cppu::OBroadcastHelper&   rBHelper;


    // statische Daten fuer die Klasse
        static jclass theClass;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                    ::com::sun::star::uno::Any & rConvertedValue,
                                    ::com::sun::star::uno::Any & rOldValue,
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                )   throw (::com::sun::star::lang::IllegalArgumentException);

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                )   throw (::com::sun::star::uno::Exception);

        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                ) const;

        virtual void createStatement(JNIEnv* _pEnv) = 0;

        virtual ~java_sql_Statement_Base();

        sal_Int32 impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID);
        sal_Int32 impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID,sal_Int32 _nDefault);

    public:
        virtual jclass getMyClass() const;

        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Statement_Base( JNIEnv * pEnv, java_sql_Connection& _rCon );

        sal_Int32   getStatementObjectID() const { return m_aLogger.getObjectID(); }

        // OComponentHelper
        virtual void SAL_CALL disposing(void);
        // XInterface
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
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
        // XCancellable
        virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);
        // XCloseable
        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XMultipleResults
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        //XGeneratedResultSet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getGeneratedValues(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    public:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };

    class OStatement_BASE2  :public java_sql_Statement_Base
                            ,public OSubComponent<OStatement_BASE2, java_sql_Statement_BASE>

    {
        friend class OSubComponent<OStatement_BASE2, java_sql_Statement_BASE>;
    public:
        OStatement_BASE2(JNIEnv * pEnv, java_sql_Connection& _rCon ) : java_sql_Statement_Base( pEnv, _rCon ),
                                OSubComponent<OStatement_BASE2, java_sql_Statement_BASE>((::cppu::OWeakObject*)(&_rCon), this){}

        // OComponentHelper
        virtual void SAL_CALL disposing(void);
        // XInterface
        virtual void SAL_CALL release() throw();
    };

    class java_sql_Statement :  public OStatement_BASE2,
                                public ::com::sun::star::sdbc::XBatchExecution,
                                public ::com::sun::star::lang::XServiceInfo
    {
    protected:
        // statische Daten fuer die Klasse
        static jclass theClass;

        virtual void createStatement(JNIEnv* _pEnv);

        virtual ~java_sql_Statement();
    public:
        DECLARE_SERVICE_INFO();
        virtual jclass getMyClass() const;

        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Statement( JNIEnv * pEnv, java_sql_Connection& _rCon ) : OStatement_BASE2( pEnv, _rCon){};

        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
        // XBatchExecution
        virtual void SAL_CALL addBatch( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_STATEMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
