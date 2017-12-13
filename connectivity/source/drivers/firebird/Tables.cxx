/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Table.hxx"
#include "Tables.hxx"
#include "Catalog.hxx"

#include <TConnection.hxx>

#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;


//----- OCollection -----------------------------------------------------------
void Tables::impl_refresh()
{
    static_cast<Catalog&>(m_rParent).refreshTables();
}

ObjectType Tables::createObject(const OUString& rName)
{
    // Only retrieving a single table, so table type is irrelevant (param 4)
    uno::Reference< XResultSet > xTables = m_xMetaData->getTables(Any(),
                                                                  OUString(),
                                                                  rName,
                                                                  uno::Sequence< OUString >());

    if (!xTables.is())
        throw RuntimeException();

    uno::Reference< XRow > xRow(xTables,UNO_QUERY_THROW);

    if (!xTables->next())
        throw RuntimeException();

    ObjectType xRet(new Table(this,
                              m_rMutex,
                              m_xMetaData->getConnection(),
                              xRow->getString(3), // Name
                              xRow->getString(4), // Type
                              xRow->getString(5))); // Description / Remarks / Comments

    if (xTables->next())
        throw RuntimeException(); // Only one table should be returned

    return xRet;
}

OUString Tables::createStandardColumnPart(const Reference< XPropertySet >& xColProp,const Reference< XConnection>& _xConnection)
{
    Reference<XDatabaseMetaData> xMetaData = _xConnection->getMetaData();

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();

    bool bIsAutoIncrement = false;
    xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))    >>= bIsAutoIncrement;

    const OUString sQuoteString = xMetaData->getIdentifierQuoteString();
    OUStringBuffer aSql = ::dbtools::quoteName(sQuoteString,::comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))));

    // check if the user enter a specific string to create autoincrement values
    OUString sAutoIncrementValue;
    Reference<XPropertySetInfo> xPropInfo = xColProp->getPropertySetInfo();

    if ( xPropInfo.is() && xPropInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION)) )
        xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION)) >>= sAutoIncrementValue;

    aSql.append(" ");

    aSql.append(dbtools::createStandardTypePart(xColProp, _xConnection));

    // Add character set for BINARY (fix) type:
    // BINARY is distinguished from other CHAR types by its character set.
    // Octets is a special character set for binary data.
    if ( xPropInfo.is() && xPropInfo->hasPropertyByName(rPropMap.getNameByIndex(
                    PROPERTY_ID_TYPE)) )
    {
        sal_Int32 aType = 0;
        xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE))
            >>= aType;
        if(aType == DataType::BINARY || aType == DataType::VARBINARY)
        {
            aSql.append(" ");
            aSql.append("CHARACTER SET OCTETS");
        }
        else if(aType == DataType::CLOB)
        {
            // CLOB is a special type of blob in Firebird context.
            // Subtype number 1 always refers to CLOB
            aSql.append(" ");
            aSql.append("SUB_TYPE 1");
        }
    }

    if ( bIsAutoIncrement && !sAutoIncrementValue.isEmpty())
    {
        aSql.append(" ");
        aSql.append(sAutoIncrementValue);
    }
    // AutoIncrement "IDENTITY" is implicitly "NOT NULL"
    else if(::comphelper::getINT32(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISNULLABLE))) == ColumnValue::NO_NULLS)
        aSql.append(" NOT NULL");

    return aSql.makeStringAndClear();
}

uno::Reference< XPropertySet > Tables::createDescriptor()
{
    // There is some internal magic so that the same class can be used as either
    // a descriptor or as a normal table. See VTable.cxx for the details. In our
    // case we just need to ensure we use the correct constructor.
    return new Table(this, m_rMutex, m_xMetaData->getConnection());
}

//----- XAppend ---------------------------------------------------------------
ObjectType Tables::appendObject(const OUString& rName,
                                const uno::Reference< XPropertySet >& rDescriptor)
{
   /* OUString sSql(::dbtools::createSqlCreateTableStatement(rDescriptor,
                                                            m_xMetaData->getConnection())); */
    OUStringBuffer aSqlBuffer("CREATE TABLE ");
    OUString sCatalog, sSchema, sComposedName, sTable;
    const Reference< XConnection>& xConnection = m_xMetaData->getConnection();

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();

    rDescriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))  >>= sCatalog;
    rDescriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))   >>= sSchema;
    rDescriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))         >>= sTable;

    sComposedName = ::dbtools::composeTableName(m_xMetaData, sCatalog, sSchema, sTable, true, ::dbtools::EComposeRule::InTableDefinitions );
    if ( sComposedName.isEmpty() )
        ::dbtools::throwFunctionSequenceException(xConnection);

    aSqlBuffer.append(sComposedName);
    aSqlBuffer.append(" (");

    // columns
    Reference<XColumnsSupplier> xColumnSup(rDescriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    // check if there are columns
    if(!xColumns.is() || !xColumns->getCount())
        ::dbtools::throwFunctionSequenceException(xConnection);

    Reference< XPropertySet > xColProp;

    sal_Int32 nCount = xColumns->getCount();
    for(sal_Int32 i=0;i<nCount;++i)
    {
        if ( (xColumns->getByIndex(i) >>= xColProp) && xColProp.is() )
        {
            aSqlBuffer.append(createStandardColumnPart(xColProp,xConnection));
            aSqlBuffer.append(",");
        }
    }
    OUString sSql = aSqlBuffer.makeStringAndClear();

    const OUString sKeyStmt = ::dbtools::createStandardKeyStatement(rDescriptor,xConnection);
    if ( !sKeyStmt.isEmpty() )
        sSql += sKeyStmt;
    else
    {
        if ( sSql.endsWith(",") )
            sSql = sSql.replaceAt(aSqlBuffer.getLength()-1, 1, ")");
        else
            sSql += ")";
    }

    m_xMetaData->getConnection()->createStatement()->execute(sSql);

    return createObject(rName);
}

//----- XDrop -----------------------------------------------------------------
void Tables::dropObject(sal_Int32 nPosition, const OUString& sName)
{
    uno::Reference< XPropertySet > xTable(getObject(nPosition));

    if (!ODescriptor::isNew(xTable))
    {
        OUStringBuffer sSql("DROP ");

        OUString sType;
        xTable->getPropertyValue("Type") >>= sType;
        sSql.append(sType);

        const OUString sQuoteString = m_xMetaData->getIdentifierQuoteString();
        sSql.append(::dbtools::quoteName(sQuoteString,sName));

        m_xMetaData->getConnection()->createStatement()->execute(sSql.makeStringAndClear());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
