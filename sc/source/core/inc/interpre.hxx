/*************************************************************************
 *
 *  $RCSfile: interpre.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:16 $
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

#ifndef SC_INTERPRE_HXX
#define SC_INTERPRE_HXX

#define SC_SPEW_ENABLED 0

#include <math.h>

#include <tools/solmath.hxx>    // SOMA_FPSIGNAL_JUMP

#include "compiler.hxx"
#include "cell.hxx"
#include "scdll.hxx"
#if SC_SPEW_ENABLED
#include "scspew.hxx"
#endif

class ScDocument;
class ScMatrix;
class SbxVariable;
class ScBaseCell;
class ScFormulaCell;
class SvNumberFormatter;

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

#ifndef _TABOPLIST_DECLARED
#define _TABOPLIST_DECLARED
DECLARE_LIST (TabOpList, USHORT*);
#endif

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

struct ScToken;

#define MAXSTACK      (4096 / sizeof(ScToken*))

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
    ifSUM,                              // Aufsummieren
    ifSUMSQ,                            // Quadratsummen
    ifPRODUCT,                          // Multiplizieren
    ifAVERAGE,                          // Durchschnitt
    ifCOUNT,                            // Anzahl Werte
    ifCOUNT2,                           // Anzahl Werte (nichtleer)
    ifMIN,                              // Minimum
    ifMAX                               // Maximum
};

class ScInterpreter
{
public:
    DECL_FIXEDMEMPOOL_NEWDEL( ScInterpreter )
#if SOMA_FPSIGNAL_JUMP
    static jmp_buf* pJumpBuf;           // fuer die mySignal-Funktion
    static ScLibSignalFunc pSignalFunc; // signal() Wrapper der App
#endif
    static USHORT nGlobalError;         // globale Fehlervariable
    static ULONG nInterpCount;          // Zaehlt aktuelle Interpreteraufrufe
    static TabOpList aTableOpList;      // Liste von Ref-Arrays fuer Mehrfachoperat.
#if SC_SPEW_ENABLED
    static ScSpew theSpew;
#endif

    static void GlobalExit();           // aus ScGlobal::Clear() gerufen
    static BOOL MayBeRegExp( const String& );   // vielleicht regular expression?
private:
    static ScTokenStack*    pGlobalStack;
    static ScErrorStack*    pGlobalErrorStack;
    static BOOL             bGlobalStackInUse;
    static const sal_Unicode cEmptyString;      // =0 for return &cEmptyString

    ScTokenIterator aCode;
    ScAddress   aPos;
    ScTokenArray& rArr;
    String      aResult;
    ScDocument* pDok;                   // Pointer aufs Dokument
    double      nResult;
    ScMatrix*   pResult;
    ScFormulaCell* pMyFormulaCell;      // die Zelle mit der Formel
    SvNumberFormatter* pFormatter;
    StackVar    eResult;

    USHORT      nGlobError;             // lokale Kopie
#if SOMA_FPSIGNAL_JUMP
    jmp_buf*    pLocalJumpBuf;          // lokale Kopie
#endif
    const ScToken* pCur;                // aktuelles Token
    String      aTempStr;               // fuer GetString()
    ScTokenStack* pStackObj;            // enthaelt den Stack
    ScToken**   pStack;                 // der Stack
    ScErrorStack* pErrorStackObj;       // enthaelt den ErrorStack
    USHORT*     pErrorStack;            // der ErrorStack
    USHORT      sp;                     // der Stackpointer
    USHORT      maxsp;                  // der maximale StackPointer
    double**    ppGlobSortArray;        // Pointer auf Array zum Sortieren
    USHORT*     pTableOp;               // der lokale Pointer auf das aktuelle TableOp
    ScMatrix**  ppTempMatArray;         // Array fuer temporaere Matrizen
    USHORT      nMatCount;              // dazugehoeriger Zaehler
    BOOL        bMatDel;                // und Kontrollvariable
    USHORT      nRetMat;                // Index der Return-Matrix
    ULONG       nFuncFmtIndex;          // NumberFormatIndex einer Funktion
    ULONG       nCurFmtIndex;           // aktueller NumberFormatIndex
    ULONG       nRetFmtIndex;           // ggbf. NumberFormatIndex des Ausdrucks
    short       nFuncFmtType;           // NumberFormatTyp einer Funktion
    short       nCurFmtType;            // aktueller NumberFormatTyp
    short       nRetFmtType;            // NumberFormatTyp des Ausdrucks
    BOOL        glSubTotal;             // Flag fuer Subtotalfunktionen
    BYTE        cPar;                   // aktuelle Anzahl Parameter
    BOOL        bCalcAsShown;           // Genauigkeit wie angezeigt
    BOOL        bMatrixFormula;         // Formelzelle ist Matrixformel

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
void ReplaceCell( ScAddress& ); // fuer MehrfachOp.
ULONG GetCellNumberFormat( const ScAddress&, const ScBaseCell* );
double GetCellValue( const ScAddress&, const ScBaseCell* );
double GetCellValueOrZero( const ScAddress&, const ScBaseCell* );
double GetValueCellValue( const ScAddress&, const ScValueCell* );
ScBaseCell* GetCell( const ScAddress& );
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
void PushTempToken( ScDoubleToken* );       //! see warnings in interpr4.cxx
void Pop();
void PopError();
BYTE PopByte();
double PopDouble();
const sal_Unicode* PopString();
void PopSingleRef( ScAddress& );
void PopSingleRef(USHORT& rCol, USHORT &rRow, USHORT& rTab);
void PopDoubleRef( ScRange& );
void PopDoubleRef(USHORT& rCol1, USHORT &rRow1, USHORT& rTab1,
                          USHORT& rCol2, USHORT &rRow2, USHORT& rTab2);
BOOL PopDoubleRefOrSingleRef( ScAddress& rAdr );
void PopDoubleRefPushMatrix();
inline void MatrixDoubleRefToMatrix();      // wenn MatrixFormula: PopDoubleRefPushMatrix
ScMatrix* PopMatrix();
//void PushByte(BYTE nVal);
void PushDouble(double nVal);
void PushInt( int nVal );
void PushString(const sal_Unicode* cString);
void PushStringObject(const String& aString);
void PushSingleRef(USHORT nCol, USHORT nRow, USHORT nTab);
void PushDoubleRef(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                 USHORT nCol2, USHORT nRow2, USHORT nTab2);
void PushMatrix(ScMatrix* pMat);
BYTE GetStackType();
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
const sal_Unicode* GetString();
ScMatrix* GetMatrix(USHORT& nMatInd);                   // in interpr2.cxx
void ScTableOp();                                       // Mehrfachoperationen
void ScErrCell();                                       // Sonderbehandlung
                                                        // Fehlerzelle
void ScDefPar();                                        // DefaultParameter
//-----------------------------allgemeine Hilfsfunktionen
void SetMaxIterationCount(USHORT n);
inline void CurFmtToFuncFmt()
    { nFuncFmtType = nCurFmtType; nFuncFmtIndex = nCurFmtIndex; }
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
void ScIsRef();
void ScIsValue();
void ScIsFormula();
void ScFormula();
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
BOOL GetDBParams(USHORT& rTab, ScQueryParam& rParam);
void DBIterator( ScIterFunc );
void ScDBSum();
void ScDBCount();
void ScDBCount2();
void ScDBAverage();
void ScDBGet();
void ScDBMax();
void ScDBMin();
void ScDBProduct();
ULONG StdDev( double& rSum, double& rSum2 );
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
void ScGame();

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
void RoundNumber( SolarMathRoundingMode eMode );
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
void ScNBW();
void ScIKV();
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
              USHORT& rPermutCounter, BOOL& bIsInvertable);
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
BOOL RGetVariances( ScMatrix* pV, ScMatrix* pX, USHORT nC, USHORT nR,
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
    static void MySigFunc(int sig);

    const     String& GetStringResult() { return aResult; }
    double    GetNumResult()            { return nResult; }
    ScMatrix* GetMatrixResult()         { return pResult; }
    StackVar  GetResultType()           { return eResult; }
    short     GetRetFormatType()    { return nRetFmtType; }
    ULONG     GetRetFormatIndex()   { return nRetFmtIndex; }
    BOOL        HadMatrix() const   { return bMatDel; }
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


#endif
