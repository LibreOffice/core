/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabTables.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:55:28 $
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

#include "MacabTables.hxx"
#ifndef _CONNECTIVITY_MACAB_TABLE_HXX_
#include "MacabTable.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_CATALOG_HXX_
#include "MacabCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_CONNECTION_HXX_
#include "MacabConnection.hxx"
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace connectivity::macab;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

sdbcx::ObjectType MacabTables::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aSchema;
    aSchema = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%"));
    aName = _rName;

    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%"));
    ::rtl::OUString sEmpty;

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(), aSchema, aName, aTypes);

    sdbcx::ObjectType xRet = NULL;
    if (xResult.is())
    {
        Reference< XRow > xRow(xResult, UNO_QUERY);
        if (xResult->next()) // there can be only one table with this name
        {
            MacabTable* pRet = new MacabTable(
                    this,
                    static_cast<MacabCatalog&>(m_rParent).getConnection(),
                    aName,
                    xRow->getString(4),
                    xRow->getString(5),
                    sEmpty);
            xRet = pRet;
        }
    }
    ::comphelper::disposeComponent(xResult);

    return xRet;
}
// -------------------------------------------------------------------------
void MacabTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<MacabCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void MacabTables::disposing(void)
{
    m_xMetaData = NULL;
    OCollection::disposing();
}
