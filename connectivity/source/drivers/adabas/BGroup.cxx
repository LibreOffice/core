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

#include "adabas/BGroup.hxx"
#include "adabas/BUsers.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "adabas/BConnection.hxx"
#include <comphelper/types.hxx>

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
OAdabasGroup::OAdabasGroup( OAdabasConnection* _pConnection) : connectivity::sdbcx::OGroup(sal_True)
                    ,m_pConnection(_pConnection)
{
    construct();
    TStringVector aVector;
    m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------
OAdabasGroup::OAdabasGroup( OAdabasConnection* _pConnection,
                const ::rtl::OUString& _Name
              ) : connectivity::sdbcx::OGroup(_Name,sal_True)
                ,m_pConnection(_pConnection)
{
    construct();
    refreshUsers();
}
// -------------------------------------------------------------------------
void OAdabasGroup::refreshUsers()
{
    if(!m_pConnection)
        return;

    TStringVector aVector;
        Reference< XStatement > xStmt = m_pConnection->createStatement(  );

    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "SELECT DISTINCT USERNAME FROM DOMAIN.USERS WHERE USERNAME IS NOT NULL AND USERNAME <> ' ' AND USERNAME <> 'CONTROL' AND GROUPNAME = '" ));
    aSql += getName( );
    aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));

    Reference< XResultSet >  xResult = xStmt->executeQuery(aSql);
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(1));
        ::comphelper::disposeComponent(xResult);
    }
    ::comphelper::disposeComponent(xStmt);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
