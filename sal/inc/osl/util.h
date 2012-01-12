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




#ifndef _OSL_UTIL_H_
#define _OSL_UTIL_H_

#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    @param pEthernetAddr 6 bytes of memory

    @return sal_True  if the ethernetaddress could be retrieved. <br>
            sal_False if no address could be found. This may be either because
           there is no ethernet card or there is no appropriate algorithm
           implemented on the platform. In this case, pEthernetAddr is
           unchanged.
*/
sal_Bool SAL_CALL osl_getEthernetAddress( sal_uInt8 *pEthernetAddr );

#ifdef __cplusplus
}
#endif

#endif
