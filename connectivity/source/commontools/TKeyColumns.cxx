/*************************************************************************
 *
 *  $RCSfile: TKeyColumns.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2002-10-25 09:01:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONNECTIVITY_TKEYCOLUMNS_HXX
#include "connectivity/TKeyColumns.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_
#include "connectivity/sdbcx/VKeyColumn.hxx"
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
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
OKeyColumnsHelper::OKeyColumnsHelper(   OTableKeyHelper* _pKey,
                ::osl::Mutex& _rMutex,
                const ::std::vector< ::rtl::OUString> &_rVector)
            : connectivity::sdbcx::OCollection(*_pKey,sal_True,_rMutex,_rVector)
            ,m_pKey(_pKey)
{
}
// -------------------------------------------------------------------------
Reference< XNamed > OKeyColumnsHelper::createObject(const ::rtl::OUString& _rName)
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUString aSchema,aTable;
    m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))   >>= aSchema;
    m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))         >>= aTable;

    // frist get the related column to _rName
    Reference< XResultSet > xResult = m_pKey->getTable()->getMetaData()->getImportedKeys(
            m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),aSchema,aTable);

    ::rtl::OUString aRefColumnName;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aTemp;
        while(xResult->next())
        {
            aTemp = xRow->getString(4);
            if(xRow->getString(8) == _rName && m_pKey->getName() == xRow->getString(12))
            {
                aRefColumnName = aTemp;
                break;
            }
        }
    }

    Reference< XNamed > xRet;

    // now describe the column _rName and set his related column
    xResult = m_pKey->getTable()->getMetaData()->getColumns(
                m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),aSchema,aTable,_rName);

    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if ( xResult->next() )
        {
            if ( xRow->getString(4) == _rName )
            {
                sal_Int32 nDataType = xRow->getInt(5);
                ::rtl::OUString aTypeName(xRow->getString(6));
                sal_Int32 nSize = xRow->getInt(7);
                sal_Int32 nDec  = xRow->getInt(9);
                sal_Int32 nNull = xRow->getInt(11);
                ::rtl::OUString sColumnDef;
                try
                {
                    sColumnDef = xRow->getString(13);
                }
                catch(const SQLException&)
                {
                    // somethimes we get an error when asking for this param
                }

                OKeyColumn* pRet = new OKeyColumn(aRefColumnName,
                                                    _rName,
                                                    aTypeName,
                                                    sColumnDef,
                                                    nNull,
                                                    nSize,
                                                    nDec,
                                                    nDataType,
                                                    sal_False,
                                                    sal_False,
                                                    sal_False,
                                                    isCaseSensitive());
                xRet = pRet;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeyColumnsHelper::createEmptyObject()
{
    return new OKeyColumn(isCaseSensitive());
}
// -------------------------------------------------------------------------
void OKeyColumnsHelper::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    m_pKey->refreshColumns();
}
// -----------------------------------------------------------------------------
Reference< XNamed > OKeyColumnsHelper::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    OKeyColumn* pColumn = new OKeyColumn(isCaseSensitive());
    Reference<XPropertySet> xProp = pColumn;
    ::comphelper::copyProperties(_xDescriptor,xProp);
    Reference< XNamed > xName(xProp,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Must be a XName interface here !");
    return xName;
}
// -----------------------------------------------------------------------------


