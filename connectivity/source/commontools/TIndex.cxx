/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TIndex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:11:01 $
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

#ifndef CONNECTIVITY_INDEXHELPER_HXX_
#include "connectivity/TIndex.hxx"
#endif
#ifndef CONNECTIVITY_INDEXCOLUMNSHELPER_HXX
#include "connectivity/TIndexColumns.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
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
OIndexHelper::OIndexHelper( OTableHelper* _pTable) : connectivity::sdbcx::OIndex(sal_True)
                 , m_pTable(_pTable)
{
    construct();
    ::std::vector< ::rtl::OUString> aVector;
    m_pColumns  = new OIndexColumns(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
OIndexHelper::OIndexHelper( OTableHelper* _pTable,
                const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Catalog,
                sal_Bool _isUnique,
                sal_Bool _isPrimaryKeyIndex,
                sal_Bool _isClustered
                ) : connectivity::sdbcx::OIndex(_Name,
                                  _Catalog,
                                  _isUnique,
                                  _isPrimaryKeyIndex,
                                  _isClustered,sal_True)
                ,m_pTable(_pTable)
{
    construct();
    refreshColumns();
}
// -------------------------------------------------------------------------

void OIndexHelper::refreshColumns()
{
    if ( !m_pTable )
        return;

    ::std::vector< ::rtl::OUString> aVector;
    if ( !isNew() )
    {
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        ::rtl::OUString aSchema,aTable;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

        Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
            aSchema,aTable,sal_False,sal_False);

        if ( xResult.is() )
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            ::rtl::OUString aColName;
            while( xResult->next() )
            {
                if ( xRow->getString(6) == m_Name )
                {
                    aColName = xRow->getString(9);
                    if ( !xRow->wasNull() )
                        aVector.push_back(aColName);
                }
            }
        }
    }
    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OIndexColumns(this,m_aMutex,aVector);
}
// -----------------------------------------------------------------------------

