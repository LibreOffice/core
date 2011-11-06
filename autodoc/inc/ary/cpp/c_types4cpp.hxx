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



#ifndef ARY_CPP_C_TYPES4CPP_HXX
#define ARY_CPP_C_TYPES4CPP_HXX



// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <cosv/tpl/range.hxx>
#include <ary/types.hxx>



namespace ary
{
namespace cpp
{

class Namespace;
class CodeEntity;
class Type;
class DefineEntity;
class Gate;
class CePilot;
class TypePilot;
class SecondariesPilot;
class Ce_Storage;
class Def_Storage;
class Type_Storage;
class UsedType;


typedef TypedId< ::ary::cpp::CodeEntity >       Ce_id;
typedef TypedId< ::ary::cpp::Type >             Type_id;
typedef TypedId< ::ary::cpp::DefineEntity >     De_id;


typedef std::vector<Ce_id>                      CesList;
typedef std::vector<De_id>                      DefsList;
typedef std::vector<Type_id>                    TypesList;

typedef std::vector<Ce_id>::const_iterator      CesConstIterator;
typedef std::vector<De_id>::const_iterator      DefsConstIterator;
typedef std::vector<Type_id>::const_iterator    TypesConstIterator;

typedef csv::range< CesConstIterator >          CesResultList;
typedef csv::range< DefsConstIterator >         DefsResultList;
typedef csv::range< TypesConstIterator >        TypesResultList;


typedef std::map<String, Ce_id>                 Map_LocalCe;



enum E_Protection
{
    PROTECT_global = 0,
    PROTECT_local,          /// within Functions
    PROTECT_public,
    PROTECT_protected,
    PROTECT_private
};

/** The sequence of E_Virtuality's values must not be changed,
    because they are used in int-comparisons.
*/
enum E_Virtuality
{
    VIRTUAL_none = 0,
    VIRTUAL_virtual,
    VIRTUAL_abstract
};

enum E_ClassKey
{
    CK_class,
    CK_struct,
    CK_union
};

enum E_TypeSpecialisation
{
    TYSP_none = 0,
    TYSP_unsigned,
    TYSP_signed
};

enum E_ConVol
{
    CONVOL_none = 0,
    CONVOL_const = 0x0001,
    CONVOL_volatile = 0x0002,
    CONVOL_both = 0x0003
};



// Backwards compatibility:
typedef Ce_id                           Cid;
typedef Type_id                         Tid;




}   //  namespace cpp
}   //  namespace ary
#endif
