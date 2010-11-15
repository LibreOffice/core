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
#include "adabas/BTables.hxx"
#include "adabas/BViews.hxx"
#include "adabas/BTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include "adabas/BCatalog.hxx"
#include "adabas/BConnection.hxx"
#include <comphelper/extract.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/types.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace ::cppu;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OTables::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aSchema;
    sal_Int32 nLen = _rName.indexOf('.');
    aSchema = _rName.copy(0,nLen);
    aName   = _rName.copy(nLen+1);

    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%"));

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),aSchema,aName,aTypes);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one table with this name
        {
            OAdabasTable* pRet = new OAdabasTable(this, static_cast<OAdabasCatalog&>(m_rParent).getConnection(),
                                        aName,xRow->getString(4),xRow->getString(5),aSchema);
            xRet = pRet;
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OAdabasCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OTables::disposing(void)
{
m_xMetaData.clear();
    OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OTables::createDescriptor()
{
    return new OAdabasTable(this,static_cast<OAdabasCatalog&>(m_rParent).getConnection());
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OTables::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    createTable(descriptor);
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
void OTables::setComments(const Reference< XPropertySet >& descriptor ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "CREATE TABLE " ));
    ::rtl::OUString aQuote  = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );
    aSql = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("COMMENT ON TABLE "));
    ::rtl::OUString sSchema;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= sSchema;
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + sDot;

    aSql += aQuote + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
            + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" '"))
            + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION)))
            + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));
    xStmt->execute(aSql);

    // columns
    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xColProp;

    aSql = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("COMMENT ON COLUMN "));
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + sDot;
    aSql += aQuote + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote  + sDot
            + aQuote;

    for(sal_Int32 i=0;i<xColumns->getCount();++i)
    {
        ::cppu::extractInterface(xColProp,xColumns->getByIndex(i));
        if(xColProp.is())
        {
            ::rtl::OUString aDescription = getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION)));
            if(aDescription.getLength())
            {
                ::rtl::OUString aCom = aSql + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                            + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" '"))
                                            + aDescription
                                            + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));
                xStmt->execute(aSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }
    }
    ::comphelper::disposeComponent(xStmt);
}
// -------------------------------------------------------------------------
// XDrop
void OTables::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< XInterface > xObject( getObject( _nPos ) );
    sal_Bool bIsNew = connectivity::sdbcx::ODescriptor::isNew( xObject );
    if (!bIsNew)
    {
        OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );

        ::rtl::OUString aName,aSchema;
        sal_Int32 nLen = _sElementName.indexOf('.');
        aSchema = _sElementName.copy(0,nLen);
        aName   = _sElementName.copy(nLen+1);
        ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "DROP " ));
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        Reference<XPropertySet> xProp(xObject,UNO_QUERY);
        sal_Bool bIsView;
        if((bIsView = (xProp.is() && ::comphelper::getString(xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))) == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW"))))) // here we have a view
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW "));
        else
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE "));

        aSql += m_xMetaData->getIdentifierQuoteString(  ) + aSchema + m_xMetaData->getIdentifierQuoteString(  );
        aSql += sDot;
        aSql += m_xMetaData->getIdentifierQuoteString(  ) + aName + m_xMetaData->getIdentifierQuoteString(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
        // if no exception was thrown we must delete it from the views
        if(bIsView)
        {
            OViews* pViews = static_cast<OViews*>(static_cast<OAdabasCatalog&>(m_rParent).getPrivateViews());
            if(pViews && pViews->hasByName(_sElementName))
                pViews->dropByNameImpl(_sElementName);
        }
    }
}
// -------------------------------------------------------------------------
void OTables::createTable( const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "CREATE TABLE " ));
    ::rtl::OUString aQuote  = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();
    ::rtl::OUString sSchema;

    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= sSchema;
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + sDot;
    else
        descriptor->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),makeAny(static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getUserName().
                    toAsciiUpperCase()
                    ));

    aSql += ::dbtools::quoteName(aQuote, getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))))
                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ("));

    // columns
    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xColProp;

    Any aTypeName;
    sal_Int32 nCount = xColumns->getCount();
    if(!nCount)
        ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(this));

    for(sal_Int32 i=0;i<nCount;++i)
    {
        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
        {

            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote;

            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
            aSql += OTables::getColumnSqlType(xColProp);
            aSql += OTables::getColumnSqlNotNullDefault(xColProp);
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        }
    }

    // keys

    Reference<XKeysSupplier> xKeySup(descriptor,UNO_QUERY);

    Reference<XIndexAccess> xKeys = xKeySup->getKeys();
    if(xKeys.is())
    {
        sal_Bool bPKey = sal_False;
        for( sal_Int32 key=0; key<xKeys->getCount(); ++key )
        {
            if(::cppu::extractInterface(xColProp,xKeys->getByIndex(key)) && xColProp.is())
            {

                sal_Int32 nKeyType      = getINT32(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));

                if(nKeyType == KeyType::PRIMARY)
                {
                    if(bPKey)
                        throw SQLException();

                    bPKey = sal_True;
                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PRIMARY KEY ("));
                    for( sal_Int32 column=0; column<xColumns->getCount(); ++column )
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(column)) && xColProp.is())
                            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                        +   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));
                }
                else if(nKeyType == KeyType::UNIQUE)
                {
                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" UNIQUE ("));
                    for( sal_Int32 column=0; column<xColumns->getCount(); ++column )
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(column)) && xColProp.is())
                            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                        + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));
                }
                else if(nKeyType == KeyType::FOREIGN)
                {
                    sal_Int32 nDeleteRule   = getINT32(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELETERULE)));

                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FOREIGN KEY "));
                    ::rtl::OUString aName,aSchema,aRefTable = getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REFERENCEDTABLE)));
                    sal_Int32 nLen = aRefTable.indexOf('.');
                    aSchema = aRefTable.copy(0,nLen);
                    aName   = aRefTable.copy(nLen+1);

                    aSql += aQuote + aSchema + aQuote + sDot
                                + aQuote + aName + aQuote
                                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ("));

                    for ( sal_Int32 column=0; column<xColumns->getCount(); ++column )
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(column)) && xColProp.is())
                            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                        + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));

                    switch(nDeleteRule)
                    {
                        case KeyRule::CASCADE:
                            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE CASCADE "));
                            break;
                        case KeyRule::RESTRICT:
                            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE RESTRICT "));
                            break;
                        case KeyRule::SET_NULL:
                            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE SET NULL "));
                            break;
                        case KeyRule::SET_DEFAULT:
                            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE SET DEFAULT "));
                            break;
                        default:
                            ;
                    }
                }
            }
        }
    }

    if(aSql.lastIndexOf(',') == (aSql.getLength()-1))
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));
    else
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));

    OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    if(getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION))).getLength())
        setComments(descriptor);
}
// -----------------------------------------------------------------------------
void OTables::appendNew(const ::rtl::OUString& _rsNewTable)
{
    insertElement(_rsNewTable,NULL);

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rsNewTable), Any(), Any());
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getColumnSqlType(const Reference<XPropertySet>& _rxColProp)
{
    ::rtl::OUString sSql;
    sal_Int32 nDataType = 0;
    _rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nDataType;
    switch(nDataType)
    {
        case DataType::VARBINARY:
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VAR"));
            /* run through*/
        case DataType::BINARY:
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CHAR"));
            break;
        default:
            {
                Any aTypeName = _rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                if(aTypeName.hasValue() && getString(aTypeName).getLength())
                    sSql += getString(aTypeName);
                else
                    sSql += OTables::getTypeString(_rxColProp) + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
            }
    }

    switch(nDataType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::FLOAT:
        case DataType::REAL:
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("("))
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))))
                        + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
            break;

        case DataType::DECIMAL:
        case DataType::NUMERIC:
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("("))
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))))
                        + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","))
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))))
                        + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("("))
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))))
                        + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(") BYTE"));
            break;
    }
    return sSql;
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getColumnSqlNotNullDefault(const Reference<XPropertySet>& _rxColProp)
{
    OSL_ENSURE(_rxColProp.is(),"OTables::getColumnSqlNotNullDefault: Column is null!");
    ::rtl::OUString sSql;
    ::rtl::OUString aDefault = getString(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE)));
    if(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))) == ColumnValue::NO_NULLS)
    {
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" NOT NULL"));
        if(aDefault.getLength())
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" WITH DEFAULT"));
    }
    else if(aDefault.getLength())
    {
        sSql +=::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DEFAULT '")) + aDefault;
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));
    }
    return sSql;
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getTypeString(const Reference< XPropertySet >& _rxColProp)
{
    ::rtl::OUString aValue;
    switch(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
    {
        case DataType::BIT:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BOOLEAN"));
            break;
        case DataType::TINYINT:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SMALLINT"));
            break;
        case DataType::SMALLINT:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SMALLINT"));
            break;
        case DataType::INTEGER:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INT"));
            break;
        case DataType::FLOAT:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FLOAT"));
            break;
        case DataType::REAL:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REAL"));
            break;
        case DataType::DOUBLE:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOUBLE"));
            break;
        case DataType::NUMERIC:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DECIMAL"));
            break;
        case DataType::DECIMAL:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DECIMAL"));
            break;
        case DataType::CHAR:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CHAR"));
            break;
        case DataType::VARCHAR:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR"));
            break;
        case DataType::LONGVARCHAR:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LONG VARCHAR"));
            break;
        case DataType::DATE:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATE"));
            break;
        case DataType::TIME:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIME"));
            break;
        case DataType::TIMESTAMP:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIMESTAMP"));
            break;
        case DataType::BINARY:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CHAR () BYTE"));
            break;
        case DataType::VARBINARY:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR () BYTE"));
            break;
        case DataType::LONGVARBINARY:
            aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LONG BYTE"));
            break;
    }
    return aValue;
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getNameForObject(const sdbcx::ObjectType& _xObject)
{
    OSL_ENSURE(_xObject.is(),"OTables::getNameForObject: Object is NULL!");
    ::rtl::OUString sName,sTemp;
    _xObject->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= sName;
    if( sName.getLength() )
    {
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();
        sName += sDot;
    }

    _xObject->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sTemp;
    sName += sTemp;

    return sName;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
