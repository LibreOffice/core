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
#include "it_xnameroom.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>
#include <cosv/tpl/tpltools.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_type.hxx>



namespace ary
{
namespace idl
{


ExplicitNameRoom::ExplicitNameRoom()
    :   aImpl()
{
}

ExplicitNameRoom::ExplicitNameRoom( const String &           i_sName,
                                    const ExplicitNameRoom & i_rParent )
    :   aImpl( i_sName, i_rParent.aImpl, i_rParent.TypeId() )
{
}

ExplicitNameRoom::~ExplicitNameRoom()
{
}

ClassId
ExplicitNameRoom::get_AryClass() const
{
    return class_id;
}

void
ExplicitNameRoom::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
ExplicitNameRoom::inq_Get_Text( StringVector &      o_module,
                                String &            ,           // o_name
                                Ce_id &             ,           // o_nRelatedCe
                                int &               ,           // o_nSequemceCount
                                const Gate &        ) const     // i_rGate
{
    StringVector::const_iterator it = NameChain_Begin();
    if ( it != NameChain_End()
            ? (*it).empty()
            : false )
    {   // Don't put out the root global namespace
        ++it;
    }

    for ( ;
          it != NameChain_End();
          ++it )
    {
        o_module.push_back(*it);
    }
}




}   // namespace idl
}   // namespace ary
