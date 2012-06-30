/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "rtl/ustring.hxx"
#include "uno/environment.h"
#include "env_subst.hxx"


CPPU_DLLPUBLIC void SAL_CALL uno_getEnvironment(uno_Environment ** ppEnv,
                                 rtl_uString      * pEnvDcp,
                                 void             * pContext)
    SAL_THROW_EXTERN_C()
{
    rtl::OUString envDcp(pEnvDcp);

    rtl::OString  a_envName("UNO_ENV_SUBST:");
    a_envName += rtl::OUStringToOString(envDcp, RTL_TEXTENCODING_ASCII_US);
    char * c_value = getenv(a_envName.getStr());
    if (c_value && *c_value)
    {
        rtl::OString a_envDcp(a_envName.copy(a_envName.indexOf(':') + 1));

        OSL_TRACE("UNO_ENV_SUBST \"%s\" -> \"%s\"", a_envDcp.getStr(), c_value);

        envDcp = rtl::OUString::createFromAscii(c_value);
    }

    uno_direct_getEnvironment(ppEnv, envDcp.pData, pContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
