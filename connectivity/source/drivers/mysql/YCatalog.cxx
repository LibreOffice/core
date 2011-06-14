/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "mysql/YCatalog.hxx"
#include "mysql/YUsers.hxx"
#include "mysql/YTables.hxx"
#include "mysql/YViews.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <comphelper/types.hxx>


// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::mysql;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OMySQLCatalog::OMySQLCatalog(const Reference< XConnection >& _xConnection) : OCatalog(_xConnection)
                ,m_xConnection(_xConnection)
{
}
// -----------------------------------------------------------------------------
void OMySQLCatalog::refreshObjects(const Sequence< ::rtl::OUString >& _sKindOfObject,TStringVector& _rNames)
{
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
                                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),
                                                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),
                                                            _sKindOfObject);
    fillNames(xResult,_rNames);
}
// -------------------------------------------------------------------------
void OMySQLCatalog::refreshTables()
{
    TStringVector aVector;
    static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
    static const ::rtl::OUString s_sTableTypeTable(RTL_CONSTASCII_USTRINGPARAM("TABLE"));
    static const ::rtl::OUString s_sAll(RTL_CONSTASCII_USTRINGPARAM("%"));

    Sequence< ::rtl::OUString > sTableTypes(3);
    sTableTypes[0] = s_sTableTypeView;
    sTableTypes[1] = s_sTableTypeTable;
    sTableTypes[2] = s_sAll;    // just to be sure to include anything else ....

    refreshObjects(sTableTypes,aVector);

    if ( m_pTables )
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OMySQLCatalog::refreshViews()
{
    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW"));

/*
    sal_Bool bSupportsViews = sal_False;
    try
    {
        Reference<XResultSet> xRes = m_xMetaData->getTableTypes();
        Reference<XRow> xRow(xRes,UNO_QUERY);
        while ( !bSupportsViews && xRow.is() && xRes->next() )
        {
            ::rtl::OUString sTableType( xRow->getString( 1 ) );
            bSupportsViews = sTableType.equalsIgnoreAsciiCase( aTypes[0] );
        }
    }
    catch(const SQLException&)
    {
    }
*/
    // let's simply assume the server is new enough to support views. Current drivers
    // as of this writing might not return the proper information in getTableTypes, so
    // don't rely on it.
    // during #73245# / 2007-10-26 / frank.schoenheit@sun.com
    bool bSupportsViews = sal_True;

    TStringVector aVector;
    if ( bSupportsViews )
        refreshObjects(aTypes,aVector);

    if ( m_pViews )
        m_pViews->reFill(aVector);
    else
        m_pViews = new OViews(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OMySQLCatalog::refreshGroups()
{
}
// -------------------------------------------------------------------------
void OMySQLCatalog::refreshUsers()
{
    TStringVector aVector;
    Reference< XStatement > xStmt = m_xConnection->createStatement(  );
    Reference< XResultSet >  xResult = xStmt->executeQuery(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("select User from mysql.user group by User")));
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
Any SAL_CALL OMySQLCatalog::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if ( rType == ::getCppuType((const Reference<XGroupsSupplier>*)0) )
        return Any();


    return OCatalog::queryInterface(rType);
}
// -----------------------------------------------------------------------------
Sequence< Type > SAL_CALL OMySQLCatalog::getTypes(  ) throw(RuntimeException)
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
