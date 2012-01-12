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
#include "tkp_cpp.hxx"

// NOT FULLY DECLARED SERVICES
#include "cx_c_std.hxx"
#include "c_dealer.hxx"


namespace cpp {




TokenParser_Cpp::TokenParser_Cpp( DYN autodoc::TkpDocuContext & let_drDocuContext )
    :   pBaseContext( new Context_CppStd( let_drDocuContext ) ),
        pCurContext(0),
        pDealer(0),
        pCharacterSource(0)
{
    SetStartContext();
}

TokenParser_Cpp::~TokenParser_Cpp()
{
}

void
TokenParser_Cpp::AssignPartners( CharacterSource &   io_rCharacterSource,
                                 cpp::Distributor &  o_rDealer )
{
    pDealer = &o_rDealer;
    pBaseContext->AssignDealer(o_rDealer);
    pCharacterSource = &io_rCharacterSource;
}

void
TokenParser_Cpp::StartNewFile( const csv::ploc::Path & i_file )
{
    csv_assert(pDealer != 0);
    pDealer->StartNewFile(i_file);

    csv_assert(pCharacterSource != 0);
    Start(*pCharacterSource);
}

void
TokenParser_Cpp::SetStartContext()
{
    pCurContext = pBaseContext.Ptr();
}

void
TokenParser_Cpp::SetCurrentContext( TkpContext & io_rContext )
{
    pCurContext = &io_rContext;
}

TkpContext &
TokenParser_Cpp::CurrentContext()
{
    return *pCurContext;
}

}   // namespace cpp

