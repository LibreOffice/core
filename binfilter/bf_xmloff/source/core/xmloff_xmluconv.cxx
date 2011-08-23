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

#include <com/sun/star/util/DateTime.hpp>

#include <tools/debug.hxx>


#include "xmlehelp.hxx"


#include <xmluconv.hxx>


#include <rtl/math.hxx>

#ifndef _TOOLS_DATE_HXX
#include <tools/date.hxx>

#include <tools/string.hxx>

#endif

#include <vcl/fldunit.hxx>

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>

#include <bf_goodies/vector3d.hxx>
namespace binfilter {

using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::binfilter::xmloff::token;

const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 11;
const sal_Int8 XML_MAXDIGITSCOUNT_DATETIME = 6;
#define XML_NULLDATE "NullDate"

OUString SvXMLUnitConverter::msXML_true;
OUString SvXMLUnitConverter::msXML_false;

void SvXMLUnitConverter::createNumTypeInfo() const
{
    // #110680#
    //Reference< lang::XMultiServiceFactory > xServiceFactory =
    //        comphelper::getProcessServiceFactory();
    //OSL_ENSURE( xServiceFactory.is(),
    //        "XMLUnitConverter: got no service factory" );

    if( mxServiceFactory.is() )
    {
        ((SvXMLUnitConverter *)this)->xNumTypeInfo =
            Reference < XNumberingTypeInfo > (
                mxServiceFactory->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.DefaultNumberingProvider") ) ), UNO_QUERY );
    }
}

/** constructs a SvXMLUnitConverter. The core measure unit is the
    default unit for numerical measures, the XML measure unit is
    the default unit for textual measures
*/

// #110680#
//SvXMLUnitConverter::SvXMLUnitConverter( MapUnit eCoreMeasureUnit,
//                                        MapUnit eXMLMeasureUnit ) :
SvXMLUnitConverter::SvXMLUnitConverter(
    MapUnit eCoreMeasureUnit,
    MapUnit eXMLMeasureUnit,
    const uno::Reference<lang::XMultiServiceFactory>& xServiceFactory ) :
    aNullDate(30, 12, 1899),
    mxServiceFactory( xServiceFactory )
{
    DBG_ASSERT( mxServiceFactory.is(), "got no service manager" );

    meCoreMeasureUnit = eCoreMeasureUnit;
    meXMLMeasureUnit = eXMLMeasureUnit;
}

SvXMLUnitConverter::~SvXMLUnitConverter()
{
}

MapUnit SvXMLUnitConverter::GetMapUnit(sal_Int16 nFieldUnit)
{
    MapUnit eUnit = MAP_INCH;
    switch( nFieldUnit )
    {
    case FUNIT_MM:
        eUnit = MAP_MM;
        break;
    case FUNIT_CM:
    case FUNIT_M:
    case FUNIT_KM:
        eUnit = MAP_CM;
        break;
    case FUNIT_TWIP:
        eUnit = MAP_TWIP;
        break;
    case FUNIT_POINT:
    case FUNIT_PICA:
        eUnit = MAP_POINT;
        break;
//  case FUNIT_INCH:
//  case FUNIT_FOOT:
//  case FUNIT_MILE:
//      eUnit = MAP_INCH;
//      break;
    case FUNIT_100TH_MM:
        eUnit = MAP_100TH_MM;
        break;
    }
    return eUnit;
}

/** convert string to measure using optional min and max values*/
sal_Bool SvXMLUnitConverter::convertMeasure( sal_Int32& nValue,
                                         const OUString& rString,
                                         sal_Int32 nMin, sal_Int32 nMax ) const
{
    return SvXMLUnitConverter::convertMeasure( nValue, rString,
                                               meCoreMeasureUnit,
                                               nMin, nMax );
}

/** convert measure to string */
void SvXMLUnitConverter::convertMeasure( OUStringBuffer& rString,
                                         sal_Int32 nMeasure ) const
{
    SvXMLUnitConverter::convertMeasure( rString, nMeasure,
                                        meCoreMeasureUnit,
                                        meXMLMeasureUnit );
}

/** convert the value from the given string to an int value
    with the given map unit using optional min and max values
*/
sal_Bool SvXMLUnitConverter::convertMeasure( sal_Int32& rValue,
                                         const OUString& rString,
                                         MapUnit eDstUnit,
                                         sal_Int32 nMin, sal_Int32 nMax )
{
    sal_Bool bNeg = sal_False;
    double nVal = 0;

    sal_Int32 nPos = 0L;
    sal_Int32 nLen = rString.getLength();

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rString[nPos] )
        nPos++;

    if( nPos < nLen && sal_Unicode('-') == rString[nPos] )
    {
        bNeg = sal_True;
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
    while( nPos < nLen && sal_Unicode(' ') == rString[nPos] )
        nPos++;

    if( nPos < nLen )
    {

        if( MAP_RELATIVE == eDstUnit )
        {
            if( sal_Unicode('%') != rString[nPos] )
                return sal_False;
        }
        else if( MAP_PIXEL == eDstUnit )
        {
            if( nPos + 1 >= nLen ||
                (sal_Unicode('p') != rString[nPos] &&
                 sal_Unicode('P') != rString[nPos])||
                (sal_Unicode('x') != rString[nPos+1] &&
                 sal_Unicode('X') != rString[nPos+1]) )
                return sal_False;
        }
        else
        {
            DBG_ASSERT( MAP_TWIP == eDstUnit || MAP_POINT ||
                        MAP_100TH_MM == eDstUnit, "unit is not supported");
            const sal_Char *aCmpsL[2] = { 0, 0 };
            const sal_Char *aCmpsU[2] = { 0, 0 };
            double aScales[2] = { 1., 1. };

            if( MAP_TWIP == eDstUnit )
            {
                switch( rString[nPos] )
                {
                case sal_Unicode('c'):
                case sal_Unicode('C'):
                    aCmpsL[0] = "cm";
                    aCmpsU[0] = "CM";
                    aScales[0] = (72.*20.)/2.54; // twip
                    break;
                case sal_Unicode('e'):
                case sal_Unicode('E'):
        //          pCmp1 = sXML_unit_em;
        //          nToken1 = CSS1_EMS;

        //          pCmp2 = sXML_unit_ex;
        //          nToken2 = CSS1_EMX;
                    break;
                case sal_Unicode('i'):
                case sal_Unicode('I'):
                    aCmpsL[0] = "inch";
                    aCmpsU[0] = "INCH";
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

        //          pCmp3 = sXML_unit_px;
        //          nToken3 = CSS1_PIXLENGTH;
                    break;
                }
            }
            else if( MAP_100TH_MM == eDstUnit )
            {
                switch( rString[nPos] )
                {
                case sal_Unicode('c'):
                case sal_Unicode('C'):
                    aCmpsL[0] = "cm";
                    aCmpsU[0] = "CM";
                    aScales[0] = 1000.; // mm/100
                    break;
                case sal_Unicode('e'):
                case sal_Unicode('E'):
        //          pCmp1 = sXML_unit_em;
        //          nToken1 = CSS1_EMS;

        //          pCmp2 = sXML_unit_ex;
        //          nToken2 = CSS1_EMX;
                    break;
                case sal_Unicode('i'):
                case sal_Unicode('I'):
                    aCmpsL[0] = "inch";
                    aCmpsU[0] = "INCH";
                    aScales[0] = 1000.*2.54; // mm/100
                    break;
                case sal_Unicode('m'):
                case sal_Unicode('M'):
                    aCmpsL[0] = "mm";
                    aCmpsU[0] = "MM";
                    aScales[0] = 100.; // mm/100
                    break;
                case sal_Unicode('p'):
                case sal_Unicode('P'):
                    aCmpsL[0] = "pt";
                    aCmpsU[0] = "PT";
                    aScales[0] = (1000.*2.54)/72.; // mm/100

                    aCmpsL[1] = "pc";
                    aCmpsU[1] = "PC";
                    aScales[1] = (1000.*2.54)/12.; // mm/100

        //          pCmp3 = sXML_unit_px;
        //          nToken3 = CSS1_PIXLENGTH;
                    break;
                }
            }
            else if( MAP_POINT == eDstUnit )
            {
                if( rString[nPos] == 'p' || rString[nPos] == 'P' )
                {
                    aCmpsL[0] = "pt";
                    aCmpsU[0] = "PT";
                    aScales[0] = 1;
                }
            }

            if( aCmpsL[0] == NULL )
                return sal_False;

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
                return sal_False;

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

    return sal_True;
}

/** convert measure in given unit to string with given unit */
void SvXMLUnitConverter::convertMeasure( OUStringBuffer& rBuffer,
                                         sal_Int32 nMeasure,
                                         MapUnit eSrcUnit,
                                         MapUnit eDstUnit )
{
    if( eSrcUnit == MAP_RELATIVE )
    {
        DBG_ASSERT( eDstUnit == MAP_RELATIVE,
                    "MAP_RELATIVE only maps to MAP_RELATIVE!" );

        rBuffer.append( nMeasure );
        rBuffer.append( sal_Unicode('%' ) );
    }
    else
    {
        SvXMLExportHelper::AddLength( nMeasure, eSrcUnit,
                                      rBuffer, eDstUnit );
    }
}

/** convert string to boolean */
sal_Bool SvXMLUnitConverter::convertBool( sal_Bool& rBool,
                                      const OUString& rString )
{
    rBool = IsXMLToken(rString, XML_TRUE);

    return rBool || IsXMLToken(rString, XML_FALSE);
}

/** convert boolean to string */
void SvXMLUnitConverter::convertBool( OUStringBuffer& rBuffer,
                                      sal_Bool bValue )
{
    rBuffer.append( GetXMLToken( bValue ? XML_TRUE : XML_FALSE ) );
}

/** convert string to percent */
sal_Bool SvXMLUnitConverter::convertPercent( sal_Int32& rPercent,
                                         const OUString& rString )
{
    return convertMeasure( rPercent, rString, MAP_RELATIVE );
}

/** convert percent to string */
void SvXMLUnitConverter::convertPercent( OUStringBuffer& rBuffer,
                                         sal_Int32 nValue )
{
    rBuffer.append( nValue );
    rBuffer.append( sal_Unicode('%' ) );
}

/** convert string to pixel measure */
sal_Bool SvXMLUnitConverter::convertMeasurePx( sal_Int32& rPixel,
                                         const OUString& rString )
{
    return convertMeasure( rPixel, rString, MAP_PIXEL );
}

/** convert pixel measure to string */
void SvXMLUnitConverter::convertMeasurePx( OUStringBuffer& rBuffer,
                                         sal_Int32 nValue )
{
    rBuffer.append( nValue );
    rBuffer.append( sal_Unicode('p' ) );
    rBuffer.append( sal_Unicode('x' ) );
}

/** convert string to enum using given enum map, if the enum is
    not found in the map, this method will return false
*/
sal_Bool SvXMLUnitConverter::convertEnum( sal_uInt16& rEnum,
                                      const OUString& rValue,
                                      const SvXMLEnumStringMapEntry *pMap )
{
    while( pMap->pName )
    {
        if( rValue.equalsAsciiL( pMap->pName, pMap->nNameLength ) )
        {
            rEnum = pMap->nValue;
            return sal_True;
        }
        pMap++;
    }

    return sal_False;
}

/** convert string to enum using given token map, if the enum is
    not found in the map, this method will return false */
sal_Bool SvXMLUnitConverter::convertEnum(
    sal_uInt16& rEnum,
    const OUString& rValue,
    const SvXMLEnumMapEntry *pMap )
{
    while( pMap->eToken != XML_TOKEN_INVALID )
    {
        if( IsXMLToken( rValue, pMap->eToken ) )
        {
            rEnum = pMap->nValue;
            return sal_True;
        }
        pMap++;
    }
    return sal_False;
}

/** convert enum to string using given token map with an optional
    default token. If the enum is not found in the map,
    this method will either use the given default or return
    false if no default is set */
sal_Bool SvXMLUnitConverter::convertEnum(
    OUStringBuffer& rBuffer,
    sal_uInt16 nValue,
    const SvXMLEnumMapEntry *pMap,
    enum XMLTokenEnum eDefault)
{
    enum XMLTokenEnum eTok = eDefault;

    while( pMap->eToken != XML_TOKEN_INVALID )
    {
        if( pMap->nValue == nValue )
        {
            eTok = pMap->eToken;
            break;
        }
        pMap++;
    }

    // the map may have contained XML_TOKEN_INVALID
    if( eTok == XML_TOKEN_INVALID )
        eTok = eDefault;

    if( eTok != XML_TOKEN_INVALID )
        rBuffer.append( GetXMLToken(eTok) );

    return (eTok != XML_TOKEN_INVALID);
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
sal_Bool SvXMLUnitConverter::convertColor( Color& rColor,
                                       const OUString& rValue )
{
    if( rValue.getLength() != 7 || rValue[0] != '#' )
        return sal_False;

    rColor.SetRed( lcl_gethex( rValue[1] ) * 16 +
                   lcl_gethex( rValue[2] ) );

    rColor.SetGreen( lcl_gethex( rValue[3] ) * 16 +
                     lcl_gethex( rValue[4] ) );

    rColor.SetBlue( lcl_gethex( rValue[5] ) * 16 +
                    lcl_gethex( rValue[6] ) );

    return sal_True;
}

static sal_Char aHexTab[] = "0123456789abcdef";

/** convert color to string */
void SvXMLUnitConverter::convertColor( OUStringBuffer& rBuffer,
                                       const Color& rCol )
{
    rBuffer.append( sal_Unicode( '#' ) );

    sal_uInt8 nCol = rCol.GetRed();
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );

    nCol = rCol.GetGreen();
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );

    nCol = rCol.GetBlue();
    rBuffer.append( sal_Unicode( aHexTab[ nCol >> 4 ] ) );
    rBuffer.append( sal_Unicode( aHexTab[ nCol & 0xf ] ) );
}

/** convert number to string */
void SvXMLUnitConverter::convertNumber( OUStringBuffer& rBuffer,
                                        sal_Int32 nNumber )
{
    rBuffer.append( sal_Int32( nNumber ) );
}

/** convert string to number with optional min and max values */
sal_Bool SvXMLUnitConverter::convertNumber( sal_Int32& rValue,
                                        const OUString& rString,
                                        sal_Int32 nMin, sal_Int32 nMax )
{
    sal_Bool bNeg = sal_False;
    rValue = 0;

    sal_Int32 nPos = 0L;
    sal_Int32 nLen = rString.getLength();

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rString[nPos] )
        nPos++;

    if( nPos < nLen && sal_Unicode('-') == rString[nPos] )
    {
        bNeg = sal_True;
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

    return nPos == nLen;
}

/** convert double number to string (using ::rtl::math) */
void SvXMLUnitConverter::convertDouble(::rtl::OUStringBuffer& rBuffer,
    double fNumber, BOOL bWriteUnits) const
{
    SvXMLUnitConverter::convertDouble(rBuffer, fNumber,
        bWriteUnits, meCoreMeasureUnit, meXMLMeasureUnit);
}

/** convert double number to string (using ::rtl::math) */
void SvXMLUnitConverter::convertDouble( ::rtl::OUStringBuffer& rBuffer,
    double fNumber, BOOL bWriteUnits, MapUnit eCoreUnit, MapUnit eDstUnit)
{
    if(MAP_RELATIVE == eCoreUnit)
    {
        DBG_ASSERT(eDstUnit == MAP_RELATIVE, "MAP_RELATIVE only maps to MAP_RELATIVE!" );
        ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', sal_True);
        if(bWriteUnits)
            rBuffer.append(sal_Unicode('%'));
    }
    else
    {
        OUStringBuffer sUnit;
        double fFactor = SvXMLExportHelper::GetConversionFactor(sUnit, eCoreUnit, eDstUnit);
        if(fFactor != 1.0)
            fNumber *= fFactor;
        ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', sal_True);
        if(bWriteUnits)
            rBuffer.append(sUnit);
    }
}

/** convert double number to string (using ::rtl::math) */
void SvXMLUnitConverter::convertDouble( ::rtl::OUStringBuffer& rBuffer, double fNumber)
{
    ::rtl::math::doubleToUStringBuffer( rBuffer, fNumber, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', sal_True);
}

/** convert string to double number (using ::rtl::math) */
sal_Bool SvXMLUnitConverter::convertDouble(double& rValue,
    const ::rtl::OUString& rString, BOOL bLookForUnits) const
{
    if(bLookForUnits)
    {
        MapUnit eSrcUnit = SvXMLExportHelper::GetUnitFromString(rString, meCoreMeasureUnit);

        return SvXMLUnitConverter::convertDouble(rValue, rString,
            eSrcUnit, meCoreMeasureUnit);
    }
    else
    {
        return SvXMLUnitConverter::convertDouble(rValue, rString);
    }
}

/** convert string to double number (using ::rtl::math) */
sal_Bool SvXMLUnitConverter::convertDouble(double& rValue,
    const ::rtl::OUString& rString, MapUnit eSrcUnit, MapUnit eCoreUnit)
{
    rtl_math_ConversionStatus eStatus;
    rValue = ::rtl::math::stringToDouble( rString, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );

    if(eStatus == rtl_math_ConversionStatus_Ok)
    {
        OUStringBuffer sUnit;
        double fFactor = SvXMLExportHelper::GetConversionFactor(sUnit, eCoreUnit, eSrcUnit);
        if(fFactor != 1.0 && fFactor != 0.0)
            rValue /= fFactor;
    }

    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert string to double number (using ::rtl::math) */
sal_Bool SvXMLUnitConverter::convertDouble(double& rValue, const ::rtl::OUString& rString)
{
    rtl_math_ConversionStatus eStatus;
    rValue = ::rtl::math::stringToDouble( rString, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );
    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** get the Null Date of the XModel and set it to the UnitConverter */
sal_Bool SvXMLUnitConverter::setNullDate(const ::com::sun::star::uno::Reference < ::com::sun::star::frame::XModel>& xModel)
{
    ::com::sun::star::uno::Reference < ::com::sun::star::util::XNumberFormatsSupplier> xNumberFormatsSupplier (xModel, ::com::sun::star::uno::UNO_QUERY);
    if (xNumberFormatsSupplier.is())
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet> xPropertySet = xNumberFormatsSupplier->getNumberFormatSettings();
        if (xPropertySet.is() )
        {
            ::com::sun::star::uno::Any aAnyNullDate = xPropertySet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_NULLDATE)));
            if (aAnyNullDate >>= aNullDate)
            {
                return sal_True;
            }
        }
    }
    return sal_False;
}

/** convert double to ISO Time String; negative durations allowed */
void SvXMLUnitConverter::convertTime( ::rtl::OUStringBuffer& rBuffer,
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
                    sal_True));
        if ( a100th.getLength() > 2 )
        {
            rBuffer.append( sal_Unicode(','));
            rBuffer.append( a100th.copy( 2 ) );     // strip 0.
        }
    }
    rBuffer.append( sal_Unicode('S'));
}

/** convert ISO Time String to double; negative durations allowed */
sal_Bool SvXMLUnitConverter::convertTime( double& fTime,
                            const ::rtl::OUString& rString)
{
    ::rtl::OUString aTrimmed = rString.trim().toAsciiUpperCase();
    const sal_Unicode* pStr = aTrimmed.getStr();

    // negative time duration?
    sal_Bool bIsNegativeDuration = sal_False;
    if ( sal_Unicode('-') == (*pStr) )
    {
        bIsNegativeDuration = sal_True;
        pStr++;
    }

    if ( *(pStr++) != sal_Unicode('P') )            // duration must start with "P"
        return sal_False;

    ::rtl::OUString sDoubleStr;
    sal_Bool bSuccess = sal_True;
    sal_Bool bDone = sal_False;
    sal_Bool bTimePart = sal_False;
    sal_Bool bIsFraction = sal_False;
    sal_Int32 nDays  = 0;
    sal_Int32 nHours = 0;
    sal_Int32 nMins  = 0;
    sal_Int32 nSecs  = 0;
    sal_Int32 nTemp = 0;

    while ( bSuccess && !bDone )
    {
        sal_Unicode c = *(pStr++);
        if ( !c )                               // end
            bDone = sal_True;
        else if ( sal_Unicode('0') <= c && sal_Unicode('9') >= c )
        {
            if ( nTemp >= SAL_MAX_INT32 / 10 )
                bSuccess = sal_False;
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
            else if ( c == sal_Unicode(',') )
            {
                nSecs = nTemp;
                nTemp = 0;
                bIsFraction = sal_True;
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
                bSuccess = sal_False;               // invalid character
        }
        else
        {
            if ( c == sal_Unicode('T') )            // "T" starts time part
                bTimePart = sal_True;
            else if ( c == sal_Unicode('D') )
            {
                nDays = nTemp;
                nTemp = 0;
            }
            else if ( c == sal_Unicode('Y') || c == sal_Unicode('M') )
            {
                //! how many days is a year or month?

                DBG_ERROR("years or months in duration: not implemented");
                bSuccess = sal_False;
            }
            else
                bSuccess = sal_False;               // invalid character
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
void SvXMLUnitConverter::convertTime( ::rtl::OUStringBuffer& rBuffer,
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
sal_Bool SvXMLUnitConverter::convertTime( ::com::sun::star::util::DateTime& rDateTime,
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

        return sal_True;
    }
    return sal_False;
}

/** convert double to ISO Date Time String */
void SvXMLUnitConverter::convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                            const double& fDateTime, const ::com::sun::star::util::Date& aTempNullDate)
{
    double fValue = fDateTime;
    sal_Int32 nValue = static_cast <sal_Int32> (::rtl::math::approxFloor (fValue));
    Date aDate (aTempNullDate.Day, aTempNullDate.Month, aTempNullDate.Year);
    aDate += nValue;
    fValue -= nValue;
    double fCount;
    if (nValue > 0)
         fCount = ::rtl::math::approxFloor (log10((double) nValue)) + 1;
    else if (nValue < 0)
         fCount = ::rtl::math::approxFloor (log10((double)(nValue * -1))) + 1;
    else
        fCount = 0.0;
    sal_Int16 nCount = sal_Int16(fCount);
    sal_Bool bHasTime(sal_False);
    double fHoursValue;
    double fMinsValue;
    double fSecsValue;
    double f100SecsValue;
    if (fValue > 0.0)
    {
        bHasTime = sal_True;
        fValue *= 24;
        fHoursValue = ::rtl::math::approxFloor (fValue);
        fValue -= fHoursValue;
        fValue *= 60;
        fMinsValue = ::rtl::math::approxFloor (fValue);
        fValue -= fMinsValue;
        fValue *= 60;
        fSecsValue = ::rtl::math::approxFloor (fValue);
        fValue -= fSecsValue;
        if (fValue > 0.0)
            f100SecsValue = ::rtl::math::round( fValue, XML_MAXDIGITSCOUNT_TIME - nCount);
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
        if (fHoursValue >= 24.0)
        {
            fHoursValue -= 24.0;
            aDate += 1;
        }
    }
    rBuffer.append( sal_Int32( aDate.GetYear()));
    rBuffer.append( sal_Unicode('-'));
    USHORT nTemp = aDate.GetMonth();
    if (nTemp < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( nTemp));
    rBuffer.append( sal_Unicode('-'));
    nTemp = aDate.GetDay();
    if (nTemp < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( nTemp));
    if(bHasTime)
    {
        rBuffer.append( sal_Unicode('T'));
        if (fHoursValue < 10)
            rBuffer.append( sal_Unicode('0'));
        rBuffer.append( sal_Int32( fHoursValue));
        rBuffer.append( sal_Unicode(':'));
        if (fMinsValue < 10)
            rBuffer.append( sal_Unicode('0'));
        rBuffer.append( sal_Int32( fMinsValue));
        rBuffer.append( sal_Unicode(':'));
        if (fSecsValue < 10)
            rBuffer.append( sal_Unicode('0'));
        rBuffer.append( sal_Int32( fSecsValue));
        if (f100SecsValue > 0.0)
        {
            ::rtl::OUString a100th( ::rtl::math::doubleToUString( fValue,
                        rtl_math_StringFormat_F,
                        XML_MAXDIGITSCOUNT_TIME - nCount, '.', sal_True));
            if ( a100th.getLength() > 2 )
            {
                rBuffer.append( sal_Unicode(','));
                rBuffer.append( a100th.copy( 2 ) );     // strip 0.
            }
        }
    }
}

/** convert ISO Date Time String to double */
sal_Bool SvXMLUnitConverter::convertDateTime( double& fDateTime,
                            const ::rtl::OUString& rString, const ::com::sun::star::util::Date& aTempNullDate)
{
    sal_Bool bSuccess = sal_True;

    ::rtl::OUString aDateStr, aTimeStr, sDoubleStr;
    sal_Int32 nPos = rString.indexOf( (sal_Unicode) 'T' );
    sal_Int32 nPos2 = rString.indexOf( (sal_Unicode) ',' );
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
        bSuccess = sal_False;
    else
    {
        sal_Int32 n = 0;
        if ( !convertNumber( nYear, aDateStr.getToken( 0, '-', n ), 0, 9999 ) )
            bSuccess = sal_False;
        if ( nDateTokens >= 2 )
            if ( !convertNumber( nMonth, aDateStr.getToken( 0, '-', n ), 0, 12 ) )
                bSuccess = sal_False;
        if ( nDateTokens >= 3 )
            if ( !convertNumber( nDay, aDateStr.getToken( 0, '-', n ), 0, 31 ) )
                bSuccess = sal_False;
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
            bSuccess = sal_False;
        else
        {
            sal_Int32 n = 0;
            if ( !convertNumber( nHour, aTimeStr.getToken( 0, ':', n ), 0, 23 ) )
                bSuccess = sal_False;
            if ( nTimeTokens >= 2 )
                if ( !convertNumber( nMin, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = sal_False;
            if ( nTimeTokens >= 3 )
                if ( !convertNumber( nSec, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = sal_False;
        }
    }

    if (bSuccess)
    {
        double fTempDateTime = 0.0;
        Date aTmpNullDate(aTempNullDate.Day, aTempNullDate.Month, aTempNullDate.Year);
        Date aTempDate((sal_uInt16)nDay, (sal_uInt16)nMonth, (sal_uInt16)nYear);
        sal_Int32 nTage = aTempDate - aTmpNullDate;
        fTempDateTime = nTage;
        double Hour = nHour;
        double Min = nMin;
        double Sec = nSec;
        double Sec100 = 0.0;
        double fFraction = sDoubleStr.toDouble();
        fTempDateTime += Hour / 24;
        fTempDateTime += Min / (24 * 60);
        fTempDateTime += Sec / (24 * 60 * 60);
        fTempDateTime += Sec100 / (24 * 60 * 60 * 60);
        fTempDateTime += fFraction / (24 * 60 * 60);
        fDateTime = fTempDateTime;
    }
    return bSuccess;
}

/** convert util::DateTime to ISO Date String */
void SvXMLUnitConverter::convertDateTime( ::rtl::OUStringBuffer& rBuffer,
                                const ::com::sun::star::util::DateTime& rDateTime )
{
    String aString( String::CreateFromInt32( rDateTime.Year ) );
    aString += '-';
    if( rDateTime.Month < 10 )
        aString += '0';
    aString += String::CreateFromInt32( rDateTime.Month );
    aString += '-';
    if( rDateTime.Day < 10 )
        aString += '0';
    aString += String::CreateFromInt32( rDateTime.Day );

    if( rDateTime.Seconds != 0 ||
        rDateTime.Minutes != 0 ||
        rDateTime.Hours   != 0 )
    {
        aString += 'T';
        if( rDateTime.Hours < 10 )
            aString += '0';
        aString += String::CreateFromInt32( rDateTime.Hours );
        aString += ':';
        if( rDateTime.Minutes < 10 )
            aString += '0';
        aString += String::CreateFromInt32( rDateTime.Minutes );
        aString += ':';
        if( rDateTime.Seconds < 10 )
            aString += '0';
        aString += String::CreateFromInt32( rDateTime.Seconds );
        if ( rDateTime.HundredthSeconds > 0)
        {
            aString += ',';
            if (rDateTime.HundredthSeconds < 10)
                aString += '0';
            aString += String::CreateFromInt32( rDateTime.HundredthSeconds );
        }
    }

    rBuffer.append( aString );
}

/** convert ISO Date String to util::DateTime */
sal_Bool SvXMLUnitConverter::convertDateTime( ::com::sun::star::util::DateTime& rDateTime,
                                     const ::rtl::OUString& rString )
{
    sal_Bool bSuccess = sal_True;

    ::rtl::OUString aDateStr, aTimeStr, sDoubleStr;
    sal_Int32 nPos = rString.indexOf( (sal_Unicode) 'T' );
    sal_Int32 nPos2 = rString.indexOf( (sal_Unicode) ',' );
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
        bSuccess = sal_False;
    else
    {
        sal_Int32 n = 0;
        if ( !convertNumber( nYear, aDateStr.getToken( 0, '-', n ), 0, 9999 ) )
            bSuccess = sal_False;
        if ( nDateTokens >= 2 )
            if ( !convertNumber( nMonth, aDateStr.getToken( 0, '-', n ), 0, 12 ) )
                bSuccess = sal_False;
        if ( nDateTokens >= 3 )
            if ( !convertNumber( nDay, aDateStr.getToken( 0, '-', n ), 0, 31 ) )
                bSuccess = sal_False;
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
            bSuccess = sal_False;
        else
        {
            sal_Int32 n = 0;
            if ( !convertNumber( nHour, aTimeStr.getToken( 0, ':', n ), 0, 23 ) )
                bSuccess = sal_False;
            if ( nTimeTokens >= 2 )
                if ( !convertNumber( nMin, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = sal_False;
            if ( nTimeTokens >= 3 )
                if ( !convertNumber( nSec, aTimeStr.getToken( 0, ':', n ), 0, 59 ) )
                    bSuccess = sal_False;
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
        rDateTime.HundredthSeconds = sDoubleStr.toDouble() * 100;
    }
    return bSuccess;
}

/** gets the position of the first comma after npos in the string
    rStr. Commas inside '"' pairs are not matched */
sal_Int32 SvXMLUnitConverter::indexOfComma( const OUString& rStr,
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

// ---

SvXMLTokenEnumerator::SvXMLTokenEnumerator( const OUString& rString, sal_Unicode cSeperator /* = sal_Unicode(' ') */ )
: maTokenString( rString ), mnNextTokenPos(0), mcSeperator( cSeperator )
{
}

sal_Bool SvXMLTokenEnumerator::getNextToken( OUString& rToken )
{
    if( -1 == mnNextTokenPos )
        return sal_False;

    int nTokenEndPos = maTokenString.indexOf( mcSeperator, mnNextTokenPos );
    if( nTokenEndPos != -1 )
    {
        rToken = maTokenString.copy( mnNextTokenPos,
                                     nTokenEndPos - mnNextTokenPos );
        mnNextTokenPos = nTokenEndPos + 1;

        // if the mnNextTokenPos is at the end of the string, we have
        // to deliver an empty token
        if( mnNextTokenPos > maTokenString.getLength() )
            mnNextTokenPos = -1;
    }
    else
    {
        rToken = maTokenString.copy( mnNextTokenPos );
        mnNextTokenPos = -1;
    }

    return sal_True;
}

// ---

/** convert string to vector3D */
sal_Bool SvXMLUnitConverter::convertVector3D( Vector3D& rVector,
    const OUString& rValue )
{
    if(!rValue.getLength() || rValue[0] != '(')
        return sal_False;

    sal_Int32 nPos(1L);
    sal_Int32 nFound = rValue.indexOf(sal_Unicode(' '), nPos);

    if(nFound == -1 || nFound <= nPos)
        return sal_False;

    OUString aContentX = rValue.copy(nPos, nFound - nPos);

    nPos = nFound + 1;
    nFound = rValue.indexOf(sal_Unicode(' '), nPos);

    if(nFound == -1 || nFound <= nPos)
        return sal_False;

    OUString aContentY = rValue.copy(nPos, nFound - nPos);

    nPos = nFound + 1;
    nFound = rValue.indexOf(sal_Unicode(')'), nPos);

    if(nFound == -1 || nFound <= nPos)
        return sal_False;

    OUString aContentZ = rValue.copy(nPos, nFound - nPos);

    rtl_math_ConversionStatus eStatus;

    rVector.X() = ::rtl::math::stringToDouble(aContentX, sal_Unicode('.'),
            sal_Unicode(','), &eStatus, NULL);

    if( eStatus != rtl_math_ConversionStatus_Ok )
        return sal_False;

    rVector.Y() = ::rtl::math::stringToDouble(aContentY, sal_Unicode('.'),
            sal_Unicode(','), &eStatus, NULL);

    if( eStatus != rtl_math_ConversionStatus_Ok )
        return sal_False;

    rVector.Z() = ::rtl::math::stringToDouble(aContentZ, sal_Unicode('.'),
            sal_Unicode(','), &eStatus, NULL);


    return ( eStatus != rtl_math_ConversionStatus_Ok );
}

/** convert vector3D to string */
void SvXMLUnitConverter::convertVector3D( OUStringBuffer &rBuffer,
    const Vector3D& rVector )
{
    rBuffer.append(sal_Unicode('('));
    convertDouble(rBuffer, rVector.X());
    rBuffer.append(sal_Unicode(' '));
    convertDouble(rBuffer, rVector.Y());
    rBuffer.append(sal_Unicode(' '));
    convertDouble(rBuffer, rVector.Z());
    rBuffer.append(sal_Unicode(')'));
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
    {											 62,255,255,255, 63, // 43-47
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



void ThreeByteToFourByte (const sal_Int8* pBuffer, const sal_Int32 nStart, const sal_Int32 nFullLen, ::rtl::OUStringBuffer& sBuffer)
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

    sal_uInt8 nIndex ((nBinaer & 0xFC0000) >> 18);
    sBuffer.setCharAt(0, aBase64EncodeTable [nIndex]);

    nIndex = (nBinaer & 0x3F000) >> 12;
    sBuffer.setCharAt(1, aBase64EncodeTable [nIndex]);
    if (nLen == 1)
        return;

    nIndex = (nBinaer & 0xFC0) >> 6;
    sBuffer.setCharAt(2, aBase64EncodeTable [nIndex]);
    if (nLen == 2)
        return;

    nIndex = (nBinaer & 0x3F);
    sBuffer.setCharAt(3, aBase64EncodeTable [nIndex]);
}

void SvXMLUnitConverter::encodeBase64( ::rtl::OUStringBuffer& aStrBuffer, const uno::Sequence<sal_Int8>& aPass)
{
    sal_Int32 i(0);
    sal_Int32 nBufferLength(aPass.getLength());
    const sal_Int8* pBuffer = aPass.getConstArray();
    while (i < nBufferLength)
    {
        ::rtl::OUStringBuffer sBuffer;
        ThreeByteToFourByte (pBuffer, i, nBufferLength, sBuffer);
        aStrBuffer.append(sBuffer);
        i += 3;
    }
}

void SvXMLUnitConverter::decodeBase64(uno::Sequence<sal_Int8>& aBuffer, const ::rtl::OUString& sBuffer)
{
    sal_Int32 nCharsDecoded = decodeBase64SomeChars( aBuffer, sBuffer );
    OSL_ENSURE( nCharsDecoded == sBuffer.getLength(),
                "some bytes left in base64 decoding!" );
}

sal_Int32 SvXMLUnitConverter::decodeBase64SomeChars(
        uno::Sequence<sal_Int8>& rOutBuffer,
        const ::rtl::OUString& rInBuffer)
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

sal_Bool SvXMLUnitConverter::convertNumFormat(
        sal_Int16& rType,
        const OUString& rNumFmt,
        const OUString& rNumLetterSync,
        sal_Bool bNumberNone ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bExt = sal_False;

    sal_Int32 nLen = rNumFmt.getLength();
    if( 0 == nLen )
    {
        if( bNumberNone )
            rType = NumberingType::NUMBER_NONE;
        else
            bRet = sal_False;
    }
    else if( 1 == nLen )
    {
        switch( rNumFmt[0] )
        {
        case sal_Unicode('1'):  rType = NumberingType::ARABIC;          break;
        case sal_Unicode('a'):  rType = NumberingType::CHARS_LOWER_LETTER;  break;
        case sal_Unicode('A'):  rType = NumberingType::CHARS_UPPER_LETTER;  break;
        case sal_Unicode('i'):  rType = NumberingType::ROMAN_LOWER; break;
        case sal_Unicode('I'):  rType = NumberingType::ROMAN_UPPER; break;
        default:                bExt = sal_True; break;
        }
        if( !bExt && IsXMLToken( rNumLetterSync, XML_TRUE ) )
        {
            switch( rType )
            {
            case NumberingType::CHARS_LOWER_LETTER:
                rType = NumberingType::CHARS_LOWER_LETTER_N;
                break;
            case NumberingType::CHARS_UPPER_LETTER:
                rType = NumberingType::CHARS_UPPER_LETTER_N;
                break;
            }
        }
    }
    else
    {
        bExt = sal_True;
    }
    if( bExt )
    {
        Reference < XNumberingTypeInfo > xInfo = getNumTypeInfo();
        if( xInfo.is() && xInfo->hasNumberingType( rNumFmt ) )
        {
            rType = xInfo->getNumberingType( rNumFmt );
        }
        else
        {
            rType = NumberingType::ARABIC;
        }
    }

    return bRet;
}

void SvXMLUnitConverter::convertNumFormat( OUStringBuffer& rBuffer,
                           sal_Int16 nType ) const
{
    enum XMLTokenEnum eFormat = XML_TOKEN_INVALID;
    sal_Bool bExt = sal_False;
    switch( nType )
    {
    case NumberingType::CHARS_UPPER_LETTER:     eFormat = XML_A_UPCASE; break;
    case NumberingType::CHARS_LOWER_LETTER:     eFormat = XML_A; break;
    case NumberingType::ROMAN_UPPER:            eFormat = XML_I_UPCASE; break;
    case NumberingType::ROMAN_LOWER:            eFormat = XML_I; break;
    case NumberingType::ARABIC:                 eFormat = XML_1; break;
    case NumberingType::CHARS_UPPER_LETTER_N:   eFormat = XML_A_UPCASE; break;
    case NumberingType::CHARS_LOWER_LETTER_N:   eFormat = XML_A; break;
    case NumberingType::NUMBER_NONE:            eFormat = XML__EMPTY; break;

    case NumberingType::CHAR_SPECIAL:
    case NumberingType::PAGE_DESCRIPTOR:
    case NumberingType::BITMAP:
        DBG_ASSERT( eFormat != XML_TOKEN_INVALID, "invalid number format" );
        break;
    default:
        bExt = sal_True;
        break;
    }

    if( eFormat != XML_TOKEN_INVALID )
    {
        rBuffer.append( GetXMLToken(eFormat) );
    }
    else
    {
        Reference < XNumberingTypeInfo > xInfo = getNumTypeInfo();
        if( xInfo.is() )
            rBuffer.append( xInfo->getNumberingIdentifier( nType ) );
    }
}

void SvXMLUnitConverter::convertNumLetterSync( OUStringBuffer& rBuffer,
                               sal_Int16 nType ) const
{
    enum XMLTokenEnum eSync = XML_TOKEN_INVALID;
    switch( nType )
    {
    case NumberingType::CHARS_UPPER_LETTER:
    case NumberingType::CHARS_LOWER_LETTER:
    case NumberingType::ROMAN_UPPER:
    case NumberingType::ROMAN_LOWER:
    case NumberingType::ARABIC:
    case NumberingType::NUMBER_NONE:
        // default
        // eSync = XML_FALSE;
        break;

    case NumberingType::CHARS_UPPER_LETTER_N:
    case NumberingType::CHARS_LOWER_LETTER_N:
        eSync = XML_TRUE;
        break;

    case NumberingType::CHAR_SPECIAL:
    case NumberingType::PAGE_DESCRIPTOR:
    case NumberingType::BITMAP:
        DBG_ASSERT( eSync != XML_TOKEN_INVALID, "invalid number format" );
        break;
    }
    if( eSync != XML_TOKEN_INVALID )
        rBuffer.append( GetXMLToken(eSync) );
}

void SvXMLUnitConverter::convertPropertySet(uno::Sequence<beans::PropertyValue>& rProps,
                    const uno::Reference<beans::XPropertySet>& aProperties)
{
    uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = aProperties->getPropertySetInfo();
    if (xPropertySetInfo.is())
    {
        uno::Sequence< beans::Property > aProps = xPropertySetInfo->getProperties();
        sal_Int32 nCount(aProps.getLength());
        if (nCount)
        {
            rProps.realloc(nCount);
            beans::PropertyValue* pProps = rProps.getArray();
            if (pProps)
            {
                for (sal_Int32 i = 0; i < nCount; i++, pProps++)
                {
                    pProps->Name = aProps[i].Name;
                    pProps->Value = aProperties->getPropertyValue(aProps[i].Name);
                }
            }
        }
    }
}

void SvXMLUnitConverter::convertPropertySet(uno::Reference<beans::XPropertySet>& rProperties,
                    const uno::Sequence<beans::PropertyValue>& aProps)
{
    sal_Int32 nCount(aProps.getLength());
    if (nCount)
    {
        uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = rProperties->getPropertySetInfo();
        if (xPropertySetInfo.is())
        {
            for (sal_Int32 i = 0; i < nCount; i++)
            {
                if (xPropertySetInfo->hasPropertyByName(aProps[i].Name))
                    rProperties->setPropertyValue(aProps[i].Name, aProps[i].Value);
            }
        }
    }
}

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
