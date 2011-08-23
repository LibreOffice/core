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
PE_EnumValue::Call_Handler( const cpp::Token &	i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_EnumValue::Setup_StatusFunctions()
{
    typedef CallFunction<PE_EnumValue>::F_Tok	F_Tok;

    static F_Tok stateF_start[] =			{ &PE_EnumValue::On_start_Identifier };
    static INT16 stateT_start[] =			{ Tid_Identifier };

    static F_Tok stateF_afterName[] =		{ &PE_EnumValue::On_afterName_SwBracket_Right,
                                              &PE_EnumValue::On_afterName_Comma,
                                              &PE_EnumValue::On_afterName_Assign };
    static INT16 stateT_afterName[] =     	{ Tid_SwBracket_Right,
                                              Tid_Comma,
                                              Tid_Assign };

    static F_Tok stateF_expectFinish[] =	{ &PE_EnumValue::On_expectFinish_SwBracket_Right,
                                              &PE_EnumValue::On_expectFinish_Comma };
    static INT16 stateT_expectFinish[] =   	{ Tid_SwBracket_Right,
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




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
