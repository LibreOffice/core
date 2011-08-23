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
#include <s2_luidl/pe_iface.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_interface.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_func2.hxx>
#include <s2_luidl/pe_attri.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <adc_cl.hxx>



namespace csi
{
namespace uidl
{

#ifdef DF
#undef DF
#endif
#define DF 	&PE_Interface::On_Default

PE_Interface::F_TOK
PE_Interface::aDispatcher[PE_Interface::e_STATES_MAX][PE_Interface::tt_MAX] =
        { 	{ DF, DF, DF, DF, DF },  // e_none
            { &PE_Interface::On_need_uik_MetaType,
                 DF, DF, DF, DF },  // need_uik
            { DF, &PE_Interface::On_uik_Identifier,
                     &PE_Interface::On_uik_Punctuation,
                         DF, DF },  // uik
            { &PE_Interface::On_need_ident_MetaType,
                 DF, DF, DF, DF },  // need_ident
            { DF, &PE_Interface::On_ident_Identifier,
                     &PE_Interface::On_ident_Punctuation,
                         DF, DF },  // ident
            { &PE_Interface::On_need_interface_MetaType,
                 DF, DF, DF, DF },  // need_interface
            { DF, &PE_Interface::On_need_name_Identifer,
                    DF, DF, DF },  // need_name
            { DF, DF, &PE_Interface::On_wait_for_base_Punctuation,
                        DF, DF },  // wait_for_base
            { DF, DF, DF, DF, DF },  // in_base
            { DF, DF, &PE_Interface::On_need_curlbr_open_Punctuation,
                          DF, DF },  // need_curlbr_open
            { &PE_Interface::On_std_Metatype,
                  &PE_Interface::On_std_GotoFunction,
                      &PE_Interface::On_std_Punctuation,
                          &PE_Interface::On_std_GotoFunction,
                              &PE_Interface::On_std_Stereotype },  // e_std
            { DF, DF, DF, DF, DF },  // in_function
            { DF, DF, DF, DF, DF },  // in_attribute
            { DF, DF, &PE_Interface::On_need_finish_Punctuation,
                          DF, DF },  // need_finish
            { DF, DF, DF, DF, DF }   // in_base_interface
        };



inline void
PE_Interface::CallHandler( const char *		i_sTokenText,
                           E_TokenType		i_eTokenType )
    { (this->*aDispatcher[eState][i_eTokenType])(i_sTokenText); }



PE_Interface::PE_Interface()
    :	eState(e_none),
        sData_Name(),
        bIsPreDeclaration(false),
        pCurInterface(0),
        nCurInterface(0),
        pPE_Function(0),
        pPE_Attribute(0),
        pPE_Type(0),
        nCurParsed_Base(0),
        bOptionalMember(false)
{
    pPE_Function 	= new PE_Function(nCurInterface);
    pPE_Type 		= new PE_Type(nCurParsed_Base);
    pPE_Attribute   = new PE_Attribute(nCurInterface);
}

void
PE_Interface::EstablishContacts( UnoIDL_PE *				io_pParentPE,
                                 ary::Repository &		io_rRepository,
                                 TokenProcessing_Result & 	o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Function->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Attribute->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Interface::~PE_Interface()
{
}

void
PE_Interface::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}


void
PE_Interface::Process_MetaType( const TokMetaType &	i_rToken )
{
    CallHandler( i_rToken.Text(), tt_metatype );
}

void
PE_Interface::Process_Identifier( const TokIdentifier & i_rToken )
{
    CallHandler( i_rToken.Text(), tt_identifier );
}

void
PE_Interface::Process_Punctuation( const TokPunctuation & i_rToken )
{
    CallHandler( i_rToken.Text(), tt_punctuation );
}

void
PE_Interface::Process_NameSeparator()
{
    CallHandler( "", tt_startoftype );
}

void
PE_Interface::Process_BuiltInType( const TokBuiltInType & i_rToken )
{
    CallHandler( i_rToken.Text(), tt_startoftype );
}

void
PE_Interface::Process_TypeModifier( const TokTypeModifier & i_rToken )
{
    CallHandler( i_rToken.Text(), tt_startoftype );
}

void
PE_Interface::Process_Stereotype( const TokStereotype & i_rToken )
{
    CallHandler( i_rToken.Text(), tt_stereotype );
}

void
PE_Interface::Process_Default()
{
    SetResult(done, stay);
}


void
PE_Interface::On_need_uik_MetaType(const char *)
{
    // Deprecated, data will be ignored
    SetResult(done, stay);
    eState = uik;
}

void
PE_Interface::On_uik_Identifier(const char *)
{
    // Deprecated, data will be ignored
    SetResult(done, stay);
}

void
PE_Interface::On_uik_Punctuation(const char * i_sText)
{
    // Deprecated, data will be ignored
    SetResult(done, stay);
    if (strcmp(",",i_sText) == 0)
    {
        eState = need_ident;
    }
}

void
PE_Interface::On_need_ident_MetaType(const char *)
{
    SetResult(done, stay);
    eState = ident;
}

void
PE_Interface::On_ident_Identifier(const char *)
{
    SetResult(done, stay);
}

void
PE_Interface::On_ident_Punctuation(const char * i_sText)
{
    SetResult(done, stay);
    if (strcmp(")",i_sText) == 0)
    {
        eState = need_interface;
    }
}

void
PE_Interface::On_need_interface_MetaType(const char *)
{
    SetResult(done, stay);
    eState = need_name;
}

void
PE_Interface::On_need_name_Identifer(const char * i_sText)
{
    SetResult(done, stay);
    sData_Name = i_sText;
    eState = wait_for_base;
}

void
PE_Interface::On_wait_for_base_Punctuation(const char * i_sText)
{
    if (i_sText[0] != ';')
    {
        switch (i_sText[0])
        {
            case ':':
                SetResult(done, push_sure, pPE_Type.Ptr());
                eState = in_base;
                break;
            case '{':
                store_Interface();

                SetResult(done,stay);
                eState = e_std;
                break;
            default:
                SetResult(not_done, pop_failure);
                eState = e_none;
        }	// end switch
    }
    else
    {
        bIsPreDeclaration = true;
        SetResult(done, pop_success);
        eState = e_none;
    }
}

void
PE_Interface::On_need_curlbr_open_Punctuation(const char * i_sText)
{
    if (i_sText[0] == '{')
    {
        store_Interface();

        SetResult(done, stay);
        eState = e_std;
    }
    else {
        csv_assert(false);
    }
}


void
PE_Interface::On_std_Metatype(const char * i_sText)
{
    if (strcmp(i_sText,"attribute") ==  0)
        On_std_GotoAttribute(i_sText);
    else if (strcmp(i_sText,"interface") ==  0)
        On_std_GotoBaseInterface(i_sText);
    else
        On_std_GotoFunction(i_sText);
}

void
PE_Interface::On_std_Punctuation(const char * i_sText)
{
    switch (i_sText[0])
    {
        case '}':
            SetResult(done, stay);
            eState = need_finish;
            break;
        case ';':   // Appears after base interface declarations.
            SetResult(done, stay);
            break;
        default:
            SetResult(not_done, pop_failure);
            eState = e_none;
    }	// end switch
}

void
PE_Interface::On_std_Stereotype(const char * i_sText)
{
    if (strcmp(i_sText,"oneway") ==  0)
        On_std_GotoFunction(i_sText);
    else if (    strcmp(i_sText,"readonly") ==  0
              OR strcmp(i_sText,"bound") ==  0 )
        On_std_GotoAttribute(i_sText);
    else if (strcmp(i_sText,"optional") ==  0)
    {
        bOptionalMember = true;
        SetResult(done, stay);
    }
    else
        SetResult(not_done, pop_failure);
}

void
PE_Interface::On_std_GotoFunction(const char * )
{
    SetResult(not_done, push_sure, pPE_Function.Ptr());
    eState = in_function;
}

void
PE_Interface::On_std_GotoAttribute(const char * )
{
        SetResult(not_done, push_sure, pPE_Attribute.Ptr());
        eState = in_attribute;
}

void
PE_Interface::On_std_GotoBaseInterface(const char * )
{
       SetResult(done, push_sure, pPE_Type.Ptr());
    eState = in_base_interface;
}

void
PE_Interface::On_need_finish_Punctuation(const char * i_sText)
{
    switch (i_sText[0])
    {
        case ';':
            SetResult(done, pop_success);
            eState = e_none;
            break;
        default:
            SetResult(not_done, pop_failure);
            eState = e_none;
    }	// end switch
}

void
PE_Interface::On_Default(const char *)
{
    SetResult(not_done, pop_failure);
}

void
PE_Interface::InitData()
{
    eState = need_interface;

    sData_Name.clear();
    bIsPreDeclaration = false;
    pCurInterface = 0;
    nCurInterface = 0;
    nCurParsed_Base = 0;
    bOptionalMember = false;
}

void
PE_Interface::TransferData()
{
    if (NOT bIsPreDeclaration)
    {
        csv_assert(sData_Name.size() > 0);
        csv_assert(nCurInterface.IsValid());
    }
    else
    {
         sData_Name.clear();
         pCurInterface = 0;
        nCurInterface = 0;
    }

    eState = e_none;
}

void
PE_Interface::ReceiveData()
{
    switch (eState)
    {
        case in_base:
                eState = need_curlbr_open;
                break;
        case in_function:
                eState = e_std;
                break;
        case in_attribute:
                eState = e_std;
                break;
        case in_base_interface:
                if (bOptionalMember)
                {
                    pPE_Type->SetOptional();
                    bOptionalMember = false;
                }
                pCurInterface->Add_Base(
                                    nCurParsed_Base,
                                    pPE_Type->ReleaseDocu());
                nCurParsed_Base = 0;
                eState = e_std;
                break;
        default:
            csv_assert(false);
    }
}

UnoIDL_PE &
PE_Interface::MyPE()
{
     return *this;
}

void
PE_Interface::store_Interface()
{
    pCurInterface = & Gate().Ces().Store_Interface(
                                        CurNamespace().CeId(),
                                        sData_Name,
                                        nCurParsed_Base );
    nCurInterface = pCurInterface->CeId();
    PassDocuAt(*pCurInterface);
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
