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


#include <xmloff/xmluconv.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/math.hxx>
#include <rtl/logfile.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/fldunit.hxx>

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <basegfx/vector/b3dvector.hxx>
#include <comphelper/processfactory.hxx>

#include <sax/tools/converter.hxx>


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;
using namespace com::sun::star::style;
using namespace ::com::sun::star::i18n;
using namespace ::xmloff::token;


const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 11;
const sal_Int8 XML_MAXDIGITSCOUNT_DATETIME = 6;
#define XML_NULLDATE "NullDate"

struct SvXMLUnitConverter::Impl
{
    sal_Int16 m_eCoreMeasureUnit;
    sal_Int16 m_eXMLMeasureUnit;
    util::Date m_aNullDate;
    mutable uno::Reference< text::XNumberingTypeInfo > m_xNumTypeInfo;
    mutable uno::Reference< i18n::XCharacterClassification > m_xCharClass;
    uno::Reference< uno::XComponentContext > m_xContext;

    Impl(uno::Reference<uno::XComponentContext> const& xContext,
            sal_Int16 const eCoreMeasureUnit,
            sal_Int16 const eXMLMeasureUnit)
        : m_eCoreMeasureUnit(eCoreMeasureUnit)
        , m_eXMLMeasureUnit(eXMLMeasureUnit)
        , m_aNullDate(30, 12, 1899)
        , m_xContext(xContext)
    {
        OSL_ENSURE( m_xContext.is(), "got no service manager" );
    }

    void createNumTypeInfo() const;
};


void SvXMLUnitConverter::Impl::createNumTypeInfo() const
{
    Reference<XDefaultNumberingProvider> xDefNum = DefaultNumberingProvider::create(m_xContext);
    m_xNumTypeInfo = Reference<XNumberingTypeInfo>(xDefNum, uno::UNO_QUERY);
}

const uno::Reference< text::XNumberingTypeInfo >&
SvXMLUnitConverter::getNumTypeInfo() const
{
    if (!m_pImpl->m_xNumTypeInfo.is())
    {
        m_pImpl->createNumTypeInfo();
    }
    return m_pImpl->m_xNumTypeInfo;
}

void SvXMLUnitConverter::SetCoreMeasureUnit(sal_Int16 const eCoreMeasureUnit)
{
    m_pImpl->m_eCoreMeasureUnit = eCoreMeasureUnit;
}

void SvXMLUnitConverter::SetXMLMeasureUnit(sal_Int16 const eXMLMeasureUnit)
{
    m_pImpl->m_eXMLMeasureUnit = eXMLMeasureUnit;
}

sal_Int16 SvXMLUnitConverter::GetXMLMeasureUnit() const
{
    return m_pImpl->m_eXMLMeasureUnit;
}

/** constructs a SvXMLUnitConverter. The core measure unit is the
    default unit for numerical measures, the XML measure unit is
    the default unit for textual measures
*/

SvXMLUnitConverter::SvXMLUnitConverter(
    const uno::Reference<uno::XComponentContext>& xContext,
    sal_Int16 const eCoreMeasureUnit,
    sal_Int16 const eXMLMeasureUnit)
: m_pImpl(new Impl(xContext, eCoreMeasureUnit, eXMLMeasureUnit))
{
}

SvXMLUnitConverter::~SvXMLUnitConverter()
{
}

sal_Int16 SvXMLUnitConverter::GetMeasureUnit(sal_Int16 const nFieldUnit)
{
    sal_Int16 eUnit = util::MeasureUnit::INCH;
    switch( nFieldUnit )
    {
    case FUNIT_MM:
        eUnit = util::MeasureUnit::MM;
        break;
    case FUNIT_CM:
    case FUNIT_M:
    case FUNIT_KM:
        eUnit = util::MeasureUnit::CM;
        break;
    case FUNIT_TWIP:
        eUnit = util::MeasureUnit::TWIP;
        break;
    case FUNIT_POINT:
    case FUNIT_PICA:
        eUnit = util::MeasureUnit::POINT;
        break;
    case FUNIT_100TH_MM:
        eUnit = util::MeasureUnit::MM_100TH;
        break;
    }
    return eUnit;
}

/** convert string to measure using optional min and max values*/
bool SvXMLUnitConverter::convertMeasureToCore( sal_Int32& nValue,
                                         const OUString& rString,
                                         sal_Int32 nMin, sal_Int32 nMax ) const
{
    return ::sax::Converter::convertMeasure( nValue, rString,
                                               m_pImpl->m_eCoreMeasureUnit,
                                               nMin, nMax );
}

/** convert measure to string */
void SvXMLUnitConverter::convertMeasureToXML( OUStringBuffer& rString,
                                         sal_Int32 nMeasure ) const
{
    ::sax::Converter::convertMeasure( rString, nMeasure,
                                        m_pImpl->m_eCoreMeasureUnit,
                                        m_pImpl->m_eXMLMeasureUnit );
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
        ++pMap;
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
        ++pMap;
    }
    return sal_False;
}

/** convert enum to string using given token map with an optional
    default token. If the enum is not found in the map,
    this method will either use the given default or return
    false if no default is set */
sal_Bool SvXMLUnitConverter::convertEnum(
    OUStringBuffer& rBuffer,
    unsigned int nValue,
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
        ++pMap;
    }

    // the map may have contained XML_TOKEN_INVALID
    if( eTok == XML_TOKEN_INVALID )
        eTok = eDefault;

    if( eTok != XML_TOKEN_INVALID )
        rBuffer.append( GetXMLToken(eTok) );

    return (eTok != XML_TOKEN_INVALID);
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

static sal_Char aHexTab[] = "0123456789abcdef";


/** convert double number to string (using ::rtl::math) */
void SvXMLUnitConverter::convertDouble(OUStringBuffer& rBuffer,
    double fNumber, sal_Bool bWriteUnits) const
{
    ::sax::Converter::convertDouble(rBuffer, fNumber,
        bWriteUnits, m_pImpl->m_eCoreMeasureUnit, m_pImpl->m_eXMLMeasureUnit);
}

/** convert string to double number (using ::rtl::math) */
sal_Bool SvXMLUnitConverter::convertDouble(double& rValue,
    const OUString& rString, sal_Bool bLookForUnits) const
{
    if(bLookForUnits)
    {
        sal_Int16 const eSrcUnit = ::sax::Converter::GetUnitFromString(
                rString, m_pImpl->m_eCoreMeasureUnit);

        return ::sax::Converter::convertDouble(rValue, rString,
            eSrcUnit, m_pImpl->m_eCoreMeasureUnit);
    }
    else
    {
        return ::sax::Converter::convertDouble(rValue, rString);
    }
}

/** get the Null Date of the XModel and set it to the UnitConverter */
sal_Bool SvXMLUnitConverter::setNullDate(const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& xModel)
{
    com::sun::star::uno::Reference <com::sun::star::util::XNumberFormatsSupplier> xNumberFormatsSupplier (xModel, com::sun::star::uno::UNO_QUERY);
    if (xNumberFormatsSupplier.is())
    {
        const com::sun::star::uno::Reference <com::sun::star::beans::XPropertySet> xPropertySet = xNumberFormatsSupplier->getNumberFormatSettings();
        return xPropertySet.is() && (xPropertySet->getPropertyValue(OUString(XML_NULLDATE)) >>= m_pImpl->m_aNullDate);
    }
    return sal_False;
}

/** convert double to ISO Date Time String */
void SvXMLUnitConverter::convertDateTime(OUStringBuffer& rBuffer,
                     const double& fDateTime, bool const bAddTimeIf0AM)
{
    convertDateTime(rBuffer, fDateTime, m_pImpl->m_aNullDate, bAddTimeIf0AM);
}

/** convert ISO Date Time String to double */
bool SvXMLUnitConverter::convertDateTime(double& fDateTime,
                     const OUString& rString)
{
    return convertDateTime(fDateTime, rString, m_pImpl->m_aNullDate);
}

/** convert double to ISO Date Time String */
void SvXMLUnitConverter::convertDateTime( OUStringBuffer& rBuffer,
        const double& fDateTime,
        const com::sun::star::util::Date& aTempNullDate,
        sal_Bool bAddTimeIf0AM )
{
    double fValue = fDateTime;
    sal_Int32 nValue = static_cast <sal_Int32> (::rtl::math::approxFloor (fValue));
    Date aDate (aTempNullDate.Day, aTempNullDate.Month, aTempNullDate.Year);
    aDate += nValue;
    fValue -= nValue;
    double fCount;
    if (nValue > 0)
         fCount = ::rtl::math::approxFloor (log10((double)nValue)) + 1;
    else if (nValue < 0)
         fCount = ::rtl::math::approxFloor (log10((double)(nValue * -1))) + 1;
    else
        fCount = 0.0;
    sal_Int16 nCount = sal_Int16(fCount);
    sal_Bool bHasTime(sal_False);
    double fHoursValue = 0;
    double fMinsValue = 0;
    double fSecsValue = 0;
    double f100SecsValue = 0;
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
    sal_uInt16 nTemp = aDate.GetYear();
    if (nTemp < 1000)
        rBuffer.append( sal_Unicode('0'));
    if (nTemp < 100)
        rBuffer.append( sal_Unicode('0'));
    if (nTemp < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( nTemp));
    rBuffer.append( sal_Unicode('-'));
    nTemp = aDate.GetMonth();
    if (nTemp < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( nTemp));
    rBuffer.append( sal_Unicode('-'));
    nTemp = aDate.GetDay();
    if (nTemp < 10)
        rBuffer.append( sal_Unicode('0'));
    rBuffer.append( sal_Int32( nTemp));
    if(bHasTime || bAddTimeIf0AM)
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
            OUString a100th( ::rtl::math::doubleToUString( fValue,
                        rtl_math_StringFormat_F,
                        XML_MAXDIGITSCOUNT_TIME - nCount, '.', sal_True));
            if ( a100th.getLength() > 2 )
            {
                rBuffer.append( sal_Unicode('.'));
                rBuffer.append( a100th.copy( 2 ) );     // strip 0.
            }
        }
    }
}

/** convert ISO Date Time String to double */
sal_Bool SvXMLUnitConverter::convertDateTime( double& fDateTime,
                            const OUString& rString, const com::sun::star::util::Date& aTempNullDate)
{
    com::sun::star::util::DateTime aDateTime;
    bool bSuccess = ::sax::Converter::parseDateTime(aDateTime, 0, rString);

    if (bSuccess)
    {
        const Date aTmpNullDate(aTempNullDate.Day, aTempNullDate.Month, aTempNullDate.Year);
        const Date aTempDate((sal_uInt16)aDateTime.Day, (sal_uInt16)aDateTime.Month, (sal_uInt16)aDateTime.Year);
        const sal_Int32 nTage = aTempDate - aTmpNullDate;
        double fTempDateTime = nTage;
        double Hour = aDateTime.Hours;
        double Min = aDateTime.Minutes;
        double Sec = aDateTime.Seconds;
        double NanoSec = aDateTime.NanoSeconds;
        fTempDateTime += Hour    / ::Time::hourPerDay;
        fTempDateTime += Min     / ::Time::minutePerDay;
        fTempDateTime += Sec     / ::Time::secondPerDay;
        fTempDateTime += NanoSec / ::Time::nanoSecPerDay;
        fDateTime = fTempDateTime;
    }
    return bSuccess;
}


SvXMLTokenEnumerator::SvXMLTokenEnumerator( const OUString& rString, sal_Unicode cSeparator /* = sal_Unicode(' ') */ )
: maTokenString( rString ), mnNextTokenPos(0), mcSeparator( cSeparator )
{
}

sal_Bool SvXMLTokenEnumerator::getNextToken( OUString& rToken )
{
    if( -1 == mnNextTokenPos )
        return sal_False;

    int nTokenEndPos = maTokenString.indexOf( mcSeparator, mnNextTokenPos );
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

static bool lcl_getPositions(const OUString& _sValue,OUString& _rContentX,OUString& _rContentY,OUString& _rContentZ)
{
    if(_sValue.isEmpty() || _sValue[0] != '(')
        return false;

    sal_Int32 nPos(1L);
    sal_Int32 nFound = _sValue.indexOf(sal_Unicode(' '), nPos);

    if(nFound == -1 || nFound <= nPos)
        return false;

    _rContentX = _sValue.copy(nPos, nFound - nPos);

    nPos = nFound + 1;
    nFound = _sValue.indexOf(sal_Unicode(' '), nPos);

    if(nFound == -1 || nFound <= nPos)
        return false;

    _rContentY = _sValue.copy(nPos, nFound - nPos);

    nPos = nFound + 1;
    nFound = _sValue.indexOf(sal_Unicode(')'), nPos);

    if(nFound == -1 || nFound <= nPos)
        return false;

    _rContentZ = _sValue.copy(nPos, nFound - nPos);
    return true;

}
/** convert string to ::basegfx::B3DVector */
sal_Bool SvXMLUnitConverter::convertB3DVector( ::basegfx::B3DVector& rVector, const OUString& rValue )
{
    OUString aContentX,aContentY,aContentZ;
    if ( !lcl_getPositions(rValue,aContentX,aContentY,aContentZ) )
        return sal_False;

    rtl_math_ConversionStatus eStatus;

    rVector.setX(::rtl::math::stringToDouble(aContentX, sal_Unicode('.'),
            sal_Unicode(','), &eStatus, NULL));

    if( eStatus != rtl_math_ConversionStatus_Ok )
        return sal_False;

    rVector.setY(::rtl::math::stringToDouble(aContentY, sal_Unicode('.'),
            sal_Unicode(','), &eStatus, NULL));

    if( eStatus != rtl_math_ConversionStatus_Ok )
        return sal_False;

    rVector.setZ(::rtl::math::stringToDouble(aContentZ, sal_Unicode('.'),
            sal_Unicode(','), &eStatus, NULL));


    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert ::basegfx::B3DVector to string */
void SvXMLUnitConverter::convertB3DVector( OUStringBuffer &rBuffer, const ::basegfx::B3DVector& rVector )
{
    rBuffer.append(sal_Unicode('('));
    ::sax::Converter::convertDouble(rBuffer, rVector.getX());
    rBuffer.append(sal_Unicode(' '));
    ::sax::Converter::convertDouble(rBuffer, rVector.getY());
    rBuffer.append(sal_Unicode(' '));
    ::sax::Converter::convertDouble(rBuffer, rVector.getZ());
    rBuffer.append(sal_Unicode(')'));
}

/** convert string to Position3D */
sal_Bool SvXMLUnitConverter::convertPosition3D( drawing::Position3D& rPosition,
    const OUString& rValue )
{
    OUString aContentX,aContentY,aContentZ;
    if ( !lcl_getPositions(rValue,aContentX,aContentY,aContentZ) )
        return sal_False;

    if ( !convertDouble( rPosition.PositionX, aContentX, sal_True ) )
        return sal_False;
    if ( !convertDouble( rPosition.PositionY, aContentY, sal_True ) )
        return sal_False;
    return convertDouble( rPosition.PositionZ, aContentZ, sal_True );
}

/** convert Position3D to string */
void SvXMLUnitConverter::convertPosition3D( OUStringBuffer &rBuffer,
                                           const drawing::Position3D& rPosition )
{
    rBuffer.append( sal_Unicode('(') );
    convertDouble( rBuffer, rPosition.PositionX, sal_True );
    rBuffer.append( sal_Unicode(' ') );
    convertDouble( rBuffer, rPosition.PositionY, sal_True );
    rBuffer.append( sal_Unicode(' ') );
    convertDouble( rBuffer, rPosition.PositionZ, sal_True );
    rBuffer.append( sal_Unicode(')') );
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
        const sal_Int32 nCount(aProps.getLength());
        if (nCount)
        {
            rProps.realloc(nCount);
            beans::PropertyValue* pProps = rProps.getArray();
            for (sal_Int32 i = 0; i < nCount; i++, ++pProps)
            {
                pProps->Name = aProps[i].Name;
                pProps->Value = aProperties->getPropertyValue(aProps[i].Name);
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


OUString SvXMLUnitConverter::encodeStyleName(
        const OUString& rName,
        sal_Bool *pEncoded ) const
{
    if( pEncoded )
        *pEncoded = sal_False;

    sal_Int32 nLen = rName.getLength();
    OUStringBuffer aBuffer( nLen );

    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        sal_Unicode c = rName[i];
        sal_Bool bValidChar = sal_False;
        if( c < 0x00ffU )
        {
            bValidChar =
                (c >= 0x0041 && c <= 0x005a) ||
                (c >= 0x0061 && c <= 0x007a) ||
                (c >= 0x00c0 && c <= 0x00d6) ||
                (c >= 0x00d8 && c <= 0x00f6) ||
                (c >= 0x00f8 && c <= 0x00ff) ||
                ( i > 0 && ( (c >= 0x0030 && c <= 0x0039) ||
                             c == 0x00b7 || c == '-' || c == '.') );
        }
        else
        {
            if( (c >= 0xf900U && c <= 0xfffeU) ||
                 (c >= 0x20ddU && c <= 0x20e0U))
            {
                bValidChar = sal_False;
            }
            else if( (c >= 0x02bbU && c <= 0x02c1U) || c == 0x0559 ||
                     c == 0x06e5 || c == 0x06e6 )
            {
                bValidChar = sal_True;
            }
            else if( c == 0x0387 )
            {
                bValidChar = i > 0;
            }
            else
            {
                if (!m_pImpl->m_xCharClass.is())
                {
                    this->m_pImpl->m_xCharClass = CharacterClassification::create( m_pImpl->m_xContext );
                }
                sal_Int16 nType = m_pImpl->m_xCharClass->getType(rName, i);

                switch( nType )
                {
                case UnicodeType::UPPERCASE_LETTER:     // Lu
                case UnicodeType::LOWERCASE_LETTER:     // Ll
                case UnicodeType::TITLECASE_LETTER:     // Lt
                case UnicodeType::OTHER_LETTER:         // Lo
                case UnicodeType::LETTER_NUMBER:        // Nl
                    bValidChar = sal_True;
                    break;
                case UnicodeType::NON_SPACING_MARK:     // Ms
                case UnicodeType::ENCLOSING_MARK:       // Me
                case UnicodeType::COMBINING_SPACING_MARK:   //Mc
                case UnicodeType::MODIFIER_LETTER:      // Lm
                case UnicodeType::DECIMAL_DIGIT_NUMBER: // Nd
                    bValidChar = i > 0;
                    break;
                }
            }
        }
        if( bValidChar )
        {
            aBuffer.append( c );
        }
        else
        {
            aBuffer.append( static_cast< sal_Unicode >( '_' ) );
            if( c > 0x0fff )
                aBuffer.append( static_cast< sal_Unicode >(
                            aHexTab[ (c >> 12) & 0x0f ]  ) );
            if( c > 0x00ff )
                aBuffer.append( static_cast< sal_Unicode >(
                        aHexTab[ (c >> 8) & 0x0f ] ) );
            if( c > 0x000f )
                aBuffer.append( static_cast< sal_Unicode >(
                        aHexTab[ (c >> 4) & 0x0f ] ) );
            aBuffer.append( static_cast< sal_Unicode >(
                        aHexTab[ c & 0x0f ] ) );
            aBuffer.append( static_cast< sal_Unicode >( '_' ) );
            if( pEncoded )
                *pEncoded = sal_True;
        }
    }

    // check for length
    if( aBuffer.getLength() > ((1<<15)-1) )
    {
        aBuffer = rName;
        if( pEncoded )
            *pEncoded = sal_False;
    }


    return aBuffer.makeStringAndClear();
}

/** convert string (hex) to number (sal_uInt32) */
sal_Bool SvXMLUnitConverter::convertHex( sal_uInt32& nVal,
                                       const OUString& rValue )
{
    if( rValue.getLength() != 8 )
        return sal_False;

    nVal = 0;
    for ( int i = 0; i < 8; i++ )
    {
        nVal = ( nVal << 4 )
            | sal::static_int_cast< sal_uInt32 >( lcl_gethex( rValue[i] ) );
    }

    return sal_True;
}

/** convert number (sal_uInt32) to string (hex) */
void SvXMLUnitConverter::convertHex( OUStringBuffer& rBuffer,
                                        sal_uInt32 nVal )
{
    for ( int i = 0; i < 8; i++ )
    {
        rBuffer.append( sal_Unicode( aHexTab[ nVal >> 28 ] ) );
        nVal <<= 4;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
