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




#include <vos/security.hxx>
#include <vos/diagnose.hxx>

using namespace vos;

/////////////////////////////////////////////////////////////////////////////
// Object super class

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OSecurity, vos), VOS_NAMESPACE(OSecurity, vos), VOS_NAMESPACE(OObject, vos), 0);

OSecurity::OSecurity()
{
    m_oslSecurity = osl_getCurrentSecurity();
}

OSecurity::~OSecurity()
{
    osl_freeSecurityHandle(m_oslSecurity);
}

sal_Bool OSecurity::logonUser(const rtl::OUString& strName,
                              const rtl::OUString& strPasswd)
{
    osl_freeSecurityHandle(m_oslSecurity);

    m_oslSecurity = NULL;

    return (osl_loginUser( strName.pData, strPasswd.pData, &m_oslSecurity)
            == osl_Security_E_None);
}


sal_Bool OSecurity::logonUser( const rtl::OUString& strName,
                               const rtl::OUString& strPasswd,
                               const rtl::OUString& strFileServer )
{
    osl_freeSecurityHandle(m_oslSecurity);

    m_oslSecurity = NULL;

    return (osl_loginUserOnFileServer(strName.pData, strPasswd.pData, strFileServer.pData, &m_oslSecurity)
            == osl_Security_E_None);
}


sal_Bool OSecurity::getUserIdent( rtl::OUString& strIdent) const
{
    VOS_ASSERT(m_oslSecurity);

    return osl_getUserIdent( m_oslSecurity, &strIdent.pData );
}


sal_Bool OSecurity::getUserName( rtl::OUString& strName ) const
{
    VOS_ASSERT(m_oslSecurity);

    return osl_getUserName( m_oslSecurity, &strName.pData );
}


sal_Bool OSecurity::getHomeDir( rtl::OUString& strDirectory) const
{
    VOS_ASSERT(m_oslSecurity);

    return osl_getHomeDir(m_oslSecurity, &strDirectory.pData );
}


sal_Bool OSecurity::getConfigDir( rtl::OUString& strDirectory ) const
{
    VOS_ASSERT(m_oslSecurity);

    return osl_getConfigDir( m_oslSecurity, &strDirectory.pData );
}

sal_Bool OSecurity::isAdministrator() const
{
    VOS_ASSERT(m_oslSecurity);

    return osl_isAdministrator(m_oslSecurity);
}

OSecurity::operator oslSecurity() const
{
    return m_oslSecurity;
}

