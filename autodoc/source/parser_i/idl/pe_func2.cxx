/*************************************************************************
 *
 *  $RCSfile: pe_func2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <precomp.h>
#include <s2_luidl/pe_func2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/codeinf2.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/pe_vari2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <csi/l_uidl/function.hxx>


namespace csi
{
namespace uidl
{


PE_Function::PE_Function( RFunction &           o_rResult,
                          const RInterface &    i_rCurInterface )
    :   eState(e_none),
        pData(0),
        pResult(&o_rResult),
        pCurInterface(&i_rCurInterface),
        pPE_Type(0),
        aCurParsedType(0),
        // sName,
        pPE_Variable(0),
        eCurParsedParam_Direction(param_in),
        aCurParsedParam_Type(0)
        // sCurParsedParam_Name
{
    pPE_Type        = new PE_Type(aCurParsedType);
    pPE_Variable    = new PE_Variable(aCurParsedParam_Type, sCurParsedParam_Name);
}


void
PE_Function::EstablishContacts( UnoIDL_PE *              io_pParentPE,
                                ary::Repository &        io_rRepository,
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
            case TokStereotype::ste_const:
                        pData->Data().bConst = true;
                        SetResult(done, stay);
                        break;
            case TokStereotype::ste_oneway:
                        pData->Data().bOneway = true;
                        SetResult(done, stay);
                        break;
            default:
                        OnDefault();
        }   // end switch
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
                    pData->Data().sName = i_rToken.Text();
                    SetResult(done,stay);
                    eState = expect_params_list;
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
                    eCurParsedParam_Direction = param_in;
                    break;
        case TokParameterHandling::ph_out:
                    eCurParsedParam_Direction = param_out;
                    break;
        case TokParameterHandling::ph_inout:
                    eCurParsedParam_Direction = param_inout;
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
    }   // end switch
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
    csv_assert(1==2);
}

void
PE_Function::InitData()
{
    eState = e_start;
    pData = new Function;
    *pResult = 0;
    aCurParsedType = 0;
    eCurParsedParam_Direction = param_in;
    aCurParsedParam_Type = 0;
    sCurParsedParam_Name = "";
}

void
PE_Function::ReceiveData()
{
    switch (eState)
    {
        case in_return_type:
                pData->Data().pReturnType = aCurParsedType;
                aCurParsedType = 0;
                eState = expect_name;
                break;
        case in_parameter_variable:
                pData->Data().aParameters.push_back(
                        Parameter(  aCurParsedParam_Type,
                                    sCurParsedParam_Name,
                                    eCurParsedParam_Direction ) );
                sCurParsedParam_Name = "";
                aCurParsedParam_Type = 0;
                eCurParsedParam_Direction = param_in;
                eState = expect_parameter_separator;
                break;
        case in_exception:
                pData->Data().aRaisedExceptions.push_back(aCurParsedType);
                eState = expect_exception_separator;
                break;
        default:
            csv_assert(false);
    }
}

void
PE_Function::TransferData()
{
    *pResult = Gate().Store_Function( pCurInterface->Id(), *pData );
    PassDocuAt(pResult->Id());

    pData = 0;

    eState = e_none;
}

UnoIDL_PE &
PE_Function::MyPE()
{
    return *this;
}



}   // namespace uidl
}   // namespace csi

