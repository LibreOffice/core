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

#include <mysql/YTables.hxx>
#include <mysql/YViews.hxx>
#include <mysql/YTable.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <mysql/YCatalog.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <TConnection.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace ::cppu;
using namespace connectivity::mysql;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace dbtools;

sdbcx::ObjectType OTables::createObject(const OUString& _rName)
{
    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData, _rName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);

    Sequence<OUString> sTableTypes{
        "VIEW", "TABLE", "%"
    }; // this last one just to be sure to include anything else...

    Any aCatalog;
    if (!sCatalog.isEmpty())
        aCatalog <<= sCatalog;
    Reference<XResultSet> xResult = m_xMetaData->getTables(aCatalog, sSchema, sTable, sTableTypes);

    sdbcx::ObjectType xRet;
    if (xResult.is())
    {
        Reference<XRow> xRow(xResult, UNO_QUERY);
        if (xResult->next()) // there can be only one table with this name
        {
            sal_Int32 const nPrivileges = Privilege::DROP | Privilege::REFERENCE | Privilege::ALTER
                                          | Privilege::CREATE | Privilege::READ | Privilege::DELETE
                                          | Privilege::UPDATE | Privilege::INSERT
                                          | Privilege::SELECT;

            xRet = new OMySQLTable(this, static_cast<OMySQLCatalog&>(m_rParent).getConnection(),
                                   sTable, xRow->getString(4), xRow->getString(5), sSchema,
                                   sCatalog, nPrivileges);
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}

void OTables::impl_refresh() { static_cast<OMySQLCatalog&>(m_rParent).refreshTables(); }

void OTables::disposing()
{
    m_xMetaData.clear();
    OCollection::disposing();
}

Reference<XPropertySet> OTables::createDescriptor()
{
    return new OMySQLTable(this, static_cast<OMySQLCatalog&>(m_rParent).getConnection());
}

// XAppend
sdbcx::ObjectType OTables::appendObject(const OUString& _rForName,
                                        const Reference<XPropertySet>& descriptor)
{
    createTable(descriptor);
    return createObject(_rForName);
}

// XDrop
void OTables::dropObject(sal_Int32 _nPos, const OUString& _sElementName)
{
    Reference<XInterface> xObject(getObject(_nPos));
    bool bIsNew = connectivity::sdbcx::ODescriptor::isNew(xObject);
    if (bIsNew)
        return;

    Reference<XConnection> xConnection = static_cast<OMySQLCatalog&>(m_rParent).getConnection();

    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData, _sElementName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);

    OUString aSql("DROP ");

    Reference<XPropertySet> xProp(xObject, UNO_QUERY);
    bool bIsView = xProp.is()
                   && ::comphelper::getString(xProp->getPropertyValue(
                          OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)))
                          == "VIEW";
    if (bIsView) // here we have a view
        aSql += "VIEW ";
    else
        aSql += "TABLE ";

    OUString sComposedName(::dbtools::composeTableName(
        m_xMetaData, sCatalog, sSchema, sTable, true, ::dbtools::EComposeRule::InDataManipulation));
    aSql += sComposedName;
    Reference<XStatement> xStmt = xConnection->createStatement();
    if (xStmt.is())
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
    // if no exception was thrown we must delete it from the views
    if (bIsView)
    {
        OViews* pViews
            = static_cast<OViews*>(static_cast<OMySQLCatalog&>(m_rParent).getPrivateViews());
        if (pViews && pViews->hasByName(_sElementName))
            pViews->dropByNameImpl(_sElementName);
    }
}

OUString OTables::adjustSQL(const OUString& _sSql)
{
    OUString sSQL = _sSql;
    static const char s_sUNSIGNED[] = "UNSIGNED";
    sal_Int32 nIndex = sSQL.indexOf(s_sUNSIGNED);
    while (nIndex != -1)
    {
        sal_Int32 nParen = sSQL.indexOf(')', nIndex);
        sal_Int32 nPos = nIndex + strlen(s_sUNSIGNED);
        OUString sNewUnsigned(sSQL.copy(nPos, nParen - nPos + 1));
        sSQL = sSQL.replaceAt(nIndex, strlen(s_sUNSIGNED) + sNewUnsigned.getLength(),
                              rtl::Concat2View(sNewUnsigned + s_sUNSIGNED));
        nIndex = sSQL.indexOf(s_sUNSIGNED, nIndex + strlen(s_sUNSIGNED) + sNewUnsigned.getLength());
    }
    return sSQL;
}

void OTables::createTable(const Reference<XPropertySet>& descriptor)
{
    const Reference<XConnection> xConnection
        = static_cast<OMySQLCatalog&>(m_rParent).getConnection();
    const OUString aSql
        = adjustSQL(::dbtools::createSqlCreateTableStatement(descriptor, xConnection));
    Reference<XStatement> xStmt = xConnection->createStatement();
    if (xStmt.is())
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

void OTables::appendNew(const OUString& _rsNewTable)
{
    insertElement(_rsNewTable, nullptr);

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), Any(_rsNewTable), Any(), Any());
    OInterfaceIteratorHelper3 aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        aListenerLoop.next()->elementInserted(aEvent);
}

OUString OTables::getNameForObject(const sdbcx::ObjectType& _xObject)
{
    OSL_ENSURE(_xObject.is(), "OTables::getNameForObject: Object is NULL!");
    return ::dbtools::composeTableName(m_xMetaData, _xObject,
                                       ::dbtools::EComposeRule::InDataManipulation, false);
}

void OTables::addComment(const Reference<XPropertySet>& descriptor, OUStringBuffer& _rOut)
{
    OUString sDesc;
    descriptor->getPropertyValue(
        OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION))
        >>= sDesc;
    if (!sDesc.isEmpty())
    {
        _rOut.append(" COMMENT '");
        _rOut.append(sDesc);
        _rOut.append("'");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
