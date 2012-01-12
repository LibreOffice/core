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
#include "pe_tydef.hxx"


// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_type.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <all_toks.hxx>
#include "pe_type.hxx"


namespace cpp {


PE_Typedef::PE_Typedef(Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Typedef> ),
        // pSpType,
        // pSpuType,
        // sName
        nType(0)
{
    Setup_StatusFunctions();

    pSpType    = new SP_Type(*this);
    pSpuType   = new SPU_Type(*pSpType, 0, &PE_Typedef::SpReturn_Type);
}

PE_Typedef::~PE_Typedef()
{
}

void
PE_Typedef::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Typedef::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Typedef>::F_Tok F_Tok;
    static F_Tok stateF_start[] =       { &PE_Typedef::On_start_typedef };
    static INT16 stateT_start[] =       { Tid_typedef };

    static F_Tok stateF_expectName[] =  { &PE_Typedef::On_expectName_Identifier };
    static INT16 stateT_expectName[] =  { Tid_Identifier };

    static F_Tok stateF_afterName[] =   { &PE_Typedef::On_afterName_Semicolon };
    static INT16 stateT_afterName[] =   { Tid_Semicolon };

    SEMPARSE_CREATE_STATUS(PE_Typedef, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Typedef, expectName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Typedef, afterName, Hdl_SyntaxError);
}

void
PE_Typedef::InitData()
{
    pStati->SetCur(start);

    sName.clear();
    nType = 0;
}

void
PE_Typedef::TransferData()
{
    pStati->SetCur(size_of_states);

    ary::cpp::Typedef &
    rTypedef = Env().AryGate().Ces().Store_Typedef(
                        Env().Context(), sName, nType );
    Env().Event_Store_Typedef(rTypedef);
}

void
PE_Typedef::Hdl_SyntaxError( const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_Typedef::SpReturn_Type()
{
    pStati->SetCur(expectName);

    nType = pSpuType->Child().Result_Type().Id();
}

void
PE_Typedef::On_start_typedef( const char * )
{
    pSpuType->Push(done);
}

void
PE_Typedef::On_expectName_Identifier( const char * i_sText )
{
    SetTokenResult(done, stay);
    pStati->SetCur(afterName);

    sName = i_sText;
}

void
PE_Typedef::On_afterName_Semicolon( const char * )
{
    SetTokenResult(done, pop_success);
}

}   // namespace cpp



