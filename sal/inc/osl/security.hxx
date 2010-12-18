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

#ifndef _OSL_SECURITY_HXX_
#define _OSL_SECURITY_HXX_

#include <rtl/ustring.hxx>

#ifndef _OSL_SECURITY_DECL_HXX
#   include <osl/security_decl.hxx>
#endif

namespace osl
{

inline Security::Security()
{
    m_handle = osl_getCurrentSecurity();
}

inline Security::~Security()
{
    osl_freeSecurityHandle(m_handle);
}

inline sal_Bool Security::logonUser(const rtl::OUString& strName,
                                    const rtl::OUString& strPasswd)
{
    osl_freeSecurityHandle(m_handle);

    m_handle = 0;

    return (osl_loginUser( strName.pData, strPasswd.pData, &m_handle)
            == osl_Security_E_None);
}

inline sal_Bool Security::logonUser( const rtl::OUString& strName,
                                      const rtl::OUString& strPasswd,
                                      const rtl::OUString& strFileServer )
{
    osl_freeSecurityHandle(m_handle);

    m_handle = NULL;

    return (osl_loginUserOnFileServer(strName.pData, strPasswd.pData, strFileServer.pData, &m_handle)
            == osl_Security_E_None);
}

inline sal_Bool Security::getUserIdent( rtl::OUString& strIdent) const
{
    return osl_getUserIdent( m_handle, &strIdent.pData );
}


inline sal_Bool Security::getUserName( rtl::OUString& strName ) const
{
    return osl_getUserName( m_handle, &strName.pData );
}


inline sal_Bool Security::getHomeDir( rtl::OUString& strDirectory) const
{
    return osl_getHomeDir(m_handle, &strDirectory.pData );
}


inline sal_Bool Security::getConfigDir( rtl::OUString& strDirectory ) const
{
    return osl_getConfigDir( m_handle, &strDirectory.pData );
}

inline sal_Bool Security::isAdministrator() const
{
    return osl_isAdministrator(m_handle);
}

inline oslSecurity Security::getHandle() const
{
    return m_handle;
}


}

#endif  // _OSL_SECURITY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
