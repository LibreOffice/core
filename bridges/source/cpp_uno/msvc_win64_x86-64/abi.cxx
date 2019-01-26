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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include "abi.hxx"

#include <rtl/ustring.hxx>

using namespace x86_64;

bool x86_64::return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef )
{
    switch ( pTypeRef->eTypeClass )
    {
        case typelib_TypeClass_VOID:
            return false;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_ENUM:
            return false;
        case typelib_TypeClass_FLOAT:
            return false;
        case typelib_TypeClass_DOUBLE:
            return false;
        case typelib_TypeClass_STRING:
        case typelib_TypeClass_TYPE:
        case typelib_TypeClass_ANY:
        case typelib_TypeClass_TYPEDEF:
        case typelib_TypeClass_UNION:
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_ARRAY:
        case typelib_TypeClass_INTERFACE:
            return true;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            {
                typelib_TypeDescription * pTypeDescr = 0;
                TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

                /* If the struct is larger than 8 bytes, pass it on the stack.  */
                if ( pTypeDescr->nSize > 8 )
                {
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                    return false;
                }
                else
                {
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                    return true;
                }
            }

        default:
#if OSL_DEBUG_LEVEL > 1
            OSL_TRACE( "Unhandled case: pType->eTypeClass == %d\n", pTypeRef->eTypeClass );
#endif
            OSL_ASSERT(0);
    }
    return 0; /* Never reached.  */
}

