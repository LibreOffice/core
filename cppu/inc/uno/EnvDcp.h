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



#ifndef INCLUDED_uno_EnvDcp_h
#define INCLUDED_uno_EnvDcp_h

#include "rtl/ustring.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** Get the OBI type part of an environment descriptor.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Descriptor)

    @param pEnvDcp        the Environment Descriptor
    @param ppEnvTypeName  the OBI type
    @since UDK 3.2.7
*/
void uno_EnvDcp_getTypeName(rtl_uString const * pEnvDcp, rtl_uString ** ppEnvTypeName);


/** Get the purpose part of an environment descriptor.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Descriptor)

    @param pEnvDcp        the Environment Descriptor
    @param ppEnvPurpose   the purpose
    @since UDK 3.2.7
*/
void uno_EnvDcp_getPurpose (rtl_uString const * pEnvDcp, rtl_uString ** ppEnvPurpose);


#ifdef __cplusplus
}
#endif


#endif
