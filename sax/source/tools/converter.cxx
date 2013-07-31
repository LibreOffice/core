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

#include <sax/tools/converter.hxx>

#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTimeWithTimezone.hpp>
#include <com/sun/star/util/DateWithTimezone.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <osl/time.h>

#include <algorithm>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace ::com::sun::star::i18n;


namespace sax {

static const sal_Char* gpsMM = "mm";
static const sal_Char* gpsCM = "cm";
static const sal_Char* gpsPT = "pt";
static const sal_Char* gpsINCH = "in";
static const sal_Char* gpsPC = "pc";

const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 14;

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
    sal_Int32 const nLen = rString.getLength();

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
    if( nSourceUnit == MeasureUnit::PERCENT )
    {
        OSL_ENSURE( nTargetUnit == MeasureUnit::PERCENT,
                    "MeasureUnit::PERCENT only maps to MeasureUnit::PERCENT!" );

        rBuffer.append( nMeasure );
        rBuffer.append( sal_Unicode('%' ) );

        return;
    }
    // the sign is processed separately
    if( nMeasure < 0 )
    {
        nMeasure = -nMeasure;
        rBuffer.append( sal_Unicode('-') );
    }

    // The new length is (nVal * nMul)/(nDiv*nFac*10)
    long nMul = 1000;
    long nDiv = 1;
    long nFac = 100;
    const sal_Char* psUnit = 0;
    switch( nSourceUnit )
    {
    case MeasureUnit::TWIP:
        switch( nTargetUnit )
        {
        case MeasureUnit::MM_100TH:
        case MeasureUnit::MM_10TH:
            OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,"output unit not supported for twip values" );
        case MeasureUnit::MM:
            // 0.01mm = 0.57twip (exactly)
            nMul = 25400;   // 25.4 * 1000
            nDiv = 1440;    // 72 * 20;
            nFac = 100;
            psUnit = gpsMM;
            break;

        case MeasureUnit::CM:
            // 0.001cm = 0.57twip (exactly)
            nMul = 25400;   // 2.54 * 10000
            nDiv = 1440;    // 72 * 20;
            nFac = 1000;
            psUnit = gpsCM;
            break;

        case MeasureUnit::POINT:
            // 0.01pt = 0.2twip (exactly)
            nMul = 1000;
            nDiv = 20;
            nFac = 100;
            psUnit = gpsPT;
            break;

        case MeasureUnit::INCH:
        default:
            OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,
                        "output unit not supported for twip values" );
            // 0.0001in = 0.144twip (exactly)
            nMul = 100000;
            nDiv = 1440;    // 72 * 20;
            nFac = 10000;
            psUnit = gpsINCH;
            break;
        }
        break;

    case MeasureUnit::POINT:
        // 1pt = 1pt (exactly)
        OSL_ENSURE( MeasureUnit::POINT == nTargetUnit,
                    "output unit not supported for pt values" );
        nMul = 10;
        nDiv = 1;
        nFac = 1;
        psUnit = gpsPT;
        break;
    case MeasureUnit::MM_10TH:
    case MeasureUnit::MM_100TH:
        {
            long nFac2 = (MeasureUnit::MM_100TH == nSourceUnit) ? 100 : 10;
            switch( nTargetUnit )
            {
            case MeasureUnit::MM_100TH:
            case MeasureUnit::MM_10TH:
                OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,
                            "output unit not supported for 1/100mm values" );
            case MeasureUnit::MM:
                // 0.01mm = 1 mm/100 (exactly)
                nMul = 10;
                nDiv = 1;
                nFac = nFac2;
                psUnit = gpsMM;
                break;

            case MeasureUnit::CM:
                // 0.001mm = 1 mm/100 (exactly)
                nMul = 10;
                nDiv = 1;   // 72 * 20;
                nFac = 10*nFac2;
                psUnit = gpsCM;
                break;

            case MeasureUnit::POINT:
                // 0.01pt = 0.35 mm/100 (exactly)
                nMul = 72000;
                nDiv = 2540;
                nFac = nFac2;
                psUnit = gpsPT;
                break;

            case MeasureUnit::INCH:
            default:
                OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,
                            "output unit not supported for 1/100mm values" );
                // 0.0001in = 0.254 mm/100 (exactly)
                nMul = 100000;
                nDiv = 2540;
                nFac = 100*nFac2;
                psUnit = gpsINCH;
                break;
            }
            break;
        }
    default:
        OSL_ENSURE(false, "sax::Converter::convertMeasure(): "
                "source unit not supported");
        break;
    }

    sal_Int64 nValue = nMeasure;
    OSL_ENSURE(nValue <= SAL_MAX_INT64 / nMul, "convertMeasure: overflow");
    nValue *= nMul;
    nValue /= nDiv;
    nValue += 5;
    nValue /= 10;

    rBuffer.append( static_cast<sal_Int64>(nValue / nFac) );
    if (nFac > 1 && (nValue % nFac) != 0)
    {
        rBuffer.append( sal_Unicode('.') );
        while (nFac > 1 && (nValue % nFac) != 0)
        {
            nFac /= 10;
            rBuffer.append( static_cast<sal_Int32>((nValue / nFac) % 10) );
        }
    }

    if( psUnit )
        rBuffer.appendAscii( psUnit );
}

static const OUString& getTrueString()
{
    static const OUString sTrue( "true" );
    return sTrue;
}

static const OUString& getFalseString()
{
    static const OUString sFalse( "false" );
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

/** convert string to rgb color */
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
    rValue = 0;
    sal_Int64 nNumber = 0;
    sal_Bool bRet = convertNumber64(nNumber,rString,nMin,nMax);
    if ( bRet )
        rValue = static_cast<sal_Int32>(nNumber);
    return bRet;
}

/** convert string to 64-bit number with optional min and max values */
bool Converter::convertNumber64( sal_Int64& rValue,
                                 const OUString& rString,
                                 sal_Int64 nMin, sal_Int64 nMax )
{
    bool bNeg = false;
    rValue = 0;

    sal_Int32 nPos = 0;
    sal_Int32 const nLen = rString.getLength();

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

    return ( nPos == nLen && rValue >= nMin && rValue <= nMax );
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
            rBuffer.append(sUnit.makeStringAndClear());
    }
}

/** convert double number to string (using ::rtl::math) */
void Converter::convertDouble( OUStringBuffer& rBuffer, double fNumber)
{
    ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true);
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue,
    const OUString& rString, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
{
    rtl_math_ConversionStatus eStatus;
    rValue = ::rtl::math::stringToDouble( rString, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );

    if(eStatus == rtl_math_ConversionStatus_Ok)
    {
        OUStringBuffer sUnit;
        // fdo#48969: switch source and target because factor is used to divide!
        double const fFactor =
            GetConversionFactor(sUnit, nTargetUnit, nSourceUnit);
        if(fFactor != 1.0 && fFactor != 0.0)
            rValue /= fFactor;
    }

    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue, const OUString& rString)
{
    rtl_math_ConversionStatus eStatus;
    rValue = ::rtl::math::stringToDouble( rString, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );
    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert double to ISO "duration" string; negative durations allowed */
void Converter::convertDuration(OUStringBuffer& rBuffer,
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

    rBuffer.append( "PT" );
    fValue *= 24;
    double fHoursValue = ::rtl::math::approxFloor (fValue);
    fValue -= fHoursValue;
    fValue *= 60;
    double fMinsValue = ::rtl::math::approxFloor (fValue);
    fValue -= fMinsValue;
    fValue *= 60;
    double fSecsValue = ::rtl::math::approxFloor (fValue);
    fValue -= fSecsValue;
    double fNanoSecsValue;
    if (fValue > 0.00000000001)
        fNanoSecsValue = ::rtl::math::round( fValue, XML_MAXDIGITSCOUNT_TIME - 5);
    else
        fNanoSecsValue = 0.0;

    if (fNanoSecsValue == 1.0)
    {
        fNanoSecsValue = 0.0;
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
    if (fNanoSecsValue > 0.0)
    {
        OUString aNS( ::rtl::math::doubleToUString( fValue,
                    rtl_math_StringFormat_F, XML_MAXDIGITSCOUNT_TIME - 5, '.',
                    true));
        if ( aNS.getLength() > 2 )
        {
            rBuffer.append( sal_Unicode('.'));
            rBuffer.append( aNS.copy( 2 ) );     // strip "0."
        }
    }
    rBuffer.append( sal_Unicode('S'));
}

/** convert ISO "duration" string to double; negative durations allowed */
bool Converter::convertDuration(double& rfTime,
                                const OUString& rString)
{
    OUString aTrimmed = rString.trim().toAsciiUpperCase();
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

    OUString sDoubleStr;
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
                sDoubleStr = OUString("0.");
            }
            else if ( c == sal_Unicode('S') )
            {
                if ( !bIsFraction )
                {
                    nSecs = nTemp;
                    nTemp = 0;
                    sDoubleStr = OUString("0.0");
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

                OSL_FAIL( "years or months in duration: not implemented");
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
        double fFraction = sDoubleStr.toDouble();
        fTempTime = fHour / 24;
        fTempTime += fMin / (24 * 60);
        fTempTime += fSec / (24 * 60 * 60);
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

/** convert util::Duration to ISO8601 "duration" string */
void Converter::convertDuration(OUStringBuffer& rBuffer,
        const ::util::Duration& rDuration)
{
    if (rDuration.Negative)
    {
        rBuffer.append(sal_Unicode('-'));
    }
    rBuffer.append(sal_Unicode('P'));
    const bool bHaveDate(rDuration.Years  != 0 ||
                         rDuration.Months != 0 ||
                         rDuration.Days   != 0);
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
    if ( rDuration.Hours != 0
         || rDuration.Minutes != 0
         || rDuration.Seconds != 0
         || rDuration.NanoSeconds != 0 )
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
        if (rDuration.Seconds != 0 || rDuration.NanoSeconds != 0)
        {
            // seconds must not be omitted (i.e. ".42S" is not valid)
            rBuffer.append(static_cast<sal_Int32>(rDuration.Seconds));
            if (rDuration.NanoSeconds)
            {
                OSL_ENSURE(rDuration.NanoSeconds < 1000000000,"NanoSeconds cannot be more than 999 999 999");
                rBuffer.append('.');
                std::ostringstream ostr;
                ostr.fill('0');
                ostr.width(9);
                ostr << rDuration.NanoSeconds;
                rBuffer.append(OUString::createFromAscii(ostr.str().c_str()));
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
readUnsignedNumber(const OUString & rString,
    sal_Int32 & io_rnPos, sal_Int32 & o_rNumber)
{
    bool bOverflow(false);
    sal_Int64 nTemp(0);
    sal_Int32 nPos(io_rnPos);

    while (nPos < rString.getLength())
    {
        const sal_Unicode c = rString[nPos];
        if ((sal_Unicode('0') <= c) && (c <= sal_Unicode('9')))
        {
            nTemp *= 10;
            nTemp += (c - sal_Unicode('0'));
            if (nTemp >= SAL_MAX_INT32)
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

static Result
readUnsignedNumberMaxDigits(int maxDigits,
                            const ::rtl::OUString & rString, sal_Int32 & io_rnPos,
                            sal_Int32 & o_rNumber)
{
    bool bOverflow(false);
    sal_Int64 nTemp(0);
    sal_Int32 nPos(io_rnPos);
    OSL_ENSURE(maxDigits >= 0, "negative amount of digits makes no sense");

    while (nPos < rString.getLength())
    {
        const sal_Unicode c = rString[nPos];
        if ((sal_Unicode('0') <= c) && (c <= sal_Unicode('9')))
        {
            if (maxDigits > 0)
            {
                nTemp *= 10;
                nTemp += (c - sal_Unicode('0'));
                if (nTemp >= SAL_MAX_INT32)
                {
                    bOverflow = true;
                }
                --maxDigits;
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
readDurationT(const OUString & rString, sal_Int32 & io_rnPos)
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
readDurationComponent(const OUString & rString,
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

/** convert ISO8601 "duration" string to util::Duration */
bool Converter::convertDuration(util::Duration& rDuration,
                                const OUString& rString)
{
    const OUString string = rString.trim().toAsciiUpperCase();
    sal_Int32 nPos(0);

    bool bIsNegativeDuration(false);
    if (!string.isEmpty() && (sal_Unicode('-') == string[0]))
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
    sal_Int32 nNanoSeconds(0);

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
            if (string[nPos] == sal_Unicode('.') ||
                string[nPos] == sal_Unicode(','))
            {
                ++nPos;
                if (-1 != nTemp)
                {
                    nSeconds = nTemp;
                    nTemp = -1;
                    const sal_Int32 nStart(nPos);
                    bSuccess = readUnsignedNumberMaxDigits(9, string, nPos, nTemp) == R_SUCCESS;
                    if ((nPos < string.getLength()) && bSuccess)
                    {
                        if (-1 != nTemp)
                        {
                            const sal_Int32 nDigits = std::min<sal_Int32>(nPos - nStart, 9);
                            OSL_ENSURE(nDigits > 0, "bad code monkey: negative digits");
                            nNanoSeconds=static_cast<double>(nTemp)*(1000000000.0/pow(10.0,nDigits));
                            nTemp=-1;
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
        rDuration.NanoSeconds   = static_cast<sal_Int32>(nNanoSeconds);
    }

    return bSuccess;
}


static void
lcl_AppendTimezone(OUStringBuffer & i_rBuffer, sal_Int16 const nOffset)
{
    if (0 == nOffset)
    {
        i_rBuffer.append(sal_Unicode('Z'));
    }
    else
    {
        if (0 < nOffset)
        {
            i_rBuffer.append(sal_Unicode('+'));
        }
        else
        {
            i_rBuffer.append(sal_Unicode('-'));
        }
        const sal_Int32 nHours  (abs(nOffset) / 60);
        const sal_Int32 nMinutes(abs(nOffset) % 60);
        SAL_WARN_IF(nHours > 14 || (nHours == 14 && nMinutes > 0),
                "sax", "convertDateTime: timezone overflow");
        if (nHours < 10)
        {
            i_rBuffer.append('0');
        }
        i_rBuffer.append(nHours);
        i_rBuffer.append(':');
        if (nMinutes < 10)
        {
            i_rBuffer.append('0');
        }
        i_rBuffer.append(nMinutes);
    }
}

/** convert util::Date to ISO "date" string */
void Converter::convertDate(
        OUStringBuffer& i_rBuffer,
        const util::Date& i_rDate,
        sal_Int16 const*const pTimeZoneOffset)
{
    const util::DateTime dt(0, 0, 0, 0,
        i_rDate.Day, i_rDate.Month, i_rDate.Year, false);
    convertDateTime(i_rBuffer, dt, pTimeZoneOffset, false);
}

/** convert util::DateTime to ISO "date" or "dateTime" string */
void Converter::convertDateTime(
        OUStringBuffer& i_rBuffer,
        const com::sun::star::util::DateTime& i_rDateTime,
        sal_Int16 const*const pTimeZoneOffset,
        bool i_bAddTimeIf0AM )
{
    const sal_Unicode dash('-');
    const sal_Unicode col (':');
    const sal_Unicode dot ('.');
    const sal_Unicode zero('0');
    const sal_Unicode tee ('T');

    sal_Int32 const nYear(abs(i_rDateTime.Year));
    if (i_rDateTime.Year < 0) {
        i_rBuffer.append(dash); // negative
    }
    if (nYear < 1000) {
        i_rBuffer.append(zero);
    }
    if (nYear < 100) {
        i_rBuffer.append(zero);
    }
    if (nYear < 10) {
        i_rBuffer.append(zero);
    }
    i_rBuffer.append(nYear).append(dash);
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
        if( i_rDateTime.NanoSeconds > 0 ) {
            OSL_ENSURE(i_rDateTime.NanoSeconds < 1000000000,"NanoSeconds cannot be more than 999 999 999");
            i_rBuffer.append(dot);
            std::ostringstream ostr;
            ostr.fill('0');
            ostr.width(9);
            ostr << i_rDateTime.NanoSeconds;
            i_rBuffer.append(OUString::createFromAscii(ostr.str().c_str()));
        }
    }

    if (pTimeZoneOffset)
    {
        lcl_AppendTimezone(i_rBuffer, *pTimeZoneOffset);
    }
    else if (i_rDateTime.IsUTC)
    {
        // append local time
        lcl_AppendTimezone(i_rBuffer, 0);
    }
}

/** convert ISO "date" or "dateTime" string to util::DateTime */
bool Converter::parseDateTime(   util::DateTime& rDateTime,
                             boost::optional<sal_Int16> *const pTimeZoneOffset,
                                 const OUString& rString )
{
    bool isDateTime;
    return parseDateOrDateTime(0, rDateTime, isDateTime, pTimeZoneOffset,
            rString);
}

static bool lcl_isLeapYear(const sal_uInt32 nYear)
{
    return ((nYear % 4) == 0)
        && (((nYear % 100) != 0) || ((nYear % 400) == 0));
}

static sal_uInt16
lcl_MaxDaysPerMonth(const sal_Int32 nMonth, const sal_Int32 nYear)
{
    static const sal_uInt16 s_MaxDaysPerMonth[12] =
        { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    OSL_ASSERT(0 < nMonth && nMonth <= 12);
    if ((2 == nMonth) && lcl_isLeapYear(nYear))
    {
        return 29;
    }
    return s_MaxDaysPerMonth[nMonth - 1];
}

static void lcl_ConvertToUTC(
        sal_Int16 & o_rYear, sal_uInt16 & o_rMonth, sal_uInt16 & o_rDay,
        sal_uInt16 & o_rHours, sal_uInt16 & o_rMinutes,
        sal_Int16 const nSourceOffset)
{
    sal_Int16 nOffsetHours(abs(nSourceOffset) / 60);
    sal_Int16 const nOffsetMinutes(abs(nSourceOffset) % 60);
    o_rMinutes += nOffsetMinutes;
    if (nSourceOffset < 0)
    {
        o_rMinutes += nOffsetMinutes;
        if (60 <= o_rMinutes)
        {
            o_rMinutes -= 60;
            ++nOffsetHours;
        }
        o_rHours += nOffsetHours;
        if (o_rHours < 24)
        {
            return;
        }
        while (24 <= o_rHours)
        {
            o_rHours -= 24;
            ++o_rDay;
        }
        sal_Int16 const nDaysInMonth(lcl_MaxDaysPerMonth(o_rMonth, o_rYear));
        if (o_rDay <= nDaysInMonth)
        {
            return;
        }
        o_rDay -= nDaysInMonth;
        ++o_rMonth;
        if (o_rMonth <= 12)
        {
            return;
        }
        o_rMonth = 1;
        ++o_rYear; // works for negative year too
    }
    else if (0 < nSourceOffset)
    {
        // argh everything is unsigned
        if (o_rMinutes < nOffsetMinutes)
        {
            o_rMinutes += 60;
            ++nOffsetHours;
        }
        o_rMinutes -= nOffsetMinutes;
        sal_Int16 nDaySubtract(0);
        while (o_rHours < nOffsetHours)
        {
            o_rHours += 24;
            ++nDaySubtract;
        }
        o_rHours -= nOffsetHours;
        if (nDaySubtract < o_rDay)
        {
            o_rDay -= nDaySubtract;
            return;
        }
        sal_Int16 const nPrevMonth((o_rMonth == 1) ? 12 : o_rMonth - 1);
        sal_Int16 const nDaysInMonth(lcl_MaxDaysPerMonth(nPrevMonth, o_rYear));
        o_rDay += nDaysInMonth;
        --o_rMonth;
        if (0 == o_rMonth)
        {
            o_rMonth = 12;
            --o_rYear; // works for negative year too
        }
        o_rDay -= nDaySubtract;
    }
}

static bool
readDateTimeComponent(const OUString & rString,
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



/** convert ISO "date" or "dateTime" string to util::DateTime or util::Date */
bool Converter::parseDateOrDateTime(
                util::Date *const pDate, util::DateTime & rDateTime,
                bool & rbDateTime,
                boost::optional<sal_Int16> *const pTimeZoneOffset,
                const OUString & rString )
{
    bool bSuccess = true;
    bool isNegative(false);

    const OUString string = rString.trim().toAsciiUpperCase();
    sal_Int32 nPos(0);
    if (string.getLength() > nPos)
    {
        if (sal_Unicode('-') == string[nPos])
        {
            isNegative = true;
            ++nPos;
        }
    }

    sal_Int32 nYear(0);
    {
        // While W3C XMLSchema specifies years with a minimum of 4 digits, be
        // leninent in what we accept for years < 1000. One digit is acceptable
        // if the remainders match.
        bSuccess = readDateTimeComponent(string, nPos, nYear, 1, false);
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
    sal_Int32 nNanoSeconds(0);
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
            (sal_Unicode('.') == string[nPos] || sal_Unicode(',') == string[nPos])) // fraction separator
        {
            ++nPos;
            const sal_Int32 nStart(nPos);
            sal_Int32 nTemp(0);
            if (R_NOTHING == readUnsignedNumberMaxDigits(9, string, nPos, nTemp))
            {
                bSuccess = false;
            }
            if (bSuccess)
            {
                const sal_Int32 nDigits = std::min<sal_Int32>(nPos - nStart, 9);
                OSL_ENSURE(nDigits > 0, "bad code monkey");
                nNanoSeconds=static_cast<double>(nTemp)*(1000000000.0/pow(10.0,nDigits));
            }
        }

        if (bSuccess && (nHours == 24))
        {
            if (!((0 == nMinutes) && (0 == nSeconds) && (0 == nNanoSeconds)))
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

    if (bSuccess)
    {
        sal_Int16 const nTimezoneOffset = ((bHaveTimezoneMinus) ? (-1) : (+1))
                        * ((nTimezoneHours * 60) + nTimezoneMinutes);
        if (!pDate || bHaveTime) // time is optional
        {
            rDateTime.Year =
                ((isNegative) ? (-1) : (+1)) * static_cast<sal_Int16>(nYear);
            rDateTime.Month = static_cast<sal_uInt16>(nMonth);
            rDateTime.Day = static_cast<sal_uInt16>(nDay);
            rDateTime.Hours = static_cast<sal_uInt16>(nHours);
            rDateTime.Minutes = static_cast<sal_uInt16>(nMinutes);
            rDateTime.Seconds = static_cast<sal_uInt16>(nSeconds);
            rDateTime.NanoSeconds = static_cast<sal_uInt32>(nNanoSeconds);
            if (bHaveTimezone)
            {
                if (pTimeZoneOffset)
                {
                    *pTimeZoneOffset = nTimezoneOffset;
                    rDateTime.IsUTC = (0 == nTimezoneOffset);
                }
                else
                {
                    lcl_ConvertToUTC(rDateTime.Year, rDateTime.Month,
                            rDateTime.Day, rDateTime.Hours, rDateTime.Minutes,
                            nTimezoneOffset);
                    rDateTime.IsUTC = true;
                }
            }
            else
            {
                if (pTimeZoneOffset)
                {
                    pTimeZoneOffset->reset();
                }
                rDateTime.IsUTC = false;
            }
            rbDateTime = bHaveTime;
        }
        else
        {
            pDate->Year =
                ((isNegative) ? (-1) : (+1)) * static_cast<sal_Int16>(nYear);
            pDate->Month = static_cast<sal_uInt16>(nMonth);
            pDate->Day = static_cast<sal_uInt16>(nDay);
            if (bHaveTimezone)
            {
                if (pTimeZoneOffset)
                {
                    *pTimeZoneOffset = nTimezoneOffset;
                }
                else
                {
                    // a Date cannot be adjusted
                    SAL_INFO("sax", "dropping timezone");
                }
            }
            else
            {
                if (pTimeZoneOffset)
                {
                    pTimeZoneOffset->reset();
                }
            }
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



void ThreeByteToFourByte (const sal_Int8* pBuffer, const sal_Int32 nStart, const sal_Int32 nFullLen, OUStringBuffer& sBuffer)
{
    sal_Int32 nLen(nFullLen - nStart);
    if (nLen > 3)
        nLen = 3;
    if (nLen == 0)
    {
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

    sal_Unicode buf[] = { '=', '=', '=', '=' };

    sal_uInt8 nIndex (static_cast<sal_uInt8>((nBinaer & 0xFC0000) >> 18));
    buf[0] = aBase64EncodeTable [nIndex];

    nIndex = static_cast<sal_uInt8>((nBinaer & 0x3F000) >> 12);
    buf[1] = aBase64EncodeTable [nIndex];
    if (nLen > 1)
    {
        nIndex = static_cast<sal_uInt8>((nBinaer & 0xFC0) >> 6);
        buf[2] = aBase64EncodeTable [nIndex];
        if (nLen > 2)
        {
            nIndex = static_cast<sal_uInt8>((nBinaer & 0x3F));
            buf[3] = aBase64EncodeTable [nIndex];
        }
    }
    sBuffer.append(buf, SAL_N_ELEMENTS(buf));
}

void Converter::encodeBase64(OUStringBuffer& aStrBuffer, const uno::Sequence<sal_Int8>& aPass)
{
    sal_Int32 i(0);
    sal_Int32 nBufferLength(aPass.getLength());
    const sal_Int8* pBuffer = aPass.getConstArray();
    while (i < nBufferLength)
    {
        ThreeByteToFourByte (pBuffer, i, nBufferLength, aStrBuffer);
        i += 3;
    }
}

void Converter::decodeBase64(uno::Sequence<sal_Int8>& aBuffer, const OUString& sBuffer)
{
#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nCharsDecoded =
#endif
    decodeBase64SomeChars( aBuffer, sBuffer );
    OSL_ENSURE( nCharsDecoded == sBuffer.getLength(), "some bytes left in base64 decoding!" );
}

sal_Int32 Converter::decodeBase64SomeChars(
        uno::Sequence<sal_Int8>& rOutBuffer,
        const OUString& rInBuffer)
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

double Converter::GetConversionFactor(OUStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
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
                    {
                        // 0.01mm = 0.57twip (exactly)
                        fRetval = ((25400.0 / 1440.0) / 10.0);
                        break;
                    }
                    case MeasureUnit::MM_10TH:
                    {
                        // 0.01mm = 0.57twip (exactly)
                        fRetval = ((25400.0 / 1440.0) / 100.0);
                        break;
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
                    case MeasureUnit::MM_100TH:
                    {
                        // 1mm = 72 / 25.4 pt (exactly)
                        fRetval = ( 2540.0 / 72.0 );
                        break;
                    }
                    case MeasureUnit::MM_10TH:
                    {
                        // 1mm = 72 / 25.4 pt (exactly)
                        fRetval = ( 254.0 / 72.0 );
                        break;
                    }
                    case MeasureUnit::MM:
                    {
                        // 1mm = 72 / 25.4 pt (exactly)
                        fRetval = ( 25.4 / 72.0 );
                        psUnit = gpsMM;
                        break;

                    }
                    case MeasureUnit::CM:
                    {
                        // 1cm = 72 / 2.54 pt (exactly)
                        fRetval = ( 2.54 / 72.0 );
                        psUnit = gpsCM;
                        break;
                    }
                    case MeasureUnit::TWIP:
                    {
                        // 1twip = 72 / 1440 pt (exactly)
                        fRetval = 20.0;     // 1440.0 / 72.0
                        psUnit = gpsPC;
                        break;
                    }
                    case MeasureUnit::INCH:
                    default:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for pt values");
                        // 1in = 72 pt (exactly)
                        fRetval = ( 1.0 / 72.0 );
                        psUnit = gpsINCH;
                        break;
                    }
                }
                break;
            }
            case MeasureUnit::MM_10TH:
            {
                switch(nTargetUnit)
                {
                    case MeasureUnit::MM_100TH:
                    {
                        fRetval = 10.0;
                        break;
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
                    case MeasureUnit::TWIP:
                    {
                        fRetval = ((20.0 * 72000.0 / 2540.0) / 100.0);
                        psUnit = gpsPC;
                        break;
                    }
                    case MeasureUnit::INCH:
                    default:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for 1/10mm values");
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
                    case MeasureUnit::MM_10TH:
                    {
                        fRetval = ((10.0 / 1.0) / 100.0);
                        break;
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
                    case MeasureUnit::TWIP:
                    {
                        fRetval = ((20.0 * 72000.0 / 2540.0) / 1000.0);
                        psUnit = gpsPC;
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
            case MeasureUnit::MM:
            {
                switch(nTargetUnit)
                {
                    case MeasureUnit::MM_100TH:
                    {
                        fRetval = 100.0;
                        break;
                    }
                    case MeasureUnit::MM_10TH:
                    {
                        fRetval = 10.0;
                        break;
                    }
                    case MeasureUnit::CM:
                    {
                        fRetval = 0.1;
                        psUnit = gpsCM;
                        break;
                    }
                    case MeasureUnit::POINT:
                    {
                        fRetval = 72.0 / (2.54 * 10);
                        psUnit = gpsPT;
                        break;
                    }
                    case MeasureUnit::TWIP:
                    {
                        fRetval = (20.0 * 72.0) / (2.54 * 10);
                        psUnit = gpsPC;
                        break;
                    }
                    case MeasureUnit::INCH:
                    default:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for cm values");
                        fRetval = 1 / (2.54 * 10);
                        psUnit = gpsINCH;
                        break;
                    }
                }
                break;
            }
            case MeasureUnit::CM:
            {
                switch(nTargetUnit)
                {
                    case MeasureUnit::MM_100TH:
                    {
                        fRetval = 1000.0;
                        break;
                    }
                    case MeasureUnit::MM_10TH:
                    {
                        fRetval = 100.0;
                        break;
                    }
                    case MeasureUnit::MM:
                    {
                        fRetval = 10.0;
                        psUnit = gpsMM;
                        break;
                    }
                    case MeasureUnit::CM:
                    {
                        break;
                    }
                    case MeasureUnit::POINT:
                    {
                        fRetval = 72.0 / 2.54;
                        psUnit = gpsPT;
                        break;
                    }
                    case MeasureUnit::TWIP:
                    {
                        fRetval = (20.0 * 72.0) / 2.54;
                        psUnit = gpsPC;
                        break;
                    }
                    case MeasureUnit::INCH:
                    default:
                    {
                        OSL_ENSURE( MeasureUnit::INCH == nTargetUnit, "output unit not supported for cm values");
                        fRetval = 1 / 2.54;
                        psUnit = gpsINCH;
                        break;
                    }
                }
                break;
            }
            case MeasureUnit::INCH:
            {
                switch (nTargetUnit)
                {
                    case MeasureUnit::MM_100TH:
                    {
                        fRetval = 2540;
                        break;
                    }
                    case MeasureUnit::MM_10TH:
                    {
                        fRetval = 254;
                        break;
                    }
                    case MeasureUnit::MM:
                    {
                        fRetval = 25.4;
                        psUnit = gpsMM;
                        break;
                    }
                    case MeasureUnit::CM:
                    {
                        fRetval = 2.54;
                        psUnit = gpsCM;
                        break;
                    }
                    case MeasureUnit::POINT:
                    {
                        fRetval = 72.0;
                        psUnit = gpsPT;
                        break;
                    }
                    case MeasureUnit::TWIP:
                    {
                        fRetval = 72.0 * 20.0;
                        psUnit = gpsPC;
                        break;
                    }
                    default:
                    {
                        OSL_FAIL("output unit not supported for in values");
                        fRetval = 1;
                        psUnit = gpsINCH;
                        break;
                    }
                }
                break;
            }
            default:
                OSL_ENSURE(false, "sax::Converter::GetConversionFactor(): "
                        "source unit not supported");
                break;
        }

        if( psUnit )
            rUnit.appendAscii( psUnit );
    }

    return fRetval;
}

sal_Int16 Converter::GetUnitFromString(const OUString& rString, sal_Int16 nDefaultUnit)
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


bool Converter::convertAny(OUStringBuffer&    rsValue,
                           OUStringBuffer&    rsType ,
                           const com::sun::star::uno::Any& rValue)
{
    bool bConverted = false;

    rsValue.setLength(0);
    rsType.setLength (0);

    switch (rValue.getValueTypeClass())
    {
        case com::sun::star::uno::TypeClass_BYTE :
        case com::sun::star::uno::TypeClass_SHORT :
        case com::sun::star::uno::TypeClass_UNSIGNED_SHORT :
        case com::sun::star::uno::TypeClass_LONG :
        case com::sun::star::uno::TypeClass_UNSIGNED_LONG :
            {
                sal_Int32 nTempValue = 0;
                if (rValue >>= nTempValue)
                {
                    rsType.appendAscii("integer");
                    bConverted = true;
                    ::sax::Converter::convertNumber(rsValue, nTempValue);
                }
            }
            break;

        case com::sun::star::uno::TypeClass_BOOLEAN :
            {
                bool bTempValue = false;
                if (rValue >>= bTempValue)
                {
                    rsType.appendAscii("boolean");
                    bConverted = true;
                    ::sax::Converter::convertBool(rsValue, bTempValue);
                }
            }
            break;

        case com::sun::star::uno::TypeClass_FLOAT :
        case com::sun::star::uno::TypeClass_DOUBLE :
            {
                double fTempValue = 0.0;
                if (rValue >>= fTempValue)
                {
                    rsType.appendAscii("float");
                    bConverted = true;
                    ::sax::Converter::convertDouble(rsValue, fTempValue);
                }
            }
            break;

        case com::sun::star::uno::TypeClass_STRING :
            {
                OUString sTempValue;
                if (rValue >>= sTempValue)
                {
                    rsType.appendAscii("string");
                    bConverted = true;
                    rsValue.append(sTempValue);
                }
            }
            break;

        case com::sun::star::uno::TypeClass_STRUCT :
            {
                com::sun::star::util::Date     aDate    ;
                com::sun::star::util::Time     aTime    ;
                com::sun::star::util::DateTime aDateTime;

                if (rValue >>= aDate)
                {
                    rsType.appendAscii("date");
                    bConverted = true;
                    com::sun::star::util::DateTime aTempValue;
                    aTempValue.Day              = aDate.Day;
                    aTempValue.Month            = aDate.Month;
                    aTempValue.Year             = aDate.Year;
                    aTempValue.NanoSeconds = 0;
                    aTempValue.Seconds          = 0;
                    aTempValue.Minutes          = 0;
                    aTempValue.Hours            = 0;
                    ::sax::Converter::convertDateTime(rsValue, aTempValue, 0);
                }
                else
                if (rValue >>= aTime)
                {
                    rsType.appendAscii("time");
                    bConverted = true;
                    com::sun::star::util::Duration aTempValue;
                    aTempValue.Days             = 0;
                    aTempValue.Months           = 0;
                    aTempValue.Years            = 0;
                    aTempValue.NanoSeconds     = aTime.NanoSeconds;
                    aTempValue.Seconds          = aTime.Seconds;
                    aTempValue.Minutes          = aTime.Minutes;
                    aTempValue.Hours            = aTime.Hours;
                    ::sax::Converter::convertDuration(rsValue, aTempValue);
                }
                else
                if (rValue >>= aDateTime)
                {
                    rsType.appendAscii("date");
                    bConverted = true;
                    ::sax::Converter::convertDateTime(rsValue, aDateTime, 0);
                }
            }
            break;
        default:
            break;
    }

    return bConverted;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
