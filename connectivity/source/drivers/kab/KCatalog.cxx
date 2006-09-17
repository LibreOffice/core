/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: KCatalog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 02:51:07 $
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

#include "KCatalog.hxx"
#ifndef _CONNECTIVITY_KAB_CONNECTION_HXX_
#include "KConnection.hxx"
#endif
#ifndef _CONNECTIVITY_KAB_TABLES_HXX_
#include "KTables.hxx"
#endif

using namespace connectivity::kab;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::cppu;

// -------------------------------------------------------------------------
KabCatalog::KabCatalog(KabConnection* _pCon)
        : connectivity::sdbcx::OCatalog(_pCon),
          m_pConnection(_pCon),
          m_xMetaData(m_pConnection->getMetaData())
{
}
// -------------------------------------------------------------------------
void KabCatalog::refreshTables()
{
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%"));
    Reference< XResultSet > xResult = m_xMetaData->getTables(
        Any(),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%")),
        aTypes);

    if (xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aName;
        // const ::rtl::OUString& sDot = KabCatalog::getDot();

        while (xResult->next())
        {
            // aName = xRow->getString(2);
            // aName += sDot;
            aName = xRow->getString(3);
            aVector.push_back(aName);
        }
    }
    if (m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new KabTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void KabCatalog::refreshViews()
{
}
// -------------------------------------------------------------------------
void KabCatalog::refreshGroups()
{
}
// -------------------------------------------------------------------------
void KabCatalog::refreshUsers()
{
}
// -------------------------------------------------------------------------
const ::rtl::OUString& KabCatalog::getDot()
{
    static const ::rtl::OUString sDot = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("."));
    return sDot;
}
// -----------------------------------------------------------------------------

// XTablesSupplier
Reference< XNameAccess > SAL_CALL KabCatalog::getTables(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    try
    {
        if (!m_pTables)
            refreshTables();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pTables;
}
