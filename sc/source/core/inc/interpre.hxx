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

#ifndef SC_INTERPRE_HXX
#define SC_INTERPRE_HXX

#include <math.h>
#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include "formula/errorcodes.hxx"
#include "formula/tokenarray.hxx"
#include "scdll.hxx"
#include "types.hxx"
#include "externalrefmgr.hxx"
#include "calcconfig.hxx"
#include "token.hxx"

#include <map>

class ScDocument;
class SbxVariable;
class ScFormulaCell;
class SvNumberFormatter;
class ScDBRangeBase;
struct ScQueryParam;
struct ScDBQueryParamBase;
struct ScQueryEntry;

struct ScCompare;
struct ScCompareOptions;
struct ScSingleRefData;
struct ScComplexRefData;

class ScToken;
class ScJumpMatrix;
struct ScRefCellValue;

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

enum ScIterFuncIf
{
    ifSUMIF,                            // Conditional sum
    ifAVERAGEIF                         // Conditional average
};

enum ScIterFuncIfs
{
    ifSUMIFS,     // Multi-Conditional sum
    ifAVERAGEIFS, // Multi-Conditional average
    ifCOUNTIFS    // Multi-Conditional count
};

struct FormulaTokenRef_less
{
    bool operator () ( const formula::FormulaConstTokenRef& r1, const formula::FormulaConstTokenRef& r2 ) const
        { return r1.get() < r2.get(); }
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

    static void SetGlobalConfig(const ScCalcConfig& rConfig);
    static const ScCalcConfig& GetGlobalConfig();

    static void GlobalExit();           // called by ScGlobal::Clear()

    /// Could string be a regular expression?
    /// If pDoc!=NULL the document options are taken into account and if
    /// RegularExpressions are disabled the function returns false regardless
    /// of the string content.
    static bool MayBeRegExp( const OUString& rStr, const ScDocument* pDoc );

    /// Fail safe division, returning an errDivisionByZero coded into a double
    /// if denominator is 0.0
    static inline double div( const double& fNumerator, const double& fDenominator );

    ScMatrixRef GetNewMat(SCSIZE nC, SCSIZE nR, bool bEmpty = false);

    enum VolatileType {
        VOLATILE,
        VOLATILE_MACRO,
        NOT_VOLATILE
    };

    VolatileType GetVolatileType() const;

private:
    static ScCalcConfig maGlobalConfig;

    static ScTokenStack*    pGlobalStack;
    static bool             bGlobalStackInUse;

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
                pCur;                   // current token
    OUString    aTempStr;               // for GetString()
    ScTokenStack* pStackObj;            // contains the stacks
    formula::FormulaToken**   pStack;   // the current stack
    sal_uInt16  nGlobalError;           // global (local to this formula expression) error
    sal_uInt16  sp;                     // stack pointer
    sal_uInt16  maxsp;                  // the maximal used stack pointer
    sal_uLong   nFuncFmtIndex;          // NumberFormatIndex of a function
    sal_uLong   nCurFmtIndex;           // current NumberFormatIndex
    sal_uLong   nRetFmtIndex;           // NumberFormatIndex of an expression, if any
    short       nFuncFmtType;           // NumberFormatType of a function
    short       nCurFmtType;            // current NumberFormatType
    short       nRetFmtType;            // NumberFormatType of an expression
    sal_uInt16  mnStringNoValueError;   // the error set in ConvertStringToValue() if no value
    bool        glSubTotal;             // flag for subtotal functions
    sal_uInt8   cPar;                   // current count of parameters
    bool        bCalcAsShown;           // precision as shown
    bool        bMatrixFormula;         // formula cell is a matrix formula

    VolatileType meVolatileType;

// nMust <= nAct <= nMax ? ok : PushError
inline bool MustHaveParamCount( short nAct, short nMust );
inline bool MustHaveParamCount( short nAct, short nMust, short nMax );
inline bool MustHaveParamCountMin( short nAct, short nMin );
void PushParameterExpected();
void PushIllegalParameter();
void PushIllegalArgument();
void PushNoValue();
void PushNA();

// Functions for accessing a document

void ReplaceCell( ScAddress& );     // for TableOp
void ReplaceCell( SCCOL& rCol, SCROW& rRow, SCTAB& rTab );  // for TableOp
bool IsTableOpInRange( const ScRange& );
sal_uLong GetCellNumberFormat( const ScAddress& rPos, ScRefCellValue& rCell );
double ConvertStringToValue( const String& );
double GetCellValue( const ScAddress&, ScRefCellValue& rCell );
double GetCellValueOrZero( const ScAddress&, ScRefCellValue& rCell );
double GetValueCellValue( const ScAddress&, double fOrig );
void GetCellString( OUString& rStr, ScRefCellValue& rCell );
sal_uInt16 GetCellErrCode( const ScRefCellValue& rCell );

bool CreateDoubleArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt8* pCellArr);
bool CreateStringArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt8* pCellArr);
bool CreateCellArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                   SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt8* pCellArr);

// Stack operations

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
        short * pRetTypeExpr, sal_uLong * pRetIndexExpr );

formula::FormulaTokenRef PopToken();
void Pop();
void PopError();
double PopDouble();
const OUString& PopString();
void ValidateRef( const ScSingleRefData & rRef );
void ValidateRef( const ScComplexRefData & rRef );
void ValidateRef( const ScRefList & rRefList );
void SingleRefToVars( const ScSingleRefData & rRef, SCCOL & rCol, SCROW & rRow, SCTAB & rTab );
void PopSingleRef( ScAddress& );
void PopSingleRef(SCCOL& rCol, SCROW &rRow, SCTAB& rTab);
void DoubleRefToRange( const ScComplexRefData&, ScRange&, bool bDontCheckForTableOp = false );
/** If formula::StackVar formula::svDoubleRef pop ScDoubleRefToken and return values of
    ScComplexRefData.
    Else if StackVar svRefList return values of the ScComplexRefData where
    rRefInList is pointing to. rRefInList is incremented. If rRefInList was the
    last element in list pop ScRefListToken and set rRefInList to 0, else
    rParam is incremented (!) to allow usage as in
    while(nParamCount--) PopDoubleRef(aRange,nParamCount,nRefInList);
  */
void PopDoubleRef( ScRange & rRange, short & rParam, size_t & rRefInList );
void PopDoubleRef( ScRange&, bool bDontCheckForTableOp = false );
void DoubleRefToVars( const ScToken* p,
        SCCOL& rCol1, SCROW &rRow1, SCTAB& rTab1,
        SCCOL& rCol2, SCROW &rRow2, SCTAB& rTab2,
        bool bDontCheckForTableOp = false );
ScDBRangeBase* PopDBDoubleRef();
void PopDoubleRef(SCCOL& rCol1, SCROW &rRow1, SCTAB& rTab1,
                          SCCOL& rCol2, SCROW &rRow2, SCTAB& rTab2,
                          bool bDontCheckForTableOp = false );
void PopExternalSingleRef(sal_uInt16& rFileId, String& rTabName, ScSingleRefData& rRef);
void PopExternalSingleRef(ScExternalRefCache::TokenRef& rToken, ScExternalRefCache::CellFormat* pFmt = NULL);
void PopExternalSingleRef(sal_uInt16& rFileId, String& rTabName, ScSingleRefData& rRef,
                          ScExternalRefCache::TokenRef& rToken, ScExternalRefCache::CellFormat* pFmt = NULL);
void PopExternalDoubleRef(sal_uInt16& rFileId, String& rTabName, ScComplexRefData& rRef);
void PopExternalDoubleRef(ScExternalRefCache::TokenArrayRef& rArray);
void PopExternalDoubleRef(ScMatrixRef& rMat);
void GetExternalDoubleRef(sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& aData, ScExternalRefCache::TokenArrayRef& rArray);
bool PopDoubleRefOrSingleRef( ScAddress& rAdr );
void PopDoubleRefPushMatrix();
// If MatrixFormula: convert formula::svDoubleRef to svMatrix, create JumpMatrix.
// Else convert area reference parameters marked as ForceArray to array.
// Returns true if JumpMatrix created.
bool ConvertMatrixParameters();
inline void MatrixDoubleRefToMatrix();      // if MatrixFormula: PopDoubleRefPushMatrix
// If MatrixFormula or ForceArray: ConvertMatrixParameters()
inline bool MatrixParameterConversion();
ScMatrixRef PopMatrix();
void QueryMatrixType(ScMatrixRef& xMat, short& rRetTypeExpr, sal_uLong& rRetIndexExpr);

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
void PushMatrix(const ScMatrixRef& pMat);
void PushError( sal_uInt16 nError );
/// Raw stack type without default replacements.
formula::StackVar GetRawStackType();
/// Stack type with replacement of defaults, e.g. svMissing and formula::svEmptyCell will result in formula::svDouble.
formula::StackVar GetStackType();
// peek StackType of Parameter, Parameter 1 == TOS, 2 == TOS-1, ...
formula::StackVar GetStackType( sal_uInt8 nParam );
sal_uInt8 GetByte() { return cPar; }
// generates a position-dependent SingleRef out of a DoubleRef
bool DoubleRefToPosSingleRef( const ScRange& rRange, ScAddress& rAdr );
double GetDoubleFromMatrix(const ScMatrixRef& pMat);
double GetDouble();
double GetDoubleWithDefault(double nDefault);
bool IsMissing();
bool GetBool() { return GetDouble() != 0.0; }
const OUString& GetString();
const OUString& GetStringFromMatrix(const ScMatrixRef& pMat);
// pop matrix and obtain one element, upper left or according to jump matrix
ScMatValType GetDoubleOrStringFromMatrix( double& rDouble, OUString& rString );
ScMatrixRef CreateMatrixFromDoubleRef( const formula::FormulaToken* pToken,
        SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
        SCCOL nCol2, SCROW nRow2, SCTAB nTab2 );
inline ScTokenMatrixMap& GetTokenMatrixMap();
ScTokenMatrixMap* CreateTokenMatrixMap();
ScMatrixRef GetMatrix();
void ScTableOp();                                       // repeated operations
void ScErrCell();                                       // special handling for
                                                        // error cell

// common helper functions

void SetMaxIterationCount(sal_uInt16 n);
inline void CurFmtToFuncFmt()
    { nFuncFmtType = nCurFmtType; nFuncFmtIndex = nCurFmtIndex; }
// Check for String overflow of rResult+rAdd and set error and erase rResult
// if so. Return true if ok, false if overflow
inline bool CheckStringResultLen( String& rResult, const String& rAdd );
// Set error according to rVal, and set rVal to 0.0 if there was an error.
inline void TreatDoubleError( double& rVal );
// Lookup using ScLookupCache, @returns true if found and result address
bool LookupQueryWithCache( ScAddress & o_rResultPos,
        const ScQueryParam & rParam ) const;

void ScIfJump();
void ScIfError( bool bNAonly );
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
ScMatrixRef QueryMat( const ScMatrixRef& pMat, ScCompareOptions& rOptions );
void ScEqual();
void ScNotEqual();
void ScLess();
void ScGreater();
void ScLessEqual();
void ScGreaterEqual();
void ScAnd();
void ScOr();
void ScXor();
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
void ScCosecant();
void ScSecant();
void ScCosecantHyp();
void ScSecantHyp();
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
void ScCellExternal();
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
void ScNumberValue();
void ScClean();
void ScChar();
void ScJis();
void ScAsc();
void ScUnicode();
void ScUnichar();
void ScMin( bool bTextAsZero = false );
void ScMax( bool bTextAsZero = false );
double IterateParameters( ScIterFunc, bool bTextAsZero = false );
void ScSumSQ();
void ScSum();
void ScProduct();
void ScAverage( bool bTextAsZero = false );
void ScCount();
void ScCount2();
void GetStVarParams( double& rVal, double& rValCount, bool bTextAsZero = false );
void ScVar( bool bTextAsZero = false );
void ScVarP( bool bTextAsZero = false );
void ScStDev( bool bTextAsZero = false );
void ScStDevP( bool bTextAsZero = false );
void ScColumns();
void ScRows();
void ScTables();
void ScColumn();
void ScRow();
void ScTable();
void ScMatch();
double IterateParametersIf( ScIterFuncIf );
void ScCountIf();
void ScSumIf();
void ScAverageIf();
double IterateParametersIfs( ScIterFuncIfs );
void ScSumIfs();
void ScAverageIfs();
void ScCountIfs();
void ScCountEmptyCells();
void ScLookup();
void ScHLookup();
void ScVLookup();
void ScSubTotal();

// If upon call rMissingField==true then the database field parameter may be
// missing (Xcl DCOUNT() syntax), or may be faked as missing by having the
// value 0.0 or being exactly the entire database range reference (old SO
// compatibility). If this was the case then rMissingField is set to true upon
// return. If rMissingField==false upon call all "missing cases" are considered
// to be an error.
ScDBQueryParamBase* GetDBParams( bool& rMissingField );

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
bool SetSbxVariable( SbxVariable* pVar, const ScAddress& );
bool SetSbxVariable( SbxVariable* pVar, SCCOL nCol, SCROW nRow, SCTAB nTab );
void ScErrorType();
void ScDBArea();
void ScColRowNameAuto();
void ScGetPivotData();
void ScHyperLink();
void ScBahtText();
void ScBitAnd();
void ScBitOr();
void ScBitXor();
void ScBitRshift();
void ScBitLshift();
void ScTTT();

/** Obtain the date serial number for a given date.
    @param bStrict
        If false, nYear < 100 takes the two-digit year setting into account,
        and rollover of invalid calendar dates takes place, e.g. 1999-02-31 =>
        1999-03-03.
        If true, the date passed must be a valid Gregorian calendar date. No
        two-digit expanding or rollover is done.

    @param bCheckGregorian
        If true, date must be Gregorian, i.e. >= 1582-10-15.
        If false, don't care, any valid date >= 0-1-1 will do.
 */
double GetDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, bool bStrict, bool bCheckGregorian );

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
void ScGetDateDif();
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

// financial functions
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
double ScGetGCD(double fx, double fy);
void ScGCD();
void ScLCM();

// matrix functions
void ScMatValue();
void MEMat(const ScMatrixRef& mM, SCSIZE n);
void ScMatDet();
void ScMatInv();
void ScMatMult();
void ScMatTrans();
void ScEMat();
void ScMatRef();
ScMatrixRef MatConcat(const ScMatrixRef& pMat1, const ScMatrixRef& pMat2);
void ScSumProduct();
void ScSumX2MY2();
void ScSumX2DY2();
void ScSumXMY2();
void ScGrowth();
bool CalculateSkew(double& fSum,double& fCount,double& vSum,std::vector<double>& values);
void CalculateSkewOrSkewp( bool bSkewp );
void CalculateSlopeIntercept(bool bSlope);
void CalculateSmallLarge(bool bSmall);
void CalculatePearsonCovar(bool _bPearson,bool _bStexy);
bool CalculateTest( bool _bTemplin
                   ,const SCSIZE nC1, const SCSIZE nC2,const SCSIZE nR1,const SCSIZE nR2
                   ,const ScMatrixRef& pMat1,const ScMatrixRef& pMat2
                   ,double& fT,double& fF);
void CalculateLookup(bool HLookup);
bool FillEntry(ScQueryEntry& rEntry);
void CalculateAddSub(bool _bSub);
void CalculateTrendGrowth(bool _bGrowth);
void CalulateRGPRKP(bool _bRKP);
void CalculateSumX2MY2SumX2DY2(bool _bSumX2DY2);
void CalculateMatrixValue(const ScMatrix* pMat,SCSIZE nC,SCSIZE nR);
bool CheckMatrix(bool _bLOG,sal_uInt8& nCase,SCSIZE& nCX,SCSIZE& nCY,SCSIZE& nRX,SCSIZE& nRY,SCSIZE& M,SCSIZE& N,ScMatrixRef& pMatX,ScMatrixRef& pMatY);
void ScRGP();
void ScRKP();
void ScForecast();
void ScNoName();
void ScBadName();
// Statistics:
double phi(double x);
double integralPhi(double x);
double taylor(double* pPolynom, sal_uInt16 nMax, double x);
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
double GetBinomDistPMF(double x, double n, double p); //probability mass function
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
void ScSkewp();
void ScMedian();
double GetMedian( ::std::vector<double> & rArray );
double GetPercentile( ::std::vector<double> & rArray, double fPercentile );
void GetNumberSequenceArray( sal_uInt8 nParamCount, ::std::vector<double>& rArray );
void GetSortArray(sal_uInt8 nParamCount, ::std::vector<double>& rSortArray, ::std::vector<long>* pIndexOrder = NULL);
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
void ScLenB();
void ScRightB();
void ScLeftB();
void ScMidB();

void ScFilterXML();
void ScWebservice();

static const double fMaxGammaArgument;

double GetGammaContFraction(double fA,double fX);
double GetGammaSeries(double fA,double fX);
double GetLowRegIGamma(double fA,double fX);    // lower regularized incomplete gamma function, GAMMAQ
double GetUpRegIGamma(double fA,double fX);     // upper regularized incomplete gamma function, GAMMAP
// probability density function; fLambda is "scale" parameter
double GetGammaDistPDF(double fX, double fAlpha, double fLambda);
// cumulative distribution function; fLambda is "scale" parameter
double GetGammaDist(double fX, double fAlpha, double fLambda);

public:
    ScInterpreter( ScFormulaCell* pCell, ScDocument* pDoc,
                    const ScAddress&, ScTokenArray& );
    ~ScInterpreter();

    formula::StackVar Interpret();

    void SetError(sal_uInt16 nError)
            { if (nError && !nGlobalError) nGlobalError = nError; }

    sal_uInt16                  GetError() const            { return nGlobalError; }
    formula::StackVar           GetResultType() const       { return xResult->GetType(); }
    const String&               GetStringResult() const     { return xResult->GetString(); }
    double                      GetNumResult() const        { return xResult->GetDouble(); }
    formula::FormulaTokenRef    GetResultToken() const      { return xResult; }
    short                       GetRetFormatType() const    { return nRetFmtType; }
    sal_uLong                   GetRetFormatIndex() const   { return nRetFmtIndex; }
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

inline bool ScInterpreter::MustHaveParamCount( short nAct, short nMust )
{
    if ( nAct == nMust )
        return true;
    if ( nAct < nMust )
        PushParameterExpected();
    else
        PushIllegalParameter();
    return false;
}

inline bool ScInterpreter::MustHaveParamCount( short nAct, short nMust, short nMax )
{
    if ( nMust <= nAct && nAct <= nMax )
        return true;
    if ( nAct < nMust )
        PushParameterExpected();
    else
        PushIllegalParameter();
    return false;
}

inline bool ScInterpreter::MustHaveParamCountMin( short nAct, short nMin )
{
    if ( nAct >= nMin )
        return true;
    PushParameterExpected();
    return false;
}

inline bool ScInterpreter::CheckStringResultLen( String& rResult, const String& rAdd )
{
    if ( (sal_uLong) rResult.Len() + rAdd.Len() > STRING_MAXLEN )
    {
        SetError( errStringOverflow );
        rResult.Erase();
        return false;
    }
    return true;
}

inline void ScInterpreter::TreatDoubleError( double& rVal )
{
    if ( !::rtl::math::isFinite( rVal ) )
    {
        sal_uInt16 nErr = GetDoubleErrorValue( rVal );
        if ( nErr )
            SetError( nErr );
        else
            SetError( errNoValue );
        rVal = 0.0;
    }
}

inline double ScInterpreter::div( const double& fNumerator, const double& fDenominator )
{
    return (fDenominator != 0.0) ? (fNumerator / fDenominator) :
        CreateDoubleError( errDivisionByZero);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
