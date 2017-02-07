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

#include "analysis.hxx"
#include "analysis.hrc"
#include "bessel.hxx"
#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <sal/macros.h>
#include <string.h>
#include <tools/resmgr.hxx>
#include <tools/rcid.h>
#include <algorithm>
#include <cmath>

#define ADDIN_SERVICE               "com.sun.star.sheet.AddIn"
#define MY_SERVICE                  "com.sun.star.sheet.addin.Analysis"
#define MY_IMPLNAME                 "com.sun.star.sheet.addin.AnalysisImpl"

using namespace                 ::com::sun::star;
using namespace sca::analysis;
using namespace std;

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL analysis_component_getFactory(
    const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    void* pRet = nullptr;

    if( pServiceManager && OUString::createFromAscii( pImplName ) == AnalysisAddIn::getImplementationName_Static() )
    {
        uno::Reference< lang::XSingleServiceFactory >  xFactory( cppu::createOneInstanceFactory(
                static_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                AnalysisAddIn::getImplementationName_Static(),
                AnalysisAddIn_CreateInstance,
                AnalysisAddIn::getSupportedServiceNames_Static() ) );

        if( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

ResMgr& AnalysisAddIn::GetResMgr()
{
    if( !pResMgr )
    {
        InitData();     // try to get resource manager

        if( !pResMgr )
            throw uno::RuntimeException();
    }

    return *pResMgr;
}

class AnalysisResourcePublisher : public Resource
{
public:
    explicit        AnalysisResourcePublisher( const AnalysisResId& rId ) : Resource( rId ) {}
    using Resource::IsAvailableRes;
    using Resource::FreeResource;
};

class AnalysisFuncRes : public Resource
{
public:
    AnalysisFuncRes( ResId& rRes, ResMgr& rResMgr, sal_uInt16 nInd, OUString& rRet );
};

AnalysisFuncRes::AnalysisFuncRes( ResId& rRes, ResMgr& rResMgr, sal_uInt16 nInd, OUString& rRet ) : Resource( rRes )
{
    rRet = AnalysisResId(nInd, rResMgr).toString();

    FreeResource();
}

OUString AnalysisAddIn::GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex )
{
    OUString                      aRet;
    AnalysisResourcePublisher   aResPubl( AnalysisResId( RID_ANALYSIS_FUNCTION_DESCRIPTIONS, GetResMgr() ) );
    AnalysisResId               aRes( nResId, GetResMgr() );
    aRes.SetRT( RSC_RESOURCE );
    if( aResPubl.IsAvailableRes( aRes ) )
    {
        AnalysisFuncRes         aSubRes( aRes, GetResMgr(), nStrIndex, aRet );
    }

    aResPubl.FreeResource();

    return aRet;
}

void AnalysisAddIn::InitData()
{
    delete pResMgr;
    pResMgr = ResMgr::CreateResMgr("analysis", LanguageTag(aFuncLoc));

    delete pFD;

    if( pResMgr )
    {
        pFD = new FuncDataList;
        InitFuncDataList( *pFD, *pResMgr );
    }
    else
    {
        pFD = nullptr;
    }

    delete pDefLocales;
    pDefLocales = nullptr;
}

AnalysisAddIn::AnalysisAddIn( const uno::Reference< uno::XComponentContext >& xContext ) :
    pDefLocales( nullptr ),
    pFD( nullptr ),
    pFactDoubles( nullptr ),
    pCDL( nullptr ),
    pResMgr( nullptr ),
    aAnyConv( xContext )
{
}

AnalysisAddIn::~AnalysisAddIn()
{
    delete pResMgr;
    delete pCDL;
    delete[] pFactDoubles;
    delete pFD;
    delete[] pDefLocales;
}

sal_Int32 AnalysisAddIn::getDateMode(
        const uno::Reference< beans::XPropertySet >& xPropSet,
        const uno::Any& rAny )
{
    sal_Int32 nMode = aAnyConv.getInt32( xPropSet, rAny, 0 );
    if( (nMode < 0) || (nMode > 4) )
        throw lang::IllegalArgumentException();
    return nMode;
}

#define MAXFACTDOUBLE   300

double AnalysisAddIn::FactDouble( sal_Int32 nNum )
{
    if( nNum < 0 || nNum > MAXFACTDOUBLE )
        throw lang::IllegalArgumentException();

    if( !pFactDoubles )
    {
        pFactDoubles = new double[ MAXFACTDOUBLE + 1 ];

        pFactDoubles[ 0 ] = 1.0;    // by default

        double      fOdd = 1.0;
        double      fEven = 2.0;

        pFactDoubles[ 1 ] = fOdd;
        pFactDoubles[ 2 ] = fEven;

        bool    bOdd = true;

        for( sal_uInt16 nCnt = 3 ; nCnt <= MAXFACTDOUBLE ; nCnt++ )
        {
            if( bOdd )
            {
                fOdd *= nCnt;
                pFactDoubles[ nCnt ] = fOdd;
            }
            else
            {
                fEven *= nCnt;
                pFactDoubles[ nCnt ] = fEven;
            }

            bOdd = !bOdd;

        }
    }

    return pFactDoubles[ nNum ];
}

OUString AnalysisAddIn::getImplementationName_Static()
{
    return OUString( MY_IMPLNAME );
}

uno::Sequence< OUString > AnalysisAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString >   aRet(2);
    OUString*         pArray = aRet.getArray();
    pArray[0] = ADDIN_SERVICE;
    pArray[1] = MY_SERVICE;
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL AnalysisAddIn_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory >& xServiceFact )
{
    return static_cast<cppu::OWeakObject*>(new AnalysisAddIn( comphelper::getComponentContext(xServiceFact) ));
}

// XServiceName
OUString SAL_CALL AnalysisAddIn::getServiceName()
{
    // name of specific AddIn service
    return OUString( MY_SERVICE );
}

// XServiceInfo
OUString SAL_CALL AnalysisAddIn::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL AnalysisAddIn::supportsService( const OUString& aName )
{
    return cppu::supportsService(this, aName);
}

uno::Sequence< OUString > SAL_CALL AnalysisAddIn::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

// XLocalizable
void SAL_CALL AnalysisAddIn::setLocale( const lang::Locale& eLocale )
{
    aFuncLoc = eLocale;

    InitData();     // change of locale invalidates resources!
}

lang::Locale SAL_CALL AnalysisAddIn::getLocale()
{
    return aFuncLoc;
}

// XAddIn
OUString SAL_CALL AnalysisAddIn::getProgrammaticFuntionName( const OUString& )
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)

    return OUString();
}

OUString SAL_CALL AnalysisAddIn::getDisplayFunctionName( const OUString& aProgrammaticName )
{
    OUString          aRet;

    auto it = std::find_if(pFD->begin(), pFD->end(), FindFuncData( aProgrammaticName ) );
    if( it != pFD->end() )
    {
        aRet = AnalysisRscStrLoader( RID_ANALYSIS_FUNCTION_NAMES, it->GetUINameID(), GetResMgr() ).GetString();
        if( it->IsDouble() )
        {
            const OUString& rSuffix = it->GetSuffix();
            if (!rSuffix.isEmpty())
                aRet += rSuffix;
            else
                aRet += "_ADD";
        }
    }
    else
    {
        aRet = "UNKNOWNFUNC_" + aProgrammaticName;
    }

    return aRet;
}

OUString SAL_CALL AnalysisAddIn::getFunctionDescription( const OUString& aProgrammaticName )
{
    OUString          aRet;

    auto it = std::find_if(pFD->begin(), pFD->end(), FindFuncData( aProgrammaticName ) );
    if( it != pFD->end() )
        aRet = GetFuncDescrStr( it->GetDescrID(), 1 );

    return aRet;
}

OUString SAL_CALL AnalysisAddIn::getDisplayArgumentName( const OUString& aName, sal_Int32 nArg )
{
    OUString          aRet;

    auto it = std::find_if(pFD->begin(), pFD->end(), FindFuncData( aName ) );
    if( it != pFD->end() && nArg <= 0xFFFF )
    {
        sal_uInt16  nStr = it->GetStrIndex( sal_uInt16( nArg ) );
        if( nStr )
            aRet = GetFuncDescrStr( it->GetDescrID(), nStr );
        else
            aRet = "internal";
    }

    return aRet;
}

OUString SAL_CALL AnalysisAddIn::getArgumentDescription( const OUString& aName, sal_Int32 nArg )
{
    OUString          aRet;

    auto it = std::find_if(pFD->begin(), pFD->end(), FindFuncData( aName ) );
    if( it != pFD->end() && nArg <= 0xFFFF )
    {
        sal_uInt16  nStr = it->GetStrIndex( sal_uInt16( nArg ) );
        if( nStr )
            aRet = GetFuncDescrStr( it->GetDescrID(), nStr + 1 );
        else
            aRet = "for internal use only";
    }

    return aRet;
}

static const char pDefCatName[] = "Add-In";

OUString SAL_CALL AnalysisAddIn::getProgrammaticCategoryName( const OUString& aName )
{
    //  return non-translated strings
    //  return OUString( "Add-In" );
    auto it = std::find_if(pFD->begin(), pFD->end(), FindFuncData( aName ) );
    OUString              aRet;
    if( it != pFD->end() )
    {
        switch( it->GetCategory() )
        {
            case FDCategory::DateTime:    aRet = "Date&Time";         break;
            case FDCategory::Finance:     aRet = "Financial";         break;
            case FDCategory::Inf:         aRet = "Information";       break;
            case FDCategory::Math:        aRet = "Mathematical";      break;
            case FDCategory::Tech:        aRet = "Technical";         break;
        }
    }
    else
        aRet = pDefCatName;

    return aRet;
}

OUString SAL_CALL AnalysisAddIn::getDisplayCategoryName( const OUString& aProgrammaticFunctionName )
{
    //  return translated strings, not used for predefined categories
    auto it = std::find_if(pFD->begin(), pFD->end(), FindFuncData( aProgrammaticFunctionName ) );
    OUString              aRet;
    if( it != pFD->end() )
    {
        switch( it->GetCategory() )
        {
            case FDCategory::DateTime:    aRet = "Date&Time";         break;
            case FDCategory::Finance:     aRet = "Financial";         break;
            case FDCategory::Inf:         aRet = "Information";       break;
            case FDCategory::Math:        aRet = "Mathematical";      break;
            case FDCategory::Tech:        aRet = "Technical";         break;
        }
    }
    else
        aRet = pDefCatName;

    return aRet;
}

static const sal_Char*      pLang[] = { "de", "en" };
static const sal_Char*      pCoun[] = { "DE", "US" };
static const sal_uInt32     nNumOfLoc = SAL_N_ELEMENTS(pLang);

void AnalysisAddIn::InitDefLocales()
{
    pDefLocales = new lang::Locale[ nNumOfLoc ];

    for( sal_uInt32 n = 0 ; n < nNumOfLoc ; n++ )
    {
        pDefLocales[ n ].Language = OUString::createFromAscii( pLang[ n ] );
        pDefLocales[ n ].Country = OUString::createFromAscii( pCoun[ n ] );
    }
}

inline const lang::Locale& AnalysisAddIn::GetLocale( sal_uInt32 nInd )
{
    if( !pDefLocales )
        InitDefLocales();

    if( nInd < sizeof( pLang ) )
        return pDefLocales[ nInd ];
    else
        return aFuncLoc;
}

uno::Sequence< sheet::LocalizedName > SAL_CALL AnalysisAddIn::getCompatibilityNames( const OUString& aProgrammaticName )
{
    auto it = std::find_if(pFD->begin(), pFD->end(), FindFuncData( aProgrammaticName ) );
    if( it == pFD->end() )
        return uno::Sequence< sheet::LocalizedName >( 0 );

    const std::vector<OUString>& r = it->GetCompNameList();
    sal_uInt32                   nCount = r.size();

    uno::Sequence< sheet::LocalizedName >                aRet( nCount );

    sheet::LocalizedName*  pArray = aRet.getArray();

    for( sal_uInt32 n = 0 ; n < nCount ; n++ )
    {
        pArray[ n ] = sheet::LocalizedName( GetLocale( n ), r[n] );
    }

    return aRet;
}

// XAnalysis
/** Workday */
sal_Int32 SAL_CALL AnalysisAddIn::getWorkday( const uno::Reference< beans::XPropertySet >& xOptions,
    sal_Int32 nDate, sal_Int32 nDays, const uno::Any& aHDay )
{
    if( !nDays )
        return nDate;

    sal_Int32                   nNullDate = GetNullDate( xOptions );

    SortedIndividualInt32List   aSrtLst;

    aSrtLst.InsertHolidayList( aAnyConv, xOptions, aHDay, nNullDate );

    sal_Int32                   nActDate = nDate + nNullDate;

    if( nDays > 0 )
    {
        if( GetDayOfWeek( nActDate ) == 5 )
            // when starting on Saturday, assuming we're starting on Sunday to get the jump over the weekend
            nActDate++;

        while( nDays )
        {
            nActDate++;

            if( GetDayOfWeek( nActDate ) < 5 )
            {
                if( !aSrtLst.Find( nActDate ) )
                    nDays--;
            }
            else
                nActDate++;     // jump over weekend
        }
    }
    else
    {
        if( GetDayOfWeek( nActDate ) == 6 )
            // when starting on Sunday, assuming we're starting on Saturday to get the jump over the weekend
            nActDate--;

        while( nDays )
        {
            nActDate--;

            if( GetDayOfWeek( nActDate ) < 5 )
            {
                if( !aSrtLst.Find( nActDate ) )
                    nDays++;
            }
            else
                nActDate--;     // jump over weekend
        }
    }

    return nActDate - nNullDate;
}

/** Yearfrac */
double SAL_CALL AnalysisAddIn::getYearfrac( const uno::Reference< beans::XPropertySet >& xOpt,
    sal_Int32 nStartDate, sal_Int32 nEndDate, const uno::Any& rMode )
{
    double fRet = GetYearFrac( xOpt, nStartDate, nEndDate, getDateMode( xOpt, rMode ) );
    RETURN_FINITE( fRet );
}

sal_Int32 SAL_CALL AnalysisAddIn::getEdate( const uno::Reference< beans::XPropertySet >& xOpt, sal_Int32 nStartDate, sal_Int32 nMonths )
{
    sal_Int32 nNullDate = GetNullDate( xOpt );
    ScaDate aDate( nNullDate, nStartDate, 5 );
    aDate.addMonths( nMonths );
    return aDate.getDate( nNullDate );
}

sal_Int32 SAL_CALL AnalysisAddIn::getWeeknum( const uno::Reference< beans::XPropertySet >& xOpt, sal_Int32 nDate, sal_Int32 nMode )
{
    nDate += GetNullDate( xOpt );

    sal_uInt16  nDay, nMonth, nYear;
    DaysToDate( nDate, nDay, nMonth, nYear );

    sal_Int32   nFirstInYear = DateToDays( 1, 1, nYear );
    sal_uInt16  nFirstDayInYear = GetDayOfWeek( nFirstInYear );

    return ( nDate - nFirstInYear + ( ( nMode == 1 )? ( nFirstDayInYear + 1 ) % 7 : nFirstDayInYear ) ) / 7 + 1;
}

sal_Int32 SAL_CALL AnalysisAddIn::getEomonth( const uno::Reference< beans::XPropertySet >& xOpt, sal_Int32 nDate, sal_Int32 nMonths )
{
    sal_Int32   nNullDate = GetNullDate( xOpt );
    nDate += nNullDate;
    sal_uInt16  nDay, nMonth, nYear;
    DaysToDate( nDate, nDay, nMonth, nYear );

    sal_Int32   nNewMonth = nMonth + nMonths;

    if( nNewMonth > 12 )
    {
        nYear = sal::static_int_cast<sal_uInt16>( nYear + ( nNewMonth / 12 ) );
        nNewMonth %= 12;
    }
    else if( nNewMonth < 1 )
    {
        nNewMonth = -nNewMonth;
        nYear = sal::static_int_cast<sal_uInt16>( nYear - ( nNewMonth / 12 ) );
        nYear--;
        nNewMonth %= 12;
        nNewMonth = 12 - nNewMonth;
    }

    return DateToDays( DaysInMonth( sal_uInt16( nNewMonth ), nYear ), sal_uInt16( nNewMonth ), nYear ) - nNullDate;
}

sal_Int32 SAL_CALL AnalysisAddIn::getNetworkdays( const uno::Reference< beans::XPropertySet >& xOpt,
        sal_Int32 nStartDate, sal_Int32 nEndDate, const uno::Any& aHDay )
{
    sal_Int32                   nNullDate = GetNullDate( xOpt );

    SortedIndividualInt32List   aSrtLst;

    aSrtLst.InsertHolidayList( aAnyConv, xOpt, aHDay, nNullDate );

    sal_Int32                   nActDate = nStartDate + nNullDate;
    sal_Int32                   nStopDate = nEndDate + nNullDate;
    sal_Int32                   nCnt = 0;

    if( nActDate <= nStopDate )
    {
        while( nActDate <= nStopDate )
        {
            if( GetDayOfWeek( nActDate ) < 5 && !aSrtLst.Find( nActDate ) )
                nCnt++;

            nActDate++;
        }
    }
    else
    {
        while( nActDate >= nStopDate )
        {
            if( GetDayOfWeek( nActDate ) < 5 && !aSrtLst.Find( nActDate ) )
                nCnt--;

            nActDate--;
        }
    }

    return nCnt;
}

sal_Int32 SAL_CALL AnalysisAddIn::getIseven( sal_Int32 nVal )
{
    return ( nVal & 0x00000001 )? 0 : 1;
}

sal_Int32 SAL_CALL AnalysisAddIn::getIsodd( sal_Int32 nVal )
{
    return ( nVal & 0x00000001 )? 1 : 0;
}

double SAL_CALL
AnalysisAddIn::getMultinomial( const uno::Reference< beans::XPropertySet >& xOpt, const uno::Sequence< uno::Sequence< sal_Int32 > >& aVLst,
                               const uno::Sequence< uno::Any >& aOptVLst )
{
    ScaDoubleListGE0 aValList;

    aValList.Append( aVLst );
    aValList.Append( aAnyConv, xOpt, aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    double nZ = 0;
    double fRet = 1.0;

    for( sal_uInt32 i = 0; i < aValList.Count(); ++i )
    {
        const double d = aValList.Get(i);
        double n = (d >= 0.0) ? rtl::math::approxFloor( d ) : rtl::math::approxCeil( d );
        if ( n < 0.0 )
            throw lang::IllegalArgumentException();

        if( n > 0.0 )
        {
            nZ += n;
            fRet *= BinomialCoefficient(nZ, n);
        }
    }
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getSeriessum( double fX, double fN, double fM, const uno::Sequence< uno::Sequence< double > >& aCoeffList )
{
    double                          fRet = 0.0;

    // #i32269# 0^0 is undefined, Excel returns #NUM! error
    if( fX == 0.0 && fN == 0 )
        throw uno::RuntimeException();

    if( fX != 0.0 )
    {
        sal_Int32       n1, n2;
        sal_Int32       nE1 = aCoeffList.getLength();
        sal_Int32       nE2;

        for( n1 = 0 ; n1 < nE1 ; n1++ )
        {
            const uno::Sequence< double >&    rList = aCoeffList[ n1 ];
            nE2 = rList.getLength();
            const double*           pList = rList.getConstArray();

            for( n2 = 0 ; n2 < nE2 ; n2++ )
            {
                fRet += pList[ n2 ] * pow( fX, fN );

                fN += fM;
            }
        }
    }

    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getQuotient( double fNum, double fDenom )
{
    double fRet;
    if( (fNum < 0) != (fDenom < 0) )
        fRet = ::rtl::math::approxCeil( fNum / fDenom );
    else
        fRet = ::rtl::math::approxFloor( fNum / fDenom );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getMround( double fNum, double fMult )
{
    if( fMult == 0.0 )
        return fMult;

    double fRet = fMult * ::rtl::math::round( fNum / fMult );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getSqrtpi( double fNum )
{
    double fRet = sqrt( fNum * PI );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getRandbetween( double fMin, double fMax )
{
    fMin = ::rtl::math::round( fMin, 0, rtl_math_RoundingMode_Up );
    fMax = ::rtl::math::round( fMax, 0, rtl_math_RoundingMode_Up );
    if( fMin > fMax )
        throw lang::IllegalArgumentException();

    double fRet = floor(comphelper::rng::uniform_real_distribution(fMin, nextafter(fMax+1, -DBL_MAX)));
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getGcd( const uno::Reference< beans::XPropertySet >& xOpt, const uno::Sequence< uno::Sequence< double > >& aVLst, const uno::Sequence< uno::Any >& aOptVLst )
{
    ScaDoubleListGT0 aValList;

    aValList.Append( aVLst );
    aValList.Append( aAnyConv, xOpt, aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    double          f = aValList.Get(0);
    for( sal_uInt32 i = 1; i < aValList.Count(); ++i )
    {
        f = GetGcd( aValList.Get(i), f );
    }

    RETURN_FINITE( f );
}

double SAL_CALL AnalysisAddIn::getLcm( const uno::Reference< beans::XPropertySet >& xOpt, const uno::Sequence< uno::Sequence< double > >& aVLst, const uno::Sequence< uno::Any >& aOptVLst )
{
    ScaDoubleListGE0 aValList;

    aValList.Append( aVLst );
    aValList.Append( aAnyConv, xOpt, aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    double f = rtl::math::approxFloor( aValList.Get(0) );
    if( f < 0.0 )
        throw lang::IllegalArgumentException();

    if( f == 0.0 )
        return f;

    for( sal_uInt32 i = 1; i < aValList.Count(); ++i )
    {
        double fTmp = rtl::math::approxFloor( aValList.Get(i) );
        if( fTmp < 0.0 )
            throw lang::IllegalArgumentException();

        f = fTmp * f / GetGcd( fTmp, f );
        if( f == 0.0 )
            return f;
    }

    RETURN_FINITE( f );
}

double SAL_CALL AnalysisAddIn::getBesseli( double fNum, sal_Int32 nOrder )
{
    double fRet = sca::analysis::BesselI( fNum, nOrder );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getBesselj( double fNum, sal_Int32 nOrder )
{
    double fRet = sca::analysis::BesselJ( fNum, nOrder );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getBesselk( double fNum, sal_Int32 nOrder )
{
    if( nOrder < 0 || fNum <= 0.0 )
        throw lang::IllegalArgumentException();

    double fRet = sca::analysis::BesselK( fNum, nOrder );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getBessely( double fNum, sal_Int32 nOrder )
{
    if( nOrder < 0 || fNum <= 0.0 )
        throw lang::IllegalArgumentException();

    double fRet = sca::analysis::BesselY( fNum, nOrder );
    RETURN_FINITE( fRet );
}

const double    SCA_MAX2        = 511.0;            // min. val for binary numbers (9 bits + sign)
const double    SCA_MIN2        = -SCA_MAX2-1.0;    // min. val for binary numbers (9 bits + sign)
const double    SCA_MAX8        = 536870911.0;      // max. val for octal numbers (29 bits + sign)
const double    SCA_MIN8        = -SCA_MAX8-1.0;    // min. val for octal numbers (29 bits + sign)
const double    SCA_MAX16       = 549755813888.0;   // max. val for hexadecimal numbers (39 bits + sign)
const double    SCA_MIN16       = -SCA_MAX16-1.0;   // min. val for hexadecimal numbers (39 bits + sign)
const sal_Int32 SCA_MAXPLACES   = 10;               // max. number of places

OUString SAL_CALL AnalysisAddIn::getBin2Oct( const uno::Reference< beans::XPropertySet >& xOpt, const OUString& aNum, const uno::Any& rPlaces )
{
    double fVal = ConvertToDec( aNum, 2, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN8, SCA_MAX8, 8, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

double SAL_CALL AnalysisAddIn::getBin2Dec( const OUString& aNum )
{
    double fRet = ConvertToDec( aNum, 2, SCA_MAXPLACES );
    RETURN_FINITE( fRet );
}

OUString SAL_CALL AnalysisAddIn::getBin2Hex( const uno::Reference< beans::XPropertySet >& xOpt, const OUString& aNum, const uno::Any& rPlaces )
{
    double fVal = ConvertToDec( aNum, 2, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN16, SCA_MAX16, 16, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

OUString SAL_CALL AnalysisAddIn::getOct2Bin( const uno::Reference< beans::XPropertySet >& xOpt, const OUString& aNum, const uno::Any& rPlaces )
{
    double fVal = ConvertToDec( aNum, 8, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN2, SCA_MAX2, 2, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

double SAL_CALL AnalysisAddIn::getOct2Dec( const OUString& aNum )
{
    double fRet = ConvertToDec( aNum, 8, SCA_MAXPLACES );
    RETURN_FINITE( fRet );
}

OUString SAL_CALL AnalysisAddIn::getOct2Hex( const uno::Reference< beans::XPropertySet >& xOpt, const OUString& aNum, const uno::Any& rPlaces )
{
    double fVal = ConvertToDec( aNum, 8, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN16, SCA_MAX16, 16, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

OUString SAL_CALL AnalysisAddIn::getDec2Bin( const uno::Reference< beans::XPropertySet >& xOpt, sal_Int32 nNum, const uno::Any& rPlaces )
{
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( nNum, SCA_MIN2, SCA_MAX2, 2, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

OUString SAL_CALL AnalysisAddIn::getDec2Oct( const uno::Reference< beans::XPropertySet >& xOpt, sal_Int32 nNum, const uno::Any& rPlaces )
{
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( nNum, SCA_MIN8, SCA_MAX8, 8, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

OUString SAL_CALL AnalysisAddIn::getDec2Hex( const uno::Reference< beans::XPropertySet >& xOpt, double fNum, const uno::Any& rPlaces )
{
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fNum, SCA_MIN16, SCA_MAX16, 16, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

OUString SAL_CALL AnalysisAddIn::getHex2Bin( const uno::Reference< beans::XPropertySet >& xOpt, const OUString& aNum, const uno::Any& rPlaces )
{
    double fVal = ConvertToDec( aNum, 16, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN2, SCA_MAX2, 2, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

double SAL_CALL AnalysisAddIn::getHex2Dec( const OUString& aNum )
{
    double fRet = ConvertToDec( aNum, 16, SCA_MAXPLACES );
    RETURN_FINITE( fRet );
}

OUString SAL_CALL AnalysisAddIn::getHex2Oct( const uno::Reference< beans::XPropertySet >& xOpt, const OUString& aNum, const uno::Any& rPlaces )
{
    double fVal = ConvertToDec( aNum, 16, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN8, SCA_MAX8, 8, nPlaces, SCA_MAXPLACES, bUsePlaces );
}

sal_Int32 SAL_CALL AnalysisAddIn::getDelta( const uno::Reference< beans::XPropertySet >& xOpt, double fNum1, const uno::Any& rNum2 )
{
    return sal_Int32(fNum1 == aAnyConv.getDouble( xOpt, rNum2, 0.0 ));
}

double SAL_CALL AnalysisAddIn::getErf( const uno::Reference< beans::XPropertySet >& xOpt, double fLL, const uno::Any& rUL )
{
    double fUL, fRet;
    bool bContainsValue = aAnyConv.getDouble( fUL, xOpt, rUL );

    fRet = bContainsValue ? (Erf( fUL ) - Erf( fLL )) : Erf( fLL );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getErfc( double f )
{
    double fRet = Erfc( f );
    RETURN_FINITE( fRet );
}

sal_Int32 SAL_CALL AnalysisAddIn::getGestep( const uno::Reference< beans::XPropertySet >& xOpt, double fNum, const uno::Any& rStep )
{
    return sal_Int32(fNum >= aAnyConv.getDouble( xOpt, rStep, 0.0 ));
}

double SAL_CALL AnalysisAddIn::getFactdouble( sal_Int32 nNum )
{
    double fRet = FactDouble( nNum );
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getImabs( const OUString& aNum )
{
    double fRet = Complex( aNum ).Abs();
    RETURN_FINITE( fRet );
}

double SAL_CALL AnalysisAddIn::getImaginary( const OUString& aNum )
{
    double fRet = Complex( aNum ).Imag();
    RETURN_FINITE( fRet );
}

OUString SAL_CALL AnalysisAddIn::getImpower( const OUString& aNum, double f )
{
    Complex     z( aNum );

    z.Power( f );

    return z.GetString();
}

double SAL_CALL AnalysisAddIn::getImargument( const OUString& aNum )
{
    double fRet = Complex( aNum ).Arg();
    RETURN_FINITE( fRet );
}

OUString SAL_CALL AnalysisAddIn::getImcos( const OUString& aNum )
{
    Complex     z( aNum );

    z.Cos();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImdiv( const OUString& aDivid, const OUString& aDivis )
{
    Complex     z( aDivid );

    z.Div( Complex( aDivis ) );

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImexp( const OUString& aNum )
{
    Complex     z( aNum );

    z.Exp();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImconjugate( const OUString& aNum )
{
    Complex     z( aNum );

    z.Conjugate();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImln( const OUString& aNum )
{
    Complex     z( aNum );

    z.Ln();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImlog10( const OUString& aNum )
{
    Complex     z( aNum );

    z.Log10();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImlog2( const OUString& aNum )
{
    Complex     z( aNum );

    z.Log2();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImproduct( const uno::Reference< beans::XPropertySet >&, const uno::Sequence< uno::Sequence< OUString > >& aNum1, const uno::Sequence< uno::Any >& aNL )
{
    ComplexList     z_list;

    z_list.Append( aNum1, AH_IgnoreEmpty );
    z_list.Append( aNL, AH_IgnoreEmpty );

    if( z_list.empty() )
        return Complex( 0 ).GetString();

    Complex         z( *(z_list.Get(0)) );
    for( sal_uInt32 i = 1; i < z_list.Count(); ++i )
        z.Mult( *(z_list.Get(i)) );

    return z.GetString();
}

double SAL_CALL AnalysisAddIn::getImreal( const OUString& aNum )
{
    double fRet = Complex( aNum ).Real();
    RETURN_FINITE( fRet );
}

OUString SAL_CALL AnalysisAddIn::getImsin( const OUString& aNum )
{
    Complex     z( aNum );

    z.Sin();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImsub( const OUString& aNum1, const OUString& aNum2 )
{
    Complex     z( aNum1 );

    z.Sub( Complex( aNum2 ) );

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImsum( const uno::Reference< beans::XPropertySet >&, const uno::Sequence< uno::Sequence< OUString > >& aNum1, const uno::Sequence< uno::Any >& aFollowingPars )
{
    ComplexList     z_list;

    z_list.Append( aNum1, AH_IgnoreEmpty );
    z_list.Append( aFollowingPars, AH_IgnoreEmpty );

    if( z_list.empty() )
        return Complex( 0 ).GetString();

    Complex         z( *(z_list.Get(0)) );
    for( sal_uInt32 i = 1; i < z_list.Count(); ++i )
        z.Add( *(z_list.Get(i)) );

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImsqrt( const OUString& aNum )
{
    Complex     z( aNum );

    z.Sqrt();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImtan( const OUString& aNum )
{
    Complex     z( aNum );

    z.Tan();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImsec( const OUString& aNum )
{
    Complex     z( aNum );

    z.Sec();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImcsc( const OUString& aNum )
{
    Complex     z( aNum );

    z.Csc();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImcot( const OUString& aNum )
{
    Complex     z( aNum );

    z.Cot();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImsinh( const OUString& aNum )
{
    Complex     z( aNum );

    z.Sinh();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImcosh( const OUString& aNum )
{
    Complex     z( aNum );

    z.Cosh();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImsech( const OUString& aNum )
{
    Complex     z( aNum );

    z.Sech();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getImcsch( const OUString& aNum )
{
    Complex     z( aNum );

    z.Csch();

    return z.GetString();
}

OUString SAL_CALL AnalysisAddIn::getComplex( double fR, double fI, const uno::Any& rSuff )
{
    bool    bi;

    switch( rSuff.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:
            bi = true;
            break;
        case uno::TypeClass_STRING:
            {
            auto   pSuff = o3tl::forceAccess<OUString>(rSuff);
            bi = *pSuff == "i" || pSuff->isEmpty();
            if( !bi && *pSuff != "j" )
                throw lang::IllegalArgumentException();
            }
            break;
        default:
            throw lang::IllegalArgumentException();
    }

    return Complex( fR, fI, bi ? 'i' : 'j' ).GetString();
}

double SAL_CALL AnalysisAddIn::getConvert( double f, const OUString& aFU, const OUString& aTU )
{
    if( !pCDL )
        pCDL = new ConvertDataList();

    double fRet = pCDL->Convert( f, aFU, aTU );
    RETURN_FINITE( fRet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
