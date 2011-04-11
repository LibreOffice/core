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
#ifndef _CALC_HXX
#define _CALC_HXX

#include <svl/svarray.hxx>
#include <unotools/syslocale.hxx>

#ifndef __SBX_SBXVALUE //autogen
#include <basic/sbxvar.hxx>
#endif
#include "swdllapi.h"

class CharClass;
class LocaleDataWrapper;
class SwFieldType;
class SwDoc;

#define TBLSZ 47                // should be a prime, because of hash table

const sal_Unicode cListDelim    = '|';

/******************************************************************************
 *                          Calculate Operations
 ******************************************************************************/
enum SwCalcOper
{
    CALC_NAME,              CALC_NUMBER,            CALC_ENDCALC,
    CALC_PLUS='+',          CALC_MINUS='-',         CALC_MUL='*',
    CALC_DIV='/',           CALC_PRINT=';',         CALC_ASSIGN='=',
    CALC_LP='(',            CALC_RP=')',            CALC_PHD='%',
    CALC_POW='^',
    CALC_LISTOP = cListDelim,
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

//-- Calculate Operations Strings -----------------------------------------

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

/******************************************************************************
 *                          Calculate ErrorCodes
 ******************************************************************************/
enum SwCalcError
{
    CALC_NOERR=0,
    CALC_SYNTAX,        //  syntax error
    CALC_ZERODIV,       //  division by zero
    CALC_BRACK,         //  faulty brackets
    CALC_POWERR,        //  overflow in power function
    CALC_VARNFND,       //  variable was not found
    CALC_OVERFLOW,      //  overflow
    CALC_WRONGTIME      //  wrong time format
};

class SwSbxValue : public SbxValue
{
    bool bVoid;
public:
    // always default to a number. otherwise it will become a SbxEMPTY
    SwSbxValue( long n = 0 ) : bVoid(false)  { PutLong( n ); }
    SwSbxValue( const double& rD ) : bVoid(false) { PutDouble( rD ); }
    SwSbxValue( const SwSbxValue& rVal ) :
        SvRefBase( rVal ),
        SbxValue( rVal ),
        bVoid(rVal.bVoid)
        {}
    virtual ~SwSbxValue();


    sal_Bool GetBool() const;
    double GetDouble() const;
    SwSbxValue& MakeDouble();

    bool IsVoidValue() {return bVoid;}
    void SetVoidValue(bool bSet) {bVoid = bSet;}
};

/******************************************************************************
 *          Calculate HashTables for VarTable und Operations
 ******************************************************************************/
struct SwHash
{
            SwHash( const String& rStr );
    virtual ~SwHash();
    String  aStr;
    SwHash *pNext;
};

struct SwCalcExp : public SwHash
{
    SwSbxValue  nValue;
    const SwFieldType* pFldType;

    SwCalcExp( const String& rStr, const SwSbxValue& rVal,
                const SwFieldType* pFldType = 0 );
};

SwHash* Find( const String& rSrch, SwHash** ppTable,
                sal_uInt16 nTblSize, sal_uInt16* pPos = 0 );

void DeleteHashTable( SwHash** ppTable, sal_uInt16 nTblSize );

// if _CalcOp != 0, this is a valid operator
struct _CalcOp;
_CalcOp* FindOperator( const String& rSearch );

/******************************************************************************
 *                               class SwCalc
 ******************************************************************************/
class SwCalc
{
    SwHash*     VarTable[ TBLSZ ];
    String      aVarName, sCurrSym;
    String      sCommand;
    SvPtrarr    aRekurStk;
    SwSbxValue  nLastLeft;
    SwSbxValue  nNumberValue;
    SwCalcExp   aErrExpr;
    xub_StrLen  nCommandPos;

    SwDoc&      rDoc;
    SvtSysLocale m_aSysLocale;
    const LocaleDataWrapper* pLclData;
    CharClass*  pCharClass;

    sal_uInt16      nListPor;
    SwCalcOper  eCurrOper;
    SwCalcOper  eCurrListOper;
    SwCalcError eError;


    SwCalcOper  GetToken();
    SwSbxValue  Expr();
    SwSbxValue  Term();
    SwSbxValue  Prim();

    sal_Bool        ParseTime( sal_uInt16*, sal_uInt16*, sal_uInt16* );

    String  GetColumnName( const String& rName );
    String  GetDBName( const String& rName );

    // dont call this methods
    SwCalc( const SwCalc& );
    SwCalc& operator=( const SwCalc& );

public:
        SwCalc( SwDoc& rD );
        ~SwCalc();

    SwSbxValue  Calculate( const String &rStr );
    String      GetStrResult( const SwSbxValue& rValue, sal_Bool bRound = sal_True );
    String      GetStrResult( double, sal_Bool bRound = sal_True );

    SwCalcExp*  VarInsert( const String& r );
    SwCalcExp*  VarLook( const String &rStr, sal_uInt16 ins = 0 );
    void        VarChange( const String& rStr, const SwSbxValue& rValue );
    void        VarChange( const String& rStr, double );
    SwHash**    GetVarTable()                       { return VarTable; }

    sal_Bool        Push( const VoidPtr pPtr );
    void        Pop( const VoidPtr pPtr );

    void        SetCalcError( SwCalcError eErr )    { eError = eErr; }
    sal_Bool        IsCalcError() const                 { return 0 != eError; }

    static bool Str2Double( const String& rStr, xub_StrLen& rPos,
                                double& rVal,
                                LocaleDataWrapper const*const pData = 0 );
    static bool Str2Double( const String& rStr, xub_StrLen& rPos,
                                double& rVal, SwDoc *const pDoc );

    SW_DLLPUBLIC static sal_Bool IsValidVarName( const String& rStr,
                                    String* pValidName = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
