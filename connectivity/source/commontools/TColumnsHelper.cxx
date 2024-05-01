/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <connectivity/TColumnsHelper.hxx>
#include <connectivity/sdbcx/VColumn.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <TConnection.hxx>
#include <connectivity/TTableHelper.hxx>

using namespace ::comphelper;


using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

namespace connectivity
{
    class OColumnsHelperImpl
    {
    public:
        explicit OColumnsHelperImpl(bool _bCase)
            : m_aColumnInfo(_bCase)
        {
        }
        ColumnInformationMap m_aColumnInfo;
    };
}

OColumnsHelper::OColumnsHelper( ::cppu::OWeakObject& _rParent
                                ,bool _bCase
                                ,::osl::Mutex& _rMutex
                                ,const ::std::vector< OUString> &_rVector
                                ,bool _bUseHardRef
            ) : OCollection(_rParent,_bCase,_rMutex,_rVector,false,_bUseHardRef)
    ,m_pTable(nullptr)
{
}

OColumnsHelper::~OColumnsHelper()
{
}

sdbcx::ObjectType OColumnsHelper::createObject(const OUString& _rName)
{
    assert(m_pTable && "NO Table set. Error!");
    Reference<XConnection> xConnection = m_pTable->getConnection();

    if ( !m_pImpl )
        m_pImpl.reset(new OColumnsHelperImpl(isCaseSensitive()));

    bool bQueryInfo     = true;
    bool bAutoIncrement = false;
    bool bIsCurrency    = false;
    sal_Int32 nDataType     = DataType::OTHER;

    ColumnInformationMap::const_iterator aFind = m_pImpl->m_aColumnInfo.find(_rName);
    if ( aFind == m_pImpl->m_aColumnInfo.end() ) // we have to fill it
    {
        OUString sComposedName = ::dbtools::composeTableNameForSelect( xConnection, m_pTable );
        collectColumnInformation(xConnection,sComposedName,u"*" ,m_pImpl->m_aColumnInfo);
        aFind = m_pImpl->m_aColumnInfo.find(_rName);
    }
    if ( aFind != m_pImpl->m_aColumnInfo.end() )
    {
        bQueryInfo      = false;
        bAutoIncrement  = aFind->second.first.first;
        bIsCurrency     = aFind->second.first.second;
        nDataType       = aFind->second.second;
    } // if ( aFind != m_pImpl->m_aColumnInfo.end() )

    sdbcx::ObjectType xRet;
    const ColumnDesc* pColDesc = m_pTable->getColumnDescription(_rName);
    if ( pColDesc )
    {
        Reference<XPropertySet> xPr = m_pTable;
        const Reference<XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(xPr);
        sal_Int32 nField11 = pColDesc->nField11;
        if ( nField11 != ColumnValue::NO_NULLS && xPrimaryKeyColumns.is() && xPrimaryKeyColumns->hasByName(_rName) )
        {
            nField11 = ColumnValue::NO_NULLS;
        } // if ( xKeys.is() )
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        OUString aCatalog, aSchema, aTable;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)) >>= aCatalog;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))  >>= aSchema;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))        >>= aTable;
        rtl::Reference<connectivity::sdbcx::OColumn> pRet = new connectivity::sdbcx::OColumn(_rName,
                                                pColDesc->aField6,
                                                pColDesc->sField13,
                                                pColDesc->sField12,
                                                nField11,
                                                pColDesc->nField7,
                                                pColDesc->nField9,
                                                pColDesc->nField5,
                                                bAutoIncrement,
                                                false,
                                                bIsCurrency,
                                                isCaseSensitive(),
                                                aCatalog,
                                                aSchema,
                                                aTable);

        xRet = pRet;
    }
    else
    {

        xRet = ::dbtools::createSDBCXColumn(  m_pTable,
                                                xConnection,
                                                _rName,
                                                isCaseSensitive(),
                                                bQueryInfo,
                                                bAutoIncrement,
                                                bIsCurrency,
                                                nDataType);
    }
    return xRet;
}

void OColumnsHelper::impl_refresh()
{
    if ( m_pTable )
    {
        m_pImpl->m_aColumnInfo.clear();
        m_pTable->refreshColumns();
    }
}

Reference< XPropertySet > OColumnsHelper::createDescriptor()
{
    return new OColumn(true);
}

// XAppend
sdbcx::ObjectType OColumnsHelper::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(m_pTable,"OColumnsHelper::appendByDescriptor: Table is null!");
    if ( !m_pTable || m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    Reference<XDatabaseMetaData> xMetaData = m_pTable->getConnection()->getMetaData();
    OUString aSql = "ALTER TABLE " +
        ::dbtools::composeTableName( xMetaData, m_pTable, ::dbtools::EComposeRule::InTableDefinitions, true ) +
        " ADD " +
        ::dbtools::createStandardColumnPart(descriptor,m_pTable->getConnection(),nullptr,m_pTable->getTypeCreatePattern());

    Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
    return createObject( _rForName );
}

// XDrop
void OColumnsHelper::dropObject(sal_Int32 /*_nPos*/, const OUString& _sElementName)
{
    OSL_ENSURE(m_pTable,"OColumnsHelper::dropByName: Table is null!");
    if ( !m_pTable || m_pTable->isNew() )
        return;

    Reference<XDatabaseMetaData> xMetaData = m_pTable->getConnection()->getMetaData();
    OUString aQuote  = xMetaData->getIdentifierQuoteString(  );
    OUString aSql = "ALTER TABLE " +
        ::dbtools::composeTableName( xMetaData, m_pTable, ::dbtools::EComposeRule::InTableDefinitions, true ) +
        " DROP " +
        ::dbtools::quoteName( aQuote,_sElementName);

    Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
