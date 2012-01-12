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
#include "it_sequence.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_type.hxx>



namespace ary
{
namespace idl
{


Sequence::Sequence( Type_id i_nRelatedType )
    :   nRelatedType(i_nRelatedType)
{
}

Sequence::~Sequence()
{
}

ClassId
Sequence::get_AryClass() const
{
    return class_id;
}

void
Sequence::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
Sequence::inq_Get_Text( StringVector &      o_module,
                        String &            o_name,
                        Ce_id &             o_nRelatedCe,
                        int &               o_nSequenceCount,
                        const Gate &        i_rGate ) const
{
    ++o_nSequenceCount;

    i_rGate.Types().Find_Type(nRelatedType)
            .Get_Text(  o_module,
                        o_name,
                        o_nRelatedCe,
                        o_nSequenceCount,
                        i_rGate );
}

const Type &
Sequence::inq_FirstEnclosedNonSequenceType(const Gate & i_rGate) const
{
    return i_rGate.Types().Find_Type(nRelatedType).FirstEnclosedNonSequenceType(i_rGate);
}


}   // namespace idl
}   // namespace ary
