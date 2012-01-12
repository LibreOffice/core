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
#include <pe_namsp.hxx>


// NOT FULLY DECLARED SERVICES
#include <all_toks.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <semantic/callf.hxx>
#include "x_parse.hxx"




namespace cpp
{

PE_Namespace::PE_Namespace( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Namespace> ),
        // sLocalname
        bPush(false)
{
    Setup_StatusFunctions();
}

PE_Namespace::~PE_Namespace()
{
}

void
PE_Namespace::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Namespace>::F_Tok   F_Tok;
    static F_Tok stateF_start[] =           { &PE_Namespace::On_start_Identifier,
                                              &PE_Namespace::On_start_SwBracket_Left };
    static INT16 stateT_start[] =           { Tid_Identifier,
                                              Tid_SwBracket_Left };
    static F_Tok stateF_gotName[] =         { &PE_Namespace::On_gotName_SwBracket_Left,
                                              &PE_Namespace::On_gotName_Assign };
    static INT16 stateT_gotName[] =         { Tid_SwBracket_Left,
                                              Tid_Assign };
    static F_Tok stateF_expectSemicolon[] = { &PE_Namespace::On_expectSemicolon_Semicolon };
    static INT16 stateT_expectSemicolon[] = { Tid_Semicolon };

    SEMPARSE_CREATE_STATUS(PE_Namespace, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Namespace, gotName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Namespace, expectSemicolon, Hdl_SyntaxError);
}

void
PE_Namespace::Call_Handler( const cpp::Token &  i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Namespace::InitData()
{
    pStati->SetCur(start);
    sLocalName = "";
    bPush = false;
}

void
PE_Namespace::TransferData()
{
    if (bPush)
    {
        ary::cpp::Namespace &
            rNew = Env().AryGate().Ces().CheckIn_Namespace(
                                             Env().Context(),
                                             sLocalName );
        Env().OpenNamespace(rNew);
    }
}

void
PE_Namespace::Hdl_SyntaxError( const char * i_sText)
{
    throw X_Parser( X_Parser::x_UnexpectedToken,
                    i_sText != 0 ? i_sText : "",
                    Env().CurFileName(),
                    Env().LineCount() );
}

void
PE_Namespace::On_start_Identifier(const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(gotName);

    sLocalName = i_sText;
}

void
PE_Namespace::On_start_SwBracket_Left(const char * )
{
    SetTokenResult(done, pop_success);
    pStati->SetCur(size_of_states);

    sLocalName = "";    // Anonymous namespace, a name is created in
                        //   Gate().CheckIn_Namespace() .

    bPush = true;
}

void
PE_Namespace::On_gotName_SwBracket_Left(const char * )
{
    SetTokenResult(done, pop_success);
    pStati->SetCur(size_of_states);

    bPush = true;
}

void
PE_Namespace::On_gotName_Assign(const char * )
{
    // KORR_FUTURE
    Hdl_SyntaxError(0);
}

void
PE_Namespace::On_expectSemicolon_Semicolon(const char * )
{
    SetTokenResult(done,pop_success);
    pStati->SetCur(size_of_states);
}

}   // namespace cpp




