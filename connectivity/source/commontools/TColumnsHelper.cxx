/*************************************************************************
 *
 *  $RCSfile: TColumnsHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:51:48 $
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

#ifndef CONNECTIVITY_COLUMNSHELPER_HXX
#include "connectivity/TColumnsHelper.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
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
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

using namespace ::comphelper;


using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

namespace connectivity
{
    class OColumnsHelperImpl
    {
    public:
        OColumnsHelperImpl(sal_Bool _bCase)
            : m_aColumnInfo(_bCase)
        {
        }
        ColumnInformationMap m_aColumnInfo;
    };
}

OColumnsHelper::OColumnsHelper( ::cppu::OWeakObject& _rParent
                                ,sal_Bool _bCase
                                ,::osl::Mutex& _rMutex
                                ,const TStringVector &_rVector
                                ,sal_Bool _bUseHardRef
            ) : OCollection(_rParent,_bCase,_rMutex,_rVector,sal_False,_bUseHardRef)
    ,m_pTable(NULL)
    ,m_pImpl(NULL)
{
}
// -----------------------------------------------------------------------------
OColumnsHelper::~OColumnsHelper()
{
    delete m_pImpl;
    m_pImpl = NULL;
}
// -----------------------------------------------------------------------------

Reference< XNamed > OColumnsHelper::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(m_pTable,"NO Table set. Error!");
    Reference<XConnection> xConnection = m_pTable->getConnection();

    if ( !m_pImpl )
        m_pImpl = new OColumnsHelperImpl(isCaseSensitive());

    sal_Bool bQueryInfo     = sal_True;
    sal_Bool bAutoIncrement = sal_False;
    sal_Bool bIsCurrency    = sal_False;
    sal_Int32 nDataType     = DataType::OTHER;

    ColumnInformationMap::iterator aFind = m_pImpl->m_aColumnInfo.find(_rName);
    if ( aFind == m_pImpl->m_aColumnInfo.end() ) // we have to fill it
    {
        Reference<XDatabaseMetaData> xMetaData = xConnection->getMetaData();
        sal_Bool bUseCatalogInSelect = isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
        sal_Bool bUseSchemaInSelect = isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);
        ::rtl::OUString sComposedName = ::dbtools::composeTableName(xMetaData,m_pTable,sal_True,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
        collectColumnInformation(xConnection,sComposedName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*")) ,m_pImpl->m_aColumnInfo);
        aFind = m_pImpl->m_aColumnInfo.find(_rName);
    }
    if ( aFind != m_pImpl->m_aColumnInfo.end() )
    {
        bQueryInfo      = sal_False;
        bAutoIncrement  = aFind->second.first.first;
        bIsCurrency     = aFind->second.first.second;
        nDataType       = aFind->second.second;
    }


    Reference< XNamed > xRet(::dbtools::createSDBCXColumn(  m_pTable,
                                                            xConnection,
                                                            _rName,
                                                            isCaseSensitive(),
                                                            bQueryInfo,
                                                            bAutoIncrement,
                                                            bIsCurrency,
                                                            nDataType),UNO_QUERY);
    return xRet;
}

// -------------------------------------------------------------------------
void OColumnsHelper::impl_refresh() throw(RuntimeException)
{
    if ( m_pTable )
        m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumnsHelper::createEmptyObject()
{
    return new OColumn(sal_True);
}
// -----------------------------------------------------------------------------
Reference< XNamed > OColumnsHelper::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    Reference<XPropertySet> xProp = createEmptyObject();
    ::comphelper::copyProperties(_xDescriptor,xProp);
    Reference< XNamed > xName(xProp,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Must be a XName interface here !");
    return xName;
}
// -----------------------------------------------------------------------------
// XAppend
void OColumnsHelper::appendObject( const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(m_pTable,"OColumnsHelper::appendByDescriptor: Table is null!");
    OSL_ENSURE(descriptor.is(),"OColumnsHelper::appendByDescriptor: descriptor is null!");

    if ( descriptor.is() && m_pTable && !m_pTable->isNew() )
    {
        Reference<XDatabaseMetaData> xMetaData = m_pTable->getConnection()->getMetaData();
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = xMetaData->getIdentifierQuoteString(  );

        aSql += ::dbtools::composeTableName(xMetaData,m_pTable,sal_True,::dbtools::eInTableDefinitions);
        aSql += ::rtl::OUString::createFromAscii(" ADD ");
        aSql += ::dbtools::createStandardColumnPart(descriptor,m_pTable->getConnection());

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        if ( xStmt.is() )
        {
            xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }
}
// -------------------------------------------------------------------------
// XDrop
void OColumnsHelper::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    OSL_ENSURE(m_pTable,"OColumnsHelper::dropByName: Table is null!");
    if ( m_pTable && !m_pTable->isNew() )
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        Reference<XDatabaseMetaData> xMetaData = m_pTable->getConnection()->getMetaData();
        ::rtl::OUString aQuote  = xMetaData->getIdentifierQuoteString(  );

        aSql += ::dbtools::composeTableName(xMetaData,m_pTable,sal_True,::dbtools::eInTableDefinitions);
        aSql += ::rtl::OUString::createFromAscii(" DROP ");
        aSql += ::dbtools::quoteName( aQuote,_sElementName);

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        if ( xStmt.is() )
        {
            xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }
}
// -----------------------------------------------------------------------------



