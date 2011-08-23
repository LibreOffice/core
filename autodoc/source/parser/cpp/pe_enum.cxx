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
    pSpuValue	= new SPU_EnumValue(*pSpValue, 0, 0);
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
    typedef CallFunction<PE_Enum>::F_Tok	F_Tok;
    static F_Tok stateF_expectName[] = 		{ &PE_Enum::On_expectName_Identifier,
                                              &PE_Enum::On_expectName_SwBracket_Left
                                            };
    static INT16 stateT_expectName[] = 		{ Tid_Identifier,
                                              Tid_SwBracket_Left
                                            };

    static F_Tok stateF_gotName[] =			{ &PE_Enum::On_gotName_SwBracket_Left };
    static INT16 stateT_gotName[] =       	{ Tid_SwBracket_Left };

    static F_Tok stateF_bodyStd[] =			{ &PE_Enum::On_bodyStd_Identifier,
                                              &PE_Enum::On_bodyStd_SwBracket_Right };
    static INT16 stateT_bodyStd[] =       	{ Tid_Identifier,
                                              Tid_SwBracket_Right };

    static F_Tok stateF_afterBlock[] = 	    { &PE_Enum::On_afterBlock_Semicolon };
    static INT16 stateT_afterBlock[] =   	{ Tid_Semicolon };

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
