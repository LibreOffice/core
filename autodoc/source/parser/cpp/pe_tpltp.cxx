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
#include "pe_tpltp.hxx"


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/tpltools.hxx>



namespace cpp {



PE_TemplateTop::PE_TemplateTop( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_TemplateTop> ),
        // aResult_Parameters,
        bCurIsConstant(false)
{
        Setup_StatusFunctions();
}


PE_TemplateTop::~PE_TemplateTop()
{
}

void
PE_TemplateTop::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_TemplateTop::Setup_StatusFunctions()
{
    typedef CallFunction<PE_TemplateTop>::F_Tok F_Tok;

    static F_Tok stateF_start[] =           { &PE_TemplateTop::On_start_Less };
    static INT16 stateT_start[] =           { Tid_Less };

    static F_Tok stateF_expect_qualifier[]= { &PE_TemplateTop::On_expect_qualifier_ClassOrTypename,
                                              &PE_TemplateTop::On_expect_qualifier_Greater,
                                              &PE_TemplateTop::On_expect_qualifier_ClassOrTypename };
    static INT16 stateT_expect_qualifier[]= { Tid_class,
                                              Tid_Greater,
                                              Tid_typename };

    static F_Tok stateF_expect_name[] =     { &PE_TemplateTop::On_expect_name_Identifier };
    static INT16 stateT_expect_name[] =     { Tid_Identifier };

    static F_Tok stateF_expect_separator[]= { &PE_TemplateTop::On_expect_separator_Comma,
                                              &PE_TemplateTop::On_expect_separator_Greater };
    static INT16 stateT_expect_separator[]= { Tid_Comma,
                                              Tid_Greater };

    SEMPARSE_CREATE_STATUS(PE_TemplateTop, start,            Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_TemplateTop, expect_qualifier, On_expect_qualifier_Other);
    SEMPARSE_CREATE_STATUS(PE_TemplateTop, expect_name,      Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_TemplateTop, expect_separator, Hdl_SyntaxError);
}

void
PE_TemplateTop::InitData()
{
    pStati->SetCur(start);
    csv::erase_container(aResult_Parameters);
    bCurIsConstant = false;
}

void
PE_TemplateTop::TransferData()
{
    pStati->SetCur(size_of_states);
}

void
PE_TemplateTop::Hdl_SyntaxError(const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_TemplateTop::On_start_Less( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_qualifier);
}

void
PE_TemplateTop::On_expect_qualifier_ClassOrTypename( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_name);
}

void
PE_TemplateTop::On_expect_qualifier_Greater(const char *)
{
    SetTokenResult(done, pop_success);
}

void
PE_TemplateTop::On_expect_qualifier_Other( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_name);

    bCurIsConstant = true;
}

void
PE_TemplateTop::On_expect_name_Identifier( const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_separator);

    StreamLock sl(50);
    if ( NOT bCurIsConstant )
    {
        String sText( sl() << "typename " << i_sText << c_str );
        aResult_Parameters.push_back(sText);
    }
    else //
    {
        String sText( sl() << "constant " << i_sText << c_str );
        aResult_Parameters.push_back(sText);
        bCurIsConstant = false;
    }  // endif
}

void
PE_TemplateTop::On_expect_separator_Comma( const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(expect_qualifier);
}

void
PE_TemplateTop::On_expect_separator_Greater( const char *)
{
    SetTokenResult(done, pop_success);
}




}   // namespace cpp
