/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_enval.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:54:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <precomp.h>
#include "pe_enval.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>
#include "pe_expr.hxx"



namespace cpp {


PE_EnumValue::PE_EnumValue( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_EnumValue> )
        // pSpExpression,
        // pSpuInitExpression
{
    Setup_StatusFunctions();

    pSpExpression       = new SP_Expression(*this);
    pSpuInitExpression  = new SPU_Expression(*pSpExpression, 0, &PE_EnumValue::SpReturn_InitExpression);
}

PE_EnumValue::~PE_EnumValue()
{
}

void
PE_EnumValue::Call_Handler( const cpp::Token &  i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_EnumValue::Setup_StatusFunctions()
{
    typedef CallFunction<PE_EnumValue>::F_Tok   F_Tok;

    static F_Tok stateF_start[] =           { &PE_EnumValue::On_start_Identifier };
    static INT16 stateT_start[] =           { Tid_Identifier };

    static F_Tok stateF_afterName[] =       { &PE_EnumValue::On_afterName_SwBracket_Right,
                                              &PE_EnumValue::On_afterName_Comma,
                                              &PE_EnumValue::On_afterName_Assign };
    static INT16 stateT_afterName[] =       { Tid_SwBracket_Right,
                                              Tid_Comma,
                                              Tid_Assign };

    static F_Tok stateF_expectFinish[] =    { &PE_EnumValue::On_expectFinish_SwBracket_Right,
                                              &PE_EnumValue::On_expectFinish_Comma };
    static INT16 stateT_expectFinish[] =    { Tid_SwBracket_Right,
                                              Tid_Comma };

    SEMPARSE_CREATE_STATUS(PE_EnumValue, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_EnumValue, afterName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_EnumValue, expectFinish, Hdl_SyntaxError);
}

void
PE_EnumValue::InitData()
{
    pStati->SetCur(start);

    sName.clear();
    sInitExpression.clear();
}

void
PE_EnumValue::TransferData()
{
    pStati->SetCur(size_of_states);

    ary::cpp::EnumValue &
    rEnVal = Env().AryGate().Ces().Store_EnumValue(
                     Env().Context(), sName, sInitExpression );
    Env().Event_Store_EnumValue(rEnVal);
}

void
PE_EnumValue::Hdl_SyntaxError( const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_EnumValue::SpReturn_InitExpression()
{
    pStati->SetCur(expectFinish);

    sInitExpression = pSpuInitExpression->Child().Result_Text();
}

void
PE_EnumValue::On_start_Identifier(const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(afterName);

    sName = i_sText;
}

void
PE_EnumValue::On_afterName_SwBracket_Right(const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_EnumValue::On_afterName_Comma(const char * )
{
    SetTokenResult(done, pop_success);
}

void
PE_EnumValue::On_afterName_Assign(const char * )
{
    pSpuInitExpression->Push(done);
}

void
PE_EnumValue::On_expectFinish_SwBracket_Right(const char * )
{
    SetTokenResult(not_done, pop_success);
}

void
PE_EnumValue::On_expectFinish_Comma(const char * )
{
    SetTokenResult(done, pop_success);
}


}   // namespace cpp




