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
#include <s2_luidl/pe_file2.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/distrib.hxx>
#include <s2_luidl/pe_servi.hxx>
#include <s2_luidl/pe_iface.hxx>
#include <s2_luidl/pe_singl.hxx>
#include <s2_luidl/pe_struc.hxx>
#include <s2_luidl/pe_excp.hxx>
#include <s2_luidl/pe_const.hxx>
#include <s2_luidl/pe_enum2.hxx>
#include <s2_luidl/pe_tydf2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>




namespace csi
{
namespace uidl
{


PE_File::PE_File( TokenDistributor & i_rTokenAdmin,
                  const ParserInfo & i_parseInfo )
    :	pTokenAdmin(&i_rTokenAdmin),
        pPE_Service(new PE_Service),
        pPE_Singleton(new PE_Singleton),
        pPE_Interface(new PE_Interface),
        pPE_Struct(new PE_Struct),
        pPE_Exception(new PE_Exception),
        pPE_Constant(new PE_Constant),
        pPE_Enum(new PE_Enum),
        pPE_Typedef(new PE_Typedef),
        pCurNamespace(0),
        pParseInfo(&i_parseInfo),
        eState(e_none),
        nBracketCount_inDefMode(0)
{
}

void
PE_File::EstablishContacts( UnoIDL_PE *					io_pParentPE,
                            ary::Repository &		io_rRepository,
                            TokenProcessing_Result & 	o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Service->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Singleton->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Interface->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Struct->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Exception->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Constant->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Enum->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Typedef->EstablishContacts(this,io_rRepository,o_rResult);

    pCurNamespace = &Gate().Ces().GlobalNamespace();
}

PE_File::~PE_File()
{
}

void
PE_File::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_File::Process_Identifier( const TokIdentifier & i_rToken )
{
    switch (eState)
    {
        case wait_for_module:
        {
                csv_assert(pCurNamespace != 0);

                ary::idl::Module & rCe = Gate().Ces().CheckIn_Module(pCurNamespace->CeId(), i_rToken.Text());
                pCurNamespace = &rCe;

                // Get docu out of normal:
                SetDocu(pTokenAdmin->ReleaseLastParsedDocu());
                PassDocuAt(rCe);

                csv_assert(pCurNamespace != 0);

                SetResult(done, stay);
                eState = wait_for_module_bracket;
        }		break;
        case on_default:
                SetResult(done, stay);
                break;
        default:
            csv_assert(false);
    }
}

void
PE_File::Process_Punctuation( const TokPunctuation & i_rToken )
{
    switch (eState)
    {
        case e_std:
                if (i_rToken.Id() == TokPunctuation::CurledBracketClose)
                {
                    csv_assert(pCurNamespace != 0);

                    pCurNamespace = &Gate().Ces().Find_Module(pCurNamespace->Owner());

                    SetResult(done, stay);
                    eState = wait_for_module_semicolon;
                }
                else
                {
                    csv_assert(false);
                }
                break;
        case wait_for_module_bracket:
                if (i_rToken.Id() == TokPunctuation::CurledBracketOpen)
                {
                    SetResult(done, stay);
                    eState = e_std;
                }
                else
                {
                    csv_assert(false);
                }
                break;
        case wait_for_module_semicolon:
                if (i_rToken.Id() == TokPunctuation::Semicolon)
                {
                    SetResult(done, stay);
                    eState = e_std;
                }
                else
                {
                    csv_assert(false);
                }
                break;
        case on_default:
                if (i_rToken.Id() == TokPunctuation::CurledBracketClose)
                {
                    nBracketCount_inDefMode--;
                }
                else if (i_rToken.Id() == TokPunctuation::CurledBracketOpen)
                {
                    nBracketCount_inDefMode++;
                }
                else if (i_rToken.Id() == TokPunctuation::Semicolon)
                {
                    if (nBracketCount_inDefMode <= 0)
                    {
                        eState = e_std;
                    }
                }
                SetResult(done, stay);
                break;
        default:
            csv_assert(false);
    }
}

void
PE_File::Process_MetaType( const TokMetaType &	i_rToken )
{
    switch (i_rToken.Id())
    {
        case TokMetaType::mt_service:
                eState = in_sub_pe;
                SetResult( not_done, push_sure, pPE_Service.Ptr());
                break;
        case TokMetaType::mt_singleton:
                eState = in_sub_pe;
                SetResult( not_done, push_sure, pPE_Singleton.Ptr());
                break;
        case TokMetaType::mt_uik:
                Cerr() << "Syntax error: [uik ....] is obsolete now." << Endl();
                SetResult( not_done, pop_failure);
                break;
        case TokMetaType::mt_interface:
                eState = in_sub_pe;
                SetResult( not_done, push_sure, pPE_Interface.Ptr());
                break;
        case TokMetaType::mt_module:
                eState = wait_for_module;
                SetResult( done, stay );
                break;
        case TokMetaType::mt_struct:
                eState = in_sub_pe;
                SetResult( done, push_sure, pPE_Struct.Ptr());
                break;
        case TokMetaType::mt_exception:
                eState = in_sub_pe;
                SetResult( done, push_sure, pPE_Exception.Ptr());
                break;
        case TokMetaType::mt_constants:
                eState = in_sub_pe;
                SetResult( done, push_sure, pPE_Constant.Ptr());
                break;
        case TokMetaType::mt_enum:
                eState = in_sub_pe;
                SetResult( done, push_sure, pPE_Enum.Ptr());
                break;
        case TokMetaType::mt_typedef:
                eState = in_sub_pe;
                SetResult( done, push_sure, pPE_Typedef.Ptr());
                break;

        default:
                Process_Default();
    } 	// end switch
}

void
PE_File::Process_Stereotype( const TokStereotype & i_rToken )
{
    if (i_rToken.Id() == TokStereotype::ste_published)
    {
        pTokenAdmin->Set_PublishedOn();

        SetResult(done, stay);
    }
    else
    {
        Process_Default();
    }
}

void
PE_File::Process_Default()
{
    if (eState != on_default)
    {
        eState = on_default;
        nBracketCount_inDefMode = 0;
    }
    SetResult(done, stay);
}

const ary::idl::Module &
PE_File::CurNamespace() const
{
    csv_assert(pCurNamespace);
    return *pCurNamespace;
}

const ParserInfo &
PE_File::ParseInfo() const
{
    csv_assert(pParseInfo);
    return *pParseInfo;
}

void
PE_File::InitData()
{
    eState = e_std;
}

void
PE_File::TransferData()
{
    eState = e_none;
}

void
PE_File::ReceiveData()
{
    eState = e_std;
}


UnoIDL_PE &
PE_File::MyPE()
{
     return *this;
}

}   // namespace uidl
}   // namespace csi


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
