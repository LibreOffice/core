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

#include <hsqldb/HUsers.hxx>
#include <hsqldb/HUser.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <TConnection.hxx>
#include <utility>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;

OUsers::OUsers( ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                const ::std::vector< OUString> &_rVector,
                css::uno::Reference< css::sdbc::XConnection > _xConnection,
                connectivity::sdbcx::IRefreshableUsers* _pParent)
    : sdbcx::OCollection(_rParent, true, _rMutex, _rVector)
    ,m_xConnection(std::move(_xConnection))
    ,m_pParent(_pParent)
{
}


sdbcx::ObjectType OUsers::createObject(const OUString& _rName)
{
    return new OHSQLUser(m_xConnection,_rName);
}

void OUsers::impl_refresh()
{
    m_pParent->refreshUsers();
}

Reference< XPropertySet > OUsers::createDescriptor()
{
    return new OUserExtend(m_xConnection);
}

// XAppend
sdbcx::ObjectType OUsers::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OUString aQuote  = m_xConnection->getMetaData()->getIdentifierQuoteString(  );
    OUString sPassword;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPassword;
    OUString aSql =  "GRANT USAGE ON * TO " +
        ::dbtools::quoteName(aQuote,_rForName) + " @\"%\" ";
    if ( !sPassword.isEmpty() )
    {
        aSql += " IDENTIFIED BY '" + sPassword + "'";
    }

    Reference< XStatement > xStmt = m_xConnection->createStatement(  );
    if(xStmt.is())
        xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}

// XDrop
void OUsers::dropObject(sal_Int32 /*nPos*/,const OUString& _sElementName)
{
    OUString aSql(  u"REVOKE ALL ON * FROM "_ustr );
    OUString aQuote  = m_xConnection->getMetaData()->getIdentifierQuoteString(  );
    aSql += ::dbtools::quoteName(aQuote,_sElementName);

    Reference< XStatement > xStmt = m_xConnection->createStatement(  );
    if(xStmt.is())
        xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
