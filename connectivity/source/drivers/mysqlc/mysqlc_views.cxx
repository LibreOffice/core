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

using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity;
using namespace connectivity::mysqlc;
using namespace css::uno;
using namespace css::beans;
using namespace css::sdbcx;
using namespace css::sdbc;
using namespace css::container;
using namespace css::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Views::Views(const Reference<XConnection>& _rxConnection, ::cppu::OWeakObject& _rParent,
             ::osl::Mutex& _rMutex, const ::std::vector<OUString>& _rVector)
    : sdbcx::OCollection(_rParent, true, _rMutex, _rVector)
    , m_xConnection(_rxConnection)
    , m_xMetaData(_rxConnection->getMetaData())
    , m_bInDrop(false)
{
}

sdbcx::ObjectType Views::createObject(const OUString& _rName)
{
    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData, _rName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);
    return new View(m_xConnection, isCaseSensitive(), sSchema, sTable);
}

void Views::impl_refresh() { static_cast<Catalog&>(m_rParent).refreshViews(); }

void Views::disposing()
{
    m_xMetaData.clear();
    OCollection::disposing();
}

Reference<XPropertySet> Views::createDescriptor()
{
    return new connectivity::sdbcx::OView(true, m_xMetaData);
}

// XAppend
sdbcx::ObjectType Views::appendObject(const OUString& _rForName,
                                      const Reference<XPropertySet>& descriptor)
{
    createView(descriptor);
    return createObject(_rForName);
}

// XDrop
void Views::dropObject(sal_Int32 _nPos, const OUString& /*_sElementName*/)
{
    if (m_bInDrop)
        return;

    Reference<XInterface> xObject(getObject(_nPos));
    bool bIsNew = connectivity::sdbcx::ODescriptor::isNew(xObject);
    if (!bIsNew)
    {
        OUString aSql("DROP VIEW");

        Reference<XPropertySet> xProp(xObject, UNO_QUERY);
        aSql += ::dbtools::composeTableName(m_xMetaData, xProp,
                                            ::dbtools::EComposeRule::InTableDefinitions, true);

        Reference<XConnection> xConnection = m_xMetaData->getConnection();
        Reference<XStatement> xStmt = xConnection->createStatement();
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

void Views::dropByNameImpl(const OUString& elementName)
{
    m_bInDrop = true;
    OCollection_TYPE::dropByName(elementName);
    m_bInDrop = false;
}

void Views::createView(const Reference<XPropertySet>& descriptor)
{
    Reference<XConnection> xConnection = m_xMetaData->getConnection();

    OUString sCommand;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND))
        >>= sCommand;

    OUString aSql = "CREATE VIEW "
                    + ::dbtools::composeTableName(m_xMetaData, descriptor,
                                                  ::dbtools::EComposeRule::InTableDefinitions, true)
                    + " AS " + sCommand;

    Reference<XStatement> xStmt = xConnection->createStatement();
    if (xStmt.is())
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
    /*  TODO find a way to refresh view to make the new one appear right away
    // insert the new view also in the tables collection
    Tables* pTables = static_cast<Tables*>(static_cast<Catalog&>(m_rParent).getPrivateTables());
    if ( pTables)
    {
        OUString sName = ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::EComposeRule::InDataManipulation, false );
        pTables->appendNew(sName);
    }
*/
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
