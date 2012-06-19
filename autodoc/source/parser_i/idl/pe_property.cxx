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
#include <s2_luidl/pe_property.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_property.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_vari2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>



namespace csi
{
namespace uidl
{



PE_Property::PE_Property( const Ce_id & i_rCurOwner )
    :   eState(e_none),
        pCurOwner(&i_rCurOwner),
        pPE_Variable(0),
        nCurParsedType(0),
        sCurParsedName(),
        bIsOptional(false),
        aStereotypes()
{
    pPE_Variable    = new PE_Variable(nCurParsedType, sCurParsedName);
}

void
PE_Property::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                                ary::Repository &       io_rRepository,
                                TokenProcessing_Result &    o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Variable->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Property::~PE_Property()
{
}

void
PE_Property::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Property::Process_Stereotype( const TokStereotype & i_rToken )
{
    switch (i_rToken.Id())
    {
        case TokStereotype::ste_optional:
            bIsOptional = true;
            break;
        case TokStereotype::ste_readonly:
            aStereotypes.Set_Flag(Stereotypes::readonly);
            break;
        case TokStereotype::ste_bound:
            aStereotypes.Set_Flag(Stereotypes::bound);
            break;
        case TokStereotype::ste_constrained:
            aStereotypes.Set_Flag(Stereotypes::constrained);
            break;
        case TokStereotype::ste_maybeambiguous:
            aStereotypes.Set_Flag(Stereotypes::maybeambiguous);
            break;
        case TokStereotype::ste_maybedefault:
            aStereotypes.Set_Flag(Stereotypes::maybedefault);
            break;
        case TokStereotype::ste_maybevoid:
            aStereotypes.Set_Flag(Stereotypes::maybevoid);
            break;
        case TokStereotype::ste_removable:
            aStereotypes.Set_Flag(Stereotypes::removable);
            break;
        case TokStereotype::ste_transient:
            aStereotypes.Set_Flag(Stereotypes::transient);
            break;

        default:
            SetResult(not_done, pop_failure);
            eState = e_none;
            return;
    }

    SetResult(done, stay);
}

void
PE_Property::Process_MetaType( const TokMetaType & i_rToken )
{
    if (eState == e_start)
    {
        if ( i_rToken.Id() == TokMetaType::mt_property )
        {
            SetResult(done, stay);
            eState = expect_variable;
            return;
        }
    }   // endif (eState == e_start)

    SetResult(not_done, pop_failure);
    eState = e_none;
}

void
PE_Property::Process_Punctuation( const TokPunctuation & i_rToken )
{
    switch (eState)
    {
        case e_start:
            SetResult(done, stay);
            break;
        case expect_variable:
            if (i_rToken.Id() == TokPunctuation::Semicolon)
            {
                SetResult(done, pop_success);
                eState = e_none;
            }
            else if (i_rToken.Id() == TokPunctuation::Comma)
                SetResult(done, stay);
            else
                SetResult(not_done, pop_failure);
            break;
        default:
            csv_assert(false);
    }
}

void
PE_Property::Process_Default()
{
    if (eState == expect_variable)
    {
        SetResult(not_done, push_sure, pPE_Variable.Ptr());
        eState = in_variable;
    }
    else
        SetResult(not_done, pop_failure);
}

void
PE_Property::InitData()
{
    eState = e_start;

    nCurParsedType = 0;
    sCurParsedName = "";

    // bIsOptional and
    // aStereotypes
    //   may be preset by the PE_Service-(or PE_Interface-)parent
    //   with PresetOptional() or
    //   PresetStereotype()
    //   - therefore it must not be set here!
}

void
PE_Property::TransferData()
{
    if (bIsOptional)
    {
        SetOptional();
        bIsOptional = false;
    }

    ary::idl::CodeEntity *
        pCe = 0;
    csv_assert(pCurOwner->IsValid());

    pCe = &Gate().Ces().Store_Property( *pCurOwner,
                                        sCurParsedName,
                                        nCurParsedType,
                                        aStereotypes );

    csv_assert(pCe != 0);
    PassDocuAt(*pCe);

    nCurParsedType = 0;
    sCurParsedName.clear();
    aStereotypes = Stereotypes();

    eState = e_none;
}

void
PE_Property::ReceiveData()
{
    eState = expect_variable;
}


UnoIDL_PE &
PE_Property::MyPE()
{
    return *this;
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
