/*************************************************************************
 *
 *  $RCSfile: FTables.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:22 $
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

#ifndef _CONNECTIVITY_FILE_TABLES_HXX_
#include "file/FTables.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _CONNECTIVITY_FILE_CATALOG_HXX_
#include "file/FCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_BCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
//#ifndef _CONNECTIVITY_FILE_OEMPTYCOLLECTION_HXX_
//#include "file/FEmptyCollection.hxx"
//#endif

using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

namespace starutil      = ::com::sun::star::util;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Reference< XNamed > OTables::createObject(const ::rtl::OUString& _rName)
{
//  ::rtl::OUString aName,aSchema;
//  sal_Int32 nLen = _rName.indexOf('.');
//  aSchema = _rName.copy(0,nLen);
//  aName   = _rName.copy(nLen+1);
//
//  Sequence< ::rtl::OUString > aTypes(1);
//  aTypes[0] = ::rtl::OUString::createFromAscii("%");
//  //  aTypes[0] = ::rtl::OUString::createFromAscii("TABLE");
//  //  aTypes[1] = ::rtl::OUString::createFromAscii("SYSTEMTABLE");
//
//  Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
//      aSchema,aName,aTypes);

    Reference< XNamed > xRet = NULL;
//  if(xResult.is())
//  {
//      Reference< XRow > xRow(xResult,UNO_QUERY);
//      if(xResult->next()) // there can be only one table with this name
//      {
//          OFileTable* pRet = new OFileTable(static_cast<OFileCatalog&>(m_rParent).getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers(),
//                                      static_cast<OFileCatalog&>(m_rParent).getConnection(),
//                                      aName,xRow->getString(4),xRow->getString(5),aSchema);
//          xRet = pRet;
//      }
//  }
//
    return xRet;
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OFileCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OTables::disposing(void)
{
    m_xMetaData = NULL;
    OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OTables::createEmptyObject()
{
    return Reference< XPropertySet >();
}
//------------------------------------------------------------------
Any SAL_CALL OTables::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XColumnLocate>*)0) ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0) ||
        rType == ::getCppuType((const Reference<XAppend>*)0) ||
        rType == ::getCppuType((const Reference<XDrop>*)0))
        return Any();

    typedef sdbcx::OCollection OTables_BASE;
    return OTables_BASE::queryInterface(rType);
}

