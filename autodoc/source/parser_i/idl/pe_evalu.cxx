/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include "sal/types.h"

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
#define DF  &PE_Value::On_Default

PE_Value::F_TOK
PE_Value::aDispatcher[PE_Value::e_STATES_MAX][PE_Value::tt_MAX] =
        {   { DF, DF, DF },  // e_none
            { &PE_Value::On_expect_name_Identifier,
                  DF, DF },  // expect_name
            { DF, &PE_Value::On_got_name_Punctuation,
                      &PE_Value::On_got_name_Assignment }  // got_name
        };



inline void
PE_Value::CallHandler( const char *     i_sTokenText,
                       E_TokenType      i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }





PE_Value::PE_Value( String  &   o_rName,
                    String  &   o_rAssignment,
                    bool        i_bIsConst )
    :   eState(e_none),
        pName(&o_rName),
        pAssignment(&o_rAssignment),
        bIsConst(i_bIsConst)
{
}

void
PE_Value::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                             ary::Repository &          io_rRepository,
                             TokenProcessing_Result &   o_rResult )
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
PE_Value::On_Default(SAL_UNUSED_PARAMETER const char * )
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
