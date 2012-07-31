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
#ifndef ANALYSISHELPER_HXX
#define ANALYSISHELPER_HXX


#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>

#include <math.h>

#include <tools/resid.hxx>
#include <tools/rc.hxx>

#include "analysisdefs.hxx"


class ResMgr;
class SortedIndividualInt32List;
class ScaAnyConverter;


#define PI          3.1415926535897932
#define EOL         ( ( const sal_Char* ) 1 )
#define EOE         ( ( const sal_Char* ) 2 )


inline sal_Bool     IsLeapYear( sal_uInt16 nYear );

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
void                DaysToDate( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear ) throw( ::com::sun::star::lang::IllegalArgumentException );
sal_Int32           GetNullDate( const REF( ::com::sun::star::beans::XPropertySet )& xOptions ) THROWDEF_RTE;
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
inline void         AlignDate( sal_uInt16& rDay, sal_uInt16 nMonth, sal_uInt16 nYear );

double              BinomialCoefficient( double n, double k );
double              GetGcd( double f1, double f2 );
double              ConvertToDec( const STRING& rFromNum, sal_uInt16 nBaseFrom, sal_uInt16 nCharLim ) THROWDEF_RTE_IAE;
STRING              ConvertFromDec(
                        double fNum, double fMin, double fMax, sal_uInt16 nBase,
                        sal_Int32 nPlaces, sal_Int32 nMaxPlaces, sal_Bool bUsePlaces ) THROWDEF_RTE_IAE;
double              Erf( double fX );
double              Erfc( double fX );
sal_Bool            ParseDouble( const sal_Unicode*& rpDoubleAsString, double& rReturn );
STRING              GetString( double fNumber, sal_Bool bLeadingSign = sal_False, sal_uInt16 nMaxNumOfDigits = 15 );
inline double       Exp10( sal_Int16 nPower );      // 10 ^ nPower

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

    using MyList::Append;
    inline void             Append( STRING* pNew );
    inline void             Append( const STRING& rNew );

    using MyList::Count;
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
    ::rtl::OUString         aIntName;
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
    inline sal_Bool         Is( const ::rtl::OUString& rCompareTo ) const;

    inline const StringList&    GetCompNameList( void ) const;

    inline FDCategory       GetCategory( void ) const;
};




class CStrList : private MyList
{
public:
    using MyList::Append;
    inline void             Append( const sal_Char* pNew );
    inline const sal_Char*  Get( sal_uInt32 nIndex ) const;
    using MyList::Count;
};




class FuncDataList : private MyList
{
    ::rtl::OUString         aLastName;
    sal_uInt32              nLast;
public:
                            FuncDataList( ResMgr& );
    virtual                 ~FuncDataList();
    using MyList::Append;
    inline void             Append( FuncData* pNew );
    inline const FuncData*  Get( sal_uInt32 nIndex ) const;
    using MyList::Count;

    const FuncData*         Get( const ::rtl::OUString& aProgrammaticName ) const;
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



//-----------------------------------------------------------------------------

/// sorted list with unique sal_Int32 values
class SortedIndividualInt32List : private MyList
{
protected:
    using MyList::Insert;
    void                        Insert( sal_Int32 nDay );
    void                        Insert( sal_Int32 nDay, sal_Int32 nNullDate, sal_Bool bInsertOnWeekend );
    void                        Insert( double fDay, sal_Int32 nNullDate, sal_Bool bInsertOnWeekend )
                                    throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bInsertOnWeekend  insertion mode: sal_False = holidays on weekend are omitted */
    void                        InsertHolidayList(
                                    const ScaAnyConverter& rAnyConv,
                                    const ::com::sun::star::uno::Any& rHolAny,
                                    sal_Int32 nNullDate,
                                    sal_Bool bInsertOnWeekend ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

public:
                                SortedIndividualInt32List();
    virtual                     ~SortedIndividualInt32List();

                                using MyList::Count;

                                /// @return  element on position nIndex or 0 on invalid index
    inline sal_Int32            Get( sal_uInt32 nIndex ) const
                                    { return (sal_Int32)(sal_IntPtr) MyList::GetObject( nIndex ); }

                                /// @return  sal_True if nVal (internal date representation) is contained
    sal_Bool                    Find( sal_Int32 nVal ) const;

                                /** @param rAnyConv  is an initialized or uninitialized ScaAnyConverter
                                    @param bInsertOnWeekend  insertion mode: sal_False = holidays on weekend are omitted */
    void                        InsertHolidayList(
                                    ScaAnyConverter& rAnyConv,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOptions,
                                    const ::com::sun::star::uno::Any& rHolAny,
                                    sal_Int32 nNullDate,
                                    sal_Bool bInsertOnWeekend ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
};


//-----------------------------------------------------------------------------

class ScaDoubleList : protected MyList
{
protected:
    inline void                 ListAppend( double fValue ) { MyList::Append( new double( fValue ) ); }

    using MyList::Append;
    inline void                 Append( double fValue ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException )
                                    { if( CheckInsert( fValue ) ) ListAppend( fValue ); }

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: sal_False = inserted as 0.0; sal_True = omitted */
    void                        Append(
                                    const ScaAnyConverter& rAnyConv,
                                    const ::com::sun::star::uno::Any& rAny,
                                    sal_Bool bIgnoreEmpty ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: sal_False = inserted as 0.0; sal_True = omitted */
    void                        Append(
                                    const ScaAnyConverter& rAnyConv,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rAnySeq,
                                    sal_Bool bIgnoreEmpty ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                /** @param rAnyConv  must be an initialized ScaAnyConmverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: sal_False = inserted as 0.0; sal_True = omitted */
    void                        Append(
                                    const ScaAnyConverter& rAnyConv,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rAnySeq,
                                    sal_Bool bIgnoreEmpty ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

public:
    virtual                     ~ScaDoubleList();

                                using MyList::Count;
    inline const double*        Get( sal_uInt32 nIndex ) const
                                        { return static_cast< const double* >( MyList::GetObject( nIndex ) ); }

    inline const double*        First() { return static_cast< const double* >( MyList::First() ); }
    inline const double*        Next()  { return static_cast< const double* >( MyList::Next() ); }

    void                        Append( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& rValueArr )
                                    throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
    void                        Append( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& rValueArr )
                                    throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                /** @param rAnyConv  is an initialized or uninitialized ScaAnyConverter
                                    @param bIgnoreEmpty  handling of empty Any's/strings: sal_False = inserted as 0.0; sal_True = omitted */
    void                        Append(
                                    ScaAnyConverter& rAnyConv,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xOpt,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rAnySeq,
                                    sal_Bool bIgnoreEmpty = sal_True ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    virtual sal_Bool            CheckInsert( double fValue ) const
                                    throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
};


//-----------------------------------------------------------------------------

/// stores double values >0.0, throws exception for double values <0.0, does nothing for 0.0
class ScaDoubleListGT0 : public ScaDoubleList
{
public:
    virtual sal_Bool            CheckInsert( double fValue ) const
                                    throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
};


//-----------------------------------------------------------------------------

/// stores double values >=0.0, throws exception for double values <0.0
class ScaDoubleListGE0 : public ScaDoubleList
{
public:
    virtual sal_Bool            CheckInsert( double fValue ) const
                                    throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
};


//-----------------------------------------------------------------------------

class Complex
{
    double                  r;
    double                  i;
    sal_Unicode             c;

public:
    inline                  Complex( double fReal, double fImag = 0.0, sal_Unicode cC = '\0' );
                            Complex( const STRING& rComplexAsString ) THROWDEF_RTE_IAE;

    inline static sal_Bool  IsImagUnit( sal_Unicode c );
    static sal_Bool         ParseString( const STRING& rComplexAsString, Complex& rReturn );
    STRING                  GetString() const THROWDEF_RTE_IAE;

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
    void                    Tan( void ) THROWDEF_RTE_IAE;
    void                    Sec( void ) THROWDEF_RTE_IAE;
    void                    Csc( void ) THROWDEF_RTE_IAE;
    void                    Cot( void ) THROWDEF_RTE_IAE;
    void                    Sinh( void ) THROWDEF_RTE_IAE;
    void                    Cosh( void ) THROWDEF_RTE_IAE;
    void                    Sech( void ) THROWDEF_RTE_IAE;
    void                    Csch( void ) THROWDEF_RTE_IAE;

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

    using MyList::Count;

    using MyList::Append;
    inline void             Append( Complex* pNew );
    void                    Append( const SEQSEQ( STRING )& rComplexNumList, ComplListAppendHandl eAH = AH_EmpyAs0 ) THROWDEF_RTE_IAE;
    void                    Append( const SEQ( ANY )& aMultPars,ComplListAppendHandl eAH = AH_EmpyAs0 ) THROWDEF_RTE_IAE;
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
    STRING                  aName;
    ConvertDataClass        eClass;
    sal_Bool                bPrefixSupport;
public:
                            ConvertData(
                                const sal_Char      pUnitName[],
                                double              fConvertConstant,
                                ConvertDataClass    eClass,
                                sal_Bool            bPrefSupport = sal_False );

    virtual                 ~ConvertData();

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
    inline sal_Bool         IsPrefixSupport( void ) const;
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
                                sal_Bool            bPrefSupport = sal_False );

    virtual                 ~ConvertDataLinear();

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
    return ( (( ( n % 4 ) == 0 ) && ( ( n % 100 ) != 0)) || ( ( n % 400 ) == 0 ) );
}


inline sal_Int32 GetDiffDate360( constREFXPS& xOpt, sal_Int32 nDate1, sal_Int32 nDate2, sal_Bool bUSAMethod )
{
    return GetDiffDate360( GetNullDate( xOpt ), nDate1, nDate2, bUSAMethod );
}


inline sal_Int16 GetDayOfWeek( sal_Int32 n )
{   // monday = 0, ..., sunday = 6
    return static_cast< sal_Int16 >( ( n - 1 ) % 7 );
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


inline sal_Bool FuncData::Is( const ::rtl::OUString& r ) const
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


inline Complex::Complex( double fReal, double fImag, sal_Unicode cC ) :
        r( fReal ), i( fImag ), c( cC )
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



inline sal_Bool ConvertData::IsPrefixSupport( void ) const
{
    return bPrefixSupport;
}

inline ConvertDataLinear::ConvertDataLinear( const sal_Char* p, double fC, double fO, ConvertDataClass e,
        sal_Bool bPrefSupport ) :
    ConvertData( p, fC, e, bPrefSupport ),
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

//-----------------------------------------------------------------------------

/// Helper class for date calculation for various financial functions
class ScaDate
{
private:
    sal_uInt16                  nOrigDay;           /// is the day of the original date.
    sal_uInt16                  nDay;               /// is the calculated day depending on the current month/year.
    sal_uInt16                  nMonth;             /// is the current month (one-based).
    sal_uInt16                  nYear;              /// is the current year.
    sal_Bool                    bLastDayMode : 1;   /// if sal_True, recalculate nDay after every calculation.
    sal_Bool                    bLastDay : 1;       /// is sal_True, if original date was the last day in month.
    sal_Bool                    b30Days : 1;        /// is sal_True, if every month has 30 days in calculations.
    sal_Bool                    bUSMode : 1;        /// is sal_True, if the US method of 30-day-calculations is used.

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
    void                        doAddYears( sal_Int32 nYearCount ) throw( ::com::sun::star::lang::IllegalArgumentException );

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
    void                        addMonths( sal_Int32 nMonthCount ) throw( ::com::sun::star::lang::IllegalArgumentException );

                                /// sets the given year, adjusts day
    inline void                 setYear( sal_uInt16 nNewYear );
                                /// adds/subtracts the given count of years, adjusts day
    inline void                 addYears( sal_Int32 nYearCount ) throw( ::com::sun::star::lang::IllegalArgumentException );

                                /// @return  the internal number of the current date
    sal_Int32                   getDate( sal_Int32 nNullDate ) const;
                                /// @return  the number of days between the two dates
    static sal_Int32            getDiff( const ScaDate& rFrom, const ScaDate& rTo ) throw( ::com::sun::star::lang::IllegalArgumentException );

    sal_Bool                    operator<( const ScaDate& rCmp ) const;
    inline sal_Bool             operator<=( const ScaDate& rCmp ) const { return !(rCmp < *this); }
    inline sal_Bool             operator>( const ScaDate& rCmp ) const  { return rCmp < *this; }
    inline sal_Bool             operator>=( const ScaDate& rCmp ) const { return !(*this < rCmp); }
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

inline void ScaDate::addYears( sal_Int32 nYearCount ) throw( ::com::sun::star::lang::IllegalArgumentException )
{
    doAddYears( nYearCount );
    setDay();
}


//-----------------------------------------------------------------------------

/// Helper class for Any->double conversion, using current language settings
class ScaAnyConverter
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > xFormatter;
    sal_Int32                   nDefaultFormat;
    sal_Bool                    bHasValidFormat;

                                /** Converts a string to double using the number formatter. If the formatter is not
                                    valid, ::rtl::math::stringToDouble() with english separators will be used.
                                    @throws com::sun::star::lang::IllegalArgumentException
                                        on strings not representing any double value.
                                    @return  the converted double value. */
    double                      convertToDouble(
                                    const ::rtl::OUString& rString ) const
                                throw( ::com::sun::star::lang::IllegalArgumentException );

public:
                                ScaAnyConverter(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFact );
                                ~ScaAnyConverter();

                                /// Initializing with current language settings
    void                        init(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet )
                                throw( ::com::sun::star::uno::RuntimeException );

                                /** Converts an Any to double (without initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws com::sun::star::lang::IllegalArgumentException
                                        on other Any types or on invalid strings.
                                    @return  sal_True if the Any contains a double or a non-empty valid string,
                                             sal_False if the Any is empty or the string is empty */
    sal_Bool                    getDouble(
                                    double& rfResult,
                                    const ::com::sun::star::uno::Any& rAny ) const
                                throw( ::com::sun::star::lang::IllegalArgumentException );

                                /** Converts an Any to double (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws com::sun::star::lang::IllegalArgumentException
                                        on other Any types or on invalid strings.
                                    @return  sal_True if the Any contains a double or a non-empty valid string,
                                             sal_False if the Any is empty or the string is empty */
    sal_Bool                    getDouble(
                                    double& rfResult,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet,
                                    const ::com::sun::star::uno::Any& rAny )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                /** Converts an Any to double (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws com::sun::star::lang::IllegalArgumentException
                                        on other Any types or on invalid strings.
                                    @return  the value of the double or string or fDefault if the Any or string is empty */
    double                      getDouble(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet,
                                    const ::com::sun::star::uno::Any& rAny,
                                    double fDefault )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                /** Converts an Any to sal_Int32 (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws com::sun::star::lang::IllegalArgumentException
                                        on other Any types or on invalid values or strings.
                                    @return  sal_True if the Any contains a double or a non-empty valid string,
                                             sal_False if the Any is empty or the string is empty */
    sal_Bool                    getInt32(
                                    sal_Int32& rnResult,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet,
                                    const ::com::sun::star::uno::Any& rAny )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

                                /** Converts an Any to sal_Int32 (with initialization).
                                    The Any can be empty or contain a double or string.
                                    @throws com::sun::star::lang::IllegalArgumentException
                                        on other Any types or on invalid values or strings.
                                    @return  the truncated value of the double or string or nDefault if the Any or string is empty */
    sal_Int32                   getInt32(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet,
                                    const ::com::sun::star::uno::Any& rAny,
                                    sal_Int32 nDefault )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
};


//-----------------------------------------------------------------------------


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
