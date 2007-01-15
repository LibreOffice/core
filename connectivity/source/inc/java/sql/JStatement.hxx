/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JStatement.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-15 13:37:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_JAVA_SQL_STATEMENT_HXX_
#define _CONNECTIVITY_JAVA_SQL_STATEMENT_HXX_

#ifndef _CONNECTIVITY_JAVA_LANG_OBJECT_HXX_
#include "java/lang/Object.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XSTATEMENT_HPP_
#include <com/sun/star/sdbc/XStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XMULTIPLERESULTS_HPP_
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XBATCHEXECUTION_HPP_
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XGENERATEDRESULTSET_HPP_
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE6_HXX_
#include <cppuhelper/compbase6.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_OSUBCOMPONENT_HXX_
#include "OSubComponent.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

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
        ::rtl::OUString             m_sSqlStatement;
        // Properties
        sal_Int32                   m_nResultSetConcurrency;
        sal_Int32                   m_nResultSetType;
        sal_Bool                    m_bEscapeProcessing;
        ::cppu::OBroadcastHelper&   rBHelper;


    // statische Daten fuer die Klasse
        static jclass theClass;
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );

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

    public:
        static jclass getMyClass();

        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Statement_Base( JNIEnv * pEnv, java_sql_Connection* _pCon );

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
        OStatement_BASE2(JNIEnv * pEnv, java_sql_Connection* _pCon ) : java_sql_Statement_Base( pEnv, _pCon ),
                                OSubComponent<OStatement_BASE2, java_sql_Statement_BASE>((::cppu::OWeakObject*)_pCon, this){}

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
        // der Destruktor um den Object-Counter zu aktualisieren
        static void saveClassRef( jclass pClass );

        virtual void createStatement(JNIEnv* _pEnv);

        virtual ~java_sql_Statement();
    public:
        DECLARE_SERVICE_INFO();
        static jclass getMyClass();

        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Statement( JNIEnv * pEnv, java_sql_Connection* _pCon ) : OStatement_BASE2( pEnv, _pCon){};

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

