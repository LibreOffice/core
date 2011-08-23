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
#include <s2_luidl/pe_const.hxx>

// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_constant.hxx>
#include <ary/idl/i_constgroup.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
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
#define DF 	&PE_Constant::On_Default

PE_Constant::F_TOK
PE_Constant::aDispatcher[PE_Constant::e_STATES_MAX][PE_Constant::tt_MAX] =
        { 	{ DF, DF, DF },  // e_none
            { DF, &PE_Constant::On_expect_name_Identifier,
                      DF },  // expect_name
            { DF, DF, &PE_Constant::On_expect_curl_bracket_open_Punctuation },  // expect_curl_bracket_open
            { &PE_Constant::On_expect_const_Stereotype,
                  DF, &PE_Constant::On_expect_const_Punctuation },  // expect_const
            { DF, &PE_Constant::On_expect_value_Identifier,
                      DF },  // expect_value
            { DF, DF, &PE_Constant::On_expect_finish_Punctuation }  // expect_finish
        };



inline void
PE_Constant::CallHandler( const char *		i_sTokenText,
                          E_TokenType		i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }




PE_Constant::PE_Constant()
    :	eState(e_none),
        sData_Name(),
        nDataId(0),
        pPE_Type(0),
        nType(0),
        pPE_Value(0),
        sName(),
        sAssignment()
{
    pPE_Type = new PE_Type(nType);
    pPE_Value = new PE_Value(sName, sAssignment, true);
}

void
PE_Constant::EstablishContacts( UnoIDL_PE *					io_pParentPE,
                                ary::Repository &		io_rRepository,
                                TokenProcessing_Result & 	o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Value->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Constant::~PE_Constant()
{
}

void
PE_Constant::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Constant::Process_Identifier( const TokIdentifier & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_identifier);
}

void
PE_Constant::Process_Punctuation( const TokPunctuation & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_punctuation);
}

void
PE_Constant::Process_Stereotype( const TokStereotype & i_rToken )
{
    CallHandler(i_rToken.Text(), tt_stereotype);
}

void
PE_Constant::On_expect_name_Identifier(const char * i_sText)
{
    sName = i_sText;

    SetResult(done,stay);
    eState = expect_curl_bracket_open;
}

void
PE_Constant::On_expect_curl_bracket_open_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == '{')
    {
        sData_Name = sName;

        ary::idl::ConstantsGroup &
        rCe = Gate().Ces().
                    Store_ConstantsGroup(CurNamespace().CeId(),sData_Name);
        PassDocuAt(rCe);
        nDataId = rCe.CeId();

        SetResult(done,stay);
        eState = expect_const;
    }
    else
    {
        On_Default(i_sText);
    }
}

void
PE_Constant::On_expect_const_Stereotype(const char *)
{
    SetResult( done, push_sure, pPE_Type.Ptr() );
}

void
PE_Constant::On_expect_const_Punctuation(const char * i_sText)
{
    if ( i_sText[0] == '}')
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
PE_Constant::On_expect_value_Identifier(const char *)
{
    SetResult( not_done, push_sure, pPE_Value.Ptr() );
}

void
PE_Constant::On_expect_finish_Punctuation(const char * i_sText)
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
PE_Constant::On_Default(const char * )
{
    SetResult(not_done,pop_failure);
    eState = e_none;
}

void
PE_Constant::EmptySingleConstData()
{
    nType = 0;
    sName = "";
    sAssignment = "";
}

void
PE_Constant::CreateSingleConstant()
{
    ary::idl::Constant &
        rCe = Gate().Ces().Store_Constant( nDataId,
                                           sName,
                                           nType,
                                           sAssignment );
    pPE_Type->PassDocuAt(rCe);
}

void
PE_Constant::InitData()
{
    eState = expect_name;

    sData_Name.clear();
    nDataId = 0;

    EmptySingleConstData();
}

void
PE_Constant::ReceiveData()
{
    switch (eState)
    {
        case expect_const:
                    eState = expect_value;
                    break;
        case expect_value:
        {
                    if (sName.length() == 0 OR sAssignment.length() == 0 OR NOT nType.IsValid())
                    {
                        Cerr() << "Constant without value found." << Endl();
                        eState = expect_const;
                        break;
                    }

                    CreateSingleConstant();
                    EmptySingleConstData();
                    eState = expect_const;
        }			break;
        default:
                    SetResult(not_done, pop_failure);
                    eState = e_none;
    }	// end switch
}

void
PE_Constant::TransferData()
{
    csv_assert(nDataId.IsValid());
    eState = e_none;
}

UnoIDL_PE &
PE_Constant::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
