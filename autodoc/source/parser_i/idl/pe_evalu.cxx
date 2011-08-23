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
#include <s2_luidl/pe_evalu.hxx>

// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_enumvalue.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <s2_luidl/tk_const.hxx>


namespace csi
{
namespace uidl
{


#ifdef DF
#undef DF
#endif
#define DF 	&PE_Value::On_Default

PE_Value::F_TOK
PE_Value::aDispatcher[PE_Value::e_STATES_MAX][PE_Value::tt_MAX] =
        { 	{ DF, DF, DF },  // e_none
            { &PE_Value::On_expect_name_Identifier,
                  DF, DF },  // expect_name
            { DF, &PE_Value::On_got_name_Punctuation,
                      &PE_Value::On_got_name_Assignment }  // got_name
        };



inline void
PE_Value::CallHandler( const char *		i_sTokenText,
                       E_TokenType		i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }





PE_Value::PE_Value( String  & 	o_rName,
                    String  &	o_rAssignment,
                    bool		i_bIsConst )
    :	eState(e_none),
        pName(&o_rName),
        pAssignment(&o_rAssignment),
        bIsConst(i_bIsConst)
{
}

void
PE_Value::EstablishContacts( UnoIDL_PE *				io_pParentPE,
                             ary::Repository &			io_rRepository,
                             TokenProcessing_Result & 	o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
}

PE_Value::~PE_Value()
{
}

void
PE_Value::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Value::Process_Identifier( const TokIdentifier & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_identifier);
}

void
PE_Value::Process_Punctuation( const TokPunctuation & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_punctuation);
}

void
PE_Value::Process_Assignment( const TokAssignment & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_assignment);
}

void
PE_Value::On_expect_name_Identifier(const char * i_sText)
{
    *pName = i_sText;
    SetResult(done,stay);
    eState = got_name;
}

void
PE_Value::On_got_name_Punctuation(const char * i_sText)
{
    if ( (i_sText[0] == ',' AND NOT IsConst())
         OR (i_sText[0] == ';' AND IsConst()) )
    {
        SetResult(done,pop_success);
        eState = e_none;
    }
    else if (i_sText[0] == '}' AND NOT IsConst())
    {
        SetResult(not_done,pop_success);
        eState = e_none;
    }
    else
        On_Default(i_sText);
}

void
PE_Value::On_got_name_Assignment(const char * i_sText)
{
    *pAssignment = i_sText;
    SetResult(done,pop_success);
    eState = e_none;
}

void
PE_Value::On_Default(const char * )
{
    SetResult(not_done,pop_failure);
}

void
PE_Value::InitData()
{
    eState = expect_name;

    *pName = "";
    *pAssignment = "";
}

void
PE_Value::TransferData()
{
    csv_assert(pName->length() > 0);
    eState = e_none;
}

UnoIDL_PE &
PE_Value::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
