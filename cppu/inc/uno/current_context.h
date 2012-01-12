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


#ifndef _UNO_CURRENT_CONTEXT_H_
#define _UNO_CURRENT_CONTEXT_H_

#include <rtl/ustring.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Gets the current task's context.
    @attention
    Don't spread the returned interface around to other threads.  Every thread has its own
    current context.

    @param ppCurrentContext  inout param current context of type com.sun.star.uno.XCurrentContext
    @param pEnvDcp           descriptor of returned interface's environment
    @param pEnvContext       context of returned interface's environment (commonly 0)
    @return                  true, if context ref was transferred (even if null ref)
*/
sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext,
    rtl_uString * pEnvDcp, void * pEnvContext )
    SAL_THROW_EXTERN_C();

/** Sets the current task's context.

    @param pCurrentContext  in param current context of type com.sun.star.uno.XCurrentContext
    @param pEnvDcp          descriptor of interface's environment
    @param pEnvContext      context of interface's environment (commonly 0)
    @return                 true, if context ref was transferred (even if null ref)
*/
sal_Bool SAL_CALL uno_setCurrentContext(
    void * pCurrentContext,
    rtl_uString * pEnvDcp, void * pEnvContext )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
