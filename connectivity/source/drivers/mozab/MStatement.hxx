/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MStatement.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:45:30 $
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
#ifndef CONNECTIVITY_SSTATEMENT_HXX
#define CONNECTIVITY_SSTATEMENT_HXX

#ifndef _COM_SUN_STAR_SDBC_XSTATEMENT_HPP_
#include <com/sun/star/sdbc/XStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
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
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef INCLUDED_LIST
#include <list>
#define INCLUDED_LIST
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include "connectivity/sqliterator.hxx"
#endif
#ifndef _CONNECTIVITY_PARSE_SQLPARSE_HXX_
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include <connectivity/FValue.hxx>
#endif
#ifndef CONNECTIVITY_TSORTINDEX_HXX
#include "TSortIndex.hxx"
#endif
#include "MConnection.hxx"
#include "MTable.hxx"

#include <memory>

namespace connectivity
{
    namespace mozab
    {
        class OResultSet;

        typedef ::cppu::WeakComponentImplHelper3<   ::com::sun::star::sdbc::XStatement,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::sdbc::XCloseable> OStatement_BASE;

        //**************************************************************
        //************ Class: OStatement_Base
        // is a base class for the normal statement and for the prepared statement
        //**************************************************************
        class OStatement_Base       :   public comphelper::OBaseMutex,
                                        public  OStatement_BASE,
                                        public  ::cppu::OPropertySetHelper,
                                        public  ::comphelper::OPropertyArrayUsageHelper<OStatement_Base>

        {
        ::com::sun::star::sdbc::SQLWarning                            m_aLastWarning;
        protected:
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> m_xDBMetaData;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>  m_xColNames; // table columns

            //  for this Statement

            ::std::list< ::rtl::OUString>               m_aBatchList;

            OTable*                                     m_pTable;
            OConnection*                                m_pConnection;  // The owning Connection object

            OValueRow                                   m_aRow;

            connectivity::OSQLParser                    m_aParser;
            ::std::auto_ptr< connectivity::OSQLParseTreeIterator >
                                                        m_pSQLIterator;

            connectivity::OSQLParseNode*                m_pParseTree;

            ::std::vector<sal_Int32>                    m_aColMapping;
            ::std::vector<sal_Int32>                    m_aOrderbyColumnNumber;
            ::std::vector<TAscendingOrder>              m_aOrderbyAscending;

            ::cppu::OBroadcastHelper&                   rBHelper;

        protected:

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
                                                                const ::com::sun::star::uno::Any& rValue)   throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                                                ::com::sun::star::uno::Any& rValue,
                                                                sal_Int32 nHandle) const;
            virtual ~OStatement_Base();

        protected:
            //
            // Driver Internal Methods
            //
            virtual sal_Bool parseSql( const ::rtl::OUString& sql , sal_Bool bAdjusted = sal_False) throw (
                        ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );

            OResultSet*  createResultSet();
            virtual void initializeResultSet( OResultSet* _pResult );
            void         createColumnMapping();
            void         analyseSQL();
            void         setOrderbyColumn( connectivity::OSQLParseNode* pColumnRef,
                                           connectivity::OSQLParseNode* pAscendingDescending);
            void         reset () throw( ::com::sun::star::sdbc::SQLException);
            void         clearMyResultSet () throw( ::com::sun::star::sdbc::SQLException);
            virtual void createTable(  ) throw (
                        ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );

        public:
            // other methods
            OConnection* getOwnConnection() const { return m_pConnection;}

            OStatement_Base(OConnection* _pConnection );
            using OStatement_BASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            // OComponentHelper
            virtual void SAL_CALL disposing(void){OStatement_BASE::disposing();}
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

        class OStatement_BASE2  :public OStatement_Base
                                ,public ::connectivity::OSubComponent<OStatement_BASE2, OStatement_BASE>

        {
            friend class OSubComponent<OStatement_BASE2, OStatement_BASE>;
        public:
            OStatement_BASE2(OConnection* _pConnection ) :  OStatement_Base(_pConnection ),
                                    ::connectivity::OSubComponent<OStatement_BASE2, OStatement_BASE>((::cppu::OWeakObject*)_pConnection, this){}
            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual void SAL_CALL release() throw();
        };

        class OStatement :  public OStatement_BASE2,
                            public ::com::sun::star::lang::XServiceInfo
        {
        protected:
            ~OStatement(){}
        public:
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OStatement( OConnection* _pConnection) : OStatement_BASE2( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
        };
    }
}
#endif // CONNECTIVITY_SSTATEMENT_HXX
