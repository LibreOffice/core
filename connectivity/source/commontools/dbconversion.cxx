/*************************************************************************
 *
 *  $RCSfile: dbconversion.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-09 08:46:09 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMNUPDATE_HPP_
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _INC_STDIO
#include <stdio.h>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif


#define MAX_DAYS    3636532

//.........................................................................
namespace dbtools
{
//.........................................................................


using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;

//------------------------------------------------------------------------------
Date DBTypeConversion::STANDARD_DB_DATE(1, 1, 1900);

//------------------------------------------------------------------------------
void DBTypeConversion::setValue(const Reference<XColumnUpdate>& xVariant,
                                const Reference<XNumberFormatter>& xFormatter,
                                const Date& rNullDate,
                                const ::rtl::OUString& rString,
                                sal_Int32 nKey,
                                sal_Int16 nFieldType,
                                sal_Int16 nKeyType) throw(::com::sun::star::lang::IllegalArgumentException)
{
    double fValue = 0;
    if (rString.len())
    {
            // Muss der String formatiert werden?
        sal_Int16 nTypeClass = nKeyType & ~NumberFormat::DEFINED;
        sal_Bool bTextFormat = nTypeClass == NumberFormat::TEXT;
        sal_Int32 nKeyToUse  = bTextFormat ? 0 : nKey;
        sal_Int16 nRealUsedTypeClass = nTypeClass;
            // bei einem Text-Format muessen wir dem Formatter etwas mehr Freiheiten einraeumen, sonst
            // wirft convertStringToNumber eine NotNumericException
        try
        {
            fValue = xFormatter->convertStringToNumber(nKeyToUse, rString);
            sal_Int32 nRealUsedKey = xFormatter->detectNumberFormat(0, rString);
            if (nRealUsedKey != nKeyToUse)
                nRealUsedTypeClass = getNumberFormatType(xFormatter, nRealUsedKey) & ~NumberFormat::DEFINED;

            // und noch eine Sonderbehandlung, diesmal fuer Prozent-Formate
            if ((NumberFormat::NUMBER == nRealUsedTypeClass) && (NumberFormat::PERCENT == nTypeClass))
            {   // die Formatierung soll eigentlich als Prozent erfolgen, aber der String stellt nur eine
                // einfache Nummer dar -> anpassen
                ::rtl::OUString sExpanded(rString);
                static ::rtl::OUString s_sPercentSymbol = ::rtl::OUString::createFromAscii("%");
                    // need a method to add a sal_Unicode to a string, 'til then we use a static string ...
                sExpanded += s_sPercentSymbol;
                fValue = xFormatter->convertStringToNumber(nKeyToUse, sExpanded);
            }

            switch (nRealUsedTypeClass)
            {
                case NumberFormat::DATE:
                case NumberFormat::DATETIME:
                    xVariant->updateDouble(toStandardDbDate(rNullDate, fValue));
                    break;
                case NumberFormat::TIME:
                case NumberFormat::CURRENCY:
                case NumberFormat::NUMBER:
                case NumberFormat::SCIENTIFIC:
                case NumberFormat::FRACTION:
                case NumberFormat::PERCENT:
                    xVariant->updateDouble(fValue);
                    break;
                default:
                    xVariant->updateString(rString);
            }
        }
        catch(...)
        {
            xVariant->updateString(rString);
        }
    }
    else
    {
        switch (nFieldType)
        {
            case ::com::sun::star::sdbc::DataType::CHAR:
            case ::com::sun::star::sdbc::DataType::VARCHAR:
            case ::com::sun::star::sdbc::DataType::LONGVARCHAR:
                xVariant->updateString(rString);
                break;
            default:
                xVariant->updateNull();
        }
    }
}

//------------------------------------------------------------------------------
void DBTypeConversion::setValue(const Reference<XColumnUpdate>& xVariant,
                                const Date& rNullDate,
                                const double& rValue,
                                sal_Int16 nKeyType) throw(::com::sun::star::lang::IllegalArgumentException)
{
    switch (nKeyType & ~NumberFormat::DEFINED)
    {
        case NumberFormat::DATE:
        case NumberFormat::DATETIME:
        //  case NumberFormat::TIME:
            xVariant->updateDouble(toStandardDbDate(rNullDate, rValue));
            break;
        default:
            xVariant->updateDouble(rValue);
    }
}

//------------------------------------------------------------------------------
double DBTypeConversion::getValue(const Reference<XColumn>& xVariant,
                                  const Date& rNullDate,
                                  sal_Int16 nKeyType)
{
    try
    {
        switch (nKeyType & ~NumberFormat::DEFINED)
        {
            case NumberFormat::DATE:
            case NumberFormat::DATETIME:
            //  case NumberFormat::TIME:
                return toNullDate(rNullDate, xVariant->getDouble());
            default:
                return xVariant->getDouble();
        }
    }
    catch(...)
    {
        return 0.0;
    }
}
//------------------------------------------------------------------------------
::rtl::OUString DBTypeConversion::getValue(const Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                                           const Reference<XNumberFormatter>& _xFormatter,
                                           const ::com::sun::star::lang::Locale& _rLocale,
                                           const Date& _rNullDate)
{
    sal_Int32 nKey;
    sal_Int16 nKeyType;

    OSL_ENSHURE(_xColumn.is() && _xFormatter.is(), "DBTypeConversion::getValue: invalid arg !");
    if (!_xColumn.is() || !_xFormatter.is())
        return ::rtl::OUString();

    try
    {
        _xColumn->getPropertyValue(::rtl::OUString::createFromAscii("FormatKey")) >>= nKey;
    }
    catch (...)
    {
    }

    if (!nKey)
    {
        Reference<XNumberFormats> xFormats( _xFormatter->getNumberFormatsSupplier()->getNumberFormats() );
        Reference<XNumberFormatTypes> xTypeList(_xFormatter->getNumberFormatsSupplier()->getNumberFormats(), UNO_QUERY);

        nKey = dbtools::getDefaultNumberFormat(_xColumn,
                                           Reference< XNumberFormatTypes > (xFormats, UNO_QUERY),
                                           _rLocale);

        nKeyType = getNumberFormatType(_xFormatter, nKey) & ~NumberFormat::DEFINED;
    }

    return DBTypeConversion::getValue(Reference< XColumn > (_xColumn, UNO_QUERY), _xFormatter, _rNullDate, nKey, nKeyType);
}

//------------------------------------------------------------------------------
::rtl::OUString DBTypeConversion::getValue(const Reference<XColumn>& xVariant,
                                   const Reference<XNumberFormatter>& xFormatter,
                                   const Date& rNullDate,
                                   sal_Int32 nKey,
                                   sal_Int16 nKeyType)
{
    ::rtl::OUString aString;
    if (xVariant.is())
    {
        try
        {
            switch (nKeyType & ~NumberFormat::DEFINED)
            {
                case NumberFormat::DATE:
                case NumberFormat::DATETIME:
                {
                    double fValue = xVariant->getDouble();
                    if (!xVariant->wasNull())
                        aString = xFormatter->convertNumberToString(nKey, toNullDate(rNullDate, fValue));
                }   break;
                case NumberFormat::TIME:
                case NumberFormat::NUMBER:
                case NumberFormat::SCIENTIFIC:
                case NumberFormat::FRACTION:
                case NumberFormat::PERCENT:
                {
                    double fValue = xVariant->getDouble();
                    if (!xVariant->wasNull())
                        aString = xFormatter->convertNumberToString(nKey, fValue);
                }   break;
                case NumberFormat::CURRENCY:
                {
                    double fValue = xVariant->getDouble();
                    if (!xVariant->wasNull())
                        aString = xFormatter->getInputString(nKey, fValue);
                }   break;
                case NumberFormat::TEXT:
                    aString = xFormatter->formatString(nKey, xVariant->getString());
                    break;
                default:
                    aString = xVariant->getString();
            }
        }
        catch(...)
        {
            aString = xVariant->getString();
        }
    }
    return aString;
}

//------------------------------------------------------------------------------
Date DBTypeConversion::toDate(sal_Int32 _nVal)
{
    Date aReturn;
    aReturn.Day = (sal_uInt16)(_nVal % 100);
    aReturn.Month = (sal_uInt16)((_nVal  / 100) % 100);
    aReturn.Year = (sal_uInt16)(_nVal  / 10000);
    return aReturn;
}

//------------------------------------------------------------------------------
Time DBTypeConversion::toTime(sal_Int32 _nVal)
{
    Time aReturn;
    aReturn.Hours = (sal_uInt16)(((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) / 1000000);
    aReturn.Minutes = (sal_uInt16)((((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) / 10000) % 100);
    aReturn.Seconds = (sal_uInt16)((((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) / 100) % 100);
    aReturn.HundredthSeconds = (sal_uInt16)(((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) % 100);
    return aReturn;
}

const double fMilliSecondsPerDay = 86400000.0;
//------------------------------------------------------------------------------
sal_Int32 DBTypeConversion::toINT32(const Date& rVal)
{
    return ((sal_Int32)(rVal.Day%100)) +
          (((sal_Int32)(rVal.Month%100))*100) +
          (((sal_Int32) rVal.Year%10000)*10000);
}

//------------------------------------------------------------------------------
sal_Int32 DBTypeConversion::toINT32(const Time& rVal)
{
    // Zeit normalisieren
    sal_Int32 nSeconds          = rVal.Seconds + rVal.HundredthSeconds / 100;
    sal_Int32 nHundredthSeconds = rVal.HundredthSeconds % 100;
    sal_Int32 nMinutes          = rVal.Minutes + nSeconds / 60;
    nSeconds                    = nSeconds % 60;
    sal_Int32 nHours            = rVal.Hours + nMinutes / 60;
    nMinutes                    = nMinutes % 60;

    // Zeit zusammenbauen
    return (sal_Int32)(nHundredthSeconds + (nSeconds*100) + (nMinutes*10000) + (nHours*1000000));
}

//------------------------------------------------------------------------------
sal_Int64 DBTypeConversion::toINT64(const DateTime& rVal)
{
    // Zeit normalisieren
    sal_Int32 nSeconds          = rVal.Seconds + rVal.HundredthSeconds / 100;
    sal_Int32 nHundredthSeconds = rVal.HundredthSeconds % 100;
    sal_Int32 nMinutes          = rVal.Minutes + nSeconds / 60;
    nSeconds                    = nSeconds % 60;
    sal_Int32 nHours            = rVal.Hours + nMinutes / 60;
    nMinutes                    = nMinutes % 60;

    // Zeit zusammenbauen
    sal_Int32 nTime = (sal_Int32)(nHundredthSeconds + (nSeconds*100) + (nMinutes*10000) + (nHours*1000000));
    sal_Int32 nDate = ((sal_Int32)(rVal.Day%100)) + (((sal_Int32)(rVal.Month%100))*100) + (((sal_Int32) rVal.Year%10000)*10000);
    sal_Int64 nRet;
    sal_setInt64(&nRet,nDate,nTime);
    return nRet;
}

//------------------------------------------------------------------------------
sal_Int32 DBTypeConversion::getMsFromTime(const Time& rVal)
{
    sal_Int16   nSign     = (toINT32(rVal) >= 0) ? +1 : -1;
    sal_Int32   nHour     = rVal.Hours;
    sal_Int32   nMin      = rVal.Minutes;
    sal_Int32   nSec      = rVal.Seconds;
    sal_Int32   n100Sec   = rVal.HundredthSeconds;

    return (((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10))*nSign);
}

//------------------------------------------------------------------------------
static sal_Int32 aDaysInMonth[12] = {   31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31 };

//------------------------------------------------------------------------------
static sal_Bool implIsLeapYear(sal_Int32 _nYear)
{
    return  (   (   ((_nYear % 4) == 0)
                &&  ((_nYear % 100) != 0)
                )
            )
            ||  ((_nYear % 400) == 0)
            ;
}

//------------------------------------------------------------------------------
static sal_Int32 implDaysInMonth(sal_Int32 _nMonth, sal_Int32 _nYear)
{
    if (_nMonth != 2)
        return aDaysInMonth[_nMonth-1];
    else
    {
        if (implIsLeapYear(_nYear))
            return aDaysInMonth[_nMonth-1] + 1;
        else
            return aDaysInMonth[_nMonth-1];
    }
}

//------------------------------------------------------------------------------
static sal_Int32 implRelativeToAbsoluteNull(const Date& _rDate)
{
    sal_Int32 nDays = 0;

    // ripped this code from the implementation of tools::Date
    sal_Int32 nNormalizedYear = _rDate.Year - 1;
    nDays = nNormalizedYear * 365;
    // leap years
    nDays += (nNormalizedYear / 4) - (nNormalizedYear / 100) + (nNormalizedYear / 400);

    for (sal_Int32 i = 1; i < _rDate.Month; ++i)
        nDays += implDaysInMonth(i, _rDate.Month);

    nDays += _rDate.Day;
    return nDays;
}
//------------------------------------------------------------------------------
static void implBuildFromRelative( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear)
{
    sal_Int32   nTempDays;
    sal_Int32   i = 0;
    sal_Bool    bCalc;

    do
    {
        nTempDays = nDays;
        rYear = ((nTempDays / 365) - i);
        nTempDays -= (rYear-1) * 365;
        nTempDays -= ((rYear-1) / 4) - ((rYear-1) / 100) + ((rYear-1) / 400);
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
                if ( (nTempDays != 366) || !implIsLeapYear( rYear ) )
                {
                    i--;
                    bCalc = sal_True;
                }
            }
        }
    }
    while ( bCalc );

    rMonth = 1;
    while ( nTempDays > implDaysInMonth( rMonth, rYear ) )
    {
        nTempDays -= implDaysInMonth( rMonth, rYear );
        rMonth++;
    }
    rDay = nTempDays;
}
//------------------------------------------------------------------------------
sal_Int32 DBTypeConversion::toDays(const Date& _rVal, const Date& _rNullDate)
{
    return implRelativeToAbsoluteNull(_rVal) - implRelativeToAbsoluteNull(_rNullDate);
}

//------------------------------------------------------------------------------
double DBTypeConversion::toDouble(const Date& rVal, const Date& _rNullDate)
{
    return (double)toDays(rVal, _rNullDate);
}

//------------------------------------------------------------------------------
double DBTypeConversion::toDouble(const Time& rVal)
{
    return (double)getMsFromTime(rVal) / fMilliSecondsPerDay;
}

//------------------------------------------------------------------------------
double DBTypeConversion::toDouble(const DateTime& _rVal, const Date& _rNullDate)
{
    sal_Int64   nTime     = toDays(Date(_rVal.Day, _rVal.Month, _rVal.Year), _rNullDate);
    sal_Int16   nSign     = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = _rVal.Hours;
    sal_Int32   nMin      = _rVal.Minutes;
    sal_Int32   nSec      = _rVal.Seconds;
    sal_Int32   n100Sec   = _rVal.HundredthSeconds;

    double nVal = (((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10))*nSign);

    return ((double)nTime) + (nVal * (1/fMilliSecondsPerDay));
}
// -------------------------------------------------------------------------
static void addDays(sal_Int32 nDays, Date& _rDate)
{
    sal_Int32   nTempDays = implRelativeToAbsoluteNull( _rDate );

    nTempDays += nDays;
    if ( nTempDays > MAX_DAYS )
    {
        _rDate.Day      = 31;
        _rDate.Month    = 12;
        _rDate.Year     = 9999;
    }
    else if ( nTempDays <= 0 )
    {
        _rDate.Day      = 1;
        _rDate.Month    = 1;
        _rDate.Year     = 00;
    }
    else
        implBuildFromRelative( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
}
// -----------------------------------------------------------------------
static void subDays( sal_Int32 nDays, Date& _rDate )
{
    sal_Int32   nTempDays = implRelativeToAbsoluteNull( _rDate );

    nTempDays -= nDays;
    if ( nTempDays > MAX_DAYS )
    {
        _rDate.Day      = 31;
        _rDate.Month    = 12;
        _rDate.Year     = 9999;
    }
    else if ( nTempDays <= 0 )
    {
        _rDate.Day      = 1;
        _rDate.Month    = 1;
        _rDate.Year     = 00;
    }
    else
        implBuildFromRelative( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
}
// -------------------------------------------------------------------------
Date DBTypeConversion::toDate(double dVal, const Date& _rNullDate)
{
    Date aRet = _rNullDate;

    if (dVal >= 0)
        addDays((sal_Int32)dVal,aRet);
    else
        subDays((sal_uInt32)(-dVal),aRet);
        //  x -= (sal_uInt32)(-nDays);

    return aRet;
}
// -------------------------------------------------------------------------
Time DBTypeConversion::toTime(double dVal)
{
    sal_Int32 nDays     = (sal_Int32)dVal;
    sal_Int32 nMS = sal_Int32((dVal - (double)nDays) * fMilliSecondsPerDay + 0.5);

    sal_uInt16 nSign;
    if ( nMS < 0 )
    {
        nMS *= -1;
        nSign = -1;
    }
    else
        nSign = 1;

    Time xRet;
    // Zeit normalisieren
    xRet.HundredthSeconds   = nMS/10;
    xRet.Seconds            = xRet.HundredthSeconds / 100;
    xRet.HundredthSeconds   = xRet.HundredthSeconds % 100;
    xRet.Minutes            = xRet.Seconds / 60;
    xRet.Seconds            = xRet.Seconds % 60;
    xRet.Hours              = xRet.Minutes / 60;
    xRet.Minutes            = xRet.Minutes % 60;

    // Zeit zusammenbauen
    sal_Int32 nTime = (sal_Int32)(xRet.HundredthSeconds + (xRet.Seconds*100) + (xRet.Minutes*10000) + (xRet.Hours*1000000)) * nSign;

    if(nTime < 0)
    {
        xRet.HundredthSeconds   = 99;
        xRet.Minutes            = 59;
        xRet.Seconds            = 59;
        xRet.Hours              = 23;
    }
    return xRet;

}
//------------------------------------------------------------------------------
DateTime DBTypeConversion::toDateTime(double dVal, const Date& _rNullDate)
{
    Date aDate = toDate(dVal, _rNullDate);
    Time aTime = toTime(dVal);

    DateTime xRet;

    xRet.Day                = aDate.Day;
    xRet.Month              = aDate.Month;
    xRet.Year               = aDate.Year;

    xRet.HundredthSeconds   = aTime.HundredthSeconds;
    xRet.Minutes            = aTime.Minutes;
    xRet.Seconds            = aTime.Seconds;
    xRet.Hours              = aTime.Hours;


    return xRet;
}
//------------------------------------------------------------------------------
Date DBTypeConversion::getNULLDate(const Reference< XNumberFormatsSupplier > &xSupplier)
{
    OSL_ENSHURE(xSupplier.is(), "getNULLDate : the formatter doesn't implement a supplier !");
    if (xSupplier.is())
    {
        try
        {
            // get the null date
            Date aDate;
            xSupplier->getNumberFormatSettings()->getPropertyValue(::rtl::OUString::createFromAscii("NullDate")) >>= aDate;
            return aDate;
        }
        catch ( ... )
        {
        }
    }

    return STANDARD_DB_DATE;
}
//------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toDateString(const Date& rDate)
{
    sal_Char s[11];
    sprintf(s,"%04d-%02d-%02d",
                (int)rDate.Year,
                (int)rDate.Month,
                (int)rDate.Day);
    s[10] = 0;
    return ::rtl::OUString::createFromAscii(s);
}
//------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toTimeString(const Time& rTime)
{
    sal_Char s[9];
    sprintf(s,"%02d:%02d:%02d",
            (int)rTime.Hours,
            (int)rTime.Minutes,
            (int)rTime.Seconds);
    s[8] = 0;
    return ::rtl::OUString::createFromAscii(s);
}

//------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toDateTimeString(const DateTime& _rDateTime)
{
    Date aDate(_rDateTime.Day,_rDateTime.Month,_rDateTime.Year);
    ::rtl::OUString aTemp(toDateString(aDate));
    aTemp += ::rtl::OUString::createFromAscii(" ");
    Time aTime(0,_rDateTime.Seconds,_rDateTime.Minutes,_rDateTime.Hours);
    aTemp += toTimeString(aTime);
    return  aTemp;
}
// -----------------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toSQLString(sal_Int32 eType, const Any& _rVal, sal_Bool bQuote,
                                              const Reference< XTypeConverter >&  _rxTypeConverter)
{
    ::rtl::OUString aRet;
    if (_rVal.hasValue())
    {
        try
        {
            switch (eType)
            {
                case DataType::INTEGER:
                case DataType::BIT:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                    if (_rVal.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_BOOLEAN)
                    {
                        if (::cppu::any2bool(_rVal))
                            aRet = ::rtl::OUString::createFromAscii("1");
                        else
                            aRet = ::rtl::OUString::createFromAscii("0");
                    }
                    else
                        _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aRet;
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                    if (bQuote)
                        aRet += ::rtl::OUString::createFromAscii("'");
                    {
                        ::rtl::OUString aTemp;
                        _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aTemp;
                        aRet += aTemp;
                    }
                    if (bQuote)
                        aRet += ::rtl::OUString::createFromAscii("'");
                    break;
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aRet;
                    break;
                case DataType::TIMESTAMP:
                {
                    DateTime aDateTime;

                    // check if this is really a timestamp or only a date
                    if ((_rVal >>= aDateTime) &&
                        (aDateTime.Hours || aDateTime.Minutes || aDateTime.Seconds || aDateTime.HundredthSeconds))
                    {
                        if (bQuote) aRet += ::rtl::OUString::createFromAscii("{TS '");
                        aRet += DBTypeConversion::toDateTimeString(aDateTime);
                        if (bQuote) aRet += ::rtl::OUString::createFromAscii("'}");
                        break;
                    }
                    // else continue
                }
                case DataType::DATE:
                {
                    Date aDate;
                    sal_Bool bRet = _rVal >>= aDate;
                    OSL_ENSHURE(bRet,"DBTypeConversion::toSQLString: _rVal is not date!");
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("{D '");
                    aRet += DBTypeConversion::toDateString(aDate);;
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("'}");
                }   break;
                case DataType::TIME:
                {
                    Time aTime;
                    sal_Bool bRet = _rVal >>= aTime;
                    OSL_ENSHURE(bRet,"DBTypeConversion::toSQLString: _rVal is not time!");
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("{T '");
                    aRet += DBTypeConversion::toTimeString(aTime);
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("'}");
                } break;
                default:
                    _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aRet;
            }
        }
        catch ( ... )
        {
            OSL_ENSHURE(0,"TypeConversion Error");
        }
    }
    else
        aRet = ::rtl::OUString::createFromAscii(" NULL ");
    return aRet;
}
// -----------------------------------------------------------------------------


//.........................................................................
}   // namespace dbtools
//.........................................................................


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2000/11/08 09:28:45  oj
 *  forget assignment of return value
 *
 *  Revision 1.5  2000/10/27 07:04:22  fs
 *  corrected the starutil namespace
 *
 *  Revision 1.4  2000/10/27 07:01:07  fs
 *  new: toDate(sal_Int32) / toTime(sal_Int32)
 *
 *  Revision 1.3  2000/10/24 15:00:32  oj
 *  make strings unique for lib's
 *
 *  Revision 1.2  2000/10/19 11:46:15  oj
 *  remove tools from dbtools
 *
 *  Revision 1.1  2000/10/05 08:50:32  fs
 *  moved the files from unotools to here
 *
 *
 *  Revision 1.0 29.09.00 08:17:18  fs
 ************************************************************************/

