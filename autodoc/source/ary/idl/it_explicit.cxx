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
#include "it_explicit.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include "it_xnameroom.hxx"



namespace ary
{
namespace idl
{


ExplicitType::ExplicitType( const String &      i_sName,
                            Type_id             i_nXNameRoom,
                            Ce_id               i_nModuleOfOccurrence,
                            const std::vector<Type_id> *
                                                i_templateParameters )
    :   Named_Type(i_sName),
        nXNameRoom(i_nXNameRoom),
        nModuleOfOccurrence(i_nModuleOfOccurrence),
        pTemplateParameters(0)
{
    if (i_templateParameters != 0)
        pTemplateParameters = new std::vector<Type_id>(*i_templateParameters);
}

ExplicitType::~ExplicitType()
{
}

ClassId
ExplicitType::get_AryClass() const
{
    return class_id;
}

void
ExplicitType::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
ExplicitType::inq_Get_Text( StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequenceCount,
                            const Gate &        i_rGate ) const
{
    const ExplicitNameRoom &
        rNameRoom = i_rGate.Types().Find_XNameRoom(nXNameRoom);
    rNameRoom.Get_Text(o_module,o_name,o_nRelatedCe,o_nSequenceCount,i_rGate);

    o_name = Name();
}

const std::vector<Type_id> *
ExplicitType::inq_TemplateParameters() const
{
    return pTemplateParameters.Ptr();
}


}   // namespace idl
}   // namespace ary
