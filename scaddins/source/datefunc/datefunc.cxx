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

#include "datefunc.hxx"
#include <datefunc.hrc>
#include <strings.hrc>
#include <com/sun/star/util/Date.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustrbuf.hxx>
#include <unotools/resmgr.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <algorithm>
#include <cmath>
#include "deffuncname.hxx"

using namespace ::com::sun::star;

constexpr OUString ADDIN_SERVICE = u"com.sun.star.sheet.AddIn"_ustr;
constexpr OUString MY_SERVICE = u"com.sun.star.sheet.addin.DateFunctions"_ustr;
constexpr OUStringLiteral MY_IMPLNAME = u"com.sun.star.sheet.addin.DateFunctionsImpl";

#define UNIQUE              false   // function name does not exist in Calc

#define STDPAR              false   // all parameters are described
#define INTPAR              true    // first parameter is internal

#define FUNCDATA( FuncName, ParamCount, Category, Double, IntPar )  \
    { "get" #FuncName, DATE_FUNCNAME_##FuncName, DATE_FUNCDESC_##FuncName, DATE_DEFFUNCNAME_##FuncName, ParamCount, Category, Double, IntPar }

const ScaFuncDataBase pFuncDataArr[] =
{
    FUNCDATA( DiffWeeks,    3, ScaCategory::DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DiffMonths,   3, ScaCategory::DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DiffYears,    3, ScaCategory::DateTime, UNIQUE, INTPAR ),
    FUNCDATA( IsLeapYear,   1, ScaCategory::DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DaysInMonth,  1, ScaCategory::DateTime, UNIQUE, INTPAR ),
    FUNCDATA( DaysInYear,   1, ScaCategory::DateTime, UNIQUE, INTPAR ),
    FUNCDATA( WeeksInYear,  1, ScaCategory::DateTime, UNIQUE, INTPAR ),
    FUNCDATA( Rot13,        1, ScaCategory::Text,     UNIQUE, STDPAR )
};

#undef FUNCDATA

ScaFuncData::ScaFuncData(const ScaFuncDataBase& rBaseData) :
    aIntName( OUString::createFromAscii( rBaseData.pIntName ) ),
    pUINameID( rBaseData.pUINameID ),
    pDescrID( rBaseData.pDescrID ),
    nParamCount( rBaseData.nParamCount ),
    eCat( rBaseData.eCat ),
    bDouble( rBaseData.bDouble ),
    bWithOpt( rBaseData.bWithOpt )
{
    aCompList.push_back(OUString::createFromAscii(rBaseData.pCompListID[0]));
    aCompList.push_back(OUString::createFromAscii(rBaseData.pCompListID[1]));
}

sal_uInt16 ScaFuncData::GetStrIndex( sal_uInt16 nParam ) const
{
    if( !bWithOpt )
        nParam++;
    return (nParam > nParamCount) ? (nParamCount * 2) : (nParam * 2);
}

static void InitScaFuncDataList(ScaFuncDataList& rList)
{
    for (const auto & nIndex : pFuncDataArr)
        rList.emplace_back(nIndex);
}

//  entry points for service registration / instantiation

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scaddins_ScaDateAddIn_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ScaDateAddIn());
}


//  "normal" service implementation
ScaDateAddIn::ScaDateAddIn()
{
}

static const char*  pLang[] = { "de", "en" };
static const char*  pCoun[] = { "DE", "US" };
const sal_uInt32 nNumOfLoc = SAL_N_ELEMENTS( pLang );

void ScaDateAddIn::InitDefLocales()
{
    pDefLocales.reset(new lang::Locale[ nNumOfLoc ]);

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

    return (nIndex < nNumOfLoc) ? pDefLocales[ nIndex ] : aFuncLoc;
}

void ScaDateAddIn::InitData()
{
    aResLocale = Translate::Create("sca", LanguageTag(aFuncLoc));
    pFuncDataList.reset();

    pFuncDataList.reset(new ScaFuncDataList);
    InitScaFuncDataList(*pFuncDataList);

    if( pDefLocales )
    {
        pDefLocales.reset();
    }
}

OUString ScaDateAddIn::GetFuncDescrStr(const TranslateId* pResId, sal_uInt16 nStrIndex)
{
    return ScaResId(pResId[nStrIndex - 1]);
}

// XServiceName
OUString SAL_CALL ScaDateAddIn::getServiceName()
{
    // name of specific AddIn service
    return MY_SERVICE;
}

// XServiceInfo
OUString SAL_CALL ScaDateAddIn::getImplementationName()
{
    return MY_IMPLNAME;
}

sal_Bool SAL_CALL ScaDateAddIn::supportsService( const OUString& aServiceName )
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence< OUString > SAL_CALL ScaDateAddIn::getSupportedServiceNames()
{
    return { ADDIN_SERVICE, MY_SERVICE };
}

// XLocalizable
void SAL_CALL ScaDateAddIn::setLocale( const lang::Locale& eLocale )
{
    aFuncLoc = eLocale;
    InitData();     // change of locale invalidates resources!
}

lang::Locale SAL_CALL ScaDateAddIn::getLocale()
{
    return aFuncLoc;
}

OUString SAL_CALL ScaDateAddIn::getProgrammaticFuntionName( const OUString& )
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)
    return OUString();
}

OUString SAL_CALL ScaDateAddIn::getDisplayFunctionName( const OUString& aProgrammaticName )
{
    OUString aRet;

    auto fDataIt = std::find_if(pFuncDataList->begin(), pFuncDataList->end(),
                                FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() )
    {
        aRet = ScaResId(fDataIt->GetUINameID());
        if( fDataIt->IsDouble() )
            aRet += "_ADD";
    }
    else
    {
        aRet = "UNKNOWNFUNC_" + aProgrammaticName;
    }

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getFunctionDescription( const OUString& aProgrammaticName )
{
    OUString aRet;

    auto fDataIt = std::find_if(pFuncDataList->begin(), pFuncDataList->end(),
                                FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() )
        aRet = GetFuncDescrStr( fDataIt->GetDescrID(), 1 );

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getDisplayArgumentName(
        const OUString& aProgrammaticName, sal_Int32 nArgument )
{
    OUString aRet;

    auto fDataIt = std::find_if(pFuncDataList->begin(), pFuncDataList->end(),
                                FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = fDataIt->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( fDataIt->GetDescrID(), nStr );
        else
            aRet = "internal";
    }

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getArgumentDescription(
        const OUString& aProgrammaticName, sal_Int32 nArgument )
{
    OUString aRet;

    auto fDataIt = std::find_if(pFuncDataList->begin(), pFuncDataList->end(),
                                FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = fDataIt->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( fDataIt->GetDescrID(), nStr + 1 );
        else
            aRet = "for internal use only";
    }

    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getProgrammaticCategoryName(
        const OUString& aProgrammaticName )
{
    OUString aRet;

    auto fDataIt = std::find_if(pFuncDataList->begin(), pFuncDataList->end(),
                                FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() )
    {
        switch( fDataIt->GetCategory() )
        {
            case ScaCategory::DateTime:   aRet = "Date&Time";    break;
            case ScaCategory::Text:       aRet = "Text";         break;
            case ScaCategory::Finance:    aRet = "Financial";    break;
            case ScaCategory::Inf:        aRet = "Information";  break;
            case ScaCategory::Math:       aRet = "Mathematical"; break;
            case ScaCategory::Tech:       aRet = "Technical";    break;
        }
    }

    if( aRet.isEmpty() )
        aRet = "Add-In";
    return aRet;
}

OUString SAL_CALL ScaDateAddIn::getDisplayCategoryName(
        const OUString& aProgrammaticName )
{
    return getProgrammaticCategoryName( aProgrammaticName );
}

// XCompatibilityNames
uno::Sequence< sheet::LocalizedName > SAL_CALL ScaDateAddIn::getCompatibilityNames(
        const OUString& aProgrammaticName )
{
    auto fDataIt = std::find_if(pFuncDataList->begin(), pFuncDataList->end(),
                                FindScaFuncData( aProgrammaticName ) );
    if( fDataIt == pFuncDataList->end() )
        return uno::Sequence< sheet::LocalizedName >( 0 );

    const std::vector<OUString>& rStrList = fDataIt->GetCompNameList();
    sal_uInt32 nCount = rStrList.size();

    uno::Sequence< sheet::LocalizedName > aRet( nCount );
    sheet::LocalizedName* pArray = aRet.getArray();

    for( sal_uInt32 nIndex = 0; nIndex < nCount; nIndex++ )
        pArray[ nIndex ] = sheet::LocalizedName( GetLocale( nIndex ), rStrList.at( nIndex ) );

    return aRet;
}

namespace {

// auxiliary functions
bool IsLeapYear( sal_uInt16 nYear )
{
    return ((((nYear % 4) == 0) && ((nYear % 100) != 0)) || ((nYear % 400) == 0));
}

sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{
    static const sal_uInt16 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
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
    sal_Int32 nDays = (static_cast<sal_Int32>(nYear)-1) * 365;
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
 *
 * @throws lang::IllegalArgumentException
 */

void DaysToDate( sal_Int32 nDays,
                sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear )
{
    if( nDays < 0 )
        throw lang::IllegalArgumentException();

    sal_Int32   nTempDays;
    sal_Int32   i = 0;
    bool    bCalc;

    do
    {
        nTempDays = nDays;
        rYear = static_cast<sal_uInt16>((nTempDays / 365) - i);
        nTempDays -= (static_cast<sal_Int32>(rYear) -1) * 365;
        nTempDays -= (( rYear -1) / 4) - (( rYear -1) / 100) + ((rYear -1) / 400);
        bCalc = false;
        if ( nTempDays < 1 )
        {
            i++;
            bCalc = true;
        }
        else
        {
            if ( nTempDays > 365 )
            {
                if ( (nTempDays != 366) || !IsLeapYear( rYear ) )
                {
                    i--;
                    bCalc = true;
                }
            }
        }
    }
    while ( bCalc );

    rMonth = 1;
    while ( nTempDays > DaysInMonth( rMonth, rYear ) )
    {
        nTempDays -= DaysInMonth( rMonth, rYear );
        rMonth++;
    }
    rDay = static_cast<sal_uInt16>(nTempDays);
}

/**
 * Get the null date used by the spreadsheet document
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function returns this internal Date value for the document null date
 *
 * @throws uno::RuntimeException
 */
sal_Int32 GetNullDate( const uno::Reference< beans::XPropertySet >& xOptions )
{
    if (xOptions.is())
    {
        try
        {
            uno::Any aAny = xOptions->getPropertyValue( u"NullDate"_ustr );
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
 * mode 0 is just a simple division by 7 calculation.
 *
 * mode 1 calculates the difference by week adhering to ISO8601.
 *
 * The International Standard IS-8601 states that Monday is the first
 * day of the week. The Gregorian Calendar is used for all dates,
 * proleptic in case of dates before 1582-10-15.
 *
 * The (consecutive) week number of a date is
 * std::floor( (date + NullDate - 1), 7.0 ),
 * with weeks starting on Monday, and week 0
 * starting on Monday, 0001-01-01 Gregorian.
 *
 * Weeks(d2,d1,m) is defined as -Weeks(d1,d2,m).
 *
 */

sal_Int32 SAL_CALL ScaDateAddIn::getDiffWeeks(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nStartDate, sal_Int32 nEndDate,
        sal_Int32 nMode )
{
    if ( nMode  == 0 )
    {
        return ( nEndDate - nStartDate ) / 7;
    }
    else if ( nMode == 1 )
    {
        sal_Int32 nNullDate = GetNullDate( xOptions );
        sal_Int32 nDays1 = nStartDate + nNullDate - 1;
        sal_Int32 nDays2 = nEndDate + nNullDate - 1;

        return ( std::floor( nDays2 / 7.0 ) - std::floor( nDays1 / 7.0 ) );
    }
    else
        throw lang::IllegalArgumentException();
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
        sal_Int32 nMode )
{
    if (nMode != 0 && nMode != 1)
        throw lang::IllegalArgumentException();

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
        sal_Int32 nMode )
{
    if (nMode != 0 && nMode != 1)
        throw lang::IllegalArgumentException();

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
        sal_Int32 nDate )
{
    sal_Int32 nNullDate = GetNullDate( xOptions );
    sal_Int32 nDays = nDate + nNullDate;

    sal_uInt16 nDay, nMonth, nYear;
    DaysToDate(nDays,nDay,nMonth,nYear);

    return static_cast<sal_Int32>(IsLeapYear(nYear));
}

/**
 * Get the Number of Days in the month for a date
 */
sal_Int32 SAL_CALL ScaDateAddIn::getDaysInMonth(
        const uno::Reference<beans::XPropertySet>& xOptions,
        sal_Int32 nDate )
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
        sal_Int32 nDate )
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
 * and leap years that start on a Wednesday have 53 weeks
 *
 * The International Standard IS-8601 has decreed that Monday
 * shall be the first day of the week.
 *
 * A WeekDay can be calculated by subtracting 1 and calculating the rest of
 * a division by 7 from the internal date representation
 * which gives a 0 - 6 value for Monday - Sunday
 *
 * @see #IsLeapYear #WeekNumber
 */
sal_Int32 SAL_CALL ScaDateAddIn::getWeeksInYear(
        const uno::Reference< beans::XPropertySet >& xOptions,
        sal_Int32 nDate )
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
OUString SAL_CALL ScaDateAddIn::getRot13( const OUString& aSrcString )
{
    OUStringBuffer aBuffer( aSrcString );
    for( sal_Int32 nIndex = 0; nIndex < aBuffer.getLength(); nIndex++ )
    {
        sal_Unicode cChar = aBuffer[nIndex];
        if( (cChar >= 'a') && (cChar <= 'z'))
        {
            cChar += 13;
            if (cChar > 'z')
                cChar -= 26;
        }
        else if( (cChar >= 'A') && (cChar <= 'Z') )
        {
            cChar += 13;
            if (cChar > 'Z')
                cChar -= 26;
        }
        aBuffer[nIndex] = cChar;
    }
    return aBuffer.makeStringAndClear();
}

OUString ScaDateAddIn::ScaResId(TranslateId aId)
{
    return Translate::get(aId, aResLocale);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
