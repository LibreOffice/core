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
#include <s2_luidl/pe_attri.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_attribute.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/pe_vari2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>



namespace csi
{
namespace uidl
{



PE_Attribute::PE_Attribute( const Ce_id & i_rCurOwner )
    :   eState(e_none),
        pCurOwner(&i_rCurOwner),
        pPE_Variable(0),
        pPE_Exception(0),
        pCurAttribute(0),
        nCurParsedType(0),
        sCurParsedName(),
        bReadOnly(false),
        bBound(false)
{
    pPE_Variable    = new PE_Variable(nCurParsedType, sCurParsedName);
    pPE_Exception   = new PE_Type(nCurParsedType);
}

void
PE_Attribute::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                                 ary::Repository &      io_rRepository,
                                 TokenProcessing_Result &   o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Variable->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Exception->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Attribute::~PE_Attribute()
{
}

void
PE_Attribute::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Attribute::Process_Identifier( const TokIdentifier & i_rToken )
{
    switch (eState)
    {
        case e_start:
            SetResult(not_done, push_sure, pPE_Variable.Ptr());
            eState = in_variable;
            break;
        case in_raise_std:
            if (strcmp(i_rToken.Text(),"get") == 0)
            {
                SetResult(done, stay);
                eState = in_get;
            }
            else if (strcmp(i_rToken.Text(),"set") == 0)
            {
                SetResult(done, stay);
                eState = in_set;
            }
            else
            {
                SetResult(not_done, pop_failure);
                eState = e_none;
            }
            break;
        case in_get:
        case in_set:
            SetResult(not_done, push_sure, pPE_Exception.Ptr());
            break;
        default:
            SetResult(not_done, pop_failure);
    }   // end switch
}

void
PE_Attribute::Process_Stereotype( const TokStereotype & i_rToken )
{
    if (eState != e_start)
    {
        SetResult(not_done, pop_failure);
        eState = e_none;
        return;
    }

    switch (i_rToken.Id())
    {
        case TokStereotype::ste_readonly:
            bReadOnly = true;
            break;
        case TokStereotype::ste_bound:
            bBound = true;
            break;
        default:
            SetResult(not_done, pop_failure);
            eState = e_none;
            return;
    }   // end switch

    SetResult(done, stay);
}

void
PE_Attribute::Process_MetaType( const TokMetaType & i_rToken )
{
    if (eState != e_start OR i_rToken.Id() != TokMetaType::mt_attribute)
    {
        SetResult(not_done, pop_failure);
        eState = e_none;
        return;
    }

    SetResult(done, stay);
}

void
PE_Attribute::Process_Punctuation( const TokPunctuation & i_rToken )
{
    switch (eState)
    {
        case e_start:
            SetResult(done, stay);
            break;
        case expect_end:
            switch(i_rToken.Id())
            {
                case TokPunctuation::Semicolon:
                    SetResult(done, pop_success);
                    eState = e_none;
                    break;
                case TokPunctuation::Comma:
                    SetResult(not_done, pop_failure);
                    Cerr() << "Autodoc does not support comma separated attributes, because those are discouraged by IDL policies." << Endl();
                    break;
                case TokPunctuation::CurledBracketOpen:
                    SetResult(done, stay);
                    eState = in_raise_std;
                    break;
                default:
                    SetResult(not_done, pop_failure);
            }   // end switch
            break;
        case in_raise_std:
            SetResult(done, stay);
            if (i_rToken.Id() == TokPunctuation::CurledBracketClose)
            {
                eState = expect_end;
            }
            break;
        case in_get:
        case in_set:
            SetResult(done, stay);
            if (i_rToken.Id() == TokPunctuation::Semicolon)
            {
                eState = in_raise_std;
            }
            break;
        default:
            csv_assert(false);
    }
}

void
PE_Attribute::Process_Raises()
{
    if (eState == in_get OR eState == in_set)
    {
        SetResult(done, stay);
    }
    else
        SetResult(not_done, pop_failure);
}

void
PE_Attribute::Process_Default()
{
    if (eState == e_start)
    {
        SetResult(not_done, push_sure, pPE_Variable.Ptr());
        eState = in_variable;
    }
    else if (eState == in_get OR eState == in_set)
        SetResult(not_done, push_sure, pPE_Exception.Ptr());
    else
        SetResult(not_done, pop_failure);
}

void
PE_Attribute::InitData()
{
    eState = e_start;

    pCurAttribute = 0;
    nCurParsedType = 0;
    sCurParsedName = "";
    bReadOnly = false;
    bBound = false;
}

void
PE_Attribute::TransferData()
{
    eState = e_none;
}

void
PE_Attribute::ReceiveData()
{
    switch (eState)
    {
        case in_variable:
            csv_assert(pCurOwner->IsValid());
            pCurAttribute = &Gate().Ces().Store_Attribute(
                                                    *pCurOwner,
                                                    sCurParsedName,
                                                    nCurParsedType,
                                                    bReadOnly,
                                                    bBound );
            PassDocuAt(*pCurAttribute);
            nCurParsedType = 0;
            eState = expect_end;
            break;
        case in_get:
            csv_assert(pCurAttribute != 0);
            pCurAttribute->Add_GetException(nCurParsedType);
            nCurParsedType = 0;
            break;
        case in_set:
            csv_assert(pCurAttribute != 0);
            pCurAttribute->Add_SetException(nCurParsedType);
            nCurParsedType = 0;
            break;
        default:
            csv_assert(false);
    }   // end switch
}


UnoIDL_PE &
PE_Attribute::MyPE()
{
    return *this;
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
