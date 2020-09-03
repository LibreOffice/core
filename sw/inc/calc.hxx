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
#include <rtl/ustrbuf.hxx>
#include <tools/solar.h>
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
    CALC_DAY=284,           CALC_PRODUCT=285,       CALC_AVERAGE=286,
    CALC_COUNT=287,         CALC_SIGN=288,          CALC_ABS=289
};

// Calculate Operations Strings
extern const char sCalc_Add[];
extern const char sCalc_Sub[];
extern const char sCalc_Mul[];
extern const char sCalc_Div[];
extern const char sCalc_Phd[];
extern const char sCalc_Sqrt[];
extern const char sCalc_Pow[];
extern const char sCalc_Or[];
extern const char sCalc_Xor[];
extern const char sCalc_And[];
extern const char sCalc_Not[];
extern const char sCalc_Eq[];
extern const char sCalc_Neq[];
extern const char sCalc_Leq[];
extern const char sCalc_Geq[];
extern const char sCalc_L[];
extern const char sCalc_G[];
extern const char sCalc_Sum[];
extern const char sCalc_Mean[];
extern const char sCalc_Average[];
extern const char sCalc_Product[];
extern const char sCalc_Count[];
extern const char sCalc_Min[];
extern const char sCalc_Max[];
extern const char sCalc_Sin[];
extern const char sCalc_Cos[];
extern const char sCalc_Tan[];
extern const char sCalc_Asin[];
extern const char sCalc_Acos[];
extern const char sCalc_Atan[];
extern const char sCalc_Round[];
extern const char sCalc_Date[];
extern const char sCalc_Sign[];
extern const char sCalc_Abs[];

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
    SwSbxValue( long n = 0 ) : m_bVoid(false), m_bDBvalue(false)  { PutLong( n ); }
    SwSbxValue( const double& rD ) : m_bVoid(false), m_bDBvalue(false) { PutDouble( rD ); }

    bool GetBool() const;
    double GetDouble() const;
    SwSbxValue& MakeDouble();

    bool IsVoidValue() const {return m_bVoid;}
    void SetVoidValue(bool bSet) {m_bVoid = bSet;}

    bool IsDBvalue() const {return m_bDBvalue;}
    void SetDBvalue(bool bSet) {m_bDBvalue = bSet;}
};

// Calculate HashTables for VarTable and Operations
struct SwHash
{
    SwHash( const OUString& rStr );
    virtual ~SwHash();
    OUString aStr;
    std::unique_ptr<SwHash> pNext;
};

struct SwCalcExp final : public SwHash
{
    SwSbxValue  nValue;
    const SwFieldType* pFieldType;

    SwCalcExp( const OUString& rStr, const SwSbxValue& rVal,
                const SwFieldType* pFieldType );
};

/// T should be a subclass of SwHash
template<class T>
class SwHashTable
{
    std::vector<std::unique_ptr<T>> m_aData;
public:
    SwHashTable(size_t nSize) : m_aData(nSize) {}
    std::unique_ptr<T> & operator[](size_t idx) { return m_aData[idx]; }
    std::unique_ptr<T> const & operator[](size_t idx) const { return m_aData[idx]; }
    void resize(size_t nSize) { m_aData.resize(nSize); }

    T* Find( const OUString& rStr, sal_uInt16* pPos = nullptr ) const
    {
        size_t nTableSize = m_aData.size();
        sal_uLong ii = 0;
        for( sal_Int32 n = 0; n < rStr.getLength(); ++n )
        {
            ii = ii << 1 ^ rStr[n];
        }
        ii %= nTableSize;

        if( pPos )
            *pPos = static_cast<sal_uInt16>(ii);

        for( T* pEntry = m_aData[ii].get(); pEntry; pEntry = static_cast<T*>(pEntry->pNext.get()) )
        {
            if( rStr == pEntry->aStr )
            {
                return pEntry;
            }
        }
        return nullptr;
    }

};


// if CalcOp != 0, this is a valid operator
struct CalcOp;
CalcOp* FindOperator( const OUString& rSearch );

extern "C" typedef double (*pfCalc)(double);

class SwCalc
{
    SwHashTable<SwCalcExp> m_aVarTable;
    OUStringBuffer m_aVarName;
    OUString    m_sCurrSym;
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
    OUString  GetDBName( const OUString& rName );

    SwCalc( const SwCalc& ) = delete;
    SwCalc& operator=( const SwCalc& ) = delete;

public:
    SwCalc(SwDoc& rD);
    ~SwCalc() COVERITY_NOEXCEPT_FALSE;

    SwSbxValue  Calculate( const OUString &rStr );
    OUString    GetStrResult( const SwSbxValue& rValue );
    OUString    GetStrResult( double );

    SwCalcExp*  VarInsert( const OUString& r );
    SwCalcExp*  VarLook( const OUString &rStr, bool bIns = false );
    void        VarChange( const OUString& rStr, const SwSbxValue& rValue );
    void        VarChange( const OUString& rStr, double );
    SwHashTable<SwCalcExp> & GetVarTable() { return m_aVarTable; }

    bool        Push(const SwUserFieldType* pUserFieldType);
    void        Pop();
    CharClass* GetCharClass();

    void        SetCalcError( SwCalcError eErr )    { m_eError = eErr; }
    bool        IsCalcError() const                 { return SwCalcError::NONE != m_eError && SwCalcError::NaN != m_eError; }
    bool        IsCalcNotANumber() const            { return SwCalcError::NaN == m_eError; }

    static bool Str2Double( const OUString& rStr, sal_Int32& rPos,
                                double& rVal );
    static bool Str2Double( const OUString& rStr, sal_Int32& rPos,
                                double& rVal, SwDoc const *const pDoc );

    SW_DLLPUBLIC static bool IsValidVarName( const OUString& rStr,
                                    OUString* pValidName = nullptr );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
