/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TDatabaseMetaDataBase.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:59:47 $
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

#ifndef _CONNECTIVITY_ODATABASEMETADATABASE_HXX_
#define _CONNECTIVITY_ODATABASEMETADATABASE_HXX_

#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA2_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData2.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#include "FDatabaseMetaDataResultSet.hxx"
#include <functional>

namespace connectivity
{
        typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >  TConditions;
        class ODatabaseMetaDataBase :   public  comphelper::OBaseMutex,
                                        public ::cppu::WeakImplHelper2< ::com::sun::star::sdbc::XDatabaseMetaData2,
                                                                        ::com::sun::star::lang::XEventListener>
        {
        private:
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aConnectionInfo;
            ::connectivity::ODatabaseMetaDataResultSet::ORows                           m_aTypeInfoRows;

            // cached database information
            ::std::pair<bool,sal_Bool>          m_isCatalogAtStart;
            ::std::pair<bool,::rtl::OUString>   m_sCatalogSeparator;
            ::std::pair<bool,::rtl::OUString>   m_sIdentifierQuoteString;
            ::std::pair<bool,sal_Bool>          m_supportsCatalogsInTableDefinitions;
            ::std::pair<bool,sal_Bool>          m_supportsSchemasInTableDefinitions;
            ::std::pair<bool,sal_Bool>          m_supportsCatalogsInDataManipulation;
            ::std::pair<bool,sal_Bool>          m_supportsSchemasInDataManipulation;
            ::std::pair<bool,sal_Bool>          m_supportsMixedCaseQuotedIdentifiers;
            ::std::pair<bool,sal_Bool>          m_supportsAlterTableWithAddColumn;
            ::std::pair<bool,sal_Bool>          m_supportsAlterTableWithDropColumn;
            ::std::pair<bool,sal_Int32>         m_MaxStatements;
            ::std::pair<bool,sal_Int32>         m_MaxTablesInSelect;
            ::std::pair<bool,sal_Bool>          m_storesMixedCaseQuotedIdentifiers;

            template <typename T> T callImplMethod(::std::pair<bool,T>& _rCache,const ::std::mem_fun_t<T,ODatabaseMetaDataBase>& _pImplMethod)
            {
                ::osl::MutexGuard aGuard( m_aMutex );
                if ( !_rCache.first )
                {
                    _rCache.second = _pImplMethod(this);
                    _rCache.first = true;
                }
                return _rCache.second;
            }
        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >     m_xConnection;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>   m_xListenerHelper; // forward the calls from the connection to me

            virtual ~ODatabaseMetaDataBase();

        protected:
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > impl_getTypeInfo_throw() = 0;
            // cached database information
            virtual ::rtl::OUString impl_getIdentifierQuoteString_throw(  )             = 0;
            virtual sal_Bool        impl_isCatalogAtStart_throw(  )                     = 0;
            virtual ::rtl::OUString impl_getCatalogSeparator_throw(  )                  = 0;
            virtual sal_Bool        impl_supportsCatalogsInTableDefinitions_throw(  )   = 0;
            virtual sal_Bool        impl_supportsSchemasInTableDefinitions_throw(  )    = 0;
            virtual sal_Bool        impl_supportsCatalogsInDataManipulation_throw(  )   = 0;
            virtual sal_Bool        impl_supportsSchemasInDataManipulation_throw(  )    = 0;
            virtual sal_Bool        impl_supportsMixedCaseQuotedIdentifiers_throw(  )   = 0;
            virtual sal_Bool        impl_supportsAlterTableWithAddColumn_throw(  )      = 0;
            virtual sal_Bool        impl_supportsAlterTableWithDropColumn_throw(  )     = 0;
            virtual sal_Int32       impl_getMaxStatements_throw(  )                     = 0;
            virtual sal_Int32       impl_getMaxTablesInSelect_throw(  )                 = 0;
            virtual sal_Bool        impl_storesMixedCaseQuotedIdentifiers_throw(  )     = 0;


        public:

            ODatabaseMetaDataBase(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo);

            // XDatabaseMetaData2
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getConnectionInfo(  ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTypeInfo(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getProcedures( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& procedureNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getProcedureColumns( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& procedureNamePattern, const ::rtl::OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getSchemas(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getCatalogs(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getColumnPrivileges( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, const ::rtl::OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTablePrivileges( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getBestRowIdentifier( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope, sal_Bool nullable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getVersionColumns( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getPrimaryKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getImportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getExportedKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getCrossReference( const ::com::sun::star::uno::Any& primaryCatalog, const ::rtl::OUString& primarySchema, const ::rtl::OUString& primaryTable, const ::com::sun::star::uno::Any& foreignCatalog, const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getIndexInfo( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Bool unique, sal_Bool approximate ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // cached database information
            virtual ::rtl::OUString SAL_CALL getIdentifierQuoteString(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isCatalogAtStart(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getCatalogSeparator(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsCatalogsInTableDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsSchemasInTableDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsCatalogsInDataManipulation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsSchemasInDataManipulation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsAlterTableWithAddColumn(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsAlterTableWithDropColumn(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMaxStatements(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMaxTablesInSelect(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
}
#endif // _CONNECTIVITY_ODATABASEMETADATABASE_HXX_
