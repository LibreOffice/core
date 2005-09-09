/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: security.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:09:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include <vos/security.hxx>
#include <vos/diagnose.hxx>

#ifdef _USE_NAMESPACE
using namespace vos;
#endif

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

