/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MTable.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-10-18 13:09:40 $
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

#ifndef _CONNECTIVITY_MOZAB_TABLE_HXX_
#define _CONNECTIVITY_MOZAB_TABLE_HXX_

#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _CONNECTIVITY_MOZAB_BCONNECTION_HXX_
#include "MConnection.hxx"
#endif

namespace connectivity
{
    namespace mozab
    {
        typedef ::connectivity::OTableHelper OTable_Base;

        class OTable :  public OTable_Base
        {
            OConnection*    m_pConnection;

        public:
            OTable( sdbcx::OCollection* _pTables, OConnection* _pConnection);
            OTable( sdbcx::OCollection* _pTables,
                    OConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description );

            OConnection* getConnection() { return m_pConnection;}

            sal_Bool isReadOnly() const { return sal_False; }

            ::rtl::OUString getTableName() const { return m_Name; }
            ::rtl::OUString getSchema() const { return m_SchemaName; }

            // OTableHelper overridables
            virtual sdbcx::OCollection* createColumns( const TStringVector& _rNames );
            virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames);
            virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames);
        private:
            using OTable_Base::getConnection;
        };
    }
}
#endif // _CONNECTIVITY_MOZAB_TABLE_HXX_

