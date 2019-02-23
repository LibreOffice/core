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

#include <connectivity/TKeys.hxx>
#include <connectivity/TKey.hxx>
#include <connectivity/TTableHelper.hxx>
#include <com/sun/star/sdb/tools/XKeyAlteration.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <TConnection.hxx>

namespace connectivity
{
using namespace comphelper;
using namespace connectivity::sdbcx;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


OKeysHelper::OKeysHelper(   OTableHelper* _pTable,
        ::osl::Mutex& _rMutex,
        const ::std::vector< OUString>& _rVector
        ) : OKeys_BASE(*_pTable,true,_rMutex,_rVector,true)
    ,m_pTable(_pTable)
{
}

sdbcx::ObjectType OKeysHelper::createObject(const OUString& _rName)
{
    sdbcx::ObjectType xRet;

    if(!_rName.isEmpty())
    {
        OTableKeyHelper* pRet = new OTableKeyHelper(m_pTable,_rName,m_pTable->getKeyProperties(_rName));
        xRet = pRet;
    }

    if(!xRet.is()) // we have a primary key with a system name
    {
        OTableKeyHelper* pRet = new OTableKeyHelper(m_pTable,_rName,m_pTable->getKeyProperties(_rName));
        xRet = pRet;
    }

    return xRet;
}

void OKeysHelper::impl_refresh()
{
    m_pTable->refreshKeys();
}

Reference< XPropertySet > OKeysHelper::createDescriptor()
{
    return new OTableKeyHelper(m_pTable);
}

/** returns the keyrule string for the primary key
*/
static OUString getKeyRuleString(bool _bUpdate,sal_Int32 _nKeyRule)
{
    const char* pKeyRule = nullptr;
    switch ( _nKeyRule )
    {
        case KeyRule::CASCADE:
            pKeyRule = _bUpdate ? " ON UPDATE CASCADE " : " ON DELETE CASCADE ";
            break;
        case KeyRule::RESTRICT:
            pKeyRule = _bUpdate ? " ON UPDATE RESTRICT " : " ON DELETE RESTRICT ";
            break;
        case KeyRule::SET_NULL:
            pKeyRule = _bUpdate ? " ON UPDATE SET NULL " : " ON DELETE SET NULL ";
            break;
        case KeyRule::SET_DEFAULT:
            pKeyRule = _bUpdate ? " ON UPDATE SET DEFAULT " : " ON DELETE SET DEFAULT ";
            break;
        default:
            ;
    }
    OUString sRet;
    if ( pKeyRule )
        sRet = OUString::createFromAscii(pKeyRule);
    return sRet;
}

void OKeysHelper::cloneDescriptorColumns( const sdbcx::ObjectType& _rSourceDescriptor, const sdbcx::ObjectType& _rDestDescriptor )
{
    Reference< XColumnsSupplier > xColSupp( _rSourceDescriptor, UNO_QUERY_THROW );
    Reference< XIndexAccess > xSourceCols( xColSupp->getColumns(), UNO_QUERY_THROW );

    xColSupp.set( _rDestDescriptor, UNO_QUERY_THROW );
    Reference< XAppend > xDestAppend( xColSupp->getColumns(), UNO_QUERY_THROW );

    sal_Int32 nCount = xSourceCols->getCount();
    for ( sal_Int32 i=0; i< nCount; ++i )
    {
        Reference< XPropertySet > xColProp( xSourceCols->getByIndex(i), UNO_QUERY );
        xDestAppend->appendByDescriptor( xColProp );
    }
}

// XAppend
sdbcx::ObjectType OKeysHelper::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if ( !xConnection.is() )
        return nullptr;
    if ( m_pTable->isNew() )
    {
        Reference< XPropertySet > xNewDescriptor( cloneDescriptor( descriptor ) );
        cloneDescriptorColumns( descriptor, xNewDescriptor );
        return xNewDescriptor;
    }

    const ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    sal_Int32 nKeyType      = getINT32(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)));
    sal_Int32 nUpdateRule = 0, nDeleteRule = 0;
    OUString sReferencedName;

    if ( nKeyType == KeyType::FOREIGN )
    {
        descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_REFERENCEDTABLE)) >>= sReferencedName;
        descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_UPDATERULE)) >>= nUpdateRule;
        descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_DELETERULE)) >>= nDeleteRule;
    }

    if ( m_pTable->getKeyService().is() )
    {
        m_pTable->getKeyService()->addKey(m_pTable,descriptor);
    }
    else
    {
        // if we're here, we belong to a table which is not new, i.e. already exists in the database.
        // In this case, really append the new index.
        OUStringBuffer aSql;
        aSql.append("ALTER TABLE ");
        OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );

        aSql.append(composeTableName( m_pTable->getConnection()->getMetaData(), m_pTable, ::dbtools::EComposeRule::InTableDefinitions, true ));
        aSql.append(" ADD ");

        if ( nKeyType == KeyType::PRIMARY )
        {
            aSql.append(" PRIMARY KEY (");
        }
        else if ( nKeyType == KeyType::FOREIGN )
        {
            aSql.append(" FOREIGN KEY (");
        }
        else
            throw SQLException();

        Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
        Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
        Reference< XPropertySet > xColProp;
        for(sal_Int32 i = 0 ; i < xColumns->getCount() ; ++i)
        {
            if ( i > 0 )
                aSql.append(",");
            xColProp.set(xColumns->getByIndex(i), css::uno::UNO_QUERY);
            aSql.append( ::dbtools::quoteName( aQuote,getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)))) );

        }
        aSql.append(")");

        if ( nKeyType == KeyType::FOREIGN )
        {
            aSql.append(" REFERENCES ");
            aSql.append(::dbtools::quoteTableName(m_pTable->getConnection()->getMetaData(),sReferencedName,::dbtools::EComposeRule::InTableDefinitions));
            aSql.append(" (");

            for(sal_Int32 i=0;i<xColumns->getCount();++i)
            {
                if ( i > 0 )
                    aSql.append(",");
                xColumns->getByIndex(i) >>= xColProp;
                aSql.append(::dbtools::quoteName( aQuote,getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_RELATEDCOLUMN)))));

            }
            aSql.append(")");
            aSql.append(getKeyRuleString(true   ,nUpdateRule));
            aSql.append(getKeyRuleString(false  ,nDeleteRule));
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql.makeStringAndClear());
    }
    // find the name which the database gave the new key
    OUString sNewName( _rForName );
    try
    {
        OUString aSchema,aTable;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;
        Reference< XResultSet > xResult;
        sal_Int32 nColumn = 12;
        if ( nKeyType == KeyType::FOREIGN )
            xResult = m_pTable->getMetaData()->getImportedKeys( m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))
                                                                                    ,aSchema
                                                                                    ,aTable);
        else
        {
            xResult = m_pTable->getMetaData()->getPrimaryKeys( m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))
                                                                                    ,aSchema
                                                                                    ,aTable);
            nColumn = 6;
        }
        if ( xResult.is() )
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            while( xResult->next() )
            {
                OUString sName = xRow->getString(nColumn);
                if ( !m_pElements->exists(sName) ) // this name wasn't inserted yet so it must be the new one
                {
                    descriptor->setPropertyValue( rPropMap.getNameByIndex( PROPERTY_ID_NAME ), makeAny( sName ) );
                    sNewName = sName;
                    break;
                }
            }
            ::comphelper::disposeComponent(xResult);
        }
    }
    catch(const SQLException&)
    {
    }

    m_pTable->addKey(sNewName,std::make_shared<sdbcx::KeyProperties>(sReferencedName,nKeyType,nUpdateRule,nDeleteRule));

    return createObject( sNewName );
}

OUString OKeysHelper::getDropForeignKey() const
{
    return OUString(" DROP CONSTRAINT ");
}

// XDrop
void OKeysHelper::dropObject(sal_Int32 _nPos, const OUString& _sElementName)
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if ( xConnection.is() && !m_pTable->isNew() )
    {
        Reference<XPropertySet> xKey(getObject(_nPos),UNO_QUERY);
        if ( m_pTable->getKeyService().is() )
        {
            m_pTable->getKeyService()->dropKey(m_pTable,xKey);
        }
        else
        {
            OUStringBuffer aSql;
            aSql.append("ALTER TABLE ");

            aSql.append( composeTableName( m_pTable->getConnection()->getMetaData(), m_pTable,::dbtools::EComposeRule::InTableDefinitions, true ));

            sal_Int32 nKeyType = KeyType::PRIMARY;
            if ( xKey.is() )
            {
                ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
                xKey->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)) >>= nKeyType;
            }
            if ( KeyType::PRIMARY == nKeyType )
            {
                aSql.append(" DROP PRIMARY KEY");
            }
            else
            {
                aSql.append(getDropForeignKey());
                const OUString aQuote    = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString();
                aSql.append( ::dbtools::quoteName( aQuote,_sElementName) );
            }

            Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
            if ( xStmt.is() )
            {
                xStmt->execute(aSql.makeStringAndClear());
                ::comphelper::disposeComponent(xStmt);
            }
        }
    }
}

} // namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
