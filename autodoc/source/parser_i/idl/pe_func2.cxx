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
#include <s2_luidl/pe_func2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_function.hxx>
#include <ary/idl/i_type.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/pe_vari2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <x_parse2.hxx>


namespace csi
{
namespace uidl
{


PE_Function::PE_Function( const RParent &	    i_rCurInterface )
    :	eState(e_none),
        sData_Name(),
        nData_ReturnType(0),
        bData_Oneway(false),
        pCurFunction(0),
        pCurParent(&i_rCurInterface),
        pPE_Type(0),
        nCurParsedType(0),
        sName(),
        pPE_Variable(0),
        eCurParsedParam_Direction(ary::idl::param_in),
        nCurParsedParam_Type(0),
        sCurParsedParam_Name(),
        bIsForConstructors(false)
{
    pPE_Type 		= new PE_Type(nCurParsedType);
    pPE_Variable 	= new PE_Variable(nCurParsedParam_Type, sCurParsedParam_Name);
}

PE_Function::PE_Function( const RParent &	  i_rCurService,
                          E_Constructor        )
    :	eState(expect_name),
        sData_Name(),
        nData_ReturnType(0),
        bData_Oneway(false),
        pCurFunction(0),
        pCurParent(&i_rCurService),
        pPE_Type(0),
        nCurParsedType(0),
        sName(),
        pPE_Variable(0),
        eCurParsedParam_Direction(ary::idl::param_in),
        nCurParsedParam_Type(0),
        sCurParsedParam_Name(),
        bIsForConstructors(true)
{
    pPE_Type 		= new PE_Type(nCurParsedType);
    pPE_Variable 	= new PE_Variable(nCurParsedParam_Type, sCurParsedParam_Name);
}

void
PE_Function::EstablishContacts( UnoIDL_PE *				 io_pParentPE,
                                ary::Repository &	     io_rRepository,
                                TokenProcessing_Result & o_rResult )
{
    UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
    pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
    pPE_Variable->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Function::~PE_Function()
{
}

void
PE_Function::ProcessToken( const Token & i_rToken )
{
    i_rToken.Trigger(*this);
}

void
PE_Function::Process_Stereotype( const TokStereotype & i_rToken )
{
    if (eState == e_start)
    {
        switch (i_rToken.Id())
        {
            case TokStereotype::ste_oneway:
                        bData_Oneway = true;
                        SetResult(done, stay);
                        break;
            default:
                        OnDefault();
        }	// end switch
    }
    else
        OnDefault();
}

void
PE_Function::Process_Identifier( const TokIdentifier & i_rToken )
{
    switch (eState)
    {
        case e_start:
                    GoIntoReturnType();
                    break;
        case expect_name:
                    sData_Name = i_rToken.Text();
                    SetResult(done,stay);
                    eState = expect_params_list;

                    if (NOT bIsForConstructors)
                    {
                        pCurFunction = &Gate().Ces().Store_Function(
                                                        *pCurParent,
                                                        sData_Name,
                                                        nData_ReturnType,
                                                        bData_Oneway );
                    }
                    else
                    {
                        pCurFunction = &Gate().Ces().Store_ServiceConstructor(
                                                        *pCurParent,
                                                        sData_Name );
                    }
                    PassDocuAt(*pCurFunction);
                    break;
        case expect_parameter_variable:
                    GoIntoParameterVariable();
                    break;
        case expect_exception:
                    GoIntoException();
                    break;
        default:
                    OnDefault();
    }
}

void
PE_Function::Process_Punctuation( const TokPunctuation & i_rToken )
{
    switch (eState)
    {
        case e_start:
                    SetResult(done,stay);
                    break;
        case expect_params_list:
                    if (i_rToken.Id() != TokPunctuation::BracketOpen)
                    {
                        OnDefault();
                        return;
                    }
                    SetResult(done,stay);
                    eState = expect_parameter;
                    break;
        case expect_parameter:
                    if (i_rToken.Id() == TokPunctuation::BracketClose)
                    {
                        SetResult(done,stay);
                        eState = params_finished;
                    }
                    else
                    {
                        OnDefault();
                        return;
                    }
                    break;
        case expect_parameter_separator:
                    if (i_rToken.Id() == TokPunctuation::Comma)
                    {
                        SetResult(done,stay);
                        eState = expect_parameter;
                    }
                    else if (i_rToken.Id() == TokPunctuation::BracketClose)
                    {
                        SetResult(done,stay);
                        eState = params_finished;
                    }
                    else
                    {
                        OnDefault();
                        return;
                    }
                    break;
        case params_finished:
        case exceptions_finished:
                    if (i_rToken.Id() != TokPunctuation::Semicolon)
                    {
                        OnDefault();
                        return;
                    }
                    SetResult(done,pop_success);
                    eState = e_none;
                    break;
        case expect_exceptions_list:
                    if (i_rToken.Id() != TokPunctuation::BracketOpen)
                    {
                        OnDefault();
                        return;
                    }
                    SetResult(done,stay);
                    eState = expect_exception;
                    break;
        case expect_exception_separator:
                    if (i_rToken.Id() == TokPunctuation::Comma)
                    {
                        SetResult(done,stay);
                        eState = expect_exception;
                    }
                    else if (i_rToken.Id() == TokPunctuation::BracketClose)
                    {
                        SetResult(done,stay);
                        eState = exceptions_finished;
                    }
                    else
                    {
                        OnDefault();
                        return;
                    }
                    break;
        default:
                    OnDefault();
    }
}

void
PE_Function::Process_BuiltInType( const TokBuiltInType & i_rToken )
{
    switch (eState)
    {
        case e_start:
                GoIntoReturnType();
                break;
        case expect_parameter_variable:
                   GoIntoParameterVariable();
                break;
        case expect_parameter_separator:
                if (i_rToken.Id() != TokBuiltInType::bty_ellipse)
                {
                    OnDefault();
                }
                else
                {
                    pCurFunction->Set_Ellipse();
                       SetResult(done,stay);
                    // eState stays the same, because we wait for the closing ")" now.
                }
                break;
        case expect_exception:
                GoIntoException();
                break;
        default:
            OnDefault();
    }	// end switch
}

void
PE_Function::Process_ParameterHandling( const TokParameterHandling & i_rToken )
{
    if (eState != expect_parameter)
    {
        OnDefault();
        return;
    }

    switch (i_rToken.Id())
    {
        case TokParameterHandling::ph_in:
                    eCurParsedParam_Direction = ary::idl::param_in;
                    break;
        case TokParameterHandling::ph_out:
                    eCurParsedParam_Direction = ary::idl::param_out;
                    break;
        case TokParameterHandling::ph_inout:
                    eCurParsedParam_Direction = ary::idl::param_inout;
                    break;
        default:
                    csv_assert(false);
    }
    SetResult(done,stay);
    eState = expect_parameter_variable;
}

void
PE_Function::Process_Raises()
{
    if (eState != params_finished)
    {
        OnDefault();
        return;
    }
    SetResult(done,stay);
    eState = expect_exceptions_list;
}

void
PE_Function::Process_Default()
{
    switch (eState)
    {
        case e_start:
                GoIntoReturnType();
                break;
        case expect_parameter_variable:
                GoIntoParameterVariable();
                break;
        case expect_exception:
                GoIntoException();
                break;
        default:
            OnDefault();
    }	// end switch
}

void
PE_Function::GoIntoReturnType()
{
    SetResult(not_done, push_sure, pPE_Type.Ptr());
    eState = in_return_type;
}

void
PE_Function::GoIntoParameterVariable()
{
    SetResult(not_done, push_sure, pPE_Variable.Ptr());
    eState = in_parameter_variable;
}

void
PE_Function::GoIntoException()
{
    SetResult(not_done, push_sure, pPE_Type.Ptr());
    eState = in_exception;
}

void
PE_Function::OnDefault()
{
    throw X_AutodocParser(X_AutodocParser::x_Any);
}

void
PE_Function::InitData()
{
    eState = e_start;

    sData_Name.clear();
    nData_ReturnType = 0;
    bData_Oneway = false;
    pCurFunction = 0;

    nCurParsedType = 0;
    eCurParsedParam_Direction = ary::idl::param_in;
    nCurParsedParam_Type = 0;
    sCurParsedParam_Name.clear();

    if (bIsForConstructors)
    {
        eState = expect_name;
    }
}

void
PE_Function::ReceiveData()
{
    switch (eState)
    {
        case in_return_type:
                nData_ReturnType = nCurParsedType;
                nCurParsedType = 0;
                eState = expect_name;
                break;
        case in_parameter_variable:
                csv_assert(pCurFunction != 0);
                pCurFunction->Add_Parameter(
                                    sCurParsedParam_Name,
                                    nCurParsedParam_Type,
                                    eCurParsedParam_Direction );
                sCurParsedParam_Name = "";
                nCurParsedParam_Type = 0;
                eCurParsedParam_Direction = ary::idl::param_in;
                eState = expect_parameter_separator;
                break;
        case in_exception:
                csv_assert(pCurFunction != 0);
                pCurFunction->Add_Exception(nCurParsedType);
                eState = expect_exception_separator;
                break;
        default:
            csv_assert(false);
    }   // end switch
}

void
PE_Function::TransferData()
{
    pCurFunction = 0;
    eState = e_none;
}

UnoIDL_PE &
PE_Function::MyPE()
{
    return *this;
}



}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
