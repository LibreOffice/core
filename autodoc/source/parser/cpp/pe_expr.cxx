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
#include "pe_expr.hxx"


// NOT FULLY DECLARED SERVICES


namespace cpp {



PE_Expression::PE_Expression( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Expression> ),
        aResult_Text(100),
        nBracketCounter(0)
{
    Setup_StatusFunctions();
}


PE_Expression::~PE_Expression()
{
}

void
PE_Expression::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());

#if 0
    switch (i_rTok.TypeId())
    {
        case Tid_SwBracket_Left:    SetTokenResult(done, stay);
                                    nBracketCounter++;
                                    bBlockOpened = true;
                                    break;
        case Tid_SwBracket_Right:   SetTokenResult(done, stay);
                                    nBracketCounter--;
                                    break;
        case Tid_Semicolon:         if (nBracketCounter == 0)
                                        SetTokenResult(done, pop_success);
                                    else
                                        SetTokenResult(done, stay);
                                    break;
        default:
                                    if  ( bBlockOpened AND nBracketCounter == 0 )
                                    {
                                        SetTokenResult(not_done, pop_success);
                                    }
                                    else
                                    {
                                        SetTokenResult(done, stay);
                                    }
    }   // end switch
#endif // 0
}

void
PE_Expression::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Expression>::F_Tok  F_Tok;

    static F_Tok stateF_std[] =             { &PE_Expression::On_std_SwBracket_Left,
                                              &PE_Expression::On_std_SwBracket_Right,
                                              &PE_Expression::On_std_ArrayBracket_Left,
                                              &PE_Expression::On_std_ArrayBracket_Right,
                                              &PE_Expression::On_std_Bracket_Left,
                                              &PE_Expression::On_std_Bracket_Right,
                                              &PE_Expression::On_std_Semicolon,
                                              &PE_Expression::On_std_Comma };
    static INT16 stateT_std[] =             { Tid_SwBracket_Left,
                                              Tid_SwBracket_Right,
                                              Tid_ArrayBracket_Left,
                                              Tid_ArrayBracket_Right,
                                              Tid_Bracket_Left,
                                              Tid_Bracket_Right,
                                              Tid_Semicolon,
                                              Tid_Comma };

    SEMPARSE_CREATE_STATUS(PE_Expression, std, On_std_Default);
}

void
PE_Expression::InitData()
{
    pStati->SetCur(std);
    aResult_Text.seekp(0);
    nBracketCounter = 0;
}

void
PE_Expression::TransferData()
{
    pStati->SetCur(size_of_states);
    if ( aResult_Text.tellp() > 0)
        aResult_Text.pop_back(1);
}

void
PE_Expression::On_std_Default( const char * i_sText)
{
    SetTokenResult(done, stay);
    aResult_Text << i_sText << " ";
}

void
PE_Expression::On_std_SwBracket_Left( const char *)
{
    SetTokenResult(done, stay);
    nBracketCounter++;
}

void
PE_Expression::On_std_SwBracket_Right( const char *)
{
    nBracketCounter--;
    if ( nBracketCounter >= 0 )
        SetTokenResult(done, stay);
    else
        SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_ArrayBracket_Left( const char *)
{
    SetTokenResult(done, stay);
    nBracketCounter++;
}

void
PE_Expression::On_std_ArrayBracket_Right( const char *)
{
    nBracketCounter--;
    if ( nBracketCounter >= 0 )
        SetTokenResult(done, stay);
    else
        SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_Bracket_Left( const char *)
{
    SetTokenResult(done, stay);
    nBracketCounter++;
}

void
PE_Expression::On_std_Bracket_Right( const char *)
{
    nBracketCounter--;
    if ( nBracketCounter >= 0 )
        SetTokenResult(done, stay);
    else
        SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_Semicolon( const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_Expression::On_std_Comma( const char *)
{
    SetTokenResult(not_done, pop_success);
}


}   // namespace cpp






