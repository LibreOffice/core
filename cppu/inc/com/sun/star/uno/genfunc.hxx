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


#ifndef _COM_SUN_STAR_UNO_GENFUNC_HXX_
#define _COM_SUN_STAR_UNO_GENFUNC_HXX_

#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/uno/Any.hxx>


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

//==================================================================================================
inline void SAL_CALL cpp_acquire( void * pCppI )
    SAL_THROW( () )
{
    reinterpret_cast< XInterface * >( pCppI )->acquire();
}
//==================================================================================================
inline void SAL_CALL cpp_release( void * pCppI )
    SAL_THROW( () )
{
    reinterpret_cast< XInterface * >( pCppI )->release();
}
//==================================================================================================
inline void * SAL_CALL cpp_queryInterface( void * pCppI, typelib_TypeDescriptionReference * pType )
    SAL_THROW( () )
{
    if (pCppI)
    {
#ifndef EXCEPTIONS_OFF
        try
        {
#endif
            Any aRet( reinterpret_cast< XInterface * >( pCppI )->queryInterface(
                * reinterpret_cast< const Type * >( &pType ) ) );
            if (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
            {
                XInterface * pRet = reinterpret_cast< XInterface * >( aRet.pReserved );
                aRet.pReserved = 0;
                return pRet;
            }
#ifndef EXCEPTIONS_OFF
        }
        catch (RuntimeException &)
        {
        }
#endif
    }
    return 0;
}

}
}
}
}

#endif
