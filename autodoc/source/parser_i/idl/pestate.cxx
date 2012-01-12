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
#include <s2_luidl/pestate.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/parsenv2.hxx>




namespace csi
{
namespace uidl
{

void
ParseEnvState::Process_Identifier( const TokIdentifier & )
{
    Process_Default();
}

void
ParseEnvState::Process_NameSeparator()
{
    Process_Default();
}

void
ParseEnvState::Process_Punctuation( const TokPunctuation & )
{
    Process_Default();
}

void
ParseEnvState::Process_BuiltInType( const TokBuiltInType & )
{
    Process_Default();
}

void
ParseEnvState::Process_TypeModifier( const TokTypeModifier & )
{
    Process_Default();
}

void
ParseEnvState::Process_MetaType( const TokMetaType &    )
{
    Process_Default();
}

void
ParseEnvState::Process_Stereotype( const TokStereotype & )
{
    Process_Default();
}

void
ParseEnvState::Process_ParameterHandling( const TokParameterHandling & )
{
    Process_Default();
}

void
ParseEnvState::Process_Raises()
{
    Process_Default();
}

void
ParseEnvState::Process_Needs()
{
    Process_Default();
}

void
ParseEnvState::Process_Observes()
{
    Process_Default();
}

void
ParseEnvState::Process_Assignment( const TokAssignment & )
{
    Process_Default();
}

void
ParseEnvState::Process_EOL()
{
    MyPE().SetResult(done,stay);
}


void
ParseEnvState::On_SubPE_Left()
{
}

void
ParseEnvState::Process_Default()
{
    if (bDefaultIsError)
        MyPE().SetResult(not_done, pop_failure);
    else    // ignore:
        MyPE().SetResult(done, stay);
}


}   // namespace uidl
}   // namespace csi

