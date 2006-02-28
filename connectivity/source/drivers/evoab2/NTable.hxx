 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NTable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-02-28 10:35:00 $
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
 *    Contributor(s): Jayant Madavi_______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_EVOAB_TABLE_HXX_
#define _CONNECTIVITY_EVOAB_TABLE_HXX_

#ifndef _CONNECTIVITY_EVOAB_CONNECTION_HXX_
#include "NConnection.hxx"
#endif

#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#include "connectivity/sdbcx/VTable.hxx"
#endif

namespace connectivity
{
    namespace evoab
    {
        typedef connectivity::sdbcx::OTable OEvoabTable_TYPEDEF;

        ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

        class OEvoabTable : public OEvoabTable_TYPEDEF
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData;
            OEvoabConnection* m_pConnection;

        public:
            OEvoabTable(    sdbcx::OCollection* _pTables, OEvoabConnection* _pConnection);
            OEvoabTable(    sdbcx::OCollection* _pTables,
                    OEvoabConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            OEvoabConnection* getConnection() { return m_pConnection;}

            virtual void refreshColumns();

            ::rtl::OUString getTableName() const { return m_Name; }
            ::rtl::OUString getSchema() const { return m_SchemaName; }
        };
    }
}

#endif // _CONNECTIVITY_EVOAB_TABLE_HXX_
