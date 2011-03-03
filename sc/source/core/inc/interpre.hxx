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

#ifndef SC_INTERPRE_HXX
#define SC_INTERPRE_HXX

#include <math.h>
#include <rtl/math.hxx>
#include "formula/errorcodes.hxx"
#include "cell.hxx"
#include "scdll.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include "externalrefmgr.hxx"

#include <math.h>
#include <map>

class ScDocument;
class SbxVariable;
class ScBaseCell;
class ScFormulaCell;
class SvNumberFormatter;
class ScDBRangeBase;
struct MatrixDoubleOp;
struct ScQueryParam;
struct ScDBQueryParamBase;

struct ScCompare
{
    double  nVal[2];
    String* pVal[2];
    BOOL    bVal[2];
    BOOL    bEmpty[2];
        ScCompare( String* p1, String* p2 )
        {
            pVal[ 0 ] = p1;
            pVal[ 1 ] = p2;
            bEmpty[0] = FALSE;
            bEmpty[1] = FALSE;
        }
};

struct ScCompareOptions
{
    ScQueryEntry        aQueryEntry;
    bool                bRegEx;
    bool                bMatchWholeCell;
    bool                bIgnoreCase;

                        ScCompareOptions( ScDocument* pDoc, const ScQueryEntry& rEntry, bool bReg );
private:
                        // Not implemented, prevent usage.
                        ScCompareOptions();
                        ScCompareOptions( const ScCompareOptions & );
     ScCompareOptions&  operator=( const ScCompareOptions & );
};

class ScToken;

#define MAXSTACK      (4096 / sizeof(formula::FormulaToken*))

class ScTokenStack
{
public:
    DECL_FIXEDMEMPOOL_NEWDEL( ScTokenStack )
    formula::FormulaToken* pPointer[ MAXSTACK ];
};

enum ScIterFunc {
    ifSUM,                              // Add up
    ifSUMSQ,                            // Sums of squares
    ifPRODUCT,                          // Product
    ifAVERAGE,                          // Average
    ifCOUNT,                            // Count Values
    ifCOUNT2,                           // Count Values (not empty)
    ifMIN,                              // Minimum
    ifMAX                               // Maximum
};

struct FormulaTokenRef_less
{
    bool operator () ( const formula::FormulaConstTokenRef& r1, const formula::FormulaConstTokenRef& r2 ) const
        { return &r1 < &r2; }
};
typedef ::std::map< const formula::FormulaConstTokenRef, formula::FormulaTokenRef, FormulaTokenRef_less> ScTokenMatrixMap;

class ScInterpreter
{
    // distibution function objects need the GetxxxDist methods
    friend class ScGammaDistFunction;
    friend class ScBetaDistFunction;
    friend class ScTDistFunction;
    friend class ScFDistFunction;
    friend class ScChiDistFunction;
    friend class ScChiSqDistFunction;

public:
    DECL_FIXEDMEMPOOL_NEWDEL( ScInterpreter )

    static void GlobalExit();           // aus ScGlobal::Clear() gerufen

    /// Could string be a regular expression?
    /// If pDoc!=NULL the document options are taken into account and if
    /// RegularExpressions are disabled the function returns FALSE regardless
    /// of the string content.
    static BOOL MayBeRegExp( const String& rStr, const ScDocument* pDoc );

    /// Fail safe division, returning an errDivisionByZero coded into a double
    /// if denominator is 0.0
    static inline double div( const double& fNumerator, const double& fDenominator );

    ScMatrixRef GetNewMat(SCSIZE nC, SCSIZE nR);

    enum VolatileType {
        VOLATILE,
        VOLATILE_MACRO,
        NOT_VOLATILE
    };

    VolatileType GetVolatileType() const;

private:
    static ScTokenStack*    pGlobalStack;
    static BOOL             bGlobalStackInUse;

    formula::FormulaTokenIterator aCode;
    ScAddress   aPos;
    ScTokenArray& rArr;
    ScDocument* pDok;
    formula::FormulaTokenRef  xResult;
    ScJumpMatrix*   pJumpMatrix;        // currently active array condition, if any
    ScTokenMatrixMap* pTokenMatrixMap;  // map ScToken* to formula::FormulaTokenRef if in array condition
    ScFormulaCell* pMyFormulaCell;      // the cell of this formula expression
    SvNumberFormatter* pFormatter;

    const formula::FormulaToken*
                pCur;                // current token
    String      aTempStr;               // for GetString()
    ScTokenStack* pStackObj;            // contains the stacks
    formula::FormulaToken**   pStack;                 // the current stack
    USHORT      nGlobalError;           // global (local to this formula expression) error
    USHORT      sp;                     // stack pointer
    USHORT      maxsp;                  // the maximal used stack pointer
    ULONG       nFuncFmtIndex;          // NumberFormatIndex of a function
    ULONG       nCurFmtIndex;           // current NumberFormatIndex
    ULONG       nRetFmtIndex;           // NumberFormatIndex of an expression, if any
    short       nFuncFmtType;           // NumberFormatType of a function
    short       nCurFmtType;            // current NumberFormatType
    short       nRetFmtType;            // NumberFormatType of an expression
    USHORT      mnStringNoValueError;   // the error set in ConvertStringToValue() if no value
    BOOL        glSubTotal;             // flag for subtotal functions
    BYTE        cPar;                   // current count of parameters
    BOOL        bCalcAsShown;           // precision as shown
    BOOL        bMatrixFormula;         // formula cell is a matrix formula

    VolatileType meVolaileType;

//---------------------------------Funktionen in interpre.cxx---------
// nMust <= nAct <= nMax ? ok : PushError
inline BOOL MustHaveParamCount( short nAct, short nMust );
inline BOOL MustHaveParamCount( short nAct, short nMust, short nMax );
inline BOOL MustHaveParamCountMin( short nAct, short nMin );
void PushParameterExpected();
void PushIllegalParameter();
void PushIllegalArgument();
void PushNoValue();
void PushNA();
//-------------------------------------------------------------------------
// Funktionen fuer den Zugriff auf das Document
//-------------------------------------------------------------------------
void ReplaceCell( ScAddress& );     // for TableOp
void ReplaceCell( SCCOL& rCol, SCROW& rRow, SCTAB& rTab );  // for TableOp
BOOL IsTableOpInRange( const ScRange& );
ULONG GetCellNumberFormat( const ScAddress&, const ScBaseCell* );
double ConvertStringToValue( const String& );
double GetCellValue( const ScAddress&, const ScBaseCell* );
double GetCellValueOrZero( const ScAddress&, const ScBaseCell* );
double GetValueCellValue( const ScAddress&, const ScValueCell* );
ScBaseCell* GetCell( const ScAddress& rPos )
    { return pDok->GetCell( rPos ); }
void GetCellString( String& rStr, const ScBaseCell* pCell );
inline USHORT GetCellErrCode( const ScBaseCell* pCell )
    { return pCell ? pCell->GetErrorCode() : 0; }
inline CellType GetCellType( const ScBaseCell* pCell )
    { return pCell ? pCell->GetCellType() : CELLTYPE_NONE; }
/// Really empty or inherited emptiness.
inline BOOL HasCellEmptyData( const ScBaseCell* pCell )
    { return pCell ? pCell->HasEmptyData() : TRUE; }
/// This includes inherited emptiness, which usually is regarded as value!
inline BOOL HasCellValueData( const ScBaseCell* pCell )
    { return pCell ? pCell->HasValueData() : FALSE; }
/// Not empty and not value.
inline BOOL HasCellStringData( const ScBaseCell* pCell )
    { return pCell ? pCell->HasStringData() : FALSE; }

BOOL CreateDoubleArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2, BYTE* pCellArr);
BOOL CreateStringArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2, BYTE* pCellArr);
BOOL CreateCellArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                   SCCOL nCol2, SCROW nRow2, SCTAB nTab2, BYTE* pCellArr);

//-----------------------------------------------------------------------------
// Stack operations
//-----------------------------------------------------------------------------

/** Does substitute with formula::FormulaErrorToken in case nGlobalError is set and the token
    passed is not formula::FormulaErrorToken.
    Increments RefCount of the original token if not substituted. */
void Push( formula::FormulaToken& r );

/** Does not substitute with formula::FormulaErrorToken in case nGlobalError is set.
    Used to push RPN tokens or from within Push() or tokens that are already
    explicit formula::FormulaErrorToken. Increments RefCount. */
void PushWithoutError( formula::FormulaToken& r );

/** Clones the token to be pushed or substitutes with formula::FormulaErrorToken if
    nGlobalError is set and the token passed is not formula::FormulaErrorToken. */
void PushTempToken( const formula::FormulaToken& );

/** Does substitute with formula::FormulaErrorToken in case nGlobalError is set and the token
    passed is not formula::FormulaErrorToken.
    Increments RefCount of the original token if not substituted.
    ATTENTION! The token had to be allocated with `new' and must not be used
    after this call if no RefCount was set because possibly it gets immediately
    deleted in case of an errStackOverflow or if substituted with formula::FormulaErrorToken! */
void PushTempToken( formula::FormulaToken* );

/** Does not substitute with formula::FormulaErrorToken in case nGlobalError is set.
    Used to push tokens from within PushTempToken() or tokens that are already
    explicit formula::FormulaErrorToken. Increments RefCount.
    ATTENTION! The token had to be allocated with `new' and must not be used
    after this call if no RefCount was set because possibly it gets immediately
    decremented again and thus deleted in case of an errStackOverflow! */
void PushTempTokenWithoutError( formula::FormulaToken* );

/** If nGlobalError is set push formula::FormulaErrorToken.
    If nGlobalError is not set do nothing.
    Used in PushTempToken() and alike to simplify handling.
    @return: <TRUE/> if nGlobalError. */
inline bool IfErrorPushError()
{
    if (nGlobalError)
    {
        PushTempTokenWithoutError( new formula::FormulaErrorToken( nGlobalError));
        return true;
    }
    return false;
}

/** Obtain cell result / content from address and push as temp token.
    bDisplayEmptyAsString is passed to ScEmptyCell in case of an empty cell
    result. Also obtain number format and type if _both_, type and index
    pointer, are not NULL. */
void PushCellResultToken( bool bDisplayEmptyAsString, const ScAddress & rAddress,
        short * pRetTypeExpr, ULONG * pRetIndexExpr );

formula::FormulaTokenRef PopToken();
void Pop();
void PopError();
double PopDouble();
const String& PopString();
void ValidateRef( const ScSingleRefData & rRef );
void ValidateRef( const ScComplexRefData & rRef );
void ValidateRef( const ScRefList & rRefList );
void SingleRefToVars( const ScSingleRefData & rRef, SCCOL & rCol, SCROW & rRow, SCTAB & rTab );
void PopSingleRef( ScAddress& );
void PopSingleRef(SCCOL& rCol, SCROW &rRow, SCTAB& rTab);
void DoubleRefToRange( const ScComplexRefData&, ScRange&, BOOL bDontCheckForTableOp = FALSE );
/** If formula::StackVar formula::svDoubleRef pop ScDoubleRefToken and return values of
    ScComplexRefData.
    Else if StackVar svRefList return values of the ScComplexRefData where
    rRefInList is pointing to. rRefInList is incremented. If rRefInList was the
    last element in list pop ScRefListToken and set rRefInList to 0, else
    rParam is incremented (!) to allow usage as in
    while(nParamCount--) PopDoubleRef(aRange,nParamCount,nRefInList);
  */
void PopDoubleRef( ScRange & rRange, short & rParam, size_t & rRefInList );
void PopDoubleRef( ScRange&, BOOL bDontCheckForTableOp = FALSE );
void DoubleRefToVars( const ScToken* p,
        SCCOL& rCol1, SCROW &rRow1, SCTAB& rTab1,
        SCCOL& rCol2, SCROW &rRow2, SCTAB& rTab2,
        BOOL bDontCheckForTableOp = FALSE );
ScDBRangeBase* PopDBDoubleRef();
void PopDoubleRef(SCCOL& rCol1, SCROW &rRow1, SCTAB& rTab1,
                          SCCOL& rCol2, SCROW &rRow2, SCTAB& rTab2,
                          BOOL bDontCheckForTableOp = FALSE );
void PopExternalSingleRef(sal_uInt16& rFileId, String& rTabName, ScSingleRefData& rRef);
void PopExternalSingleRef(ScExternalRefCache::TokenRef& rToken, ScExternalRefCache::CellFormat* pFmt = NULL);
void PopExternalDoubleRef(sal_uInt16& rFileId, String& rTabName, ScComplexRefData& rRef);
void PopExternalDoubleRef(ScExternalRefCache::TokenArrayRef& rArray);
void PopExternalDoubleRef(ScMatrixRef& rMat);
BOOL PopDoubleRefOrSingleRef( ScAddress& rAdr );
void PopDoubleRefPushMatrix();
// If MatrixFormula: convert formula::svDoubleRef to svMatrix, create JumpMatrix.
// Else convert area reference parameters marked as ForceArray to array.
// Returns TRUE if JumpMatrix created.
bool ConvertMatrixParameters();
inline void MatrixDoubleRefToMatrix();      // if MatrixFormula: PopDoubleRefPushMatrix
// If MatrixFormula or ForceArray: ConvertMatrixParameters()
inline bool MatrixParameterConversion();
ScMatrixRef PopMatrix();
void QueryMatrixType(ScMatrixRef& xMat, short& rRetTypeExpr, ULONG& rRetIndexExpr);
//void PushByte(BYTE nVal);
void PushDouble(double nVal);
void PushInt( int nVal );
void PushStringBuffer( const sal_Unicode* pString );
void PushString( const String& rString );
void PushSingleRef(SCCOL nCol, SCROW nRow, SCTAB nTab);
void PushDoubleRef(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                   SCCOL nCol2, SCROW nRow2, SCTAB nTab2);
void PushExternalSingleRef(sal_uInt16 nFileId, const String& rTabName,
                           SCCOL nCol, SCROW nRow, SCTAB nTab);
void PushExternalDoubleRef(sal_uInt16 nFileId, const String& rTabName,
                           SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                           SCCOL nCol2, SCROW nRow2, SCTAB nTab2);
void PushMatrix(ScMatrix* pMat);
void PushError( USHORT nError );
/// Raw stack type without default replacements.
formula::StackVar GetRawStackType();
/// Stack type with replacement of defaults, e.g. svMissing and formula::svEmptyCell will result in formula::svDouble.
formula::StackVar GetStackType();
// peek StackType of Parameter, Parameter 1 == TOS, 2 == TOS-1, ...
formula::StackVar GetStackType( BYTE nParam );
BYTE GetByte() { return cPar; }
// generiert aus DoubleRef positionsabhaengige SingleRef
BOOL DoubleRefToPosSingleRef( const ScRange& rRange, ScAddress& rAdr );
double GetDoubleFromMatrix(const ScMatrixRef& pMat);
double GetDouble();
double GetDoubleWithDefault(double nDefault);
BOOL IsMissing();
BOOL GetBool() { return GetDouble() != 0.0; }
const String& GetString();
const String& GetStringFromMatrix(const ScMatrixRef& pMat);
// pop matrix and obtain one element, upper left or according to jump matrix
ScMatValType GetDoubleOrStringFromMatrix( double& rDouble, String& rString );
ScMatrixRef CreateMatrixFromDoubleRef( const formula::FormulaToken* pToken,
        SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
        SCCOL nCol2, SCROW nRow2, SCTAB nTab2 );
inline ScTokenMatrixMap& GetTokenMatrixMap();
ScTokenMatrixMap* CreateTokenMatrixMap();
ScMatrixRef GetMatrix();
void ScTableOp();                                       // Mehrfachoperationen
void ScErrCell();                                       // Sonderbehandlung
                                                        // Fehlerzelle
//-----------------------------allgemeine Hilfsfunktionen
void SetMaxIterationCount(USHORT n);
inline void CurFmtToFuncFmt()
    { nFuncFmtType = nCurFmtType; nFuncFmtIndex = nCurFmtIndex; }
// Check for String overflow of rResult+rAdd and set error and erase rResult
// if so. Return TRUE if ok, FALSE if overflow
inline BOOL CheckStringResultLen( String& rResult, const String& rAdd );
// Set error according to rVal, and set rVal to 0.0 if there was an error.
inline void TreatDoubleError( double& rVal );
// Lookup using ScLookupCache, @returns TRUE if found and result address
bool LookupQueryWithCache( ScAddress & o_rResultPos,
        const ScQueryParam & rParam ) const;

//---------------------------------Funktionen in interpr1.cxx---------
void ScIfJump();
void ScChoseJump();

// Be sure to only call this if pStack[sp-nStackLevel] really contains a
// ScJumpMatrixToken, no further checks are applied!
// Returns true if last jump was executed and result matrix pushed.
bool JumpMatrix( short nStackLevel );

/** @param pOptions
        NULL means case sensitivity document option is to be used!
 */
double CompareFunc( const ScCompare& rComp, ScCompareOptions* pOptions = NULL );
double Compare();
/** @param pOptions
        NULL means case sensitivity document option is to be used!
 */
ScMatrixRef CompareMat( ScCompareOptions* pOptions = NULL );
ScMatrixRef QueryMat( ScMatrix* pMat, ScCompareOptions& rOptions );
void ScEqual();
void ScNotEqual();
void ScLess();
void ScGreater();
void ScLessEqual();
void ScGreaterEqual();
void ScAnd();
void ScOr();
void ScNot();
void ScNeg();
void ScPercentSign();
void ScIntersect();
void ScRangeFunc();
void ScUnionFunc();
void ScPi();
void ScRandom();
void ScTrue();
void ScFalse();
void ScDeg();
void ScRad();
void ScSin();
void ScCos();
void ScTan();
void ScCot();
void ScArcSin();
void ScArcCos();
void ScArcTan();
void ScArcCot();
void ScSinHyp();
void ScCosHyp();
void ScTanHyp();
void ScCotHyp();
void ScArcSinHyp();
void ScArcCosHyp();
void ScArcTanHyp();
void ScArcCotHyp();
void ScExp();
void ScLn();
void ScLog10();
void ScSqrt();
void ScIsEmpty();
short IsString();
void ScIsString();
void ScIsNonString();
void ScIsLogical();
void ScType();
void ScCell();
void ScIsRef();
void ScIsValue();
void ScIsFormula();
void ScFormula();
void ScRoman();
void ScArabic();
void ScIsNV();
void ScIsErr();
void ScIsError();
short IsEven();
void ScIsEven();
void ScIsOdd();
void ScN();
void ScCode();
void ScTrim();
void ScUpper();
void ScPropper();
void ScLower();
void ScLen();
void ScT();
void ScValue();
void ScClean();
void ScChar();
void ScJis();
void ScAsc();
void ScUnicode();
void ScUnichar();
void ScMin( BOOL bTextAsZero = FALSE );
void ScMax( BOOL bTextAsZero = FALSE );
double IterateParameters( ScIterFunc, BOOL bTextAsZero = FALSE );
void ScSumSQ();
void ScSum();
void ScProduct();
void ScAverage( BOOL bTextAsZero = FALSE );
void ScCount();
void ScCount2();
void GetStVarParams( double& rVal, double& rValCount, BOOL bTextAsZero = FALSE );
void ScVar( BOOL bTextAsZero = FALSE );
void ScVarP( BOOL bTextAsZero = FALSE );
void ScStDev( BOOL bTextAsZero = FALSE );
void ScStDevP( BOOL bTextAsZero = FALSE );
void ScColumns();
void ScRows();
void ScTables();
void ScColumn();
void ScRow();
void ScTable();
void ScMatch();
void ScCountIf();
void ScSumIf();
void ScCountEmptyCells();
void ScLookup();
void ScHLookup();
void ScVLookup();
void ScSubTotal();

// If upon call rMissingField==TRUE then the database field parameter may be
// missing (Xcl DCOUNT() syntax), or may be faked as missing by having the
// value 0.0 or being exactly the entire database range reference (old SO
// compatibility). If this was the case then rMissingField is set to TRUE upon
// return. If rMissingField==FALSE upon call all "missing cases" are considered
// to be an error.
ScDBQueryParamBase* GetDBParams( BOOL& rMissingField );

void DBIterator( ScIterFunc );
void ScDBSum();
void ScDBCount();
void ScDBCount2();
void ScDBAverage();
void ScDBGet();
void ScDBMax();
void ScDBMin();
void ScDBProduct();
void GetDBStVarParams( double& rVal, double& rValCount );
void ScDBStdDev();
void ScDBStdDevP();
void ScDBVar();
void ScDBVarP();
void ScIndirect();
void ScAddressFunc();
void ScOffset();
void ScIndex();
void ScMultiArea();
void ScAreas();
void ScCurrency();
void ScReplace();
void ScFixed();
void ScFind();
void ScExact();
void ScLeft();
void ScRight();
void ScSearch();
void ScMid();
void ScText();
void ScSubstitute();
void ScRept();
void ScConcat();
void ScExternal();
void ScMissing();
void ScMacro();
BOOL SetSbxVariable( SbxVariable* pVar, const ScAddress& );
BOOL SetSbxVariable( SbxVariable* pVar, SCCOL nCol, SCROW nRow, SCTAB nTab );
void ScErrorType();
void ScDBArea();
void ScColRowNameAuto();
void ScGetPivotData();
void ScHyperLink();
void ScBahtText();
void ScTTT();

//----------------Funktionen in interpr2.cxx---------------

/** Obtain the date serial number for a given date.
    @param bStrict
        If FALSE, nYear < 100 takes the two-digit year setting into account,
        and rollover of invalid calendar dates takes place, e.g. 1999-02-31 =>
        1999-03-03.
        If TRUE, the date passed must be a valid Gregorian calendar date. No
        two-digit expanding or rollover is done.
 */
double GetDateSerial( INT16 nYear, INT16 nMonth, INT16 nDay, bool bStrict );

void ScGetActDate();
void ScGetActTime();
void ScGetYear();
void ScGetMonth();
void ScGetDay();
void ScGetDayOfWeek();
void ScGetWeekOfYear();
void ScEasterSunday();
void ScGetHour();
void ScGetMin();
void ScGetSec();
void ScPlusMinus();
void ScAbs();
void ScInt();
void ScEven();
void ScOdd();
void ScCeil();
void ScFloor();
void RoundNumber( rtl_math_RoundingMode eMode );
void ScRound();
void ScRoundUp();
void ScRoundDown();
void ScGetDateValue();
void ScGetTimeValue();
void ScArcTan2();
void ScLog();
void ScGetDate();
void ScGetTime();
void ScGetDiffDate();
void ScGetDiffDate360();
void ScPower();
void ScAmpersand();
void ScAdd();
void ScSub();
void ScMul();
void ScDiv();
void ScPow();
void ScCurrent();
void ScStyle();
void ScDde();
void ScBase();
void ScDecimal();
void ScConvert();
void ScEuroConvert();

//----------------------- Finanzfunktionen ------------------------------------
void ScNPV();
void ScIRR();
void ScMIRR();
void ScISPMT();

double ScGetBw(double fZins, double fZzr, double fRmz,
                      double fZw, double fF);
void ScBW();
void ScDIA();
double ScGetGDA(double fWert, double fRest, double fDauer,
                       double fPeriode, double fFaktor);
void ScGDA();
void ScGDA2();
double ScInterVDB(double fWert,double fRest,double fDauer,double fDauer1,
                double fPeriode,double fFaktor);
void ScVDB();
void ScLaufz();
void ScLIA();
double ScGetRmz(double fZins, double fZzr, double fBw,
                       double fZw, double fF);
void ScRMZ();
void ScZGZ();
double ScGetZw(double fZins, double fZzr, double fRmz,
                      double fBw, double fF);
void ScZW();
void ScZZR();
bool RateIteration(double fNper, double fPayment, double fPv,
                                double fFv, double fPayType, double& fGuess);
void ScZins();
double ScGetZinsZ(double fZins, double fZr, double fZzr, double fBw,
                         double fZw, double fF, double& fRmz);
void ScZinsZ();
void ScKapz();
void ScKumZinsZ();
void ScKumKapZ();
void ScEffektiv();
void ScNominal();
void ScMod();
void ScBackSolver();
void ScIntercept();
//-------------------------Funktionen in interpr5.cxx--------------------------
double ScGetGCD(double fx, double fy);
void ScGCD();
void ScLCM();
//-------------------------- Matrixfunktionen ---------------------------------

void ScMatValue();
void MEMat(ScMatrix* mM, SCSIZE n);
void MFastMult(ScMatrix* pA, ScMatrix* pB, ScMatrix* pR, SCSIZE n, SCSIZE m, SCSIZE l);
void ScMatDet();
void ScMatInv();
void ScMatMult();
void ScMatTrans();
void ScEMat();
void ScMatRef();
ScMatrixRef MatConcat(ScMatrix* pMat1, ScMatrix* pMat2);
void ScSumProduct();
void ScSumX2MY2();
void ScSumX2DY2();
void ScSumXMY2();
void ScGrowth();
// multiple Regression: Varianzen der Koeffizienten
BOOL RGetVariances( ScMatrix* pV, ScMatrix* pX, SCSIZE nC, SCSIZE nR,
    BOOL bSwapColRow, BOOL bZeroConstant );
void Calculate(ScMatrixRef& pResMat,ScMatrixRef& pE,ScMatrixRef& pQ,ScMatrixRef& pV,ScMatrixRef& pMatX,BOOL bConstant,SCSIZE N,SCSIZE M,BYTE nCase);
ScMatrixRef Calculate2(const BOOL bConstant,const SCSIZE M ,const SCSIZE N,ScMatrixRef& pMatX,ScMatrixRef& pMatY,BYTE nCase);
bool Calculate3(const SCSIZE M ,ScMatrixRef& pQ);
bool Calculate4(BOOL _bExp,ScMatrixRef& pResMat,ScMatrixRef& pQ,BOOL bConstant,SCSIZE N,SCSIZE M);
bool CalculateSkew(double& fSum,double& fCount,double& vSum,std::vector<double>& values);
void CalculateSlopeIntercept(BOOL bSlope);
void CalculateSmallLarge(BOOL bSmall);
void CalculatePearsonCovar(BOOL _bPearson,BOOL _bStexy);
bool CalculateTest( BOOL _bTemplin
                   ,const SCSIZE nC1, const SCSIZE nC2,const SCSIZE nR1,const SCSIZE nR2
                   ,const ScMatrixRef& pMat1,const ScMatrixRef& pMat2
                   ,double& fT,double& fF);
void CalculateLookup(BOOL HLookup);
bool FillEntry(ScQueryEntry& rEntry);
void CalculateAddSub(BOOL _bSub);
void CalculateTrendGrowth(BOOL _bGrowth);
void CalulateRGPRKP(BOOL _bRKP);
void CalculateSumX2MY2SumX2DY2(BOOL _bSumX2DY2);
void CalculateMatrixValue(const ScMatrix* pMat,SCSIZE nC,SCSIZE nR);
bool CheckMatrix(BOOL _bLOG,BOOL _bTrendGrowth,BYTE& nCase,SCSIZE& nCX,SCSIZE& nCY,SCSIZE& nRX,SCSIZE& nRY,SCSIZE& M,SCSIZE& N,ScMatrixRef& pMatX,ScMatrixRef& pMatY);

void ScRGP();
void ScRKP();
void ScForecast();
//------------------------- Functions in interpr3.cxx -------------------------
void ScNoName();
void ScBadName();
// Statistik:
double phi(double x);
double integralPhi(double x);
double taylor(double* pPolynom, USHORT nMax, double x);
double gauss(double x);
double gaussinv(double x);
double GetBetaDist(double x, double alpha, double beta);  //cumulative distribution function
double GetBetaDistPDF(double fX, double fA, double fB); //probability density function)
double GetChiDist(double fChi, double fDF);     // for LEGACY.CHIDIST, returns right tail
double GetChiSqDistCDF(double fX, double fDF);  // for CHISQDIST, returns left tail
double GetChiSqDistPDF(double fX, double fDF);  // probability density function
double GetFDist(double x, double fF1, double fF2);
double GetTDist(double T, double fDF);
double Fakultaet(double x);
double BinomKoeff(double n, double k);
double GetGamma(double x);
double GetLogGamma(double x);
double GetBeta(double fAlpha, double fBeta);
double GetLogBeta(double fAlpha, double fBeta);
void ScLogGamma();
void ScGamma();
void ScPhi();
void ScGauss();
void ScStdNormDist();
void ScFisher();
void ScFisherInv();
void ScFact();
void ScNormDist();
void ScGammaDist();
void ScGammaInv();
void ScExpDist();
void ScBinomDist();
void ScPoissonDist();
void ScKombin();
void ScKombin2();
void ScVariationen();
void ScVariationen2();
void ScB();
void ScHypGeomDist();
void ScLogNormDist();
void ScLogNormInv();
void ScTDist();
void ScFDist();
void ScChiDist();   // for LEGACY.CHIDIST, returns right tail
void ScChiSqDist(); // returns left tail or density
void ScChiSqInv(); //invers to CHISQDIST
void ScWeibull();
void ScBetaDist();
void ScFInv();
void ScTInv();
void ScChiInv();
void ScBetaInv();
void ScCritBinom();
void ScNegBinomDist();
void ScKurt();
void ScHarMean();
void ScGeoMean();
void ScStandard();
void ScSkew();
void ScMedian();
double GetMedian( ::std::vector<double> & rArray );
double GetPercentile( ::std::vector<double> & rArray, double fPercentile );
void GetNumberSequenceArray( BYTE nParamCount, ::std::vector<double>& rArray );
void GetSortArray(BYTE nParamCount, ::std::vector<double>& rSortArray, ::std::vector<long>* pIndexOrder = NULL);
void QuickSort(::std::vector<double>& rSortArray, ::std::vector<long>* pIndexOrder = NULL);
void ScModalValue();
void ScAveDev();
void ScDevSq();
void ScZTest();
void ScTTest();
void ScFTest();
void ScChiTest();
void ScRank();
void ScPercentile();
void ScPercentrank();
void ScLarge();
void ScSmall();
void ScFrequency();
void ScQuartile();
void ScNormInv();
void ScSNormInv();
void ScConfidence();
void ScTrimMean();
void ScProbability();
void ScCorrel();
void ScCovar();
void ScPearson();
void ScRSQ();
void ScSTEXY();
void ScSlope();
void ScTrend();
void ScInfo();

//------------------------ Functions in interpr6.cxx -------------------------

static const double fMaxGammaArgument;  // defined in interpr3.cxx

double GetGammaContFraction(double fA,double fX);
double GetGammaSeries(double fA,double fX);
double GetLowRegIGamma(double fA,double fX);    // lower regularized incomplete gamma function, GAMMAQ
double GetUpRegIGamma(double fA,double fX);     // upper regularized incomplete gamma function, GAMMAP
// probability density function; fLambda is "scale" parameter
double GetGammaDistPDF(double fX, double fAlpha, double fLambda);
// cumulative distribution function; fLambda is "scale" parameter
double GetGammaDist(double fX, double fAlpha, double fLambda);

/**
 * Go through all tokens to see if the array contains a volatile token.  We
 * need to do this since a conditional token such as IF function may skip
 * some tokens and it may incorrectly mark the token array non-volatile.
 */
void CheckForVolatileToken();

public:
    ScInterpreter( ScFormulaCell* pCell, ScDocument* pDoc,
                    const ScAddress&, ScTokenArray& );
    ~ScInterpreter();

    formula::StackVar Interpret();

    void SetError(USHORT nError)
            { if (nError && !nGlobalError) nGlobalError = nError; }

    USHORT GetError()                               const   { return nGlobalError; }
    formula::StackVar  GetResultType()              const   { return xResult->GetType(); }
    const String&   GetStringResult()               const   { return xResult->GetString(); }
    double          GetNumResult()                  const   { return xResult->GetDouble(); }
    formula::FormulaTokenRef
                    GetResultToken()                const   { return xResult; }
    short           GetRetFormatType()              const   { return nRetFmtType; }
    ULONG           GetRetFormatIndex()             const   { return nRetFmtIndex; }
};


inline void ScInterpreter::MatrixDoubleRefToMatrix()
{
    if ( bMatrixFormula && GetStackType() == formula::svDoubleRef )
    {
        GetTokenMatrixMap();    // make sure it exists, create if not.
        PopDoubleRefPushMatrix();
    }
}


inline bool ScInterpreter::MatrixParameterConversion()
{
    if ( (bMatrixFormula || pCur->HasForceArray()) && !pJumpMatrix && sp > 0 )
        return ConvertMatrixParameters();
    return false;
}


inline ScTokenMatrixMap& ScInterpreter::GetTokenMatrixMap()
{
    if (!pTokenMatrixMap)
        pTokenMatrixMap = CreateTokenMatrixMap();
    return *pTokenMatrixMap;
}


inline BOOL ScInterpreter::MustHaveParamCount( short nAct, short nMust )
{
    if ( nAct == nMust )
        return TRUE;
    if ( nAct < nMust )
        PushParameterExpected();
    else
        PushIllegalParameter();
    return FALSE;
}


inline BOOL ScInterpreter::MustHaveParamCount( short nAct, short nMust, short nMax )
{
    if ( nMust <= nAct && nAct <= nMax )
        return TRUE;
    if ( nAct < nMust )
        PushParameterExpected();
    else
        PushIllegalParameter();
    return FALSE;
}


inline BOOL ScInterpreter::MustHaveParamCountMin( short nAct, short nMin )
{
    if ( nAct >= nMin )
        return TRUE;
    PushParameterExpected();
    return FALSE;
}


inline BOOL ScInterpreter::CheckStringResultLen( String& rResult, const String& rAdd )
{
    if ( (ULONG) rResult.Len() + rAdd.Len() > STRING_MAXLEN )
    {
        SetError( errStringOverflow );
        rResult.Erase();
        return FALSE;
    }
    return TRUE;
}


inline void ScInterpreter::TreatDoubleError( double& rVal )
{
    if ( !::rtl::math::isFinite( rVal ) )
    {
        USHORT nErr = GetDoubleErrorValue( rVal );
        if ( nErr )
            SetError( nErr );
        else
            SetError( errNoValue );
        rVal = 0.0;
    }
}


// static
inline double ScInterpreter::div( const double& fNumerator, const double& fDenominator )
{
    return (fDenominator != 0.0) ? (fNumerator / fDenominator) :
        CreateDoubleError( errDivisionByZero);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
