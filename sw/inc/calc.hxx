/*************************************************************************
 *
 *  $RCSfile: calc.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-13 20:31:57 $
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
#ifndef _CALC_HXX
#define _CALC_HXX


#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef __SBX_SBXVALUE //autogen
#include <svtools/sbxvar.hxx>
#endif

class CharClass;
class LocaleDataWrapper;
class SwFieldType;
class SwDoc;

#define TBLSZ 47                // sollte Primzahl sein, wegen HashTable

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
    CALC_ROUND=281
};

//-- Calculate Operations Strings -----------------------------------------

extern const sal_Char __FAR_DATA sCalc_Add[];
extern const sal_Char __FAR_DATA sCalc_Sub[];
extern const sal_Char __FAR_DATA sCalc_Mul[];
extern const sal_Char __FAR_DATA sCalc_Div[];
extern const sal_Char __FAR_DATA sCalc_Phd[];
extern const sal_Char __FAR_DATA sCalc_Sqrt[];
extern const sal_Char __FAR_DATA sCalc_Pow[];
extern const sal_Char __FAR_DATA sCalc_Or[];
extern const sal_Char __FAR_DATA sCalc_Xor[];
extern const sal_Char __FAR_DATA sCalc_And[];
extern const sal_Char __FAR_DATA sCalc_Not[];
extern const sal_Char __FAR_DATA sCalc_Eq[];
extern const sal_Char __FAR_DATA sCalc_Neq[];
extern const sal_Char __FAR_DATA sCalc_Leq[];
extern const sal_Char __FAR_DATA sCalc_Geq[];
extern const sal_Char __FAR_DATA sCalc_L[];
extern const sal_Char __FAR_DATA sCalc_G[];
extern const sal_Char __FAR_DATA sCalc_Sum[];
extern const sal_Char __FAR_DATA sCalc_Mean[];
extern const sal_Char __FAR_DATA sCalc_Min[];
extern const sal_Char __FAR_DATA sCalc_Max[];
extern const sal_Char __FAR_DATA sCalc_Sin[];
extern const sal_Char __FAR_DATA sCalc_Cos[];
extern const sal_Char __FAR_DATA sCalc_Tan[];
extern const sal_Char __FAR_DATA sCalc_Asin[];
extern const sal_Char __FAR_DATA sCalc_Acos[];
extern const sal_Char __FAR_DATA sCalc_Atan[];
extern const sal_Char __FAR_DATA sCalc_Tdif[];
extern const sal_Char __FAR_DATA sCalc_Round[];

/******************************************************************************
 *                          Calculate ErrorCodes
 ******************************************************************************/
enum SwCalcError
{
    CALC_NOERR=0,
    CALC_SYNTAX,        //  Syntax Fehler
    CALC_ZERODIV,       //  Division durch Null
    CALC_BRACK,         //  Fehlerhafte Klammerung
    CALC_POWERR,        //  Ueberlauf in Quadratfunktion
    CALC_VARNFND,       //  Variable wurde nicht gefunden
    CALC_OVERFLOW,      //  Ueberlauf
    CALC_WRONGTIME      //  falsches Zeitformat
};

class SwSbxValue : public SbxValue
{
public:
    //JP 03.02.99: immer auf eine Zahl defaulten, damit auch gerechnet wird.
    //              Ansonsten wird daraus ein SbxEMPTY und damit ist nichts
    //              anzufangen.
    SwSbxValue( long n = 0 )        { PutLong( n ); }
    SwSbxValue( const double& rD )  { PutDouble( rD ); }
    SwSbxValue( const SwSbxValue& rVal ) : SbxValue( rVal ) {}
    virtual ~SwSbxValue();

    // Strings sonderbehandeln
    BOOL GetBool() const;
    // Strings sonderbehandeln
    double GetDouble() const;
    SwSbxValue& MakeDouble();
};

/******************************************************************************
 *          Calculate HashTables fuer VarTable und Operations
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
                USHORT nTblSize, USHORT* pPos = 0 );

void DeleteHashTable( SwHash** ppTable, USHORT nTblSize );

// falls _CalcOp != 0, dann ist das ein gueltiger Operator
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
    LocaleDataWrapper* pLclData;
    CharClass*  pCharClass;

    USHORT      nListPor;
    SwCalcOper  eCurrOper;
    SwCalcOper  eCurrListOper;
    SwCalcError eError;


    SwCalcOper  GetToken();
    SwSbxValue  Expr();
    SwSbxValue  Term();
    SwSbxValue  Prim();

    BOOL        ParseTime( USHORT*, USHORT*, USHORT* );

    String  GetColumnName( const String& rName );
    String  GetDBName( const String& rName );

    // dont call this methods
    SwCalc( const SwCalc& );
    SwCalc& operator=( const SwCalc& );

public:
        SwCalc( SwDoc& rD );
        ~SwCalc();

    SwSbxValue  Calculate( const String &rStr );
    String      GetStrResult( const SwSbxValue& rValue, BOOL bRound = TRUE );
    String      GetStrResult( double, BOOL bRound = TRUE );

    SwCalcExp*  VarInsert( const String& r );
    SwCalcExp*  VarLook( const String &rStr, USHORT ins = 0 );
    void        VarChange( const String& rStr, const SwSbxValue& rValue );
    void        VarChange( const String& rStr, double );
    SwHash**    GetVarTable()                       { return VarTable; }

    BOOL        Push( const VoidPtr pPtr );
    void        Pop( const VoidPtr pPtr );

    void        SetCalcError( SwCalcError eErr )    { eError = eErr; }
    BOOL        IsCalcError() const                 { return eError; }

    static FASTBOOL Str2Double( const String& rStr, xub_StrLen& rPos,
                                double& rVal,
                                const LocaleDataWrapper* pData = 0 );
    static FASTBOOL Str2Double( const String& rStr, xub_StrLen& rPos,
                                double& rVal, SwDoc* pDoc );

    static FASTBOOL IsValidVarName( const String& rStr,
                                    String* pValidName = 0 );
};

#endif
