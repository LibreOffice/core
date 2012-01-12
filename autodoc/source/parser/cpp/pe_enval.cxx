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
#include "pe_enval.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>
#include "pe_expr.hxx"



namespace cpp {


PE_EnumValue::PE_EnumValue( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_EnumValue> )
        // pSpExpression,
        // pSpuInitExpression
{
    Setup_StatusFunctions();

    pSpExpression       = new SP_Expression(*this);
    pSpuInitExpression  = new SPU_Expression(*pSpExpression, 0, &PE_EnumValue::SpReturn_InitExpression);
}

PE_EnumValue::~PE_EnumValue()
{
}

void
PE_EnumValue::Call_Handler( const cpp::Token &  i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_EnumValue::Setup_StatusFunctions()
{
    typedef CallFunction<PE_EnumValue>::F_Tok   F_Tok;

    static F_Tok stateF_start[] =           { &PE_EnumValue::On_start_Identifier };
    static INT16 stateT_start[] =           { Tid_Identifier };

    static F_Tok stateF_afterName[] =       { &PE_EnumValue::On_afterName_SwBracket_Right,
                                              &PE_EnumValue::On_afterName_Comma,
                                              &PE_EnumValue::On_afterName_Assign };
    static INT16 stateT_afterName[] =       { Tid_SwBracket_Right,
                                              Tid_Comma,
                                              Tid_Assign };

    static F_Tok stateF_expectFinish[] =    { &PE_EnumValue::On_expectFinish_SwBracket_Right,
                                              &PE_EnumValue::On_expectFinish_Comma };
    static INT16 stateT_expectFinish[] =    { Tid_SwBracket_Right,
                                              Tid_Comma };

    SEMPARSE_CREATE_STATUS(PE_EnumValue, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_EnumValue, afterName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_EnumValue, expectFinish, Hdl_SyntaxError);
}

void
PE_EnumValue::InitData()
{
    pStati->SetCur(start);

    sName.clear();
    sInitExpression.clear();
}

void
PE_EnumValue::TransferData()
{
    pStati->SetCur(size_of_states);

    ary::cpp::EnumValue &
    rEnVal = Env().AryGate().Ces().Store_EnumValue(
                     Env().Context(), sName, sInitExpression );
    Env().Event_Store_EnumValue(rEnVal);
}

void
PE_EnumValue::Hdl_SyntaxError( const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_EnumValue::SpReturn_InitExpression()
{
    pStati->SetCur(expectFinish);

    sInitExpression = pSpuInitExpression->Child().Result_Text();
}

void
PE_EnumValue::On_start_Identifier(const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(afterName);

    sName = i_sText;
}

void
PE_EnumValue::On_afterName_SwBracket_Right(const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_EnumValue::On_afterName_Comma(const char * )
{
    SetTokenResult(done, pop_success);
}

void
PE_EnumValue::On_afterName_Assign(const char * )
{
    pSpuInitExpression->Push(done);
}

void
PE_EnumValue::On_expectFinish_SwBracket_Right(const char * )
{
    SetTokenResult(not_done, pop_success);
}

void
PE_EnumValue::On_expectFinish_Comma(const char * )
{
    SetTokenResult(done, pop_success);
}


}   // namespace cpp




