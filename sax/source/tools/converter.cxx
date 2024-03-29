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
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/util/Time.hpp>
#include <optional>

#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <rtl/character.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <o3tl/unit_conversion.hxx>
#include <osl/diagnose.h>
#include <tools/long.hxx>

#include <algorithm>
#include <string_view>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;


namespace sax {

const std::string_view gpsMM = "mm";
const std::string_view gpsCM = "cm";
const std::string_view gpsPT = "pt";
const std::string_view gpsINCH = "in";
const std::string_view gpsPC = "pc";

const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 14;

static sal_Int64 toInt64_WithLength(const sal_Unicode * str, sal_Int16 radix, sal_Int32 nStrLength )
{
    return rtl_ustr_toInt64_WithLength(str, radix, nStrLength);
}
static sal_Int64 toInt64_WithLength(const char * str, sal_Int16 radix, sal_Int32 nStrLength )
{
    return rtl_str_toInt64_WithLength(str, radix, nStrLength);
}

namespace
{
o3tl::Length Measure2O3tlUnit(sal_Int16 nUnit)
{
    switch (nUnit)
    {
        case MeasureUnit::TWIP:
            return o3tl::Length::twip;
        case MeasureUnit::POINT:
            return o3tl::Length::pt;
        case MeasureUnit::MM_10TH:
            return o3tl::Length::mm10;
        case MeasureUnit::MM_100TH:
            return o3tl::Length::mm100;
        case MeasureUnit::MM:
            return o3tl::Length::mm;
        case MeasureUnit::CM:
            return o3tl::Length::cm;
        default:
            SAL_WARN("sax", "unit not supported for length");
            [[fallthrough]];
        case MeasureUnit::INCH:
            return o3tl::Length::in;
    }
}

std::string_view Measure2UnitString(sal_Int16 nUnit)
{
    switch (nUnit)
    {
        case MeasureUnit::TWIP:
            return gpsPC; // ??
        case MeasureUnit::POINT:
            return gpsPT;
        case MeasureUnit::MM_10TH:
        case MeasureUnit::MM_100TH:
            return {};
        case MeasureUnit::MM:
            return gpsMM;
        case MeasureUnit::CM:
            return gpsCM;
        case MeasureUnit::INCH:
        default:
            return gpsINCH;
    }
}

template <typename V> bool wordEndsWith(V string, std::string_view expected)
{
    V substr = string.substr(0, expected.size());
    return std::equal(substr.begin(), substr.end(), expected.begin(), expected.end(),
                      [](sal_uInt32 c1, sal_uInt32 c2) { return rtl::toAsciiLowerCase(c1) == c2; })
           && (string.size() == expected.size() || string[expected.size()] == ' ');
}

}

/** convert string to measure using optional min and max values*/
template<typename V>
static bool lcl_convertMeasure( sal_Int32& rValue,
                                V rString,
                                sal_Int16 nTargetUnit /* = MeasureUnit::MM_100TH */,
                                sal_Int32 nMin /* = SAL_MIN_INT32 */,
                                sal_Int32 nMax /* = SAL_MAX_INT32 */ )
{
    bool bNeg = false;
    double nVal = 0;

    sal_Int32 nPos = 0;
    sal_Int32 const nLen = rString.size();

    // skip white space
    while( (nPos < nLen) && (rString[nPos] <= ' ') )
        nPos++;

    if( nPos < nLen && '-' == rString[nPos] )
    {
        bNeg = true;
        nPos++;
    }

    // get number
    while( nPos < nLen &&
           '0' <= rString[nPos] &&
           '9' >= rString[nPos] )
    {
        // TODO: check overflow!
        nVal *= 10;
        nVal += (rString[nPos] - '0');
        nPos++;
    }
    if( nPos < nLen && '.' == rString[nPos] )
    {
        nPos++;
        double nDiv = 1.;

        while( nPos < nLen &&
               '0' <= rString[nPos] &&
               '9' >= rString[nPos] )
        {
            // TODO: check overflow!
            nDiv *= 10;
            nVal += ( static_cast<double>(rString[nPos] - '0') / nDiv );
            nPos++;
        }
    }

    // skip white space
    while( (nPos < nLen) && (rString[nPos] <= ' ') )
        nPos++;

    if( nPos < nLen )
    {

        if( MeasureUnit::PERCENT == nTargetUnit )
        {
            if( '%' != rString[nPos] )
                return false;
        }
        else if( MeasureUnit::PIXEL == nTargetUnit )
        {
            if( nPos + 1 >= nLen ||
                ('p' != rString[nPos] &&
                 'P' != rString[nPos])||
                ('x' != rString[nPos+1] &&
                 'X' != rString[nPos+1]) )
                return false;
        }
        else
        {
            OSL_ENSURE( MeasureUnit::TWIP == nTargetUnit || MeasureUnit::POINT == nTargetUnit ||
                        MeasureUnit::MM_100TH == nTargetUnit || MeasureUnit::MM_10TH == nTargetUnit ||
                        MeasureUnit::PIXEL == nTargetUnit, "unit is not supported");

            o3tl::Length eFrom = o3tl::Length::invalid;

            if( MeasureUnit::TWIP == nTargetUnit )
            {
                switch (rtl::toAsciiLowerCase<sal_uInt32>(rString[nPos]))
                {
                case u'c':
                    if (wordEndsWith(rString.substr(nPos + 1), "m"))
                        eFrom = o3tl::Length::cm;
                    break;
                case u'i':
                    if (wordEndsWith(rString.substr(nPos + 1), "n"))
                        eFrom = o3tl::Length::in;
                    break;
                case u'm':
                    if (wordEndsWith(rString.substr(nPos + 1), "m"))
                        eFrom = o3tl::Length::mm;
                    break;
                case u'p':
                    if (wordEndsWith(rString.substr(nPos + 1), "t"))
                        eFrom = o3tl::Length::pt;
                    else if (wordEndsWith(rString.substr(nPos + 1), "c"))
                        eFrom = o3tl::Length::pc;
                    break;
                }
            }
            else if( MeasureUnit::MM_100TH == nTargetUnit || MeasureUnit::MM_10TH == nTargetUnit )
            {
                switch (rtl::toAsciiLowerCase<sal_uInt32>(rString[nPos]))
                {
                case u'c':
                    if (wordEndsWith(rString.substr(nPos + 1), "m"))
                        eFrom = o3tl::Length::cm;
                    break;
                case u'i':
                    if (wordEndsWith(rString.substr(nPos + 1), "n"))
                        eFrom = o3tl::Length::in;
                    break;
                case u'm':
                    if (wordEndsWith(rString.substr(nPos + 1), "m"))
                        eFrom = o3tl::Length::mm;
                    break;
                case u'p':
                    if (wordEndsWith(rString.substr(nPos + 1), "t"))
                        eFrom = o3tl::Length::pt;
                    else if (wordEndsWith(rString.substr(nPos + 1), "c"))
                        eFrom = o3tl::Length::pc;
                    else if (wordEndsWith(rString.substr(nPos + 1), "x"))
                        eFrom = o3tl::Length::px;
                    break;
                }
            }
            else if( MeasureUnit::POINT == nTargetUnit )
            {
                if (wordEndsWith(rString.substr(nPos), "pt"))
                    eFrom = o3tl::Length::pt;
            }

            if (eFrom == o3tl::Length::invalid)
                return false;

            // TODO: check overflow
            nVal = o3tl::convert(nVal, eFrom, Measure2O3tlUnit(nTargetUnit));
        }
    }

    nVal += .5;
    if( bNeg )
        nVal = -nVal;

    if( nVal <= static_cast<double>(nMin) )
        rValue = nMin;
    else if( nVal >= static_cast<double>(nMax) )
        rValue = nMax;
    else
        rValue = static_cast<sal_Int32>(nVal);

    return true;
}

/** convert string to measure using optional min and max values*/
bool Converter::convertMeasure( sal_Int32& rValue,
                                std::u16string_view rString,
                                sal_Int16 nTargetUnit /* = MeasureUnit::MM_100TH */,
                                sal_Int32 nMin /* = SAL_MIN_INT32 */,
                                sal_Int32 nMax /* = SAL_MAX_INT32 */ )
{
    return lcl_convertMeasure(rValue, rString, nTargetUnit, nMin, nMax);
}

/** convert string to measure using optional min and max values*/
bool Converter::convertMeasure( sal_Int32& rValue,
                                std::string_view rString,
                                sal_Int16 nTargetUnit /* = MeasureUnit::MM_100TH */,
                                sal_Int32 nMin /* = SAL_MIN_INT32 */,
                                sal_Int32 nMax /* = SAL_MAX_INT32 */ )
{
    return lcl_convertMeasure(rValue, rString, nTargetUnit, nMin, nMax);
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
        rBuffer.append( '%' );

        return;
    }
    sal_Int64 nValue(nMeasure); // extend to 64-bit first to avoid overflow
    // the sign is processed separately
    if (nValue < 0)
    {
        nValue = -nValue;
        rBuffer.append( '-' );
    }

    o3tl::Length eFrom = o3tl::Length::in, eTo = o3tl::Length::in;
    int nFac = 100; // used to get specific number of decimals (2 by default)
    std::string_view psUnit;
    switch( nSourceUnit )
    {
    case MeasureUnit::TWIP:
        eFrom = o3tl::Length::twip;
        switch( nTargetUnit )
        {
        case MeasureUnit::MM_100TH:
        case MeasureUnit::MM_10TH:
            OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,"output unit not supported for twip values" );
            [[fallthrough]];
        case MeasureUnit::MM:
            eTo = o3tl::Length::mm;
            nFac = 100;
            psUnit = gpsMM;
            break;

        case MeasureUnit::CM:
            eTo = o3tl::Length::cm;
            nFac = 1000;
            psUnit = gpsCM;
            break;

        case MeasureUnit::POINT:
            eTo = o3tl::Length::pt;
            nFac = 100;
            psUnit = gpsPT;
            break;

        case MeasureUnit::INCH:
        default:
            OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,
                        "output unit not supported for twip values" );
            nFac = 10000;
            psUnit = gpsINCH;
            break;
        }
        break;

    case MeasureUnit::POINT:
        // 1pt = 1pt (exactly)
        OSL_ENSURE( MeasureUnit::POINT == nTargetUnit,
                    "output unit not supported for pt values" );
        eFrom = eTo = o3tl::Length::pt;
        nFac = 1;
        psUnit = gpsPT;
        break;
    case MeasureUnit::MM_10TH:
    case MeasureUnit::MM_100TH:
        {
            int nFac2 = (MeasureUnit::MM_100TH == nSourceUnit) ? 100 : 10;
            eFrom = Measure2O3tlUnit(nSourceUnit);
            switch( nTargetUnit )
            {
            case MeasureUnit::MM_100TH:
            case MeasureUnit::MM_10TH:
                OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,
                            "output unit not supported for 1/100mm values" );
                [[fallthrough]];
            case MeasureUnit::MM:
                eTo = o3tl::Length::mm;
                nFac = nFac2;
                psUnit = gpsMM;
                break;

            case MeasureUnit::CM:
                eTo = o3tl::Length::cm;
                nFac = 10*nFac2;
                psUnit = gpsCM;
                break;

            case MeasureUnit::POINT:
                eTo = o3tl::Length::pt;
                nFac = nFac2;
                psUnit = gpsPT;
                break;

            case MeasureUnit::INCH:
            default:
                OSL_ENSURE( MeasureUnit::INCH == nTargetUnit,
                            "output unit not supported for 1/100mm values" );
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

    nValue = o3tl::convert(nValue * nFac, eFrom, eTo);

    rBuffer.append( static_cast<sal_Int64>(nValue / nFac) );
    if (nFac > 1 && (nValue % nFac) != 0)
    {
        rBuffer.append( '.' );
        while (nFac > 1 && (nValue % nFac) != 0)
        {
            nFac /= 10;
            rBuffer.append( static_cast<sal_Int32>((nValue / nFac) % 10) );
        }
    }

    if (psUnit.length() > 0)
        rBuffer.appendAscii(psUnit.data(), psUnit.length());
}

/** convert string to boolean */
bool Converter::convertBool( bool& rBool, std::u16string_view rString )
{
    rBool = rString == u"true";

    return rBool || (rString == u"false");
}

/** convert string to boolean */
bool Converter::convertBool( bool& rBool, std::string_view rString )
{
    rBool = rString == "true";

    return rBool || (rString == "false");
}

/** convert boolean to string */
void Converter::convertBool( OUStringBuffer& rBuffer, bool bValue )
{
    rBuffer.append( bValue );
}

/** convert string to percent */
bool Converter::convertPercent( sal_Int32& rPercent, std::u16string_view rString )
{
    return convertMeasure( rPercent, rString, MeasureUnit::PERCENT );
}

/** convert string to percent */
bool Converter::convertPercent( sal_Int32& rPercent, std::string_view rString )
{
    return convertMeasure( rPercent, rString, MeasureUnit::PERCENT );
}

/** convert percent to string */
void Converter::convertPercent( OUStringBuffer& rBuffer, sal_Int32 nValue )
{
    rBuffer.append( nValue );
    rBuffer.append( '%' );
}

/** convert string to pixel measure */
bool Converter::convertMeasurePx( sal_Int32& rPixel, std::u16string_view rString )
{
    return convertMeasure( rPixel, rString, MeasureUnit::PIXEL );
}

/** convert string to pixel measure */
bool Converter::convertMeasurePx( sal_Int32& rPixel, std::string_view rString )
{
    return convertMeasure( rPixel, rString, MeasureUnit::PIXEL );
}

/** convert pixel measure to string */
void Converter::convertMeasurePx( OUStringBuffer& rBuffer, sal_Int32 nValue )
{
    rBuffer.append( nValue );
    rBuffer.append( 'p' );
    rBuffer.append( 'x' );
}

static int lcl_gethex( int nChar )
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
template<typename V>
static bool lcl_convertColor( sal_Int32& rColor, V rValue )
{
    if( rValue.size() != 7 || rValue[0] != '#' )
        return false;

    rColor = lcl_gethex( rValue[1] ) * 16 + lcl_gethex( rValue[2] );
    rColor <<= 8;

    rColor |= lcl_gethex( rValue[3] ) * 16 + lcl_gethex( rValue[4] );
    rColor <<= 8;

    rColor |= lcl_gethex( rValue[5] ) * 16 + lcl_gethex( rValue[6] );

    return true;
}

/** convert string to rgb color */
bool Converter::convertColor( sal_Int32& rColor, std::u16string_view rValue )
{
    return lcl_convertColor(rColor, rValue);
}

/** convert string to rgb color */
bool Converter::convertColor( sal_Int32& rColor, std::string_view rValue )
{
    return lcl_convertColor(rColor, rValue);
}

const char aHexTab[] = "0123456789abcdef";

/** convert color to string */
void Converter::convertColor( OUStringBuffer& rBuffer, sal_Int32 nColor )
{
    rBuffer.append( '#' );

    sal_uInt8 nCol = static_cast<sal_uInt8>(nColor >> 16);
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );

    nCol = static_cast<sal_uInt8>(nColor >> 8);
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );

    nCol = static_cast<sal_uInt8>(nColor);
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );
}

/** convert string to number with optional min and max values */
bool Converter::convertNumber(  sal_Int32& rValue,
                                std::u16string_view aString,
                                sal_Int32 nMin, sal_Int32 nMax )
{
    rValue = 0;
    sal_Int64 nNumber = 0;
    bool bRet = convertNumber64(nNumber,aString,nMin,nMax);
    if ( bRet )
        rValue = static_cast<sal_Int32>(nNumber);
    return bRet;
}

/** convert string to number with optional min and max values */
bool Converter::convertNumber(  sal_Int32& rValue,
                                std::string_view aString,
                                sal_Int32 nMin, sal_Int32 nMax )
{
    rValue = 0;
    sal_Int64 nNumber = 0;
    bool bRet = convertNumber64(nNumber,aString,nMin,nMax);
    if ( bRet )
        rValue = static_cast<sal_Int32>(nNumber);
    return bRet;
}

/** convert string to 64-bit number with optional min and max values */
template<typename V>
static bool lcl_convertNumber64( sal_Int64& rValue,
                                 V aString,
                                 sal_Int64 nMin, sal_Int64 nMax )
{
    sal_Int32 nPos = 0;
    sal_Int32 const nLen = aString.size();

    // skip white space
    while( (nPos < nLen) && (aString[nPos] <= ' ') )
        nPos++;

    sal_Int32 nNumberStartPos = nPos;

    if( nPos < nLen && '-' == aString[nPos] )
    {
        nPos++;
    }

    // get number
    while( nPos < nLen &&
           '0' <= aString[nPos] &&
           '9' >= aString[nPos] )
    {
        nPos++;
    }

    rValue = toInt64_WithLength(aString.data() + nNumberStartPos, 10, nPos - nNumberStartPos);

    if( rValue < nMin )
        rValue = nMin;
    else if( rValue > nMax )
        rValue = nMax;

    return ( nPos == nLen && rValue >= nMin && rValue <= nMax );
}

/** convert string to 64-bit number with optional min and max values */
bool Converter::convertNumber64( sal_Int64& rValue,
                                 std::u16string_view aString,
                                 sal_Int64 nMin, sal_Int64 nMax )
{
    return lcl_convertNumber64(rValue, aString, nMin, nMax);
}

/** convert string to 64-bit number with optional min and max values */
bool Converter::convertNumber64( sal_Int64& rValue,
                                 std::string_view aString,
                                 sal_Int64 nMin, sal_Int64 nMax )
{
    return lcl_convertNumber64(rValue, aString, nMin, nMax);
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
            rBuffer.append('%');
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
void Converter::convertDouble( OUStringBuffer& rBuffer, double fNumber)
{
    ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true);
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue,
    std::u16string_view rString, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
{
    if (!convertDouble(rValue, rString))
        return false;

    OUStringBuffer sUnit;
    // fdo#48969: switch source and target because factor is used to divide!
    double const fFactor =
        GetConversionFactor(sUnit, nTargetUnit, nSourceUnit);
    if(fFactor != 1.0 && fFactor != 0.0)
        rValue /= fFactor;
    return true;
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue,
    std::string_view rString, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
{
    if (!convertDouble(rValue, rString))
        return false;

    OStringBuffer sUnit;
    // fdo#48969: switch source and target because factor is used to divide!
    double const fFactor =
        GetConversionFactor(sUnit, nTargetUnit, nSourceUnit);
    if(fFactor != 1.0 && fFactor != 0.0)
        rValue /= fFactor;
    return true;
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue, std::u16string_view rString)
{
    rtl_math_ConversionStatus eStatus;
    rValue = rtl_math_uStringToDouble(rString.data(),
                                     rString.data() + rString.size(),
                                     /*cDecSeparator*/'.', /*cGroupSeparator*/',',
                                     &eStatus, nullptr);
    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert string to double number (using ::rtl::math) */
bool Converter::convertDouble(double& rValue, std::string_view rString)
{
    rtl_math_ConversionStatus eStatus;
    rValue = rtl_math_stringToDouble(rString.data(),
                                     rString.data() + rString.size(),
                                     /*cDecSeparator*/'.', /*cGroupSeparator*/',',
                                     &eStatus, nullptr);
    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert number, 10th of degrees with range [0..3600] to SVG angle */
void Converter::convertAngle(OUStringBuffer& rBuffer, sal_Int16 const nAngle,
        SvtSaveOptions::ODFSaneDefaultVersion const nVersion)
{
    if (nVersion < SvtSaveOptions::ODFSVER_012 || nVersion == SvtSaveOptions::ODFSVER_012_EXT_COMPAT)
    {
        // wrong, but backward compatible with OOo/LO < 4.4
        rBuffer.append(static_cast<sal_Int32>(nAngle));
    }
    else
    { // OFFICE-3774 tdf#89475 write valid ODF 1.2 angle; needs LO 4.4 to import
        double fAngle(double(nAngle) / 10.0);
        ::sax::Converter::convertDouble(rBuffer, fAngle);
        rBuffer.append("deg");
    }
}

/** convert SVG angle to number, 10th of degrees with range [0..3600] */
bool Converter::convertAngle(sal_Int16& rAngle, std::u16string_view rString,
        bool const isWrongOOo10thDegAngle)
{
    // ODF 1.1 leaves it undefined what the number means, but ODF 1.2 says it's
    // degrees, while OOo has historically used 10th of degrees :(
    // So import degrees when we see the "deg" suffix but continue with 10th of
    // degrees for now for the sake of existing OOo/LO documents, until the
    // new versions that can read "deg" suffix are widely deployed and we can
    // start to write the "deg" suffix.
    sal_Int32 nValue(0);
    double fValue(0.0);
    bool bRet = ::sax::Converter::convertDouble(fValue, rString);
    if (std::u16string_view::npos != rString.find(u"deg"))
    {
        nValue = fValue * 10.0;
    }
    else if (std::u16string_view::npos != rString.find(u"grad"))
    {
        nValue = (fValue * 9.0 / 10.0) * 10.0;
    }
    else if (std::u16string_view::npos != rString.find(u"rad"))
    {
        nValue = basegfx::rad2deg<10>(fValue);
    }
    else // no explicit unit
    {
        if (isWrongOOo10thDegAngle)
        {
            nValue = fValue; // wrong, but backward compatible with OOo/LO < 7.0
        }
        else
        {
            nValue = fValue * 10.0; // ODF 1.2
        }
    }
    // limit to valid range [0..3600]
    nValue = nValue % 3600;
    if (nValue < 0)
    {
        nValue += 3600;
    }
    assert(0 <= nValue && nValue <= 3600);
    if (bRet)
    {
        rAngle = sal::static_int_cast<sal_Int16>(nValue);
    }
    return bRet;
}

/** convert SVG angle to number, 10th of degrees with range [0..3600] */
bool Converter::convertAngle(sal_Int16& rAngle, std::string_view rString,
        bool const isWrongOOo10thDegAngle)
{
    // ODF 1.1 leaves it undefined what the number means, but ODF 1.2 says it's
    // degrees, while OOo has historically used 10th of degrees :(
    // So import degrees when we see the "deg" suffix but continue with 10th of
    // degrees for now for the sake of existing OOo/LO documents, until the
    // new versions that can read "deg" suffix are widely deployed and we can
    // start to write the "deg" suffix.
    sal_Int32 nValue(0);
    double fValue(0.0);
    bool bRet = ::sax::Converter::convertDouble(fValue, rString);
    if (std::string_view::npos != rString.find("deg"))
    {
        nValue = fValue * 10.0;
    }
    else if (std::string_view::npos != rString.find("grad"))
    {
        nValue = (fValue * 9.0 / 10.0) * 10.0;
    }
    else if (std::string_view::npos != rString.find("rad"))
    {
        nValue = basegfx::rad2deg<10>(fValue);
    }
    else // no explicit unit
    {
        if (isWrongOOo10thDegAngle)
        {
            nValue = fValue; // wrong, but backward compatible with OOo/LO < 7.0
        }
        else
        {
            nValue = fValue * 10.0; // ODF 1.2
        }
    }
    // limit to valid range [0..3600]
    nValue = nValue % 3600;
    if (nValue < 0)
    {
        nValue += 3600;
    }
    assert(0 <= nValue && nValue <= 3600);
    if (bRet)
    {
        rAngle = sal::static_int_cast<sal_Int16>(nValue);
    }
    return bRet;
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
        rBuffer.append('-');
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
        rBuffer.append( '0');
    rBuffer.append( sal_Int32( fHoursValue));
    rBuffer.append( 'H');
    if (fMinsValue < 10)
        rBuffer.append( '0');
    rBuffer.append( sal_Int32( fMinsValue));
    rBuffer.append( 'M');
    if (fSecsValue < 10)
        rBuffer.append( '0');
    rBuffer.append( sal_Int32( fSecsValue));
    if (fNanoSecsValue > 0.0)
    {
        OUString aNS( ::rtl::math::doubleToUString( fValue,
                    rtl_math_StringFormat_F, XML_MAXDIGITSCOUNT_TIME - 5, '.',
                    true));
        if ( aNS.getLength() > 2 )
        {
            rBuffer.append( '.');
            rBuffer.append( aNS.subView(2) );     // strip "0."
        }
    }
    rBuffer.append( 'S');
}

/** helper function of Converter::convertDuration */
template<typename V>
static bool convertDurationHelper(double& rfTime, V pStr)
{
    // negative time duration?
    bool bIsNegativeDuration = false;
    if ( '-' == (*pStr) )
    {
        bIsNegativeDuration = true;
        pStr++;
    }

    if ( *pStr != 'P' && *pStr != 'p' )            // duration must start with "P"
        return false;
    pStr++;

    OUStringBuffer sDoubleStr;
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
        else if ( '0' <= c && '9' >= c )
        {
            if ( nTemp >= SAL_MAX_INT32 / 10 )
                bSuccess = false;
            else
            {
                if ( !bIsFraction )
                {
                    nTemp *= 10;
                    nTemp += (c - u'0');
                }
                else
                {
                    sDoubleStr.append(c);
                }
            }
        }
        else if ( bTimePart )
        {
            if ( c == 'H' || c == 'h' )
            {
                nHours = nTemp;
                nTemp = 0;
            }
            else if ( c == 'M' || c == 'm')
            {
                nMins = nTemp;
                nTemp = 0;
            }
            else if ( (c == ',') || (c == '.') )
            {
                nSecs = nTemp;
                nTemp = 0;
                bIsFraction = true;
                sDoubleStr = "0.";
            }
            else if ( c == 'S' || c == 's' )
            {
                if ( !bIsFraction )
                {
                    nSecs = nTemp;
                    nTemp = 0;
                    sDoubleStr = "0.0";
                }
            }
            else
                bSuccess = false;               // invalid character
        }
        else
        {
            if ( c == 'T' || c == 't' )            // "T" starts time part
                bTimePart = true;
            else if ( c == 'D' || c == 'd')
            {
                nDays = nTemp;
                nTemp = 0;
            }
            else if ( c == 'Y' || c == 'y' || c == 'M' || c == 'm' )
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
        double fHour = nHours;
        double fMin = nMins;
        double fSec = nSecs;
        double fFraction = o3tl::toDouble(sDoubleStr);
        double fTempTime = fHour / 24;
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

/** convert ISO "duration" string to double; negative durations allowed */
bool Converter::convertDuration(double& rfTime,
                                std::string_view rString)
{
    std::string_view aTrimmed = o3tl::trim(rString);
    const char* pStr = aTrimmed.data();

    return convertDurationHelper(rfTime, pStr);
}

/** convert util::Duration to ISO8601 "duration" string */
void Converter::convertDuration(OUStringBuffer& rBuffer,
        const ::util::Duration& rDuration)
{
    if (rDuration.Negative)
    {
        rBuffer.append('-');
    }
    rBuffer.append('P');
    const bool bHaveDate(rDuration.Years  != 0 ||
                         rDuration.Months != 0 ||
                         rDuration.Days   != 0);
    if (rDuration.Years)
    {
        rBuffer.append(static_cast<sal_Int32>(rDuration.Years));
        rBuffer.append('Y');
    }
    if (rDuration.Months)
    {
        rBuffer.append(static_cast<sal_Int32>(rDuration.Months));
        rBuffer.append('M');
    }
    if (rDuration.Days)
    {
        rBuffer.append(static_cast<sal_Int32>(rDuration.Days));
        rBuffer.append('D');
    }
    if ( rDuration.Hours != 0
         || rDuration.Minutes != 0
         || rDuration.Seconds != 0
         || rDuration.NanoSeconds != 0 )
    {
        rBuffer.append('T'); // time separator
        if (rDuration.Hours)
        {
            rBuffer.append(static_cast<sal_Int32>(rDuration.Hours));
            rBuffer.append('H');
        }
        if (rDuration.Minutes)
        {
            rBuffer.append(static_cast<sal_Int32>(rDuration.Minutes));
            rBuffer.append('M');
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
                rBuffer.appendAscii(ostr.str().c_str());
            }
            rBuffer.append('S');
        }
    }
    else if (!bHaveDate)
    {
        // zero duration: XMLSchema-2 says there must be at least one component
        rBuffer.append('0');
        rBuffer.append('D');
    }
}

namespace {

enum Result { R_NOTHING, R_OVERFLOW, R_SUCCESS };

}

template <typename V>
static Result
readUnsignedNumber(V rString,
    size_t & io_rnPos, sal_Int32 & o_rNumber)
{
    size_t nPos(io_rnPos);

    while (nPos < rString.size())
    {
        const typename V::value_type c = rString[nPos];
        if (('0' > c) || (c > '9'))
            break;
        ++nPos;
    }

    if (io_rnPos == nPos) // read something?
    {
        o_rNumber = -1;
        return R_NOTHING;
    }

    const sal_Int64 nTemp = toInt64_WithLength(rString.data() + io_rnPos, 10, nPos - io_rnPos);

    const bool bOverflow = (nTemp >= SAL_MAX_INT32);

    io_rnPos = nPos;
    o_rNumber = nTemp;
    return bOverflow ? R_OVERFLOW : R_SUCCESS;
}

template<typename V>
static Result
readUnsignedNumberMaxDigits(int maxDigits,
                            V rString, size_t & io_rnPos,
                            sal_Int32 & o_rNumber)
{
    bool bOverflow(false);
    sal_Int64 nTemp(0);
    size_t nPos(io_rnPos);
    OSL_ENSURE(maxDigits >= 0, "negative amount of digits makes no sense");

    while (nPos < rString.size())
    {
        const sal_Unicode c = rString[nPos];
        if (('0' <= c) && (c <= '9'))
        {
            if (maxDigits > 0)
            {
                nTemp *= 10;
                nTemp += (c - u'0');
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
    return bOverflow ? R_OVERFLOW : R_SUCCESS;
}

template<typename V>
static bool
readDurationT(V rString, size_t & io_rnPos)
{
    if ((io_rnPos < rString.size()) &&
        (rString[io_rnPos] == 'T' || rString[io_rnPos] == 't'))
    {
        ++io_rnPos;
        return true;
    }
    return false;
}

template<typename V>
static bool
readDurationComponent(V rString,
    size_t & io_rnPos, sal_Int32 & io_rnTemp, bool & io_rbTimePart,
    sal_Int32 & o_rnTarget, const sal_Unicode cLower, const sal_Unicode cUpper)
{
    if (io_rnPos < rString.size())
    {
        if (cLower == rString[io_rnPos] || cUpper == rString[io_rnPos])
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

template <typename V>
static bool convertDurationHelper(util::Duration& rDuration, V string)
{
    size_t nPos(0);

    bool bIsNegativeDuration(false);
    if (!string.empty() && ('-' == string[0]))
    {
        bIsNegativeDuration = true;
        ++nPos;
    }

    if (nPos < string.size()
        && string[nPos] != 'P' && string[nPos] != 'p') // duration must start with "P"
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
                     nYears, 'y', 'Y');
    }

    if (!bTimePart && bSuccess)
    {
        bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                     nMonths, 'm', 'M');
    }

    if (!bTimePart && bSuccess)
    {
        bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                     nDays, 'd', 'D');
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
                         nHours, 'h', 'H');
        }

        if (bSuccess)
        {
            bSuccess = readDurationComponent(string, nPos, nTemp, bTimePart,
                         nMinutes, 'm', 'M');
        }

        // eeek! seconds are icky.
        if ((nPos < string.size()) && bSuccess)
        {
            if (string[nPos] == '.' ||
                string[nPos] == ',')
            {
                ++nPos;
                if (-1 != nTemp)
                {
                    nSeconds = nTemp;
                    nTemp = -1;
                    const sal_Int32 nStart(nPos);
                    bSuccess = readUnsignedNumberMaxDigits(9, string, nPos, nTemp) == R_SUCCESS;
                    if ((nPos < string.size()) && bSuccess)
                    {
                        if (-1 != nTemp)
                        {
                            nNanoSeconds = nTemp;
                            sal_Int32 nDigits = nPos - nStart;
                            assert(nDigits >= 0);
                            for (; nDigits < 9; ++nDigits)
                            {
                                nNanoSeconds *= 10;
                            }
                            nTemp=-1;
                            if ('S' == string[nPos] || 's' == string[nPos])
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
            else if ('S' == string[nPos] || 's' == string[nPos])
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

    if (nPos != string.size()) // string not processed completely?
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
        rDuration.NanoSeconds   = nNanoSeconds;
    }

    return bSuccess;
}

/** convert ISO8601 "duration" string to util::Duration */
bool Converter::convertDuration(util::Duration& rDuration,
                                std::u16string_view rString)
{
    return convertDurationHelper(rDuration, o3tl::trim(rString));
}

/** convert ISO8601 "duration" string to util::Duration */
bool Converter::convertDuration(util::Duration& rDuration,
                                std::string_view rString)
{
    return convertDurationHelper(rDuration, o3tl::trim(rString));
}

static void
lcl_AppendTimezone(OUStringBuffer & i_rBuffer, int const nOffset)
{
    if (0 == nOffset)
    {
        i_rBuffer.append('Z');
    }
    else
    {
        if (0 < nOffset)
        {
            i_rBuffer.append('+');
        }
        else
        {
            i_rBuffer.append('-');
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
    convertDateTime(i_rBuffer, dt, pTimeZoneOffset);
}

static void convertTime(
        OUStringBuffer& i_rBuffer,
        const css::util::DateTime& i_rDateTime)
{
    if (i_rDateTime.Hours   < 10) {
        i_rBuffer.append('0');
    }
    i_rBuffer.append( OUString::number(static_cast<sal_Int32>(i_rDateTime.Hours)) + ":");
    if (i_rDateTime.Minutes < 10) {
        i_rBuffer.append('0');
    }
    i_rBuffer.append( OUString::number(static_cast<sal_Int32>(i_rDateTime.Minutes) ) + ":");
    if (i_rDateTime.Seconds < 10) {
        i_rBuffer.append('0');
    }
    i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Seconds) );
    if (i_rDateTime.NanoSeconds > 0) {
        OSL_ENSURE(i_rDateTime.NanoSeconds < 1000000000,"NanoSeconds cannot be more than 999 999 999");
        i_rBuffer.append('.');
        std::ostringstream ostr;
        ostr.fill('0');
        ostr.width(9);
        ostr << i_rDateTime.NanoSeconds;
        i_rBuffer.appendAscii(ostr.str().c_str());
    }
}

static void convertTimeZone(
        OUStringBuffer& i_rBuffer,
        const css::util::DateTime& i_rDateTime,
        sal_Int16 const* pTimeZoneOffset)
{
    if (pTimeZoneOffset)
    {
        lcl_AppendTimezone(i_rBuffer, *pTimeZoneOffset);
    }
    else if (i_rDateTime.IsUTC)
    {
        lcl_AppendTimezone(i_rBuffer, 0);
    }
}

/** convert util::DateTime to ISO "time" or "dateTime" string */
void Converter::convertTimeOrDateTime(
        OUStringBuffer& i_rBuffer,
        const css::util::DateTime& i_rDateTime)
{
    if (i_rDateTime.Year == 0 ||
        i_rDateTime.Month < 1 || i_rDateTime.Month > 12 ||
        i_rDateTime.Day < 1 || i_rDateTime.Day > 31)
    {
        convertTime(i_rBuffer, i_rDateTime);
        convertTimeZone(i_rBuffer, i_rDateTime, nullptr);
    }
    else
    {
        convertDateTime(i_rBuffer, i_rDateTime, nullptr, true);
    }
}

/** convert util::DateTime to ISO "date" or "dateTime" string */
void Converter::convertDateTime(
        OUStringBuffer& i_rBuffer,
        const css::util::DateTime& i_rDateTime,
        sal_Int16 const*const pTimeZoneOffset,
        bool i_bAddTimeIf0AM )
{
    const sal_Unicode dash('-');
    const sal_Unicode zero('0');

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
    i_rBuffer.append( OUString::number(nYear) + OUStringChar(dash) );
    if( i_rDateTime.Month < 10 ) {
        i_rBuffer.append(zero);
    }
    i_rBuffer.append( OUString::number(i_rDateTime.Month) + OUStringChar(dash) );
    if( i_rDateTime.Day   < 10 ) {
        i_rBuffer.append(zero);
    }
    i_rBuffer.append( static_cast<sal_Int32>(i_rDateTime.Day)   );

    if( i_rDateTime.Seconds != 0 ||
        i_rDateTime.Minutes != 0 ||
        i_rDateTime.Hours   != 0 ||
        i_bAddTimeIf0AM )
    {
        i_rBuffer.append('T');
        convertTime(i_rBuffer, i_rDateTime);
    }

    convertTimeZone(i_rBuffer, i_rDateTime, pTimeZoneOffset);
}

/** convert ISO "date" or "dateTime" string to util::DateTime */
bool Converter::parseDateTime(   util::DateTime& rDateTime,
                                 std::u16string_view rString )
{
    bool isDateTime;
    return parseDateOrDateTime(nullptr, rDateTime, isDateTime, nullptr,
            rString);
}

/** convert ISO "date" or "dateTime" string to util::DateTime */
bool Converter::parseDateTime(   util::DateTime& rDateTime,
                                 std::string_view rString )
{
    bool isDateTime;
    return parseDateOrDateTime(nullptr, rDateTime, isDateTime, nullptr,
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
    assert(0 < nMonth && nMonth <= 12);
    if ((2 == nMonth) && lcl_isLeapYear(nYear))
    {
        return 29;
    }
    return s_MaxDaysPerMonth[nMonth - 1];
}

static void lcl_ConvertToUTC(
        sal_Int16 & o_rYear, sal_uInt16 & o_rMonth, sal_uInt16 & o_rDay,
        sal_uInt16 & o_rHours, sal_uInt16 & o_rMinutes,
        int const nSourceOffset)
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
        sal_Int16 nDayAdd(0);
        while (24 <= o_rHours)
        {
            o_rHours -= 24;
            ++nDayAdd;
        }
        if (o_rDay == 0)
        {
            return; // handle time without date - don't adjust what isn't there
        }
        o_rDay += nDayAdd;
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
        if (o_rDay == 0)
        {
            return; // handle time without date - don't adjust what isn't there
        }
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

template <typename V>
static bool
readDateTimeComponent(V rString,
    size_t & io_rnPos, sal_Int32 & o_rnTarget,
    const sal_Int32 nMinLength, const bool bExactLength)
{
    const size_t nOldPos(io_rnPos);
    sal_Int32 nTemp(0);
    if (R_SUCCESS != readUnsignedNumber<V>(rString, io_rnPos, nTemp))
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
template<typename V>
static bool lcl_parseDate(
                bool & isNegative,
                sal_Int32 & nYear, sal_Int32 & nMonth, sal_Int32 & nDay,
                bool & bHaveTime,
                size_t & nPos,
                V string,
                bool const bIgnoreInvalidOrMissingDate)
{
    bool bSuccess = true;

    if (string.size() > nPos)
    {
        if ('-' == string[nPos])
        {
            isNegative = true;
            ++nPos;
        }
    }

    {
        // While W3C XMLSchema specifies years with a minimum of 4 digits, be
        // lenient in what we accept for years < 1000. One digit is acceptable
        // if the remainders match.
        bSuccess = readDateTimeComponent<V>(string, nPos, nYear, 1, false);
        if (!bIgnoreInvalidOrMissingDate)
        {
            bSuccess &= (0 < nYear);
        }
        bSuccess &= (nPos < string.size()); // not last token
    }
    if (bSuccess && ('-' != string[nPos])) // separator
    {
        bSuccess = false;
    }
    if (bSuccess)
    {
        ++nPos;

        bSuccess = readDateTimeComponent<V>(string, nPos, nMonth, 2, true);
        if (!bIgnoreInvalidOrMissingDate)
        {
            bSuccess &= (0 < nMonth);
        }
        bSuccess &= (nMonth <= 12);
        bSuccess &= (nPos < string.size()); // not last token
    }
    if (bSuccess && ('-' != string[nPos])) // separator
    {
        bSuccess = false;
    }
    if (bSuccess)
    {
        ++nPos;

        bSuccess = readDateTimeComponent(string, nPos, nDay, 2, true);
        if (!bIgnoreInvalidOrMissingDate)
        {
            bSuccess &= (0 < nDay);
        }
        if (nMonth > 0) // not possible to check if month was missing
        {
            bSuccess &= (nDay <= lcl_MaxDaysPerMonth(nMonth, nYear));
        }
        else assert(bIgnoreInvalidOrMissingDate);
    }

    if (bSuccess && (nPos < string.size()))
    {
        if ('T' == string[nPos] || 't' == string[nPos]) // time separator
        {
            bHaveTime = true;
            ++nPos;
        }
    }

    return bSuccess;
}

/** convert ISO "date" or "dateTime" string to util::DateTime or util::Date */
template <typename V>
static bool lcl_parseDateTime(
                util::Date *const pDate, util::DateTime & rDateTime,
                bool & rbDateTime,
                std::optional<sal_Int16> *const pTimeZoneOffset,
                V string,
                bool const bIgnoreInvalidOrMissingDate)
{
    bool bSuccess = true;

    string = o3tl::trim(string);

    bool isNegative(false);
    sal_Int32 nYear(0);
    sal_Int32 nMonth(0);
    sal_Int32 nDay(0);
    size_t nPos(0);
    bool bHaveTime(false);

    if (    !bIgnoreInvalidOrMissingDate
        ||  string.find(':') == V::npos  // no time?
        ||  (string.find('-') != V::npos
             && string.find('-') < string.find(':')))
    {
        bSuccess &= lcl_parseDate<V>(isNegative, nYear, nMonth, nDay,
                bHaveTime, nPos, string, bIgnoreInvalidOrMissingDate);
    }
    else
    {
        bHaveTime = true;
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
            bSuccess &= (nPos < string.size()); // not last token
        }
        if (bSuccess && (':' != string[nPos])) // separator
        {
            bSuccess = false;
        }
        if (bSuccess)
        {
            ++nPos;

            bSuccess = readDateTimeComponent(string, nPos, nMinutes, 2, true);
            bSuccess &= (0 <= nMinutes) && (nMinutes < 60);
            bSuccess &= (nPos < string.size()); // not last token
        }
        if (bSuccess && (':' != string[nPos])) // separator
        {
            bSuccess = false;
        }
        if (bSuccess)
        {
            ++nPos;

            bSuccess = readDateTimeComponent(string, nPos, nSeconds, 2, true);
            bSuccess &= (0 <= nSeconds) && (nSeconds < 60);
        }
        if (bSuccess && (nPos < string.size()) &&
            ('.' == string[nPos] || ',' == string[nPos])) // fraction separator
        {
            ++nPos;
            const sal_Int32 nStart(nPos);
            sal_Int32 nTemp(0);
            if (R_NOTHING == readUnsignedNumberMaxDigits<V>(9, string, nPos, nTemp))
            {
                bSuccess = false;
            }
            if (bSuccess)
            {
                sal_Int32 nDigits = std::min<sal_Int32>(nPos - nStart, 9);
                assert(nDigits > 0);
                for (; nDigits < 9; ++nDigits)
                {
                    nTemp *= 10;
                }
                nNanoSeconds = nTemp;
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
    if (bSuccess && (nPos < string.size()))
    {
        const sal_Unicode c(string[nPos]);
        if ('+' == c)
        {
            bHaveTimezone = true;
            bHaveTimezonePlus = true;
            ++nPos;
        }
        else if ('-' == c)
        {
            bHaveTimezone = true;
            bHaveTimezoneMinus = true;
            ++nPos;
        }
        else if ('Z' == c || 'z' == c)
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
        bSuccess = readDateTimeComponent<V>(
                        string, nPos, nTimezoneHours, 2, true);
        bSuccess &= (0 <= nTimezoneHours) && (nTimezoneHours <= 14);
        bSuccess &= (nPos < string.size()); // not last token
        if (bSuccess && (':' != string[nPos])) // separator
        {
            bSuccess = false;
        }
        if (bSuccess)
        {
            ++nPos;

            bSuccess = readDateTimeComponent<V>(
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

    bSuccess &= (nPos == string.size()); // trailing junk?

    if (bSuccess)
    {
        sal_Int16 const nTimezoneOffset = (bHaveTimezoneMinus ? -1 : +1)
                        * ((nTimezoneHours * 60) + nTimezoneMinutes);
        if (!pDate || bHaveTime) // time is optional
        {
            rDateTime.Year =
                (isNegative ? -1 : +1) * static_cast<sal_Int16>(nYear);
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
                (isNegative ? -1 : +1) * static_cast<sal_Int16>(nYear);
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

/** convert ISO "time" or "dateTime" string to util::DateTime */
bool Converter::parseTimeOrDateTime(
                util::DateTime & rDateTime,
                std::u16string_view rString)
{
    bool dummy;
    return lcl_parseDateTime(
                nullptr, rDateTime, dummy, nullptr, rString, true);
}

/** convert ISO "time" or "dateTime" string to util::DateTime */
bool Converter::parseTimeOrDateTime(
                util::DateTime & rDateTime,
                std::string_view rString)
{
    bool dummy;
    return lcl_parseDateTime(
                nullptr, rDateTime, dummy, nullptr, rString, true);
}

/** convert ISO "date" or "dateTime" string to util::DateTime or util::Date */
bool Converter::parseDateOrDateTime(
                util::Date *const pDate, util::DateTime & rDateTime,
                bool & rbDateTime,
                std::optional<sal_Int16> *const pTimeZoneOffset,
                std::u16string_view rString )
{
    return lcl_parseDateTime(
                pDate, rDateTime, rbDateTime, pTimeZoneOffset, rString, false);
}

/** convert ISO "date" or "dateTime" string to util::DateTime or util::Date */
bool Converter::parseDateOrDateTime(
                util::Date *const pDate, util::DateTime & rDateTime,
                bool & rbDateTime,
                std::optional<sal_Int16> *const pTimeZoneOffset,
                std::string_view rString )
{
    return lcl_parseDateTime(
                pDate, rDateTime, rbDateTime, pTimeZoneOffset, rString, false);
}

/** gets the position of the first comma after npos in the string
    rStr. Commas inside '"' pairs are not matched */
sal_Int32 Converter::indexOfComma( std::u16string_view rStr,
                                            sal_Int32 nPos )
{
    sal_Unicode cQuote = 0;
    sal_Int32 nLen = rStr.size();
    for( ; nPos < nLen; nPos++ )
    {
        sal_Unicode c = rStr[nPos];
        switch( c )
        {
        case u'\'':
            if( 0 == cQuote )
                cQuote = c;
            else if( '\'' == cQuote )
                cQuote = 0;
            break;

        case u'"':
            if( 0 == cQuote )
                cQuote = c;
            else if( '\"' == cQuote )
                cQuote = 0;
            break;

        case u',':
            if( 0 == cQuote )
                return nPos;
            break;
        }
    }

    return -1;
}

double Converter::GetConversionFactor(OUStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
{
    double fRetval(1.0);
    rUnit.setLength(0);


    if(nSourceUnit != nTargetUnit)
    {
        const o3tl::Length eFrom = Measure2O3tlUnit(nSourceUnit);
        const o3tl::Length eTo = Measure2O3tlUnit(nTargetUnit);
        fRetval = o3tl::convert(1.0, eFrom, eTo);

        if (const auto sUnit = Measure2UnitString(nTargetUnit); sUnit.size() > 0)
            rUnit.appendAscii(sUnit.data(), sUnit.size());
    }

    return fRetval;
}

double Converter::GetConversionFactor(OStringBuffer& rUnit, sal_Int16 nSourceUnit, sal_Int16 nTargetUnit)
{
    double fRetval(1.0);
    rUnit.setLength(0);


    if(nSourceUnit != nTargetUnit)
    {
        const o3tl::Length eFrom = Measure2O3tlUnit(nSourceUnit);
        const o3tl::Length eTo = Measure2O3tlUnit(nTargetUnit);
        fRetval = o3tl::convert(1.0, eFrom, eTo);

        if (const auto sUnit = Measure2UnitString(nTargetUnit); sUnit.size() > 0)
            rUnit.append(sUnit.data(), sUnit.size());
    }

    return fRetval;
}

template<typename V>
static sal_Int16 lcl_GetUnitFromString(V rString, sal_Int16 nDefaultUnit)
{
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rString.size();
    sal_Int16 nRetUnit = nDefaultUnit;

    // skip white space
    while( nPos < nLen && ' ' == rString[nPos] )
        nPos++;

    // skip negative
    if( nPos < nLen && '-' == rString[nPos] )
        nPos++;

    // skip number
    while( nPos < nLen && '0' <= rString[nPos] && '9' >= rString[nPos] )
        nPos++;

    if( nPos < nLen && '.' == rString[nPos] )
    {
        nPos++;
        while( nPos < nLen && '0' <= rString[nPos] && '9' >= rString[nPos] )
            nPos++;
    }

    // skip white space
    while( nPos < nLen && ' ' == rString[nPos] )
        nPos++;

    if( nPos < nLen )
    {
        switch(rString[nPos])
        {
            case '%' :
            {
                nRetUnit = MeasureUnit::PERCENT;
                break;
            }
            case 'c':
            case 'C':
            {
                if(nPos+1 < nLen && (rString[nPos+1] == 'm'
                    || rString[nPos+1] == 'M'))
                    nRetUnit = MeasureUnit::CM;
                break;
            }
            case 'e':
            case 'E':
            {
                // CSS1_EMS or CSS1_EMX later
                break;
            }
            case 'i':
            case 'I':
            {
                if(nPos+1 < nLen && (rString[nPos+1] == 'n'
                    || rString[nPos+1] == 'N'))
                    nRetUnit = MeasureUnit::INCH;
                break;
            }
            case 'm':
            case 'M':
            {
                if(nPos+1 < nLen && (rString[nPos+1] == 'm'
                    || rString[nPos+1] == 'M'))
                    nRetUnit = MeasureUnit::MM;
                break;
            }
            case 'p':
            case 'P':
            {
                if(nPos+1 < nLen && (rString[nPos+1] == 't'
                    || rString[nPos+1] == 'T'))
                    nRetUnit = MeasureUnit::POINT;
                if(nPos+1 < nLen && (rString[nPos+1] == 'c'
                    || rString[nPos+1] == 'C'))
                    nRetUnit = MeasureUnit::TWIP;
                break;
            }
        }
    }

    return nRetUnit;
}

sal_Int16 Converter::GetUnitFromString(std::u16string_view rString, sal_Int16 nDefaultUnit)
{
    return lcl_GetUnitFromString(rString, nDefaultUnit);
}
sal_Int16 Converter::GetUnitFromString(std::string_view rString, sal_Int16 nDefaultUnit)
{
    return lcl_GetUnitFromString(rString, nDefaultUnit);
}

bool Converter::convertAny(OUStringBuffer&    rsValue,
                           OUStringBuffer&    rsType ,
                           const css::uno::Any& rValue)
{
    bool bConverted = false;

    rsValue.setLength(0);
    rsType.setLength (0);

    switch (rValue.getValueTypeClass())
    {
        case css::uno::TypeClass_BYTE :
        case css::uno::TypeClass_SHORT :
        case css::uno::TypeClass_UNSIGNED_SHORT :
        case css::uno::TypeClass_LONG :
        case css::uno::TypeClass_UNSIGNED_LONG :
            {
                sal_Int32 nTempValue = 0;
                if (rValue >>= nTempValue)
                {
                    rsType.append("integer");
                    bConverted = true;
                    rsValue.append(nTempValue);
                }
            }
            break;

        case css::uno::TypeClass_BOOLEAN :
            {
                bool bTempValue = false;
                if (rValue >>= bTempValue)
                {
                    rsType.append("boolean");
                    bConverted = true;
                    ::sax::Converter::convertBool(rsValue, bTempValue);
                }
            }
            break;

        case css::uno::TypeClass_FLOAT :
        case css::uno::TypeClass_DOUBLE :
            {
                double fTempValue = 0.0;
                if (rValue >>= fTempValue)
                {
                    rsType.append("float");
                    bConverted = true;
                    ::sax::Converter::convertDouble(rsValue, fTempValue);
                }
            }
            break;

        case css::uno::TypeClass_STRING :
            {
                OUString sTempValue;
                if (rValue >>= sTempValue)
                {
                    rsType.append("string");
                    bConverted = true;
                    rsValue.append(sTempValue);
                }
            }
            break;

        case css::uno::TypeClass_STRUCT :
            {
                css::util::Date     aDate    ;
                css::util::Time     aTime    ;
                css::util::DateTime aDateTime;

                if (rValue >>= aDate)
                {
                    rsType.append("date");
                    bConverted = true;
                    css::util::DateTime aTempValue;
                    aTempValue.Day              = aDate.Day;
                    aTempValue.Month            = aDate.Month;
                    aTempValue.Year             = aDate.Year;
                    aTempValue.NanoSeconds = 0;
                    aTempValue.Seconds          = 0;
                    aTempValue.Minutes          = 0;
                    aTempValue.Hours            = 0;
                    ::sax::Converter::convertDateTime(rsValue, aTempValue, nullptr);
                }
                else
                if (rValue >>= aTime)
                {
                    rsType.append("time");
                    bConverted = true;
                    css::util::Duration aTempValue;
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
                    rsType.append("date");
                    bConverted = true;
                    ::sax::Converter::convertDateTime(rsValue, aDateTime, nullptr);
                }
            }
            break;
        default:
            break;
    }

    return bConverted;
}

void Converter::convertBytesToHexBinary(OUStringBuffer& rBuffer, const void* pBytes,
                                        sal_Int32 nBytes)
{
    rBuffer.setLength(0);
    rBuffer.ensureCapacity(nBytes * 2);
    auto pChars = static_cast<const unsigned char*>(pBytes);
    for (sal_Int32 i = 0; i < nBytes; ++i)
    {
        sal_Int32 c = *pChars++;
        if (c < 16)
            rBuffer.append('0');
        rBuffer.append(c, 16);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
