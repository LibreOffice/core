/*************************************************************************
 *
 *  $RCSfile: security.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-14 16:28:31 $
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

#ifndef _OSL_SECURITY_HXX_
#define _OSL_SECURITY_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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

