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
    typedef CallFunction<PE_Namespace>::F_Tok	F_Tok;
    static F_Tok stateF_start[] =			{ &PE_Namespace::On_start_Identifier,
                                              &PE_Namespace::On_start_SwBracket_Left };
    static INT16 stateT_start[] =      		{ Tid_Identifier,
                                              Tid_SwBracket_Left };
    static F_Tok stateF_gotName[] =			{ &PE_Namespace::On_gotName_SwBracket_Left,
                                              &PE_Namespace::On_gotName_Assign };
    static INT16 stateT_gotName[] =      	{ Tid_SwBracket_Left,
                                              Tid_Assign };
    static F_Tok stateF_expectSemicolon[] =	{ &PE_Namespace::On_expectSemicolon_Semicolon };
    static INT16 stateT_expectSemicolon[] = { Tid_Semicolon };

    SEMPARSE_CREATE_STATUS(PE_Namespace, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Namespace, gotName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Namespace, expectSemicolon, Hdl_SyntaxError);
}

void
PE_Namespace::Call_Handler( const cpp::Token &	i_rTok )
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




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
