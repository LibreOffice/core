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
#include "cpp_pe.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldcppdocu.hxx>
#include "cpp_tok.hxx"




namespace cpp {

void
Cpp_PE::SetTokenResult( E_TokenDone         i_eDone,
                        E_EnvStackAction    i_eWhat2DoWithEnvStack,
                        ParseEnvironment *  i_pParseEnv2Push )
{
    rMyEnv.SetTokenResult(      i_eDone,
                                i_eWhat2DoWithEnvStack,
                                i_pParseEnv2Push );
}

Cpp_PE::Cpp_PE( Cpp_PE * io_pParent )
    :   ParseEnvironment( io_pParent ),
        rMyEnv( io_pParent->Env() )
{
    csv_assert(io_pParent != 0);
}

Cpp_PE::Cpp_PE( EnvData & i_rEnv )
    :   ParseEnvironment(0),
        rMyEnv(i_rEnv)
{
}

void
Cpp_PE::StdHandlingOfSyntaxError( const char * )
{
    SetTokenResult(not_done, pop_failure);
}


Cpp_PE *
Cpp_PE::Handle_ChildFailure()
{
     return 0;
}

}   // namespace cpp

