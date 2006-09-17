 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NColumns.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 02:28:47 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "NColumns.hxx"
#ifndef _CONNECTIVITY_EVOAB_TABLE_HXX_
#include "NTable.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_TABLES_HXX_
#include "NTables.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_CATALOG_HXX_
#include "NCatalog.hxx"
#endif

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif

using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace connectivity::evoab;

// -------------------------------------------------------------------------
sdbcx::ObjectType OEvoabColumns::createObject(const ::rtl::OUString& _rName)
{
    Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getColumns(
        Any(),
        m_pTable->getSchema(),
        m_pTable->getTableName(),
        _rName);

    sdbcx::ObjectType xRet = NULL;
    if (xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);

        while (xResult->next())
        {
            if (xRow->getString(4) == _rName)
            {
                OColumn* pRet = new OColumn(
                        _rName,
                        xRow->getString(6),
                        xRow->getString(13),
                        xRow->getInt(11),
                        xRow->getInt(7),
                        xRow->getInt(9),
                        xRow->getInt(5),
                        sal_False,
                        sal_False,
                        sal_False,
                        sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OEvoabColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
