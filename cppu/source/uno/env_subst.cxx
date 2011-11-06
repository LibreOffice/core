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
