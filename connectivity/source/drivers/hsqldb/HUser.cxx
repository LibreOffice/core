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

#include "hsqldb/HUser.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include "TConnection.hxx"
#include "strings.hrc"

using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OHSQLUser::OHSQLUser(   const css::uno::Reference< css::sdbc::XConnection >& _xConnection) : connectivity::sdbcx::OUser(true)
                ,m_xConnection(_xConnection)
{
    construct();
}

OHSQLUser::OHSQLUser(   const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                const OUString& Name
            ) : connectivity::sdbcx::OUser(Name,true)
                ,m_xConnection(_xConnection)
{
    construct();
}

void OHSQLUser::refreshGroups()
{
}

OUserExtend::OUserExtend(   const css::uno::Reference< css::sdbc::XConnection >& _xConnection) : OHSQLUser(_xConnection)
{
    construct();
}

void OUserExtend::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD),    PROPERTY_ID_PASSWORD,0,&m_Password,::cppu::UnoType<OUString>::get());
}

cppu::IPropertyArrayHelper* OUserExtend::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new cppu::OPropertyArrayHelper(aProps);
}

cppu::IPropertyArrayHelper & OUserExtend::getInfoHelper()
{
    return *OUserExtend_PROP::getArrayHelper();
}
typedef connectivity::sdbcx::OUser_BASE OUser_BASE_RBHELPER;

sal_Int32 SAL_CALL OHSQLUser::getPrivileges( const OUString& objName, sal_Int32 objType )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights,nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName,objType,nRights,nRightsWithGrant);
    return nRights;
}

void OHSQLUser::findPrivilegesAndGrantPrivileges(const OUString& objName, sal_Int32 objType,sal_Int32& nRights,sal_Int32& nRightsWithGrant)
{
    nRightsWithGrant = nRights = 0;
    // first we need to create the sql stmt to select the privs
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(xMeta,objName,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);
    Reference<XResultSet> xRes;
    switch(objType)
    {
        case PrivilegeObject::TABLE:
        case PrivilegeObject::VIEW:
            {
                Any aCatalog;
                if ( !sCatalog.isEmpty() )
                    aCatalog <<= sCatalog;
                xRes = xMeta->getTablePrivileges(aCatalog,sSchema,sTable);
            }
            break;

        case PrivilegeObject::COLUMN:
            {
                Any aCatalog;
                if ( !sCatalog.isEmpty() )
                    aCatalog <<= sCatalog;
                xRes = xMeta->getColumnPrivileges(aCatalog,sSchema,sTable,"%");
            }
            break;
    }

    if ( xRes.is() )
    {
        static const char sYes      [] = "YES";

        nRightsWithGrant = nRights = 0;

        Reference<XRow> xCurrentRow(xRes,UNO_QUERY);
        while( xCurrentRow.is() && xRes->next() )
        {
            OUString sGrantee    = xCurrentRow->getString(5);
            OUString sPrivilege  = xCurrentRow->getString(6);
            OUString sGrantable  = xCurrentRow->getString(7);

            if (!m_Name.equalsIgnoreAsciiCase(sGrantee))
                continue;

            if (sPrivilege.equalsIgnoreAsciiCase("SELECT"))
            {
                nRights |= Privilege::SELECT;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::SELECT;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("INSERT"))
            {
                nRights |= Privilege::INSERT;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::INSERT;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("UPDATE"))
            {
                nRights |= Privilege::UPDATE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::UPDATE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("DELETE"))
            {
                nRights |= Privilege::DELETE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::DELETE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("READ"))
            {
                nRights |= Privilege::READ;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::READ;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("CREATE"))
            {
                nRights |= Privilege::CREATE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::CREATE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("ALTER"))
            {
                nRights |= Privilege::ALTER;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::ALTER;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("REFERENCE"))
            {
                nRights |= Privilege::REFERENCE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::REFERENCE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase("DROP"))
            {
                nRights |= Privilege::DROP;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::DROP;
            }
        }
        ::comphelper::disposeComponent(xRes);
    }
}

sal_Int32 SAL_CALL OHSQLUser::getGrantablePrivileges( const OUString& objName, sal_Int32 objType )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights,nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName,objType,nRights,nRightsWithGrant);
    return nRightsWithGrant;
}

void SAL_CALL OHSQLUser::grantPrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges )
{
    if ( objType != PrivilegeObject::TABLE )
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(STR_PRIVILEGE_NOT_GRANTED));
        ::dbtools::throwGenericSQLException(sError,*this);
    } // if ( objType != PrivilegeObject::TABLE )


    ::osl::MutexGuard aGuard(m_aMutex);

    OUString sPrivs = getPrivilegeString(objPrivileges);
    if(!sPrivs.isEmpty())
    {
        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        OUString sGrant = "GRANT " +  sPrivs +
            " ON " + ::dbtools::quoteTableName(xMeta,objName,::dbtools::EComposeRule::InDataManipulation) +
            " TO " + ::dbtools::quoteName(xMeta->getIdentifierQuoteString(), m_Name);

        Reference<XStatement> xStmt = m_xConnection->createStatement();
        if(xStmt.is())
            xStmt->execute(sGrant);
        ::comphelper::disposeComponent(xStmt);
    }
}

void SAL_CALL OHSQLUser::revokePrivileges( const OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges )
{
    if ( objType != PrivilegeObject::TABLE )
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(STR_PRIVILEGE_NOT_REVOKED));
        ::dbtools::throwGenericSQLException(sError,*this);
    } // if ( objType != PrivilegeObject::TABLE )

    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);
    OUString sPrivs = getPrivilegeString(objPrivileges);
    if(!sPrivs.isEmpty())
    {
        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        OUString sGrant = "REVOKE " + sPrivs +
            " ON " + ::dbtools::quoteTableName(xMeta,objName,::dbtools::EComposeRule::InDataManipulation) +
            " FROM " + ::dbtools::quoteName(xMeta->getIdentifierQuoteString(), m_Name);

        Reference<XStatement> xStmt = m_xConnection->createStatement();
        if(xStmt.is())
            xStmt->execute(sGrant);
        ::comphelper::disposeComponent(xStmt);
    }
}

// XUser
void SAL_CALL OHSQLUser::changePassword( const OUString& /*oldPassword*/, const OUString& newPassword )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();

    if( m_Name != xMeta->getUserName() )
    {
        ::dbtools::throwGenericSQLException("HSQLDB can only change password of the current user.", *this);
    }

    OUString sAlterPwd = "SET PASSWORD " +
        ::dbtools::quoteName(xMeta->getIdentifierQuoteString(), newPassword);

    Reference<XStatement> xStmt = m_xConnection->createStatement();
    if ( xStmt.is() )
    {
        xStmt->execute(sAlterPwd);
        ::comphelper::disposeComponent(xStmt);
    }
}

OUString OHSQLUser::getPrivilegeString(sal_Int32 nRights)
{
    OUString sPrivs;
    if((nRights & Privilege::INSERT) == Privilege::INSERT)
        sPrivs += "INSERT";

    if((nRights & Privilege::DELETE) == Privilege::DELETE)
    {
        if(!sPrivs.isEmpty())
            sPrivs += ",";
        sPrivs += "DELETE";
    }

    if((nRights & Privilege::UPDATE) == Privilege::UPDATE)
    {
        if(!sPrivs.isEmpty())
            sPrivs += ",";
        sPrivs += "UPDATE";
    }

    if((nRights & Privilege::ALTER) == Privilege::ALTER)
    {
        if(!sPrivs.isEmpty())
            sPrivs += ",";
        sPrivs += "ALTER";
    }

    if((nRights & Privilege::SELECT) == Privilege::SELECT)
    {
        if(!sPrivs.isEmpty())
            sPrivs += ",";
        sPrivs += "SELECT";
    }

    if((nRights & Privilege::REFERENCE) == Privilege::REFERENCE)
    {
        if(!sPrivs.isEmpty())
            sPrivs += ",";
        sPrivs += "REFERENCES";
    }

    return sPrivs;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
