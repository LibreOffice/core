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


#ifndef _COM_SUN_STAR_UNO_GENFUNC_H_
#define _COM_SUN_STAR_UNO_GENFUNC_H_

#include "sal/types.h"

typedef struct _typelib_TypeDescriptionReference typelib_TypeDescriptionReference;

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** Function to acquire a C++ interface.

    @param pCppI C++ interface pointer
*/
inline void SAL_CALL cpp_acquire( void * pCppI )
    SAL_THROW( () );
/** Function to release a C++ interface.

    @param pCppI C++ interface pointer
*/
inline void SAL_CALL cpp_release( void * pCppI )
    SAL_THROW( () );
/** Function to query for a C++ interface.

    @param pCppI C++ interface pointer
    @param pType demanded interface type
    @return acquired C++ interface pointer or null
*/
inline void * SAL_CALL cpp_queryInterface( void * pCppI, typelib_TypeDescriptionReference * pType )
    SAL_THROW( () );

}
}
}
}

#endif
