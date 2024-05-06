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

#include "mysqlc_tables.hxx"
#include "mysqlc_views.hxx"
#include "mysqlc_view.hxx"
#include "mysqlc_catalog.hxx"
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <TConnection.hxx>

connectivity::mysqlc::Views::Views(const css::uno::Reference<css::sdbc::XConnection>& _rxConnection,
                                   ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
                                   const ::std::vector<OUString>& _rVector)
    : sdbcx::OCollection(_rParent, true, _rMutex, _rVector)
    , m_xConnection(_rxConnection)
    , m_xMetaData(_rxConnection->getMetaData())
{
}

connectivity::sdbcx::ObjectType connectivity::mysqlc::Views::createObject(const OUString& _rName)
{
    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData, _rName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);
    return new View(m_xConnection, isCaseSensitive(), sSchema, sTable);
}

void connectivity::mysqlc::Views::impl_refresh()
{
    static_cast<Catalog&>(m_rParent).refreshViews();
}

css::uno::Reference<css::beans::XPropertySet> connectivity::mysqlc::Views::createDescriptor()
{
    return new connectivity::sdbcx::OView(true, m_xMetaData);
}

// XAppend
connectivity::sdbcx::ObjectType connectivity::mysqlc::Views::appendObject(
    const OUString& _rForName, const css::uno::Reference<css::beans::XPropertySet>& descriptor)
{
    createView(descriptor);
    return createObject(_rForName);
}

// XDrop
void connectivity::mysqlc::Views::dropObject(sal_Int32 _nPos, const OUString& /*_sElementName*/)
{
    css::uno::Reference<XInterface> xObject(getObject(_nPos));
    bool bIsNew = connectivity::sdbcx::ODescriptor::isNew(xObject);
    if (!bIsNew)
    {
        OUString aSql(u"DROP VIEW"_ustr);

        css::uno::Reference<css::beans::XPropertySet> xProp(xObject, css::uno::UNO_QUERY);
        aSql += ::dbtools::composeTableName(m_xMetaData, xProp,
                                            ::dbtools::EComposeRule::InTableDefinitions, true);

        css::uno::Reference<css::sdbc::XConnection> xConnection = m_xMetaData->getConnection();
        css::uno::Reference<css::sdbc::XStatement> xStmt = xConnection->createStatement();
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

void connectivity::mysqlc::Views::createView(
    const css::uno::Reference<css::beans::XPropertySet>& descriptor)
{
    css::uno::Reference<css::sdbc::XConnection> xConnection = m_xMetaData->getConnection();

    OUString sCommand;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND))
        >>= sCommand;

    OUString aSql = "CREATE VIEW "
                    + ::dbtools::composeTableName(m_xMetaData, descriptor,
                                                  ::dbtools::EComposeRule::InTableDefinitions, true)
                    + " AS " + sCommand;

    css::uno::Reference<css::sdbc::XStatement> xStmt = xConnection->createStatement();
    if (xStmt.is())
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
    connectivity::mysqlc::Tables* pTables = static_cast<connectivity::mysqlc::Tables*>(
        static_cast<connectivity::mysqlc::Catalog&>(m_rParent).getPrivateTables());
    if (pTables)
    {
        OUString sName = ::dbtools::composeTableName(
            m_xMetaData, descriptor, ::dbtools::EComposeRule::InDataManipulation, false);
        pTables->appendNew(sName);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
