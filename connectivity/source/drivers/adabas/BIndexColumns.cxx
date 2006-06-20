/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BIndexColumns.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:09:10 $
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

#ifndef _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_
#include "adabas/BIndexColumns.hxx"
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
#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

using namespace connectivity::adabas;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
ObjectType OIndexColumns::createObject(const ::rtl::OUString& _rName)
{

    Reference< XResultSet > xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getIndexInfo(Any(),
                    m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getTableName(),sal_False,sal_False);

    sal_Bool bAsc = sal_True;
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aD(::rtl::OUString::createFromAscii("D"));
        while(xResult->next())
        {
            if(xRow->getString(9) == _rName)
                bAsc = xRow->getString(10) != aD;
        }
        ::comphelper::disposeComponent(xResult);
    }

    xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getColumns(Any(),
            m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getTableName(),_rName);

    ObjectType xRet = NULL;
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                sal_Int32 nType             = xRow->getInt(5);
                ::rtl::OUString sTypeName   = xRow->getString(6);
                sal_Int32 nPrec             = xRow->getInt(7);
                OAdabasCatalog::correctColumnProperties(nPrec,nType,sTypeName);

                OIndexColumn* pRet = new OIndexColumn(bAsc,
                                                    _rName,
                                                    sTypeName,
                                                    xRow->getString(13),
                                                    xRow->getInt(11),
                                                    nPrec,
                                                    xRow->getInt(9),
                                                    nType,
                                                    sal_False,sal_False,sal_False,sal_True);
                xRet = pRet;
                break;
            }
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexColumns::createEmptyObject()
{
    return new OIndexColumn(sal_True);
}
// -------------------------------------------------------------------------
ObjectType OIndexColumns::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    OIndexColumn* pColumn = new OIndexColumn(sal_True);
    Reference<XPropertySet> xProp = pColumn;
    ::comphelper::copyProperties(_xDescriptor,xProp);
    return xProp;
}
// -----------------------------------------------------------------------------
void OIndexColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    m_pIndex->refreshColumns();
}
// -----------------------------------------------------------------------------
void OIndexColumns::appendObject( const Reference< XPropertySet >& /*descriptor*/ )
{
    // nothing to do here
}
// -----------------------------------------------------------------------------



