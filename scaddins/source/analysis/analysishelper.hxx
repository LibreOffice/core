/*************************************************************************
 *
 *  $RCSfile: analysishelper.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: gt $ $Date: 2001-08-23 16:13:18 $
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

#ifndef ANALYSISHELPER_HXX
#define ANALYSISHELPER_HXX


#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>
#include <com/sun/star/util/Date.hpp>

#include <math.h>
//#include <tools/list.hxx>
//#include <tools/solmath.hxx>

#include <tools/resid.hxx>
#include <tools/rc.hxx>

#include "analysisdefs.hxx"
//#include "analysis_solmath.hxx"


using namespace com::sun::star;
using namespace rtl;


class ResMgr;
class SortedIndividualInt32List;


#define PI          3.1415926535897932
#define PI_2        (PI/2.0)
//#define   EULER       2.7182818284590452
#define EOL         ( ( const sal_Char* ) 1 )
#define EOE         ( ( const sal_Char* ) 2 )


//double                _Test( sal_Int32 nMode, double f1, double f2, double f3 );
inline sal_Bool     IsLeapYear( sal_uInt16 nYear );
sal_uInt16          DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear );
sal_uInt16          DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear, sal_Bool bLeapYear );
sal_Int32           DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear );
void                DaysToDate( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear );
sal_Int32           GetNullDate( const REF( beans::XPropertySet )& xOptions ) THROWDEF_RTE;
sal_Int32           GetDiffDate360(
                        sal_uInt16 nDay1, sal_uInt16 nMonth1, sal_uInt16 nYear1, sal_Bool bLeapYear1,
                        sal_uInt16 nDay2, sal_uInt16 nMonth2, sal_uInt16 nYear2,
                        sal_Bool bUSAMethod );
inline sal_Int32    GetDiffDate360( constREFXPS& xOpt, sal_Int32 nDate1, sal_Int32 nDate2, sal_Bool bUSAMethod );
sal_Int32           GetDiffDate360( sal_Int32 nNullDate, sal_Int32 nDate1, sal_Int32 nDate2, sal_Bool bUSAMethod );

sal_Int32           GetDaysInYears( sal_uInt16 nYear1, sal_uInt16 nYear2 );
inline sal_Int16    GetDayOfWeek( sal_Int32 nDate );
void                GetDiffParam( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode,
                        sal_uInt16& rYears, sal_Int32& rDayDiffPart, sal_Int32& rDaysInYear ) THROWDEF_RTE_IAE;
                        // rYears = full num of years
                        // rDayDiffPart = num of days for last year
                        // rDaysInYear = num of days in first year
sal_Int32           GetDiffDate( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode,
                                sal_Int32* pOptDaysIn1stYear = NULL ) THROWDEF_RTE_IAE;
double              GetYearDiff( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                                THROWDEF_RTE_IAE;
sal_Int32           GetDaysInYear( sal_Int32 nNullDate, sal_Int32 nDate, sal_Int32 nMode ) THROWDEF_RTE_IAE;
double              GetYearFrac( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                        THROWDEF_RTE_IAE;
inline double       GetYearFrac( constREFXPS& xOpt, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                        THROWDEF_RTE_IAE;
void                AddDate( sal_uInt16& nDay, sal_uInt16& nMonth, sal_uInt16& nYear,
                            sal_Int32 nDeltaDay, sal_Int32 nDeltaMonth, sal_Int32 nDeltaYear );
void                AddDate( sal_Int32 nNullDate, sal_Int32& rDate,
                            sal_Int32 nDeltaDay, sal_Int32 nDeltaMonth, sal_Int32 nDeltaYear );
inline void         AlignDate( sal_uInt16& rDay, sal_uInt16 nMonth, sal_uInt16 nYear );

double              Fak( sal_Int32 n );
double              GetGcd( double f1, double f2 );
double              GammaHelp( double& x, sal_Bool& bReflect );
double              Gamma( double f );
double              GammaN( double f, sal_uInt32 nIter );
double              Bessel( double fNum, sal_Int32 nOrder, sal_Bool bModfied ) THROWDEF_RTE_IAE;
double              Besselk0( double fNum );
double              Besselk1( double fNum );
double              Besselk( double fNum, sal_Int32 nOrder );
double              Bessely0( double fNum );
double              Bessely1( double fNum );
double              Bessely( double fNum, sal_Int32 nOrder );
double              ConvertToDec( const STRING& rFromNum, sal_uInt16 nBaseFrom, sal_uInt16 nCharLim ) THROWDEF_RTE_IAE;
STRING              ConvertFromDec(
                        sal_Int64 nNum, sal_Int64 nMin, sal_Int64 nMax, sal_uInt16 nBase,
                        sal_Int32 nPlaces, sal_Int32 nMaxPlaces ) THROWDEF_RTE_IAE;
double              Erf( double fX );
sal_Bool            ParseDouble( const sal_Unicode*& rpDoubleAsString, double& rReturn );
STRING              GetString( double fNumber, sal_Bool bLeadingSign = sal_False, sal_uInt16 nMaxNumOfDigits = 15 );
inline double       Exp10( sal_Int16 nPower );      // 10 ^ nPower
sal_Int32           GetOpt( const ANY& rAny, sal_Int32 nDefault ) THROWDEF_RTE_IAE;
double              GetOpt( const ANY& rAny, double fDefault ) THROWDEF_RTE_IAE;
inline sal_Int32    GetOptBase( const ANY& rAny );

double              GetAmordegrc( sal_Int32 nNullDate, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer,
                                double fRestVal, double fPer, double fRate, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetAmorlinc( sal_Int32 nNullDate, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer,
                                double fRestVal, double fPer, double fRate, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetDuration( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fCoup,
                                double fYield, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetYieldmat( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
                                double fRate, double fPrice, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetOddfprice( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
                                sal_Int32 nFirstCoup, double fRate, double fYield, double fRedemp,
                                sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              getYield_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice,
                                double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              getPrice_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield,
                                double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetOddfyield( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
                                sal_Int32 nFirstCoup, double fRate, double fPrice, double fRedemp,
                                sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetOddlprice( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
                                double fRate, double fYield, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetOddlyield( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
                                double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetRmz( double fZins, double fZzr, double fBw, double fZw, sal_Int32 nF );
double              GetZw( double fZins, double fZzr, double fRmz, double fBw, sal_Int32 nF );
//double                TBillYield( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice )THROWDEF_RTE_IAE;

double              GetCouppcd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetCoupncd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetCoupdaybs( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetCoupdaysnc( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) THROWDEF_RTE_IAE;

double              GetCoupnum( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat,
                                sal_Int32 nFreq, sal_Int32 nBase ) THROWDEF_RTE_IAE;
double              GetCoupdays( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) THROWDEF_RTE_IAE;




//-----------------------------------------------------------------------------
// date helper class for financial functions COUP***

class ScAddInDate
{
private:
    void                        SetDay();

    inline sal_uInt16           GetDaysInMonth() const;
    inline sal_uInt16           GetDaysInMonth( sal_uInt16 _nMon ) const;

    sal_Int32                   GetDaysInMonthRange( sal_uInt16 nFrom, sal_uInt16 nTo ) const;
    sal_Int32                   GetDaysInYearRange( sal_uInt16 nFrom, sal_uInt16 nTo ) const;

public:
    sal_uInt16                  nOrigDay;
    sal_uInt16                  nDay;
    sal_uInt16                  nMonth;
    sal_uInt16                  nYear;
    sal_Bool                    bLastDay;
    sal_Bool                    b30Days;
    sal_Bool                    bUSMode;

                                ScAddInDate();
                                ScAddInDate( sal_Int32 nNullDate, sal_Int32 nDate, sal_Int32 nBase );
                                ScAddInDate( const ScAddInDate& rCopy );
    ScAddInDate&                operator=( const ScAddInDate& rCopy );

    void                        AddMonths( sal_uInt16 nAddMonths );
    void                        SubMonths( sal_uInt16 nSubMonths );
    inline void                 SetYear( sal_uInt16 nNewYear );
    inline void                 AddYears( sal_uInt16 nAddYears );
    inline void                 SubYears( sal_uInt16 nSubYears );

    sal_Int32                   GetDate( sal_Int32 nNullDate ) const;
    static sal_Int32            GetDiff( const ScAddInDate& rFrom, const ScAddInDate& rTo );

    sal_Bool                    operator<( const ScAddInDate& rCmp ) const;
    inline sal_Bool             operator<=( const ScAddInDate& rCmp ) const { return !(rCmp < *this); }
    inline sal_Bool             operator>( const ScAddInDate& rCmp ) const  { return rCmp < *this; }
    inline sal_Bool             operator>=( const ScAddInDate& rCmp ) const { return !(*this < rCmp); }
};

inline sal_uInt16 ScAddInDate::GetDaysInMonth() const
{
    return GetDaysInMonth( nMonth );
}

inline sal_uInt16 ScAddInDate::GetDaysInMonth( sal_uInt16 _nMon ) const
{
    return b30Days ? 30 : DaysInMonth( _nMon, nYear );
}

inline void ScAddInDate::SetYear( sal_uInt16 nNewYear )
{
    nYear = nNewYear;
    SetDay();
}

inline void ScAddInDate::AddYears( sal_uInt16 nAddYears )
{
    nYear += nAddYears;
    SetDay();
}

inline void ScAddInDate::SubYears( sal_uInt16 nSubYears )
{
    nYear -= nSubYears;
    SetDay();
}


//-----------------------------------------------------------------------------



class MyList
{
private:
    static const sal_uInt32 nStartSize;
    static const sal_uInt32 nIncrSize;

    void**                  pData;          // pointer array
    sal_uInt32              nSize;          // array size
    sal_uInt32              nNew;           // next index to be inserted at
    sal_uInt32              nAct;           // actual for iterations

    void                    _Grow( void );
    inline void             Grow( void );
protected:
public:
                            MyList( void );
    virtual                 ~MyList();

    inline const void*      GetObject( sal_uInt32 nIndex ) const;
    inline const void*      First( void );
    inline const void*      Next( void );

    inline void             Append( void* pNewElement );
    void                    Insert( void* pNewLement, sal_uInt32 nPlace );

    inline sal_uInt32       Count( void ) const;
};




class StringList : protected MyList
{
public:
    virtual                 ~StringList();

    inline const STRING*    First( void );
    inline const STRING*    Next( void );
    inline const STRING*    Get( sal_uInt32 nIndex ) const;

    sal_Bool                Contains( const STRING& rSearchString ) const;

    inline void             Append( STRING* pNew );
    inline void             Append( const STRING& rNew );

    MyList::Count;
};




enum FDCategory
{
    FDCat_AddIn,
    FDCat_DateTime,
    FDCat_Finance,
    FDCat_Inf,
    FDCat_Math,
    FDCat_Tech
};


struct FuncDataBase
{
    const sal_Char*         pIntName;
    sal_uInt16              nUINameID;          // resource ID to UI name
    sal_uInt16              nDescrID;           // resource ID to description, parameter names and ~ description
    sal_Bool                bDouble;            // name already exist in Calc
    sal_Bool                bWithOpt;           // first parameter is internal
    sal_uInt16              nCompListID;        // resource ID to list of valid names
    sal_uInt16              nNumOfParams;       // number of named / described parameters
    FDCategory              eCat;               // function category
};




class FuncData
{
private:
    OUString                aIntName;
    sal_uInt16              nUINameID;
    sal_uInt16              nDescrID;           // leads also to parameter descriptions!
    sal_Bool                bDouble;            // flag for names, wich already exist in Calc
    sal_Bool                bWithOpt;           // has internal parameter on first position

    sal_uInt16              nParam;             // num of parameters
    sal_uInt16              nCompID;
    StringList              aCompList;          // list of all valid names
    FDCategory              eCat;               // function category
public:
                            FuncData( const FuncDataBase& rBaseData, ResMgr& );
    virtual                 ~FuncData();

    inline sal_uInt16       GetUINameID( void ) const;
    inline sal_uInt16       GetDescrID( void ) const;
    inline sal_Bool         IsDouble( void ) const;
    inline sal_Bool         HasIntParam( void ) const;

    sal_uInt16              GetStrIndex( sal_uInt16 nParamNum ) const;
    inline sal_Bool         Is( const OUString& rCompareTo ) const;

    inline const StringList&    GetCompNameList( void ) const;

    inline FDCategory       GetCategory( void ) const;
};




class CStrList : private MyList
{
public:
    inline void             Append( const sal_Char* pNew );
    inline const sal_Char*  Get( sal_uInt32 nIndex ) const;
    MyList::Count;
};




class FuncDataList : private MyList
{
    OUString                aLastName;
    sal_uInt32              nLast;
public:
                            FuncDataList( ResMgr& );
    virtual                 ~FuncDataList();
    inline void             Append( FuncData* pNew );
    inline const FuncData*  Get( sal_uInt32 nIndex ) const;
    MyList::Count;

    const FuncData*         Get(  const OUString& aProgrammaticName ) const;
};



class AnalysisResId : public ResId
{
 public:
                    AnalysisResId( sal_uInt16 nId, ResMgr& rResMgr );
};




class AnalysisRscStrLoader : public Resource
{
private:
    String          aStr;
public:
    AnalysisRscStrLoader( sal_uInt16 nRsc, sal_uInt16 nStrId, ResMgr& rResMgr ) :
        Resource( AnalysisResId( nRsc, rResMgr ) ),
        aStr( AnalysisResId( nStrId, rResMgr ) )
    {
        FreeResource();
    }

    const String&   GetString() const { return aStr; }

};




class SortedIndividualInt32List : private MyList
{
    // sorted list were values are unique
private:
protected:
public:
                        SortedIndividualInt32List();
    virtual             ~SortedIndividualInt32List();

    void                Insert( sal_Int32 nNewVal );

    MyList::Count;

    inline sal_Int32    Get( sal_uInt32 nIndex ) const; // reterns a value in every situation,
                                                        //  even if nIndex is invalid,
                                                        //  so care about nIndex _before_ using Get()

    sal_Int32           CountCondition( sal_Int32 nMinVal, sal_Int32 nMaxVal ) const;
                                                        // count number of elements, which are in the range
                                                        //  of nMinVal ... nMaxVal (both included)
    sal_Bool            Find( sal_Int32 nVal ) const;

    void                InsertHolidayList(
                            const SEQSEQ( sal_Int32 )& aHDay,
                            sal_Int32 nNullDate, sal_Bool bInsertAlsoOnWeekends );
    void                InsertHolidayList(
                            const SEQ( double )& aHDay,
                            sal_Int32 nNullDate, sal_Bool bInsertAlsoOnWeekends ) THROWDEF_RTE_IAE;
    void                InsertHolidayList(
                            const ANY& aHDay,
                            sal_Int32 nNullDate, sal_Bool bInsertAlsoOnWeekends ) THROWDEF_RTE_IAE;
};




class DoubleList : protected MyList
{
protected:
    inline void             _Append( double fVal );
    inline void             AppendVoid( sal_Bool bForceErrorOnEmpty ) THROWDEF_RTE_IAE;
    inline void             AppendDouble( double fVal ) THROWDEF_RTE_IAE;
    void                    AppendString( const ANY& r, sal_Bool bEmptyStringAs0 ) THROWDEF_RTE_IAE;
    void                    AppendDouble( const ANY& r ) THROWDEF_RTE_IAE;
    inline void             AppendAnyArray2( const ANY& r ) THROWDEF_RTE_IAE;
public:
    virtual                 ~DoubleList();


    inline const double*    Get( sal_uInt32 nIndex ) const;
    inline const double*    First( void );
    inline const double*    Next( void );

    MyList::Count;

    sal_Bool                Append( double fVal );
    void                    Append( const SEQSEQ( double )& aValList ) THROWDEF_RTE_IAE;
    void                    Append( const SEQSEQ( sal_Int32 )& aValList ) THROWDEF_RTE_IAE;
    void                    Append( const SEQ( ANY )& aValList,
                                sal_Bool bEmptyStringAs0 = sal_True,
                                sal_Bool bForceErrorOnEmpty = sal_False ) THROWDEF_RTE_IAE;
                                // when bEmptyStringAs0, no empty entry is possible as result ->
                                //  bForceErrorOnEmpty has no effect, but an exception is thrown,
                                //  when the string is _not_ empty
                                // when bForceErrorOnEmpty, no voids and empty strings are allowed
    void                    Append( const SEQSEQ( ANY )& aValList,
                                sal_Bool bEmptyStringAs0 = sal_True,
                                sal_Bool bForceErrorOnEmpty = sal_False ) THROWDEF_RTE_IAE;
    virtual sal_Bool        IsProper( double fVal ) const;
    virtual sal_Bool        IsFaulty( double fVal ) const;
};




class ChkDoubleList1 : public DoubleList
{// proper values are > 0.0
public:
    virtual sal_Bool        IsProper( double fVal ) const;
    virtual sal_Bool        IsFaulty( double fVal ) const;
};




class ChkDoubleList2 : public DoubleList
{   // proper values are >= 0.0
public:
    virtual sal_Bool        IsProper( double fVal ) const;
    virtual sal_Bool        IsFaulty( double fVal ) const;
};




class Complex
{
    double                  r;
    double                  i;

public:
//  inline                  Complex( void );
    inline                  Complex( double fReal, double fImag = 0.0 );
    inline                  Complex( const Complex& rCopy );
                            Complex( const STRING& rComplexAsString ) THROWDEF_RTE_IAE;

    inline static sal_Bool  IsImagUnit( sal_Unicode c );
    static sal_Bool         ParseString( const STRING& rComplexAsString, Complex& rReturn );
    STRING                  GetString( sal_Bool bUse_i = sal_True ) const;

    inline double           Real( void ) const;
    inline double           Imag( void ) const;

    double                  Arg( void ) const THROWDEF_RTE_IAE;
    inline double           Abs( void ) const;

    // following functions change the complex number itself to avoid unnecessary copy actions!
    void                    Power( double fPower ) THROWDEF_RTE_IAE;
    void                    Sqrt( void );
    void                    Sin( void ) THROWDEF_RTE_IAE;
    void                    Cos( void ) THROWDEF_RTE_IAE;
    void                    Div( const Complex& rDivisor ) THROWDEF_RTE_IAE;
    void                    Exp( void );
    inline void             Conjugate( void );
    void                    Ln( void ) THROWDEF_RTE_IAE;
    void                    Log10( void ) THROWDEF_RTE_IAE;
    void                    Log2( void ) THROWDEF_RTE_IAE;
    inline void             Mult( double fFact );
    inline void             Mult( const Complex& rMult );
    inline void             Sub( const Complex& rMult );
    inline void             Add( const Complex& rAdd );
};




enum ComplListAppendHandl
{
    AH_EmptyAsErr,
    AH_EmpyAs0,
    AH_IgnoreEmpty
};


class ComplexList : protected MyList
{
public:
    virtual                 ~ComplexList();

    inline const Complex*   Get( sal_uInt32 nIndex ) const;
    inline const Complex*   First( void );
    inline const Complex*   Next( void );

    MyList::Count;

    inline void             Append( Complex* pNew );
    void                    Append( const SEQSEQ( STRING )& rComplexNumList, ComplListAppendHandl eAH = AH_EmpyAs0 ) THROWDEF_RTE_IAE;
    void                    Append( const SEQ( ANY )& aMultPars,ComplListAppendHandl eAH = AH_EmpyAs0 ) THROWDEF_RTE_IAE;
};




enum ConvertDataClass
{
    CDC_Mass, CDC_Length, CDC_Time, CDC_Pressure, CDC_Force, CDC_Energy, CDC_Power, CDC_Magnetism,
    CDC_Temperature, CDC_Volume, CDC_Area, CDC_Speed
};


#define INV_MATCHLEV        1764                    // guess, what this is... :-)


class ConvertDataList;




class ConvertData
{
protected:
    friend ConvertDataList;
    double                  fConst;
    STRING                  aName;
    ConvertDataClass        eClass;
public:
                            ConvertData(
                                const sal_Char      pUnitName[],
                                double              fConvertConstant,
                                ConvertDataClass    eClass );

    sal_Int16               GetMatchingLevel( const STRING& rRef ) const;
                                    // 0.0 = no equality
                                    // 1.0 = matches exact
                                    // rest = matches without an assumed prefix of one character
                                    //  rest gives power for 10 represented by the prefix (e.g. 3 for k or -9 for n

    virtual double          Convert( double fVal, const ConvertData& rTo,
                                sal_Int16 nMatchLevelFrom, sal_Int16 nMatchLevelTo ) const THROWDEF_RTE_IAE;
                                    // converts fVal from this unit to rFrom unit
                                    // throws exception if not from same class
                                    // this implementation is for proportional cases only
    virtual double          ConvertToBase( double fVal, sal_Int16 nMatchLevel ) const;
    virtual double          ConvertFromBase( double fVal, sal_Int16 nMatchLevel ) const;

    inline ConvertDataClass Class( void ) const;
};




class ConvertDataLinear : public ConvertData
{
protected:
    double                  fOffs;
public:
    inline                  ConvertDataLinear(
                                const sal_Char      pUnitName[],
                                double              fConvertConstant,
                                double              fConvertOffset,
                                ConvertDataClass    eClass );

    virtual double          Convert( double fVal, const ConvertData& rTo,
                                sal_Int16 nMatchLevelFrom, sal_Int16 nMatchLevelTo ) const THROWDEF_RTE_IAE;
                                    // for cases where f(x) = a + bx applies (e.g. Temperatures)

    virtual double          ConvertToBase( double fVal, sal_Int16 nMatchLevel ) const;
    virtual double          ConvertFromBase( double fVal, sal_Int16 nMatchLevel ) const;
};




class ConvertDataList : protected MyList
{
private:
protected:
    inline ConvertData*     First( void );
    inline ConvertData*     Next( void );
public:
                            ConvertDataList( void );
    virtual                 ~ConvertDataList();

    double                  Convert( double fVal, const STRING& rFrom, const STRING& rTo ) THROWDEF_RTE_IAE;
};




inline sal_Bool IsLeapYear( sal_uInt16 n )
{
    return ( ( ( n % 4 ) == 0 ) && ( ( n % 100 ) != 0) || ( ( n % 400 ) == 0 ) );
}


inline sal_Int32 GetDiffDate360( constREFXPS& xOpt, sal_Int32 nDate1, sal_Int32 nDate2, sal_Bool bUSAMethod )
{
    return GetDiffDate360( GetNullDate( xOpt ), nDate1, nDate2, bUSAMethod );
}


inline sal_Int16 GetDayOfWeek( sal_Int32 n )
{   // monday = 0, ..., sunday = 6
    return ( n - 1 ) % 7;
}


inline double GetYearFrac( constREFXPS& xOpt, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode ) THROWDEF_RTE_IAE
{
    return GetYearFrac( GetNullDate( xOpt ), nStartDate, nEndDate, nMode );
}


inline void AlignDate( sal_uInt16& rD, sal_uInt16 nM, sal_uInt16 nY )
{
    sal_uInt16  nMax = DaysInMonth( nM, nY );

    if( rD > nMax )
        rD = nMax;
}


inline sal_Int32 GetOptBase( const ANY& r )
{
    return GetOpt( r, sal_Int32( 0 ) );
}




inline void MyList::Grow( void )
{
    if( nNew >= nSize )
        _Grow();
}


inline const void* MyList::GetObject( sal_uInt32 n ) const
{
    if( n < nNew )
        return pData[ n ];
    else
        return NULL;
}


inline const void* MyList::First( void )
{
    nAct = 0;
    if( nNew )
        return pData[ 0 ];
    else
        return NULL;
}


inline const void* MyList::Next( void )
{
    nAct++;
    if( nAct < nNew )
        return pData[ nAct ];
    else
    {
        nAct--;
        return NULL;
    }
}


inline void MyList::Append( void* p )
{
    Grow();
    pData[ nNew ] = p;
    nNew++;
}


inline sal_uInt32 MyList::Count( void ) const
{
    return nNew;
}




inline const STRING* StringList::First( void )
{
    return ( const STRING* ) MyList::First();
}


inline const STRING* StringList::Next( void )
{
    return ( const STRING* ) MyList::Next();
}


inline const STRING* StringList::Get( sal_uInt32 n ) const
{
    return ( const STRING* ) MyList::GetObject( n );
}


inline void StringList::Append( STRING* p )
{
    MyList::Append( p );
}


inline void StringList::Append( const STRING& r )
{
    MyList::Append( new STRING( r ) );
}




inline sal_uInt16 FuncData::GetUINameID( void ) const
{
    return nUINameID;
}


inline sal_uInt16 FuncData::GetDescrID( void ) const
{
    return nDescrID;
}


inline sal_Bool FuncData::IsDouble( void ) const
{
    return bDouble;
}


inline sal_Bool FuncData::HasIntParam( void ) const
{
    return bWithOpt;
}


inline sal_Bool FuncData::Is( const OUString& r ) const
{
    return aIntName == r;
}


inline const StringList& FuncData::GetCompNameList( void ) const
{
    return aCompList;
}


inline FDCategory FuncData::GetCategory( void ) const
{
    return eCat;
}




inline void CStrList::Append( const sal_Char* p )
{
    MyList::Append( ( void* ) p );
}


inline const sal_Char* CStrList::Get( sal_uInt32 n ) const
{
    return ( const sal_Char* ) MyList::GetObject( n );
}




inline void FuncDataList::Append( FuncData* p )
{
    MyList::Append( p );
}


inline const FuncData* FuncDataList::Get( sal_uInt32 n ) const
{
    return ( const FuncData* ) MyList::GetObject( n );
}




inline sal_Int32 SortedIndividualInt32List::Get( sal_uInt32 n ) const
{
    return ( sal_Int32 ) MyList::GetObject( n );
}




inline void DoubleList::_Append( double f )
{
    MyList::Append( new double( f ) );
}


inline const double* DoubleList::Get( sal_uInt32 n ) const
{
    return ( const double* ) MyList::GetObject( n );
}


inline const double* DoubleList::First( void )
{
    return ( const double* ) MyList::First();
}


inline const double* DoubleList::Next( void )
{
    return ( const double* ) MyList::Next();
}


inline sal_Bool DoubleList::Append( double f )
{
    if( IsFaulty( f ) )
        return sal_False;
    if( IsProper( f ) )
        _Append( f );
    return sal_True;
}




/*inline Complex::Complex( void )
{
//#ifdef DEBUG
//  SetNAN( i );
//  r = i;
//#endif
}*/


inline Complex::Complex( double fReal, double fImag ) : r( fReal ), i( fImag )
{
}


inline Complex::Complex( const Complex& rCpy ) : r( rCpy.r ), i( rCpy.i )
{
}


inline double Complex::Real( void ) const
{
    return r;
}


inline double Complex::Imag( void ) const
{
    return i;
}


inline double Complex::Abs( void ) const
{
    return sqrt( r * r + i * i );
}


void Complex::Conjugate( void )
{
    i = -i;
}


inline void Complex::Mult( double f )
{
    i *= f;
    r *= f;
}


inline void Complex::Mult( const Complex& rM )
{
    double  r_ = r;
    double  i_ = i;

    r = r_ * rM.r - i_ * rM.i;
    i = r_ * rM.i + i_ * rM.r;
}


inline void Complex::Sub( const Complex& rC )
{
    r -= rC.r;
    i -= rC.i;
}


inline void Complex::Add( const Complex& rAdd )
{
    r += rAdd.r;
    i += rAdd.i;
}




inline const Complex* ComplexList::Get( sal_uInt32 n ) const
{
    return ( const Complex* ) MyList::GetObject( n );
}


inline const Complex* ComplexList::First( void )
{
    return ( const Complex* ) MyList::First();
}


inline const Complex* ComplexList::Next( void )
{
    return ( const Complex* ) MyList::Next();
}


inline void ComplexList::Append( Complex* p )
{
    MyList::Append( p );
}




inline ConvertDataClass ConvertData::Class( void ) const
{
    return eClass;
}




inline ConvertDataLinear::ConvertDataLinear( const sal_Char* p, double fC, double fO, ConvertDataClass e ) :
    ConvertData( p, fC, e ),
    fOffs( fO )
{
}




inline ConvertData* ConvertDataList::First( void )
{
    return ( ConvertData* ) MyList::First();
}


inline ConvertData* ConvertDataList::Next( void )
{
    return ( ConvertData* ) MyList::Next();
}


#endif

