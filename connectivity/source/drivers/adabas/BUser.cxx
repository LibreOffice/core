/*************************************************************************
 *
 *  $RCSfile: BUser.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-24 06:12:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADABAS_USER_HXX_
#include "adabas/BUser.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_GROUPS_HXX_
#include "adabas/BGroups.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGEOBJECT_HPP_
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#endif

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OAdabasUser::OAdabasUser(   OAdabasConnection* _pConnection) : connectivity::sdbcx::OUser(sal_True)
                ,m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
OAdabasUser::OAdabasUser(   OAdabasConnection* _pConnection,
                const ::rtl::OUString& _Name
            ) : connectivity::sdbcx::OUser(_Name,sal_True)
                ,m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
void OAdabasUser::refreshGroups()
{
    if(!m_pConnection)
        return;

    TStringVector aVector;
    aVector.reserve(7); // we don't know the excatly count of users but this should fit the normal need
    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("SELECT DISTINCT GROUPNAME FROM DOMAIN.USERS WHERE GROUPNAME IS NOT NULL AND GROUPNAME <> ' ' AND USERNAME = '");
    aSql += getName( );
    aSql += ::rtl::OUString::createFromAscii("'");

    Reference< XResultSet > xResult = xStmt->executeQuery(aSql);
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(1));
    }

    if(m_pGroups)
        m_pGroups->reFill(aVector);
    else
        m_pGroups = new OGroups(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(   OAdabasConnection* _pConnection) : OAdabasUser(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(   OAdabasConnection* _pConnection,const ::rtl::OUString& _Name) : OAdabasUser(_pConnection,_Name)
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
sal_Int32 SAL_CALL OAdabasUser::getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights,nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName,objType,nRights,nRightsWithGrant);
    return nRights;
}
// -----------------------------------------------------------------------------
void OAdabasUser::findPrivilegesAndGrantPrivileges(const ::rtl::OUString& objName, sal_Int32 objType,sal_Int32& nRights,sal_Int32& nRightsWithGrant) throw(SQLException, RuntimeException)
{
    nRightsWithGrant = nRights = 0;
    // first we need to create the sql stmt to select the privs
    Reference<XDatabaseMetaData> xMeta = m_pConnection->getMetaData();
    ::rtl::OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(xMeta,objName,sCatalog,sSchema,sTable);
    Reference<XStatement> xStmt = m_pConnection->createStatement();
    ::rtl::OUString sSql = ::rtl::OUString::createFromAscii("SELECT REFTABLENAME,PRIVILEGES FROM DOMAIN.USR_USES_TAB WHERE REFOBJTYPE <> 'SYSTEM' AND DEFUSERNAME = '");
    sSql += m_Name;
    sSql += ::rtl::OUString::createFromAscii("' AND REFTABLENAME = '");
    sSql += sTable;
    sSql += ::rtl::OUString::createFromAscii("'");
    if(xStmt.is())
    {
        Reference<XResultSet> xRes = xStmt->executeQuery(sSql);
        if(xRes.is())
        {
            Reference<XRow> xRow(xRes,UNO_QUERY);
            if(xRow.is() && xRes->next())
            {
                ::rtl::OUString sPrivs = xRow->getString(2);
                static const ::rtl::OUString sInsert    = ::rtl::OUString::createFromAscii("INS");
                static const ::rtl::OUString sDelete    = ::rtl::OUString::createFromAscii("DEL");
                static const ::rtl::OUString sUpdate    = ::rtl::OUString::createFromAscii("UPD");
                static const ::rtl::OUString sAlter     = ::rtl::OUString::createFromAscii("ALT");
                static const ::rtl::OUString sSelect    = ::rtl::OUString::createFromAscii("SEL");
                static const ::rtl::OUString sReference = ::rtl::OUString::createFromAscii("REF");
                static const ::rtl::OUString sGrant     = ::rtl::OUString::createFromAscii("+");

                sal_Int32 nIndex = -1;
                if(nIndex = sPrivs.indexOf(sInsert) != -1)
                {
                    nRights |= Privilege::INSERT;
                    if(sGrant == sPrivs.copy(nIndex+2,1))
                        nRightsWithGrant |= Privilege::INSERT;
                }
                if(nIndex = sPrivs.indexOf(sDelete) != -1)
                {
                    nRights |= Privilege::DELETE;
                    if(sGrant == sPrivs.copy(nIndex+2,1))
                        nRightsWithGrant |= Privilege::DELETE;
                }
                if(nIndex = sPrivs.indexOf(sUpdate) != -1)
                {
                    nRights |= Privilege::UPDATE;
                    if(sGrant == sPrivs.copy(nIndex+2,1))
                        nRightsWithGrant |= Privilege::UPDATE;
                }
                if(nIndex = sPrivs.indexOf(sAlter) != -1)
                {
                    nRights |= Privilege::ALTER;
                    if(sGrant == sPrivs.copy(nIndex+2,1))
                        nRightsWithGrant |= Privilege::ALTER;
                }
                if(nIndex = sPrivs.indexOf(sSelect) != -1)
                {
                    nRights |= Privilege::SELECT;
                    if(sGrant == sPrivs.copy(nIndex+2,1))
                        nRightsWithGrant |= Privilege::SELECT;
                }
                if(nIndex = sPrivs.indexOf(sReference) != -1)
                {
                    nRights |= Privilege::REFERENCE;
                    if(sGrant == sPrivs.copy(nIndex+2,1))
                        nRightsWithGrant |= Privilege::REFERENCE;
                }
            }
        }
    }
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OAdabasUser::getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);

    sal_Int32 nRights,nRightsWithGrant;
    findPrivilegesAndGrantPrivileges(objName,objType,nRights,nRightsWithGrant);
    return nRightsWithGrant;
}
// -------------------------------------------------------------------------
void SAL_CALL OAdabasUser::grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    ::rtl::OUString sPrivs = getPrivilegeString(objPrivileges);
    if(sPrivs.getLength())
    {
        ::rtl::OUString sGrant;
        sGrant += ::rtl::OUString::createFromAscii("GRANT ");
        sGrant += sPrivs;
        sGrant += ::rtl::OUString::createFromAscii(" ON ");
        Reference<XDatabaseMetaData> xMeta = m_pConnection->getMetaData();
        sGrant += ::dbtools::quoteTableName(xMeta,objName);
        sGrant += ::rtl::OUString::createFromAscii(" TO ");
        sGrant += m_Name;

        Reference<XStatement> xStmt = m_pConnection->createStatement();
        if(xStmt.is())
            xStmt->execute(sGrant);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OAdabasUser::revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);
    ::rtl::OUString sPrivs = getPrivilegeString(objPrivileges);
    if(sPrivs.getLength())
    {
        ::rtl::OUString sGrant;
        sGrant += ::rtl::OUString::createFromAscii("REVOKE ");
        sGrant += sPrivs;
        sGrant += ::rtl::OUString::createFromAscii(" ON ");
        Reference<XDatabaseMetaData> xMeta = m_pConnection->getMetaData();
        sGrant += ::dbtools::quoteTableName(xMeta,objName);
        sGrant += ::rtl::OUString::createFromAscii(" FROM ");
        sGrant += m_Name;

        Reference<XStatement> xStmt = m_pConnection->createStatement();
        if(xStmt.is())
            xStmt->execute(sGrant);
    }
}
// -----------------------------------------------------------------------------
// XUser
void SAL_CALL OAdabasUser::changePassword( const ::rtl::OUString& objPassword, const ::rtl::OUString& newPassword ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE_RBHELPER::rBHelper.bDisposed);
    ::rtl::OUString sAlterPwd;
    sAlterPwd = ::rtl::OUString::createFromAscii("ALTER PASSWORD \"");
    sAlterPwd += objPassword.toAsciiUpperCase();
    sAlterPwd += ::rtl::OUString::createFromAscii("\" TO \"") ;
    sAlterPwd += newPassword.toAsciiUpperCase();
    sAlterPwd += ::rtl::OUString::createFromAscii("\"") ;

    sal_Bool bDisposeConnection = sal_False;
    Reference<XConnection> xConnection = m_pConnection;
    if(m_pConnection->getMetaData()->getUserName() != m_Name)
    {
        OAdabasConnection* pNewConnection = new OAdabasConnection(m_pConnection->getDriverHandle(),m_pConnection->getDriver());
        xConnection = pNewConnection;
        if(pNewConnection)
        {
            Sequence< PropertyValue> aSeq(2);
            aSeq.getArray()[0].Name     = ::rtl::OUString::createFromAscii("user") ;
            aSeq.getArray()[0].Value    <<= m_Name;
            aSeq.getArray()[1].Name     = ::rtl::OUString::createFromAscii("password") ;
            aSeq.getArray()[1].Value    <<= objPassword;
            pNewConnection->Construct(m_pConnection->getMetaData()->getURL(),aSeq);
        }
        bDisposeConnection = sal_True;
    }
    if(xConnection.is())
    {
        Reference<XStatement> xStmt = xConnection->createStatement();
        if(xStmt.is())
            xStmt->execute(sAlterPwd);
        if(bDisposeConnection)
            ::comphelper::disposeComponent(xConnection);
    }
    else
        ::dbtools::throwFunctionSequenceException(*this);
}
// -----------------------------------------------------------------------------
::rtl::OUString OAdabasUser::getPrivilegeString(sal_Int32 nRights) const
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

