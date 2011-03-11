/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include "sax/tools/converter.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace ::com::sun::star::i18n;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace sax {

static const sal_Char* gpsMM = "mm";
static const sal_Char* gpsCM = "cm";
static const sal_Char* gpsPT = "pt";
static const sal_Char* gpsINCH = "in";
static const sal_Char* gpsPC = "pc";

const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 11;
const sal_Int8 XML_MAXDIGITSCOUNT_DATETIME = 6;
#define XML_NULLDATE "NullDate"

/** convert string to measure using optional min and max values*/
bool Converter::convertMeasure( sal_Int32& rValue,
                                const OUString& rString,
                                sal_Int16 nTargetUnit /* = MeasureUnit::MM_100TH */,
                                sal_Int32 nMin /* = SAL_MIN_INT32 */,
                                sal_Int32 nMax /* = SAL_MAX_INT32 */ )
{
    bool bNeg = false;
    double nVal = 0;

    sal_Int32 nPos = 0;
    sal_Int32 nLen = rString.getLength();

    // skip white space
    while( (nPos < nLen) && (rString[nPos] <= sal_Unicode(' ')) )
        nPos++;

    if( nPos < nLen && sal_Unicode('-') == rString[nPos] )
    {
        bNeg = true;
        nPos++;
    }

    // get number
    while( nPos < nLen &&
           sal_Unicode('0') <= rString[nPos] &&
           sal_Unicode('9') >= rString[nPos] )
    {
        // TODO: check overflow!
        nVal *= 10;
        nVal += (rString[nPos] - sal_Unicode('0'));
        nPos++;
    }
    double nDiv = 1.;
    if( nPos < nLen && sal_Unicode('.') == rString[nPos] )
    {
        nPos++;

        while( nPos < nLen &&
               sal_Unicode('0') <= rString[nPos] &&
               sal_Unicode('9') >= rString[nPos] )
        {
            // TODO: check overflow!
            nDiv *= 10;
            nVal += ( ((double)(rString[nPos] - sal_Unicode('0'))) / nDiv );
            nPos++;
        }
    }

    // skip white space
    while( (nPos < nLen) && (rString[nPos] <= sal_Unicode(' ')) )
        nPos++;

    if( nPos < nLen )
    {

        if( MeasureUnit::PERCENT == nTargetUnit )
        {
            if( sal_Unicode('%') != rString[nPos] )
                return false;
        }
        else if( MeasureUnit::PIXEL == nTargetUnit )
        {
            if( nPos + 1 >= nLen ||
                (sal_Unicode('p') != rString[nPos] &&
                 sal_Unicode('P') != rString[nPos])||
                (sal_Unicode('x') != rString[nPos+1] &&
                 sal_Unicode('X') != rString[nPos+1]) )
                return false;
        }
        else
        {
            OSL_ENSURE( MeasureUnit::TWIP == nTargetUnit || MeasureUnit::POINT == nTargetUnit ||
                        MeasureUnit::MM_100TH == nTargetUnit || MeasureUnit::MM_10TH == nTargetUnit, "unit is not supported");
            const sal_Char *aCmpsL[2] = { 0, 0 };
            const sal_Char *aCmpsU[2] = { 0, 0 };
            double aScales[2] = { 1., 1. };

            if( MeasureUnit::TWIP == nTargetUnit )
            {
                switch( rString[nPos] )
                {
                case sal_Unicode('c'):
                case sal_Unicode('C'):
                    aCmpsL[0] = "cm";
                    aCmpsU[0] = "CM";
                    aScales[0] = (72.*20.)/2.54; // twip
                    break;
                case sal_Unicode('i'):
                case sal_Unicode('I'):
                    aCmpsL[0] = "in";
                    aCmpsU[0] = "IN";
                    aScales[0] = 72.*20.; // twip
                    break;
                case sal_Unicode('m'):
                case sal_Unicode('M'):
                    aCmpsL[0] = "mm";
                    aCmpsU[0] = "MM";
                    aScales[0] = (72.*20.)/25.4; // twip
                    break;
                case sal_Unicode('p'):
                case sal_Unicode('P'):
                    aCmpsL[0] = "pt";
                    aCmpsU[0] = "PT";
                    aScales[0] = 20.; // twip

                    aCmpsL[1] = "pc";
                    aCmpsU[1] = "PC";
                    aScales[1] = 12.*20.; // twip
                    break;
                }
            }
            else if( MeasureUnit::MM_100TH == nTargetUnit || MeasureUnit::MM_10TH == nTargetUnit )
            {
                double nScaleFactor = (MeasureUnit::MM_100TH == nTargetUnit) ? 100.0 : 10.0;
                switch( rString[nPos] )
                {
                case sal_Unicode('c'):
                case sal_Unicode('C'):
                    aCmpsL[0] = "cm";
                    aCmpsU[0] = "CM";
                    aScales[0] = 10.0 * nScaleFactor; // mm/100
                    break;
                case sal_Unicode('i'):
                case sal_Unicode('I'):
                    aCmpsL[0] = "in";
                    aCmpsU[0] = "IN";
                    aScales[0] = 1000.*2.54; // mm/100
                    break;
                case sal_Unicode('m'):
                case sal_Unicode('M'):
                    aCmpsL[0] = "mm";
                    aCmpsU[0] = "MM";
                    aScales[0] = 1.0 * nScaleFactor; // mm/100
                    break;
                case sal_Unicode('p'):
                case sal_Unicode('P'):
                    aCmpsL[0] = "pt";
                    aCmpsU[0] = "PT";
                    aScales[0] = (10.0 * nScaleFactor*2.54)/72.; // mm/100

                    aCmpsL[1] = "pc";
                    aCmpsU[1] = "PC";
                    aScales[1] = (10.0 * nScaleFactor*2.54)/12.; // mm/100
                    break;
                }
            }
            else if( MeasureUnit::POINT == nTargetUnit )
            {
                if( rString[nPos] == 'p' || rString[nPos] == 'P' )
                {
                    aCmpsL[0] = "pt";
                    aCmpsU[0] = "PT";
                    aScales[0] = 1;
                }
            }

            if( aCmpsL[0] == NULL )
                return false;

            double nScale = 0.;
            for( sal_uInt16 i= 0; i < 2; i++ )
            {
                const sal_Char *pL = aCmpsL[i];
                if( pL )
                {
                    const sal_Char *pU = aCmpsU[i];
                    while( nPos < nLen && *pL )
                    {
                        sal_Unicode c = rString[nPos];
                        if( c != *pL && c != *pU )
                            break;
                        pL++;
                        pU++;
                        nPos++;
                    }
                    if( !*pL && (nPos == nLen || ' ' == rString[nPos]) )
                    {
                        nScale = aScales[i];
                        break;
                    }
                }
            }

            if( 0. == nScale )
                return false;

            // TODO: check overflow
            if( nScale != 1. )
                nVal *= nScale;
        }
    }

    nVal += .5;
    if( bNeg )
        nVal = -nVal;

    if( nVal <= (double)nMin )
        rValue = nMin;
    else if( nVal >= (double)nMax )
        rValue = nMax;
    else
        rValue = (sal_Int32)nVal;

    return true;
}

/** convert measure in given unit to string with given unit */
void Converter::convertMeasure( OUStringBuffer& rBuffer,
                                sal_Int32 nMeasure,
                                sal_Int16 nSourceUnit /* = MeasureUnit::MM_100TH */,
                                sal_Int16 nTargetUnit /* = MeasureUnit::INCH */  )
{
    OSL_ENSURE( false, "Converter::convertMeasure - not implemented, tools/BigInt needs replacement" );
    (void)rBuffer;
    (void)nMeasure;
    (void)nSourceUnit;
    (void)nTargetUnit;
}

static const OUString& getTrueString()
{
    static const OUString sTrue( RTL_CONSTASCII_USTRINGPARAM( "true" ) );
    return sTrue;
}

static const OUString& getFalseString()
{
    static const OUString sFalse( RTL_CONSTASCII_USTRINGPARAM( "false" ) );
    return sFalse;
}

/** convert string to boolean */
bool Converter::convertBool( bool& rBool, const OUString& rString )
{
    rBool = rString == getTrueString();

    return rBool || (rString == getFalseString());
}

/** convert boolean to string */
void Converter::convertBool( OUStringBuffer& rBuffer, bool bValue )
{
    rBuffer.append( bValue ? getTrueString() : getFalseString() );
}

/** convert string to percent */
bool Converter::convertPercent( sal_Int32& rPercent, const OUString& rString )
{
    return convertMeasure( rPercent, rString, MeasureUnit::PERCENT );
}

/** convert percent to string */
void Converter::convertPercent( OUStringBuffer& rBuffer, sal_Int32 nValue )
{
    rBuffer.append( nValue );
    rBuffer.append( sal_Unicode('%' ) );
}

/** convert string to pixel measure */
bool Converter::convertMeasurePx( sal_Int32& rPixel, const OUString& rString )
{
    return convertMeasure( rPixel, rString, MeasureUnit::PIXEL );
}

/** convert pixel measure to string */
void Converter::convertMeasurePx( OUStringBuffer& rBuffer, sal_Int32 nValue )
{
    rBuffer.append( nValue );
    rBuffer.append( sal_Unicode('p' ) );
    rBuffer.append( sal_Unicode('x' ) );
}

int lcl_gethex( int nChar )
{
    if( nChar >= '0' && nChar <= '9' )
        return nChar - '0';
    else if( nChar >= 'a' && nChar <= 'f' )
        return nChar - 'a' + 10;
    else if( nChar >= 'A' && nChar <= 'F' )
        return nChar - 'A' + 10;
    else
        return 0;
}

/** convert string to color */
bool Converter::convertColor( sal_Int32& rColor, const OUString& rValue )
{
    if( rValue.getLength() != 7 || rValue[0] != '#' )
        return false;

    rColor = lcl_gethex( rValue[1] ) * 16 + lcl_gethex( rValue[2] );
    rColor <<= 8;

    rColor |= ( lcl_gethex( rValue[3] ) * 16 + lcl_gethex( rValue[4] ) );
    rColor <<= 8;

    rColor |= ( lcl_gethex( rValue[5] ) * 16 + lcl_gethex( rValue[6] ) );

    return true;
}

static sal_Char aHexTab[] = "0123456789abcdef";

/** convert color to string */
void Converter::convertColor( OUStringBuffer& rBuffer, sal_Int32 nColor )
{
    rBuffer.append( sal_Unicode( '#' ) );

    sal_uInt8 nCol = (sal_uInt8)(nColor >> 16);
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );

    nCol = (sal_uInt8)(nColor >> 8);
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );

    nCol = (sal_uInt8)nColor;
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );
}

/** convert number to string */
void Converter::convertNumber( OUStringBuffer& rBuffer, sal_Int32 nNumber )
{
    rBuffer.append( nNumber );
}

/** convert string to number with optional min and max values */
bool Converter::convertNumber(  sal_Int32& rValue,
                                const OUString& rString,
                                sal_Int32 nMin, sal_Int32 nMax )
{
    bool bNeg = false;
    rValue = 0;

    sal_Int32 nPos = 0;
    sal_Int32 nLen = rString.getLength();

    // skip white space
    while( (nPos < nLen) && (rString[nPos] <= sal_Unicode(' ')) )
        nPos++;

    if( nPos < nLen && sal_Unicode('-') == rString[nPos] )
    {
        bNeg = true;
        nPos++;
    }

    // get number
    while( nPos < nLen &&
           sal_Unicode('0') <= rString[nPos] &&
           sal_Unicode('9') >= rString[nPos] )
    {
        // TODO: check overflow!
        rValue *= 10;
        rValue += (rString[nPos] - sal_Unicode('0'));
        nPos++;
    }

    if( bNeg )
        rValue *= -1;

    if( rValue < nMin )
        rValue = nMin;
    else if( rValue > nMax )
        rValue = nMax;

    return nPos == nLen;
}

/** convert double number to string (using ::rtl::math) */
void Converter::convertDouble(  OUStringBuffer& rBuffer,
                                double fNumber,
                                bool bWriteUnits,
                                sal_Int16 nSourceUnit,
                                sal_Int16 nTargetUnit)
{
    if(MeasureUnit::PERCENT == nSourceUnit)
    {
        OSL_ENSURE( nTargetUnit == MeasureUnit::PERCENT, "MeasureUnit::PERCENT only maps to MeasureUnit::PERCENT!" );
        ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true);
        if(bWriteUnits)
            rBuffer.append(sal_Unicode('%'));
    }
    else
    {
        OUStringBuffer sUnit;
        double fFactor = GetConversionFactor(sUnit, nSourceUnit, nTargetUnit);
        if(fFactor != 1.0)
            fNumber *= fFactor;
        ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true);
        if(bWriteUnits)
            rBuffer.append(sUnit);
    }
}

/** convert double number to string (using ::rtl::math) */
void Converter::convertDouble( ::rtl::OUStringBuffer& rBuffer, double fNumber)
{
    ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true);
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue,
    const ::rtl::OUString& rString, sal_Int16 nTargetUnit)
{
    sal_Int16 nSourceUnit = GetUnitFromString(rString, nTargetUnit);

    return convertDouble(rValue, rString, nSourceUnit, nTargetUnit );
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue,
    const ::rtl::OUString& rString, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
{
    rtl_math_ConversionStatus eStatus;
    rValue = ::rtl::math::stringToDouble( rString, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );

    if(eStatus == rtl_math_ConversionStatus_Ok)
    {
        OUStringBuffer sUnit;
        double fFactor = GetConversionFactor(sUnit, nSourceUnit, nTargetUnit);
        if(fFactor != 1.0 && fFactor != 0.0)
            rValue /= fFactor;
    }

    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue, const ::rtl::OUString& rString)
{
    rtl_math_ConversionStatus eStatus;
    rValue = ::rtl::math::stringToDouble( rString, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );
    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert double to ISO "duration" string; negative durations allowed */
void Converter::convertDuration(::rtl::OUStringBuffer& rBuffer,
                                const double fTime)
{
    double fValue = fTime;

    // take care of negative durations as specified in:
    // XML Schema, W3C Working Draft 07 April 2000, section 3.2.6.1
    if (fValue < 0.0)
    {
        rBuffer.append(sal_Unicode('-'));
        fValue = - fValue;
    }

    rBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM( "PT" ));
    fValue *= 24;
    double fHoursValue = ::rtl::math::approxFloor (fValue);
    fValue -= fHoursValue;
    fValue *= 60;
    double fMinsValue = ::rtl::math::approxFloor (fValue);
    fValue -= fMinsValue;
    fValue *= 60;
    double fSecsValue = ::rtl::math::approxFloor (fValue);
    fValue -= fSecsValue;
    double f100SecsValue;
    if (fValue > 0.00001)
        f100SecsValue = ::rtl::math::round( fValue, XML_MAXDIGITSCOUNT_TIME - 5);
    else
        f100SecsValue = 0.0;

    if (f100SecsValue == 1.0)
    {
        f100SecsValue = 0.0;
        fSecsValue += 1.0;
    }
    if (fSecsValue >= 60.0)
    {
        fSecsValue -= 60.0;
        fMinsValue += 1.0;
    }
    if (fMinsValue >= 60.0)
    {
        fMinsValue -= 60.0;
        fHoursValue += 1.0;
    }

    if (fHoursValue < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( fHoursValue));
    rBuffer.append( sal_Unicode('H'));
    if (fMinsValue < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( fMinsValue));
    rBuffer.append( sal_Unicode('M'));
    if (fSecsValue < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( fSecsValue));
    if (f100SecsValue > 0.0)
    {
        ::rtl::OUString a100th( ::rtl::math::doubleToUString( fValue,
                    rtl_math_StringFormat_F, XML_MAXDIGITSCOUNT_TIME - 5, '.',
                    true));
        if ( a100th.getLength() > 2 )
        {
            rBuffer.append( sal_Unicode('.'));
            rBuffer.append( a100th.copy( 2 ) );     // strip 0.
        }
    }
    rBuffer.append( sal_Unicode('S'));
}

/** convert ISO "duration" string to double; negative durations allowed */
bool Converter::convertDuration(double& rfTime,
                                const ::rtl::OUString& rString)
{
    rtl::OUString aTrimmed = rString.trim().toAsciiUpperCase();
    const sal_Unicode* pStr = aTrimmed.getStr();

    // negative time duration?
    bool bIsNegativeDuration = false;
    if ( sal_Unicode('-') == (*pStr) )
    {
        bIsNegativeDuration = true;
        pStr++;
    }

    if ( *(pStr++) != sal_Unicode('P') )            // duration must start with "P"
        return false;

    rtl::OUString sDoubleStr;
    bool bSuccess = true;
    bool bDone = false;
    bool bTimePart = false;
    bool bIsFraction = false;
    sal_Int32 nDays  = 0;
    sal_Int32 nHours = 0;
    sal_Int32 nMins  = 0;
    sal_Int32 nSecs  = 0;
    sal_Int32 nTemp = 0;

    while ( bSuccess && !bDone )
    {
        sal_Unicode c = *(pStr++);
        if ( !c )                               // end
            bDone = true;
        else if ( sal_Unicode('0') <= c && sal_Unicode('9') >= c )
        {
            if ( nTemp >= SAL_MAX_INT32 / 10 )
                bSuccess = false;
            else
            {
                if ( !bIsFraction )
                {
                    nTemp *= 10;
                    nTemp += (c - sal_Unicode('0'));
                }
                else
                {
                    sDoubleStr += OUString::valueOf(c);
                }
            }
        }
        else if ( bTimePart )
        {
            if ( c == sal_Unicode('H') )
            {
                nHours = nTemp;
                nTemp = 0;
            }
            else if ( c == sal_Unicode('M') )
            {
                nMins = nTemp;
                nTemp = 0;
            }
            else if ( (c == sal_Unicode(',')) || (c == sal_Unicode('.')) )
            {
                nSecs = nTemp;
                nTemp = 0;
                bIsFraction = true;
                sDoubleStr = OUString(RTL_CONSTASCII_USTRINGPARAM("0."));
            }
            else if ( c == sal_Unicode('S') )
            {
                if ( !bIsFraction )
                {
                    nSecs = nTemp;
                    nTemp = 0;
                    sDoubleStr = OUString(RTL_CONSTASCII_USTRINGPARAM("0.0"));
                }
            }
            else
                bSuccess = false;               // invalid character
        }
        else
        {
            if ( c == sal_Unicode('T') )            // "T" starts time part
                bTimePart = true;
            else if ( c == sal_Unicode('D') )
            {
                nDays = nTemp;
                nTemp = 0;
            }
            else if ( c == sal_Unicode('Y') || c == sal_Unicode('M') )
            {
                //! how many days is a year or month?

                OSL_ENSURE( false, "years or months in duration: not implemented");
                bSuccess = false;
            }
            else
                bSuccess = false;               // invalid character
        }
    }

    if ( bSuccess )
    {
        if ( nDays )
            nHours += nDays * 24;               // add the days to the hours part
        double fTempTime = 0.0;
        double fHour = nHours;
        double fMin = nMins;
        double fSec = nSecs;
        double fSec100 = 0.0;
        double fFraction = sDoubleStr.toDouble();
        fTempTime = fHour / 24;
        fTempTime += fMin / (24 * 60);
        fTempTime += fSec / (24 * 60 * 60);
        fTempTime += fSec100 / (24 * 60 * 60 * 60);
        fTempTime += fFraction / (24 * 60 * 60);

        // negative duration?
        if ( bIsNegativeDuration )
        {
            fTempTime = -fTempTime;
        }

        rfTime = fTempTime;
    }
    return bSuccess;
}

/** convert util::Duration to ISO "duration" string */
void Converter::convertDuration(::rtl::OUStringBuffer& rBuffer,
        const ::util::Duration& rDuration)
{
    if (rDuration.Negative)
    {
        rBuffer.append(sal_Unicode('-'));
    }
    rBuffer.append(sal_Unicode('P'));
    const bool bHaveDate(static_cast<sal_Int32>(rDuration.Years)
                        +static_cast<sal_Int32>(rDuration.Months)
                        +static_cast<sal_Int32>(rDuration.Days));
    if (rDuration.Years)
    {
        rBuffer.append(static_cast<sal_Int32>(rDuration.Years));
        rBuffer.append(sal_Unicode('Y'));
    }
    if (rDuration.Months)
    {
        rBuffer.append(static_cast<sal_Int32>(rDuration.Months));
        rBuffer.append(sal_Unicode('M'));
    }
    if (rDuration.Days)
    {
        rBuffer.append(static_cast<sal_Int32>(rDuration.Days));
        rBuffer.append(sal_Unicode('D'));
    }
    const sal_Int32 nMSecs(static_cast<sal_Int32>(rDuration.Seconds)
                         + static_cast<sal_Int32>(rDuration.MilliSeconds));
    if (static_cast<sal_Int32>(rDuration.Hours) +
        static_cast<sal_Int32>(rDuration.Minutes) + nMSecs)
    {
        rBuffer.append(sal_Unicode('T')); // time separator
        if (rDuration.Hours)
        {
            rBuffer.append(static_cast<sal_Int32>(rDuration.Hours));
            rBuffer.append(sal_Unicode('H'));
        }
        if (rDuration.Minutes)
        {
            rBuffer.append(static_cast<sal_Int32>(rDuration.Minutes));
            rBuffer.append(sal_Unicode('M'));
        }
        if (nMSecs)
        {
            // seconds must not be omitted (i.e. ".42S" is not valid)
            rBuffer.append(static_cast<sal_Int32>(rDuration.Seconds));
            if (rDuration.MilliSeconds)
            {
                rBuffer.append(sal_Unicode('.'));
                const sal_Int32 nMilliSeconds(rDuration.MilliSeconds % 1000);
                if (nMilliSeconds < 100)
                {
                    rBuffer.append(sal_Unicode('0'));
                }
                if (nMilliSeconds < 10)
                {
                    rBuffer.append(sal_Unicode('0'));
                }
                if (0 == (nMilliSeconds % 10))
                {
                    if (0 == (nMilliSeconds % 100))
                    {
                        rBuffer.append(nMilliSeconds / 100);
                    }
                    else
                    {
                        rBuffer.append(nMilliSeconds / 10);
                    }
                }
                else
                {
                    rBuffer.append(nMilliSeconds);
                }
            }
            rBuffer.append(sal_Unicode('S'));
        }
    }
    else if (!bHaveDate)
    {
        // zero duration: XMLSchema-2 says there must be at least one component
        rBuffer.append(sal_Unicode('0'));
        rBuffer.append(sal_Unicode('D'));
    }
}

enum Result { R_NOTHING, R_OVERFLOW, R_SUCCESS };

static Result
readUnsignedNumber(const ::rtl::OUString & rString,
    sal_Int32 & io_rnPos, sal_Int32 & o_rNumber)
{
    bool bOverflow(false);
    sal_Int32 nTemp(0);
    sal_Int32 nPos(io_rnPos);

    while (nPos < rString.getLength())
    {
        const sal_Unicode c = rString[nPos];
        if ((sal_Unicode('0') <= c) && (c <= sal_Unicode('9')))
        {
            nTemp *= 10;
            nTemp += (c - sal_Unicode('0'));
            if (nTemp >= SAL_MAX_INT16)
            {
                bOverflow = true;
            }
        }
        else
        {
            break;
        }
        ++nPos;
    }

    if (io_rnPos == nPos) // read something?
    {
        o_rNumber = -1;
        return R_NOTHING;
    }

    io_rnPos = nPos;
    o_rNumber = nTemp;
    return (bOverflow) ? R_OVERFLOW : R_SUCCESS;
}

static bool
readDurationT(const ::rtl::OUString & rString, sal_Int32 & io_rnPos)
{
    if ((io_rnPos < rString.getLength()) &&
        (rString[io_rnPos] == sal_Unicode('T')))
    {
        ++io_rnPos;
        return true;
    }
    return false;
}

static bool
readDurationComponent(const ::rtl::OUString & rString,
    sal_Int32 & io_rnPos, sal_Int32 & io_rnTemp, bool & io_rbTimePart,
    sal_Int32 & o_rnTarget, const sal_Unicode c)
{
    if ((io_rnPos < rString.getLength()))
    {
        if (c == rString[io_rnPos])
        {
            ++io_rnPos;
            if (-1 != io_rnTemp)
            {
                o_rnTarget = io_rnTemp;
                io_rnTemp = -1;
                if (!io_rbTimePart)
                {
                    io_rbTimePart = readDurationT(rString, io_rnPos);
                }
                return (R_OVERFLOW !=
                        readUnsignedNumber(rString, io_rnPos, io_rnTemp));
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

/** convert ISO "duration" string to util::Duration */
bool Converter::convertDuration(util::Duration& rDuration,
                                const ::rtl::OUString& rString)
{
    const ::rtl::OUString string = rString.trim().toAsciiUpperCase();
    sal_Int32 nPos(0);

    bool bIsNegativeDuration(false);
    if (string.getLength() && (sal_Unicode('-') == string[0]))
    {
        bIsNegativeDuration = true;
        ++nPos;
    }

    if ((nPos < string.getLength())
        && (string[nPos] != sal_Unicode('P'))) // duration must start with "P"
    {
        return false;
    }

    ++nPos;

    /// last read number; -1 == no valid number! always reset after using!
    sal_Int32 nTemp(-1);
    bool bTimePart(false); // have we read 'T'?
    bool bSuccess(false);
    sal_Int32 nYears(0);
    sal_Int32 nMonths(0);
    sal_Int32 nDays(0);
    sal_Int32 nHours(0);
    sal_Int32 nMinutes(0);
    sal_Int32 nSeconds(0);
    sal_Int32 nMilliSeconds(0);

    bTimePart = readDurationT(string, nPos);
    bSuccess = (R_SUCCESS == readUnsignedNumber(string, nPos, nTemp));

    if (!bTimePart && bSuccess)
    {
        bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                     nYears, sal_Unicode('Y'));
    }

    if (!bTimePart && bSuccess)
    {
        bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                     nMonths, sal_Unicode('M'));
    }

    if (!bTimePart && bSuccess)
    {
        bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                     nDays, sal_Unicode('D'));
    }

    if (bTimePart)
    {
        if (-1 == nTemp) // a 'T' must be followed by a component
        {
            bSuccess = false;
        }

        if (bSuccess)
        {
            bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                         nHours, sal_Unicode('H'));
        }

        if (bSuccess)
        {
            bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                         nMinutes, sal_Unicode('M'));
        }

        // eeek! seconds are icky.
        if ((nPos < string.getLength()) && bSuccess)
        {
            if (sal_Unicode('.') == string[nPos])
            {
                ++nPos;
                if (-1 != nTemp)
                {
                    nSeconds = nTemp;
                    nTemp = -1;
                    const sal_Int32 nStart(nPos);
                    bSuccess =
                        (R_NOTHING != readUnsignedNumber(string, nPos, nTemp));
                    if ((nPos < string.getLength()) && bSuccess)
                    {
                        if (-1 != nTemp)
                        {
                            nTemp = -1;
                            const sal_Int32 nDigits = nPos - nStart;
                            OSL_ENSURE(nDigits > 0, "bad code monkey");
                            const sal_Unicode cZero('0');
                            nMilliSeconds = 100 * (string[nStart] - cZero);
                            if (nDigits >= 2)
                            {
                                nMilliSeconds += 10 *
                                    (string[nStart+1] - cZero);
                                if (nDigits >= 3)
                                {
                                    nMilliSeconds += (string[nStart+2] - cZero);
                                }
                            }

                            if (sal_Unicode('S') == string[nPos])
                            {
                                ++nPos;
                            }
                            else
                            {
                                bSuccess = false;
                            }
                        }
                        else
                        {
                            bSuccess = false;
                        }
                    }
                }
                else
                {
                    bSuccess = false;
                }
            }
            else if (sal_Unicode('S') == string[nPos])
            {
                ++nPos;
                if (-1 != nTemp)
                {
                    nSeconds = nTemp;
                    nTemp = -1;
                }
                else
                {
                    bSuccess = false;
                }
            }
        }
    }

    if (nPos != string.getLength()) // string not processed completely?
    {
        bSuccess = false;
    }

    if (nTemp != -1) // unprocessed number?
    {
        bSuccess = false;
    }

    if (bSuccess)
    {
        rDuration.Negative      = bIsNegativeDuration;
        rDuration.Years         = static_cast<sal_Int16>(nYears);
        rDuration.Months        = static_cast<sal_Int16>(nMonths);
        rDuration.Days          = static_cast<sal_Int16>(nDays);
        rDuration.Hours         = static_cast<sal_Int16>(nHours);
        rDuration.Minutes       = static_cast<sal_Int16>(nMinutes);
        rDuration.Seconds       = static_cast<sal_Int16>(nSeconds);
        rDuration.MilliSeconds  = static_cast<sal_Int16>(nMilliSeconds);
    }

    return bSuccess;
}


/** convert util::Date to ISO "date" string */
void Converter::convertDate(
        ::rtl::OUStringBuffer& i_rBuffer,
        const util::Date& i_rDate)
{
    const util::DateTime dt(
            0, 0, 0, 0, i_rDate.Day, i_rDate.Month, i_rDate.Year);
    convertDateTime(i_rBuffer, dt, false);
}

/** convert util::DateTime to ISO "date" or "dateTime" string */
void Converter::convertDateTime(
        ::rtl::OUStringBuffer& i_rBuffer,
        const com::sun::star::util::DateTime& i_rDateTime,
        bool i_bAddTimeIf0AM )
{
    const sal_Unicode dash('-');
    const sal_Unicode col (':');
    const sal_Unicode dot ('.');
    const sal_Unicode zero('0');
    const sal_Unicode tee ('T');

    if (i_rDateTime.Year < 1000) {
        i_rBuffer.append(zero);
    }
    if (i_rDateTime.Year < 100) {
        i_rBuffer.append(zero);
    }
    if (i_rDateTime.Year < 10) {
        i_rBuffer.append(zero);
    }
    i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Year)  ).append(dash);
    if( i_rDateTime.Month < 10 ) {
        i_rBuffer.append(zero);
    }
    i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Month) ).append(dash);
    if( i_rDateTime.Day   < 10 ) {
        i_rBuffer.append(zero);
    }
    i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Day)   );

    if( i_rDateTime.Seconds != 0 ||
        i_rDateTime.Minutes != 0 ||
        i_rDateTime.Hours   != 0 ||
        i_bAddTimeIf0AM )
    {
        i_rBuffer.append(tee);
        if( i_rDateTime.Hours   < 10 ) {
            i_rBuffer.append(zero);
        }
        i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Hours)   )
                 .append(col);
        if( i_rDateTime.Minutes < 10 ) {
            i_rBuffer.append(zero);
        }
        i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Minutes) )
                 .append(col);
        if( i_rDateTime.Seconds < 10 ) {
            i_rBuffer.append(zero);
        }
        i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Seconds) );
        if( i_rDateTime.HundredthSeconds > 0 ) {
            i_rBuffer.append(dot);
            if( i_rDateTime.HundredthSeconds < 10 ) {
                i_rBuffer.append(zero);
            }
            i_rBuffer.append(
                static_cast<sal_Int32>(i_rDateTime.HundredthSeconds) );
        }
    }
}

/** convert ISO "date" or "dateTime" string to util::DateTime */
bool Converter::convertDateTime( util::DateTime& rDateTime,
                                 const ::rtl::OUString& rString )
{
    bool isDateTime;
    util::Date date;
    if (convertDateOrDateTime(date, rDateTime, isDateTime, rString))
    {
        if (!isDateTime)
        {
            rDateTime.Year = date.Year;
            rDateTime.Month = date.Month;
            rDateTime.Day = date.Day;
            rDateTime.Hours = 0;
            rDateTime.Minutes = 0;
            rDateTime.Seconds = 0;
            rDateTime.HundredthSeconds = 0;
        }
        return true;
    }
    else
    {
        return false;
    }
}

static bool
readDateTimeComponent(const ::rtl::OUString & rString,
    sal_Int32 & io_rnPos, sal_Int32 & o_rnTarget,
    const sal_Int32 nMinLength, const bool bExactLength)
{
    const sal_Int32 nOldPos(io_rnPos);
    sal_Int32 nTemp(0);
    if (R_SUCCESS != readUnsignedNumber(rString, io_rnPos, nTemp))
    {
        return false;
    }
    const sal_Int32 nTokenLength(io_rnPos - nOldPos);
    if ((nTokenLength < nMinLength) ||
        (bExactLength && (nTokenLength > nMinLength)))
    {
        return false; // bad length
    }
    o_rnTarget = nTemp;
    return true;
}

static bool lcl_isLeapYear(const sal_uInt32 nYear)
{
    return ((nYear % 4) == 0)
        && !(((nYear % 100) == 0) || ((nYear % 400) == 0));
}

static sal_uInt16
lcl_MaxDaysPerMonth(const sal_Int32 nMonth, const sal_Int32 nYear)
{
    static sal_uInt16 s_MaxDaysPerMonth[12] =
        { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    OSL_ASSERT(0 < nMonth && nMonth <= 12);
    if ((2 == nMonth) && lcl_isLeapYear(nYear))
    {
        return 29;
    }
    return s_MaxDaysPerMonth[nMonth - 1];
}

/** convert ISO "date" or "dateTime" string to util::DateTime or util::Date */
bool Converter::convertDateOrDateTime(
                util::Date & rDate, util::DateTime & rDateTime,
                bool & rbDateTime, const ::rtl::OUString & rString )
{
    bool bSuccess = true;

    const ::rtl::OUString string = rString.trim().toAsciiUpperCase();
    sal_Int32 nPos(0);
    if ((string.getLength() > nPos) && (sal_Unicode('-') == string[nPos]))
    {
        //Negative Number
        ++nPos;
    }

    sal_Int32 nYear(0);
    {
        bSuccess = readDateTimeComponent(string, nPos, nYear, 4, false);
        bSuccess &= (0 < nYear);
        bSuccess &= (nPos < string.getLength()); // not last token
    }
    if (bSuccess && (sal_Unicode('-') != string[nPos])) // separator
    {
        bSuccess = false;
    }
    if (bSuccess)
    {
        ++nPos;
    }

    sal_Int32 nMonth(0);
    if (bSuccess)
    {
        bSuccess = readDateTimeComponent(string, nPos, nMonth, 2, true);
        bSuccess &= (0 < nMonth) && (nMonth <= 12);
        bSuccess &= (nPos < string.getLength()); // not last token
    }
    if (bSuccess && (sal_Unicode('-') != string[nPos])) // separator
    {
        bSuccess = false;
    }
    if (bSuccess)
    {
        ++nPos;
    }

    sal_Int32 nDay(0);
    if (bSuccess)
    {
        bSuccess = readDateTimeComponent(string, nPos, nDay, 2, true);
        bSuccess &= (0 < nDay) && (nDay <= lcl_MaxDaysPerMonth(nMonth, nYear));
    }

    bool bHaveTime(false);
    if (bSuccess && (nPos < string.getLength()))
    {
        if (sal_Unicode('T') == string[nPos]) // time separator
        {
            bHaveTime = true;
            ++nPos;
        }
    }

    sal_Int32 nHours(0);
    sal_Int32 nMinutes(0);
    sal_Int32 nSeconds(0);
    sal_Int32 nMilliSeconds(0);
    if (bSuccess && bHaveTime)
    {
        {
            bSuccess = readDateTimeComponent(string, nPos, nHours, 2, true);
            bSuccess &= (0 <= nHours) && (nHours <= 24);
            bSuccess &= (nPos < string.getLength()); // not last token
        }
        if (bSuccess && (sal_Unicode(':') != string[nPos])) // separator
        {
            bSuccess = false;
        }
        if (bSuccess)
        {
            ++nPos;
        }

        if (bSuccess)
        {
            bSuccess = readDateTimeComponent(string, nPos, nMinutes, 2, true);
            bSuccess &= (0 <= nMinutes) && (nMinutes < 60);
            bSuccess &= (nPos < string.getLength()); // not last token
        }
        if (bSuccess && (sal_Unicode(':') != string[nPos])) // separator
        {
            bSuccess = false;
        }
        if (bSuccess)
        {
            ++nPos;
        }

        if (bSuccess)
        {
            bSuccess = readDateTimeComponent(string, nPos, nSeconds, 2, true);
            bSuccess &= (0 <= nSeconds) && (nSeconds < 60);
        }
        if (bSuccess && (nPos < string.getLength()) &&
            (sal_Unicode('.') == string[nPos])) // fraction separator
        {
            ++nPos;
            const sal_Int32 nStart(nPos);
            sal_Int32 nTemp(0);
            if (R_NOTHING == readUnsignedNumber(string, nPos, nTemp))
            {
                bSuccess = false;
            }
            if (bSuccess)
            {
                // cannot use nTemp because of possible leading zeros
                // and possible overflow => read digits directly
                const sal_Int32 nDigits(nPos - nStart);
                OSL_ENSURE(nDigits > 0, "bad code monkey");
                const sal_Unicode cZero('0');
                nMilliSeconds = 100 * (string[nStart] - cZero);
                if (nDigits >= 2)
                {
                    nMilliSeconds += 10 * (string[nStart+1] - cZero);
                    if (nDigits >= 3)
                    {
                        nMilliSeconds += (string[nStart+2] - cZero);
                    }
                }
            }
        }

        if (bSuccess && (nHours == 24))
        {
            if (!((0 == nMinutes) && (0 == nSeconds) && (0 == nMilliSeconds)))
            {
                bSuccess = false; // only 24:00:00 is valid
            }
        }
    }

    bool bHaveTimezone(false);
    bool bHaveTimezonePlus(false);
    bool bHaveTimezoneMinus(false);
    if (bSuccess && (nPos < string.getLength()))
    {
        const sal_Unicode c(string[nPos]);
        if (sal_Unicode('+') == c)
        {
            bHaveTimezone = true;
            bHaveTimezonePlus = true;
            ++nPos;
        }
        else if (sal_Unicode('-') == c)
        {
            bHaveTimezone = true;
            bHaveTimezoneMinus = true;
            ++nPos;
        }
        else if (sal_Unicode('Z') == c)
        {
            bHaveTimezone = true;
            ++nPos;
        }
        else
        {
            bSuccess = false;
        }
    }
    sal_Int32 nTimezoneHours(0);
    sal_Int32 nTimezoneMinutes(0);
    if (bSuccess && (bHaveTimezonePlus || bHaveTimezoneMinus))
    {
        bSuccess = readDateTimeComponent(
                        string, nPos, nTimezoneHours, 2, true);
        bSuccess &= (0 <= nTimezoneHours) && (nTimezoneHours <= 14);
        bSuccess &= (nPos < string.getLength()); // not last token
        if (bSuccess && (sal_Unicode(':') != string[nPos])) // separator
        {
            bSuccess = false;
        }
        if (bSuccess)
        {
            ++nPos;
        }
        if (bSuccess)
        {
            bSuccess = readDateTimeComponent(
                        string, nPos, nTimezoneMinutes, 2, true);
            bSuccess &= (0 <= nTimezoneMinutes) && (nTimezoneMinutes < 60);
        }
        if (bSuccess && (nTimezoneHours == 14))
        {
            if (0 != nTimezoneMinutes)
            {
                bSuccess = false; // only +-14:00 is valid
            }
        }
    }

    bSuccess &= (nPos == string.getLength()); // trailing junk?

    if (bSuccess && bHaveTimezone)
    {
        // util::DateTime does not support timezones!
    }

    if (bSuccess)
    {
        if (bHaveTime) // time is optional
        {
            // util::DateTime does not support negative years!
            rDateTime.Year = static_cast<sal_uInt16>(nYear);
            rDateTime.Month = static_cast<sal_uInt16>(nMonth);
            rDateTime.Day = static_cast<sal_uInt16>(nDay);
            rDateTime.Hours = static_cast<sal_uInt16>(nHours);
            rDateTime.Minutes = static_cast<sal_uInt16>(nMinutes);
            rDateTime.Seconds = static_cast<sal_uInt16>(nSeconds);
            // util::DateTime does not support 3 decimal digits of precision!
            rDateTime.HundredthSeconds =
                static_cast<sal_uInt16>(nMilliSeconds / 10);
            rbDateTime = true;
        }
        else
        {
            rDate.Year = static_cast<sal_uInt16>(nYear);
            rDate.Month = static_cast<sal_uInt16>(nMonth);
            rDate.Day = static_cast<sal_uInt16>(nDay);
            rbDateTime = false;
        }
    }
    return bSuccess;
}


/** gets the position of the first comma after npos in the string
    rStr. Commas inside '"' pairs are not matched */
sal_Int32 Converter::indexOfComma( const OUString& rStr,
                                            sal_Int32 nPos )
{
    sal_Unicode cQuote = 0;
    sal_Int32 nLen = rStr.getLength();
    for( ; nPos < nLen; nPos++ )
    {
        sal_Unicode c = rStr[nPos];
        switch( c )
        {
        case sal_Unicode('\''):
            if( 0 == cQuote )
                cQuote = c;
            else if( '\'' == cQuote )
                cQuote = 0;
            break;

        case sal_Unicode('"'):
            if( 0 == cQuote )
                cQuote = c;
            else if( '\"' == cQuote )
                cQuote = 0;
            break;

        case sal_Unicode(','):
            if( 0 == cQuote )
                return nPos;
            break;
        }
    }

    return -1;
}

const
  sal_Char aBase64EncodeTable[] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

const
  sal_uInt8 aBase64DecodeTable[]  =
    {                                            62,255,255,255, 63, // 43-47
//                                                +               /

     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255, // 48-63
//    0   1   2   3   4   5   6   7   8   9               =

    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 64-79
//        A   B   C   D   E   F   G   H   I   J   K   L   M   N   O

     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255, // 80-95
//    P   Q   R   S   T   U   V   W   X   Y   Z

      0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
//        a   b   c   d   e   f   g   h   i   j   k   l   m   n   o

     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 }; // 112-123
//    p   q   r   s   t   u   v   w   x   y   z



void ThreeByteToFourByte (const sal_Int8* pBuffer, const sal_Int32 nStart, const sal_Int32 nFullLen, rtl::OUStringBuffer& sBuffer)
{
    sal_Int32 nLen(nFullLen - nStart);
    if (nLen > 3)
        nLen = 3;
    if (nLen == 0)
    {
        sBuffer.setLength(0);
        return;
    }

    sal_Int32 nBinaer;
    switch (nLen)
    {
        case 1:
        {
            nBinaer = ((sal_uInt8)pBuffer[nStart + 0]) << 16;
        }
        break;
        case 2:
        {
            nBinaer = (((sal_uInt8)pBuffer[nStart + 0]) << 16) +
                    (((sal_uInt8)pBuffer[nStart + 1]) <<  8);
        }
        break;
        default:
        {
            nBinaer = (((sal_uInt8)pBuffer[nStart + 0]) << 16) +
                    (((sal_uInt8)pBuffer[nStart + 1]) <<  8) +
                    ((sal_uInt8)pBuffer[nStart + 2]);
        }
        break;
    }

    sBuffer.appendAscii("====");

    sal_uInt8 nIndex (static_cast<sal_uInt8>((nBinaer & 0xFC0000) >> 18));
    sBuffer.setCharAt(0, aBase64EncodeTable [nIndex]);

    nIndex = static_cast<sal_uInt8>((nBinaer & 0x3F000) >> 12);
    sBuffer.setCharAt(1, aBase64EncodeTable [nIndex]);
    if (nLen == 1)
        return;

    nIndex = static_cast<sal_uInt8>((nBinaer & 0xFC0) >> 6);
    sBuffer.setCharAt(2, aBase64EncodeTable [nIndex]);
    if (nLen == 2)
        return;

    nIndex = static_cast<sal_uInt8>((nBinaer & 0x3F));
    sBuffer.setCharAt(3, aBase64EncodeTable [nIndex]);
}

void Converter::encodeBase64(rtl::OUStringBuffer& aStrBuffer, const uno::Sequence<sal_Int8>& aPass)
{
    sal_Int32 i(0);
    sal_Int32 nBufferLength(aPass.getLength());
    const sal_Int8* pBuffer = aPass.getConstArray();
    while (i < nBufferLength)
    {
        rtl::OUStringBuffer sBuffer;
        ThreeByteToFourByte (pBuffer, i, nBufferLength, sBuffer);
        aStrBuffer.append(sBuffer);
        i += 3;
    }
}

void Converter::decodeBase64(uno::Sequence<sal_Int8>& aBuffer, const rtl::OUString& sBuffer)
{
#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nCharsDecoded =
#endif
    decodeBase64SomeChars( aBuffer, sBuffer );
    OSL_ENSURE( nCharsDecoded == sBuffer.getLength(), "some bytes left in base64 decoding!" );
}

sal_Int32 Converter::decodeBase64SomeChars(
        uno::Sequence<sal_Int8>& rOutBuffer,
        const rtl::OUString& rInBuffer)
{
    sal_Int32 nInBufferLen = rInBuffer.getLength();
    sal_Int32 nMinOutBufferLen = (nInBufferLen / 4) * 3;
    if( rOutBuffer.getLength() < nMinOutBufferLen )
        rOutBuffer.realloc( nMinOutBufferLen );

    const sal_Unicode *pInBuffer = rInBuffer.getStr();
    sal_Int8 *pOutBuffer = rOutBuffer.getArray();
    sal_Int8 *pOutBufferStart = pOutBuffer;
    sal_Int32 nCharsDecoded = 0;

    sal_uInt8 aDecodeBuffer[4];
    sal_Int32 nBytesToDecode = 0;
    sal_Int32 nBytesGotFromDecoding = 3;
    sal_Int32 nInBufferPos= 0;
    while( nInBufferPos < nInBufferLen )
    {
        sal_Unicode cChar = *pInBuffer;
        if( cChar >= '+' && cChar <= 'z' )
        {
            sal_uInt8 nByte = aBase64DecodeTable[cChar-'+'];
            if( nByte != 255 )
            {
                // We have found a valid character!
                aDecodeBuffer[nBytesToDecode++] = nByte;

                // One '=' character at the end means 2 out bytes
                // Two '=' characters at the end mean 1 out bytes
                if( '=' == cChar && nBytesToDecode > 2 )
                    nBytesGotFromDecoding--;
                if( 4 == nBytesToDecode )
                {
                    // Four characters found, so we may convert now!
                    sal_uInt32 nOut = (aDecodeBuffer[0] << 18) +
                                      (aDecodeBuffer[1] << 12) +
                                      (aDecodeBuffer[2] << 6) +
                                       aDecodeBuffer[3];

                    *pOutBuffer++  = (sal_Int8)((nOut & 0xff0000) >> 16);
                    if( nBytesGotFromDecoding > 1 )
                        *pOutBuffer++  = (sal_Int8)((nOut & 0xff00) >> 8);
                    if( nBytesGotFromDecoding > 2 )
                        *pOutBuffer++  = (sal_Int8)(nOut & 0xff);
                    nCharsDecoded = nInBufferPos + 1;
                    nBytesToDecode = 0;
                    nBytesGotFromDecoding = 3;
                }
            }
            else
            {
                nCharsDecoded++;
            }
        }
        else
        {
            nCharsDecoded++;
        }

        nInBufferPos++;
        pInBuffer++;
    }
    if( (pOutBuffer - pOutBufferStart) != rOutBuffer.getLength() )
        rOutBuffer.realloc( pOutBuffer - pOutBufferStart );

    return nCharsDecoded;
}

void Converter::clearUndefinedChars(rtl::OUString& rTarget, const rtl::OUString& rSource)
{
    sal_uInt32 nLength(rSource.getLength());
    rtl::OUStringBuffer sBuffer(nLength);
    for (sal_uInt32 i = 0; i < nLength; i++)
    {
        sal_Unicode cChar = rSource[i];
        if (!(cChar < 0x0020) ||
            (cChar == 0x0009) ||        // TAB
            (cChar == 0x000A) ||        // LF
            (cChar == 0x000D))          // legal character
            sBuffer.append(cChar);
    }
    rTarget = sBuffer.makeStringAndClear();
}

double Converter::GetConversionFactor(::rtl::OUStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
{
    double fRetval(1.0);
    rUnit.setLength(0L);

    const sal_Char* psUnit = 0;

    if(nSourceUnit != nTargetUnit)
    {
        switch(nSourceUnit)
        {
            case MeasureUnit::TWIP:
            {
                switch(nTargetUnit)
                {
                    case MeasureUnit::MM_100TH:
                    case MeasureUnit::MM_10TH:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for twip values");
                    }
                    case MeasureUnit::MM:
                    {
                        // 0.01mm = 0.57twip (exactly)
                        fRetval = ((25400.0 / 1440.0) / 1000.0);
                        psUnit = gpsMM;
                        break;
                    }
                    case MeasureUnit::CM:
                    {
                        // 0.001cm = 0.57twip (exactly)
                        fRetval = ((25400.0 / 1440.0) / 10000.0);
                        psUnit = gpsCM;
                        break;
                    }
                    case MeasureUnit::POINT:
                    {
                        // 0.01pt = 0.2twip (exactly)
                        fRetval = ((1000.0 / 20.0) / 1000.0);
                        psUnit = gpsPT;
                        break;
                    }
                    case MeasureUnit::INCH:
                    default:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for twip values");
                        // 0.0001in = 0.144twip (exactly)
                        fRetval = ((100000.0 / 1440.0) / 100000.0);
                        psUnit = gpsINCH;
                        break;
                    }
                }
                break;
            }
            case MeasureUnit::POINT:
            {
                switch(nTargetUnit)
                {
                    case MeasureUnit::MM:
                        // 1mm = 72 / 25.4 pt (exactly)
                        fRetval = ( 25.4 / 72.0 );
                        psUnit = gpsMM;
                        break;

                    case MeasureUnit::CM:
                        // 1cm = 72 / 2.54 pt (exactly)
                        fRetval = ( 2.54 / 72.0 );
                        psUnit = gpsCM;
                        break;

                    case MeasureUnit::TWIP:
                        // 1twip = 72 / 1440 pt (exactly)
                        fRetval = 20.0;     // 1440.0 / 72.0
                        psUnit = gpsPC;
                        break;

                    case MeasureUnit::INCH:
                    default:
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for pt values");
                        // 1in = 72 pt (exactly)
                        fRetval = ( 1.0 / 72.0 );
                        psUnit = gpsINCH;
                        break;
                }
                break;
            }
            case MeasureUnit::MM_10TH:
            {
                switch(nTargetUnit)
                {
                    case MeasureUnit::MM_100TH:
                    case MeasureUnit::MM_10TH:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for 1/100mm values");
                    }
                    case MeasureUnit::MM:
                    {
                        // 0.01mm = 1 mm/100 (exactly)
                        fRetval = ((10.0 / 1.0) / 100.0);
                        psUnit = gpsMM;
                        break;
                    }
                    case MeasureUnit::CM:
                    {
                        // 0.001mm = 1 mm/100 (exactly)
                        fRetval = ((10.0 / 1.0) / 1000.0);
                        psUnit = gpsCM;
                        break;
                    }
                    case MeasureUnit::POINT:
                    {
                        // 0.01pt = 0.35 mm/100 (exactly)
                        fRetval = ((72000.0 / 2540.0) / 100.0);
                        psUnit = gpsPT;
                        break;
                    }
                    case MeasureUnit::INCH:
                    default:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for 1/100mm values");
                        // 0.0001in = 0.254 mm/100 (exactly)
                        fRetval = ((100000.0 / 2540.0) / 10000.0);
                        psUnit = gpsINCH;
                        break;
                    }
                }
                break;
            }
            case MeasureUnit::MM_100TH:
            {
                switch(nTargetUnit)
                {
                    case MeasureUnit::MM_100TH:
                    case MeasureUnit::MM_10TH:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for 1/100mm values");
                    }
                    case MeasureUnit::MM:
                    {
                        // 0.01mm = 1 mm/100 (exactly)
                        fRetval = ((10.0 / 1.0) / 1000.0);
                        psUnit = gpsMM;
                        break;
                    }
                    case MeasureUnit::CM:
                    {
                        // 0.001mm = 1 mm/100 (exactly)
                        fRetval = ((10.0 / 1.0) / 10000.0);
                        psUnit = gpsCM;
                        break;
                    }
                    case MeasureUnit::POINT:
                    {
                        // 0.01pt = 0.35 mm/100 (exactly)
                        fRetval = ((72000.0 / 2540.0) / 1000.0);
                        psUnit = gpsPT;
                        break;
                    }
                    case MeasureUnit::INCH:
                    default:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for 1/100mm values");
                        // 0.0001in = 0.254 mm/100 (exactly)
                        fRetval = ((100000.0 / 2540.0) / 100000.0);
                        psUnit = gpsINCH;
                        break;
                    }
                }
                break;
            }
        }

        if( psUnit )
            rUnit.appendAscii( psUnit );
    }

    return fRetval;
}

sal_Int16 Converter::GetUnitFromString(const ::rtl::OUString& rString, sal_Int16 nDefaultUnit)
{
    sal_Int32 nPos = 0L;
    sal_Int32 nLen = rString.getLength();
    sal_Int16 nRetUnit = nDefaultUnit;

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rString[nPos] )
        nPos++;

    // skip negative
    if( nPos < nLen && sal_Unicode('-') == rString[nPos] )
        nPos++;

    // skip number
    while( nPos < nLen && sal_Unicode('0') <= rString[nPos] && sal_Unicode('9') >= rString[nPos] )
        nPos++;

    if( nPos < nLen && sal_Unicode('.') == rString[nPos] )
    {
        nPos++;
        while( nPos < nLen && sal_Unicode('0') <= rString[nPos] && sal_Unicode('9') >= rString[nPos] )
            nPos++;
    }

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rString[nPos] )
        nPos++;

    if( nPos < nLen )
    {
        switch(rString[nPos])
        {
            case sal_Unicode('%') :
            {
                nRetUnit = MeasureUnit::PERCENT;
                break;
            }
            case sal_Unicode('c'):
            case sal_Unicode('C'):
            {
                if(nPos+1 < nLen && (rString[nPos+1] == sal_Unicode('m')
                    || rString[nPos+1] == sal_Unicode('M')))
                    nRetUnit = MeasureUnit::CM;
                break;
            }
            case sal_Unicode('e'):
            case sal_Unicode('E'):
            {
                // CSS1_EMS or CSS1_EMX later
                break;
            }
            case sal_Unicode('i'):
            case sal_Unicode('I'):
            {
                if(nPos+1 < nLen && (rString[nPos+1] == sal_Unicode('n')
                    || rString[nPos+1] == sal_Unicode('n')))
                    nRetUnit = MeasureUnit::INCH;
                break;
            }
            case sal_Unicode('m'):
            case sal_Unicode('M'):
            {
                if(nPos+1 < nLen && (rString[nPos+1] == sal_Unicode('m')
                    || rString[nPos+1] == sal_Unicode('M')))
                    nRetUnit = MeasureUnit::MM;
                break;
            }
            case sal_Unicode('p'):
            case sal_Unicode('P'):
            {
                if(nPos+1 < nLen && (rString[nPos+1] == sal_Unicode('t')
                    || rString[nPos+1] == sal_Unicode('T')))
                    nRetUnit = MeasureUnit::POINT;
                if(nPos+1 < nLen && (rString[nPos+1] == sal_Unicode('c')
                    || rString[nPos+1] == sal_Unicode('C')))
                    nRetUnit = MeasureUnit::TWIP;
                break;
            }
        }
    }

    return nRetUnit;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
