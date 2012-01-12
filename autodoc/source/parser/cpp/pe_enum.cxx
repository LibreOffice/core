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
#include "pe_enum.hxx"


// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <all_toks.hxx>
#include "pe_enval.hxx"


namespace cpp {


PE_Enum::PE_Enum(Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Enum> ),
        // pSpValue,
        // pSpuValue,
        // sLocalName,
        pCurObject(0),
        eResult_KindOf(is_declaration)
{
    Setup_StatusFunctions();

    pSpValue    = new SP_EnumValue(*this);
    pSpuValue   = new SPU_EnumValue(*pSpValue, 0, 0);
}


PE_Enum::~PE_Enum()
{
}

void
PE_Enum::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Enum::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Enum>::F_Tok    F_Tok;
    static F_Tok stateF_expectName[] =      { &PE_Enum::On_expectName_Identifier,
                                              &PE_Enum::On_expectName_SwBracket_Left
                                            };
    static INT16 stateT_expectName[] =      { Tid_Identifier,
                                              Tid_SwBracket_Left
                                            };

    static F_Tok stateF_gotName[] =         { &PE_Enum::On_gotName_SwBracket_Left };
    static INT16 stateT_gotName[] =         { Tid_SwBracket_Left };

    static F_Tok stateF_bodyStd[] =         { &PE_Enum::On_bodyStd_Identifier,
                                              &PE_Enum::On_bodyStd_SwBracket_Right };
    static INT16 stateT_bodyStd[] =         { Tid_Identifier,
                                              Tid_SwBracket_Right };

    static F_Tok stateF_afterBlock[] =      { &PE_Enum::On_afterBlock_Semicolon };
    static INT16 stateT_afterBlock[] =      { Tid_Semicolon };

    SEMPARSE_CREATE_STATUS(PE_Enum, expectName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Enum, gotName, On_gotName_Return2Type);
    SEMPARSE_CREATE_STATUS(PE_Enum, bodyStd, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Enum, afterBlock, On_afterBlock_Return2Type);
}

void
PE_Enum::InitData()
{
    pStati->SetCur(expectName);
    pCurObject = 0;
    sLocalName.clear();
    eResult_KindOf = is_declaration;
}

void
PE_Enum::TransferData()
{
    pStati->SetCur(size_of_states);
}

void
PE_Enum::Hdl_SyntaxError( const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_Enum::On_expectName_Identifier( const char * i_sText )
{
    SetTokenResult(done, stay);
    pStati->SetCur(gotName);

    sLocalName = i_sText;
    pCurObject = & Env().AryGate().Ces().Store_Enum( Env().Context(), sLocalName );
}

void
PE_Enum::On_expectName_SwBracket_Left( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(bodyStd);

    sLocalName = "";
    pCurObject = & Env().AryGate().Ces().Store_Enum( Env().Context(), sLocalName );
    sLocalName = pCurObject->LocalName();

    Env().OpenEnum(*pCurObject);
}

void
PE_Enum::On_gotName_SwBracket_Left( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(bodyStd);
    Env().OpenEnum(*pCurObject);
}

void
PE_Enum::On_gotName_Return2Type( const char * )
{
    SetTokenResult(not_done, pop_success);

    eResult_KindOf = is_qualified_typename;
}

void
PE_Enum::On_bodyStd_Identifier( const char * )
{
    pSpuValue->Push(not_done);
}

void
PE_Enum::On_bodyStd_SwBracket_Right( const char * )
{
    SetTokenResult(done, stay);
    pStati->SetCur(afterBlock);

    Env().CloseEnum();
}

void
PE_Enum::On_afterBlock_Semicolon( const char * )
{
    SetTokenResult(not_done, pop_success);
    eResult_KindOf = is_declaration;
}

void
PE_Enum::On_afterBlock_Return2Type( const char * )
{
    SetTokenResult(not_done, pop_success);
    eResult_KindOf = is_implicit_declaration;
}

}   // namespace cpp



