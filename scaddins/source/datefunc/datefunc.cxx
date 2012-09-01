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

//------------------------------------------------------------------
//
// date functions add in
//
//------------------------------------------------------------------

#include "datefunc.hxx"
#include "datefunc.hrc"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <tools/resmgr.hxx>
#include <tools/rcid.h>
#include <com/sun/star/util/Date.hpp>

using namespace ::com::sun::star;
using namespace ::rtl;

//------------------------------------------------------------------

#define ADDIN_SERVICE           "com.sun.star.sheet.AddIn"
#define MY_SERVICE              "com.sun.star.sheet.addin.DateFunctions"
#define MY_IMPLNAME             "com.sun.star.sheet.addin.DateFunctionsImpl"

//------------------------------------------------------------------

#define STR_FROM_ANSI( s )      OUString( s, strlen( s ), RTL_TEXTENCODING_MS_1252 )

//------------------------------------------------------------------

const sal_uInt32 ScaList::nStartSize = 16;
const sal_uInt32 ScaList::nIncrSize = 16;

ScaList::ScaList() :
    pData( new void*[ nStartSize ] ),
    nSize( nStartSize ),
    nCount( 0 ),
    nCurr( 0 )
{
}

ScaList::~ScaList()
{
    delete[] pData;
}

void ScaList::_Grow()
{
    nSize += nIncrSize;

    void** pNewData = new void*[ nSize ];
    memcpy( pNewData, pData, nCount * sizeof( void* ) );

    delete[] pData;
    pData = pNewData;
}

//------------------------------------------------------------------

ScaStringList::~ScaStringList()
{
    for( OUString* pStr = First(); pStr; pStr = Next() )
        delete pStr;
}

//------------------------------------------------------------------

ScaResId::ScaResId( sal_uInt16 nId, ResMgr& rResMgr ) :
    ResId( nId, rResMgr )
{
}


//------------------------------------------------------------------

#define UNIQUE              sal_False   // function name does not exist in Calc
#define DOUBLE              sal_True    // function name exists in Calc

#define STDPAR              sal_False   // all parameters are described
#define INTPAR              sal_True    // first parameter is internal

#define FUNCDATA( FuncName, ParamCount, Category, Double, IntPar )  \
    { "get" #FuncName, DATE_FUNCNAME_##FuncName, DATE_FUNCDESC_##FuncName, DATE_DEFFUNCNAME_##FuncName, ParamCount, Category, Double, IntPar }

const ScaFuncDataBase pFuncDataArr[] =
{
    FUNCDATA( DiffWeeks,    3, ScaCat_DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DiffMonths,   3, ScaCat_DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DiffYears,    3, ScaCat_DateTime, UNIQUE, INTPAR ),
    FUNCDATA( IsLeapYear,   1, ScaCat_DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DaysInMonth,  1, ScaCat_DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DaysInYear,   1, ScaCat_DateTime, UNIQUE, INTPAR ),
    FUNCDATA( WeeksInYear,  1, ScaCat_DateTime, UNIQUE, INTPAR ),
    FUNCDATA( Rot13,        1, ScaCat_Text,     UNIQUE, STDPAR )
};

#undef FUNCDATA


//------------------------------------------------------------------

ScaFuncData::ScaFuncData( const ScaFuncDataBase& rBaseData, ResMgr& rResMgr ) :
    aIntName( OUString::createFromAscii( rBaseData.pIntName ) ),
    nUINameID( rBaseData.nUINameID ),
    nDescrID( rBaseData.nDescrID ),
    nCompListID( rBaseData.nCompListID ),
    nParamCount( rBaseData.nParamCount ),
    eCat( rBaseData.eCat ),
    bDouble( rBaseData.bDouble ),
    bWithOpt( rBaseData.bWithOpt )
{
    ScaResStringArrLoader aArrLoader( RID_DATE_DEFFUNCTION_NAMES, nCompListID, rResMgr );
    const ResStringArray& rArr = aArrLoader.GetStringArray();

    for( sal_uInt16 nIndex = 0; nIndex < rArr.Count(); nIndex++ )
        aCompList.Append( rArr.GetString( nIndex ) );
}

ScaFuncData::~ScaFuncData()
{
}

sal_uInt16 ScaFuncData::GetStrIndex( sal_uInt16 nParam ) const
{
    if( !bWithOpt )
        nParam++;
    return (nParam > nParamCount) ? (nParamCount * 2) : (nParam * 2);
}


//------------------------------------------------------------------

ScaFuncDataList::ScaFuncDataList( ResMgr& rResMgr ) :
    nLast( 0xFFFFFFFF )
{
    for( sal_uInt16 nIndex = 0; nIndex < SAL_N_ELEMENTS(pFuncDataArr); nIndex++ )
        Append( new ScaFuncData( pFuncDataArr[ nIndex ], rResMgr ) );
}

ScaFuncDataList::~ScaFuncDataList()
{
    for( ScaFuncData* pFData = First(); pFData; pFData = Next() )
        delete pFData;
}

const ScaFuncData* ScaFuncDataList::Get( const OUString& rProgrammaticName ) const
{
    if( aLastName == rProgrammaticName )
        return Get( nLast );

    for( sal_uInt32 nIndex = 0; nIndex < Count(); nIndex++ )
    {
        const ScaFuncData* pCurr = Get( nIndex );
        if( pCurr->Is( rProgrammaticName ) )
        {
            const_cast< ScaFuncDataList* >( this )->aLastName = rProgrammaticName;
            const_cast< ScaFuncDataList* >( this )->nLast = nIndex;
            return pCurr;
        }
    }
    return NULL;
}


//------------------------------------------------------------------

ScaFuncRes::ScaFuncRes( ResId& rResId, ResMgr& rResMgr, sal_uInt16 nIndex, OUString& rRet ) :
    Resource( rResId )
{
    rRet = String( ScaResId( nIndex, rResMgr ) );
    FreeResource();
}


//------------------------------------------------------------------
//
//  entry points for service registration / instantiation
//
//------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL ScaDateAddIn_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory >& )
{
    static uno::Reference< uno::XInterface > xInst = (cppu::OWeakObject*) new ScaDateAddIn();
    return xInst;
}


//------------------------------------------------------------------------

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL date_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void* pRet = 0;

    if ( pServiceManager &&
            OUString::createFromAscii( pImplName ) == ScaDateAddIn::getImplementationName_Static() )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory( cppu::createOneInstanceFactory(
                reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                ScaDateAddIn::getImplementationName_Static(),
                ScaDateAddIn_CreateInstance,
                ScaDateAddIn::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}   // extern C

//------------------------------------------------------------------------
//
//  "normal" service implementation
//
//------------------------------------------------------------------------

ScaDateAddIn::ScaDateAddIn() :
    pDefLocales( NULL ),
    pResMgr( NULL ),
    pFuncDataList( NULL )
{
}

ScaDateAddIn::~ScaDateAddIn()
{
    if( pFuncDataList )
        delete pFuncDataList;
    if( pDefLocales )
        delete[] pDefLocales;

    // pResMgr already deleted (_all_ resource managers are deleted _before_ this dtor is called)
}

static const sal_Char*  pLang[] = { "de", "en" };
static const sal_Char*  pCoun[] = { "DE", "US" };
static const sal_uInt32 nNumOfLoc = SAL_N_ELEMENTS( pLang );

void ScaDateAddIn::InitDefLocales()
{
    pDefLocales = new lang::Locale[ nNumOfLoc ];

    for( sal_uInt32 nIndex = 0; nIndex < nNumOfLoc; nIndex++ )
    {
        pDefLocales[ nIndex ].Language = OUString::createFromAscii( pLang[ nIndex ] );
        pDefLocales[ nIndex ].Country = OUString::createFromAscii( pCoun[ nIndex ] );
    }
}

const lang::Locale& ScaDateAddIn::GetLocale( sal_uInt32 nIndex )
{
    if( !pDefLocales )
        InitDefLocales();

    return (nIndex < sizeof( pLang )) ? pDefLocales[ nIndex ] : aFuncLoc;
}

ResMgr& ScaDateAddIn::GetResMgr() throw( uno::RuntimeException )
{
    if( !pResMgr )
    {
        InitData();     // try to get resource manager
        if( !pResMgr )
            throw uno::RuntimeException();
    }
    return *pResMgr;
}

void ScaDateAddIn::InitData()
{
    if( pResMgr )
        delete pResMgr;

    OString aModName( "date" );
    pResMgr = ResMgr::CreateResMgr( aModName.getStr(), aFuncLoc );

    if( pFuncDataList )
        delete pFuncDataList;

    pFuncDataList = pResMgr ? new ScaFuncDataList( *pResMgr ) : NULL;

    if( pDefLocales )
    {
        delete pDefLocales;
        pDefLocales = NULL;
    }
}

OUString ScaDateAddIn::GetDisplFuncStr( sal_uInt16 nResId ) throw( uno::RuntimeException )
{
    return ScaResStringLoader( RID_DATE_FUNCTION_NAMES, nResId, GetResMgr() ).GetString();
}

OUString ScaDateAddIn::GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex ) throw( uno::RuntimeException )
{
    OUString aRet;

    ScaResPublisher aResPubl( ScaResId( RID_DATE_FUNCTION_DESCRIPTIONS, GetResMgr() ) );
    ScaResId aResId( nResId, GetResMgr() );
    aResId.SetRT( RSC_RESOURCE );

    if( aResPubl.IsAvailableRes( aResId ) )
        ScaFuncRes aSubRes( aResId, GetResMgr(), nStrIndex, aRet );

    aResPubl.FreeResource();
    return aRet;
}


//------------------------------------------------------------------------

OUString ScaDateAddIn::getImplementationName_Static()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM( MY_IMPLNAME ));
}

uno::Sequence< OUString > ScaDateAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aRet( 2 );
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString(RTL_CONSTASCII_USTRINGPARAM( ADDIN_SERVICE ));
    pArray[1] = OUString(RTL_CONSTASCII_USTRINGPARAM( MY_SERVICE ));
    return aRet;
}

// XServiceName

OUString SAL_CALL ScaDateAddIn::getServiceName() throw( uno::RuntimeException )
{
    // name of specific AddIn service
    return OUString(RTL_CONSTASCII_USTRINGPARAM( MY_SERVICE ));
}

// XServiceInfo

OUString SAL_CALL ScaDateAddIn::getImplementationName() throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ScaDateAddIn::supportsService( const OUString& aServiceName ) throw( uno::RuntimeException )
{
    return aServiceName == ADDIN_SERVICE || aServiceName == MY_SERVICE;
}

uno::Sequence< OUString > SAL_CALL ScaDateAddIn::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// XLocalizable

void SAL_CALL ScaDateAddIn::setLocale( const lang::Locale& eLocale ) throw( uno::RuntimeException )
{
    aFuncLoc = eLocale;
    InitData();     // change of locale invalidates resources!
}

lang::Locale SAL_CALL ScaDateAddIn::getLocale() throw( uno::RuntimeException )
{
    return aFuncLoc;
}

//------------------------------------------------------------------
//
//  function descriptions start here
//
//------------------------------------------------------------------

// XAddIn

OUString SAL_CALL ScaDateAddIn::getProgrammaticFuntionName( const OUString& ) throw( uno::RuntimeException )
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)
    return OUString();
}

OUString SAL_CALL ScaDateAddIn::getDisplayFunctionName( const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData )
    {
        aRet = GetDisplFuncStr( pFData->GetUINameID() );
        if( pFData->IsDouble() )
            aRet += STR_FROM_ANSI( "_ADD" );
    }
    else
    {
        aRet = STR_FROM_ANSI( "UNKNOWNFUNC_" );
        aRet += aProgrammaticName;
    }

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getFunctionDescription( const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData )
        aRet = GetFuncDescrStr( pFData->GetDescrID(), 1 );

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getDisplayArgumentName(
        const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = pFData->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( pFData->GetDescrID(), nStr );
        else
            aRet = STR_FROM_ANSI( "internal" );
    }

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getArgumentDescription(
        const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = pFData->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( pFData->GetDescrID(), nStr + 1 );
        else
            aRet = STR_FROM_ANSI( "for internal use only" );
    }

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getProgrammaticCategoryName(
        const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData )
    {
        switch( pFData->GetCategory() )
        {
            case ScaCat_DateTime:   aRet = STR_FROM_ANSI( "Date&Time" );    break;
            case ScaCat_Text:       aRet = STR_FROM_ANSI( "Text" );         break;
            case ScaCat_Finance:    aRet = STR_FROM_ANSI( "Financial" );    break;
            case ScaCat_Inf:        aRet = STR_FROM_ANSI( "Information" );  break;
            case ScaCat_Math:       aRet = STR_FROM_ANSI( "Mathematical" ); break;
            case ScaCat_Tech:       aRet = STR_FROM_ANSI( "Technical" );    break;
            default:    // to prevent compiler warnings
                break;
        }
    }

    if( aRet.isEmpty() )
        aRet = STR_FROM_ANSI( "Add-In" );
    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getDisplayCategoryName(
        const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    return getProgrammaticCategoryName( aProgrammaticName );
}


// XCompatibilityNames

uno::Sequence< sheet::LocalizedName > SAL_CALL ScaDateAddIn::getCompatibilityNames(
        const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( !pFData )
        return uno::Sequence< sheet::LocalizedName >( 0 );

    const ScaStringList& rStrList = pFData->GetCompNameList();
    sal_uInt32 nCount = rStrList.Count();

    uno::Sequence< sheet::LocalizedName > aRet( nCount );
    sheet::LocalizedName* pArray = aRet.getArray();

    for( sal_uInt32 nIndex = 0; nIndex < nCount; nIndex++ )
        pArray[ nIndex ] = sheet::LocalizedName( GetLocale( nIndex ), *rStrList.Get( nIndex ) );

    return aRet;
}

namespace {
// auxiliary functions

sal_Bool IsLeapYear( sal_uInt16 nYear )
{
    return ((((nYear % 4) == 0) && ((nYear % 100) != 0)) || ((nYear % 400) == 0));
}

sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{
    static sal_uInt16 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31 };

    if ( nMonth != 2 )
        return aDaysInMonth[nMonth-1];
    else
    {
        if ( IsLeapYear(nYear) )
            return aDaysInMonth[nMonth-1] + 1;
        else
            return aDaysInMonth[nMonth-1];
    }
}

/**
 * Convert a date to a count of days starting from 01/01/0001
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts a Day , Month, Year representation
 * to this internal Date value.
 */

sal_Int32 DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
{
    sal_Int32 nDays = ((sal_Int32)nYear-1) * 365;
    nDays += ((nYear-1) / 4) - ((nYear-1) / 100) + ((nYear-1) / 400);

    for( sal_uInt16 i = 1; i < nMonth; i++ )
        nDays += DaysInMonth(i,nYear);
    nDays += nDay;

    return nDays;
}

/**
 * Convert a count of days starting from 01/01/0001 to a date
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts this internal Date value
 * to a Day , Month, Year representation of a Date.
 */

void DaysToDate( sal_Int32 nDays,
                sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear )
        throw( lang::IllegalArgumentException )
{
    if( nDays < 0 )
        throw lang::IllegalArgumentException();

    sal_Int32   nTempDays;
    sal_Int32   i = 0;
    sal_Bool    bCalc;

    do
    {
        nTempDays = nDays;
        rYear = (sal_uInt16)((nTempDays / 365) - i);
        nTempDays -= ((sal_Int32) rYear -1) * 365;
        nTempDays -= (( rYear -1) / 4) - (( rYear -1) / 100) + ((rYear -1) / 400);
        bCalc = sal_False;
        if ( nTempDays < 1 )
        {
            i++;
            bCalc = sal_True;
        }
        else
        {
            if ( nTempDays > 365 )
            {
                if ( (nTempDays != 366) || !IsLeapYear( rYear ) )
                {
                    i--;
                    bCalc = sal_True;
                }
            }
        }
    }
    while ( bCalc );

    rMonth = 1;
    while ( (sal_Int32)nTempDays > DaysInMonth( rMonth, rYear ) )
    {
        nTempDays -= DaysInMonth( rMonth, rYear );
        rMonth++;
    }
    rDay = (sal_uInt16)nTempDays;
}

/**
 * Get the null date used by the spreadsheet document
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function returns this internal Date value for the document null date
 *
 */

sal_Int32 GetNullDate( const uno::Reference< beans::XPropertySet >& xOptions )
        throw( uno::RuntimeException )
{
    if (xOptions.is())
    {
        try
        {
            uno::Any aAny = xOptions->getPropertyValue(
                                        OUString(RTL_CONSTASCII_USTRINGPARAM( "NullDate" )) );
            util::Date aDate;
            if ( aAny >>= aDate )
                return DateToDays( aDate.Day, aDate.Month, aDate.Year );
        }
        catch (uno::Exception&)
        {
        }
    }

    // no null date available -> no calculations possible
    throw uno::RuntimeException();
}

}
// XDateFunctions

/**
 * Get week difference between 2 dates
 *
 * new Weeks(date1,date2,mode) function for StarCalc
 *
 * Two modes of operation are provided.
 * The first is just a simple division by 7 calculation.
 *
 * The second calculates the diffence by week of year.
 *
 * The International Standard IS-8601 has decreed that Monday
 * shall be the first day of the week.
 *
 * A week that lies partly in one year and partly in annother
 * is assigned a number in the the year in which most of its days lie.
 *
 * That means that week 1 of any year is the week that contains the 4. January
 *
 * The internal representation of a Date used in the Addin is the number of days based on 01/01/0001
 *
 * A WeekDay can be then calculated by substracting 1 and calculating the rest of
 * a division by 7, which gives a 0 - 6 value for Monday - Sunday
 *
 * Using the 4. January rule explained above the formula
 *
 *  nWeek1= ( nDays1 - nJan4 + ( (nJan4-1) % 7 ) ) / 7 + 1;
 *
 * calculates a number between 0-53 for each day which is in the same year as nJan4
 * where 0 means that this week belonged to the year before.
 *
 * If a day in the same or annother year is used in this formula this calculates
 * an calendar week offset from a given 4. January
 *
 *  nWeek2 = ( nDays2 - nJan4 + ( (nJan4-1) % 7 ) ) / 7 + 1;
 *
 * The 4.January of first Date Argument can thus be used to calculate
 * the week difference by calendar weeks which is then nWeek = nWeek2 - nWeek1
 *
 * which can be optimized to
 *
 * nWeek = ( (nDays2-nJan4+((nJan4-1)%7))/7 ) - ( (nDays1-nJan4+((nJan4-1)%7))/7 )
 *
 * Note: All calculations are operating on the long integer data type
 * % is the modulo operator in C which calculates the rest of an Integer division
 *
 *
 * mode 0 is the interval between the dates in month, that is days / 7
 *
 * mode 1 is the difference by week of year
 *
 */

sal_Int32 SAL_CALL ScaDateAddIn::getDiffWeeks(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nStartDate, sal_Int32 nEndDate,
        sal_Int32 nMode ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nNullDate = GetNullDate( xOptions );

    sal_Int32 nDays1 = nStartDate + nNullDate;
    sal_Int32 nDays2 = nEndDate + nNullDate;

    sal_Int32 nRet;

    if ( nMode == 1 )
    {
        sal_uInt16 nDay,nMonth,nYear;
        DaysToDate( nDays1, nDay, nMonth, nYear );
        sal_Int32 nJan4 = DateToDays( 4, 1, nYear );

        nRet = ( (nDays2-nJan4+((nJan4-1)%7))/7 ) - ( (nDays1-nJan4+((nJan4-1)%7))/7 );
    }
    else
    {
        nRet = (nDays2 - nDays1) / 7;
    }
    return nRet;
}

/**
 * Get month difference between 2 dates
 * =Month(start, end, mode) Function for StarCalc
 *
 * two modes are provided
 *
 * mode 0 is the interval between the dates in month
 *
 * mode 1 is the difference in calendar month
 */

sal_Int32 SAL_CALL ScaDateAddIn::getDiffMonths(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nStartDate, sal_Int32 nEndDate,
        sal_Int32 nMode ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nNullDate = GetNullDate( xOptions );

    sal_Int32 nDays1 = nStartDate + nNullDate;
    sal_Int32 nDays2 = nEndDate + nNullDate;

    sal_uInt16 nDay1,nMonth1,nYear1;
    sal_uInt16 nDay2,nMonth2,nYear2;
    DaysToDate(nDays1,nDay1,nMonth1,nYear1);
    DaysToDate(nDays2,nDay2,nMonth2,nYear2);

    sal_Int32 nRet = nMonth2 - nMonth1 + (nYear2 - nYear1) * 12;
    if ( nMode == 1 || nDays1 == nDays2 ) return nRet;

    if ( nDays1 < nDays2 )
    {
        if ( nDay1 > nDay2 )
        {
            nRet -= 1;
        }
    }
    else
    {
        if ( nDay1 < nDay2 )
        {
            nRet += 1;
        }
    }

    return nRet;
}

/**
 * Get Year difference between 2 dates
 *
 * two modes are provided
 *
 * mode 0 is the interval between the dates in years
 *
 * mode 1 is the difference in calendar years
 */

sal_Int32 SAL_CALL ScaDateAddIn::getDiffYears(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nStartDate, sal_Int32 nEndDate,
        sal_Int32 nMode ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    if ( nMode != 1 )
        return getDiffMonths( xOptions, nStartDate, nEndDate, nMode ) / 12;

    sal_Int32 nNullDate = GetNullDate( xOptions );

    sal_Int32 nDays1 = nStartDate + nNullDate;
    sal_Int32 nDays2 = nEndDate + nNullDate;

    sal_uInt16 nDay1,nMonth1,nYear1;
    sal_uInt16 nDay2,nMonth2,nYear2;
    DaysToDate(nDays1,nDay1,nMonth1,nYear1);
    DaysToDate(nDays2,nDay2,nMonth2,nYear2);

    return nYear2 - nYear1;
}

/**
 * Check if a Date is in a leap year in the Gregorian calendar
 */

sal_Int32 SAL_CALL ScaDateAddIn::getIsLeapYear(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nDate ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nNullDate = GetNullDate( xOptions );
    sal_Int32 nDays = nDate + nNullDate;

    sal_uInt16 nDay, nMonth, nYear;
    DaysToDate(nDays,nDay,nMonth,nYear);

    return (sal_Int32)IsLeapYear(nYear);
}

/**
 * Get the Number of Days in the month for a date
 */

sal_Int32 SAL_CALL ScaDateAddIn::getDaysInMonth(
        const uno::Reference<beans::XPropertySet>& xOptions,
        sal_Int32 nDate ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nNullDate = GetNullDate( xOptions );
    sal_Int32 nDays = nDate + nNullDate;

    sal_uInt16 nDay, nMonth, nYear;
    DaysToDate(nDays,nDay,nMonth,nYear);

    return DaysInMonth( nMonth, nYear );
}

/**
 * Get number of days in the year of a date specified
 */

sal_Int32 SAL_CALL ScaDateAddIn::getDaysInYear(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nDate ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nNullDate = GetNullDate( xOptions );
    sal_Int32 nDays = nDate + nNullDate;

    sal_uInt16 nDay, nMonth, nYear;
    DaysToDate(nDays,nDay,nMonth,nYear);

    return ( IsLeapYear(nYear) ? 366 : 365 );
}

/**
 * Get number of weeks in the year for a date
 *
 * Most years have 52 weeks, but years that start on a Thursday
 * and leep years that start on a Wednesday have 53 weeks
 *
 * The International Standard IS-8601 has decreed that Monday
 * shall be the first day of the week.
 *
 * A WeekDay can be calculated by substracting 1 and calculating the rest of
 * a division by 7 from the internal date represention
 * which gives a 0 - 6 value for Monday - Sunday
 *
 * @see #IsLeapYear #WeekNumber
 */

sal_Int32 SAL_CALL ScaDateAddIn::getWeeksInYear(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nDate ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    sal_Int32 nNullDate = GetNullDate( xOptions );
    sal_Int32 nDays = nDate + nNullDate;

    sal_uInt16 nDay, nMonth, nYear;
    DaysToDate(nDays,nDay,nMonth,nYear);

    sal_Int32 nJan1WeekDay = ( DateToDays(1,1,nYear) - 1) % 7;

    sal_Int32 nRet;
    if ( nJan1WeekDay == 3 )        /* Thursday */
        nRet = 53;
    else if ( nJan1WeekDay == 2 )   /* Wednesday */
        nRet = ( IsLeapYear(nYear) ? 53 : 52 );
    else
        nRet = 52;

    return nRet;
}

/**
 * Encrypt or decrypt a string using ROT13 algorithm
 *
 * This function rotates each character by 13 in the alphabet.
 * Only the characters 'a' ... 'z' and 'A' ... 'Z' are modified.
 */

OUString SAL_CALL ScaDateAddIn::getRot13( const OUString& aSrcString ) throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    OUStringBuffer aBuffer( aSrcString );
    for( sal_Int32 nIndex = 0; nIndex < aBuffer.getLength(); nIndex++ )
    {
        sal_Unicode cChar = aBuffer[nIndex];
        if( ((cChar >= 'a') && (cChar <= 'z') && ((cChar += 13) > 'z')) ||
            ((cChar >= 'A') && (cChar <= 'Z') && ((cChar += 13) > 'Z')) )
            cChar -= 26;
        aBuffer[nIndex] = cChar;
    }
    return aBuffer.makeStringAndClear();
}

//------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
