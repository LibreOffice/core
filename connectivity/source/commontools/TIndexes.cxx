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

#include <connectivity/TIndexes.hxx>
#include <connectivity/TIndex.hxx>
#include <connectivity/TTableHelper.hxx>
#include <com/sun/star/sdb/tools/XIndexAlteration.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <connectivity/dbtools.hxx>
#include <TConnection.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <rtl/ustrbuf.hxx>
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace cppu;


OIndexesHelper::OIndexesHelper(OTableHelper* _pTable,
                 ::osl::Mutex& _rMutex,
             const std::vector< OUString> &_rVector
             )
    : OCollection(*_pTable,true,_rMutex,_rVector)
    ,m_pTable(_pTable)
{
}


sdbcx::ObjectType OIndexesHelper::createObject(const OUString& _rName)
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if ( !xConnection.is() )
        return nullptr;

    sdbcx::ObjectType xRet;
    OUString aName,aQualifier;
    sal_Int32 nLen = _rName.indexOf('.');
    if ( nLen != -1 )
    {
        aQualifier  = _rName.copy(0,nLen);
        aName       = _rName.copy(nLen+1);
    }
    else
        aName       = _rName;

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    OUString aSchema,aTable;
    m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
    m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

    Any aCatalog = m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME));
    Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(aCatalog,aSchema,aTable,false,false);

    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while( xResult->next() )
        {
            bool bUnique = !xRow->getBoolean(4);
            if((aQualifier.isEmpty() || xRow->getString(5) == aQualifier ) && xRow->getString(6) == aName)
            {
                sal_Int32 nClustered = xRow->getShort(7);
                bool bPrimarKeyIndex = false;
                xRow.clear();
                xResult.clear();
                try
                {
                    xResult = m_pTable->getMetaData()->getPrimaryKeys(aCatalog,aSchema,aTable);
                    xRow.set(xResult,UNO_QUERY);

                    if ( xRow.is() && xResult->next() ) // there can be only one primary key
                    {
                        bPrimarKeyIndex = xRow->getString(6) == aName;
                    }
                }
                catch(const Exception&)
                {
                }
                OIndexHelper* pRet = new OIndexHelper(m_pTable,aName,aQualifier,bUnique,
                    bPrimarKeyIndex,
                    nClustered == IndexType::CLUSTERED);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}

void OIndexesHelper::impl_refresh()
{
    m_pTable->refreshIndexes();
}

Reference< XPropertySet > OIndexesHelper::createDescriptor()
{
    return new OIndexHelper(m_pTable);
}

// XAppend
sdbcx::ObjectType OIndexesHelper::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if ( !xConnection.is() )
        return nullptr;
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    if ( m_pTable->getIndexService().is() )
    {
        m_pTable->getIndexService()->addIndex(m_pTable,descriptor);
    }
    else
    {
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        OUStringBuffer aSql( "CREATE " );
        OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );

        if(comphelper::getBOOL(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISUNIQUE))))
            aSql.append("UNIQUE ");
        aSql.append("INDEX ");


        OUString aCatalog,aSchema,aTable;
        dbtools::qualifiedNameComponents(m_pTable->getMetaData(),m_pTable->getName(),aCatalog,aSchema,aTable,::dbtools::EComposeRule::InDataManipulation);
        OUString aComposedName;

        aComposedName = dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema,aTable, true, ::dbtools::EComposeRule::InIndexDefinitions);
        if (!_rForName.isEmpty() )
        {
            aSql.append( ::dbtools::quoteName( aQuote, _rForName ) );
            aSql.append(" ON ");
            aSql.append(aComposedName);
            aSql.append(" ( ");

            Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
            Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
            Reference< XPropertySet > xColProp;
            bool bAddIndexAppendix = ::dbtools::getBooleanDataSourceSetting( m_pTable->getConnection(), "AddIndexAppendix" );
            sal_Int32 nCount = xColumns->getCount();
            for(sal_Int32 i = 0 ; i < nCount; ++i)
            {
                xColProp.set(xColumns->getByIndex(i),UNO_QUERY);
                aSql.append(::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)))));

                if ( bAddIndexAppendix )
                {

                    aSql.appendAscii(any2bool(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISASCENDING)))
                                                ?
                                    " ASC"
                                                :
                                    " DESC");
                }
                aSql.append(",");
            }
            aSql[aSql.getLength() - 1] = ')';
        }
        else
        {
            aSql.append(aComposedName);

            Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
            Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
            Reference< XPropertySet > xColProp;
            if(xColumns->getCount() != 1)
                throw SQLException();

            xColumns->getByIndex(0) >>= xColProp;

            aSql.append(".");
            aSql.append(::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)))));
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        if ( xStmt.is() )
        {
            OUString sSql = aSql.makeStringAndClear();
            xStmt->execute(sSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }

    return createObject( _rForName );
}

// XDrop
void OIndexesHelper::dropObject(sal_Int32 /*_nPos*/,const OUString& _sElementName)
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if( xConnection.is() && !m_pTable->isNew())
    {
        if ( m_pTable->getIndexService().is() )
        {
            m_pTable->getIndexService()->dropIndex(m_pTable,_sElementName);
        }
        else
        {
            OUString aName,aSchema;
            sal_Int32 nLen = _sElementName.indexOf('.');
            if(nLen != -1)
                aSchema = _sElementName.copy(0,nLen);
            aName   = _sElementName.copy(nLen+1);

            OUString aSql( "DROP INDEX " );

            OUString aComposedName = dbtools::composeTableName( m_pTable->getMetaData(), m_pTable, ::dbtools::EComposeRule::InIndexDefinitions, true );
            OUString sIndexName;
            sIndexName = dbtools::composeTableName( m_pTable->getMetaData(), OUString(), aSchema, aName, true, ::dbtools::EComposeRule::InIndexDefinitions );

            aSql += sIndexName + " ON " + aComposedName;

            Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
            if ( xStmt.is() )
            {
                xStmt->execute(aSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
