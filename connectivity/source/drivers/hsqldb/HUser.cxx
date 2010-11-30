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
#include "hsqldb/HUser.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include "TConnection.hxx"
#include "resource/hsqldb_res.hrc"

using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OHSQLUser::OHSQLUser(   const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection) : connectivity::sdbcx::OUser(sal_True)
                ,m_xConnection(_xConnection)
{
    construct();
}
// -------------------------------------------------------------------------
OHSQLUser::OHSQLUser(   const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                const ::rtl::OUString& _Name
            ) : connectivity::sdbcx::OUser(_Name,sal_True)
                ,m_xConnection(_xConnection)
{
    construct();
}
// -------------------------------------------------------------------------
void OHSQLUser::refreshGroups()
{
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(   const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection) : OHSQLUser(_xConnection)
{
    construct();
}
// -------------------------------------------------------------------------
typedef connectivity::sdbcx::OUser  OUser_TYPEDEF;
void OUserExtend::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD),    PROPERTY_ID_PASSWORD,0,&m_Password,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -----------------------------------------------------------------------------
cppu::IPropertyArrayHelper* OUserExtend::createArrayHelper() const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
cppu::IPropertyArrayHelper & OUserExtend::getInfoHelper()
{
    return *OUserExtend_PROP::getArrayHelper();
}
typedef connectivity::sdbcx::OUser_BASE OUser_BASE_RBHELPER;
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OHSQLUser::getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights,nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName,objType,nRights,nRightsWithGrant);
    return nRights;
}
// -----------------------------------------------------------------------------
void OHSQLUser::findPrivilegesAndGrantPrivileges(const ::rtl::OUString& objName, sal_Int32 objType,sal_Int32& nRights,sal_Int32& nRightsWithGrant) throw(SQLException, RuntimeException)
{
    nRightsWithGrant = nRights = 0;
    // first we need to create the sql stmt to select the privs
    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    ::rtl::OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(xMeta,objName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
    Reference<XResultSet> xRes;
    switch(objType)
    {
        case PrivilegeObject::TABLE:
        case PrivilegeObject::VIEW:
            {
                Any aCatalog;
                if ( sCatalog.getLength() )
                    aCatalog <<= sCatalog;
                xRes = xMeta->getTablePrivileges(aCatalog,sSchema,sTable);
            }
            break;

        case PrivilegeObject::COLUMN:
            {
                Any aCatalog;
                if ( sCatalog.getLength() )
                    aCatalog <<= sCatalog;
                xRes = xMeta->getColumnPrivileges(aCatalog,sSchema,sTable,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")));
            }
            break;
    }

    if ( xRes.is() )
    {
        static const ::rtl::OUString sSELECT( RTL_CONSTASCII_USTRINGPARAM( "SELECT" ));
        static const ::rtl::OUString sINSERT( RTL_CONSTASCII_USTRINGPARAM( "INSERT" ));
        static const ::rtl::OUString sUPDATE( RTL_CONSTASCII_USTRINGPARAM( "UPDATE" ));
        static const ::rtl::OUString sDELETE( RTL_CONSTASCII_USTRINGPARAM( "DELETE" ));
        static const ::rtl::OUString sREAD( RTL_CONSTASCII_USTRINGPARAM( "READ" ));
        static const ::rtl::OUString sCREATE( RTL_CONSTASCII_USTRINGPARAM( "CREATE" ));
        static const ::rtl::OUString sALTER( RTL_CONSTASCII_USTRINGPARAM( "ALTER" ));
        static const ::rtl::OUString sREFERENCE( RTL_CONSTASCII_USTRINGPARAM( "REFERENCE" ));
        static const ::rtl::OUString sDROP( RTL_CONSTASCII_USTRINGPARAM( "DROP" ));
        static const ::rtl::OUString sYes( RTL_CONSTASCII_USTRINGPARAM( "YES" ));

        nRightsWithGrant = nRights = 0;

        Reference<XRow> xCurrentRow(xRes,UNO_QUERY);
        while( xCurrentRow.is() && xRes->next() )
        {
            ::rtl::OUString sGrantee    = xCurrentRow->getString(5);
            ::rtl::OUString sPrivilege  = xCurrentRow->getString(6);
            ::rtl::OUString sGrantable  = xCurrentRow->getString(7);

            if (!m_Name.equalsIgnoreAsciiCase(sGrantee))
                continue;

            if (sPrivilege.equalsIgnoreAsciiCase(sSELECT))
            {
                nRights |= Privilege::SELECT;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::SELECT;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sINSERT))
            {
                nRights |= Privilege::INSERT;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::INSERT;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sUPDATE))
            {
                nRights |= Privilege::UPDATE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::UPDATE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sDELETE))
            {
                nRights |= Privilege::DELETE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::DELETE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sREAD))
            {
                nRights |= Privilege::READ;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::READ;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sCREATE))
            {
                nRights |= Privilege::CREATE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::CREATE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sALTER))
            {
                nRights |= Privilege::ALTER;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::ALTER;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sREFERENCE))
            {
                nRights |= Privilege::REFERENCE;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::REFERENCE;
            }
            else if (sPrivilege.equalsIgnoreAsciiCase(sDROP))
            {
                nRights |= Privilege::DROP;
                if ( sGrantable.equalsIgnoreAsciiCase(sYes) )
                    nRightsWithGrant |= Privilege::DROP;
            }
        }
        ::comphelper::disposeComponent(xRes);
    }
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OHSQLUser::getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights,nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName,objType,nRights,nRightsWithGrant);
    return nRightsWithGrant;
}
// -------------------------------------------------------------------------
void SAL_CALL OHSQLUser::grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(SQLException, RuntimeException)
{
    if ( objType != PrivilegeObject::TABLE )
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sError( aResources.getResourceString(STR_PRIVILEGE_NOT_GRANTED));
        ::dbtools::throwGenericSQLException(sError,*this);
    } // if ( objType != PrivilegeObject::TABLE )


    ::osl::MutexGuard aGuard(m_aMutex);

    ::rtl::OUString sPrivs = getPrivilegeString(objPrivileges);
    if(sPrivs.getLength())
    {
        ::rtl::OUString sGrant;
        sGrant += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GRANT "));
        sGrant += sPrivs;
        sGrant += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON "));
        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        sGrant += ::dbtools::quoteTableName(xMeta,objName,::dbtools::eInDataManipulation);
        sGrant += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TO "));
        sGrant += m_Name;

        Reference<XStatement> xStmt = m_xConnection->createStatement();
        if(xStmt.is())
            xStmt->execute(sGrant);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OHSQLUser::revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(SQLException, RuntimeException)
{
    if ( objType != PrivilegeObject::TABLE )
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sError( aResources.getResourceString(STR_PRIVILEGE_NOT_REVOKED));
        ::dbtools::throwGenericSQLException(sError,*this);
    } // if ( objType != PrivilegeObject::TABLE )

    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);
    ::rtl::OUString sPrivs = getPrivilegeString(objPrivileges);
    if(sPrivs.getLength())
    {
        ::rtl::OUString sGrant;
        sGrant += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REVOKE "));
        sGrant += sPrivs;
        sGrant += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON "));
        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        sGrant += ::dbtools::quoteTableName(xMeta,objName,::dbtools::eInDataManipulation);
        sGrant += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM "));
        sGrant += m_Name;

        Reference<XStatement> xStmt = m_xConnection->createStatement();
        if(xStmt.is())
            xStmt->execute(sGrant);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
// XUser
void SAL_CALL OHSQLUser::changePassword( const ::rtl::OUString& /*oldPassword*/, const ::rtl::OUString& newPassword ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);
    ::rtl::OUString sAlterPwd;
    sAlterPwd = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SET PASSWORD FOR "));
    sAlterPwd += m_Name;
    sAlterPwd += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("@\"%\" = PASSWORD('")) ;
    sAlterPwd += newPassword;
    sAlterPwd += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("')")) ;


    Reference<XStatement> xStmt = m_xConnection->createStatement();
    if ( xStmt.is() )
    {
        xStmt->execute(sAlterPwd);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString OHSQLUser::getPrivilegeString(sal_Int32 nRights) const
{
    ::rtl::OUString sPrivs;
    if((nRights & Privilege::INSERT) == Privilege::INSERT)
        sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INSERT"));

    if((nRights & Privilege::DELETE) == Privilege::DELETE)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DELETE"));
    }

    if((nRights & Privilege::UPDATE) == Privilege::UPDATE)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UPDATE"));
    }

    if((nRights & Privilege::ALTER) == Privilege::ALTER)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ALTER"));
    }

    if((nRights & Privilege::SELECT) == Privilege::SELECT)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT"));
    }

    if((nRights & Privilege::REFERENCE) == Privilege::REFERENCE)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        sPrivs += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REFERENCES"));
    }

    return sPrivs;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
