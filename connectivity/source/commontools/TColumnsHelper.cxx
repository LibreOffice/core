/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TColumnsHelper.cxx,v $
 * $Revision: 1.10 $
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
#include "connectivity/TColumnsHelper.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/types.hxx>
#include "connectivity/dbtools.hxx"
#include "TConnection.hxx"
#include "connectivity/TTableHelper.hxx"
#include <comphelper/property.hxx>

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



