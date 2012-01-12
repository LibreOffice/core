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
#include <ary/idl/i_enumvalue.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/ik_enumvalue.hxx>


namespace ary
{
namespace idl
{

EnumValue::EnumValue( const String &      i_sName,
                      Ce_id               i_nOwner,
                      Ce_id               i_nNameRoom,
                      const String &      i_sInitValue )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nNameRoom(i_nNameRoom),
        sValue(i_sInitValue)
{
}

EnumValue::~EnumValue()
{
}

void
EnumValue::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
EnumValue::get_AryClass() const
{
    return class_id;
}

const String &
EnumValue::inq_LocalName() const
{
    return sName;
}

Ce_id
EnumValue::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
EnumValue::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
EnumValue::inq_SightLevel() const
{
    return sl_Member;
}



namespace ifc_enumvalue
{

inline const EnumValue &
enumvalue_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == EnumValue::class_id );
    return static_cast< const EnumValue& >(i_ce);
}

const String &
attr::Value( const CodeEntity & i_ce )
{
    return enumvalue_cast(i_ce).sValue;
}


} // namespace ifc_enumvalue


}   //  namespace   idl
}   //  namespace   ary
