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
#include "pe_type.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/inpcontx.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/cp_type.hxx>
#include "pe_class.hxx"
#include "pe_enum.hxx"
#include <x_parse.hxx>



class NullType : public ary::cpp::Type
{
  private:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    virtual ary::ClassId
                        get_AryClass() const;
    virtual bool        inq_IsConst() const;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
};

void
NullType::do_Accept(csv::ProcessorIfc & ) const
{
    // Does nothing.
}

ary::ClassId
NullType::get_AryClass() const
{
    return 0;
}

bool
NullType::inq_IsConst() const
{
    return true;
}

void
NullType::inq_Get_Text(		StreamStr &         ,
                            StreamStr &         ,
                            StreamStr &         ,
                            const ary::cpp::Gate &	) const
{
    // Does nothing.
}




namespace cpp
{


inline bool
PE_Type::IsType() const
    { return eResult_KindOf == is_type; }


PE_Type::PE_Type( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        pStati( new PeStatusArray<PE_Type> ),
        pSpType(0),
        pSpuType_TemplateParameter(0),
        // pSpClass,
        // pSpuClass,
        // pSpEnum,
        // pSpuEnum,
        pType(0),
        pCurTemplate_ParameterList(0),
        // sOwningClassName,
        // sParsedClass_Name,
        pResult_Type(0),
        eResult_KindOf(is_none),
        bIsCastOperatorType(false)
{
    Setup_StatusFunctions();

    pSpType         = new SP_Type(*this);
    pSpClass        = new SP_Class(*this);
    pSpEnum         = new SP_Enum(*this);

    pSpuType_TemplateParameter
                    = new SPU_Type(	 *pSpType, 0,
                                     &PE_Type::SpReturn_Type_TemplateParameter );
    pSpuClass  		= new SPU_Class( *pSpClass, 0,
                                &    PE_Type::SpReturn_Class );
    pSpuEnum  		= new SPU_Enum( *pSpEnum, 0,
                                &    PE_Type::SpReturn_Enum );
}

PE_Type::~PE_Type()
{
}

void
PE_Type::Init_AsCastOperatorType()
{
    bIsCastOperatorType = true;
}

void
PE_Type::Call_Handler( const cpp::Token & i_rTok )
{
    pStati->Cur().Call_Handler(i_rTok.TypeId(), i_rTok.Text());
}

void
PE_Type::Setup_StatusFunctions()
{
    typedef CallFunction<PE_Type>::F_Tok	F_Tok;
    static F_Tok stateF_start[] =				{ &PE_Type::On_start_Identifier,
                                                  &PE_Type::On_start_class,
                                                  &PE_Type::On_start_class,
                                                  &PE_Type::On_start_class,
                                                  &PE_Type::On_start_enum,
                                                  &PE_Type::On_start_const,
                                                  &PE_Type::On_start_volatile,
                                                  &PE_Type::On_start_Bracket_Right,
                                                  &PE_Type::On_start_DoubleColon,
                                                  &PE_Type::On_start_typename,
                                                  &PE_Type::On_start_BuiltInType,
                                                  &PE_Type::On_start_TypeSpecializer };
    static INT16 stateT_start[] =       		{ Tid_Identifier,
                                                  Tid_class,
                                                  Tid_struct,
                                                  Tid_union,
                                                  Tid_enum,
                                                  Tid_const,
                                                  Tid_volatile,
                                                  Tid_Bracket_Right,
                                                  Tid_DoubleColon,
                                                  Tid_typename,
                                                  Tid_BuiltInType,
                                                  Tid_TypeSpecializer };

    static F_Tok stateF_expect_namesegment[] =  { &PE_Type::On_expect_namesegment_Identifier,
                                                  &PE_Type::On_expect_namesegment_Identifier };
    static INT16 stateT_expect_namesegment[] =  { Tid_Identifier,
                                                  Tid_BuiltInType };

    static F_Tok stateF_after_namesegment[] =	{ &PE_Type::On_after_namesegment_const,
                                                  &PE_Type::On_after_namesegment_volatile,
                                                  &PE_Type::On_after_namesegment_Bracket_Left,
                                                  &PE_Type::On_after_namesegment_DoubleColon,
                                                  &PE_Type::On_after_namesegment_Less,
                                                  &PE_Type::On_after_namesegment_Asterix,
                                                  &PE_Type::On_after_namesegment_AmpersAnd };
    static INT16 stateT_after_namesegment[] =   { Tid_const,
                                                  Tid_volatile,
                                                  Tid_Bracket_Left,
                                                  Tid_DoubleColon,
                                                  Tid_Less,
                                                  Tid_Asterix,
                                                  Tid_AmpersAnd };

    static F_Tok stateF_afterclass_expect_semicolon[] =
                                                { &PE_Type::On_afterclass_expect_semicolon_Semicolon };
    static INT16 stateT_afterclass_expect_semicolon[] =
                                                { Tid_Semicolon };

    static F_Tok stateF_within_template[] =		{ &PE_Type::On_within_template_Comma,
                                                  &PE_Type::On_within_template_Greater,
                                                  &PE_Type::On_within_template_Constant };
    static INT16 stateT_within_template[] =   	{ Tid_Comma,
                                                  Tid_Greater,
                                                  Tid_Constant };

    static F_Tok stateF_within_indirection[] =	{ &PE_Type::On_within_indirection_const,
                                                  &PE_Type::On_within_indirection_volatile,
                                                  &PE_Type::On_within_indirection_Asterix,
                                                  &PE_Type::On_within_indirection_AmpersAnd };
    static INT16 stateT_within_indirection[] =  { Tid_const,
                                                  Tid_volatile,
                                                  Tid_Asterix,
                                                  Tid_AmpersAnd };

    SEMPARSE_CREATE_STATUS(PE_Type, start, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Type, expect_namesegment, On_EndOfType);
    SEMPARSE_CREATE_STATUS(PE_Type, after_namesegment, On_EndOfType);
    SEMPARSE_CREATE_STATUS(PE_Type, afterclass_expect_semicolon, Hdl_SyntaxError);
    SEMPARSE_CREATE_STATUS(PE_Type, within_template, On_within_template_TypeStart);
    SEMPARSE_CREATE_STATUS(PE_Type, within_indirection, On_EndOfType);
}

void
PE_Type::InitData()
{
    pStati->SetCur(start);

    ary::cpp::Ce_id
        scope_id = Env().Context().CurClass() != 0
                    ?   Env().Context().CurClass()->CeId()
                    :   Env().Context().CurNamespace().CeId();

    pType = new ary::cpp::UsedType(scope_id);
    pCurTemplate_ParameterList = 0;
    sOwningClassName
            =   Env().Context().CurClass() != 0
                    ?   Env().Context().CurClass()->LocalName().c_str()
                    :   "";
    sParsedClass_Name.clear();
    pResult_Type = 0;
    eResult_KindOf = is_type;
    bIsCastOperatorType = false;
}

void
PE_Type::TransferData()
{
    pStati->SetCur(size_of_states);

    if ( IsType() )
        pResult_Type = & Env().AryGate().Types().CheckIn_UsedType(
                                                        Env().Context(),
                                                        *pType.Release() );
    else
        pResult_Type = new NullType;
}

void
PE_Type::Hdl_SyntaxError( const char * i_sText )
{
    StdHandlingOfSyntaxError( i_sText );
}

void
PE_Type::SpReturn_Type_TemplateParameter()
{
    if ( pSpuType_TemplateParameter->Child().Result_KindOf() != is_type )
        throw X_Parser(X_Parser::x_UnspecifiedSyntaxError, "", String::Null_(), 0);

    pCurTemplate_ParameterList->AddParam_Type(
            pSpuType_TemplateParameter->Child().Result_Type().TypeId() );
}

void
PE_Type::SpReturn_Class()
{
    switch ( pSpuClass->Child().Result_KindOf() )
    {
        case PE_Class::is_declaration:
                    pStati->SetCur(afterclass_expect_semicolon);
                    eResult_KindOf = is_explicit_class_declaration;
                    break;
        case PE_Class::is_implicit_declaration:
                    pStati->SetCur(after_namesegment);
                    pType->Add_NameSegment(
                            pSpuClass->Child().Result_LocalName() );
                    break;
        case PE_Class::is_predeclaration:
                    pStati->SetCur(afterclass_expect_semicolon);
                    eResult_KindOf = is_class_predeclaration;
                    break;
        case PE_Class::is_qualified_typename:
                    pStati->SetCur(after_namesegment);
                    pType->Add_NameSegment(
                            pSpuClass->Child().Result_FirstNameSegment() );
                    break;
        default:
                    csv_assert(false);
    }
}

void
PE_Type::SpReturn_Enum()
{
    switch ( pSpuEnum->Child().Result_KindOf() )
    {
        case PE_Enum::is_declaration:
                    pStati->SetCur(afterclass_expect_semicolon);
                    eResult_KindOf = is_explicit_enum_declaration;
                    break;
        case PE_Enum::is_implicit_declaration:
                    pStati->SetCur(after_namesegment);
                    pType->Add_NameSegment(
                            pSpuEnum->Child().Result_LocalName() );
                    break;
        case PE_Enum::is_qualified_typename:
                    pStati->SetCur(after_namesegment);
                    pType->Add_NameSegment(
                            pSpuEnum->Child().Result_FirstNameSegment() );
                    break;
        default:
                    csv_assert(false);
    }
}

void
PE_Type::On_EndOfType(const char *)
{
    SetTokenResult(not_done, pop_success);
}

void
PE_Type::On_start_Identifier( const char * i_sText )
{
    SetTokenResult(done,stay);
    pStati->SetCur(after_namesegment);

    pType->Add_NameSegment(i_sText);
}

void
PE_Type::On_start_class(const char *)
{
    pSpuClass->Push(not_done);
}

void
PE_Type::On_start_enum(const char *)
{
    pSpuEnum->Push(done);
}

void
PE_Type::On_start_const(const char *)
{
    SetTokenResult(done,stay);
    pType->Set_Const();
}

void
PE_Type::On_start_volatile(const char *)
{
    SetTokenResult(done,stay);
    pType->Set_Volatile();
}

void
PE_Type::On_start_Bracket_Right(const char *)
{
    SetTokenResult(not_done,pop_success);

    eResult_KindOf = is_none;
}

void
PE_Type::On_start_DoubleColon(const char *)
{
    SetTokenResult(done,stay);
    pType->Set_Absolute();
}

void
PE_Type::On_start_BuiltInType(const char * i_sText)
{
    SetTokenResult(done, stay);
    pStati->SetCur(after_namesegment);
    pType->Set_BuiltIn(i_sText);
}

void
PE_Type::On_start_TypeSpecializer(const char * i_sText)
{
    SetTokenResult(done,stay);
    if (*i_sText == 'u') {
        pType->Set_Unsigned();
    }
    else if (*i_sText == 's') {
        pType->Set_Signed();
    }
    else {
        csv_assert(false);
    }
}

void
PE_Type::On_start_typename(const char *)
{
    SetTokenResult(done,stay);
}

void
PE_Type::On_expect_namesegment_Identifier(const char * i_sText)
{
    SetTokenResult(done,stay);
    pStati->SetCur(after_namesegment);
    pType->Add_NameSegment(i_sText);
}

void
PE_Type::On_after_namesegment_const(const char *)
{
    SetTokenResult(done,stay);
    pType->Set_Const();
}

void
PE_Type::On_after_namesegment_volatile(const char *)
{
    SetTokenResult(done,stay);
    pType->Set_Volatile();
}

void
PE_Type::On_after_namesegment_Bracket_Left(const char * i_sText)
{
    if ( bIsCastOperatorType )
    {
        SetTokenResult(not_done, pop_success);
    }
    else if ( pType->LocalName() == sOwningClassName )
    {
        SetTokenResult(not_done,pop_success);
        eResult_KindOf = is_constructor;

    }
    else //
    {
        On_EndOfType(i_sText);
    }  // endif
}

void
PE_Type::On_after_namesegment_DoubleColon(const char *)
{
    SetTokenResult(done,stay);
    pStati->SetCur(expect_namesegment);
}

void
PE_Type::On_after_namesegment_Less(const char *)
{
    SetTokenResult(done,stay);
    pStati->SetCur(within_template);

    pCurTemplate_ParameterList = & pType->Enter_Template();
}

void
PE_Type::On_after_namesegment_Asterix(const char *)
{
    SetTokenResult(done,stay);
    pStati->SetCur(within_indirection);
    pType->Add_PtrLevel();
}

void
PE_Type::On_after_namesegment_AmpersAnd(const char *)
{
    SetTokenResult(done,pop_success);
    pType->Set_Reference();
}

void
PE_Type::On_afterclass_expect_semicolon_Semicolon(const char *)
{
    csv_assert( NOT IsType() );
    SetTokenResult(not_done,pop_success);
}

void
PE_Type::On_within_template_Comma(const char *)
{
    SetTokenResult(done,stay);
}

void
PE_Type::On_within_template_Greater(const char *)
{
    SetTokenResult(done,stay);
    pStati->SetCur(after_namesegment);

    pCurTemplate_ParameterList = 0;
}

void
PE_Type::On_within_template_Constant(const char * i_sText)
{
    // KORR_FUTURE
    Cerr() << "Templates with constants as parameters are not yet supported by Autodoc" << Endl();
    Hdl_SyntaxError(i_sText);
}

void
PE_Type::On_within_template_TypeStart(const char *)
{
    pSpuType_TemplateParameter->Push(not_done);
}

void
PE_Type::On_within_indirection_const(const char *)
{
    SetTokenResult(done,stay);
    pType->Set_Const();
}

void
PE_Type::On_within_indirection_volatile(const char *)
{
    SetTokenResult(done,stay);
    pType->Set_Volatile();
}

void
PE_Type::On_within_indirection_Asterix(const char *)
{
    SetTokenResult(done,stay);
    pType->Add_PtrLevel();
}

void
PE_Type::On_within_indirection_AmpersAnd(const char *)
{
    SetTokenResult(done,pop_success);
    pType->Set_Reference();
}

}   // namespace cpp





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
