/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    typedef CallFunction<PE_TemplateTop>::F_Tok	F_Tok;

    static F_Tok stateF_start[] =		    { &PE_TemplateTop::On_start_Less };
    static INT16 stateT_start[] =           { Tid_Less };

    static F_Tok stateF_expect_qualifier[]= { &PE_TemplateTop::On_expect_qualifier_ClassOrTypename,
                                              &PE_TemplateTop::On_expect_qualifier_Greater,
                                              &PE_TemplateTop::On_expect_qualifier_ClassOrTypename };
    static INT16 stateT_expect_qualifier[]= { Tid_class,
                                              Tid_Greater,
                                              Tid_typename };

    static F_Tok stateF_expect_name[] =	    { &PE_TemplateTop::On_expect_name_Identifier };
    static INT16 stateT_expect_name[] =     { Tid_Identifier };

    static F_Tok stateF_expect_separator[]=	{ &PE_TemplateTop::On_expect_separator_Comma,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
