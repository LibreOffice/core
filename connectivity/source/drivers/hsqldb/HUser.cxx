/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD),    PROPERTY_ID_PASSWORD,0,&m_Password,::getCppuType(static_cast< ::rtl::OUString*>(NULL)));
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
        static const ::rtl::OUString sSELECT    = ::rtl::OUString::createFromAscii("SELECT");
        static const ::rtl::OUString sINSERT    = ::rtl::OUString::createFromAscii("INSERT");
        static const ::rtl::OUString sUPDATE    = ::rtl::OUString::createFromAscii("UPDATE");
        static const ::rtl::OUString sDELETE    = ::rtl::OUString::createFromAscii("DELETE");
        static const ::rtl::OUString sREAD      = ::rtl::OUString::createFromAscii("READ");
        static const ::rtl::OUString sCREATE    = ::rtl::OUString::createFromAscii("CREATE");
        static const ::rtl::OUString sALTER     = ::rtl::OUString::createFromAscii("ALTER");
        static const ::rtl::OUString sREFERENCE = ::rtl::OUString::createFromAscii("REFERENCE");
        static const ::rtl::OUString sDROP      = ::rtl::OUString::createFromAscii("DROP");
        static const ::rtl::OUString sYes       = ::rtl::OUString::createFromAscii("YES");

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
        sGrant += ::rtl::OUString::createFromAscii("GRANT ");
        sGrant += sPrivs;
        sGrant += ::rtl::OUString::createFromAscii(" ON ");
        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        sGrant += ::dbtools::quoteTableName(xMeta,objName,::dbtools::eInDataManipulation);
        sGrant += ::rtl::OUString::createFromAscii(" TO ");
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
        sGrant += ::rtl::OUString::createFromAscii("REVOKE ");
        sGrant += sPrivs;
        sGrant += ::rtl::OUString::createFromAscii(" ON ");
        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        sGrant += ::dbtools::quoteTableName(xMeta,objName,::dbtools::eInDataManipulation);
        sGrant += ::rtl::OUString::createFromAscii(" FROM ");
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
    sAlterPwd = ::rtl::OUString::createFromAscii("SET PASSWORD FOR ");
    sAlterPwd += m_Name;
    sAlterPwd += ::rtl::OUString::createFromAscii("@\"%\" = PASSWORD('") ;
    sAlterPwd += newPassword;
    sAlterPwd += ::rtl::OUString::createFromAscii("')") ;


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
        sPrivs += ::rtl::OUString::createFromAscii("INSERT");

    if((nRights & Privilege::DELETE) == Privilege::DELETE)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString::createFromAscii(",");
        sPrivs += ::rtl::OUString::createFromAscii("DELETE");
    }

    if((nRights & Privilege::UPDATE) == Privilege::UPDATE)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString::createFromAscii(",");
        sPrivs += ::rtl::OUString::createFromAscii("UPDATE");
    }

    if((nRights & Privilege::ALTER) == Privilege::ALTER)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString::createFromAscii(",");
        sPrivs += ::rtl::OUString::createFromAscii("ALTER");
    }

    if((nRights & Privilege::SELECT) == Privilege::SELECT)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString::createFromAscii(",");
        sPrivs += ::rtl::OUString::createFromAscii("SELECT");
    }

    if((nRights & Privilege::REFERENCE) == Privilege::REFERENCE)
    {
        if(sPrivs.getLength())
            sPrivs += ::rtl::OUString::createFromAscii(",");
        sPrivs += ::rtl::OUString::createFromAscii("REFERENCES");
    }

    return sPrivs;
}
// -----------------------------------------------------------------------------

