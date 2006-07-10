/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AKeyColumns.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:23:45 $
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

#ifndef _CONNECTIVITY_ADABAS_KEYCOLUMNS_HXX_
#include "adabas/BKeyColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_KEYCOLUMN_HXX_
#include "adabas/BKeyColumn.hxx"
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

using namespace connectivity::adabas;
// -------------------------------------------------------------------------
sdbcx::ObjectType OKeyColumns::createObject(const ::rtl::OUString& _rName)
{

    Reference< starsdbc::XResultSet >
        xResult = m_pTable->getConnection()->getMetaData()->getImportedKeys(Any(),
                    m_pTable->getSchema(),m_pTable->getName());

    ::rtl::OUString aRefColumnName;
    if(xResult.is())
    {
        Reference< starsdbc::XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(8) == _rName)
            {
                aRefColumnName = xRow->getString(4);
                break;
            }
        }
    }

     xResult = m_pTable->getConnection()->getMetaData()->getColumns(Any(),
        m_pTable->getSchema(),m_pTable->getName(),_rName);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< starsdbc::XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                OKeyColumn* pRet = new OKeyColumn(aRefColumnName,
                                                    _rName,
                                                    xRow->getString(6),
                                                    xRow->getString(13),
                                                    xRow->getInt(11),
                                                    xRow->getInt(7),
                                                    xRow->getInt(9),
                                                    xRow->getInt(5),
                                                    sal_False);
                xRet = pRet;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeyColumns::createDescriptor()
{
    OKeyColumn* pNew = new OKeyColumn();
    return pNew;
}
// -------------------------------------------------------------------------

