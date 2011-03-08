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
#include "adabas/BUsers.hxx"
#include "adabas/BUser.hxx"
#include "adabas/BTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include <comphelper/types.hxx>
#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"
#include "resource/adabas_res.hrc"

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OUsers::createObject(const ::rtl::OUString& _rName)
{
    return new OAdabasUser(m_pConnection,_rName);
}
// -------------------------------------------------------------------------
void OUsers::impl_refresh() throw(RuntimeException)
{
    m_pParent->refreshUsers();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OUsers::createDescriptor()
{
    OUserExtend* pNew = new OUserExtend(m_pConnection);
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OUsers::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "CREATE USER " ));
    ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );

    ::rtl::OUString sUserName( _rForName );
    sUserName = sUserName.toAsciiUpperCase();
    descriptor->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(sUserName));
    aSql += ::dbtools::quoteName(aQuote,sUserName)
                + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PASSWORD "))
                + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)));
    aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" RESOURCE NOT EXCLUSIVE"));

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    if(xStmt.is())
        xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OUsers::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    {
        // first we have to check if this user is live relevaant for the database
        // which means with out these users the database will miss more than one important system table
        ::rtl::OUString sUsers( RTL_CONSTASCII_USTRINGPARAM( "SELECT USERMODE,USERNAME FROM DOMAIN.USERS WHERE USERNAME = '" ));
        sUsers += _sElementName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));
        Reference< XStatement > xStmt = m_pConnection->createStatement();
        if(xStmt.is())
        {
            Reference<XResultSet> xRes = xStmt->executeQuery(sUsers);
            Reference<XRow> xRow(xRes,UNO_QUERY);
            if(xRes.is() && xRow.is() && xRes->next()) // there can only be one user with this name
            {
                static const ::rtl::OUString sDbaUser( RTL_CONSTASCII_USTRINGPARAM( "DBA" ));
                if(xRow->getString(1) == sDbaUser)
                {
                    ::comphelper::disposeComponent(xStmt);
                    m_pConnection->throwGenericSQLException(STR_USER_NO_DELETE,static_cast< XDrop* >( this ));
                }
            }
            ::comphelper::disposeComponent(xStmt);
        }
    }

    {
        ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "DROP USER " ));
        ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );
        aSql += ::dbtools::quoteName(aQuote,_sElementName);

        Reference< XStatement > xStmt = m_pConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
