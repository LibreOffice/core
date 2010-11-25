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
#include "dbase/DIndexColumns.hxx"
#include "dbase/DTable.hxx"
#include "connectivity/sdbcx/VIndexColumn.hxx"
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include <connectivity/dbexception.hxx>

using namespace ::comphelper;

using namespace connectivity::dbase;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;


sdbcx::ObjectType ODbaseIndexColumns::createObject(const ::rtl::OUString& _rName)
{
    const ODbaseTable* pTable = m_pIndex->getTable();

    ::vos::ORef<OSQLColumns> aCols = pTable->getTableColumns();
    OSQLColumns::Vector::const_iterator aIter = find(aCols->get().begin(),aCols->get().end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));

    Reference< XPropertySet > xCol;
    if(aIter != aCols->get().end())
        xCol = *aIter;

    if(!xCol.is())
        return sdbcx::ObjectType();

    sdbcx::ObjectType xRet = new sdbcx::OIndexColumn(sal_True,_rName
                                                    ,getString(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)))
                                                    ,::rtl::OUString()
                                                    ,getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)))
                                                    ,getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)))
                                                    ,getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)))
                                                    ,getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)))
                                                    ,sal_False
                                                    ,sal_False
                                                    ,sal_False
                                                    ,pTable->getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers());

    return xRet;
}

// -------------------------------------------------------------------------
void ODbaseIndexColumns::impl_refresh() throw(RuntimeException)
{
    m_pIndex->refreshColumns();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseIndexColumns::createDescriptor()
{
    return new sdbcx::OIndexColumn(m_pIndex->getTable()->getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
sdbcx::ObjectType ODbaseIndexColumns::appendObject( const ::rtl::OUString& /*_rForName*/, const Reference< XPropertySet >& descriptor )
{
    return cloneDescriptor( descriptor );
}
// -----------------------------------------------------------------------------


