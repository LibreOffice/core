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

#include <precomp.h>
#include <s2_luidl/pe_vari2.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_property.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>


namespace csi
{
namespace uidl
{


PE_Variable::PE_Variable( ary::idl::Type_id &   i_rResult_Type,
                          String &              i_rResult_Name )
    :   eState(e_none),
        pResult_Type(&i_rResult_Type),
        pResult_Name(&i_rResult_Name),
        pPE_Type(0)
{
    pPE_Type = new PE_Type(i_rResult_Type);
}

void
PE_Variable::EstablishContacts( UnoIDL_PE *                 io_pParentPE,
                                ary::Repository &           io_rRepository,
                                TokenProcessing_Result &    o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Variable::~PE_Variable()
{
}

void
PE_Variable::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}


void
PE_Variable::Process_Default()
{
    if (eState == expect_type)
    {
        SetResult( not_done, push_sure, pPE_Type.Ptr() );
    }
    else{
        csv_assert(false);
    }
}

void
PE_Variable::Process_Identifier( const TokIdentifier & i_rToken )
{
    if (eState == expect_type)
    {
        SetResult( not_done, push_sure, pPE_Type.Ptr() );
    }
    else if (eState == expect_name)
    {
        *pResult_Name = i_rToken.Text();
        SetResult( done, stay );
        eState = expect_finish;
    }
    else {
        csv_assert(false);
    }
}

void
PE_Variable::Process_Punctuation( const TokPunctuation & )
{
    if (eState == expect_finish)
    {
        SetResult( not_done, pop_success );
        eState = e_none;
    }
    else if (eState == expect_name)
    {
        SetResult( not_done, pop_success );
        eState = e_none;
    }
    else {
        csv_assert(false);
    }
}

void
PE_Variable::Process_BuiltInType( const TokBuiltInType & i_rToken )
{
    if (eState == expect_type)
    {
        SetResult( not_done, push_sure, pPE_Type.Ptr() );
    }
    else if (eState == expect_name AND i_rToken.Id() == TokBuiltInType::bty_ellipse)
    {
        SetResult( not_done, pop_success );
        eState = e_none;
    }
    else {
        csv_assert(false);
    }
}

void
PE_Variable::InitData()
{
    eState = expect_type;

    *pResult_Type = 0;
    *pResult_Name = "";
}

void
PE_Variable::ReceiveData()
{
    eState = expect_name;
}

void
PE_Variable::TransferData()
{
    eState = e_none;
}

UnoIDL_PE &
PE_Variable::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
