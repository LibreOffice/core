/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: env_subst.cxx,v $
 * $Revision: 1.3 $
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
