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
#include <ary/idl/i_sisingleton.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/ik_sisingleton.hxx>



namespace ary
{
namespace idl
{

SglIfcSingleton::SglIfcSingleton( const String &      i_sName,
                                  Ce_id               i_nOwner,
                                  Type_id             i_nBaseInterface )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nBaseInterface(i_nBaseInterface)
{
}

SglIfcSingleton::~SglIfcSingleton()
{
}

void
SglIfcSingleton::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
SglIfcSingleton::get_AryClass() const
{
    return class_id;
}

const String &
SglIfcSingleton::inq_LocalName() const
{
    return sName;
}

Ce_id
SglIfcSingleton::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
SglIfcSingleton::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
SglIfcSingleton::inq_SightLevel() const
{
    return sl_File;
}

namespace ifc_sglifcsingleton
{

inline const SglIfcSingleton &
sglifcsingleton_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == SglIfcSingleton::class_id );
    return static_cast< const SglIfcSingleton& >(i_ce);
}

Type_id
attr::BaseInterface( const CodeEntity & i_ce )
{
    return sglifcsingleton_cast(i_ce).nBaseInterface;
}


} // namespace ifc_sglifcsingleton


}   //  namespace   idl
}   //  namespace   ary
