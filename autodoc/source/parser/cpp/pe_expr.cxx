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






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
