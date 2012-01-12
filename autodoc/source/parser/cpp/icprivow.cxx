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

#include <precomp.h>
#include <icprivow.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>



namespace cpp
{



//******************          Owner_Namespace          ********************//
Owner_Namespace::Owner_Namespace()
    :   pScope(0)
{
}

void
Owner_Namespace::SetAnotherNamespace( ary::cpp::Namespace & io_rScope )
{
    pScope = &io_rScope;
}

bool
Owner_Namespace::HasClass( const String & i_sLocalName )
{
    return pScope->Search_LocalClass(i_sLocalName).IsValid();
}

void
Owner_Namespace::do_Add_Class( const String  &     i_sLocalName,
                               Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalClass(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Enum( const String  &     i_sLocalName,
                              Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalEnum(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Typedef( const String  &     i_sLocalName,
                                 Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalTypedef(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Operation( const String  &     i_sLocalName,
                                   Cid                 i_nId,
                                   bool                )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalOperation(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Variable( const String  &     i_sLocalName,
                                  Cid                 i_nId,
                                  bool                i_bIsConst,
                                  bool                )
{
    csv_assert(pScope != 0);
    if (i_bIsConst)
        pScope->Add_LocalConstant(i_sLocalName, i_nId);
    else
        pScope->Add_LocalVariable(i_sLocalName, i_nId);
}


Cid
Owner_Namespace::inq_CeId() const
{
    csv_assert(pScope != 0);
    return pScope->CeId();
}


//******************            Owner_Class          ********************//

Owner_Class::Owner_Class()
    :   pScope(0)
{
}

void
Owner_Class::SetAnotherClass( ary::cpp::Class & io_rScope )
{
    pScope = &io_rScope;
}

bool
Owner_Class::HasClass( const String  & )
{
    return false;
}

void
Owner_Class::do_Add_Class( const String  &     i_sLocalName,
                               Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalClass(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Enum( const String  &     i_sLocalName,
                              Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalEnum(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Typedef( const String  &     i_sLocalName,
                             Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalTypedef(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Operation( const String  &      i_sLocalName,
                               Cid                  i_nId,
                               bool                 i_bIsStatic )
{
    csv_assert(pScope != 0);
    if (i_bIsStatic)
        pScope->Add_LocalStaticOperation(i_sLocalName, i_nId);
    else
        pScope->Add_LocalOperation(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Variable( const String  &     i_sLocalName,
                              Cid                 i_nId,
                              bool                ,
                              bool                i_bIsStatic )
{
    csv_assert(pScope != 0);
    if (i_bIsStatic)
        pScope->Add_LocalStaticData(i_sLocalName, i_nId);
    else
        pScope->Add_LocalData(i_sLocalName, i_nId);
}

Cid
Owner_Class::inq_CeId() const
{
    csv_assert(pScope != 0);
    return pScope->CeId();
}


}   // namespace cpp
