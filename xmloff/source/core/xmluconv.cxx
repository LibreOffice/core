/*************************************************************************
 *
 *  $RCSfile: xmluconv.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-21 09:57:12 $
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

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLEHELP_HXX
#include "xmlehelp.hxx"
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmlement.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmluconv.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmlkywd.hxx>
#endif

#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif

#ifndef _TOOLS_DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XNUMBERINGTYPEINFO_HPP_
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _SVX_VECTOR3D_HXX
#include <goodies/vector3d.hxx>
#endif

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;
using namespace com::sun::star::style;

const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 11;
const sal_Int8 XML_MAXDIGITSCOUNT_DATETIME = 6;
#define XML_NULLDATE "NullDate"

OUString SvXMLUnitConverter::msXML_true;
OUString SvXMLUnitConverter::msXML_false;

void SvXMLUnitConverter::initXMLStrings()
{
    if( msXML_true.getLength() == 0 )
    {
        msXML_true = OUString::createFromAscii( sXML_true );
        msXML_false = OUString::createFromAscii( sXML_false );
    }
}

void SvXMLUnitConverter::createNumTypeInfo() const
{
    Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
    OSL_ENSURE( xServiceFactory.is(),
            "XMLUnitConverter: got no service factory" );
    if( xServiceFactory.is() )
    {
        ((SvXMLUnitConverter *)this)->xNumTypeInfo =
            Reference < XNumberingTypeInfo > (
                xServiceFactory->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.DefaultNumberingProvider") ) ), UNO_QUERY );
    }
}

/** constructs a SvXMLUnitConverter. The core measure unit is the
    default unit for numerical measures, the XML measure unit is
    the default unit for textual measures
*/
SvXMLUnitConverter::SvXMLUnitConverter( MapUnit eCoreMeasureUnit,
                                        MapUnit eXMLMeasureUnit ) :
    aNullDate(30, 12, 1899)
{
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

/** convert measure with given unit to string */
void SvXMLUnitConverter::convertMeasure( OUStringBuffer& rString,
                                         sal_Int32 nMeasure,
                                         MapUnit eSrcUnit ) const
{
    SvXMLUnitConverter::convertMeasure( rString, nMeasure,
                                        eSrcUnit,
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
    rBool = rString.compareToAscii( sXML_true ) == 0L;

    return rBool || rString.compareToAscii( sXML_false ) == 0L;
}

/** convert boolean to string */
void SvXMLUnitConverter::convertBool( OUStringBuffer& rBuffer,
                                      sal_Bool bValue )
{
    rBuffer.appendAscii( bValue ? sXML_true : sXML_false );
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
                                      const SvXMLEnumMapEntry *pMap )
{
    while( pMap->pName )
    {
        if( rValue.compareToAscii( pMap->pName ) == 0 )
        {
            rEnum = pMap->nValue;
            return sal_True;
        }
        pMap++;
    }

    return sal_False;
}

/** convert enum to string using given enum map with optional
    default string. If the enum is not found in the map,
    this method will either use the given default or return
    false if not default is set
*/
sal_Bool SvXMLUnitConverter::convertEnum( OUStringBuffer& rBuffer,
                                      sal_uInt16 nValue,
                                      const SvXMLEnumMapEntry *pMap,
                                      sal_Char * pDefault /* = NULL */ )
{
    const sal_Char *pStr = pDefault;

    while( pMap->pName )
    {
        if( pMap->nValue == nValue )
        {
            pStr = pMap->pName;
            break;
        }
        pMap++;
    }

    if( NULL == pStr )
        pStr = pDefault;

    if( NULL != pStr )
        rBuffer.appendAscii( pStr );

    return NULL != pStr;
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

/** convert double number to string (using SolarMath) */
void SvXMLUnitConverter::convertDouble(::rtl::OUStringBuffer& rBuffer,
    double fNumber, BOOL bWriteUnits) const
{
    SvXMLUnitConverter::convertDouble(rBuffer, fNumber,
        bWriteUnits, meCoreMeasureUnit, meXMLMeasureUnit);
}

/** convert double number to string (using SolarMath) */
void SvXMLUnitConverter::convertDouble( ::rtl::OUStringBuffer& rBuffer,
    double fNumber, BOOL bWriteUnits, MapUnit eCoreUnit, MapUnit eDstUnit)
{
    if(MAP_RELATIVE == eCoreUnit)
    {
        DBG_ASSERT(eDstUnit == MAP_RELATIVE, "MAP_RELATIVE only maps to MAP_RELATIVE!" );
        String aResult;
        SolarMath::DoubleToString(aResult, fNumber, 'A', INT_MAX, '.', sal_True);
        rBuffer.append(rtl::OUString(aResult));
        if(bWriteUnits)
            rBuffer.append(sal_Unicode('%'));
    }
    else
    {
        OUStringBuffer sUnit;
        String aResult;
        double fFactor = SvXMLExportHelper::GetConversionFactor(sUnit, eCoreUnit, eDstUnit);
        if(fFactor != 1.0)
            fNumber *= fFactor;
        SolarMath::DoubleToString(aResult, fNumber, 'A', INT_MAX, '.', sal_True);
        rBuffer.append(rtl::OUString(aResult));
        if(bWriteUnits)
            rBuffer.append(sUnit);
    }
}

/** convert double number to string (using SolarMath) */
void SvXMLUnitConverter::convertDouble( ::rtl::OUStringBuffer& rBuffer, double fNumber)
{
    String aResult;
    SolarMath::DoubleToString(aResult, fNumber, 'A', INT_MAX, '.', sal_True);
    rBuffer.append(rtl::OUString(aResult));
}

/** convert string to double number (using SolarMath) */
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

/** convert string to double number (using SolarMath) */
sal_Bool SvXMLUnitConverter::convertDouble(double& rValue,
    const ::rtl::OUString& rString, MapUnit eSrcUnit, MapUnit eCoreUnit)
{
    int nErr;
    rValue = SolarMath::StringToDouble( rString, (sal_Unicode)(','), (sal_Unicode)('.'), nErr );

    if(nErr == 0)
    {
        OUStringBuffer sUnit;
        double fFactor = SvXMLExportHelper::GetConversionFactor(sUnit, eCoreUnit, eSrcUnit);
        if(fFactor != 1.0 && fFactor != 0.0)
            rValue /= fFactor;
    }

    return ( nErr == 0 );
}

/** convert string to double number (using SolarMath) */
sal_Bool SvXMLUnitConverter::convertDouble(double& rValue, const ::rtl::OUString& rString)
{
    int nErr;
    rValue = SolarMath::StringToDouble( rString, (sal_Unicode)(','), (sal_Unicode)('.'), nErr );
    return ( nErr == 0 );
}

/** get the Null Date of the XModel and set it to the UnitConverter */
sal_Bool SvXMLUnitConverter::setNullDate(const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& xModel)
{
    com::sun::star::uno::Reference <com::sun::star::util::XNumberFormatsSupplier> xNumberFormatsSupplier (xModel, com::sun::star::uno::UNO_QUERY);
    if (xNumberFormatsSupplier.is())
    {
        com::sun::star::uno::Reference <com::sun::star::beans::XPropertySet> xPropertySet = xNumberFormatsSupplier->getNumberFormatSettings();
        if (xPropertySet.is() )
        {
            com::sun::star::uno::Any aAnyNullDate = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_NULLDATE)));
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

    String sDate = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( "PT" ));
    fValue *= 24;
    double fHoursValue = SolarMath::ApproxFloor (fValue);
    fValue -= fHoursValue;
    fValue *= 60;
    double fMinsValue = SolarMath::ApproxFloor (fValue);
    fValue -= fMinsValue;
    fValue *= 60;
    double fSecsValue = SolarMath::ApproxFloor (fValue);
    fValue -= fSecsValue;
    double f100SecsValue;
    if (fValue > 0.00001)
    {
        String sTemp;
        SolarMath::DoubleToString(sTemp, fValue, 'A', XML_MAXDIGITSCOUNT_TIME - 5, '.', sal_True);
        rtl::OUString sOUDate(sTemp);
        f100SecsValue = sOUDate.toDouble();
    }
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
        sDate += '0';
    SolarMath::DoubleToString(sDate, fHoursValue, 'A', INT_MAX, '.', sal_True);
    sDate += 'H';
    if (fMinsValue < 10)
        sDate += '0';
    SolarMath::DoubleToString(sDate, fMinsValue, 'A', INT_MAX, '.', sal_True);
    sDate += 'M';
    if (fSecsValue < 10)
        sDate += '0';
    SolarMath::DoubleToString(sDate, fSecsValue, 'A', INT_MAX, '.', sal_True);
    if (f100SecsValue > 0.0)
    {
        sDate += ',';
        xub_StrLen nDateLen = sDate.Len();
        SolarMath::DoubleToString(sDate, fValue, 'A', XML_MAXDIGITSCOUNT_TIME - 5, '.', sal_True);
        if (nDateLen + 2 < sDate.Len())
            sDate.Erase(nDateLen, 2);
        else
            sDate.Erase(nDateLen - 1, 2);
    }
    sDate += 'S';

    rBuffer.append(sDate);
}

/** convert ISO Time String to double; negative durations allowed */
sal_Bool SvXMLUnitConverter::convertTime( double& fTime,
                            const ::rtl::OUString& rString)
{
    rtl::OUString aTrimmed = rString.trim().toUpperCase();
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

    rtl::OUString sDoubleStr;
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
            if ( nTemp >= LONG_MAX / 10 )
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
    double fTempTime = 0.0;
    if( convertTime( fTempTime, rString ) )
    {
        fTempTime *= 24;
        double fHoursValue = SolarMath::ApproxFloor (fTempTime);
        fTempTime -= fHoursValue;
        fTempTime *= 60;
        double fMinsValue = SolarMath::ApproxFloor (fTempTime);
        fTempTime -= fMinsValue;
        fTempTime *= 60;
        double fSecsValue = SolarMath::ApproxFloor (fTempTime);
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
                            const double& fDateTime, const com::sun::star::util::Date& aTempNullDate)
{
    double fValue = fDateTime;
    sal_Int32 nValue = static_cast <sal_Int32> (SolarMath::ApproxFloor (fValue));
    Date aDate (aTempNullDate.Day, aTempNullDate.Month, aTempNullDate.Year);
    aDate += nValue;
    fValue -= nValue;
    double fCount;
    if (nValue > 0)
         fCount = SolarMath::ApproxFloor (log10(nValue)) + 1;
    else if (nValue < 0)
         fCount = SolarMath::ApproxFloor (log10(nValue * -1)) + 1;
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
        fHoursValue = SolarMath::ApproxFloor (fValue);
        fValue -= fHoursValue;
        fValue *= 60;
        fMinsValue = SolarMath::ApproxFloor (fValue);
        fValue -= fMinsValue;
        fValue *= 60;
        fSecsValue = SolarMath::ApproxFloor (fValue);
        fValue -= fSecsValue;
        if (fValue > 0.0)
        {
            String sTemp;
            SolarMath::DoubleToString(sTemp, fValue, 'A', XML_MAXDIGITSCOUNT_TIME - nCount, '.', sal_True);
            rtl::OUString sOUDate(sTemp);
            f100SecsValue = sOUDate.toDouble();
        }
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
    String sDate;
    SolarMath::DoubleToString(sDate, static_cast <double> (aDate.GetYear()), 'A', INT_MAX, '.', sal_True);
    sDate += '-';
    USHORT nTemp = aDate.GetMonth();
    if (nTemp < 10)
        sDate += '0';
    SolarMath::DoubleToString(sDate, static_cast <double> (nTemp), 'A', INT_MAX, '.', sal_True);
    sDate += '-';
    nTemp = aDate.GetDay();
    if (nTemp < 10)
        sDate += '0';
    SolarMath::DoubleToString(sDate, static_cast <double> (nTemp), 'A', INT_MAX, '.', sal_True);
    if(bHasTime)
    {
        sDate += 'T';
        if (fHoursValue < 10)
            sDate += '0';
        SolarMath::DoubleToString(sDate, fHoursValue, 'A', INT_MAX, '.', sal_True);
        sDate += ':';
        if (fMinsValue < 10)
            sDate += '0';
        SolarMath::DoubleToString(sDate, fMinsValue, 'A', INT_MAX, '.', sal_True);
        sDate += ':';
        if (fSecsValue < 10)
            sDate += '0';
        SolarMath::DoubleToString(sDate, fSecsValue, 'A', INT_MAX, '.', sal_True);
        if (f100SecsValue > 0.0)
        {
            sDate += ',';
            xub_StrLen nDateLen = sDate.Len();
            SolarMath::DoubleToString(sDate, fValue, 'A', XML_MAXDIGITSCOUNT_TIME - nCount, '.', sal_True);
            if (nDateLen + 2 < sDate.Len())
                sDate.Erase(nDateLen, 2);
            else
                sDate.Erase(nDateLen - 1, 2);
        }
    }
    rBuffer.append(sDate);
}

/** convert ISO Date Time String to double */
sal_Bool SvXMLUnitConverter::convertDateTime( double& fDateTime,
                            const ::rtl::OUString& rString, const com::sun::star::util::Date& aTempNullDate)
{
    sal_Bool bSuccess = sal_True;

    rtl::OUString aDateStr, aTimeStr, sDoubleStr;
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

    sal_Int32 nDateTokens = aDateStr.getTokenCount('-');
    if ( nDateTokens > 3 || aDateStr.len() == 0 )
        bSuccess = sal_False;
    else
    {
        if ( !convertNumber( nYear, aDateStr.getToken( 0, '-' ), 0, 9999 ) )
            bSuccess = sal_False;
        if ( nDateTokens >= 2 )
            if ( !convertNumber( nMonth, aDateStr.getToken( 1, '-' ), 0, 12 ) )
                bSuccess = sal_False;
        if ( nDateTokens >= 3 )
            if ( !convertNumber( nDay, aDateStr.getToken( 2, '-' ), 0, 31 ) )
                bSuccess = sal_False;
    }

    if ( aTimeStr.len() > 0 )           // time is optional
    {
        sal_Int32 nTimeTokens = aTimeStr.getTokenCount(':');
        if ( nTimeTokens > 3 )
            bSuccess = sal_False;
        else
        {
            if ( !convertNumber( nHour, aTimeStr.getToken( 0, ':' ), 0, 23 ) )
                bSuccess = sal_False;
            if ( nTimeTokens >= 2 )
                if ( !convertNumber( nMin, aTimeStr.getToken( 1, ':' ), 0, 59 ) )
                    bSuccess = sal_False;
            if ( nTimeTokens >= 3 )
                if ( !convertNumber( nSec, aTimeStr.getToken( 2, ':' ), 0, 59 ) )
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
                                const com::sun::star::util::DateTime& rDateTime )
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
    }

    rBuffer.append( aString );
}

/** convert ISO Date String to util::DateTime */
sal_Bool SvXMLUnitConverter::convertDateTime( com::sun::star::util::DateTime& rDateTime,
                                     const ::rtl::OUString& rString )
{
    sal_Bool bSuccess = sal_True;

    rtl::OUString aDateStr, aTimeStr, sDoubleStr;
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

    sal_Int32 nDateTokens = aDateStr.getTokenCount('-');
    if ( nDateTokens > 3 || aDateStr.len() == 0 )
        bSuccess = sal_False;
    else
    {
        if ( !convertNumber( nYear, aDateStr.getToken( 0, '-' ), 0, 9999 ) )
            bSuccess = sal_False;
        if ( nDateTokens >= 2 )
            if ( !convertNumber( nMonth, aDateStr.getToken( 1, '-' ), 0, 12 ) )
                bSuccess = sal_False;
        if ( nDateTokens >= 3 )
            if ( !convertNumber( nDay, aDateStr.getToken( 2, '-' ), 0, 31 ) )
                bSuccess = sal_False;
    }

    if ( aTimeStr.len() > 0 )           // time is optional
    {
        sal_Int32 nTimeTokens = aTimeStr.getTokenCount(':');
        if ( nTimeTokens > 3 )
            bSuccess = sal_False;
        else
        {
            if ( !convertNumber( nHour, aTimeStr.getToken( 0, ':' ), 0, 23 ) )
                bSuccess = sal_False;
            if ( nTimeTokens >= 2 )
                if ( !convertNumber( nMin, aTimeStr.getToken( 1, ':' ), 0, 59 ) )
                    bSuccess = sal_False;
            if ( nTimeTokens >= 3 )
                if ( !convertNumber( nSec, aTimeStr.getToken( 2, ':' ), 0, 59 ) )
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
        rDateTime.HundredthSeconds = 0;
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

    int nErr;

    rVector.X() = SolarMath::StringToDouble(aContentX, sal_Unicode(','), sal_Unicode('.'),nErr);

    if(nErr)
        return sal_False;

    rVector.Y() = SolarMath::StringToDouble(aContentY, sal_Unicode(','), sal_Unicode('.'),nErr);

    if(nErr)
        return sal_False;

    rVector.Z() = SolarMath::StringToDouble(aContentZ, sal_Unicode(','), sal_Unicode('.'),nErr);

    return (nErr == 0L);
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
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0-15

      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 16-31

      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63, // 32-47
//                                                +               /

     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0, // 48-63
//    0   1   2   3   4   5   6   7   8   9               =

      0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 64-79
//        A   B   C   D   E   F   G   H   I   J   K   L   M   N   O

     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0,  0, // 80-95
//    P   Q   R   S   T   U   V   W   X   Y   Z

      0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
//        a   b   c   d   e   f   g   h   i   j   k   l   m   n   o

     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  0,  0,  0,  0,  0, // 112-127
//    p   q   r   s   t   u   v   w   x   y   z

      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};


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

void SvXMLUnitConverter::encodeBase64(rtl::OUStringBuffer& aStrBuffer, const uno::Sequence<sal_Int8>& aPass)
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

const rtl::OUString s2equal(RTL_CONSTASCII_USTRINGPARAM("=="));
const rtl::OUString s1equal(RTL_CONSTASCII_USTRINGPARAM("="));

void FourByteToThreeByte (sal_Int8* pBuffer, sal_Int32& nLength, const sal_Int32 nStart, const rtl::OUString& sString)
{
    nLength = 0;
    sal_Int32 nLen (sString.getLength());

    if (nLen != 4)
    {
        DBG_ERROR("wrong length");
        return;
    }


    if (sString.indexOf(s2equal) == 2)
        nLength = 1;
    else if (sString.indexOf(s1equal) == 3)
        nLength = 2;
    else
        nLength = 3;

    sal_Int32 nBinaer ((aBase64DecodeTable [sString [0]] << 18) +
            (aBase64DecodeTable [sString [1]] << 12) +
            (aBase64DecodeTable [sString [2]] <<  6) +
            (aBase64DecodeTable [sString [3]]));

    sal_uInt8 OneByte ((nBinaer & 0xFF0000) >> 16);
    pBuffer[nStart + 0] = (sal_Int8)OneByte;

    if (nLength == 1)
        return;

    OneByte = (nBinaer & 0xFF00) >> 8;
    pBuffer[nStart + 1] = (sal_Int8)OneByte;

    if (nLength == 2)
        return;

    OneByte = nBinaer & 0xFF;
    pBuffer[nStart + 2] = (sal_Int8)OneByte;
}

void SvXMLUnitConverter::decodeBase64(uno::Sequence<sal_Int8>& aBuffer, const rtl::OUString& sBuffer)
{
    sal_Int32 nFirstLength((sBuffer.getLength() / 4) * 3);
    sal_Int8* pBuffer = new sal_Int8[nFirstLength];
    sal_Int32 nSecondLength(0);
    sal_Int32 nLength(0);
    sal_Int32 i = 0;
    sal_Int32 k = 0;
    while (i < sBuffer.getLength())
    {
        FourByteToThreeByte (pBuffer, nLength, k, sBuffer.copy(i, 4));
        nSecondLength += nLength;
        nLength = 0;
        i += 4;
        k += 3;
    }
    aBuffer = uno::Sequence<sal_Int8>(pBuffer, nSecondLength);
    delete[] pBuffer;
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
        if( !bExt && rNumLetterSync.equalsAsciiL( sXML_true, sizeof(sXML_true)-1 ) )
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
    const sal_Char *pFormat = 0;
    sal_Bool bExt = sal_False;
    switch( nType )
    {
    case NumberingType::CHARS_UPPER_LETTER: pFormat = sXML_A; break;
    case NumberingType::CHARS_LOWER_LETTER: pFormat = sXML_a; break;
    case NumberingType::ROMAN_UPPER:            pFormat = sXML_I; break;
    case NumberingType::ROMAN_LOWER:            pFormat = sXML_i; break;
    case NumberingType::ARABIC:             pFormat = sXML_1; break;
    case NumberingType::CHARS_UPPER_LETTER_N:   pFormat = sXML_A; break;
    case NumberingType::CHARS_LOWER_LETTER_N:   pFormat = sXML_a; break;
    case NumberingType::NUMBER_NONE:            pFormat = sXML__empty; break;

    case NumberingType::CHAR_SPECIAL:
    case NumberingType::PAGE_DESCRIPTOR:
    case NumberingType::BITMAP:
        DBG_ASSERT( pFormat, "invalid number format" );
        break;
    default:
        bExt = sal_True;
        break;
    }

    if( pFormat )
    {
        rBuffer.appendAscii( pFormat );
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
    const sal_Char *pSync = 0;
    switch( nType )
    {
    case NumberingType::CHARS_UPPER_LETTER:
    case NumberingType::CHARS_LOWER_LETTER:
    case NumberingType::ROMAN_UPPER:
    case NumberingType::ROMAN_LOWER:
    case NumberingType::ARABIC:
    case NumberingType::NUMBER_NONE:
        // default
        // pSync = sXML_false;
        break;

    case NumberingType::CHARS_UPPER_LETTER_N:
    case NumberingType::CHARS_LOWER_LETTER_N:
        pSync = sXML_true;
        break;

    case NumberingType::CHAR_SPECIAL:
    case NumberingType::PAGE_DESCRIPTOR:
    case NumberingType::BITMAP:
        DBG_ASSERT( pSync, "invalid number format" );
        break;
    }
    if( pSync )
        rBuffer.appendAscii( pSync );
}


