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

#include "hsqldb/HUsers.hxx"
#include "hsqldb/HUser.hxx"
#include "hsqldb/HTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include "TConnection.hxx"

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OUsers::OUsers( ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                const TStringVector &_rVector,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                connectivity::sdbcx::IRefreshableUsers* _pParent)
    : sdbcx::OCollection(_rParent, true, _rMutex, _rVector)
    ,m_xConnection(_xConnection)
    ,m_pParent(_pParent)
{
}


sdbcx::ObjectType OUsers::createObject(const OUString& _rName)
{
    return new OHSQLUser(m_xConnection,_rName);
}

void OUsers::impl_refresh() throw(RuntimeException)
{
    m_pParent->refreshUsers();
}

Reference< XPropertySet > OUsers::createDescriptor()
{
    OUserExtend* pNew = new OUserExtend(m_xConnection);
    return pNew;
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
    {
        OUString aSql(  "REVOKE ALL ON * FROM " );
        OUString aQuote  = m_xConnection->getMetaData()->getIdentifierQuoteString(  );
        aSql += ::dbtools::quoteName(aQuote,_sElementName);

        Reference< XStatement > xStmt = m_xConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
