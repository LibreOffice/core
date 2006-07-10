/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DIndexColumns.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:25:20 $
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

#ifndef _CONNECTIVITY_DBASE_INDEXCOLUMNS_HXX_
#include "dbase/DIndexColumns.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEXCOLUMN_HXX_
#include "connectivity/sdbcx/VIndexColumn.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

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
    OSQLColumns::const_iterator aIter = find(aCols->begin(),aCols->end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));

    Reference< XPropertySet > xCol;
    if(aIter != aCols->end())
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
                                                    ,pTable->getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers());

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
    return new sdbcx::OIndexColumn(m_pIndex->getTable()->getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers());
}
// -------------------------------------------------------------------------
sdbcx::ObjectType ODbaseIndexColumns::appendObject( const ::rtl::OUString& /*_rForName*/, const Reference< XPropertySet >& descriptor )
{
    return cloneDescriptor( descriptor );
}
// -----------------------------------------------------------------------------


