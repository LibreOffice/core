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
#include <basic/sbxvar.hxx>
#include <unotools/syslocale.hxx>
#include "swdllapi.h"

class CharClass;
class LocaleDataWrapper;
class SwFieldType;
class SwDoc;
class SwUserFieldType;

#define TBLSZ 47                // should be a prime, because of hash table

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
    CALC_DAY=284
};

// Calculate Operations Strings
extern const sal_Char sCalc_Add[];
extern const sal_Char sCalc_Sub[];
extern const sal_Char sCalc_Mul[];
extern const sal_Char sCalc_Div[];
extern const sal_Char sCalc_Phd[];
extern const sal_Char sCalc_Sqrt[];
extern const sal_Char sCalc_Pow[];
extern const sal_Char sCalc_Or[];
extern const sal_Char sCalc_Xor[];
extern const sal_Char sCalc_And[];
extern const sal_Char sCalc_Not[];
extern const sal_Char sCalc_Eq[];
extern const sal_Char sCalc_Neq[];
extern const sal_Char sCalc_Leq[];
extern const sal_Char sCalc_Geq[];
extern const sal_Char sCalc_L[];
extern const sal_Char sCalc_G[];
extern const sal_Char sCalc_Sum[];
extern const sal_Char sCalc_Mean[];
extern const sal_Char sCalc_Min[];
extern const sal_Char sCalc_Max[];
extern const sal_Char sCalc_Sin[];
extern const sal_Char sCalc_Cos[];
extern const sal_Char sCalc_Tan[];
extern const sal_Char sCalc_Asin[];
extern const sal_Char sCalc_Acos[];
extern const sal_Char sCalc_Atan[];
extern const sal_Char sCalc_Tdif[];
extern const sal_Char sCalc_Round[];
extern const sal_Char sCalc_Date[];

//  Calculate ErrorCodes
enum class SwCalcError
{
    NONE=0,
    Syntax,           //  syntax error
    DivByZero,        //  division by zero
    FaultyBrackets,   //  faulty brackets
    OverflowInPower,  //  overflow in power function
    Overflow,         //  overflow
};

class SwSbxValue : public SbxValue
{
    bool bVoid;
    bool bDBvalue;
public:
    // always default to a number. otherwise it will become a SbxEMPTY
    SwSbxValue( long n = 0 ) : bVoid(false), bDBvalue(false)  { PutLong( n ); }
    SwSbxValue( const double& rD ) : bVoid(false), bDBvalue(false) { PutDouble( rD ); }

    bool GetBool() const;
    double GetDouble() const;
    SwSbxValue& MakeDouble();

    bool IsVoidValue() {return bVoid;}
    void SetVoidValue(bool bSet) {bVoid = bSet;}

    bool IsDBvalue() {return bDBvalue;}
    void SetDBvalue(bool bSet) {bDBvalue = bSet;}
};

// Calculate HashTables for VarTable and Operations
struct SwHash
{
    SwHash( const OUString& rStr );
    virtual ~SwHash();
    OUString aStr;
    std::unique_ptr<SwHash> pNext;
};

struct SwCalcExp : public SwHash
{
    SwSbxValue  nValue;
    const SwFieldType* pFieldType;

    SwCalcExp( const OUString& rStr, const SwSbxValue& rVal,
                const SwFieldType* pFieldType );
};

SwHash* Find( const OUString& rSrch, SwHash* const * ppTable,
                sal_uInt16 nTableSize, sal_uInt16* pPos = nullptr );

void DeleteHashTable( SwHash** ppTable, sal_uInt16 nTableSize );

// if CalcOp != 0, this is a valid operator
struct CalcOp;
CalcOp* FindOperator( const OUString& rSearch );

extern "C" typedef double (*pfCalc)(double);

class SwCalc
{
    SwHash*     m_aVarTable[ TBLSZ ];
    OUString    m_aVarName, m_sCurrSym;
    OUString    m_sCommand;
    std::vector<const SwUserFieldType*> m_aRekurStack;
    SwSbxValue  m_nLastLeft;
    SwSbxValue  m_nNumberValue;
    SwCalcExp   m_aErrExpr;
    sal_Int32   m_nCommandPos;

    SwDoc&      m_rDoc;
    SvtSysLocale m_aSysLocale;
    const LocaleDataWrapper* m_pLocaleDataWrapper;
    CharClass*  m_pCharClass;

    sal_uInt16      m_nListPor;
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
    OUString  GetDBName( const OUString& rName );

    SwCalc( const SwCalc& ) = delete;
    SwCalc& operator=( const SwCalc& ) = delete;

public:
        SwCalc( SwDoc& rD );
        ~SwCalc();

    SwSbxValue  Calculate( const OUString &rStr );
    OUString    GetStrResult( const SwSbxValue& rValue );
    OUString    GetStrResult( double );

    SwCalcExp*  VarInsert( const OUString& r );
    SwCalcExp*  VarLook( const OUString &rStr, bool bIns = false );
    void        VarChange( const OUString& rStr, const SwSbxValue& rValue );
    void        VarChange( const OUString& rStr, double );
    SwHash**    GetVarTable()                       { return m_aVarTable; }

    bool        Push(const SwUserFieldType* pUserFieldType);
    void        Pop();

    void        SetCalcError( SwCalcError eErr )    { m_eError = eErr; }
    bool        IsCalcError() const                 { return SwCalcError::NONE != m_eError; }

    static bool Str2Double( const OUString& rStr, sal_Int32& rPos,
                                double& rVal );
    static bool Str2Double( const OUString& rStr, sal_Int32& rPos,
                                double& rVal, SwDoc *const pDoc );

    SW_DLLPUBLIC static bool IsValidVarName( const OUString& rStr,
                                    OUString* pValidName = nullptr );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
