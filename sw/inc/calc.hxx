/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_INC_CALC_HXX
#define INCLUDED_SW_INC_CALC_HXX

#include <memory>
#include <vector>
#include <unordered_map>
#include <i18nlangtag/lang.h>
#include <basic/sbxvar.hxx>
#include <unotools/syslocale.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/solar.h>
#include <tools/long.hxx>
#include <o3tl/safeint.hxx>
#include "swdllapi.h"

class CharClass;
class LocaleDataWrapper;
class SwFieldType;
class SwDoc;
class SwUserFieldType;

const sal_Unicode cListDelim    = '|';

enum SwCalcOper
{
    CALC_NAME,              CALC_NUMBER,            CALC_ENDCALC,
    CALC_PLUS='+',          CALC_MINUS='-',         CALC_MUL='*',
    CALC_DIV='/',           CALC_PRINT=';',         CALC_ASSIGN='=',
    CALC_LP='(',            CALC_RP=')',            CALC_PHD='%',
    CALC_POW='^',
    CALC_NOT=256,           CALC_AND=257,           CALC_OR=258,
    CALC_XOR=259,           CALC_EQ=260,            CALC_NEQ=261,
    CALC_LEQ=262,           CALC_GEQ=263,           CALC_LES=264,
    CALC_GRE=265,           CALC_SUM=266,           CALC_MEAN=267,
    CALC_SQRT=268,          CALC_MIN=269,           CALC_MIN_IN=270,
    CALC_MAX=271,           CALC_MAX_IN=272,        CALC_SIN=273,
    CALC_COS=274,           CALC_TAN=275,           CALC_ASIN=276,
    CALC_ACOS=278,          CALC_ATAN=279,          CALC_TDIF=280,
    CALC_ROUND=281,         CALC_DATE=282,          CALC_MONTH=283,
    CALC_DAY=284,           CALC_PRODUCT=285,       CALC_AVERAGE=286,
    CALC_COUNT=287,         CALC_SIGN=288,          CALC_ABS=289,
    CALC_INT=290
};

// Calculate Operations Strings
inline constexpr OUString sCalc_Add  = u"add"_ustr;
inline constexpr OUString sCalc_Sub  = u"sub"_ustr;
inline constexpr OUString sCalc_Mul  = u"mul"_ustr;
inline constexpr OUString sCalc_Div  = u"div"_ustr;
inline constexpr OUString sCalc_Phd  = u"phd"_ustr;
inline constexpr OUString sCalc_Sqrt = u"sqrt"_ustr;
inline constexpr OUString sCalc_Pow  = u"pow"_ustr;
inline constexpr OUString sCalc_Or   = u"or"_ustr;
inline constexpr OUString sCalc_Xor  = u"xor"_ustr;
inline constexpr OUString sCalc_And  = u"and"_ustr;
inline constexpr OUString sCalc_Not  = u"not"_ustr;
inline constexpr OUString sCalc_Eq   = u"eq"_ustr;
inline constexpr OUString sCalc_Neq  = u"neq"_ustr;
inline constexpr OUString sCalc_Leq  = u"leq"_ustr;
inline constexpr OUString sCalc_Geq  = u"geq"_ustr;
inline constexpr OUString sCalc_L    = u"l"_ustr;
inline constexpr OUString sCalc_G    = u"g"_ustr;
inline constexpr OUString sCalc_Sum  = u"sum"_ustr;
inline constexpr OUString sCalc_Mean = u"mean"_ustr;
inline constexpr OUString sCalc_Min  = u"min"_ustr;
inline constexpr OUString sCalc_Max  = u"max"_ustr;
inline constexpr OUString sCalc_Sin  = u"sin"_ustr;
inline constexpr OUString sCalc_Cos  = u"cos"_ustr;
inline constexpr OUString sCalc_Tan  = u"tan"_ustr;
inline constexpr OUString sCalc_Asin = u"asin"_ustr;
inline constexpr OUString sCalc_Acos = u"acos"_ustr;
inline constexpr OUString sCalc_Atan = u"atan"_ustr;
inline constexpr OUString sCalc_Round= u"round"_ustr;
inline constexpr OUString sCalc_Date = u"date"_ustr;
inline constexpr OUString sCalc_Product = u"product"_ustr;
inline constexpr OUString sCalc_Average = u"average"_ustr;
inline constexpr OUString sCalc_Count= u"count"_ustr;
inline constexpr OUString sCalc_Sign = u"sign"_ustr;
inline constexpr OUString sCalc_Abs  = u"abs"_ustr;
inline constexpr OUString sCalc_Int  = u"int"_ustr;

//  Calculate ErrorCodes
enum class SwCalcError
{
    NONE=0,
    NaN,              //  not a number (not an error, used for interoperability)
    Syntax,           //  syntax error
    DivByZero,        //  division by zero
    FaultyBrackets,   //  faulty brackets
    OverflowInPower,  //  overflow in power function
    Overflow,         //  overflow
};

class SwSbxValue final : public SbxValue
{
    bool m_bVoid;
    bool m_bDBvalue;
public:
    // always default to a number. otherwise it will become a SbxEMPTY
    SwSbxValue( tools::Long n = 0 ) : m_bVoid(false), m_bDBvalue(false)  { PutLong( n ); }
    SwSbxValue( const double& rD ) : m_bVoid(false), m_bDBvalue(false) { PutDouble( rD ); }

    bool GetBool() const;
    double GetDouble() const;
    SwSbxValue& MakeDouble();

    bool IsVoidValue() const {return m_bVoid;}
    void SetVoidValue(bool bSet) {m_bVoid = bSet;}

    bool IsDBvalue() const {return m_bDBvalue;}
    void SetDBvalue(bool bSet) {m_bDBvalue = bSet;}
};

struct SwCalcExp
{
    SwSbxValue  nValue;
    const SwFieldType* pFieldType;

    SwCalcExp( SwSbxValue aVal, const SwFieldType* pFieldType );
};


// if CalcOp != 0, this is a valid operator
struct CalcOp;
CalcOp* FindOperator( const OUString& rSearch );

extern "C" typedef double (*pfCalc)(double);

class SwCalc
{
    std::unordered_map<OUString, SwCalcExp> m_aVarTable;
    OUStringBuffer m_aVarName;
    OUString    m_sCurrSym;
    OUString    m_sCommand;
    std::vector<const SwUserFieldType*> m_aRekurStack;
    SwSbxValue  m_nLastLeft;
    SwSbxValue  m_nNumberValue;
    SwCalcExp   m_aErrExpr;
    sal_Int32   m_nCommandPos;

    SwDoc&      m_rDoc;
    std::unique_ptr<LocaleDataWrapper> m_xLocaleDataWrapper;
    CharClass*  m_pCharClass;

    sal_uInt16      m_nListPor;
    bool        m_bHasNumber; // fix COUNT() and AVERAGE(), if all cells are NaN
    SwCalcOper  m_eCurrOper;
    SwCalcOper  m_eCurrListOper;
    SwCalcError m_eError;

    SwCalcOper  GetToken();
    SwSbxValue  Expr();
    SwSbxValue  Term();
    SwSbxValue  PrimFunc(bool &rChkPow);
    SwSbxValue  Prim();
    SwSbxValue  StdFunc(pfCalc pFnc, bool bChkTrig);

    static OUString  GetColumnName( const OUString& rName );
    OUString  GetDBName( std::u16string_view rName );

    SwCalc( const SwCalc& ) = delete;
    SwCalc& operator=( const SwCalc& ) = delete;

    void ImplDestroy();

public:
    SwCalc(SwDoc& rD);
    ~SwCalc();

    SwSbxValue  Calculate( const OUString &rStr );
    OUString    GetStrResult( const SwSbxValue& rValue );
    OUString    GetStrResult( double );

    SwCalcExp*  VarInsert( const OUString& r );
    SwCalcExp*  VarLook( const OUString &rStr, bool bIns = false );
    void        VarChange( const OUString& rStr, const SwSbxValue& rValue );
    void        VarChange( const OUString& rStr, double );
    std::unordered_map<OUString, SwCalcExp> & GetVarTable() { return m_aVarTable; }

    bool        Push(const SwUserFieldType* pUserFieldType);
    void        Pop();
    const CharClass*  GetCharClass() const;
    void        SetCharClass(const LanguageTag& rLanguageTag);

    void        SetCalcError( SwCalcError eErr )    { m_eError = eErr; }
    bool        IsCalcError() const                 { return SwCalcError::NONE != m_eError && SwCalcError::NaN != m_eError; }
    bool        IsCalcNotANumber() const            { return SwCalcError::NaN == m_eError; }

    static bool Str2Double( const OUString& rStr, sal_Int32& rPos,
                                double& rVal );
    static bool Str2Double( const OUString& rStr, sal_Int32& rPos,
                                double& rVal, SwDoc const *const pDoc );

    static LanguageType GetDocAppScriptLang( SwDoc const & rDoc );

    SW_DLLPUBLIC static bool IsValidVarName( const OUString& rStr,
                                    OUString* pValidName = nullptr );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
