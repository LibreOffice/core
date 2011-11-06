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



#ifndef INCLUDED_cppu_helper_purpenv_Mapping_hxx
#define INCLUDED_cppu_helper_purpenv_Mapping_hxx

#include "com/sun/star/uno/Any.h"

#include "uno/environment.h"
#include "uno/mapping.h"


namespace cppu { namespace helper { namespace purpenv {

/** C++ helper for implementing Purpose Environments.
    (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Purpose_Bridge_Implementation_Helper)

    @since UDK 3.2.7
*/

typedef void ProbeFun(
    bool                                pre,
    void                              * pThis,
    void                              * pContext,
    typelib_TypeDescriptionReference  * pReturnTypeRef,
    typelib_MethodParameter           * pParams,
    sal_Int32                           nParams,
    typelib_TypeDescription     const * pMemberType,
    void                              * pReturn,
    void                              * pArgs[],
    uno_Any                          ** ppException );



void createMapping(uno_Mapping     ** ppMapping,
                   uno_Environment  * pFrom,
                   uno_Environment  * pTo,
                   ProbeFun         * probeFun  = NULL,
                   void             * pContext  = NULL
    );

}}}

#endif
