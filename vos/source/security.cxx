/*************************************************************************
 *
 *  $RCSfile: security.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:17 $
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

