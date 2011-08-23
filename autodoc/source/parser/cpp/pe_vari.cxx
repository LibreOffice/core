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
#include "pe_vari.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include "pe_expr.hxx"




namespace cpp {


PE_Variable::PE_Variable( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Variable> )
        // pSpExpression,
        // pSpuArraySizeExpression,
        // pSpuInitExpression,
        // sResultSizeExpression,
        // sResultInitExpression
{
    Setup_StatusFunctions();

    pSpExpression       = new SP_Expression(*this);

    pSpuArraySizeExpression  = new SPU_Expression(*pSpExpression, 0, &PE_Variable::SpReturn_ArraySizeExpression);
    pSpuInitExpression  = new SPU_Expression(*pSpExpression, 0, &PE_Variable::SpReturn_InitExpression);
}

PE_Variable::~PE_Variable()
{
}

void
PE_Variable::Call_Handler( const cpp::Token &	i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Variable::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Variable>::F_Tok	F_Tok;

    static F_Tok stateF_afterName[] =		{ &PE_Variable::On_afterName_ArrayBracket_Left,
                                              &PE_Variable::On_afterName_Semicolon,
                                              &PE_Variable::On_afterName_Comma,
                                              &PE_Variable::On_afterName_Assign };
    static INT16 stateT_afterName[] =     	{ Tid_ArrayBracket_Left,
                                              Tid_Semicolon,
                                              Tid_Comma,
                                              Tid_Assign };
    static F_Tok stateF_afterSize[] =		{ &PE_Variable::On_afterSize_ArrayBracket_Right };
    static INT16 stateT_afterSize[] =		{ Tid_ArrayBracket_Right };
    static F_Tok stateF_expectFinish[] =	{ &PE_Variable::On_expectFinish_Bracket_Right,
                                              &PE_Variable::On_expectFinish_Semicolon,
                                              &PE_Variable::On_expectFinish_Comma };
    static INT16 stateT_expectFinish[] =   	{ Tid_Bracket_Right,
                                              Tid_Semicolon,
                                              Tid_Comma };

    SEMPARSE_CREATE_STATUS(PE_Variable, afterName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Variable, afterSize, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Variable, expectFinish, Hdl_SyntaxError);
}

void
PE_Variable::InitData()
{
    pStati->SetCur(afterName);

    sResultSizeExpression.clear();
    sResultInitExpression.clear();
}

void
PE_Variable::TransferData()
{
    pStati->SetCur(size_of_states);
}

void
PE_Variable::Hdl_SyntaxError( const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_Variable::SpReturn_ArraySizeExpression()
{
    pStati->SetCur(afterSize);

    sResultSizeExpression = pSpuArraySizeExpression->Child().Result_Text();
}

void
PE_Variable::SpReturn_InitExpression()
{
    pStati->SetCur(expectFinish);

    sResultInitExpression = pSpuInitExpression->Child().Result_Text();
}

void
PE_Variable::On_afterName_ArrayBracket_Left(const char *)
{
    pSpuArraySizeExpression->Push(done);
}

void
PE_Variable::On_afterName_Semicolon(const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_Variable::On_afterName_Comma(const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_Variable::On_afterName_Assign(const char *)
{
    pSpuInitExpression->Push(done);
}

void
PE_Variable::On_afterSize_ArrayBracket_Right(const char *)
{
    SetTokenResult(done, stay);
    pStati->SetCur(afterName);
}

void
PE_Variable::On_expectFinish_Semicolon(const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_Variable::On_expectFinish_Comma(const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_Variable::On_expectFinish_Bracket_Right(const char *)
{
    SetTokenResult(not_done, pop_success);
}


}   // namespace cpp




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
