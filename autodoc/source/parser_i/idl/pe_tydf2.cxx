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
#include <s2_luidl/pe_tydf2.hxx>

// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
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
#define DF  &PE_Typedef::On_Default

PE_Typedef::F_TOK
PE_Typedef::aDispatcher[PE_Typedef::e_STATES_MAX][PE_Typedef::tt_MAX] =
        {   { DF, DF, DF },  // e_none
            { &PE_Typedef::On_expect_description_Any,
                  &PE_Typedef::On_expect_description_Any,
                      DF },  // expect_description
            { DF, &PE_Typedef::On_expect_name_Identifier,
                      DF },  // expect_name
            { DF, DF, &PE_Typedef::On_got_name_Punctuation }  // got_name
        };



inline void
PE_Typedef::CallHandler( const char *       i_sTokenText,
                         E_TokenType        i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }





PE_Typedef::PE_Typedef()
    :   eState(e_none),
        pPE_Type(0),
        nType(0),
        sName()
{
    pPE_Type = new PE_Type(nType);
}

void
PE_Typedef::EstablishContacts( UnoIDL_PE *              io_pParentPE,
                               ary::Repository &    io_rRepository,
                               TokenProcessing_Result & o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Typedef::~PE_Typedef()
{
}

void
PE_Typedef::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Typedef::Process_Identifier( const TokIdentifier & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_identifier);
}

void
PE_Typedef::Process_Punctuation( const TokPunctuation & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_punctuation);
}

void
PE_Typedef::Process_Default()
{
    CallHandler("", tt_any);
}

void
PE_Typedef::On_expect_description_Any(SAL_UNUSED_PARAMETER const char *)
{
    SetResult(not_done,push_sure, pPE_Type.Ptr());
}

void
PE_Typedef::On_expect_name_Identifier(const char * i_sText)
{
    sName = i_sText;
    SetResult(done,stay);
    eState = got_name;
}

void
PE_Typedef::On_got_name_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == ';' )
    {
        SetResult(done,pop_success);
        eState = e_none;
    }
    else
        On_Default(i_sText);
}

void
PE_Typedef::On_Default(SAL_UNUSED_PARAMETER const char * )
{
    SetResult(not_done,pop_failure);
}

void
PE_Typedef::InitData()
{
    eState = expect_description;
    nType = 0;
    sName = "";
}

void
PE_Typedef::ReceiveData()
{
    eState = expect_name;
}

void
PE_Typedef::TransferData()
{
    ary::idl::Typedef &
        rCe = Gate().Ces().Store_Typedef(CurNamespace().CeId(), sName, nType);
    PassDocuAt(rCe);
    eState = e_none;
}

UnoIDL_PE &
PE_Typedef::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
