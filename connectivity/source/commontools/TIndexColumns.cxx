/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TIndexColumns.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:18:59 $
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

#ifndef CONNECTIVITY_INDEXCOLUMNSHELPER_HXX
#include "connectivity/TIndexColumns.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEXCOLUMN_HXX_
#include "connectivity/sdbcx/VIndexColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef CONNECTIVITY_INDEXHELPER_HXX_
#include "connectivity/TIndex.hxx"
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OIndexColumns::OIndexColumns(   OIndexHelper* _pIndex,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector)
            : connectivity::sdbcx::OCollection(*_pIndex,sal_True,_rMutex,_rVector)
            ,m_pIndex(_pIndex)
{
}
// -------------------------------------------------------------------------
sdbcx::ObjectType OIndexColumns::createObject(const ::rtl::OUString& _rName)
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUString aSchema,aTable;
    m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
    m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

    Reference< XResultSet > xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getIndexInfo(
        m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
        aSchema,aTable,sal_False,sal_False);

    sal_Bool bAsc = sal_True;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aD(::rtl::OUString::createFromAscii("D"));
        while( xResult->next() )
        {
            if(xRow->getString(9) == _rName)
                bAsc = xRow->getString(10) != aD;
        }
    }

    xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getColumns(
        m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
        aSchema,aTable,_rName);

    sdbcx::ObjectType xRet = NULL;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while( xResult->next() )
        {
            if ( xRow->getString(4) == _rName )
            {
                sal_Int32 nDataType = xRow->getInt(5);
                ::rtl::OUString aTypeName(xRow->getString(6));
                sal_Int32 nSize = xRow->getInt(7);
                sal_Int32 nDec  = xRow->getInt(9);
                sal_Int32 nNull = xRow->getInt(11);
                ::rtl::OUString aColumnDef(xRow->getString(13));

                OIndexColumn* pRet = new OIndexColumn(bAsc,
                                                    _rName,
                                                    aTypeName,
                                                    aColumnDef,
                                                    nNull,
                                                    nSize,
                                                    nDec,
                                                    nDataType,
                                                    sal_False,sal_False,sal_False,sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexColumns::createDescriptor()
{
    return new OIndexColumn(sal_True);
}
// -------------------------------------------------------------------------
void OIndexColumns::impl_refresh() throw(RuntimeException)
{
    m_pIndex->refreshColumns();
}
// -----------------------------------------------------------------------------
