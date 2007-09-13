/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabTable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:55:10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "MacabTable.hxx"
#ifndef _CONNECTIVITY_MACAB_TABLES_HXX_
#include "MacabTables.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_COLUMNS_HXX_
#include "MacabColumns.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_CATALOG_HXX_
#include "MacabCatalog.hxx"
#endif

using namespace connectivity::macab;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
MacabTable::MacabTable( sdbcx::OCollection* _pTables, MacabConnection* _pConnection)
    : MacabTable_TYPEDEF(_pTables, sal_True),
    m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
MacabTable::MacabTable( sdbcx::OCollection* _pTables,
                MacabConnection* _pConnection,
                const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Type,
                const ::rtl::OUString& _Description ,
                const ::rtl::OUString& _SchemaName,
                const ::rtl::OUString& _CatalogName
                ) : MacabTable_TYPEDEF(_pTables,sal_True,
                                  _Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName),
                    m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
void MacabTable::refreshColumns()
{
    TStringVector aVector;

    if (!isNew())
    {
        Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(
                Any(),
                m_SchemaName,
                m_Name,
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")));

        if (xResult.is())
        {
        Reference< XRow > xRow(xResult, UNO_QUERY);
        while (xResult->next())
                aVector.push_back(xRow->getString(4));
        }
    }

    if (m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new MacabColumns(this,m_aMutex,aVector);
}
