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

#define SC_SPEW_ENABLED 0

#include <math.h>

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_SCDLL_HXX
#include "scdll.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#if SC_SPEW_ENABLED
#include "scspew.hxx"
#endif
class SbxVariable;
namespace binfilter {

class SvNumberFormatter;
class ScDocument;
class ScMatrix;
class ScBaseCell;
class ScFormulaCell;

#define MAX_ANZ_MAT 150
                                        // Maximale Anzahl fuer Zwischenergebnis
                                        // Matrizen
                                        // Maximum ca. 85 wird bei Invertierung
                                        // von 128 x 128 benoetigt!


#ifndef MSC
    #include <setjmp.h>
#else
    extern "C"
    {
    #define _JBLEN  9  /* bp, di, si, sp, ret addr, ds */
    typedef  int  jmp_buf[_JBLEN];
    #define _JMP_BUF_DEFINED
    #define setjmp  _setjmp
    int  __cdecl _setjmp(jmp_buf);
    void __cdecl longjmp(jmp_buf, int);
    };
#endif


struct ScCompare
{
    double	nVal[2];
    String*	pVal[2];
    BOOL	bVal[2];
    BOOL	bEmpty[2];
        ScCompare( String* p1, String* p2 )
        {
            pVal[ 0 ] = p1;
            pVal[ 1 ] = p2;
            bEmpty[0] = FALSE;
            bEmpty[1] = FALSE;
        }
};

class ScToken;

#define MAXSTACK	  (4096 / sizeof(ScToken*))

class ScTokenStack
{
public:
    DECL_FIXEDMEMPOOL_NEWDEL( ScTokenStack )
    ScToken* pPointer[ MAXSTACK ];
};

class ScErrorStack
{
public:
    DECL_FIXEDMEMPOOL_NEWDEL( ScErrorStack )
    USHORT pPointer[ MAXSTACK ];
};

enum ScIterFunc {
    ifSUM,								// Aufsummieren
    ifSUMSQ,							// Quadratsummen
    ifPRODUCT,							// Multiplizieren
    ifAVERAGE,							// Durchschnitt
    ifCOUNT,							// Anzahl Werte
    ifCOUNT2,							// Anzahl Werte (nichtleer)
    ifMIN,								// Minimum
    ifMAX								// Maximum
};

class ScInterpreter
{
    // distibution function objects need the GetxxxDist methods
    friend class ScGammaDistFunction;
    friend class ScBetaDistFunction;
    friend class ScTDistFunction;
    friend class ScFDistFunction;
    friend class ScChiDistFunction;

public:
    DECL_FIXEDMEMPOOL_NEWDEL( ScInterpreter )
    static USHORT nGlobalError;			// globale Fehlervariable
#if SC_SPEW_ENABLED
    static ScSpew theSpew;
#endif

    static void GlobalExit();			// aus ScGlobal::Clear() gerufen

    /// Could string be a regular expression?
    /// If pDoc!=NULL the document options are taken into account and if
    /// RegularExpressions are disabled the function returns FALSE regardless
    /// of the string content.
    static BOOL MayBeRegExp( const String& rStr, const ScDocument* pDoc );

private:
    static ScTokenStack*	pGlobalStack;
    static ScErrorStack*	pGlobalErrorStack;
    static BOOL				bGlobalStackInUse;

    ScTokenIterator aCode;
    ScAddress	aPos;
    ScTokenArray& rArr;
    String		aResult;
    ScDocument*	pDok;					// Pointer aufs Dokument
    double		nResult;
    ScMatrix*	pResult;
    ScFormulaCell* pMyFormulaCell;		// die Zelle mit der Formel
    SvNumberFormatter* pFormatter;
    StackVar	eResult;

    USHORT 		nGlobError;				// lokale Kopie
    const ScToken* pCur;				// aktuelles Token
    String		aTempStr;				// fuer GetString()
    ScTokenStack* pStackObj;			// enthaelt den Stack
    ScToken**	pStack;					// der Stack
    ScErrorStack* pErrorStackObj;		// enthaelt den ErrorStack
    USHORT*		pErrorStack;			// der ErrorStack
    USHORT		sp;						// der Stackpointer
    USHORT		maxsp;					// der maximale StackPointer
    double**	ppGlobSortArray;		// Pointer auf Array zum Sortieren
    ScMatrix**  ppTempMatArray;			// Array fuer temporaere Matrizen
    USHORT      nMatCount;				// dazugehoeriger Zaehler
    BOOL		bMatDel;				// und Kontrollvariable
    USHORT		nRetMat;				// Index der Return-Matrix
    ULONG		nFuncFmtIndex;			// NumberFormatIndex einer Funktion
    ULONG		nCurFmtIndex;			// aktueller NumberFormatIndex
    ULONG		nRetFmtIndex;			// ggbf. NumberFormatIndex des Ausdrucks
    short		nFuncFmtType;			// NumberFormatTyp einer Funktion
    short		nCurFmtType;			// aktueller NumberFormatTyp
    short		nRetFmtType;			// NumberFormatTyp des Ausdrucks
    BOOL		glSubTotal;				// Flag fuer Subtotalfunktionen
    BYTE		cPar;					// aktuelle Anzahl Parameter
    BOOL		bCalcAsShown;			// Genauigkeit wie angezeigt
    BOOL		bMatrixFormula;			// Formelzelle ist Matrixformel

//---------------------------------Funktionen in interpre.cxx---------
// nMust <= nAct <= nMax ? ok : SetError, PushInt
inline BOOL MustHaveParamCount( BYTE nAct, BYTE nMust );
inline BOOL MustHaveParamCount( BYTE nAct, BYTE nMust, BYTE nMax );
inline BOOL MustHaveParamCountMin( BYTE nAct, BYTE nMin );
void SetParameterExpected();
void SetIllegalParameter();
void SetIllegalArgument();
void SetNoValue();
void SetNV();
//-------------------------------------------------------------------------
// Funktionen für den Zugriff auf das Document
//-------------------------------------------------------------------------
void ReplaceCell( ScAddress& );		// for TableOp
void ReplaceCell( USHORT& rCol, USHORT& rRow, USHORT& rTab );	// for TableOp
BOOL IsTableOpInRange( const ScRange& );
ULONG GetCellNumberFormat( const ScAddress&, const ScBaseCell* );
double GetCellValue( const ScAddress&, const ScBaseCell* );
double GetCellValueOrZero( const ScAddress&, const ScBaseCell* );
double GetValueCellValue( const ScAddress&, const ScValueCell* );
ScBaseCell* GetCell( const ScAddress& rPos )
    { return pDok->GetCell( rPos ); }
void GetCellString( String& rStr, const ScBaseCell* pCell );
USHORT GetCellErrCode( const ScBaseCell* pCell );
inline CellType GetCellType( const ScBaseCell* pCell )
    { return pCell ? pCell->GetCellType() : CELLTYPE_NONE; }
inline BOOL HasCellValueData( const ScBaseCell* pCell )
    { return pCell ? pCell->HasValueData() : FALSE; }
inline BOOL HasCellStringData( const ScBaseCell* pCell )
    { return pCell ? pCell->HasStringData() : FALSE; }
BOOL CreateDoubleArr(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                     USHORT nCol2, USHORT nRow2, USHORT nTab2, BYTE* pCellArr);
BOOL CreateStringArr(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                     USHORT nCol2, USHORT nRow2, USHORT nTab2, BYTE* pCellArr);
BOOL CreateCellArr(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                   USHORT nCol2, USHORT nRow2, USHORT nTab2, BYTE* pCellArr);
//-----------------------------------------------------------------------------
// Stackoperationen
//-----------------------------------------------------------------------------
void Push( ScToken& r );
void PushTempToken( const ScToken& );
void PushTempToken( ScToken* );				//! see warnings in interpr4.cxx
void Pop();
void PopError();
BYTE PopByte();
double PopDouble();
const String& PopString();
void PopSingleRef( ScAddress& );
void PopSingleRef(USHORT& rCol, USHORT &rRow, USHORT& rTab);
void PopDoubleRef( ScRange&, BOOL bDontCheckForTableOp = FALSE );
void PopDoubleRef(USHORT& rCol1, USHORT &rRow1, USHORT& rTab1,
                          USHORT& rCol2, USHORT &rRow2, USHORT& rTab2,
                          BOOL bDontCheckForTableOp = FALSE );
BOOL PopDoubleRefOrSingleRef( ScAddress& rAdr );
void PopDoubleRefPushMatrix();
inline void MatrixDoubleRefToMatrix();		// wenn MatrixFormula: PopDoubleRefPushMatrix
ScMatrix* PopMatrix();
//void PushByte(BYTE nVal);
void PushDouble(double nVal);
void PushInt( int nVal );
void PushStringBuffer( const sal_Unicode* pString );
void PushString( const String& rString );
void PushSingleRef(USHORT nCol, USHORT nRow, USHORT nTab);
void PushDoubleRef(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                 USHORT nCol2, USHORT nRow2, USHORT nTab2);
void PushMatrix(ScMatrix* pMat);
StackVar GetStackType();
// peek StackType of Parameter, Parameter 1 == TOS, 2 == TOS-1, ...
StackVar GetStackType( BYTE nParam );
BYTE GetByte() { return cPar; }
/*
short GetShort();
double GetLong();
*/
// generiert aus DoubleRef positionsabhaengige SingleRef
BOOL DoubleRefToPosSingleRef( const ScRange& rRange, ScAddress& rAdr );
double GetDouble();
BOOL GetBool() { return GetDouble() != 0.0; }
const String& GetString();
ScMatrix* GetMatrix(USHORT& nMatInd);					// in interpr2.cxx
void ScTableOp();										// Mehrfachoperatione;
void ScErrCell();										// Sonderbehandlung

//-----------------------------allgemeine Hilfsfunktionen
void SetMaxIterationCount(USHORT n);
inline void CurFmtToFuncFmt()
    { nFuncFmtType = nCurFmtType; nFuncFmtIndex = nCurFmtIndex; }
// Check for String overflow of rResult+rAdd and set error and erase rResult
// if so. Return TRUE if ok, FALSE if overflow
inline BOOL CheckStringResultLen( String& rResult, const String& rAdd );
//---------------------------------Funktionen in interpr1.cxx---------
//-----------------------------Textfunktionen
void ScIfJump();
void ScChoseJump();
short CompareFunc( const ScCompare& rComp );
short Compare();
ScMatrix* CompareMat();
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
void ScIsLogical(UINT16 aOldNumType);
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
BOOL GetDBParams( USHORT& rTab, ScQueryParam& rParam, BOOL& rMissingField );

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
void ScAdress();
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
BOOL SetSbxVariable( SbxVariable* pVar, USHORT nCol, USHORT nRow, USHORT nTab );
void ScErrorType();
void ScDBArea();
void ScColRowNameAuto();
void ScCalcTeam();
void ScAnswer();
void ScTTT();
void ScSpewFunc();
void ScGame(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void ScGame();

//----------------Funktionen in interpr2.cxx---------------
double GetDate(INT16 nYear, INT16 nMonth, INT16 nDay);
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

//----------------------- Finanzfunktionen ------------------------------------
void ScNBW();
void ScIKV();
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
double GetZinsIterationEps(double fZzr, double fRmz, double fBw, double fZw,
                                  double fF,   double& fSchaetzwert);
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
double ScGetGGT(double fx, double fy);
void ScGGT();
void ScKGV();
//-------------------------- Matrixfunktionen ---------------------------------
ScMatrix* GetNewMat(USHORT nC, USHORT nR, USHORT& nMatInd);
void ResetNewMat(USHORT nIndex);
void ScMatValue();
void MEMat(ScMatrix* mM, USHORT n);
void MFastMult(ScMatrix* pA, ScMatrix* pB, ScMatrix* pR, USHORT n, USHORT m, USHORT l);
void MFastSub(ScMatrix* pA, ScMatrix* pB, ScMatrix* pR, USHORT n, USHORT m);
void MFastTrans(ScMatrix* pA, ScMatrix* pR, USHORT n, USHORT m);
BOOL MFastBackSubst(ScMatrix* pA, ScMatrix* pR, USHORT n, BOOL bIsUpper);
BOOL ScMatLUP(ScMatrix* mA, USHORT m, USHORT p,
              ScMatrix* mL, ScMatrix* mU, ScMatrix* mP,
              ULONG& rPermutCounter, BOOL& bIsInvertable);
void ScMatDet();
void ScMatInv();
void ScMatMult();
void ScMatTrans();
void ScEMat();
void ScMatRef();
ScMatrix* MatAdd(ScMatrix* pMat1, ScMatrix* pMat2);
ScMatrix* MatSub(ScMatrix* pMat1, ScMatrix* pMat2);
ScMatrix* MatMul(ScMatrix* pMat1, ScMatrix* pMat2);
ScMatrix* MatDiv(ScMatrix* pMat1, ScMatrix* pMat2);
ScMatrix* MatPow(ScMatrix* pMat1, ScMatrix* pMat2);
ScMatrix* MatConcat(ScMatrix* pMat1, ScMatrix* pMat2);
void ScSumProduct();
void ScSumX2MY2();
void ScSumX2DY2();
void ScSumXMY2();
void ScGrowth();
// multiple Regression: Varianzen der Koeffizienten
BOOL RGetVariances( ScMatrix* pV, ScMatrix* pX,	USHORT nC, USHORT nR,
    BOOL bSwapColRow, BOOL bZeroConstant );
void ScRGP();
void ScRKP();
void ScForecast();
//--------------------------------------------------------------------------------
// Funktionen in interpr3.cxx
// Statistik:
void ScNoName();
double phi(double x);
double taylor(double* pPolynom, USHORT nMax, double x);
double gauss(double x);
double gaussinv(double x);
double GetGammaDist(double x, double alpha, double beta);
double GetBetaDist(double x, double alpha, double beta);
double GetChiDist(double fChi, double fDF);
double GetFDist(double x, double fF1, double fF2);
double GetTDist(double T, double fDF);
double Fakultaet(double x);
double BinomKoeff(double n, double k);
double GammaHelp(double& x, BOOL& bReflect);
double GetGamma(double x);
double GetLogGamma(double x);
void ScLogGamma();
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
void ScChiDist();
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
void GetSortArray(BYTE nParamCount, double** ppSortArray, ULONG& nSize);
void QuickSort(long nLo, long nHi, double* pSortArr);
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


public:
    ScInterpreter( ScFormulaCell* pCell, ScDocument* pDoc,
                    const ScAddress&, ScTokenArray& );
    ~ScInterpreter();

    StackVar Interpret();

    static void SetError(USHORT nError)
            { if (nError && !nGlobalError) nGlobalError = nError; }

    static USHORT GetError() { return nGlobalError; }

    const     String& GetStringResult() { return aResult; }
    double    GetNumResult() 			{ return nResult; }
    ScMatrix* GetMatrixResult()			{ return pResult; }
    StackVar  GetResultType()			{ return eResult; }
    short     GetRetFormatType()	{ return nRetFmtType; }
    ULONG     GetRetFormatIndex()	{ return nRetFmtIndex; }
    BOOL		HadMatrix() const	{ return bMatDel; }
};


inline void ScInterpreter::MatrixDoubleRefToMatrix()
{
    if ( bMatrixFormula && GetStackType() == svDoubleRef )
        PopDoubleRefPushMatrix();
}


inline BOOL ScInterpreter::MustHaveParamCount( BYTE nAct, BYTE nMust )
{
    if ( nAct == nMust )
        return TRUE;
    if ( nAct < nMust )
        SetParameterExpected();
    else
        SetIllegalParameter();
    return FALSE;
}


inline BOOL ScInterpreter::MustHaveParamCount( BYTE nAct, BYTE nMust, BYTE nMax )
{
    if ( nMust <= nAct && nAct <= nMax )
        return TRUE;
    if ( nAct < nMust )
        SetParameterExpected();
    else
        SetIllegalParameter();
    return FALSE;
}


inline BOOL ScInterpreter::MustHaveParamCountMin( BYTE nAct, BYTE nMin )
{
    if ( nAct >= nMin )
        return TRUE;
    SetParameterExpected();
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

} //namespace binfilter
#endif
