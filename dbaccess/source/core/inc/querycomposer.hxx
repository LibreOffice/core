/*************************************************************************
 *
 *  $RCSfile: querycomposer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:41:49 $
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
#ifndef DBACCESS_CORE_API_QUERYCOMPOSER_HXX
#define DBACCESS_CORE_API_QUERYCOMPOSER_HXX

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include <connectivity/sqliterator.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif



namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormatsSupplier;
    class XNumberFormatter;
}}}}

namespace dbaccess
{
    typedef ::cppu::ImplHelper4< ::com::sun::star::sdb::XSQLQueryComposer,
                                    ::com::sun::star::sdbcx::XTablesSupplier,
                                    ::com::sun::star::sdbcx::XColumnsSupplier,
                                    ::com::sun::star::lang::XServiceInfo    > OQueryComposer_BASE;

    class OPrivateColumns;
    class OPrivateTables;

    class OQueryComposer :  public ::comphelper::OBaseMutex,
                            public OSubComponent,
                            public OQueryComposer_BASE
    {
        ::connectivity::OSQLParser              m_aSqlParser;
        ::connectivity::OSQLParseTreeIterator   m_aSqlIterator;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>              m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>         m_xTableSupplier;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xNumberFormatsSupplier;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>         m_xColumns;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;

        ::connectivity::OSQLParseNode*          m_pSqlParseNode;

        OPrivateColumns*                        m_pColumns;
        OPrivateTables*                         m_pTables;

        ::rtl::OUString                         m_aQuery;
        ::rtl::OUString                         m_aFilter;  // curently used where clause
        ::rtl::OUString                         m_aOrder;   // curently used order by clause

        ::rtl::OUString                         m_aWorkSql;
        ::rtl::OUString                         m_aOrgFilter;
        ::rtl::OUString                         m_aOrgOrder;
        ::com::sun::star::lang::Locale          m_aLocale;


        sal_Bool setORCriteria(::connectivity::OSQLParseNode* pCondition,
            ::std::vector< ::std::vector < ::com::sun::star::beans::PropertyValue > >& rFilters, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const;
        sal_Bool setANDCriteria(::connectivity::OSQLParseNode* pCondition,
            ::std::vector < ::com::sun::star::beans::PropertyValue > & rFilters, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const;
        sal_Bool setComparsionPredicate(::connectivity::OSQLParseNode* pCondition,
            ::std::vector < ::com::sun::star::beans::PropertyValue > & rFilters, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const;

        ::rtl::OUString getColumnName(::connectivity::OSQLParseNode* pColumnRef) const;
        void resetIterator(const ::rtl::OUString& aSql);
        ::rtl::OUString getComposedSort() const;
        ::rtl::OUString getComposedFilter() const;
        ::rtl::OUString getGroupBy() const;
    public:

        OQueryComposer( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTableSupplier,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory);

        ~OQueryComposer();

        DECLARE_CTY_DEFAULTS(OSubComponent);  // impl acquire and release

        void SAL_CALL disposing(void);
        // ::com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);
        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
                throw(::com::sun::star::uno::RuntimeException);
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        // XSQLQueryComposer
        virtual ::rtl::OUString SAL_CALL getQuery(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setQuery( const ::rtl::OUString& command ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getComposedQuery(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getFilter(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL getStructuredFilter(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getOrder(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendFilterByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendOrderByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column, sal_Bool ascending ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFilter( const ::rtl::OUString& filter ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setOrder( const ::rtl::OUString& order ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XTablesSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
        // XColumnsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);
    };
}
#endif // DBACCESS_CORE_API_QUERYCOMPOSER_HXX
