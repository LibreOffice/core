/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TColumnsHelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:18:48 $
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
    ,m_pImpl(NULL)
    ,m_pTable(NULL)
{
}
// -----------------------------------------------------------------------------
OColumnsHelper::~OColumnsHelper()
{
    delete m_pImpl;
    m_pImpl = NULL;
}
// -----------------------------------------------------------------------------

sdbcx::ObjectType OColumnsHelper::createObject(const ::rtl::OUString& _rName)
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
        ::rtl::OUString sComposedName = ::dbtools::composeTableNameForSelect( xConnection, m_pTable );
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


    sdbcx::ObjectType xRet(::dbtools::createSDBCXColumn(    m_pTable,
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
    {
        m_pImpl->m_aColumnInfo.clear();
        m_pTable->refreshColumns();
    }
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumnsHelper::createDescriptor()
{
    return new OColumn(sal_True);
}
// -----------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OColumnsHelper::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(m_pTable,"OColumnsHelper::appendByDescriptor: Table is null!");
    if ( !m_pTable || m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    Reference<XDatabaseMetaData> xMetaData = m_pTable->getConnection()->getMetaData();
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
    ::rtl::OUString aQuote  = xMetaData->getIdentifierQuoteString(  );

    aSql += ::dbtools::composeTableName( xMetaData, m_pTable, ::dbtools::eInTableDefinitions, false, false, true );
    aSql += ::rtl::OUString::createFromAscii(" ADD ");
    aSql += ::dbtools::createStandardColumnPart(descriptor,m_pTable->getConnection());

    Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OColumnsHelper::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    OSL_ENSURE(m_pTable,"OColumnsHelper::dropByName: Table is null!");
    if ( m_pTable && !m_pTable->isNew() )
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        Reference<XDatabaseMetaData> xMetaData = m_pTable->getConnection()->getMetaData();
        ::rtl::OUString aQuote  = xMetaData->getIdentifierQuoteString(  );

        aSql += ::dbtools::composeTableName( xMetaData, m_pTable, ::dbtools::eInTableDefinitions, false, false, true );
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



