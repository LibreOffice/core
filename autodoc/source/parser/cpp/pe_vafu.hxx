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


#ifndef ADC_CPP_PE_VAFU_HXX
#define ADC_CPP_PE_VAFU_HXX

// BASE CLASSES
#include "cpp_pe.hxx"
// USED SERVICES
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_vfflag.hxx>



namespace cpp
{

class PE_Type;
class PE_Variable;
class PE_Function;
class PE_Ignore;




class PE_VarFunc : public Cpp_PE
{
  public:
    enum E_State
    {
        start,
        expectCtor,
        afterClassDecl,     // Also used for after enum declaration.
        expectName,
        afterName,
        afterName_inErraneousTemplate,
        finished,
        finishedIncludingFunctionImplementation,
        size_of_states
    };
    enum E_ResultType
    {
        result_unknown = 0,
        result_ignore,          /// Used for class and enum declarations and predeclarations and for extern variables and functions.
        result_variable,
        result_function
    };

    typedef ary::cpp::E_Protection              E_Protection;


                        PE_VarFunc(
                            Cpp_PE *            i_pParent );
                        ~PE_VarFunc();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    const std::vector<ary::cpp::Ce_id> &
                        Result_Ids() const;
    ary::cpp::Type_id   Result_FrontType() const;
    const StringVector &
                        Result_Names() const;
    E_ResultType        Result_CeType() const;

  private:
    typedef SubPe< PE_VarFunc, PE_Type >        SP_Type;
    typedef SubPeUse< PE_VarFunc, PE_Type >     SPU_Type;
    typedef SubPe< PE_VarFunc, PE_Variable >    SP_Variable;
    typedef SubPeUse< PE_VarFunc, PE_Variable > SPU_Variable;
    typedef SubPe< PE_VarFunc, PE_Function >    SP_Function;
    typedef SubPeUse< PE_VarFunc, PE_Function > SPU_Function;
    typedef SubPe< PE_VarFunc, PE_Ignore >      SP_Ignore;
    typedef SubPeUse< PE_VarFunc, PE_Ignore >   SPU_Ignore;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_UnknownToken( const char *);

    void                SpInit_FunctionStd();
    void                SpInit_FunctionCtor();
    void                SpInit_FunctionDtor();
    void                SpInit_FunctionCastOperator();
    void                SpInit_FunctionNormalOperator();
    void                SpReturn_Type();
    void                SpReturn_Variable();
    void                SpReturn_FunctionStd();
    void                SpReturn_Ignore();

    void                On_start_Identifier(const char * i_sText);
    void                On_start_operator(const char * i_sText);
    void                On_start_TypeKey(const char * i_sText);
    void                On_start_virtual(const char * i_sText);
    void                On_start_Tilde(const char * i_sText);
    void                On_start_const(const char * i_sText);
    void                On_start_volatile(const char * i_sText);
    void                On_start_extern(const char * i_sText);
    void                On_start_static(const char * i_sText);
    void                On_start_mutable(const char * i_sText);
    void                On_start_register(const char * i_sText);
    void                On_start_inline(const char * i_sText);
    void                On_start_explicit(const char * i_sText);
    void                On_start_Bracket_Right(const char * i_sText);
    void                On_start_typename(const char * i_sText);

    void                On_expectCtor_Bracket_Left(const char * i_sText);

    void                On_afterClassDecl_Semicolon(const char * i_sText);

    void                On_expectName_Identifier(const char * i_sText);
    void                On_expectName_operator(const char * i_sText);
    void                On_expectName_Bracket_Left(const char * i_sText);

    void                On_afterName_ArrayBracket_Left(const char * i_sText);
    void                On_afterName_Bracket_Left(const char * i_sText);
    void                On_afterName_DoubleColon(const char * i_sText);
    void                On_afterName_Semicolon(const char * i_sText);
    void                On_afterName_Comma(const char * i_sText);
    void                On_afterName_Assign(const char * i_sText);
    void                On_afterName_Less(const char * i_sText);

    void                On_afterName_inErraneousTemplate_Less(const char * i_sText);
    void                On_afterName_inErraneousTemplate_Greater(const char * i_sText);
    void                On_afterName_inErraneousTemplate_Default(const char * i_sText);

    void                On_finished_Semicolon(const char * i_sText);
    void                On_finished_Comma(const char * i_sText);
    void                On_finished_Default(const char * i_sText);

    void                On_finishedIncludingFunctionImplementation_Default(const char * i_sText);

    ary::cpp::FunctionFlags
                        CreateFunctionFlags();

    // DATA
    Dyn< PeStatusArray<PE_VarFunc> >
                        pStati;

    Dyn<SP_Type>        pSpType;
    Dyn<SPU_Type>       pSpuType;
    Dyn<SP_Variable>    pSpVariable;
    Dyn<SPU_Variable>   pSpuVariable;
    Dyn<SP_Function>    pSpFunction;
    Dyn<SPU_Function>   pSpuFunctionStd;
    Dyn<SPU_Function>   pSpuFunctionCtor;
    Dyn<SPU_Function>   pSpuFunctionDtor;
    Dyn<SPU_Function>   pSpuFunctionCastOperator;
    Dyn<SPU_Function>   pSpuFunctionNormalOperator;
    Dyn<SP_Ignore>      pSpIgnore;
    Dyn<SPU_Ignore>     pSpuIgnore;

    intt                nCounter_TemplateBrackets;
    bool                bInDestructor;

    std::vector<ary::cpp::Ce_id>
                        aResultIds;
    ary::cpp::Type_id   nResultFrontType;
    E_ResultType        eResultType;

    // Pre-Results
    String              sName;

    bool                bVirtual;
    bool                bStatic;
    bool                bExtern;
    bool                bExternC;
    bool                bMutable;
    bool                bInline;
    bool                bRegister;
    bool                bExplicit;
};



// IMPLEMENTATION

inline const std::vector<ary::cpp::Ce_id> &
PE_VarFunc::Result_Ids() const
    { return aResultIds; }
inline ary::cpp::Type_id
PE_VarFunc::Result_FrontType() const
    { return nResultFrontType; }
inline PE_VarFunc::E_ResultType
PE_VarFunc::Result_CeType() const
    { return eResultType; }



}   // namespace cpp




#endif


/*  // Overview of Stati

Undecided
---------

start           // vor und während storage class specifiern
    any         ->stay
    operaator   ->goto Function

->Typ

expectName
    Identifier  ->stay
    operator    ->goto Function

afterName       ->goto Variable or Function




Variable
--------

start           // vor und während storage class specifiern

->Typ

expectName      // Typ ist da  -> im Falle von '(': notyetimplemented
afterName

expectSize      // after [
expectFinish
                // vor ; oder ,
expectNextVarName  // anders als bei expectName kann hier auch * oder & kommen





Function
--------

start               // vor und während storage class specifiern

->Typ

expectName          // Typ ist da
expectBracket       // Nach Name
expectParameter     // nach ( oder ,
-> Parameter
after Parameters    // before const, volatile throw or = 0.
after throw         // expect (
expectException     // after (
after exceptions    // = 0 oder ; oder ,


expectNextVarName  // anders als bei expectName kann hier auch * oder & kommen







*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
