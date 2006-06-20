/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BTable.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:59:33 $
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

#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#define _CONNECTIVITY_ADABAS_TABLE_HXX_

#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#include "connectivity/sdbcx/VTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
#endif

namespace connectivity
{
    namespace adabas
    {
        typedef connectivity::OTableHelper OTable_TYPEDEF;

        ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

        class OAdabasTable : public OTableHelper
        {
            OAdabasConnection* m_pConnection;
        protected:
            /** creates the column collection for the table
                @param  _rNames
                    The column names.
            */
            virtual sdbcx::OCollection* createColumns(const TStringVector& _rNames);

            /** creates the key collection for the table
                @param  _rNames
                    The key names.
            */
            virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames);

            /** creates the index collection for the table
                @param  _rNames
                    The index names.
            */
            virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames);

        public:
            OAdabasTable(   sdbcx::OCollection* _pTables,
                            OAdabasConnection* _pConnection);
            OAdabasTable(   sdbcx::OCollection* _pTables,
                            OAdabasConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            ::rtl::OUString getTableName() const { return m_Name; }
            ::rtl::OUString getSchema() const { return m_SchemaName; }

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            // XNamed
            virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
            /**
                returns the ALTER TABLE XXX COLUMN statement
            */
            ::rtl::OUString getAlterTableColumnPart(const ::rtl::OUString& _rsColumnName );

            // starts a sql transaaction
            void beginTransAction();
            // rolls back a sql transaaction
            void rollbackTransAction();
            // ends a sql transaaction
            void endTransAction();
            // some methods to alter table structures
            void alterColumnType(const ::rtl::OUString& _rColName,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDescriptor);
            void alterNotNullValue(sal_Int32 _nNewNullable,const ::rtl::OUString& _rColName);
            void alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName);
            void dropDefaultValue(const ::rtl::OUString& _sNewDefault);
            void addDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName);
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_TABLE_HXX_

