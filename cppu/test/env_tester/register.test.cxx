/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: register.test.cxx,v $
 * $Revision: 1.4 $
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

#include <iostream>

#include "sal/main.h"
#include "rtl/ustring.hxx"
#include "rtl/string.hxx"
#include "rtl/alloc.h"
#include "uno/environment.hxx"
#include "uno/lbnames.h"

using namespace com::sun::star;


static rtl::OUString s_test_getEnvironment(rtl::OUString const & envDcp, void * pContext)
{
    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\ts_test_getEnvironment("));
    result += envDcp;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", "));
    result += rtl::OUString::valueOf((long)pContext);
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")\n"));

    uno::Environment env(envDcp, pContext);

    if (!env.is())
    {
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE: couldn't get env.\n"));
        return result;
    }

    if (rtl::OUString(env.getTypeName()).compareTo(envDcp))
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE: got environment has wrong descriptor.\n"));

    if (env.getContext() != pContext)
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE: got environment has wrong context.\n"));


    return result;
}

static rtl::OUString s_test_regetEnvironment(rtl::OUString const & envDcp1,
                                             rtl::OUString const & envDcp2 ,
                                             void * pContext1, void * pContext2)
{
    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\ts_test_regetEnvironment("));
    result += envDcp1;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", "));
    result += envDcp2;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", "));
    result += rtl::OUString::valueOf((long)pContext1);
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", " ));
    result += rtl::OUString::valueOf((long)pContext2);
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")\n"));

    uno::Environment env1(envDcp1, pContext1);
    uno::Environment env2(envDcp2, pContext2);

    if (! ((pContext1 == pContext2 && envDcp1 == envDcp2) ? env1.get() == env2.get() : env1.get() != env2.get())
        && env1.is() && env2.is())
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE:\n"));

    return result;
}

static rtl::OUString s_test_regetEnvironment(rtl::OUString const & envDcp, void * pContext)
{
    return s_test_regetEnvironment(envDcp, envDcp, pContext, pContext);
}

// static rtl::OUString s_test_regetEnvironment(rtl::OUString const & envDcp, void * pContext1, void * pContext2)
// {
//  return s_test_regetEnvironment(envDcp, envDcp, pContext1, pContext2);
// }

static rtl::OUString s_test_regetEnvironment(rtl::OUString const & envDcp1, rtl::OUString const & envDcp2, void * pContext)
{
    return s_test_regetEnvironment(envDcp1, envDcp2, pContext, pContext);
}


static int s_is_registered(rtl::OUString const & envDcp, void * pContext)
{
    int result = 0;

    uno_Environment ** ppEnvs = NULL;
    sal_Int32          nLen   = 0;

    uno_getRegisteredEnvironments(&ppEnvs, &nLen, rtl_allocateMemory, envDcp.pData);
    for (sal_Int32 i = 0; !result && i < nLen; ++ i)
    {
        result = result || (ppEnvs[i]->pContext == pContext && !rtl::OUString(ppEnvs[i]->pTypeName).compareTo(envDcp));

        ppEnvs[i]->release(ppEnvs[i]);
    }
    rtl_freeMemory(ppEnvs);

    return result;
}

static rtl::OUString s_test_uno_getRegisteredEnvironments_registered(rtl::OUString const & envDcp, void * pContext)
{
    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\ts_test_uno_getRegisteredEnvironments_registered("));
    result += envDcp;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", "));
    result += rtl::OUString::valueOf((long)pContext);
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")\n"));

    uno::Environment env(envDcp, pContext);

    if (!env.is() || !s_is_registered(envDcp, pContext))
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE:\n"));

    return result;
}

static rtl::OUString s_test_uno_getRegisteredEnvironments_notRegistered(rtl::OUString const & envDcp, void * pContext)
{
    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\ts_test_uno_getRegisteredEnvironments_notRegistered("));
    result += envDcp;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", "));
    result += rtl::OUString::valueOf((long)pContext);
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")\n"));

    uno::Environment env(envDcp, pContext);

    if (!env.is() && !s_is_registered(envDcp, pContext))
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE:\n"));

    return result;
}


static rtl::OUString s_test_uno_createEnvironment(rtl::OUString const & envDcp, void * pContext)
{
    rtl::OUString result(RTL_CONSTASCII_USTRINGPARAM("\ts_test_uno_createEnvironment("));
    result += envDcp;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", "));
    result += rtl::OUString::valueOf((long)pContext);
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")\n"));

    uno_Environment * pEnv = NULL;
    uno_createEnvironment(&pEnv, envDcp.pData, pContext);

    if (!pEnv && !s_is_registered(envDcp, pContext))
        result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tFAILURE:\n"));

    if (pEnv)
        pEnv->release(pEnv);

    return result;
}


rtl::OUString register_test(rtl::OUString const & envDcp)
{
    rtl::OUString env_A(envDcp);
    env_A += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(":purpA"));

    rtl::OUString env_B(envDcp);
    env_B += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(":purpB"));

    rtl::OUString result;

    result = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("registration_test"));
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    result += s_test_getEnvironment(envDcp, NULL);
    result += s_test_getEnvironment(env_A,  NULL);
    result += s_test_getEnvironment(env_B,  NULL);

    result += s_test_regetEnvironment(envDcp, NULL);
    result += s_test_regetEnvironment(envDcp, (void *)0x1);
    result += s_test_regetEnvironment(env_A,  NULL)       ;
      result += s_test_regetEnvironment(env_A,  (void *)0x1);
    result += s_test_regetEnvironment(env_B,  NULL)       ;
      result += s_test_regetEnvironment(env_B,  (void *)0x1);

     result += s_test_regetEnvironment(envDcp, env_A, (void *)NULL);
     result += s_test_regetEnvironment(envDcp, env_A, (void *)0x1) ;
     result += s_test_regetEnvironment(envDcp, env_B, (void *)NULL);
     result += s_test_regetEnvironment(envDcp, env_B, (void *)0x1) ;
     result += s_test_regetEnvironment(env_A,  env_B, (void *)NULL);
     result += s_test_regetEnvironment(env_A,  env_B, (void *)0x1) ;

    result += s_test_regetEnvironment(env_A, env_B, (void *)NULL, (void *)0x1);

     result += s_test_uno_getRegisteredEnvironments_registered(envDcp, (void *)NULL);
    result += s_test_uno_getRegisteredEnvironments_registered(envDcp, (void *)0x1) ;
    result += s_test_uno_getRegisteredEnvironments_registered(env_A,  (void *)NULL);
    result += s_test_uno_getRegisteredEnvironments_registered(env_A,  (void *)0x1) ;
    result += s_test_uno_getRegisteredEnvironments_registered(env_B,  (void *)NULL);
    result += s_test_uno_getRegisteredEnvironments_registered(env_B,  (void *)0x1) ;

      result += s_test_uno_getRegisteredEnvironments_notRegistered(envDcp, NULL)       ;
      result += s_test_uno_getRegisteredEnvironments_notRegistered(envDcp, (void *)0x1);
      result += s_test_uno_getRegisteredEnvironments_notRegistered(env_A,  NULL)       ;
      result += s_test_uno_getRegisteredEnvironments_notRegistered(env_A,  (void *)0x1);
      result += s_test_uno_getRegisteredEnvironments_notRegistered(env_B,  NULL)       ;
      result += s_test_uno_getRegisteredEnvironments_notRegistered(env_B,  (void *)0x1);

     result += s_test_uno_createEnvironment(envDcp, NULL)       ;
     result += s_test_uno_createEnvironment(envDcp, (void *)0x1);
     result += s_test_uno_createEnvironment(env_A,  NULL)       ;
     result += s_test_uno_createEnvironment(env_A,  (void *)0x1);
     result += s_test_uno_createEnvironment(env_B,  NULL)       ;
     result += s_test_uno_createEnvironment(env_B,  (void *)0x1);

    return result;
}
