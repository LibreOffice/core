/*************************************************************************
 *
 *  $RCSfile: datefunc.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-16 14:31:06 $
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

//------------------------------------------------------------------
//
// date functions add in
//
//------------------------------------------------------------------

#include "datefunc.hxx"

#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/Date.hpp>

using namespace com::sun::star;
using namespace rtl;

//------------------------------------------------------------------

#define ADDIN_SERVICE       "com.sun.star.sheet.AddIn"
#define MY_SERVICE          "com.sun.star.sheet.addin.DateFunctions"
#define MY_IMPLNAME         "com.sun.star.sheet.addin.DateFunctionsImpl"

//------------------------------------------------------------------
//
//  entry points for service registration / instantiation
//
//------------------------------------------------------------------

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, registry::XRegistryKey * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            OUString aImpl = OUString::createFromAscii( "/" );
            aImpl += DateFunctionAddIn::getImplementationName_Static();
            aImpl += OUString::createFromAscii( "/UNO/SERVICES" );

            uno::Reference<registry::XRegistryKey> xNewKey(
                reinterpret_cast<registry::XRegistryKey*>(pRegistryKey)->createKey(aImpl) );

            uno::Sequence <OUString> aSequ = DateFunctionAddIn::getSupportedServiceNames_Static();
            const OUString * pArray = aSequ.getConstArray();
            for( sal_Int32 i = 0; i < aSequ.getLength(); i++ )
                xNewKey->createKey( pArray[i] );

            return sal_True;
        }
        catch (registry::InvalidRegistryException&)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void* pRet = 0;

    if ( pServiceManager &&
            OUString::createFromAscii(pImplName) == DateFunctionAddIn::getImplementationName_Static() )
    {
        uno::Reference<lang::XSingleServiceFactory> xFactory( cppu::createOneInstanceFactory(
                reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
                DateFunctionAddIn::getImplementationName_Static(),
                DateFunctionAddIn_CreateInstance,
                DateFunctionAddIn::getSupportedServiceNames_Static() ) );

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

DateFunctionAddIn::DateFunctionAddIn()
{
}

DateFunctionAddIn::~DateFunctionAddIn()
{
}

OUString DateFunctionAddIn::getImplementationName_Static()
{
    return OUString::createFromAscii( MY_IMPLNAME );
}

uno::Sequence<OUString> DateFunctionAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString::createFromAscii( ADDIN_SERVICE );
    pArray[1] = OUString::createFromAscii( MY_SERVICE );
    return aRet;
}

uno::Reference<uno::XInterface> SAL_CALL DateFunctionAddIn_CreateInstance(
        const uno::Reference<lang::XMultiServiceFactory>& )
{
    static uno::Reference<uno::XInterface> xInst = (cppu::OWeakObject*)new DateFunctionAddIn();
    return xInst;
}

// XServiceName

OUString SAL_CALL DateFunctionAddIn::getServiceName() throw(uno::RuntimeException)
{
    // name of specific AddIn service
    return OUString::createFromAscii( MY_SERVICE );
}

// XServiceInfo

OUString SAL_CALL DateFunctionAddIn::getImplementationName() throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL DateFunctionAddIn::supportsService( const OUString& aServiceName )
                                                        throw(uno::RuntimeException)
{
    return aServiceName.compareToAscii( ADDIN_SERVICE ) == 0 ||
           aServiceName.compareToAscii( MY_SERVICE ) == 0;
}

uno::Sequence<OUString> SAL_CALL DateFunctionAddIn::getSupportedServiceNames()
                                                        throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// XLocalizable

void SAL_CALL DateFunctionAddIn::setLocale( const lang::Locale& eLocale ) throw(uno::RuntimeException)
{
    aFuncLoc = eLocale;
}

lang::Locale SAL_CALL DateFunctionAddIn::getLocale() throw(uno::RuntimeException)
{
    return aFuncLoc;
}

//------------------------------------------------------------------
//
//  function descriptions start here
//
//------------------------------------------------------------------

// auxiliary functions

enum DateFunc
{
    DF_GETDIFFWEEKS,
    DF_GETDIFFMONTHS,
    DF_GETDIFFYEARS,
    DF_ISLEAPYEAR,
    DF_GETDAYSINMONTH,
    DF_GETDAYSINYEAR,
    DF_GETWEEKSINYEAR,
    DF_INVALID,
    DF_COUNT = DF_INVALID
};

DateFunc GetFunction( const OUString& aProgrammaticName )
{
    static const sal_Char* aNames[] =
    {
        "getDiffWeeks",
        "getDiffMonths",
        "getDiffYears",
        "isLeapYear",
        "getDaysInMonth",
        "getDaysInYear",
        "getWeeksInYear",
        NULL
    };

    for ( sal_Int16 i=0; i<DF_COUNT; i++ )
        if ( aProgrammaticName.compareToAscii(aNames[i]) == 0 )
            return (DateFunc) i;
    return DF_INVALID;
}

// XAddIn

OUString SAL_CALL DateFunctionAddIn::getProgrammaticFuntionName( const OUString& aDisplayName )
                                                throw(uno::RuntimeException)
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)

    return OUString();
}

OUString SAL_CALL DateFunctionAddIn::getDisplayFunctionName( const OUString& aProgrammaticName )
                                                throw(uno::RuntimeException)
{
    //! allow different languages
    sal_Bool bGerman = ( aFuncLoc.Language.equalsIgnoreCase( OUString::createFromAscii("DE") ) );

    OUString aRet;
    switch ( GetFunction(aProgrammaticName) )
    {
        case DF_GETDIFFWEEKS:
            aRet = OUString::createFromAscii( bGerman ? "Wochen" : "Weeks" );
            break;
        case DF_GETDIFFMONTHS:
            aRet = OUString::createFromAscii( bGerman ? "Monate" : "Months" );
            break;
        case DF_GETDIFFYEARS:
            aRet = OUString::createFromAscii( bGerman ? "Jahre" : "Years" );
            break;
        case DF_ISLEAPYEAR:
            aRet = OUString::createFromAscii( bGerman ? "IstSchaltJahr" : "IsLeapYear" );
            break;
        case DF_GETDAYSINMONTH:
            aRet = OUString::createFromAscii( bGerman ? "TageImMonat" : "DaysInMonth" );
            break;
        case DF_GETDAYSINYEAR:
            aRet = OUString::createFromAscii( bGerman ? "TageImJahr" : "DaysInYear" );
            break;
        case DF_GETWEEKSINYEAR:
            aRet = OUString::createFromAscii( bGerman ? "WochenImJahr" : "WeeksInYear" );
            break;
    }
    return aRet;
}

OUString SAL_CALL DateFunctionAddIn::getFunctionDescription( const OUString& aProgrammaticName )
                                                throw(uno::RuntimeException)
{
    //! return translated strings

    OUString aRet;
    switch ( GetFunction(aProgrammaticName) )
    {
        case DF_GETDIFFWEEKS:
            aRet = OUString::createFromAscii( "Returns the difference in weeks between two dates" );
            break;
        case DF_GETDIFFMONTHS:
            aRet = OUString::createFromAscii( "Determines the number of months between two dates" );
            break;
        case DF_GETDIFFYEARS:
            aRet = OUString::createFromAscii( "Returns the difference in years between two dates" );
            break;
        case DF_ISLEAPYEAR:
            aRet = OUString::createFromAscii( "Returns 1 (TRUE) if a leap year is used, otherwise 0 (FALSE) is returned" );
            break;
        case DF_GETDAYSINMONTH:
            aRet = OUString::createFromAscii( "Returns the number of days in the month in relation to the date entered" );
            break;
        case DF_GETDAYSINYEAR:
            aRet = OUString::createFromAscii( "Returns the number of days in a year in relation to the date entered" );
            break;
        case DF_GETWEEKSINYEAR:
            aRet = OUString::createFromAscii( "Returns the number of weeks in the year in relation to a date" );
            break;
    }
    return aRet;
}

OUString SAL_CALL DateFunctionAddIn::getDisplayArgumentName(
                                        const OUString& aProgrammaticFunctionName,
                                        sal_Int32 nArgument ) throw(uno::RuntimeException)
{
    //! return translated strings

    OUString aRet;
    switch ( GetFunction(aProgrammaticFunctionName) )
    {
        case DF_GETDIFFWEEKS:
        case DF_GETDIFFMONTHS:
        case DF_GETDIFFYEARS:
            switch (nArgument)
            {
                case 0:
                    aRet = OUString::createFromAscii( "(internal)" );
                    break;
                case 1:
                    aRet = OUString::createFromAscii( "Start date" );
                    break;
                case 2:
                    aRet = OUString::createFromAscii( "End date" );
                    break;
                case 3:
                    aRet = OUString::createFromAscii( "Mode" );
                    break;
            }
            break;
        case DF_ISLEAPYEAR:
        case DF_GETDAYSINMONTH:
        case DF_GETDAYSINYEAR:
        case DF_GETWEEKSINYEAR:
            switch (nArgument)
            {
                case 0:
                    aRet = OUString::createFromAscii( "(internal)" );
                    break;
                case 1:
                    aRet = OUString::createFromAscii( "Date" );
                    break;
            }
            break;
    }
    return aRet;
}

OUString SAL_CALL DateFunctionAddIn::getArgumentDescription(
                                        const OUString& aProgrammaticFunctionName,
                                        sal_Int32 nArgument ) throw(uno::RuntimeException)
{
    //! return translated strings

    OUString aRet;
    switch ( GetFunction(aProgrammaticFunctionName) )
    {
        case DF_GETDIFFWEEKS:
            switch (nArgument)
            {
                case 0:
                    aRet = OUString::createFromAscii( "(internal)" );
                    break;
                case 1:
                    aRet = OUString::createFromAscii( "The start date for calculating the difference weeks" );
                    break;
                case 2:
                    aRet = OUString::createFromAscii( "The end date for calculating the difference in weeks" );
                    break;
                case 3:
                    aRet = OUString::createFromAscii( "Type of difference calculation: mode=0 means the interval, mode=1 means calendar weeks" );
                    break;
            }
            break;
        case DF_GETDIFFMONTHS:
            switch (nArgument)
            {
                case 0:
                    aRet = OUString::createFromAscii( "(internal)" );
                    break;
                case 1:
                    aRet = OUString::createFromAscii( "The start date for calculating the difference in months" );
                    break;
                case 2:
                    aRet = OUString::createFromAscii( "The end date for calculating the difference in months" );
                    break;
                case 3:
                    aRet = OUString::createFromAscii( "Type of difference calculation: Mode = 0 means interval, mode = 1 means in calendar months" );
                    break;
            }
            break;
        case DF_GETDIFFYEARS:
            switch (nArgument)
            {
                case 0:
                    aRet = OUString::createFromAscii( "(internal)" );
                    break;
                case 1:
                    aRet = OUString::createFromAscii( "The start date for calculating the difference in years" );
                    break;
                case 2:
                    aRet = OUString::createFromAscii( "The end date for calculating the difference in years" );
                    break;
                case 3:
                    aRet = OUString::createFromAscii( "Type of difference calculation: Mode=0 means interval, mode=1 means in calendar years" );
                    break;
            }
            break;
        case DF_ISLEAPYEAR:
        case DF_GETDAYSINMONTH:
        case DF_GETDAYSINYEAR:
        case DF_GETWEEKSINYEAR:
            switch (nArgument)
            {
                case 0:
                    aRet = OUString::createFromAscii( "(internal)" );
                    break;
                case 1:
                    aRet = OUString::createFromAscii( "Internal number of the date" );
                    break;
            }
            break;
    }
    return aRet;
}

OUString SAL_CALL DateFunctionAddIn::getProgrammaticCategoryName(
                                        const OUString& aProgrammaticFunctionName )
                                        throw(uno::RuntimeException)
{
    //  return non-translated strings

    return OUString::createFromAscii("Add-In");
}

OUString SAL_CALL DateFunctionAddIn::getDisplayCategoryName(
                                        const OUString& aProgrammaticFunctionName )
                                        throw(uno::RuntimeException)
{
    //  return translated strings, not used for predefined categories

    return OUString::createFromAscii("Add-In");
}

//------------------------------------------------------------------
//
//  function implementation starts here
//
//------------------------------------------------------------------

// auxiliary functions

sal_Bool IsLeapYear( sal_uInt16 nYear )
{
    return (((nYear % 4) == 0) && ((nYear % 100) != 0) || ((nYear % 400) == 0));
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
 *
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
 *
 */

void DaysToDate( sal_Int32 nDays,
                 sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear )
{
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

sal_Int32 GetNullDate( const uno::Reference<beans::XPropertySet>& xOptions )
                        throw(uno::RuntimeException)
{
    if (xOptions.is())
    {
        try
        {
            uno::Any aAny = xOptions->getPropertyValue(
                                        OUString::createFromAscii( "NullDate" ) );
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

sal_Int32 SAL_CALL DateFunctionAddIn::getDiffWeeks(
                        const uno::Reference<beans::XPropertySet>& xOptions,
                        sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                    throw(uno::RuntimeException)
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
 *
 */

sal_Int32 SAL_CALL DateFunctionAddIn::getDiffMonths(
                        const uno::Reference<beans::XPropertySet>& xOptions,
                        sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                    throw(uno::RuntimeException)
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
 *
 */

sal_Int32 SAL_CALL DateFunctionAddIn::getDiffYears(
                        const uno::Reference<beans::XPropertySet>& xOptions,
                        sal_Int32 nStartDate, sal_Int32 nEndDate, sal_Int32 nMode )
                    throw(uno::RuntimeException)
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
 *
 */

sal_Int32 SAL_CALL DateFunctionAddIn::isLeapYear(
                        const uno::Reference<beans::XPropertySet>& xOptions,
                        sal_Int32 nDate )
                    throw(uno::RuntimeException)
{
    sal_Int32 nNullDate = GetNullDate( xOptions );
    sal_Int32 nDays = nDate + nNullDate;

    sal_uInt16 nDay, nMonth, nYear;
    DaysToDate(nDays,nDay,nMonth,nYear);

    return (sal_Int32)IsLeapYear(nYear);
}

/**
 * Get the Number of Days in the month for a date
 *
 */

sal_Int32 SAL_CALL DateFunctionAddIn::getDaysInMonth(
                        const uno::Reference<beans::XPropertySet>& xOptions,
                        sal_Int32 nDate )
                    throw(uno::RuntimeException)
{
    sal_Int32 nNullDate = GetNullDate( xOptions );
    sal_Int32 nDays = nDate + nNullDate;

    sal_uInt16 nDay, nMonth, nYear;
    DaysToDate(nDays,nDay,nMonth,nYear);

    return DaysInMonth( nMonth, nYear );
}

/**
 * Get number of days in the year of a date specified
 *
 */

sal_Int32 SAL_CALL DateFunctionAddIn::getDaysInYear(
                        const uno::Reference<beans::XPropertySet>& xOptions,
                        sal_Int32 nDate )
                    throw(uno::RuntimeException)
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
 *
 */

sal_Int32 SAL_CALL DateFunctionAddIn::getWeeksInYear(
                        const uno::Reference<beans::XPropertySet>& xOptions,
                        sal_Int32 nDate )
                    throw(uno::RuntimeException)
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

//------------------------------------------------------------------

