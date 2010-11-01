/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "connectivity/TKeys.hxx"
#include "connectivity/TKey.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include "connectivity/dbtools.hxx"
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include "TConnection.hxx"

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
        const TStringVector& _rVector
        ) : OKeys_BASE(*_pTable,sal_True,_rMutex,_rVector,sal_True)
    ,m_pTable(_pTable)
{
}
// -------------------------------------------------------------------------
sdbcx::ObjectType OKeysHelper::createObject(const ::rtl::OUString& _rName)
{
    sdbcx::ObjectType xRet = NULL;

    if(_rName.getLength())
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
// -------------------------------------------------------------------------
void OKeysHelper::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshKeys();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeysHelper::createDescriptor()
{
    return new OTableKeyHelper(m_pTable);
}
// -----------------------------------------------------------------------------
/** returns the keyrule string for the primary key
*/
::rtl::OUString getKeyRuleString(sal_Bool _bUpdate,sal_Int32 _nKeyRule)
{
    const char* pKeyRule = NULL;
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
    ::rtl::OUString sRet;
    if ( pKeyRule )
        sRet = ::rtl::OUString::createFromAscii(pKeyRule);
    return sRet;
}
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OKeysHelper::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if ( !xConnection.is() )
        return NULL;
    if ( m_pTable->isNew() )
    {
        Reference< XPropertySet > xNewDescriptor( cloneDescriptor( descriptor ) );
        cloneDescriptorColumns( descriptor, xNewDescriptor );
        return xNewDescriptor;
    }

    const ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    sal_Int32 nKeyType      = getINT32(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)));
    sal_Int32 nUpdateRule = 0, nDeleteRule = 0;
    ::rtl::OUString sReferencedName;

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
        ::rtl::OUStringBuffer aSql;
        aSql.appendAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));

        aSql.append(composeTableName( m_pTable->getConnection()->getMetaData(), m_pTable, ::dbtools::eInTableDefinitions, false, false, true ));
        aSql.appendAscii(" ADD ");

        if ( nKeyType == KeyType::PRIMARY )
        {
            aSql.appendAscii(" PRIMARY KEY (");
        }
        else if ( nKeyType == KeyType::FOREIGN )
        {
            aSql.appendAscii(" FOREIGN KEY (");
        }
        else
            throw SQLException();

        Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
        Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
        Reference< XPropertySet > xColProp;
        for(sal_Int32 i = 0 ; i < xColumns->getCount() ; ++i)
        {
            if ( i > 0 )
                aSql.appendAscii(",");
            ::cppu::extractInterface(xColProp,xColumns->getByIndex(i));
            aSql.append( ::dbtools::quoteName( aQuote,getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)))) );

        }
        aSql.appendAscii(")");

        if ( nKeyType == KeyType::FOREIGN )
        {
            aSql.appendAscii(" REFERENCES ");
            aSql.append(::dbtools::quoteTableName(m_pTable->getConnection()->getMetaData(),sReferencedName,::dbtools::eInTableDefinitions));
            aSql.appendAscii(" (");

            for(sal_Int32 i=0;i<xColumns->getCount();++i)
            {
                if ( i > 0 )
                    aSql.appendAscii(",");
                xColumns->getByIndex(i) >>= xColProp;
                aSql.append(::dbtools::quoteName( aQuote,getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_RELATEDCOLUMN)))));

            }
            aSql.appendAscii(")");
            aSql.append(getKeyRuleString(sal_True   ,nUpdateRule));
            aSql.append(getKeyRuleString(sal_False  ,nDeleteRule));
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql.makeStringAndClear());
    }
    // find the name which the database gave the new key
    ::rtl::OUString sNewName( _rForName );
    try
    {
        ::rtl::OUString aSchema,aTable;
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
                ::rtl::OUString sName = xRow->getString(nColumn);
                if ( !m_pElements->exists(sName) ) // this name wasn't inserted yet so it must be te new one
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

    m_pTable->addKey(sNewName,sdbcx::TKeyProperties(new sdbcx::KeyProperties(sReferencedName,nKeyType,nUpdateRule,nDeleteRule)));

    return createObject( sNewName );
}
// -----------------------------------------------------------------------------
::rtl::OUString OKeysHelper::getDropForeignKey() const
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP CONSTRAINT "));
}
// -------------------------------------------------------------------------
// XDrop
void OKeysHelper::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
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
            ::rtl::OUStringBuffer aSql;
            aSql.appendAscii("ALTER TABLE ");

            aSql.append( composeTableName( m_pTable->getConnection()->getMetaData(), m_pTable,::dbtools::eInTableDefinitions, false, false, true ));

            sal_Int32 nKeyType = KeyType::PRIMARY;
            if ( xKey.is() )
            {
                ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
                xKey->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)) >>= nKeyType;
            }
            if ( KeyType::PRIMARY == nKeyType )
            {
                aSql.appendAscii(" DROP PRIMARY KEY");
            }
            else
            {
                aSql.append(getDropForeignKey());
                const ::rtl::OUString aQuote    = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString();
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
// -----------------------------------------------------------------------------
} // namespace connectivity
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
