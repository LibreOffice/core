/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_funct.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:26:31 $
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
#include "pe_funct.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/cpp/inpcontx.hxx>
#include <ary/cpp/c_rwgate.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/ca_type.hxx>
#include "pe_type.hxx"
#include "pe_param.hxx"




namespace cpp {


inline void
PE_Function::PerformFinishingPunctuation()
{ SetTokenResult(not_done,pop_success);
}


PE_Function::PE_Function( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Function> ),
        // pSpParameter,
        // pSpuParameter,
        // pSpType,
        // pSpuException,
        // pSpuCastOperatorType,
        nResult(0),
        bResult_WithImplementation(false),
        aName(60),
        eVirtuality(ary::cpp::VIRTUAL_none),
        eConVol(ary::cpp::CONVOL_none),
        // aFlags,
        nReturnType(0),
        // aParameters
        // aExceptions,
        bThrow(false),
        nBracketCounterInImplementation(0)
{
    Setup_StatusFunctions();

    pSpParameter     = new SP_Parameter(*this);
    pSpType          = new SP_Type(*this);

    pSpuParameter           = new SPU_Parameter(*pSpParameter, 0, &PE_Function::SpReturn_Parameter);
    pSpuException           = new SPU_Type(*pSpType, 0, &PE_Function::SpReturn_Exception);
    pSpuCastOperatorType    = new SPU_Type(*pSpType, &PE_Function::SpInit_CastOperatorType, &PE_Function::SpReturn_CastOperatorType);
}

PE_Function::~PE_Function()
{

}

void
PE_Function::Init_Std( const udmstri &          i_sName,
                       ary::Tid                 i_nReturnType,
                       bool                     i_bVirtual,
                       ary::cpp::FunctionFlags  i_aFlags )
{
    aName << i_sName;
    eVirtuality = i_bVirtual ? ary::cpp::VIRTUAL_virtual : ary::cpp::VIRTUAL_none;
    aFlags = i_aFlags;
    nReturnType = i_nReturnType;
    pStati->SetCur(afterName);
}

void
PE_Function::Init_Ctor( const udmstri &         i_sName,
                        ary::cpp::FunctionFlags i_aFlags )
{
    aName << i_sName;
    eVirtuality = ary::cpp::VIRTUAL_none;
    aFlags = i_aFlags;
    nReturnType = 0;
    pStati->SetCur(afterName);
}

void
PE_Function::Init_Dtor( const udmstri &         i_sName,
                        bool                    i_bVirtual,
                        ary::cpp::FunctionFlags i_aFlags )
{
    aName << "~" << i_sName;
    eVirtuality = i_bVirtual ? ary::cpp::VIRTUAL_virtual : ary::cpp::VIRTUAL_none;
    aFlags = i_aFlags;
    nReturnType = 0;
    pStati->SetCur(afterName);
}

void
PE_Function::Init_CastOperator( bool                    i_bVirtual,
                                ary::cpp::FunctionFlags i_aFlags )
{
    aName << "operator ";
    eVirtuality = i_bVirtual ? ary::cpp::VIRTUAL_virtual : ary::cpp::VIRTUAL_none;
    aFlags = i_aFlags;
    nReturnType = 0;
    pStati->SetCur(afterCastOperator);
}

void
PE_Function::Init_NormalOperator( ary::Tid                  i_nReturnType,
                                  bool                      i_bVirtual,
                                  ary::cpp::FunctionFlags   i_aFlags )
{
    aName << "operator";
    eVirtuality = i_bVirtual ? ary::cpp::VIRTUAL_virtual : ary::cpp::VIRTUAL_none;
    aFlags = i_aFlags;
    nReturnType = i_nReturnType;
    pStati->SetCur(afterStdOperator);
}

ary::Cid
PE_Function::Result_Id() const
{
    return nResult;
}

void
PE_Function::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Function::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Function>::F_Tok    F_Tok;

    static F_Tok stateF_afterStdOperator[] =
                                            { &PE_Function::On_afterOperator_Std_Operator,
                                              &PE_Function::On_afterOperator_Std_LeftBracket,
                                              &PE_Function::On_afterOperator_Std_LeftBracket,
                                              &PE_Function::On_afterOperator_Std_Operator,
                                              &PE_Function::On_afterOperator_Std_Operator,
                                              &PE_Function::On_afterOperator_Std_Operator,
                                              &PE_Function::On_afterOperator_Std_Operator,
                                              &PE_Function::On_afterOperator_Std_Operator,
                                              &PE_Function::On_afterOperator_Std_Operator };
    static INT16 stateT_afterStdOperator[] =
                                            { Tid_Operator,
                                              Tid_ArrayBracket_Left,
                                              Tid_Bracket_Left,
                                              Tid_Comma,
                                              Tid_Assign,
                                              Tid_Less,
                                              Tid_Greater,
                                              Tid_Asterix,
                                              Tid_AmpersAnd };

    static F_Tok stateF_afterStdOperatorLeftBracket[] =
                                            { &PE_Function::On_afterStdOperatorLeftBracket_RightBracket,
                                              &PE_Function::On_afterStdOperatorLeftBracket_RightBracket };
    static INT16 stateT_afterStdOperatorLeftBracket[] =
                                            { Tid_ArrayBracket_Right,
                                              Tid_Bracket_Right };

    static F_Tok stateF_afterCastOperator[] =
                                            { &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type,
                                              &PE_Function::On_afterOperator_Cast_Type };
    static INT16 stateT_afterCastOperator[] =
                                            { Tid_Identifier,
                                              Tid_class,
                                              Tid_struct,
                                              Tid_union,
                                              Tid_enum,
                                              Tid_const,
                                              Tid_volatile,
                                              Tid_DoubleColon,
                                              Tid_BuiltInType,
                                              Tid_TypeSpecializer,
                                              Tid_typename };

    static F_Tok stateF_afterName[] =       { &PE_Function::On_afterName_Bracket_Left };
    static INT16 stateT_afterName[] =       { Tid_Bracket_Left };

    static F_Tok stateF_expectParameterSeparator[] =
                                            { &PE_Function::On_expectParameterSeparator_BracketRight,
                                              &PE_Function::On_expectParameterSeparator_Comma };
    static INT16 stateT_expectParameterSeparator[] =
                                            { Tid_Bracket_Right,
                                              Tid_Comma };

    static F_Tok stateF_afterParameters[] = { &PE_Function::On_afterParameters_const,
                                              &PE_Function::On_afterParameters_volatile,
                                              &PE_Function::On_afterParameters_throw,
                                              &PE_Function::On_afterParameters_SwBracket_Left,
                                              &PE_Function::On_afterParameters_Semicolon,
                                              &PE_Function::On_afterParameters_Comma,
                                              &PE_Function::On_afterParameters_Colon,
                                              &PE_Function::On_afterParameters_Assign };
    static INT16 stateT_afterParameters[] = { Tid_const,
                                              Tid_volatile,
                                              Tid_throw,
                                              Tid_SwBracket_Left,
                                              Tid_Semicolon,
                                              Tid_Comma,
                                              Tid_Colon,
                                              Tid_Assign };

    static F_Tok stateF_afterThrow[] =      { &PE_Function::On_afterThrow_Bracket_Left };
    static INT16 stateT_afterThrow[] =      { Tid_Bracket_Left };

    static F_Tok stateF_expectExceptionSeparator[] =
                                            { &PE_Function::On_expectExceptionSeparator_BracketRight,
                                              &PE_Function::On_expectExceptionSeparator_Comma };
    static INT16 stateT_expectExceptionSeparator[] =
                                            { Tid_Bracket_Right,
                                              Tid_Comma };

    static F_Tok stateF_afterExceptions[] = { &PE_Function::On_afterExceptions_SwBracket_Left,
                                              &PE_Function::On_afterExceptions_Semicolon,
                                              &PE_Function::On_afterExceptions_Comma,
                                              &PE_Function::On_afterExceptions_Colon,
                                              &PE_Function::On_afterExceptions_Assign };
    static INT16 stateT_afterExceptions[] = { Tid_SwBracket_Left,
                                              Tid_Semicolon,
                                              Tid_Comma,
                                              Tid_Colon,
                                              Tid_Assign };

    static F_Tok stateF_expectZero[] =      { &PE_Function::On_expectZero_Constant };
    static INT16 stateT_expectZero[] =      { Tid_Constant };

    static F_Tok stateF_inImplementation[] =
                                            { &PE_Function::On_inImplementation_SwBracket_Left,
                                              &PE_Function::On_inImplementation_SwBracket_Right };
    static INT16 stateT_inImplementation[] =
                                            { Tid_SwBracket_Left,
                                              Tid_SwBracket_Right };

    SEMPARSE_CREATE_STATUS(PE_Function, afterStdOperator, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, afterStdOperatorLeftBracket, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, afterCastOperator, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, afterName, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, expectParameterSeparator, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, afterParameters, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, afterThrow, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, expectExceptionSeparator, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, afterExceptions, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, expectZero, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Function, inImplementation, On_inImplementation_Default );
}

void
PE_Function::InitData()
{
    pStati->SetCur( afterName ),
    nResult = 0;
    bResult_WithImplementation = false;
    aName.seekp(0);
    eVirtuality = ary::cpp::VIRTUAL_none;
    eConVol = ary::cpp::CONVOL_none;
    aFlags.Reset();
    nReturnType = 0;
    csv::erase_container(aParameters);
    csv::erase_container(aExceptions);
    bThrow = false;
}

void
PE_Function::TransferData()
{
    udmstri sName( aName.c_str() );
    ary::cpp::Function & rFunction =
            Env().AryGate().Store_Operation(
                    Env().Context(),
                    sName,
                    nReturnType,
                    aParameters,
                    eVirtuality,
                    eConVol,
                    aFlags,
                    bThrow,
                    aExceptions );

    Dyn< StringVector >
            pTplParams ( Env().Get_CurTemplateParameters() );
    if ( pTplParams )
    {
          for ( StringVector::const_iterator it = pTplParams->begin();
              it !=  pTplParams->end();
              ++it )
        {
            rFunction.Add_TemplateParameterType( *it, 0 );
        }  // end for
    }

    Env().Event_Store_Function(rFunction);

    pStati->SetCur(size_of_states);
}

void
PE_Function::Hdl_SyntaxError(const char * i_sText)
{
    StdHandlingOfSyntaxError(i_sText);
}

void
PE_Function::SpInit_CastOperatorType()
{
    pSpuCastOperatorType->Child().Init_AsCastOperatorType();
}

void
PE_Function::SpReturn_Parameter()
{
    pStati->SetCur(expectParameterSeparator);

    ary::Tid nParamType = pSpuParameter->Child().Result_FrontType();
    if ( nParamType != 0 )      // Check, if there was a parameter, or only the closing ')'.
    {
        aParameters.push_back( pSpuParameter->Child().Result_ParamInfo() );
    }
}

void
PE_Function::SpReturn_Exception()
{
    pStati->SetCur(expectExceptionSeparator);

    ary::Tid nException = pSpuException->Child().Result_Type().Id();
    if ( nException != 0 AND pSpuException->Child().Result_KindOf() == PE_Type::is_type )
    {
        aExceptions.push_back( nException );
    }
}

void
PE_Function::SpReturn_CastOperatorType()
{
    pStati->SetCur(afterName);

    Env().AryGate().RoGate().Get_TypeText(
            aName, pSpuCastOperatorType->Child().Result_Type().Id() );
}

void
PE_Function::On_afterOperator_Std_Operator(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(afterName);

    if ( 'a' <= *i_sText AND *i_sText <= 'z' )
        aName << ' ';
    aName << i_sText;
}

void
PE_Function::On_afterOperator_Std_LeftBracket(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(afterStdOperatorLeftBracket);

    aName << i_sText;
}

void
PE_Function::On_afterStdOperatorLeftBracket_RightBracket(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(afterName);

    aName << i_sText;
}

void
PE_Function::On_afterOperator_Cast_Type(const char * i_sText)
{
    pSpuCastOperatorType->Push(not_done);
}

void
PE_Function::On_afterName_Bracket_Left(const char * i_sText)
{
    pSpuParameter->Push(done);
}

void
PE_Function::On_expectParameterSeparator_BracketRight(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(afterParameters);
}

void
PE_Function::On_expectParameterSeparator_Comma(const char * i_sText)
{
    pSpuParameter->Push(done);
}

void
PE_Function::On_afterParameters_const(const char * i_sText)
{
    SetTokenResult(done,stay);
    eConVol = static_cast<E_ConVol>(
                    static_cast<int>(eConVol) | static_cast<int>(ary::cpp::CONVOL_const) );
}

void
PE_Function::On_afterParameters_volatile(const char * i_sText)
{
    SetTokenResult(done,stay);
    eConVol = static_cast<E_ConVol>(
                    static_cast<int>(eConVol) | static_cast<int>(ary::cpp::CONVOL_volatile) );
}

void
PE_Function::On_afterParameters_throw(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(afterThrow);
    bThrow = true;
}

void
PE_Function::On_afterParameters_SwBracket_Left(const char * i_sText)
{
    EnterImplementation(1);
}

void
PE_Function::On_afterParameters_Semicolon(const char * i_sText)
{
    PerformFinishingPunctuation();
}

void
PE_Function::On_afterParameters_Comma(const char * i_sText)
{
    PerformFinishingPunctuation();
}

void
PE_Function::On_afterParameters_Colon(const char * i_sText)
{
    EnterImplementation(0);
}

void
PE_Function::On_afterParameters_Assign(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(expectZero);
}

void
PE_Function::On_afterThrow_Bracket_Left(const char * i_sText)
{
    pSpuException->Push(done);
}

void
PE_Function::On_expectExceptionSeparator_BracketRight(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(afterExceptions);
}

void
PE_Function::On_expectExceptionSeparator_Comma(const char * i_sText)
{
    pSpuException->Push(done);
}

void
PE_Function::On_afterExceptions_SwBracket_Left(const char * i_sText)
{
    EnterImplementation(1);
}

void
PE_Function::On_afterExceptions_Semicolon(const char * i_sText)
{
    PerformFinishingPunctuation();
}

void
PE_Function::On_afterExceptions_Comma(const char * i_sText)
{
    PerformFinishingPunctuation();
}

void
PE_Function::On_afterExceptions_Colon(const char * i_sText)
{
    EnterImplementation(0);
}

void
PE_Function::On_afterExceptions_Assign(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(expectZero);
}

void
PE_Function::On_expectZero_Constant(const char * i_sText)
{
    if ( strcmp(i_sText,"0") != 0 )
        Hdl_SyntaxError(i_sText);

    SetTokenResult(done,stay);
    pStati->SetCur(afterExceptions);

    eVirtuality = ary::cpp::VIRTUAL_abstract;
}

void
PE_Function::On_inImplementation_SwBracket_Left(const char * i_sText)
{
    SetTokenResult(done,stay);
    nBracketCounterInImplementation++;
}

void
PE_Function::On_inImplementation_SwBracket_Right(const char * i_sText)
{
    nBracketCounterInImplementation--;
    if (nBracketCounterInImplementation == 0)
    {
        SetTokenResult(done,pop_success);
    }
    else
    {
        SetTokenResult(done,stay);
    }
}

void
PE_Function::On_inImplementation_Default(const char * i_sText)
{
    SetTokenResult(done,stay);
}

void
PE_Function::EnterImplementation( intt i_nBracketCountStart )
{
    SetTokenResult(done,stay);
    pStati->SetCur(inImplementation);

    bResult_WithImplementation = true;
    nBracketCounterInImplementation = i_nBracketCountStart;
    if ( Env().Context().CurClass() != 0 )
    {
        aFlags.SetInline();
    }
}



}   // namespace cpp





