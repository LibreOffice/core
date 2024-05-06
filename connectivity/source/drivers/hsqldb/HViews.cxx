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


#include <hsqldb/HTables.hxx>
#include <hsqldb/HViews.hxx>
#include <hsqldb/HView.hxx>
#include <hsqldb/HCatalog.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <TConnection.hxx>

using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace css::uno;
using namespace css::beans;
using namespace css::sdbc;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;


HViews::HViews( const Reference< XConnection >& _rxConnection, ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
    const ::std::vector< OUString> &_rVector )
    :sdbcx::OCollection( _rParent, true, _rMutex, _rVector )
    ,m_xConnection( _rxConnection )
    ,m_xMetaData( _rxConnection->getMetaData() )
    ,m_bInDrop( false )
{
}


sdbcx::ObjectType HViews::createObject(const OUString& _rName)
{
    OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData,
                                        _rName,
                                        sCatalog,
                                        sSchema,
                                        sTable,
                                        ::dbtools::EComposeRule::InDataManipulation);
    return new HView( m_xConnection, isCaseSensitive(), sSchema, sTable );
}


void HViews::impl_refresh(  )
{
    static_cast<OHCatalog&>(m_rParent).refreshTables();
}

void HViews::disposing()
{
    m_xMetaData.clear();
    OCollection::disposing();
}

Reference< XPropertySet > HViews::createDescriptor()
{
    Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();
    return new connectivity::sdbcx::OView(true, xConnection->getMetaData());
}

// XAppend
sdbcx::ObjectType HViews::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    createView(descriptor);
    return createObject( _rForName );
}

// XDrop
void HViews::dropObject(sal_Int32 _nPos,const OUString& /*_sElementName*/)
{
    if ( m_bInDrop )
        return;

    Reference< XInterface > xObject( getObject( _nPos ) );
    bool bIsNew = connectivity::sdbcx::ODescriptor::isNew( xObject );
    if (!bIsNew)
    {
        OUString aSql(  u"DROP VIEW"_ustr );

        Reference<XPropertySet> xProp(xObject,UNO_QUERY);
        aSql += ::dbtools::composeTableName( m_xMetaData, xProp, ::dbtools::EComposeRule::InTableDefinitions, true );

        Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = xConnection->createStatement(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

void HViews::dropByNameImpl(const OUString& elementName)
{
    m_bInDrop = true;
    OCollection_TYPE::dropByName(elementName);
    m_bInDrop = false;
}

void HViews::createView( const Reference< XPropertySet >& descriptor )
{
    Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();

    OUString sCommand;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND)) >>= sCommand;

    OUString aSql = "CREATE VIEW " +
        ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::EComposeRule::InTableDefinitions, true ) +
        " AS " + sCommand;

    Reference< XStatement > xStmt = xConnection->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }

    // insert the new view also in the tables collection
    OTables* pTables = static_cast<OTables*>(static_cast<OHCatalog&>(m_rParent).getPrivateTables());
    if ( pTables )
    {
        OUString sName = ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::EComposeRule::InDataManipulation, false );
        pTables->appendNew(sName);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
