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
#include "mysql/YUsers.hxx"
#include "mysql/YUser.hxx"
#include "mysql/YTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include <comphelper/types.hxx>
#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"
#include "TConnection.hxx"

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::mysql;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

OUsers::OUsers( ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                const TStringVector &_rVector,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                connectivity::sdbcx::IRefreshableUsers* _pParent)
    : sdbcx::OCollection(_rParent,sal_True,_rMutex,_rVector)
    ,m_xConnection(_xConnection)
    ,m_pParent(_pParent)
{
}
// -----------------------------------------------------------------------------

sdbcx::ObjectType OUsers::createObject(const ::rtl::OUString& _rName)
{
    return new OMySQLUser(m_xConnection,_rName);
}
// -------------------------------------------------------------------------
void OUsers::impl_refresh() throw(RuntimeException)
{
    m_pParent->refreshUsers();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OUsers::createDescriptor()
{
    OUserExtend* pNew = new OUserExtend(m_xConnection);
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OUsers::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "GRANT USAGE ON * TO " ));
    ::rtl::OUString aQuote  = m_xConnection->getMetaData()->getIdentifierQuoteString(  );
    ::rtl::OUString sUserName( _rForName );
    aSql += ::dbtools::quoteName(aQuote,sUserName)
                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" @\"%\" "));
    ::rtl::OUString sPassword;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPassword;
    if ( sPassword.getLength() )
    {
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IDENTIFIED BY '"));
        aSql += sPassword;
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));
    }

    Reference< XStatement > xStmt = m_xConnection->createStatement(  );
    if(xStmt.is())
        xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OUsers::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "REVOKE ALL ON * FROM " ));
    ::rtl::OUString aQuote  = m_xConnection->getMetaData()->getIdentifierQuoteString(  );
    aSql += ::dbtools::quoteName(aQuote,_sElementName);

    Reference< XStatement > xStmt = m_xConnection->createStatement(  );
    if(xStmt.is())
        xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);
}

// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
