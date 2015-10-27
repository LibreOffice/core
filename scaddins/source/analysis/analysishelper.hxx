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
#ifndef INCLUDED_SCADDINS_SOURCE_ANALYSIS_ANALYSISHELPER_HXX
#define INCLUDED_SCADDINS_SOURCE_ANALYSIS_ANALYSISHELPER_HXX


#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/XNumberFormatter2.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>

#include <math.h>

#include <tools/resid.hxx>
#include <tools/rc.hxx>

#include "analysisdefs.hxx"


class ResMgr;

namespace sca { namespace analysis {

class SortedIndividualInt32List;
class ScaAnyConverter;


#define PI          3.1415926535897932
#define EOL         ( ( const sal_Char* ) 1 )
#define EOE         ( ( const sal_Char* ) 2 )


inline bool     IsLeapYear( sal_uInt16 nYear );

#ifdef DISABLE_DYNLOADING

// Avoid clash with the functions with same name in
// scaddins/source/datefunc/datefunc.cxx. I am not sure if each pair
// have identical semantics, but if yes, one copy should be enough,
// but what would be a suitable library where such functions could go?
// Or can the analysis library depend on the date library or the other
// way around?

#define DaysInMonth analysishelper_DaysInMonth
#define DateToDays analysishelper_DateToDays
#define DaysToDate analysishelper_DaysToDate
#define GetNullDate analysishelper_GetNullDate

#endif

sal_uInt16          DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear );
sal_Int32           DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear );
void                DaysToDate( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear ) throw( css::lang::IllegalArgumentException );
sal_Int32           GetNullDate( const css::uno::Reference< css::beans::XPropertySet >& xOptions ) throw( css::uno::RuntimeException );
sal_Int32           GetDiffDate360(
                        sal_uInt16 nDay1, sal_uInt16 nMonth1, sal_uInt16 nYear1, bool bLeapYear1,
                        sal_uInt16 nDay2, sal_uInt16 nMonth2, sal_uInt16 nYear2,
                        bool bUSAMethod );
inline sal_Int32    GetDiffDate360( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nDate1, sal_Int32 nDate2, bool bUSAMethod );
sal_Int32           GetDiffDate360( sal_Int32 nNullDate, sal_Int32 nDate1, sal_Int32 nDate2, bool bUSAMethod );

sal_Int32           GetDaysInYears( sal_uInt16 nYear1, sal_uInt16 nYear2 );
inline sal_Int16    GetDayOfWeek( sal_Int32 nDate );
sal_Int32           GetDiffDate( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode,
                                sal_Int32* pOptDaysIn1stYear = NULL ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetYearDiff( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                                throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
sal_Int32           GetDaysInYear( sal_Int32 nNullDate, sal_Int32 nDate, sal_Int32 nMode ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetYearFrac( sal_Int32 nNullDate, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                        throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
inline double       GetYearFrac( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                        throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

double              BinomialCoefficient( double n, double k );
double              GetGcd( double f1, double f2 );
double              ConvertToDec( const OUString& rFromNum, sal_uInt16 nBaseFrom, sal_uInt16 nCharLim ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
OUString              ConvertFromDec(
                        double fNum, double fMin, double fMax, sal_uInt16 nBase,
                        sal_Int32 nPlaces, sal_Int32 nMaxPlaces, bool bUsePlaces ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              Erf( double fX );
double              Erfc( double fX );
bool                ParseDouble( const sal_Unicode*& rpDoubleAsString, double& rReturn );
OUString            GetString( double fNumber, bool bLeadingSign = false, sal_uInt16 nMaxNumOfDigits = 15 );

double              GetAmordegrc( sal_Int32 nNullDate, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer,
                                double fRestVal, double fPer, double fRate, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetAmorlinc( sal_Int32 nNullDate, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer,
                                double fRestVal, double fPer, double fRate, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetDuration( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fCoup,
                                double fYield, sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetYieldmat( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
                                double fRate, double fPrice, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetOddfprice( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
                                sal_Int32 nFirstCoup, double fRate, double fYield, double fRedemp,
                                sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              getYield_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice,
                                double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              getPrice_( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield,
                                double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetOddfyield( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue,
                                sal_Int32 nFirstCoup, double fRate, double fPrice, double fRedemp,
                                sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetOddlprice( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
                                double fRate, double fYield, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetOddlyield( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
                                double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetRmz( double fZins, double fZzr, double fBw, double fZw, sal_Int32 nF );
double              GetZw( double fZins, double fZzr, double fRmz, double fBw, sal_Int32 nF );

double              GetCouppcd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetCoupncd( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetCoupdaybs( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetCoupdaysnc( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

double              GetCoupnum( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat,
                                sal_Int32 nFreq, sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
double              GetCoupdays( sal_Int32 nNullDate, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq,
                                sal_Int32 nBase ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );




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
    bool                bDouble;            // name already exist in Calc
    bool                bWithOpt;           // first parameter is internal
    sal_uInt16              nCompListID;        // resource ID to list of valid names
    sal_uInt16              nNumOfParams;       // number of named / described parameters
    FDCategory              eCat;               // function category
    const char*             pSuffix;            // if bDouble, append a suffix other than "_ADD" for UI
};


class FuncData
{
private:
    OUString         aIntName;
    sal_uInt16              nUINameID;
    sal_uInt16              nDescrID;           // leads also to parameter descriptions!
    bool                bDouble;            // flag for names that already exist in Calc
    bool                bWithOpt;           // has internal parameter on first position

    sal_uInt16              nParam;             // num of parameters
    sal_uInt16              nCompID;
    std::vector<OUString>  aCompList;          // list of all valid names
    FDCategory              eCat;               // function category
    OUString                aSuffix;            // if bDouble and not empty, append a suffix other than "_ADD" for UI

public:
                            FuncData( const FuncDataBase& rBaseData, ResMgr& );
    virtual                 ~FuncData();

    inline sal_uInt16       GetUINameID() const;
    inline sal_uInt16       GetDescrID() const;
    inline bool             IsDouble() const;
    inline const OUString&  GetSuffix() const;

    sal_uInt16              GetStrIndex( sal_uInt16 nParamNum ) const;
    inline bool             Is( const OUString& rCompareTo ) const;

    inline const std::vector<OUString> &
                            GetCompNameList() const;

    inline FDCategory       GetCategory() const;
};

typedef std::vector< FuncData > FuncDataList;

void InitFuncDataList ( FuncDataList& rList, ResMgr& rResMgr );

// Predicate for use with std::find_if
struct FindFuncData
{
    const OUString& m_rId;
    explicit FindFuncData( const OUString& rId ) : m_rId(rId) {}
    bool operator() ( FuncData& rCandidate ) const { return rCandidate.Is(m_rId); }
};

class AnalysisResId : public ResId
{
 public:
                    AnalysisResId( sal_uInt16 nId, ResMgr& rResMgr );
};


class AnalysisRscStrLoader : public Resource
{
private:
    OUString        aStr;
public:
    AnalysisRscStrLoader( sal_uInt16 nRsc, sal_uInt16 nStrId, ResMgr& rResMgr ) :
        Resource( AnalysisResId( nRsc, rResMgr ) ),
        aStr( AnalysisResId( nStrId, rResMgr ) )
    {
        FreeResource();
    }

    const OUString& GetString() const { return aStr; }

};



/// sorted list with unique sal_Int32 values
class SortedIndividualInt32List
{
private:
    std::vector<sal_Int32>      maVector;
protected:
    void                        Insert( sal_Int32 nDay );
    void                        Insert( sal_Int32 nDay, sal_Int32 nNullDate, bool bInsertOnWeekend );
    void                        Insert( double fDay, sal_Int32 nNullDate, bool bInsertOnWeekend )
                                    throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bInsertOnWeekend  insertion mode: false = holidays on weekend are omitted */
    void                        InsertHolidayList(
                                    const ScaAnyConverter& rAnyConv,
                                    const css::uno::Any& rHolAny,
                                    sal_Int32 nNullDate,
                                    bool bInsertOnWeekend ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

public:
                                SortedIndividualInt32List();
    virtual                     ~SortedIndividualInt32List();

    inline sal_uInt32           Count() const
                                    { return maVector.size(); }

                                /// @return  element on position nIndex or 0 on invalid index
    inline sal_Int32            Get( sal_uInt32 n ) const
                                    { return maVector[n]; }

                                /// @return  true if nVal (internal date representation) is contained
    bool                        Find( sal_Int32 nVal ) const;

                                /** @param rAnyConv  is an initialized or uninitialized ScaAnyConverter
                                    @param bInsertOnWeekend  insertion mode: false = holidays on weekend are omitted */
    void                        InsertHolidayList(
                                    ScaAnyConverter& rAnyConv,
                                    const css::uno::Reference< css::beans::XPropertySet >& xOptions,
                                    const css::uno::Any& rHolAny,
                                    sal_Int32 nNullDate,
                                    bool bInsertOnWeekend ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
};


class ScaDoubleList
{
private:
    std::vector<double>         maVector;
protected:
    inline void                 ListAppend( double fValue ) { maVector.push_back(fValue); }

    inline void                 Append( double fValue ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException )
                                    { if( CheckInsert( fValue ) ) ListAppend( fValue ); }

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: false = inserted as 0.0; true = omitted */
    void                        Append(
                                    const ScaAnyConverter& rAnyConv,
                                    const css::uno::Any& rAny,
                                    bool bIgnoreEmpty ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: false = inserted as 0.0; true = omitted */
    void                        Append(
                                    const ScaAnyConverter& rAnyConv,
                                    const css::uno::Sequence< css::uno::Any >& rAnySeq,
                                    bool bIgnoreEmpty ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: false = inserted as 0.0; true = omitted */
    void                        Append(
                                    const ScaAnyConverter& rAnyConv,
                                    const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& rAnySeq,
                                    bool bIgnoreEmpty ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

public:
    virtual                     ~ScaDoubleList() {}

    inline sal_uInt32           Count() const
                                    { return maVector.size(); }
    inline double               Get( sal_uInt32 n ) const
                                        { return maVector[n]; }

    void                        Append( const css::uno::Sequence< css::uno::Sequence< double > >& rValueArr )
                                    throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                        Append( const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& rValueArr )
                                    throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

                                /** @param rAnyConv  is an initialized or uninitialized ScaAnyConverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: false = inserted as 0.0; true = omitted */
    void                        Append(
                                    ScaAnyConverter& rAnyConv,
                                    const css::uno::Reference< css::beans::XPropertySet >& xOpt,
                                    const css::uno::Sequence< css::uno::Any >& rAnySeq,
                                    bool bIgnoreEmpty = true ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

    virtual bool                CheckInsert( double fValue ) const
                                    throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
};


/// stores double values >0.0, throws exception for double values <0.0, does nothing for 0.0
class ScaDoubleListGT0 : public ScaDoubleList
{
public:
    virtual bool                CheckInsert( double fValue ) const
                                    throw( css::uno::RuntimeException, css::lang::IllegalArgumentException ) override;
};


/// stores double values >=0.0, throws exception for double values <0.0
class ScaDoubleListGE0 : public ScaDoubleList
{
public:
    virtual bool                CheckInsert( double fValue ) const
                                    throw( css::uno::RuntimeException, css::lang::IllegalArgumentException ) override;
};


class Complex
{
    double                  r;
    double                  i;
    sal_Unicode             c;

public:
    inline                  Complex( double fReal, double fImag = 0.0, sal_Unicode cC = '\0' );
                            Complex( const OUString& rComplexAsString ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

    inline static bool      IsImagUnit( sal_Unicode c );
    static bool             ParseString( const OUString& rComplexAsString, Complex& rReturn );
    OUString                GetString() const throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

    inline double           Real() const;
    inline double           Imag() const;

    double                  Arg() const throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    inline double           Abs() const;

    // following functions change the complex number itself to avoid unnecessary copy actions!
    void                    Power( double fPower ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Sqrt();
    void                    Sin() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Cos() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Div( const Complex& rDivisor ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Exp();
    inline void             Conjugate();
    void                    Ln() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Log10() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Log2() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    inline void             Mult( double fFact );
    inline void             Mult( const Complex& rMult );
    inline void             Sub( const Complex& rMult );
    inline void             Add( const Complex& rAdd );
    void                    Tan() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Sec() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Csc() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Cot() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Sinh() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Cosh() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Sech() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Csch() throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

};


enum ComplListAppendHandl
{
    AH_EmptyAsErr,
    AH_EmpyAs0,
    AH_IgnoreEmpty
};


class ComplexList
{
private:
    std::vector<Complex*>  maVector;
public:
    virtual                 ~ComplexList();

    inline const Complex*   Get( sal_uInt32 nIndex ) const;

    inline bool             empty() const
                                { return maVector.empty(); }
    inline sal_uInt32       Count() const
                                { return maVector.size(); }

    inline void             Append( Complex* pNew );
    void                    Append( const css::uno::Sequence< css::uno::Sequence< OUString > >& rComplexNumList, ComplListAppendHandl eAH = AH_EmpyAs0 ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
    void                    Append( const css::uno::Sequence< css::uno::Any >& aMultPars,ComplListAppendHandl eAH = AH_EmpyAs0 ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
};


enum ConvertDataClass
{
    CDC_Mass, CDC_Length, CDC_Time, CDC_Pressure, CDC_Force, CDC_Energy, CDC_Power, CDC_Magnetism,
    CDC_Temperature, CDC_Volume, CDC_Area, CDC_Speed, CDC_Information
};


#define INV_MATCHLEV        1764                    // guess, what this is... :-)


class ConvertDataList;

class ConvertData
{
protected:
    friend class ConvertDataList;
    double                  fConst;
    OUString                  aName;
    ConvertDataClass        eClass;
    bool                bPrefixSupport;
public:
                            ConvertData(
                                const sal_Char      pUnitName[],
                                double              fConvertConstant,
                                ConvertDataClass    eClass,
                                bool                bPrefSupport = false );

    virtual                 ~ConvertData();

    sal_Int16               GetMatchingLevel( const OUString& rRef ) const;
                                    // 0.0 = no equality
                                    // 1.0 = matches exact
                                    // rest = matches without an assumed prefix of one character
                                    //  rest gives power for 10 represented by the prefix (e.g. 3 for k or -9 for n

    virtual double          Convert( double fVal, const ConvertData& rTo,
                                sal_Int16 nMatchLevelFrom, sal_Int16 nMatchLevelTo ) const throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
                                    // converts fVal from this unit to rFrom unit
                                    // throws exception if not from same class
                                    // this implementation is for proportional cases only
    virtual double          ConvertToBase( double fVal, sal_Int16 nMatchLevel ) const;
    virtual double          ConvertFromBase( double fVal, sal_Int16 nMatchLevel ) const;

    inline ConvertDataClass Class() const;
    inline bool         IsPrefixSupport() const;
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
                                ConvertDataClass    eClass,
                                bool            bPrefSupport = false );

    virtual                 ~ConvertDataLinear();

    virtual double          Convert( double fVal, const ConvertData& rTo,
                                sal_Int16 nMatchLevelFrom, sal_Int16 nMatchLevelTo ) const throw( css::uno::RuntimeException, css::lang::IllegalArgumentException ) override;
                                    // for cases where f(x) = a + bx applies (e.g. Temperatures)

    virtual double          ConvertToBase( double fVal, sal_Int16 nMatchLevel ) const override;
    virtual double          ConvertFromBase( double fVal, sal_Int16 nMatchLevel ) const override;
};


class ConvertDataList
{
private:
    std::vector<ConvertData*> maVector;
public:
                            ConvertDataList();
    virtual                 ~ConvertDataList();

    double                  Convert( double fVal, const OUString& rFrom, const OUString& rTo ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
};


inline bool IsLeapYear( sal_uInt16 n )
{
    return ( (( ( n % 4 ) == 0 ) && ( ( n % 100 ) != 0)) || ( ( n % 400 ) == 0 ) );
}


inline sal_Int32 GetDiffDate360( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nDate1, sal_Int32 nDate2, bool bUSAMethod )
{
    return GetDiffDate360( GetNullDate( xOpt ), nDate1, nDate2, bUSAMethod );
}


inline sal_Int16 GetDayOfWeek( sal_Int32 n )
{   // monday = 0, ..., sunday = 6
    return static_cast< sal_Int16 >( ( n - 1 ) % 7 );
}


inline double GetYearFrac( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException )
{
    return GetYearFrac( GetNullDate( xOpt ), nStartDate, nEndDate, nMode );
}


inline sal_uInt16 FuncData::GetUINameID() const
{
    return nUINameID;
}


inline sal_uInt16 FuncData::GetDescrID() const
{
    return nDescrID;
}


inline bool FuncData::IsDouble() const
{
    return bDouble;
}


inline const OUString& FuncData::GetSuffix() const
{
    return aSuffix;
}


inline bool FuncData::Is( const OUString& r ) const
{
    return aIntName == r;
}


inline const std::vector<OUString> & FuncData::GetCompNameList() const
{
    return aCompList;
}


inline FDCategory FuncData::GetCategory() const
{
    return eCat;
}


inline Complex::Complex( double fReal, double fImag, sal_Unicode cC ) :
        r( fReal ), i( fImag ), c( cC )
{
}


inline double Complex::Real() const
{
    return r;
}


inline double Complex::Imag() const
{
    return i;
}


inline double Complex::Abs() const
{
    return sqrt( r * r + i * i );
}


void Complex::Conjugate()
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

    if( !c ) c = rM.c;
}


inline void Complex::Sub( const Complex& rC )
{
    r -= rC.r;
    i -= rC.i;
    if( !c ) c = rC.c;
}


inline void Complex::Add( const Complex& rAdd )
{
    r += rAdd.r;
    i += rAdd.i;
    if( !c ) c = rAdd.c;
}


inline const Complex* ComplexList::Get( sal_uInt32 n ) const
{
    return maVector[n];
}


inline void ComplexList::Append( Complex* p )
{
    maVector.push_back(p);
}


inline ConvertDataClass ConvertData::Class() const
{
    return eClass;
}

inline bool ConvertData::IsPrefixSupport() const
{
    return bPrefixSupport;
}

inline ConvertDataLinear::ConvertDataLinear( const sal_Char* p, double fC, double fO, ConvertDataClass e,
        bool bPrefSupport ) :
    ConvertData( p, fC, e, bPrefSupport ),
    fOffs( fO )
{
}




/// Helper class for date calculation for various financial functions
class ScaDate
{
private:
    sal_uInt16                  nOrigDay;           /// is the day of the original date.
    sal_uInt16                  nDay;               /// is the calculated day depending on the current month/year.
    sal_uInt16                  nMonth;             /// is the current month (one-based).
    sal_uInt16                  nYear;              /// is the current year.
    bool                        bLastDayMode : 1;   /// if true, recalculate nDay after every calculation.
    bool                        bLastDay : 1;       /// is true, if original date was the last day in month.
    bool                        b30Days : 1;        /// is true, if every month has 30 days in calculations.
    bool                        bUSMode : 1;        /// is true, if the US method of 30-day-calculations is used.

                                /// Calculates nDay from nOrigDay and current date.
    void                        setDay();

                                /// @return  count of days in current month
    inline sal_uInt16           getDaysInMonth() const;
                                /// @return  count of days in given month
    inline sal_uInt16           getDaysInMonth( sal_uInt16 _nMon ) const;

                                /// @ return  count of days in the given month range
    sal_Int32                   getDaysInMonthRange( sal_uInt16 nFrom, sal_uInt16 nTo ) const;
                                /// @ return  count of days in the given year range
    sal_Int32                   getDaysInYearRange( sal_uInt16 nFrom, sal_uInt16 nTo ) const;

                                /// Adds/subtracts the given count of years, does not adjust day.
    void                        doAddYears( sal_Int32 nYearCount ) throw( css::lang::IllegalArgumentException );

public:
                                ScaDate();
                                /** @param nBase
                                        date handling mode (days in month / days in year):
                                        0 = 30 days / 360 days (US NASD)
                                        1 = exact / exact
                                        2 = exact / 360
                                        3 = exact / 365
                                        4 = 30 days / 360 days (Europe)
                                        5 = exact / exact (no last day adjustment) */
                                ScaDate( sal_Int32 nNullDate, sal_Int32 nDate, sal_Int32 nBase );
                                ScaDate( const ScaDate& rCopy );
    ScaDate&                    operator=( const ScaDate& rCopy );

                                /// @return  the current month.
    inline sal_uInt16           getMonth() const    { return nMonth; };
                                /// @return  the current year.
    inline sal_uInt16           getYear() const     { return nYear; };

                                /// adds/subtracts the given count of months, adjusts day
    void                        addMonths( sal_Int32 nMonthCount ) throw( css::lang::IllegalArgumentException );

                                /// sets the given year, adjusts day
    inline void                 setYear( sal_uInt16 nNewYear );
                                /// adds/subtracts the given count of years, adjusts day
    inline void                 addYears( sal_Int32 nYearCount ) throw( css::lang::IllegalArgumentException );

                                /// @return  the internal number of the current date
    sal_Int32                   getDate( sal_Int32 nNullDate ) const;
                                /// @return  the number of days between the two dates
    static sal_Int32            getDiff( const ScaDate& rFrom, const ScaDate& rTo ) throw( css::lang::IllegalArgumentException );

    bool                        operator<( const ScaDate& rCmp ) const;
    inline bool                 operator<=( const ScaDate& rCmp ) const { return !(rCmp < *this); }
    inline bool                 operator>( const ScaDate& rCmp ) const  { return rCmp < *this; }
    inline bool                 operator>=( const ScaDate& rCmp ) const { return !(*this < rCmp); }
};

inline sal_uInt16 ScaDate::getDaysInMonth() const
{
    return getDaysInMonth( nMonth );
}

inline sal_uInt16 ScaDate::getDaysInMonth( sal_uInt16 _nMon ) const
{
    return b30Days ? 30 : DaysInMonth( _nMon, nYear );
}

inline void ScaDate::setYear( sal_uInt16 nNewYear )
{
    nYear = nNewYear;
    setDay();
}

inline void ScaDate::addYears( sal_Int32 nYearCount ) throw( css::lang::IllegalArgumentException )
{
    doAddYears( nYearCount );
    setDay();
}


/// Helper class for Any->double conversion, using current language settings
class ScaAnyConverter
{
private:
    css::uno::Reference< css::util::XNumberFormatter2 > xFormatter;
    sal_Int32                   nDefaultFormat;
    bool                    bHasValidFormat;

                                /** Converts a string to double using the number formatter. If the formatter is not
                                    valid, ::rtl::math::stringToDouble() with english separators will be used.
                                    @throws css::lang::IllegalArgumentException
                                        on strings not representing any double value.
                                    @return  the converted double value. */
    double                      convertToDouble(
                                    const OUString& rString ) const
                                throw( css::lang::IllegalArgumentException );

public:
                                ScaAnyConverter(
                                    const css::uno::Reference< css::uno::XComponentContext >& xContext );
                                ~ScaAnyConverter();

                                /// Initializing with current language settings
    void                        init(
                                    const css::uno::Reference< css::beans::XPropertySet >& xPropSet )
                                throw( css::uno::RuntimeException );

                                /** Converts an Any to double (without initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws css::lang::IllegalArgumentException
                                        on other Any types or on invalid strings.
                                    @return  true if the Any contains a double or a non-empty valid string,
                                             false if the Any is empty or the string is empty */
    bool                        getDouble(
                                    double& rfResult,
                                    const css::uno::Any& rAny ) const
                                throw( css::lang::IllegalArgumentException );

                                /** Converts an Any to double (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws css::lang::IllegalArgumentException
                                        on other Any types or on invalid strings.
                                    @return  true if the Any contains a double or a non-empty valid string,
                                             false if the Any is empty or the string is empty */
    bool                        getDouble(
                                    double& rfResult,
                                    const css::uno::Reference< css::beans::XPropertySet >& xPropSet,
                                    const css::uno::Any& rAny )
                                throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

                                /** Converts an Any to double (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws css::lang::IllegalArgumentException
                                        on other Any types or on invalid strings.
                                    @return  the value of the double or string or fDefault if the Any or string is empty */
    double                      getDouble(
                                    const css::uno::Reference< css::beans::XPropertySet >& xPropSet,
                                    const css::uno::Any& rAny,
                                    double fDefault )
                                throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

                                /** Converts an Any to sal_Int32 (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws css::lang::IllegalArgumentException
                                        on other Any types or on invalid values or strings.
                                    @return  true if the Any contains a double or a non-empty valid string,
                                             false if the Any is empty or the string is empty */
    bool                        getInt32(
                                    sal_Int32& rnResult,
                                    const css::uno::Reference< css::beans::XPropertySet >& xPropSet,
                                    const css::uno::Any& rAny )
                                throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

                                /** Converts an Any to sal_Int32 (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws css::lang::IllegalArgumentException
                                        on other Any types or on invalid values or strings.
                                    @return  the truncated value of the double or string or nDefault if the Any or string is empty */
    sal_Int32                   getInt32(
                                    const css::uno::Reference< css::beans::XPropertySet >& xPropSet,
                                    const css::uno::Any& rAny,
                                    sal_Int32 nDefault )
                                throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
