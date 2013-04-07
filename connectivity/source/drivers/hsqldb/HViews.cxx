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


#include "hsqldb/HTables.hxx"
#include "hsqldb/HViews.hxx"
#include "hsqldb/HView.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/CheckOption.hpp>
#include "hsqldb/HCatalog.hxx"
#include <comphelper/extract.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/types.hxx>
#include "TConnection.hxx"

using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

// -------------------------------------------------------------------------
HViews::HViews( const Reference< XConnection >& _rxConnection, ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
    const TStringVector &_rVector )
    :sdbcx::OCollection( _rParent, sal_True, _rMutex, _rVector )
    ,m_xConnection( _rxConnection )
    ,m_xMetaData( _rxConnection->getMetaData() )
    ,m_bInDrop( sal_False )
{
}

// -------------------------------------------------------------------------
sdbcx::ObjectType HViews::createObject(const OUString& _rName)
{
    OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData,
                                        _rName,
                                        sCatalog,
                                        sSchema,
                                        sTable,
                                        ::dbtools::eInDataManipulation);
    return new HView( m_xConnection, isCaseSensitive(), sSchema, sTable );
}

// -------------------------------------------------------------------------
void HViews::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OHCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void HViews::disposing(void)
{
m_xMetaData.clear();
    OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > HViews::createDescriptor()
{
    Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();
    connectivity::sdbcx::OView* pNew = new connectivity::sdbcx::OView(sal_True,xConnection->getMetaData());
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType HViews::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    createView(descriptor);
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void HViews::dropObject(sal_Int32 _nPos,const OUString /*_sElementName*/)
{
    if ( m_bInDrop )
        return;

    Reference< XInterface > xObject( getObject( _nPos ) );
    sal_Bool bIsNew = connectivity::sdbcx::ODescriptor::isNew( xObject );
    if (!bIsNew)
    {
        OUString aSql(  "DROP VIEW" );

        Reference<XPropertySet> xProp(xObject,UNO_QUERY);
        aSql += ::dbtools::composeTableName( m_xMetaData, xProp, ::dbtools::eInTableDefinitions, false, false, true );

        Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = xConnection->createStatement(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void HViews::dropByNameImpl(const OUString& elementName)
{
    m_bInDrop = sal_True;
    OCollection_TYPE::dropByName(elementName);
    m_bInDrop = sal_False;
}
// -----------------------------------------------------------------------------
void HViews::createView( const Reference< XPropertySet >& descriptor )
{
    Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();

    OUString aSql(  "CREATE VIEW " );
    OUString sCommand;

    aSql += ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::eInTableDefinitions, false, false, true );

    aSql += OUString(" AS ");
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND)) >>= sCommand;
    aSql += sCommand;

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
        OUString sName = ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::eInDataManipulation, false, false, false );
        pTables->appendNew(sName);
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
