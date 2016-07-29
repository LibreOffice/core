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

#include <string.h>
#include "hsqldb/HTable.hxx"
#include "hsqldb/HTables.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <comphelper/property.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sdbcx/VColumn.hxx>
#include <connectivity/TKeys.hxx>
#include <connectivity/TIndexes.hxx>
#include <connectivity/TColumnsHelper.hxx>
#include "hsqldb/HCatalog.hxx"
#include "hsqldb/HColumns.hxx"
#include "TConnection.hxx"

#include <tools/diagnose_ex.h>


using namespace ::comphelper;
using namespace connectivity::hsqldb;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OHSQLTable::OHSQLTable( sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection)
    :OTableHelper(_pTables,_xConnection,true)
{
    // we create a new table here, so we should have all the rights or ;-)
    m_nPrivileges = Privilege::DROP         |
                    Privilege::REFERENCE    |
                    Privilege::ALTER        |
                    Privilege::CREATE       |
                    Privilege::READ         |
                    Privilege::DELETE       |
                    Privilege::UPDATE       |
                    Privilege::INSERT       |
                    Privilege::SELECT;
    construct();
}

OHSQLTable::OHSQLTable( sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection,
                    const OUString& Name,
                    const OUString& Type,
                    const OUString& Description ,
                    const OUString& SchemaName,
                    const OUString& CatalogName,
                    sal_Int32 _nPrivileges
                ) : OTableHelper(   _pTables,
                                    _xConnection,
                                    true,
                                    Name,
                                    Type,
                                    Description,
                                    SchemaName,
                                    CatalogName)
 , m_nPrivileges(_nPrivileges)
{
    construct();
}

void OHSQLTable::construct()
{
    OTableHelper::construct();
    if ( !isNew() )
        registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRIVILEGES),  PROPERTY_ID_PRIVILEGES,PropertyAttribute::READONLY,&m_nPrivileges,  cppu::UnoType<decltype(m_nPrivileges)>::get());
}

::cppu::IPropertyArrayHelper* OHSQLTable::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper & OHSQLTable::getInfoHelper()
{
    return *static_cast<OHSQLTable_PROP*>(this)->getArrayHelper(isNew() ? 1 : 0);
}

sdbcx::OCollection* OHSQLTable::createColumns(const TStringVector& _rNames)
{
    OHSQLColumns* pColumns = new OHSQLColumns(*this,m_aMutex,_rNames);
    pColumns->setParent(this);
    return pColumns;
}

sdbcx::OCollection* OHSQLTable::createKeys(const TStringVector& _rNames)
{
    return new OKeysHelper(this,m_aMutex,_rNames);
}

sdbcx::OCollection* OHSQLTable::createIndexes(const TStringVector& _rNames)
{
    return new OIndexesHelper(this,m_aMutex,_rNames);
}

Sequence< sal_Int8 > OHSQLTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// css::lang::XUnoTunnel

sal_Int64 OHSQLTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException, std::exception)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OTable_TYPEDEF::getSomething(rId);
}

// XAlterTable
void SAL_CALL OHSQLTable::alterColumnByName( const OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef __GNUC__
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    if ( !m_pColumns || !m_pColumns->hasByName(colName) )
        throw NoSuchElementException(colName,*this);


    if ( !isNew() )
    {
        // first we have to check what should be altered
        Reference<XPropertySet> xProp;
        m_pColumns->getByName(colName) >>= xProp;
        // first check the types
        sal_Int32 nOldType = 0,nNewType = 0,nOldPrec = 0,nNewPrec = 0,nOldScale = 0,nNewScale = 0;
        OUString sOldTypeName, sNewTypeName;

        ::dbtools::OPropertyMap& rProp = OMetaConnection::getPropMap();

        // type/typename
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPE))         >>= nOldType;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPE))    >>= nNewType;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPENAME))     >>= sOldTypeName;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPENAME))>>= sNewTypeName;

        // and precsions and scale
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_PRECISION))    >>= nOldPrec;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_PRECISION))>>= nNewPrec;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_SCALE))        >>= nOldScale;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_SCALE))   >>= nNewScale;

        // second: check the "is nullable" value
        sal_Int32 nOldNullable = 0,nNewNullable = 0;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISNULLABLE))       >>= nOldNullable;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISNULLABLE))  >>= nNewNullable;

        // check also the auto_increment
        bool bOldAutoIncrement = false,bAutoIncrement = false;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))      >>= bOldAutoIncrement;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;

        // now we should look if the name of the column changed
        OUString sNewColumnName;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_NAME)) >>= sNewColumnName;
        if ( !sNewColumnName.equals(colName) )
        {
            const OUString sQuote = getMetaData()->getIdentifierQuoteString(  );

            OUString sSql = getAlterTableColumnPart() +
                " ALTER COLUMN " +
                ::dbtools::quoteName(sQuote,colName) +
                " RENAME TO " +
                ::dbtools::quoteName(sQuote,sNewColumnName);

            executeStatement(sSql);
        }

        if  (   nOldType != nNewType
            ||  sOldTypeName != sNewTypeName
            ||  nOldPrec != nNewPrec
            ||  nOldScale != nNewScale
            ||  nNewNullable != nOldNullable
            ||  bOldAutoIncrement != bAutoIncrement )
        {
            // special handling because they change the type names to distinguish
            // if a column should be an auto_incmrement one
            if ( bOldAutoIncrement != bAutoIncrement )
            {
                /// TODO: insert special handling for auto increment "IDENTITY" and primary key
            }
            alterColumnType(nNewType,sNewColumnName,descriptor);
        }

        // third: check the default values
        OUString sNewDefault,sOldDefault;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DEFAULTVALUE))     >>= sOldDefault;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DEFAULTVALUE)) >>= sNewDefault;

        if(!sOldDefault.isEmpty())
        {
            dropDefaultValue(colName);
            if(!sNewDefault.isEmpty() && sOldDefault != sNewDefault)
                alterDefaultValue(sNewDefault,sNewColumnName);
        }
        else if(sOldDefault.isEmpty() && !sNewDefault.isEmpty())
            alterDefaultValue(sNewDefault,sNewColumnName);

        m_pColumns->refresh();
    }
    else
    {
        if(m_pColumns)
        {
            m_pColumns->dropByName(colName);
            m_pColumns->appendByDescriptor(descriptor);
        }
    }

}

void OHSQLTable::alterColumnType(sal_Int32 nNewType,const OUString& _rColName, const Reference<XPropertySet>& _xDescriptor)
{
    OUString sSql = getAlterTableColumnPart() + " ALTER COLUMN ";
#if OSL_DEBUG_LEVEL > 0
    try
    {
        OUString sDescriptorName;
        OSL_ENSURE( _xDescriptor.is()
                &&  ( _xDescriptor->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_NAME ) ) >>= sDescriptorName )
                &&  ( sDescriptorName == _rColName ),
                "OHSQLTable::alterColumnType: unexpected column name!" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
#else
    (void)_rColName;
#endif

    OHSQLColumn* pColumn = new OHSQLColumn;
    Reference<XPropertySet> xProp = pColumn;
    ::comphelper::copyProperties(_xDescriptor,xProp);
    xProp->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),makeAny(nNewType));

    sSql += ::dbtools::createStandardColumnPart(xProp,getConnection());
    executeStatement(sSql);
}

void OHSQLTable::alterDefaultValue(const OUString& _sNewDefault,const OUString& _rColName)
{
    const OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    OUString sSql = getAlterTableColumnPart() +
        " ALTER COLUMN " +
        ::dbtools::quoteName(sQuote,_rColName) +
        " SET DEFAULT '" + _sNewDefault + "'";

    executeStatement(sSql);
}

void OHSQLTable::dropDefaultValue(const OUString& _rColName)
{
    const OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    OUString sSql = getAlterTableColumnPart() +
        " ALTER COLUMN " +
        ::dbtools::quoteName(sQuote,_rColName) +
        " DROP DEFAULT";

    executeStatement(sSql);
}

OUString OHSQLTable::getAlterTableColumnPart()
{
    OUString sSql(  "ALTER TABLE " );

    OUString sComposedName( ::dbtools::composeTableName( getMetaData(), m_CatalogName, m_SchemaName, m_Name, true, ::dbtools::EComposeRule::InTableDefinitions ) );
    sSql += sComposedName;

    return sSql;
}

void OHSQLTable::executeStatement(const OUString& _rStatement )
{
    OUString sSQL = _rStatement;
    if(sSQL.endsWith(","))
        sSQL = sSQL.replaceAt(sSQL.getLength()-1, 1, ")");

    Reference< XStatement > xStmt = getConnection()->createStatement(  );
    if ( xStmt.is() )
    {
        try { xStmt->execute(sSQL); }
        catch( const Exception& )
        {
            ::comphelper::disposeComponent(xStmt);
            throw;
        }
        ::comphelper::disposeComponent(xStmt);
    }
}

Sequence< Type > SAL_CALL OHSQLTable::getTypes(  ) throw(RuntimeException, std::exception)
{
    if ( m_Type == "VIEW" )
    {
        Sequence< Type > aTypes = OTableHelper::getTypes();
        ::std::vector<Type> aOwnTypes;
        aOwnTypes.reserve(aTypes.getLength());
        const Type* pIter = aTypes.getConstArray();
        const Type* pEnd = pIter + aTypes.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if( *pIter != cppu::UnoType<XRename>::get())
            {
                aOwnTypes.push_back(*pIter);
            }
        }
        return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
    }
    return OTableHelper::getTypes();
}

// XRename
void SAL_CALL OHSQLTable::rename( const OUString& newName ) throw(SQLException, ElementExistException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef __GNUC__
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    if(!isNew())
    {
        OUString sSql = "ALTER ";
        if ( m_Type == "VIEW" )
            sSql += " VIEW ";
        else
            sSql += " TABLE ";

        OUString sCatalog,sSchema,sTable;
        ::dbtools::qualifiedNameComponents(getMetaData(),newName,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);

        sSql +=
            ::dbtools::composeTableName( getMetaData(), m_CatalogName, m_SchemaName, m_Name, true, ::dbtools::EComposeRule::InDataManipulation )
            + " RENAME TO "
            + ::dbtools::composeTableName( getMetaData(), sCatalog, sSchema, sTable, true, ::dbtools::EComposeRule::InDataManipulation );

        executeStatement(sSql);

        ::connectivity::OTable_TYPEDEF::rename(newName);
    }
    else
        ::dbtools::qualifiedNameComponents(getMetaData(),newName,m_CatalogName,m_SchemaName,m_Name,::dbtools::EComposeRule::InTableDefinitions);
}


Any SAL_CALL OHSQLTable::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    if( m_Type == "VIEW" && rType == cppu::UnoType<XRename>::get())
        return Any();

    return OTableHelper::queryInterface(rType);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
