/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <utility>

#include "mysqlc_user.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include <TConnection.hxx>

using namespace ::connectivity;
using namespace ::connectivity::mysqlc;
using namespace ::connectivity::sdbcx;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

User::User(css::uno::Reference<css::sdbc::XConnection> xConnection)
    : OUser(true) // Case Sensitive
    , m_xConnection(std::move(xConnection))
{
}

User::User(css::uno::Reference<css::sdbc::XConnection> xConnection, const OUString& rName)
    : OUser(rName,
            true) // Case Sensitive
    , m_xConnection(std::move(xConnection))
{
}

OUserExtend::OUserExtend(const css::uno::Reference<css::sdbc::XConnection>& xConnection,
                         const OUString& rName)
    : User(xConnection, rName)
{
    construct();
}

void OUserExtend::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),
                     PROPERTY_ID_NAME, 0, &m_Name, ::cppu::UnoType<OUString>::get());

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD),
                     PROPERTY_ID_PASSWORD, 0, &m_Password, ::cppu::UnoType<OUString>::get());
}

void User::changePassword(const OUString& /* oldPassword */, const OUString& newPassword)
{
    css::uno::Reference<XStatement> statement = m_xConnection->createStatement();
    statement->execute("SET PASSWORD FOR " + m_Name + " = PASSWORD('" + newPassword + "')");
    ::comphelper::disposeComponent(statement);
}

cppu::IPropertyArrayHelper* OUserExtend::createArrayHelper() const
{
    css::uno::Sequence<css::beans::Property> aProps;
    describeProperties(aProps);
    return new cppu::OPropertyArrayHelper(aProps);
}

cppu::IPropertyArrayHelper& OUserExtend::getInfoHelper()
{
    return *OUserExtend_PROP::getArrayHelper();
}

typedef connectivity::sdbcx::OUser_BASE OUser_BASE_RBHELPER;

sal_Int32 SAL_CALL User::getPrivileges(const OUString& objName, sal_Int32 objType)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights, nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName, objType, nRights, nRightsWithGrant);
    return nRights;
}

void User::findPrivilegesAndGrantPrivileges(const OUString& objName, sal_Int32 objType,
                                            sal_Int32& nRights, sal_Int32& nRightsWithGrant)
{
    nRightsWithGrant = nRights = 0;
    // first we need to create the sql stmt to select the privs
    css::uno::Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(xMeta, objName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);
    css::uno::Reference<XResultSet> xRes;
    switch (objType)
    {
        case css::sdbcx::PrivilegeObject::TABLE:
        case css::sdbcx::PrivilegeObject::VIEW:
        {
            css::uno::Any aCatalog;
            if (!sCatalog.isEmpty())
                aCatalog <<= sCatalog;
            xRes = xMeta->getTablePrivileges(aCatalog, sSchema, sTable);
        }
        break;

        case css::sdbcx::PrivilegeObject::COLUMN:
        {
            css::uno::Any aCatalog;
            if (!sCatalog.isEmpty())
                aCatalog <<= sCatalog;
            xRes = xMeta->getColumnPrivileges(aCatalog, sSchema, sTable, u"%"_ustr);
        }
        break;
    }

    if (!xRes.is())
        return;

    static const char sYes[] = "YES";

    nRightsWithGrant = nRights = 0;

    css::uno::Reference<XRow> xCurrentRow(xRes, css::uno::UNO_QUERY);
    while (xCurrentRow.is() && xRes->next())
    {
        OUString sGrantee = xCurrentRow->getString(5);
        OUString sPrivilege = xCurrentRow->getString(6);
        OUString sGrantable = xCurrentRow->getString(7);

        if (!m_Name.equalsIgnoreAsciiCase(sGrantee))
            continue;

        if (sPrivilege.equalsIgnoreAsciiCase("SELECT"))
        {
            nRights |= Privilege::SELECT;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::SELECT;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("INSERT"))
        {
            nRights |= Privilege::INSERT;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::INSERT;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("UPDATE"))
        {
            nRights |= Privilege::UPDATE;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::UPDATE;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("DELETE"))
        {
            nRights |= Privilege::DELETE;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::DELETE;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("READ"))
        {
            nRights |= Privilege::READ;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::READ;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("CREATE"))
        {
            nRights |= Privilege::CREATE;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::CREATE;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("ALTER"))
        {
            nRights |= Privilege::ALTER;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::ALTER;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("REFERENCES"))
        {
            nRights |= Privilege::REFERENCE;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::REFERENCE;
        }
        else if (sPrivilege.equalsIgnoreAsciiCase("DROP"))
        {
            nRights |= Privilege::DROP;
            if (sGrantable.equalsIgnoreAsciiCase(sYes))
                nRightsWithGrant |= Privilege::DROP;
        }
    }
    ::comphelper::disposeComponent(xRes);
}

sal_Int32 SAL_CALL User::getGrantablePrivileges(const OUString& objName, sal_Int32 objType)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights, nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName, objType, nRights, nRightsWithGrant);
    return nRightsWithGrant;
}

//----- IRefreshableGroups ----------------------------------------------------
void User::refreshGroups()
{
    // TODO: implement.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
