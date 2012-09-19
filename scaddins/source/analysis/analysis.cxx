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

#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <sal/macros.h>
#include <string.h>

#include <tools/resmgr.hxx>
#include <tools/rcid.h>
#include "analysis.hrc"
#include "bessel.hxx"

#define ADDIN_SERVICE               "com.sun.star.sheet.AddIn"
#define MY_SERVICE                  "com.sun.star.sheet.addin.Analysis"
#define MY_IMPLNAME                 "com.sun.star.sheet.addin.AnalysisImpl"

using namespace                 ::rtl;
using namespace                 ::com::sun::star;


extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL analysis_component_getFactory(
    const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    void* pRet = 0;

    if( pServiceManager && STRING::createFromAscii( pImplName ) == AnalysisAddIn::getImplementationName_Static() )
    {
        REF( lang::XSingleServiceFactory )  xFactory( cppu::createOneInstanceFactory(
                reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
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


//------------------------------------------------------------------------
//
//  "normal" service implementation
//
//------------------------------------------------------------------------


ResMgr& AnalysisAddIn::GetResMgr( void ) THROWDEF_RTE
{
    if( !pResMgr )
    {
        InitData();     // try to get resource manager

        if( !pResMgr )
            THROW_RTE;
    }

    return *pResMgr;
}


STRING AnalysisAddIn::GetDisplFuncStr( sal_uInt16 nFuncNum ) THROWDEF_RTE
{
    return String( AnalysisRscStrLoader( RID_ANALYSIS_FUNCTION_NAMES, nFuncNum, GetResMgr() ).GetString() );
}


class AnalysisResourcePublisher : public Resource
{
public:
                    AnalysisResourcePublisher( const AnalysisResId& rId ) : Resource( rId ) {}
    sal_Bool            IsAvailableRes( const ResId& rId ) const { return Resource::IsAvailableRes( rId ); }
    void            FreeResource() { Resource::FreeResource(); }
};


class AnalysisFuncRes : public Resource
{
public:
    AnalysisFuncRes( ResId& rRes, ResMgr& rResMgr, sal_uInt16 nInd, STRING& rRet );
};


AnalysisFuncRes::AnalysisFuncRes( ResId& rRes, ResMgr& rResMgr, sal_uInt16 nInd, STRING& rRet ) : Resource( rRes )
{
    rRet = String( AnalysisResId( nInd, rResMgr ) );

    FreeResource();
}


STRING AnalysisAddIn::GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex ) THROWDEF_RTE
{
    STRING                      aRet;
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


void AnalysisAddIn::InitData( void )
{
    if( pResMgr )
        delete pResMgr;

    OString             aModName( "analysis" );
    pResMgr = ResMgr::CreateResMgr( aModName.getStr(), aFuncLoc );

    if( pFD )
        delete pFD;

    if( pResMgr )
        pFD = new FuncDataList( *pResMgr );
    else
        pFD = NULL;

    if( pDefLocales )
    {
        delete pDefLocales;
        pDefLocales = NULL;
    }
}


AnalysisAddIn::AnalysisAddIn( const uno::Reference< uno::XComponentContext >& xContext ) :
    pDefLocales( NULL ),
    pFD( NULL ),
    pFactDoubles( NULL ),
    pCDL( NULL ),
    pResMgr( NULL ),
    aAnyConv( xContext )
{
}


AnalysisAddIn::~AnalysisAddIn()
{
    if( pFD )
        delete pFD;

    if( pFactDoubles )
        delete[] pFactDoubles;

    if( pCDL )
        delete pCDL;

    if( pDefLocales )
        delete[] pDefLocales;
}


sal_Int32 AnalysisAddIn::getDateMode(
        const uno::Reference< beans::XPropertySet >& xPropSet,
        const uno::Any& rAny ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nMode = aAnyConv.getInt32( xPropSet, rAny, 0 );
    if( (nMode < 0) || (nMode > 4) )
        throw lang::IllegalArgumentException();
    return nMode;
}



//-----------------------------------------------------------------------------


#define MAXFACTDOUBLE   300

double AnalysisAddIn::FactDouble( sal_Int32 nNum ) THROWDEF_RTE_IAE
{
    if( nNum < 0 || nNum > MAXFACTDOUBLE )
        THROW_IAE;

    if( !pFactDoubles )
    {
        pFactDoubles = new double[ MAXFACTDOUBLE + 1 ];

        pFactDoubles[ 0 ] = 1.0;    // by default

        double      fOdd = 1.0;
        double      fEven = 2.0;

        pFactDoubles[ 1 ] = fOdd;
        pFactDoubles[ 2 ] = fEven;

        sal_Bool    bOdd = sal_True;

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


STRING AnalysisAddIn::getImplementationName_Static()
{
    return STRFROMASCII( MY_IMPLNAME );
}


SEQ( STRING ) AnalysisAddIn::getSupportedServiceNames_Static()
{
    SEQ( STRING )   aRet(2);
    STRING*         pArray = aRet.getArray();
    pArray[0] = STRFROMASCII( ADDIN_SERVICE );
    pArray[1] = STRFROMASCII( MY_SERVICE );
    return aRet;
}


REF( uno::XInterface ) SAL_CALL AnalysisAddIn_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory >& xServiceFact )
{
    static uno::Reference< uno::XInterface > xInst = (cppu::OWeakObject*) new AnalysisAddIn( comphelper::getComponentContext(xServiceFact) );
    return xInst;
}


// XServiceName

STRING SAL_CALL AnalysisAddIn::getServiceName() THROWDEF_RTE
{
    // name of specific AddIn service
    return STRFROMASCII( MY_SERVICE );
}


// XServiceInfo

STRING SAL_CALL AnalysisAddIn::getImplementationName() THROWDEF_RTE
{
    return getImplementationName_Static();
}


sal_Bool SAL_CALL AnalysisAddIn::supportsService( const STRING& aName ) THROWDEF_RTE
{
    return aName.compareToAscii( ADDIN_SERVICE ) == 0 || aName.compareToAscii( MY_SERVICE ) == 0;
}


SEQ( STRING ) SAL_CALL AnalysisAddIn::getSupportedServiceNames() THROWDEF_RTE
{
    return getSupportedServiceNames_Static();
}


// XLocalizable

void SAL_CALL AnalysisAddIn::setLocale( const lang::Locale& eLocale ) THROWDEF_RTE
{
    aFuncLoc = eLocale;

    InitData();     // change of locale invalidates resources!
}

lang::Locale SAL_CALL AnalysisAddIn::getLocale() THROWDEF_RTE
{
    return aFuncLoc;
}


// XAddIn

STRING SAL_CALL AnalysisAddIn::getProgrammaticFuntionName( const STRING& ) THROWDEF_RTE
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)

    return STRING();
}


STRING SAL_CALL AnalysisAddIn::getDisplayFunctionName( const STRING& aProgrammaticName ) THROWDEF_RTE
{
    STRING          aRet;

    const FuncData* p = pFD->Get( aProgrammaticName );
    if( p )
    {
        aRet = GetDisplFuncStr( p->GetUINameID() );
        if( p->IsDouble() )
            aRet += STRFROMANSI( "_ADD" );
    }
    else
    {
        aRet = STRFROMANSI( "UNKNOWNFUNC_" );
        aRet += aProgrammaticName;
    }

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getFunctionDescription( const STRING& aProgrammaticName ) THROWDEF_RTE
{
    STRING          aRet;

    const FuncData* p = pFD->Get( aProgrammaticName );
    if( p )
        aRet = GetFuncDescrStr( p->GetDescrID(), 1 );

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getDisplayArgumentName( const STRING& aName, sal_Int32 nArg ) THROWDEF_RTE
{
    STRING          aRet;

    const FuncData* p = pFD->Get( aName );
    if( p && nArg <= 0xFFFF )
    {
        sal_uInt16  nStr = p->GetStrIndex( sal_uInt16( nArg ) );
        if( nStr )
            aRet = GetFuncDescrStr( p->GetDescrID(), nStr );
        else
            aRet = STRFROMANSI( "internal" );
    }

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getArgumentDescription( const STRING& aName, sal_Int32 nArg ) THROWDEF_RTE
{
    STRING          aRet;

    const FuncData* p = pFD->Get( aName );
    if( p && nArg <= 0xFFFF )
    {
        sal_uInt16  nStr = p->GetStrIndex( sal_uInt16( nArg ) );
        if( nStr )
            aRet = GetFuncDescrStr( p->GetDescrID(), nStr + 1 );
        else
            aRet = STRFROMANSI( "for internal use only" );
    }

    return aRet;
}


static const char*  pDefCatName = "Add-In";


STRING SAL_CALL AnalysisAddIn::getProgrammaticCategoryName( const STRING& aName ) THROWDEF_RTE
{
    //  return non-translated strings
//  return STRFROMASCII( "Add-In" );
    const FuncData*     p = pFD->Get( aName );
    STRING              aRet;
    if( p )
    {
        const sal_Char* pStr;

        switch( p->GetCategory() )
        {
            case FDCat_DateTime:    pStr = "Date&Time";         break;
            case FDCat_Finance:     pStr = "Financial";         break;
            case FDCat_Inf:         pStr = "Information";       break;
            case FDCat_Math:        pStr = "Mathematical";      break;
            case FDCat_Tech:        pStr = "Technical";         break;
            default:
                                    pStr = pDefCatName;         break;
        }

        aRet = STRFROMASCII( pStr );
    }
    else
        aRet = STRFROMASCII( pDefCatName );

    return aRet;
}


STRING SAL_CALL AnalysisAddIn::getDisplayCategoryName( const STRING& aProgrammaticFunctionName ) THROWDEF_RTE
{
    //  return translated strings, not used for predefined categories
//  return STRFROMASCII( "Add-In" );
    const FuncData*     p = pFD->Get( aProgrammaticFunctionName );
    STRING              aRet;
    if( p )
    {
        const sal_Char* pStr;

        switch( p->GetCategory() )
        {
            case FDCat_DateTime:    pStr = "Date&Time";         break;
            case FDCat_Finance:     pStr = "Financial";         break;
            case FDCat_Inf:         pStr = "Information";       break;
            case FDCat_Math:        pStr = "Mathematical";      break;
            case FDCat_Tech:        pStr = "Technical";         break;
            default:
                                    pStr = pDefCatName;         break;
        }

        aRet = STRFROMASCII( pStr );
    }
    else
        aRet = STRFROMASCII( pDefCatName );

    return aRet;
}


static const sal_Char*      pLang[] = { "de", "en" };
static const sal_Char*      pCoun[] = { "DE", "US" };
static const sal_uInt32     nNumOfLoc = SAL_N_ELEMENTS(pLang);


void AnalysisAddIn::InitDefLocales( void )
{
    pDefLocales = new ::com::sun::star::lang::Locale[ nNumOfLoc ];

    for( sal_uInt32 n = 0 ; n < nNumOfLoc ; n++ )
    {
        pDefLocales[ n ].Language = STRING::createFromAscii( pLang[ n ] );
        pDefLocales[ n ].Country = STRING::createFromAscii( pCoun[ n ] );
    }
}


inline const ::com::sun::star::lang::Locale& AnalysisAddIn::GetLocale( sal_uInt32 nInd )
{
    if( !pDefLocales )
        InitDefLocales();

    if( nInd < sizeof( pLang ) )
        return pDefLocales[ nInd ];
    else
        return aFuncLoc;
}


SEQofLocName SAL_CALL AnalysisAddIn::getCompatibilityNames( const STRING& aProgrammaticName ) THROWDEF_RTE
{
    const FuncData*             p = pFD->Get( aProgrammaticName );

    if( !p )
        return SEQofLocName( 0 );

    const StringList&           r = p->GetCompNameList();
    sal_uInt32                  nCount = r.Count();

    SEQofLocName                aRet( nCount );

    ::com::sun::star::sheet::LocalizedName*  pArray = aRet.getArray();

    for( sal_uInt32 n = 0 ; n < nCount ; n++ )
    {
        pArray[ n ] = ::com::sun::star::sheet::LocalizedName( GetLocale( n ), *r.Get( n ) );
    }

    return aRet;
}


// XAnalysis

/**
 * Workday
 */

sal_Int32 SAL_CALL AnalysisAddIn::getWorkday( constREFXPS& xOptions,
    sal_Int32 nDate, sal_Int32 nDays, const ANY& aHDay ) THROWDEF_RTE_IAE
{
    if( !nDays )
        return nDate;

    sal_Int32                   nNullDate = GetNullDate( xOptions );

    SortedIndividualInt32List   aSrtLst;

    aSrtLst.InsertHolidayList( aAnyConv, xOptions, aHDay, nNullDate, sal_False );

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


/**
 * Yearfrac
 */

double SAL_CALL AnalysisAddIn::getYearfrac( constREFXPS& xOpt,
    sal_Int32 nStartDate, sal_Int32 nEndDate, const ANY& rMode ) THROWDEF_RTE_IAE
{
    double fRet = GetYearFrac( xOpt, nStartDate, nEndDate, getDateMode( xOpt, rMode ) );
    RETURN_FINITE( fRet );
}


sal_Int32 SAL_CALL AnalysisAddIn::getEdate( constREFXPS& xOpt, sal_Int32 nStartDate, sal_Int32 nMonths ) THROWDEF_RTE_IAE
{
    sal_Int32 nNullDate = GetNullDate( xOpt );
    ScaDate aDate( nNullDate, nStartDate, 5 );
    aDate.addMonths( nMonths );
    return aDate.getDate( nNullDate );
}


sal_Int32 SAL_CALL AnalysisAddIn::getWeeknum( constREFXPS& xOpt, sal_Int32 nDate, sal_Int32 nMode ) THROWDEF_RTE_IAE
{
    nDate += GetNullDate( xOpt );

    sal_uInt16  nDay, nMonth, nYear;
    DaysToDate( nDate, nDay, nMonth, nYear );

    sal_Int32   nFirstInYear = DateToDays( 1, 1, nYear );
    sal_uInt16  nFirstDayInYear = GetDayOfWeek( nFirstInYear );

    return ( nDate - nFirstInYear + ( ( nMode == 1 )? ( nFirstDayInYear + 1 ) % 7 : nFirstDayInYear ) ) / 7 + 1;
}


sal_Int32 SAL_CALL AnalysisAddIn::getEomonth( constREFXPS& xOpt, sal_Int32 nDate, sal_Int32 nMonths ) THROWDEF_RTE_IAE
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


sal_Int32 SAL_CALL AnalysisAddIn::getNetworkdays( constREFXPS& xOpt,
        sal_Int32 nStartDate, sal_Int32 nEndDate, const ANY& aHDay ) THROWDEF_RTE_IAE
{
    sal_Int32                   nNullDate = GetNullDate( xOpt );

    SortedIndividualInt32List   aSrtLst;

    aSrtLst.InsertHolidayList( aAnyConv, xOpt, aHDay, nNullDate, sal_False );

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


sal_Int32 SAL_CALL AnalysisAddIn::getIseven( sal_Int32 nVal ) THROWDEF_RTE_IAE
{
    return ( nVal & 0x00000001 )? 0 : 1;
}


sal_Int32 SAL_CALL AnalysisAddIn::getIsodd( sal_Int32 nVal ) THROWDEF_RTE_IAE
{
    return ( nVal & 0x00000001 )? 1 : 0;
}

double SAL_CALL
AnalysisAddIn::getMultinomial( constREFXPS& xOpt, const SEQSEQ( sal_Int32 )& aVLst,
                               const SEQ( uno::Any )& aOptVLst ) THROWDEF_RTE_IAE
{
    ScaDoubleListGE0 aValList;

    aValList.Append( aVLst );
    aValList.Append( aAnyConv, xOpt, aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    double nZ = 0;
    double fRet = 1.0;

    for( const double *p = aValList.First(); p; p = aValList.Next() )
    {
        double n = (*p >= 0.0) ? rtl::math::approxFloor( *p ) : rtl::math::approxCeil( *p );
        if ( n < 0.0 )
            THROW_IAE;

        if( n > 0.0 )
        {
            nZ += n;
            fRet *= BinomialCoefficient(nZ, n);
        }
    }
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getSeriessum( double fX, double fN, double fM, const SEQSEQ( double )& aCoeffList ) THROWDEF_RTE_IAE
{
    double                          fRet = 0.0;

    // #i32269# 0^0 is undefined, Excel returns #NUM! error
    if( fX == 0.0 && fN == 0 )
        THROW_RTE;

    if( fX != 0.0 )
    {
        sal_Int32       n1, n2;
        sal_Int32       nE1 = aCoeffList.getLength();
        sal_Int32       nE2;

        for( n1 = 0 ; n1 < nE1 ; n1++ )
        {
            const SEQ( double )&    rList = aCoeffList[ n1 ];
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


double SAL_CALL AnalysisAddIn::getQuotient( double fNum, double fDenom ) THROWDEF_RTE_IAE
{
    double fRet;
    if( (fNum < 0) != (fDenom < 0) )
        fRet = ::rtl::math::approxCeil( fNum / fDenom );
    else
        fRet = ::rtl::math::approxFloor( fNum / fDenom );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getMround( double fNum, double fMult ) THROWDEF_RTE_IAE
{
    if( fMult == 0.0 )
        return fMult;

    double fRet = fMult * ::rtl::math::round( fNum / fMult );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getSqrtpi( double fNum ) THROWDEF_RTE_IAE
{
    double fRet = sqrt( fNum * PI );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getRandbetween( double fMin, double fMax ) THROWDEF_RTE_IAE
{
    fMin = ::rtl::math::round( fMin, 0, rtl_math_RoundingMode_Up );
    fMax = ::rtl::math::round( fMax, 0, rtl_math_RoundingMode_Up );
    if( fMin > fMax )
        THROW_IAE;

    // fMax -> range
    double fRet = fMax - fMin + 1.0;
    fRet *= rand();
    fRet /= (RAND_MAX + 1.0);
    fRet += fMin;
    fRet = floor( fRet );   // simple floor is sufficient here
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getGcd( constREFXPS& xOpt, const SEQSEQ( double )& aVLst, const SEQ( uno::Any )& aOptVLst ) THROWDEF_RTE_IAE
{
    ScaDoubleListGT0 aValList;

    aValList.Append( aVLst );
    aValList.Append( aAnyConv, xOpt, aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    const double*   p = aValList.First();
    double          f = *p;

    p = aValList.Next();

    while( p )
    {
        f = GetGcd( *p, f );
        p = aValList.Next();
    }

    RETURN_FINITE( f );
}


double SAL_CALL AnalysisAddIn::getLcm( constREFXPS& xOpt, const SEQSEQ( double )& aVLst, const SEQ( uno::Any )& aOptVLst ) THROWDEF_RTE_IAE
{
    ScaDoubleListGE0 aValList;

    aValList.Append( aVLst );
    aValList.Append( aAnyConv, xOpt, aOptVLst );

    if( aValList.Count() == 0 )
        return 0.0;

    const double*   p = aValList.First();
    double          f = *p;

    if( f == 0.0 )
        return f;

    p = aValList.Next();

    while( p )
    {
        double      fTmp = *p;
        if( f == 0.0 )
            return f;
        else
            f = fTmp * f / GetGcd( fTmp, f );
        p = aValList.Next();
    }

    RETURN_FINITE( f );
}


double SAL_CALL AnalysisAddIn::getBesseli( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE
{
    double fRet = sca::analysis::BesselI( fNum, nOrder );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getBesselj( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE
{
    double fRet = sca::analysis::BesselJ( fNum, nOrder );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getBesselk( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE
{
    if( nOrder < 0 || fNum <= 0.0 )
        THROW_IAE;

    double fRet = sca::analysis::BesselK( fNum, nOrder );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getBessely( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE
{
    if( nOrder < 0 || fNum <= 0.0 )
        THROW_IAE;

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


STRING SAL_CALL AnalysisAddIn::getBin2Oct( constREFXPS& xOpt, const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    double fVal = ConvertToDec( aNum, 2, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN8, SCA_MAX8, 8, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


double SAL_CALL AnalysisAddIn::getBin2Dec( const STRING& aNum ) THROWDEF_RTE_IAE
{
    double fRet = ConvertToDec( aNum, 2, SCA_MAXPLACES );
    RETURN_FINITE( fRet );
}


STRING SAL_CALL AnalysisAddIn::getBin2Hex( constREFXPS& xOpt, const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    double fVal = ConvertToDec( aNum, 2, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN16, SCA_MAX16, 16, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


STRING SAL_CALL AnalysisAddIn::getOct2Bin( constREFXPS& xOpt, const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    double fVal = ConvertToDec( aNum, 8, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN2, SCA_MAX2, 2, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


double SAL_CALL AnalysisAddIn::getOct2Dec( const STRING& aNum ) THROWDEF_RTE_IAE
{
    double fRet = ConvertToDec( aNum, 8, SCA_MAXPLACES );
    RETURN_FINITE( fRet );
}


STRING SAL_CALL AnalysisAddIn::getOct2Hex( constREFXPS& xOpt, const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    double fVal = ConvertToDec( aNum, 8, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN16, SCA_MAX16, 16, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


STRING SAL_CALL AnalysisAddIn::getDec2Bin( constREFXPS& xOpt, sal_Int32 nNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( nNum, SCA_MIN2, SCA_MAX2, 2, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


STRING SAL_CALL AnalysisAddIn::getDec2Oct( constREFXPS& xOpt, sal_Int32 nNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( nNum, SCA_MIN8, SCA_MAX8, 8, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


STRING SAL_CALL AnalysisAddIn::getDec2Hex( constREFXPS& xOpt, double fNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fNum, SCA_MIN16, SCA_MAX16, 16, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


STRING SAL_CALL AnalysisAddIn::getHex2Bin( constREFXPS& xOpt, const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    double fVal = ConvertToDec( aNum, 16, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN2, SCA_MAX2, 2, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


double SAL_CALL AnalysisAddIn::getHex2Dec( const STRING& aNum ) THROWDEF_RTE_IAE
{
    double fRet = ConvertToDec( aNum, 16, SCA_MAXPLACES );
    RETURN_FINITE( fRet );
}


STRING SAL_CALL AnalysisAddIn::getHex2Oct( constREFXPS& xOpt, const STRING& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE
{
    double fVal = ConvertToDec( aNum, 16, SCA_MAXPLACES );
    sal_Int32 nPlaces = 0;
    sal_Bool bUsePlaces = aAnyConv.getInt32( nPlaces, xOpt, rPlaces );
    return ConvertFromDec( fVal, SCA_MIN8, SCA_MAX8, 8, nPlaces, SCA_MAXPLACES, bUsePlaces );
}


sal_Int32 SAL_CALL AnalysisAddIn::getDelta( constREFXPS& xOpt, double fNum1, const ANY& rNum2 ) THROWDEF_RTE_IAE
{
    return fNum1 == aAnyConv.getDouble( xOpt, rNum2, 0.0 );
}


double SAL_CALL AnalysisAddIn::getErf( constREFXPS& xOpt, double fLL, const ANY& rUL ) THROWDEF_RTE_IAE
{
    double fUL, fRet;
    sal_Bool bContainsValue = aAnyConv.getDouble( fUL, xOpt, rUL );

    fRet = bContainsValue ? (Erf( fUL ) - Erf( fLL )) : Erf( fLL );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getErfc( double f ) THROWDEF_RTE_IAE
{
    double fRet = Erfc( f );
    RETURN_FINITE( fRet );
}


sal_Int32 SAL_CALL AnalysisAddIn::getGestep( constREFXPS& xOpt, double fNum, const ANY& rStep ) THROWDEF_RTE_IAE
{
    return fNum >= aAnyConv.getDouble( xOpt, rStep, 0.0 );
}


double SAL_CALL AnalysisAddIn::getFactdouble( sal_Int32 nNum ) THROWDEF_RTE_IAE
{
    double fRet = FactDouble( nNum );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getImabs( const STRING& aNum ) THROWDEF_RTE_IAE
{
    double fRet = Complex( aNum ).Abs();
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getImaginary( const STRING& aNum ) THROWDEF_RTE_IAE
{
    double fRet = Complex( aNum ).Imag();
    RETURN_FINITE( fRet );
}


STRING SAL_CALL AnalysisAddIn::getImpower( const STRING& aNum, double f ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Power( f );

    return z.GetString();
}


double SAL_CALL AnalysisAddIn::getImargument( const STRING& aNum ) THROWDEF_RTE_IAE
{
    double fRet = Complex( aNum ).Arg();
    RETURN_FINITE( fRet );
}


STRING SAL_CALL AnalysisAddIn::getImcos( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Cos();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImdiv( const STRING& aDivid, const STRING& aDivis ) THROWDEF_RTE_IAE
{
    Complex     z( aDivid );

    z.Div( Complex( aDivis ) );

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImexp( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Exp();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImconjugate( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Conjugate();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImln( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Ln();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImlog10( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Log10();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImlog2( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Log2();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImproduct( constREFXPS&, const SEQSEQ( STRING )& aNum1, const SEQ( uno::Any )& aNL ) THROWDEF_RTE_IAE
{
    ComplexList     z_list;

    z_list.Append( aNum1, AH_IgnoreEmpty );
    z_list.Append( aNL, AH_IgnoreEmpty );

    const Complex*  p = z_list.First();

    if( !p )
        return Complex( 0 ).GetString();

    Complex         z( *p );

    for( p = z_list.Next() ; p ; p = z_list.Next() )
        z.Mult( *p );

    return z.GetString();
}


double SAL_CALL AnalysisAddIn::getImreal( const STRING& aNum ) THROWDEF_RTE_IAE
{
    double fRet = Complex( aNum ).Real();
    RETURN_FINITE( fRet );
}


STRING SAL_CALL AnalysisAddIn::getImsin( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Sin();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsub( const STRING& aNum1, const STRING& aNum2 ) THROWDEF_RTE_IAE
{
    Complex     z( aNum1 );

    z.Sub( Complex( aNum2 ) );

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsum( constREFXPS&, const SEQSEQ( STRING )& aNum1, const SEQ( ::com::sun::star::uno::Any )& aFollowingPars ) THROWDEF_RTE_IAE
{
    ComplexList     z_list;

    z_list.Append( aNum1, AH_IgnoreEmpty );
    z_list.Append( aFollowingPars, AH_IgnoreEmpty );

    const Complex*  p = z_list.First();

    if( !p )
        return Complex( 0 ).GetString();

    Complex         z( *p );

    for( p = z_list.Next() ; p ; p = z_list.Next() )
        z.Add( *p );

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsqrt( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Sqrt();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImtan( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Tan();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsec( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Sec();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImcsc( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Csc();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImcot( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Cot();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsinh( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Sinh();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImcosh( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Cosh();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImsech( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Sech();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getImcsch( const STRING& aNum ) THROWDEF_RTE_IAE
{
    Complex     z( aNum );

    z.Csch();

    return z.GetString();
}


STRING SAL_CALL AnalysisAddIn::getComplex( double fR, double fI, const ANY& rSuff ) THROWDEF_RTE_IAE
{
    sal_Bool    bi;

    switch( rSuff.getValueTypeClass() )
    {
        case uno::TypeClass_VOID:
            bi = sal_True;
            break;
        case uno::TypeClass_STRING:
            {
            const STRING*   pSuff = ( const STRING* ) rSuff.getValue();
            bi = pSuff->compareToAscii( "i" ) == 0 || pSuff->isEmpty();
            if( !bi && pSuff->compareToAscii( "j" ) != 0 )
                THROW_IAE;
            }
            break;
        default:
            THROW_IAE;
    }

    return Complex( fR, fI, bi ? 'i' : 'j' ).GetString();
}


double SAL_CALL AnalysisAddIn::getConvert( double f, const STRING& aFU, const STRING& aTU ) THROWDEF_RTE_IAE
{
    if( !pCDL )
        pCDL = new ConvertDataList();

    double fRet = pCDL->Convert( f, aFU, aTU );
    RETURN_FINITE( fRet );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
