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


#include <vos/security.hxx>
#include <osl/diagnose.h>

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
    OSL_ASSERT(m_oslSecurity);

    return osl_getUserIdent( m_oslSecurity, &strIdent.pData );
}


sal_Bool OSecurity::getUserName( rtl::OUString& strName ) const
{
    OSL_ASSERT(m_oslSecurity);

    return osl_getUserName( m_oslSecurity, &strName.pData );
}


sal_Bool OSecurity::getHomeDir( rtl::OUString& strDirectory) const
{
    OSL_ASSERT(m_oslSecurity);

    return osl_getHomeDir(m_oslSecurity, &strDirectory.pData );
}


sal_Bool OSecurity::getConfigDir( rtl::OUString& strDirectory ) const
{
    OSL_ASSERT(m_oslSecurity);

    return osl_getConfigDir( m_oslSecurity, &strDirectory.pData );
}

sal_Bool OSecurity::isAdministrator() const
{
    OSL_ASSERT(m_oslSecurity);

    return osl_isAdministrator(m_oslSecurity);
}

OSecurity::operator oslSecurity() const
{
    return m_oslSecurity;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
