/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: env_subst.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:38:48 $
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

#include "rtl/ustring.hxx"
#include "uno/environment.h"
#include "env_subst.hxx"


void SAL_CALL uno_getEnvironment(uno_Environment ** ppEnv,
                                 rtl_uString      * pEnvDcp,
                                 void             * pContext)
    SAL_THROW_EXTERN_C()
{
    rtl::OUString envDcp(pEnvDcp);

    rtl::OString  a_envName("UNO_ENV_SUBST:");
    a_envName += rtl::OUStringToOString(envDcp, RTL_TEXTENCODING_ASCII_US);
    char * c_value = getenv(a_envName.getStr());
    if (c_value && rtl_str_getLength(c_value))
    {
        rtl::OString a_envDcp(a_envName.copy(a_envName.indexOf(':') + 1));

        OSL_TRACE("UNO_ENV_SUBST \"%s\" -> \"%s\"", a_envDcp.getStr(), c_value);
        rtl::OUString value(c_value, rtl_str_getLength(c_value), RTL_TEXTENCODING_ASCII_US);

         envDcp = value;
    }

    uno_direct_getEnvironment(ppEnv, envDcp.pData, pContext);
}
