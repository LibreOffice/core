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
#include <s2_luidl/pe_enum2.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_enum.hxx>
#include <ary/idl/i_enumvalue.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_evalu.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_keyw.hxx>


namespace csi
{
namespace uidl
{


#ifdef DF
#undef DF
#endif
#define DF  &PE_Enum::On_Default

PE_Enum::F_TOK
PE_Enum::aDispatcher[PE_Enum::e_STATES_MAX][PE_Enum::tt_MAX] =
        {   {  DF, DF },  // e_none
            {  &PE_Enum::On_expect_name_Identifier,
                   DF },  // expect_name
            {  DF, &PE_Enum::On_expect_curl_bracket_open_Punctuation },  // expect_curl_bracket_open
            {  &PE_Enum::On_expect_value_Identifier,
                   &PE_Enum::On_expect_value_Punctuation },  // expect_value
            {  DF, &PE_Enum::On_expect_finish_Punctuation }  // expect_finish
        };



inline void
PE_Enum::CallHandler( const char *      i_sTokenText,
                          E_TokenType       i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }




PE_Enum::PE_Enum()
    :   eState(e_none),
        sData_Name(),
        nDataId(0),
        pPE_Value(0),
        sName(),
        sAssignment()
{
    pPE_Value = new PE_Value(sName, sAssignment, false);
}

void
PE_Enum::EstablishContacts( UnoIDL_PE *                 io_pParentPE,
                            ary::Repository &       io_rRepository,
                            TokenProcessing_Result &    o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Value->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Enum::~PE_Enum()
{
}

void
PE_Enum::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Enum::Process_Identifier( const TokIdentifier & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_identifier);
}

void
PE_Enum::Process_Punctuation( const TokPunctuation & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_punctuation);
}

void
PE_Enum::On_expect_name_Identifier(const char * i_sText)
{
    sName = i_sText;

    SetResult(done,stay);
    eState = expect_curl_bracket_open;
}

void
PE_Enum::On_expect_curl_bracket_open_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == '{')
    {
        sData_Name = sName;
        ary::idl::Enum &
            rCe = Gate().Ces().Store_Enum(CurNamespace().CeId(), sData_Name);
        PassDocuAt(rCe);
        nDataId = rCe.CeId();

        SetResult(done,stay);
        eState = expect_value;
    }
    else
    {
        On_Default(i_sText);
    }
}

void
PE_Enum::On_expect_value_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == '}' )
    {
        SetResult(done,stay);
        eState = expect_finish;
    }
    else
    {
        On_Default(i_sText);
    }
}

void
PE_Enum::On_expect_value_Identifier(SAL_UNUSED_PARAMETER const char *)
{
    SetResult( not_done, push_sure, pPE_Value.Ptr() );
}

void
PE_Enum::On_expect_finish_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == ';')
    {
        SetResult(done,pop_success);
        eState = e_none;
    }
    else
    {
        On_Default(i_sText);
    }
}

void
PE_Enum::On_Default(SAL_UNUSED_PARAMETER const char * )
{
    SetResult(not_done,pop_failure);
    eState = e_none;
}

void
PE_Enum::EmptySingleValueData()
{
    sName = "";
    sAssignment = "";
}

void
PE_Enum::CreateSingleValue()
{
    ary::idl::EnumValue &
        rCe = Gate().Ces().Store_EnumValue( nDataId, sName, sAssignment );
    pPE_Value->PassDocuAt(rCe);
}

void
PE_Enum::InitData()
{
    eState = expect_name;

    sData_Name.clear();
    nDataId = 0;

    EmptySingleValueData();
}

void
PE_Enum::ReceiveData()
{
    switch (eState)
    {
        case expect_value:
        {
                    if (sName.length() == 0)
                    {
                        On_Default("");
                        break;
                    }

                    CreateSingleValue();
                    EmptySingleValueData();
        }           break;
        default:
                    SetResult(not_done, pop_failure);
                    eState = e_none;
    }   // end switch
}

void
PE_Enum::TransferData()
{
    csv_assert(sData_Name.length() > 0);
    eState = e_none;
}

UnoIDL_PE &
PE_Enum::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
