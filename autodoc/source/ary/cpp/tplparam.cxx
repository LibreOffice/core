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
#include "tplparam.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_type.hxx>


namespace ary
{
namespace cpp
{
namespace ut
{

TplParameter_Type::TplParameter_Type( Tid i_nType )
    :   nType(i_nType)
{
}

TplParameter_Type::~TplParameter_Type()
{
}

intt
TplParameter_Type::Compare( const TemplateParameter & i_rOther ) const
{
    const TplParameter_Type * pOther
            = dynamic_cast< const TplParameter_Type* >( &i_rOther );
    if (pOther == 0)
        return -1;

    return static_cast<long>(nType.Value())
           - static_cast<long>(pOther->nType.Value());
}

void
TplParameter_Type::Get_Text( StreamStr &                    o_rOut,
                             const ary::cpp::Gate & i_rGate ) const
{
    i_rGate.Types().Get_TypeText( o_rOut, nType );
}

}   // namespace ut
}   // namespace cpp
}   // namespace ary
