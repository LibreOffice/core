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
#include <s2_luidl/pe_type2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_type.hxx>
#include <ary/idl/ip_type.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/uidl_tok.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_punct.hxx>



/** Implementation Concept for Parsing a Type

Example Type:
    sequence < ::abc::TName< TplType > >  AnyName;

Status Changes:

expect_type:
    sequence        -> expect_type
    <               -> expect_type
    ::              -> expect_quname_part
    abc             -> expect_quname_separator
    ::              -> expect_quname_part
    TName           -> expect_quname_separator
    <               -> in_template_type (process in nested PE_Type instance)

        expect_type:
            TplType         ->expect_quname_separator
            >               -> e_none (finish, '>' not handled)

    >               -> expect_quname_separator
    >               -> expect_quname_separator (not finish, because sequencecounter > 0)
    AnyName         -> e_none  (finish)
*/


namespace csi
{
namespace uidl
{


PE_Type::PE_Type( ary::idl::Type_id & o_rResult )
    :	pResult(&o_rResult),
        nIsSequenceCounter(0),
        nSequenceDownCounter(0),
        bIsUnsigned(false),
        sFullType(),
        eState(e_none),
        sLastPart(),
        pPE_TemplateType(0), // @attention Recursion, only initiate, if needed!
        nTemplateType(0),
        aTemplateParameters()
{
}

PE_Type::~PE_Type()
{
}

void
PE_Type::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Type::Process_Identifier( const TokIdentifier & i_rToken )
{
    if (eState == expect_type)
    {
        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_part)
    {
        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        Finish();
    }
}

void
PE_Type::Process_NameSeparator()
{
    if (eState == expect_type)
    {
        sFullType.Init(true);
        eState = expect_quname_part;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        sFullType += sLastPart;
        eState = expect_quname_part;
        SetResult(done, stay);
    }
}

void
PE_Type::Process_Punctuation( const TokPunctuation & i_rToken )
{
    if (eState == expect_type)
    {
        csv_assert(i_rToken.Id() == TokPunctuation::Lesser);
           SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        switch (i_rToken.Id())
        {
            case TokPunctuation::Lesser:
                eState = in_template_type;
                SetResult( done, push_sure, &MyTemplateType() );
                break;

            case TokPunctuation::Greater:
                if (nSequenceDownCounter > 0)
                {
                    nSequenceDownCounter--;
                    SetResult(done, stay);
                }
                else
                {
                    Finish();
                }
                break;

            default:
                Finish();
        }   // end switch
    }
    else if (eState == in_template_type)
    {
        aTemplateParameters.push_back(nTemplateType);
        nTemplateType = 0;

        if (i_rToken.Id() == TokPunctuation::Greater)
        {
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
        else if (i_rToken.Id() == TokPunctuation::Comma)
        {
              SetResult(done, push_sure, &MyTemplateType());
        }
        else
        {
            csv_assert(false);
            Finish();
        }
    }
}

void
PE_Type::Process_BuiltInType( const TokBuiltInType & i_rToken )
{
    if (eState == expect_type)
    {
        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_part)
    {
        // Can this happen?

        sLastPart = i_rToken.Text();
        eState = expect_quname_separator;
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        // Can this happen?

        Finish();
    }
}

void
PE_Type::Process_TypeModifier( const TokTypeModifier & i_rToken )
{
    if (eState == expect_type)
    {
        switch ( i_rToken.Id() )
        {
            case TokTypeModifier::tmod_unsigned:
                    bIsUnsigned = true;
                    break;
            case TokTypeModifier::tmod_sequence:
                    nIsSequenceCounter++;
                    nSequenceDownCounter++;
                    break;
            default:
                csv_assert(false);
        }
        SetResult(done, stay);
    }
    else if (eState == expect_quname_separator)
    {
        // Can this happen?

        Finish();
    }
}

void
PE_Type::Process_Default()
{
    Finish();
}

void
PE_Type::Finish()
{
    csv_assert(nSequenceDownCounter == 0);

    sFullType.SetLocalName(sLastPart);
    SetResult(not_done, pop_success);
}

PE_Type &
PE_Type::MyTemplateType()
{
    if (NOT pPE_TemplateType)
    {
        pPE_TemplateType = new PE_Type(nTemplateType);
        pPE_TemplateType->EstablishContacts( this,
                                             MyRepository(),
                                             TokenResult() );
    }
    return *pPE_TemplateType;
}

void
PE_Type::InitData()
{
    eState = expect_type;

    nIsSequenceCounter = 0;
    nSequenceDownCounter = 0;
    bIsUnsigned = false;
    sFullType.Empty();
    sLastPart.clear();
    nTemplateType = 0;
    csv::erase_container(aTemplateParameters);
}

void
PE_Type::TransferData()
{
    if (bIsUnsigned)
    {
        StreamLock sl(40);
        String sName( sl() << "unsigned " << sFullType.LocalName() << c_str );
        sFullType.SetLocalName(sName);
    }

    const ary::idl::Type &
        result = Gate().Types().CheckIn_Type( sFullType,
                                              nIsSequenceCounter,
                                              CurNamespace().CeId(),
                                              &aTemplateParameters );
    *pResult = result.TypeId();
    eState = e_none;
}

UnoIDL_PE &
PE_Type::MyPE()
{
     return *this;
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
