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
#include "mysql/YTable.hxx"
#include "mysql/YTables.hxx"
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
#include "connectivity/dbtools.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/TKeys.hxx"
#include "connectivity/TIndexes.hxx"
#include "connectivity/TColumnsHelper.hxx"
#include "mysql/YCatalog.hxx"
#include "mysql/YColumns.hxx"
#include "TConnection.hxx"


using namespace ::comphelper;
using namespace connectivity::mysql;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
namespace connectivity
{
    namespace mysql
    {
        class OMySQLKeysHelper : public OKeysHelper
        {
        protected:
            // -----------------------------------------------------------------------------
            virtual ::rtl::OUString getDropForeignKey() const
            {
                return ::rtl::OUString(" DROP FOREIGN KEY ");
            }
        public:
            OMySQLKeysHelper(   OTableHelper* _pTable,
                ::osl::Mutex& _rMutex,
                const TStringVector& _rVector
                ) : OKeysHelper(_pTable,_rMutex,_rVector){}

        };
    }
}

OMySQLTable::OMySQLTable(   sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection)
    :OTableHelper(_pTables,_xConnection,sal_True)
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
// -------------------------------------------------------------------------
OMySQLTable::OMySQLTable(   sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName,
                    sal_Int32 _nPrivileges
                ) : OTableHelper(   _pTables,
                                    _xConnection,
                                    sal_True,
                                    _Name,
                                    _Type,
                                    _Description,
                                    _SchemaName,
                                    _CatalogName)
 , m_nPrivileges(_nPrivileges)
{
    construct();
}
// -------------------------------------------------------------------------
void OMySQLTable::construct()
{
    OTableHelper::construct();
    if ( !isNew() )
        registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRIVILEGES),  PROPERTY_ID_PRIVILEGES,PropertyAttribute::READONLY,&m_nPrivileges,  ::getCppuType(&m_nPrivileges));
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OMySQLTable::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OMySQLTable::getInfoHelper()
{
    return *static_cast<OMySQLTable_PROP*>(const_cast<OMySQLTable*>(this))->getArrayHelper(isNew() ? 1 : 0);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OMySQLTable::createColumns(const TStringVector& _rNames)
{
    OMySQLColumns* pColumns = new OMySQLColumns(*this,sal_True,m_aMutex,_rNames);
    pColumns->setParent(this);
    return pColumns;
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OMySQLTable::createKeys(const TStringVector& _rNames)
{
    return new OMySQLKeysHelper(this,m_aMutex,_rNames);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OMySQLTable::createIndexes(const TStringVector& _rNames)
{
    return new OIndexesHelper(this,m_aMutex,_rNames);
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OMySQLTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
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

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OMySQLTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OTable_TYPEDEF::getSomething(rId);
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OMySQLTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef GCC
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    if ( m_pColumns && !m_pColumns->hasByName(colName) )
        throw NoSuchElementException(colName,*this);


    if ( !isNew() )
    {
        // first we have to check what should be altered
        Reference<XPropertySet> xProp;
        m_pColumns->getByName(colName) >>= xProp;
        // first check the types
        sal_Int32 nOldType = 0,nNewType = 0,nOldPrec = 0,nNewPrec = 0,nOldScale = 0,nNewScale = 0;

        ::dbtools::OPropertyMap& rProp = OMetaConnection::getPropMap();
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPE))         >>= nOldType;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPE))    >>= nNewType;
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
        sal_Bool bOldAutoIncrement = sal_False,bAutoIncrement = sal_False;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))      >>= bOldAutoIncrement;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;
        bool bColumnNameChanged = false;
        ::rtl::OUString sOldDesc,sNewDesc;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DESCRIPTION))      >>= sOldDesc;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DESCRIPTION)) >>= sNewDesc;

        if (    nOldType != nNewType
            ||  nOldPrec != nNewPrec
            ||  nOldScale != nNewScale
            ||  nNewNullable != nOldNullable
            ||  bOldAutoIncrement != bAutoIncrement
            || sOldDesc != sNewDesc )
        {
            // special handling because they change dthe type names to distinguish
            // if a column should be an auto_incmrement one
            if ( bOldAutoIncrement != bAutoIncrement )
            {
                ::rtl::OUString sTypeName;
                descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPENAME)) >>= sTypeName;

                static ::rtl::OUString s_sAutoIncrement("auto_increment");
                if ( bAutoIncrement )
                {
                    if ( sTypeName.indexOf(s_sAutoIncrement) == -1 )
                    {
                        sTypeName += ::rtl::OUString(" ");
                        sTypeName += s_sAutoIncrement;
                    }
                }
                else
                {
                    sal_Int32 nIndex = 0;
                    if ( !sTypeName.isEmpty() && (nIndex = sTypeName.indexOf(s_sAutoIncrement)) != -1 )
                    {
                        sTypeName = sTypeName.copy(0,nIndex);
                        descriptor->setPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPENAME),makeAny(sTypeName));
                    }
                }
            }
            alterColumnType(nNewType,colName,descriptor);
            bColumnNameChanged = true;
        }

        // third: check the default values
        ::rtl::OUString sNewDefault,sOldDefault;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DEFAULTVALUE))     >>= sOldDefault;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DEFAULTVALUE)) >>= sNewDefault;

        if(!sOldDefault.isEmpty())
        {
            dropDefaultValue(colName);
            if(!sNewDefault.isEmpty() && sOldDefault != sNewDefault)
                alterDefaultValue(sNewDefault,colName);
        }
        else if(!sNewDefault.isEmpty())
            alterDefaultValue(sNewDefault,colName);

        // now we should look if the name of the column changed
        ::rtl::OUString sNewColumnName;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_NAME)) >>= sNewColumnName;
        if ( !sNewColumnName.equalsIgnoreAsciiCase(colName) && !bColumnNameChanged )
        {
            ::rtl::OUString sSql = getAlterTableColumnPart();
            sSql += ::rtl::OUString(" CHANGE ");
            const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
            sSql += ::dbtools::quoteName(sQuote,colName);
            sSql += ::rtl::OUString(" ");
            sSql += OTables::adjustSQL(::dbtools::createStandardColumnPart(descriptor,getConnection(),static_cast<OTables*>(m_pTables),getTypeCreatePattern()));
            executeStatement(sSql);
        }
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
// -----------------------------------------------------------------------------
void OMySQLTable::alterColumnType(sal_Int32 nNewType,const ::rtl::OUString& _rColName, const Reference<XPropertySet>& _xDescriptor)
{
    ::rtl::OUString sSql = getAlterTableColumnPart();
    sSql += ::rtl::OUString(" CHANGE ");
    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    sSql += ::dbtools::quoteName(sQuote,_rColName);
    sSql += ::rtl::OUString(" ");

    OColumn* pColumn = new OColumn(sal_True);
    Reference<XPropertySet> xProp = pColumn;
    ::comphelper::copyProperties(_xDescriptor,xProp);
    xProp->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),makeAny(nNewType));

    sSql += OTables::adjustSQL(::dbtools::createStandardColumnPart(xProp,getConnection(),static_cast<OTables*>(m_pTables),getTypeCreatePattern()));
    executeStatement(sSql);
}
// -----------------------------------------------------------------------------
::rtl::OUString OMySQLTable::getTypeCreatePattern() const
{
    static const ::rtl::OUString s_sCreatePattern("(M,D)");
    return s_sCreatePattern;
}
// -----------------------------------------------------------------------------
void OMySQLTable::alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart();
    sSql += ::rtl::OUString(" ALTER ");

    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    sSql += ::dbtools::quoteName(sQuote,_rColName);
    sSql += ::rtl::OUString(" SET DEFAULT '") + _sNewDefault;
    sSql += ::rtl::OUString("'");

    executeStatement(sSql);
}
// -----------------------------------------------------------------------------
void OMySQLTable::dropDefaultValue(const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart();
    sSql += ::rtl::OUString(" ALTER ");

    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    sSql += ::dbtools::quoteName(sQuote,_rColName);
    sSql += ::rtl::OUString(" DROP DEFAULT");

    executeStatement(sSql);
}
// -----------------------------------------------------------------------------
::rtl::OUString OMySQLTable::getAlterTableColumnPart()
{
    ::rtl::OUString sSql(  "ALTER TABLE " );
    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );

    ::rtl::OUString sComposedName(
        ::dbtools::composeTableName( getMetaData(), m_CatalogName, m_SchemaName, m_Name, sal_True, ::dbtools::eInTableDefinitions ) );
    sSql += sComposedName;

    return sSql;
}
// -----------------------------------------------------------------------------
void OMySQLTable::executeStatement(const ::rtl::OUString& _rStatement )
{
    ::rtl::OUString sSQL = _rStatement;
    if(sSQL.lastIndexOf(',') == (sSQL.getLength()-1))
        sSQL = sSQL.replaceAt(sSQL.getLength()-1,1,::rtl::OUString(")"));

    Reference< XStatement > xStmt = getConnection()->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(sSQL);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString OMySQLTable::getRenameStart() const
{
    return ::rtl::OUString("RENAME TABLE ");
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
