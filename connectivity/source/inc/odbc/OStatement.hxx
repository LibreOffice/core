/*************************************************************************
 *
 *  $RCSfile: OStatement.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ODBC_OSTATEMENT_HXX_
#define _CONNECTIVITY_ODBC_OSTATEMENT_HXX_

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
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _UNOTOOLS_PROPERTY_ARRAY_HELPER_HXX_
#include <unotools/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OCONNECTION_HXX_
#include "odbc/OConnection.hxx"
#endif
#ifndef _LIST_
#include <list>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace connectivity
{
    namespace odbc
    {

        typedef ::cppu::WeakComponentImplHelper5<   ::com::sun::star::sdbc::XStatement,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::util::XCancellable,
                                                    ::com::sun::star::sdbc::XCloseable,
                                                    ::com::sun::star::sdbc::XMultipleResults> OStatement_BASE;

        //**************************************************************
        //************ Class: java.sql.Statement
        //**************************************************************
        class OStatement_Base       :   public OBaseMutex,
                                        public  OStatement_BASE,
                                        public  ::cppu::OPropertySetHelper,
                                        public  ::utl::OPropertyArrayUsageHelper<OStatement_Base>

        {
        ::com::sun::star::sdbc::SQLWarning                                           m_aLastWarning;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
                                                                        //  for this Statement
        protected:
            ::std::list< ::rtl::OUString>               m_aBatchList;

            OConnection*                                m_pConnection;// The owning Connection object
            SQLHANDLE                                   m_aStatementHandle;
            SQLUSMALLINT*                               m_pRowStatusArray;

            //using OStatement_BASE::rBHelper;
        private:

            sal_Int32 getQueryTimeOut()             const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getMaxFieldSize()             const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getMaxRows()                  const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getResultSetConcurrency()     const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getResultSetType()            const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchDirection()           const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchSize()                const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::rtl::OUString getCursorName()         const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Bool isUsingBookmarks()             const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            void setQueryTimeOut(sal_Int32 _par0)   throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setMaxFieldSize(sal_Int32 _par0)   throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setMaxRows(sal_Int32 _par0)        throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setResultSetType(sal_Int32 _par0)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setFetchDirection(sal_Int32 _par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setFetchSize(sal_Int32 _par0)      throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setResultSetConcurrency(sal_Int32 _par0)       throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setCursorName(const ::rtl::OUString &_par0)    throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setUsingBookmarks(sal_Bool _bUseBookmark)      throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        protected:

            void reset () throw( ::com::sun::star::sdbc::SQLException);
            void clearMyResultSet () throw( ::com::sun::star::sdbc::SQLException);
            void setWarning (const  ::com::sun::star::sdbc::SQLWarning &ex) throw( ::com::sun::star::sdbc::SQLException);
            sal_Bool lockIfNecessary (const ::rtl::OUString& sql) throw( ::com::sun::star::sdbc::SQLException);
            sal_Int32 getColumnCount () throw( ::com::sun::star::sdbc::SQLException);
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > getResultSet (sal_Bool checkCount) throw( ::com::sun::star::sdbc::SQLException);
            sal_Int32 getRowCount () throw( ::com::sun::star::sdbc::SQLException);
            sal_Int32 getStmtOption (short fOption) const throw( ::com::sun::star::sdbc::SQLException);

            void disposeResultSet();

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
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                 throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        public:
            ::cppu::OBroadcastHelper& rBHelper;
            DECLARE_CTY_ACQUIRE(OStatement_BASE);
            OStatement_Base(OConnection* _pConnection );
    //  #if SUPD > 569
            using OStatement_BASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;
    //#else
    //      using OStatement_BASE::operator ::com::sun::star::uno::XInterface*;
    //#endif
            // OComponentHelper
            virtual void SAL_CALL disposing(void){OStatement_BASE::disposing();}
            // XInterface
            virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException){}
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
            {
                return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
            }
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

            // other methods
            SQLHANDLE getConnectionHandle() { return m_pConnection->getConnection(); }
        };

        class OStatement_BASE2 : public OStatement_Base,
                     public OSubComponent< OStatement_BASE2>

        {
            friend class OSubComponent< OStatement_BASE2>;
        public:
            OStatement_BASE2(OConnection* _pConnection ) :  OStatement_Base(_pConnection ),
                                    OSubComponent< OStatement_BASE2>((::cppu::OWeakObject*)_pConnection){}
            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);
        };

        class OStatement :  public OStatement_BASE2,
                            public ::com::sun::star::sdbc::XBatchExecution,
                            public ::com::sun::star::lang::XServiceInfo
        {
        public:
            DECLARE_CTY_DEFAULTS(OStatement_BASE2);
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OStatement( OConnection* _pConnection) : OStatement_BASE2( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            // XBatchExecution
            virtual void SAL_CALL addBatch( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_ODBC_OSTATEMENT_HXX_

