/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <precomp.h>
#include <s2_luidl/pe_servi.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/i_siservice.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_func2.hxx>
#include <s2_luidl/pe_property.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>



namespace csi
{
namespace uidl
{



PE_Service::PE_Service()
    :   eState(e_none),
        sData_Name(),
        bIsPreDeclaration(false),
        pCurService(0),
        pCurSiService(0),
        nCurService(0),
        pPE_Property(0),
        nCurParsed_Property(0),
        pPE_Type(0),
        nCurParsed_Type(0),
        pPE_Constructor(0),
        bOptionalMember(false)
{
    pPE_Property    = new PE_Property(nCurService);
    pPE_Type        = new PE_Type(nCurParsed_Type);
    pPE_Constructor = new PE_Function(nCurService, PE_Function::constructor);
}

void
PE_Service::EstablishContacts( UnoIDL_PE *              io_pParentPE,
                               ary::Repository &        io_rRepository,
                               TokenProcessing_Result & o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Property->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Constructor->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Service::~PE_Service()
{
}

void
PE_Service::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}


void
PE_Service::Process_MetaType( const TokMetaType &   i_rToken )
{
    switch ( i_rToken.Id() )
    {
        case TokMetaType::mt_service:
                    if (eState == need_name)
                        SetResult(done, stay );
                    else if (eState == e_std)
                    {
                        SetResult(done, push_sure, pPE_Type.Ptr());
                        eState = in_service_type;
                    }
                    else
                        On_Default();
                    break;
        case TokMetaType::mt_interface:
                    if (eState == e_std)
                    {
                        SetResult(done, push_sure, pPE_Type.Ptr());
                        eState = in_ifc_type;
                    }
                    else
                        On_Default();
                    break;
        case TokMetaType::mt_property:
                    if (eState == e_std)
                    {
                        StartProperty();
                    }
                    else
                        On_Default();
                    break;
        default:
            // KORR_FUTURE:
            // Should throw syntax error warning.
                    ;
    }   // end switch
}

void
PE_Service::Process_Identifier( const TokIdentifier & i_rToken )
{
    if (eState == need_name)
    {
        sData_Name = i_rToken.Text();
        SetResult(done, stay);
        eState = need_curlbr_open;
    }
    else if (eState == e_std_sib)
    {
        SetResult(not_done, push_sure, pPE_Constructor.Ptr());
    }
    else
        On_Default();
}

void
PE_Service::Process_Punctuation( const TokPunctuation & i_rToken )
{
    switch (i_rToken.Id())
    {
        case TokPunctuation::Colon:
                    if (eState == need_curlbr_open)
                    {
                        SetResult(done, push_sure, pPE_Type.Ptr());
                        eState = need_base_interface;
                    }
                    else
                        On_Default();
                    break;

        case TokPunctuation::CurledBracketOpen:
                    if (eState == need_curlbr_open)
                    {
                        pCurService = &Gate().Ces().Store_Service(
                                                        CurNamespace().CeId(),
                                                        sData_Name );
                        nCurService = pCurService->CeId();
                        PassDocuAt(*pCurService);
                        SetResult(done, stay);
                        eState = e_std;
                    }
                    else if (eState == need_curlbr_open_sib)
                    {
                        SetResult(done, stay);
                        eState = e_std_sib;
                    }
                    else
                        On_Default();
                    break;
        case TokPunctuation::CurledBracketClose:
                    if (eState == e_std OR eState == e_std_sib)
                    {
                        SetResult(done, stay);
                        eState = need_finish;
                    }
                    else
                        On_Default();
                    break;
        case TokPunctuation::Comma:
                    if (eState == expect_ifc_separator)
                    {
                        SetResult(done, push_sure, pPE_Type.Ptr());
                        eState = in_ifc_type;
                    }
                    else if (eState == expect_service_separator)
                    {
                        SetResult(done, push_sure, pPE_Type.Ptr());
                        eState = in_service_type;
                    }
                    else if (eState == e_std)
                    {
                        SetResult(done, stay);
                    }
                    else
                        On_Default();
                    break;
        case TokPunctuation::Semicolon:
                    switch (eState)
                    {
                       case need_curlbr_open:
                                    sData_Name.clear();
                                    bIsPreDeclaration = true;
                                    SetResult(done, pop_success);
                                    eState = e_none;
                                    break;
                       case need_curlbr_open_sib:
                                    SetResult(done, pop_success);
                                    eState = e_none;
                                    break;
                       case expect_ifc_separator:
                       case expect_service_separator:
                                    SetResult(done, stay);
                                    eState = e_std;
                                    break;
                       case need_finish:
                                    SetResult(done, pop_success);
                                    eState = e_none;
                                    break;
                       case at_ignore:
                                    SetResult(done, stay);
                                    eState = e_std;
                                    break;
                       default:
                                    On_Default();
                    }   // end switch
                    break;
        default:
                    On_Default();
    }   // end switch
}

void
PE_Service::Process_Stereotype( const TokStereotype & i_rToken )
{
    if (i_rToken.Id() == TokStereotype::ste_optional)
    {
        bOptionalMember = true;
        SetResult(done, stay);
    }
    else if ( eState == e_std )
    {
        StartProperty();
    }
    else
        On_Default();
}

void
PE_Service::Process_Needs()
{
    SetResult(done,stay);
    eState = at_ignore;
}

void
PE_Service::Process_Observes()
{
    SetResult(done,stay);
    eState = at_ignore;
}

void
PE_Service::Process_Default()
{
    On_Default();
}


void
PE_Service::On_Default()
{
    if (eState == at_ignore)
        SetResult(done, stay);
    else
        SetResult(not_done, pop_failure);
}

void
PE_Service::InitData()
{
    eState = need_name;
    sData_Name.clear();
    bIsPreDeclaration = false;
    pCurService = 0;
    pCurSiService = 0;
    nCurService = 0;
    nCurParsed_Property = 0;
    nCurParsed_Type = 0;
    bOptionalMember = false;
}

void
PE_Service::TransferData()
{
    if (NOT bIsPreDeclaration)
    {
        csv_assert(! sData_Name.empty());
        csv_assert( (pCurService != 0) != (pCurSiService != 0) );
    }

    eState = e_none;
}

void
PE_Service::ReceiveData()
{
    switch (eState)
    {
        case in_property:
                eState = e_std;
                break;
        case in_ifc_type:
                if (bOptionalMember)
                {
                    pPE_Type->SetOptional();
                }
                pCurService->AddRef_SupportedInterface(
                                    nCurParsed_Type,
                                    pPE_Type->ReleaseDocu());
                nCurParsed_Type = 0;
                eState = expect_ifc_separator;
                break;
        case in_service_type:
                if (bOptionalMember)
                {
                    pPE_Type->SetOptional();
                }
                pCurService->AddRef_IncludedService(
                                    nCurParsed_Type,
                                    pPE_Type->ReleaseDocu());
                nCurParsed_Type = 0;
                eState = expect_service_separator;
                break;
        case need_base_interface:
                pCurSiService = &Gate().Ces().Store_SglIfcService(
                                                CurNamespace().CeId(),
                                                sData_Name,
                                                nCurParsed_Type );
                nCurService = pCurSiService->CeId();
                PassDocuAt(*pCurSiService);

                nCurParsed_Type = 0;
                eState = need_curlbr_open_sib;
                break;
        case e_std_sib:
                break;
        default:
            csv_assert(false);
    }

    bOptionalMember = false;
}


UnoIDL_PE &
PE_Service::MyPE()
{
    return *this;
}

void
PE_Service::StartProperty()
{
    SetResult(not_done, push_sure, pPE_Property.Ptr());
    eState = in_property;

    if (bOptionalMember)
    {
        pPE_Property->PresetOptional();
        bOptionalMember = false;
    }
}


}   // namespace uidl
}   // namespace csi

