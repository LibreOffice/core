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



#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_TYPES_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_TYPES_HXX

#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace bridges { namespace cpp_uno { namespace shared {

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param typeClass a type class
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeClass typeClass);

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param type a non-null pointer to a type description reference
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeDescriptionReference const * type);

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param type a non-null pointer to a type description
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeDescription const * type);

/**
 * Determines whether a type relates to an interface type (is itself an
 * interface type, or might contain entities of interface type).
 *
 * @param type a non-null pointer to a type description
 * @return true if the given type relates to an interface type
 */
bool relatesToInterfaceType(typelib_TypeDescription const * type);

} } }

#endif
