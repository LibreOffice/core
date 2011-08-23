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
#include <s2_luidl/pe_singl.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_singleton.hxx>
#include <ary/idl/i_sisingleton.hxx>
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


PE_Singleton::PE_Singleton()
    :	eState(e_none),
        sData_Name(),
        bIsPreDeclaration(false),
        pCurSingleton(0),
        pCurSiSingleton(0),
        pPE_Type(0),
        nCurParsed_Type(0)
{
    pPE_Type 		= new PE_Type(nCurParsed_Type);
}

void
PE_Singleton::EstablishContacts( UnoIDL_PE *				io_pParentPE,
                               ary::Repository &		io_rRepository,
                               TokenProcessing_Result & o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Singleton::~PE_Singleton()
{
}

void
PE_Singleton::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}


void
PE_Singleton::Process_MetaType( const TokMetaType &	i_rToken )
{
    switch ( i_rToken.Id() )
    {
        case TokMetaType::mt_service:
                    if (eState == e_std)
                    {
                        SetResult(done, push_sure, pPE_Type.Ptr());
                        eState = in_service;
                    }
                    else
                        On_Default();
                    break;
        case TokMetaType::mt_singleton:
                    if (eState == need_name)
                        SetResult(done, stay);
                    else
                        On_Default();
                    break;
        default:
            // KORR_FUTURE
            // Should throw syntax error warning
                    ;

    }	// end switch
}

void
PE_Singleton::Process_Identifier( const TokIdentifier & i_rToken )
{
    if (eState == need_name)
    {
        sData_Name = i_rToken.Text();
        SetResult(done, stay);
        eState = need_curlbr_open;
    }
    else
        On_Default();
}

void
PE_Singleton::Process_Punctuation( const TokPunctuation & i_rToken )
{
    switch (i_rToken.Id())
    {
        case TokPunctuation::CurledBracketOpen:
                    if (eState == need_curlbr_open)
                    {
                        pCurSingleton = &Gate().Ces().Store_Singleton(
                                                        CurNamespace().CeId(),
                                                        sData_Name );
                        PassDocuAt(*pCurSingleton);
                        SetResult(done, stay);
                        eState = e_std;
                    }
                    else
                        On_Default();
                    break;
        case TokPunctuation::CurledBracketClose:
                    if (eState == e_std)
                    {
                        SetResult(done, stay);
                        eState = need_finish;
                    }
                    else
                        On_Default();
                    break;
        case TokPunctuation::Semicolon:
                    switch (eState)
                    {
                       case e_std:  SetResult(done, stay);
                                    break;
                       case need_finish:
                                    SetResult(done, pop_success);
                                    eState = e_none;
                                    break;
                       default:
                                    On_Default();
                    }	// end switch
                    break;
        case TokPunctuation::Colon:
                    switch (eState)
                    {
                       case need_curlbr_open:
                                    SetResult(done, push_sure, pPE_Type.Ptr());
                                    eState = in_base_interface;
                                    break;
                       default:
                                    On_Default();
                    }	// end switch
                    break;
        default:
                    On_Default();
    }	// end switch
}

void
PE_Singleton::Process_Default()
{
    On_Default();
}


void
PE_Singleton::On_Default()
{
    SetResult(not_done, pop_failure);
}

void
PE_Singleton::InitData()
{
    eState = need_name;
    sData_Name.clear();
    bIsPreDeclaration = false;
    pCurSingleton = 0;
    pCurSiSingleton = 0;
    nCurParsed_Type = 0;
}

void
PE_Singleton::TransferData()
{
    if (NOT bIsPreDeclaration)
    {
        csv_assert(sData_Name.size() > 0);
        csv_assert( (pCurSingleton != 0) != (pCurSiSingleton != 0) );
    }

    eState = e_none;
}

void
PE_Singleton::ReceiveData()
{
    switch (eState)
    {
        case in_service:
                    pCurSingleton->Set_Service(nCurParsed_Type);
                    nCurParsed_Type = 0;
                    eState = e_std;
                    break;
        case in_base_interface:
                    pCurSiSingleton = &Gate().Ces().Store_SglIfcSingleton(
                                                    CurNamespace().CeId(),
                                                    sData_Name,
                                                    nCurParsed_Type );
                    PassDocuAt(*pCurSiSingleton);
                    nCurParsed_Type = 0;
                    eState = need_finish;
                    break;
        default:
            csv_assert(false);
    }   // end switch
}

UnoIDL_PE &
PE_Singleton::MyPE()
{
    return *this;
}

}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
