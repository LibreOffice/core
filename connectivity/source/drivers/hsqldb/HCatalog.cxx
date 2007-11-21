/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HCatalog.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:00:39 $
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

#ifndef CONNECTIVITY_HSQLDB_CATALOG_HXX
#include "hsqldb/HCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_HSQLDB_USERS_HXX_
#include "hsqldb/HUsers.hxx"
#endif
#ifndef CONNECTIVITY_HSQLDB_TABLES_HXX
#include "hsqldb/HTables.hxx"
#endif
#ifndef _CONNECTIVITY_HSQLDB_VIEWS_HXX_
#include "hsqldb/HViews.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif


// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::hsqldb;
//using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OHCatalog::OHCatalog(const Reference< XConnection >& _xConnection) : sdbcx::OCatalog(_xConnection)
                ,m_xConnection(_xConnection)
{
}
// -----------------------------------------------------------------------------
void OHCatalog::refreshObjects(const Sequence< ::rtl::OUString >& _sKindOfObject,TStringVector& _rNames)
{
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
                                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),
                                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),
                                                            _sKindOfObject);
    fillNames(xResult,_rNames);
}
// -------------------------------------------------------------------------
void OHCatalog::refreshTables()
{
    TStringVector aVector;
    static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
    static const ::rtl::OUString s_sTableTypeTable(RTL_CONSTASCII_USTRINGPARAM("TABLE"));

    Sequence< ::rtl::OUString > sTableTypes(2);
    sTableTypes[0] = s_sTableTypeView;
    sTableTypes[1] = s_sTableTypeTable;

    refreshObjects(sTableTypes,aVector);

    if ( m_pTables )
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OHCatalog::refreshViews()
{
    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW"));

    sal_Bool bSupportsViews = sal_False;
    try
    {
        Reference<XResultSet> xRes = m_xMetaData->getTableTypes();
        Reference<XRow> xRow(xRes,UNO_QUERY);
        while ( xRow.is() && xRes->next() )
        {
            if ( (bSupportsViews = xRow->getString(1).equalsIgnoreAsciiCase(aTypes[0])) )
            {
                break;
            }
        }
    }
    catch(const SQLException&)
    {
    }

    TStringVector aVector;
    if ( bSupportsViews )
        refreshObjects(aTypes,aVector);

    if ( m_pViews )
        m_pViews->reFill(aVector);
    else
        m_pViews = new HViews( m_xConnection, *this, m_aMutex, aVector );
}
// -------------------------------------------------------------------------
void OHCatalog::refreshGroups()
{
}
// -------------------------------------------------------------------------
void OHCatalog::refreshUsers()
{
    TStringVector aVector;
    Reference< XStatement > xStmt = m_xConnection->createStatement(  );
    Reference< XResultSet >  xResult = xStmt->executeQuery(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("select User from hsqldb.user group by User")));
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        TString2IntMap aMap;
        while( xResult->next() )
            aVector.push_back(xRow->getString(1));
        ::comphelper::disposeComponent(xResult);
    }
    ::comphelper::disposeComponent(xStmt);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(*this,m_aMutex,aVector,m_xConnection,this);
}
// -----------------------------------------------------------------------------
Any SAL_CALL OHCatalog::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if ( rType == ::getCppuType((const Reference<XGroupsSupplier>*)0) )
        return Any();

    return OCatalog::queryInterface(rType);
}
// -----------------------------------------------------------------------------
Sequence< Type > SAL_CALL OHCatalog::getTypes(  ) throw(RuntimeException)
{
    Sequence< Type > aTypes = OCatalog::getTypes();
    ::std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if ( !(*pBegin == ::getCppuType((const Reference<XGroupsSupplier>*)0)))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    const Type* pTypes = aOwnTypes.empty() ? 0 : &aOwnTypes[0];
    return Sequence< Type >(pTypes, aOwnTypes.size());
}
// -----------------------------------------------------------------------------


