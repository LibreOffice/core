/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: converter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:41:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _COM_SUN_STAR_I18N_UNICODETYPE_HPP_
#include <com/sun/star/i18n/UnicodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _SAX_CONVERTER_HXX
#include "sax/tools/converter.hxx"
#endif

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
//using namespace com::sun::star::text;
//using namespace com::sun::star::style;
using namespace ::com::sun::star::i18n;

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

    sal_Int32 nPos = 0L;
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
#if 0
    if( nSourceUnit == MeasureUnit::PERCENT )
    {
        OSL_ENSURE( nTargetUnit == MeasureUnit::PERCENT,
                    "MeasureUnit::PERCENT only maps to MeasureUnit::PERCENT!" );

        rBuffer.append( nMeasure );
        rBuffer.append( sal_Unicode('%' ) );
    }
    else
    {
    // the sign is processed seperatly
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
    }

    long nLongVal = 0;
    bool bOutLongVal = true;
    if( nMeasure > SAL_INT32_MAX / nMul )
    {
        // A big int is required for calculation
        BigInt nBigVal( nMeasure );
        BigInt nBigFac( nFac );
        nBigVal *= nMul;
        nBigVal /= nDiv;
        nBigVal += 5;
        nBigVal /= 10;

        if( nBigVal.IsLong() )
        {
            // To convert the value into a string a long is sufficient
            nLongVal = (long)nBigVal;
        }
        else
        {
            BigInt nBigFac2( nFac );
            BigInt nBig10( 10 );
            rBuffer.append( (sal_Int32)(nBigVal / nBigFac2) );
            if( !(nBigVal % nBigFac2).IsZero() )
            {
                rBuffer.append( sal_Unicode('.') );
                while( nFac > 1 && !(nBigVal % nBigFac2).IsZero() )
                {
                    nFac /= 10;
                    nBigFac2 = nFac;
                    rBuffer.append( (sal_Int32)((nBigVal / nBigFac2) % nBig10 ) );
                }
            }
            bOutLongVal = false;
        }
    }
    else
    {
        nLongVal = nMeasure * nMul;
        nLongVal /= nDiv;
        nLongVal += 5;
        nLongVal /= 10;
    }

    if( bOutLongVal )
    {
        rBuffer.append( (sal_Int32)(nLongVal / nFac) );
        if( nFac > 1 && (nLongVal % nFac) != 0 )
        {
            rBuffer.append( sal_Unicode('.') );
            while( nFac > 1 && (nLongVal % nFac) != 0 )
            {
                nFac /= 10;
                rBuffer.append( (sal_Int32)((nLongVal / nFac) % 10) );
            }
        }
    }

    if( psUnit )
        rBuffer.appendAscii( psUnit );
    }
#endif
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

    sal_Int32 nPos = 0L;
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

/** convert double to ISO Time String; negative durations allowed */
void Converter::convertTime( ::rtl::OUStringBuffer& rBuffer,
                            const double& fTime)
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

/** convert ISO Time String to double; negative durations allowed */
bool Converter::convertTime( double& fTime,
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

        fTime = fTempTime;
    }
    return bSuccess;
}

/** convert util::DateTime to ISO Time String */
void Converter::convertTime( ::rtl::OUStringBuffer& rBuffer,
                            const ::com::sun::star::util::DateTime& rDateTime )
{
    double fHour = rDateTime.Hours;
    double fMin = rDateTime.Minutes;
    double fSec = rDateTime.Seconds;
    double fSec100 = rDateTime.HundredthSeconds;
    double fTempTime = fHour / 24;
    fTempTime += fMin / (24 * 60);
    fTempTime += fSec / (24 * 60 * 60);
    fTempTime += fSec100 / (24 * 60 * 60 * 100);
    convertTime( rBuffer, fTempTime );
}

/** convert ISO Time String to util::DateTime */
bool Converter::convertTime( ::com::sun::star::util::DateTime& rDateTime,
                             const ::rtl::OUString& rString )
{
    double fCalculatedTime = 0.0;
    if( convertTime( fCalculatedTime, rString ) )
    {
        // #101357# declare as volatile to prevent optimization
        // (gcc 3.0.1 Linux)
        volatile double fTempTime = fCalculatedTime;
        fTempTime *= 24;
        double fHoursValue = ::rtl::math::approxFloor (fTempTime);
        fTempTime -= fHoursValue;
        fTempTime *= 60;
        double fMinsValue = ::rtl::math::approxFloor (fTempTime);
        fTempTime -= fMinsValue;
        fTempTime *= 60;
        double fSecsValue = ::rtl::math::approxFloor (fTempTime);
        fTempTime -= fSecsValue;
        double f100SecsValue = 0.0;

        if( fTempTime > 0.00001 )
            f100SecsValue = fTempTime;

        rDateTime.Year = 0;
        rDateTime.Month = 0;
        rDateTime.Day = 0;
        rDateTime.Hours = static_cast < sal_uInt16 > ( fHoursValue );
        rDateTime.Minutes = static_cast < sal_uInt16 > ( fMinsValue );
        rDateTime.Seconds = static_cast < sal_uInt16 > ( fSecsValue );
        rDateTime.HundredthSeconds = static_cast < sal_uInt16 > ( f100SecsValue * 100.0 );

        return true;
    }
    return false;
}

/** convert util::DateTime to ISO Date String */
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

/** convert ISO Date String to util::DateTime */
bool Converter::convertDateTime( com::sun::star::util::DateTime& rDateTime,
                                     const ::rtl::OUString& rString )
{
    bool bSuccess = true;

    rtl::OUString aDateStr, aTimeStr, sDoubleStr;
    sal_Int32 nPos = rString.indexOf( (sal_Unicode) 'T' );
    sal_Int32 nPos2 = rString.indexOf( (sal_Unicode) ',' );
    if (nPos2 < 0)
        nPos2 = rString.indexOf( (sal_Unicode) '.' );
    if ( nPos >= 0 )
    {
        aDateStr = rString.copy( 0, nPos );
        if ( nPos2 >= 0 )
        {
            aTimeStr = rString.copy( nPos + 1, nPos2 - nPos - 1 );
            sDoubleStr = OUString(RTL_CONSTASCII_USTRINGPARAM("0."));
            sDoubleStr += rString.copy( nPos2 + 1 );
        }
        else
        {
            aTimeStr = rString.copy(nPos + 1);
            sDoubleStr = OUString(RTL_CONSTASCII_USTRINGPARAM("0.0"));
        }
    }
    else
        aDateStr = rString;         // no separator: only date part

    sal_Int32 nYear  = 1899;
    sal_Int32 nMonth = 12;
    sal_Int32 nDay   = 30;
    sal_Int32 nHour  = 0;
    sal_Int32 nMin   = 0;
    sal_Int32 nSec   = 0;

    const sal_Unicode* pStr = aDateStr.getStr();
    sal_Int32 nDateTokens = 1;
    while ( *pStr )
    {
        if ( *pStr == '-' )
            nDateTokens++;
        pStr++;
    }
    if ( nDateTokens > 3 || aDateStr.getLength() == 0 )
        bSuccess = false;
    else
    {
        sal_Int32 n = 0;
        if ( !convertNumber( nYear, aDateStr.getToken( 0, '-', n ), 0, 9999 ) )
            bSuccess = false;
        if ( nDateTokens >= 2 )
            if ( !convertNumber( nMonth, aDateStr.getToken( 0, '-', n ), 0, 12 ) )
                bSuccess = false;
        if ( nDateTokens >= 3 )
            if ( !convertNumber( nDay, aDateStr.getToken( 0, '-', n ), 0, 31 ) )
                bSuccess = false;
    }

    if ( aTimeStr.getLength() > 0 )           // time is optional
    {
        pStr = aTimeStr.getStr();
        sal_Int32 nTimeTokens = 1;
        while ( *pStr )
        {
            if ( *pStr == ':' )
                nTimeTokens++;
            pStr++;
        }
        if ( nTimeTokens > 3 )
            bSuccess = false;
        else
        {
            sal_Int32 n = 0;
            if ( !convertNumber( nHour, aTimeStr.getToken( 0, ':', n ), 0, 23 ) )
                bSuccess = false;
            if ( nTimeTokens >= 2 )
                if ( !convertNumber( nMin, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = false;
            if ( nTimeTokens >= 3 )
                if ( !convertNumber( nSec, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = false;
        }
    }

    if (bSuccess)
    {
        rDateTime.Year = (sal_uInt16)nYear;
        rDateTime.Month = (sal_uInt16)nMonth;
        rDateTime.Day = (sal_uInt16)nDay;
        rDateTime.Hours = (sal_uInt16)nHour;
        rDateTime.Minutes = (sal_uInt16)nMin;
        rDateTime.Seconds = (sal_uInt16)nSec;
        rDateTime.HundredthSeconds = (sal_uInt16)(sDoubleStr.toDouble() * 100);
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
