/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabTable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:55:19 $
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

#ifndef _CONNECTIVITY_MACAB_TABLE_HXX_
#define _CONNECTIVITY_MACAB_TABLE_HXX_

#ifndef _CONNECTIVITY_MACAB_CONNECTION_HXX_
#include "MacabConnection.hxx"
#endif

#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#include "connectivity/sdbcx/VTable.hxx"
#endif

namespace connectivity
{
    namespace macab
    {
        typedef connectivity::sdbcx::OTable MacabTable_TYPEDEF;

        ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

        class MacabTable : public MacabTable_TYPEDEF
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData;
            MacabConnection* m_pConnection;

        public:
            MacabTable( sdbcx::OCollection* _pTables, MacabConnection* _pConnection);
            MacabTable( sdbcx::OCollection* _pTables,
                    MacabConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            MacabConnection* getConnection() { return m_pConnection;}

            virtual void refreshColumns();

            ::rtl::OUString getTableName() const { return m_Name; }
            ::rtl::OUString getSchema() const { return m_SchemaName; }
        };
    }
}

#endif // _CONNECTIVITY_MACAB_TABLE_HXX_
