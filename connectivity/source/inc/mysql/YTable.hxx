/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: YTable.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:04:54 $
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

#ifndef CONNECTIVITY_MYSQL_TABLE_HXX
#define CONNECTIVITY_MYSQL_TABLE_HXX

#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
#include <comphelper/IdPropArrayHelper.hxx>
#endif

namespace connectivity
{
    namespace mysql
    {

        ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

        class OMySQLTable;
        typedef ::comphelper::OIdPropertyArrayUsageHelper< OMySQLTable >    OMySQLTable_PROP;
        class OMySQLTable : public OTableHelper
                            ,public OMySQLTable_PROP
        {
            sal_Int32 m_nPrivileges; // we have to set our privileges by our own

            /** executes the statmenmt.
                @param  _rStatement
                    The statement to execute.
                */
            void executeStatement(const ::rtl::OUString& _rStatement );
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

            /** used to implement the creation of the array helper which is shared amongst all instances of the class.
                This method needs to be implemented in derived classes.
                <BR>
                The method gets called with s_aMutex acquired.
                <BR>
                as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
                assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
                @return                         an pointer to the newly created array helper. Must not be NULL.
            */
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        public:
            OMySQLTable(    sdbcx::OCollection* _pTables,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);
            OMySQLTable(    sdbcx::OCollection* _pTables,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                            const ::rtl::OUString& _Name,
                            const ::rtl::OUString& _Type,
                            const ::rtl::OUString& _Description = ::rtl::OUString(),
                            const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                            const ::rtl::OUString& _CatalogName = ::rtl::OUString(),
                            sal_Int32 _nPrivileges = 0
                );

            // ODescriptor
            virtual void construct();
            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            /** returns the ALTER TABLE XXX statement
            */
            ::rtl::OUString getAlterTableColumnPart();

            // some methods to alter table structures
            void alterColumnType(sal_Int32 nNewType,const ::rtl::OUString& _rColName,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDescriptor);
            void alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName);
            void dropDefaultValue(const ::rtl::OUString& _sNewDefault);

        };
    }
}
#endif // CONNECTIVITY_MYSQL_TABLE_HXX

