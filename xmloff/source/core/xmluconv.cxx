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

#include <sal/config.h>

#include <string_view>

#include <xmloff/xmluconv.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/math.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/fldunit.hxx>

#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <basegfx/vector/b3dvector.hxx>

#include <sax/tools/converter.hxx>


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;
using namespace com::sun::star::style;
using namespace ::com::sun::star::i18n;
using namespace ::xmloff::token;


const sal_Int8 XML_MAXDIGITSCOUNT_TIME = 11;
#define XML_NULLDATE "NullDate"

struct SvXMLUnitConverter::Impl
{
    sal_Int16 m_eCoreMeasureUnit; /*css::util::MeasureUnit*/
    sal_Int16 m_eXMLMeasureUnit; /*css::util::MeasureUnit*/
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
    m_xNumTypeInfo.set(xDefNum, uno::UNO_QUERY);
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

void SvXMLUnitConverter::SetCoreMeasureUnit(sal_Int16 const eCoreMeasureUnit/*css::util::MeasureUnit*/)
{
    m_pImpl->m_eCoreMeasureUnit = eCoreMeasureUnit;
}

void SvXMLUnitConverter::SetXMLMeasureUnit(sal_Int16 const eXMLMeasureUnit/*css::util::MeasureUnit*/)
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

sal_Int16 SvXMLUnitConverter::GetMeasureUnit(FieldUnit const nFieldUnit)
{
    sal_Int16 eUnit = util::MeasureUnit::INCH;
    switch( nFieldUnit )
    {
    case FieldUnit::MM:
        eUnit = util::MeasureUnit::MM;
        break;
    case FieldUnit::CM:
    case FieldUnit::M:
    case FieldUnit::KM:
        eUnit = util::MeasureUnit::CM;
        break;
    case FieldUnit::TWIP:
        eUnit = util::MeasureUnit::TWIP;
        break;
    case FieldUnit::POINT:
    case FieldUnit::PICA:
        eUnit = util::MeasureUnit::POINT;
        break;
    case FieldUnit::MM_100TH:
        eUnit = util::MeasureUnit::MM_100TH;
        break;
    case FieldUnit::INCH:
        eUnit = util::MeasureUnit::INCH;
        break;
    default:
        assert(false);
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
bool SvXMLUnitConverter::convertEnumImpl( sal_uInt16& rEnum,
                                      const OUString& rValue,
                                      const SvXMLEnumStringMapEntry<sal_uInt16> *pMap )
{
    while( pMap->GetName() )
    {
        if( rValue.equalsAsciiL( pMap->GetName(), pMap->GetNameLength() ) )
        {
            rEnum = pMap->GetValue();
            return true;
        }
        ++pMap;
    }

    return false;
}

/** convert string to enum using given token map, if the enum is
    not found in the map, this method will return false */
bool SvXMLUnitConverter::convertEnumImpl(
    sal_uInt16& rEnum,
    const OUString& rValue,
    const SvXMLEnumMapEntry<sal_uInt16> *pMap )
{
    while( pMap->GetToken() != XML_TOKEN_INVALID )
    {
        if( IsXMLToken( rValue, pMap->GetToken() ) )
        {
            rEnum = pMap->GetValue();
            return true;
        }
        ++pMap;
    }
    return false;
}

/** convert enum to string using given token map with an optional
    default token. If the enum is not found in the map,
    this method will either use the given default or return
    false if no default is set */
bool SvXMLUnitConverter::convertEnumImpl(
    OUStringBuffer& rBuffer,
    sal_uInt16 nValue,
    const SvXMLEnumMapEntry<sal_uInt16> *pMap,
    enum XMLTokenEnum eDefault)
{
    enum XMLTokenEnum eTok = eDefault;

    while( pMap->GetToken() != XML_TOKEN_INVALID )
    {
        if( pMap->GetValue() == nValue )
        {
            eTok = pMap->GetToken();
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

static const sal_Char aHexTab[] = "0123456789abcdef";


/** convert double number to string (using ::rtl::math) */
void SvXMLUnitConverter::convertDouble(OUStringBuffer& rBuffer,
    double fNumber) const
{
    ::sax::Converter::convertDouble(rBuffer, fNumber,
        true/*bWriteUnits*/, m_pImpl->m_eCoreMeasureUnit, m_pImpl->m_eXMLMeasureUnit);
}

/** convert string to double number (using ::rtl::math) */
bool SvXMLUnitConverter::convertDouble(double& rValue,
    const OUString& rString) const
{
    sal_Int16 const eSrcUnit = ::sax::Converter::GetUnitFromString(
            rString, m_pImpl->m_eCoreMeasureUnit);

    return ::sax::Converter::convertDouble(rValue, rString,
        eSrcUnit, m_pImpl->m_eCoreMeasureUnit);
}

/** get the Null Date of the XModel and set it to the UnitConverter */
bool SvXMLUnitConverter::setNullDate(const css::uno::Reference <css::frame::XModel>& xModel)
{
    css::uno::Reference <css::util::XNumberFormatsSupplier> xNumberFormatsSupplier (xModel, css::uno::UNO_QUERY);
    if (xNumberFormatsSupplier.is())
    {
        const css::uno::Reference <css::beans::XPropertySet> xPropertySet = xNumberFormatsSupplier->getNumberFormatSettings();
        return xPropertySet.is() && (xPropertySet->getPropertyValue(XML_NULLDATE) >>= m_pImpl->m_aNullDate);
    }
    return false;
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
        const css::util::Date& aTempNullDate,
        bool bAddTimeIf0AM )
{
    double fValue = fDateTime;
    const sal_Int32 nDays = static_cast <sal_Int32> (::rtl::math::approxFloor (fValue));
    Date aDate (aTempNullDate.Day, aTempNullDate.Month, aTempNullDate.Year);
    aDate.AddDays( nDays);
    fValue -= nDays;
    const bool bHasTime = (fValue > 0.0);

    sal_Int16 nTempYear = aDate.GetYear();
    assert(nTempYear != 0);
    if (nTempYear < 0)
    {
        rBuffer.append( '-');
        nTempYear = -nTempYear;
    }
    if (nTempYear < 1000)
        rBuffer.append( '0');
    if (nTempYear < 100)
        rBuffer.append( '0');
    if (nTempYear < 10)
        rBuffer.append( '0');
    rBuffer.append( sal_Int32( nTempYear));
    rBuffer.append( '-');
    sal_uInt16 nTemp = aDate.GetMonth();
    assert(1 <= nTemp && nTemp <= 12);
    if (nTemp < 10)
        rBuffer.append( '0');
    rBuffer.append( sal_Int32( nTemp));
    rBuffer.append( '-');
    nTemp = aDate.GetDay();
    assert(1 <= nTemp && nTemp <= 31);
    if (nTemp < 10)
        rBuffer.append( '0');
    rBuffer.append( sal_Int32( nTemp));
    if (bHasTime || bAddTimeIf0AM)
    {
        double fCount;
        if (nDays > 0)
            fCount = ::rtl::math::approxFloor (log10(static_cast<double>(nDays))) + 1;
        else if (nDays < 0)
            fCount = ::rtl::math::approxFloor (log10(static_cast<double>(nDays * -1))) + 1;
        else
            fCount = 0.0;
        const int nDigits = sal_Int16(fCount) + 4;  // +4 for *86400 in seconds
        const int nFractionDecimals = std::max( XML_MAXDIGITSCOUNT_TIME - nDigits, 0);

        sal_uInt16 nHour, nMinute, nSecond;
        double fFractionOfSecond;
        // Pass the original date+time value for proper scaling and rounding.
        tools::Time::GetClock( fDateTime, nHour, nMinute, nSecond, fFractionOfSecond, nFractionDecimals);

        rBuffer.append( 'T');
        if (nHour < 10)
            rBuffer.append( '0');
        rBuffer.append( sal_Int32( nHour));
        rBuffer.append( ':');
        if (nMinute < 10)
            rBuffer.append( '0');
        rBuffer.append( sal_Int32( nMinute));
        rBuffer.append( ':');
        if (nSecond < 10)
            rBuffer.append( '0');
        rBuffer.append( sal_Int32( nSecond));
        if (nFractionDecimals)
        {
            // nFractionDecimals+1 to not round up what GetClock() carefully
            // truncated.
            OUString aFraction( ::rtl::math::doubleToUString( fFractionOfSecond,
                        rtl_math_StringFormat_F,
                        nFractionDecimals + 1, '.', true));
            const sal_Int32 nLen = aFraction.getLength();
            if ( nLen > 2 )
            {
                // Truncate nFractionDecimals+1 digit if it was not rounded to zero.
                const sal_Int32 nCount = nLen - 2 - static_cast<int>(nLen > nFractionDecimals + 2);
                rBuffer.append( '.');
                rBuffer.append( std::u16string_view(aFraction).substr(2, nCount));     // strip 0.
            }
        }
    }
}

/** convert ISO Date Time String to double */
bool SvXMLUnitConverter::convertDateTime( double& fDateTime,
                            const OUString& rString, const css::util::Date& aTempNullDate)
{
    css::util::DateTime aDateTime;
    bool bSuccess = ::sax::Converter::parseDateTime(aDateTime, rString);

    if (bSuccess)
    {
        const Date aTmpNullDate(aTempNullDate.Day, aTempNullDate.Month, aTempNullDate.Year);
        const Date aTempDate(aDateTime.Day, aDateTime.Month, aDateTime.Year);
        const sal_Int32 nTage = aTempDate - aTmpNullDate;
        double fTempDateTime = nTage;
        double Hour = aDateTime.Hours;
        double Min = aDateTime.Minutes;
        double Sec = aDateTime.Seconds;
        double NanoSec = aDateTime.NanoSeconds;
        fTempDateTime += Hour    / ::tools::Time::hourPerDay;
        fTempDateTime += Min     / ::tools::Time::minutePerDay;
        fTempDateTime += Sec     / ::tools::Time::secondPerDay;
        fTempDateTime += NanoSec / ::tools::Time::nanoSecPerDay;
        fDateTime = fTempDateTime;
    }
    return bSuccess;
}


SvXMLTokenEnumerator::SvXMLTokenEnumerator( const OUString& rString, sal_Unicode cSeparator /* = ' ' */ )
: maTokenString( rString ), mnNextTokenPos(0), mcSeparator( cSeparator )
{
}

bool SvXMLTokenEnumerator::getNextToken( OUString& rToken )
{
    if( -1 == mnNextTokenPos )
        return false;

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

    return true;
}

static bool lcl_getPositions(const OUString& _sValue,OUString& _rContentX,OUString& _rContentY,OUString& _rContentZ)
{
    if(_sValue.isEmpty() || _sValue[0] != '(')
        return false;

    sal_Int32 nPos(1);
    sal_Int32 nFound = _sValue.indexOf(' ', nPos);

    if(nFound == -1 || nFound <= nPos)
        return false;

    _rContentX = _sValue.copy(nPos, nFound - nPos);

    nPos = nFound + 1;
    nFound = _sValue.indexOf(' ', nPos);

    if(nFound == -1 || nFound <= nPos)
        return false;

    _rContentY = _sValue.copy(nPos, nFound - nPos);

    nPos = nFound + 1;
    nFound = _sValue.indexOf(')', nPos);

    if(nFound == -1 || nFound <= nPos)
        return false;

    _rContentZ = _sValue.copy(nPos, nFound - nPos);
    return true;

}
/** convert string to ::basegfx::B3DVector */
bool SvXMLUnitConverter::convertB3DVector( ::basegfx::B3DVector& rVector, const OUString& rValue )
{
    OUString aContentX,aContentY,aContentZ;
    if ( !lcl_getPositions(rValue,aContentX,aContentY,aContentZ) )
        return false;

    rtl_math_ConversionStatus eStatus;

    rVector.setX(::rtl::math::stringToDouble(aContentX, '.',
            ',', &eStatus));

    if( eStatus != rtl_math_ConversionStatus_Ok )
        return false;

    rVector.setY(::rtl::math::stringToDouble(aContentY, '.',
            ',', &eStatus));

    if( eStatus != rtl_math_ConversionStatus_Ok )
        return false;

    rVector.setZ(::rtl::math::stringToDouble(aContentZ, '.',
            ',', &eStatus));


    return ( eStatus == rtl_math_ConversionStatus_Ok );
}

/** convert ::basegfx::B3DVector to string */
void SvXMLUnitConverter::convertB3DVector( OUStringBuffer &rBuffer, const ::basegfx::B3DVector& rVector )
{
    rBuffer.append('(');
    ::sax::Converter::convertDouble(rBuffer, rVector.getX());
    rBuffer.append(' ');
    ::sax::Converter::convertDouble(rBuffer, rVector.getY());
    rBuffer.append(' ');
    ::sax::Converter::convertDouble(rBuffer, rVector.getZ());
    rBuffer.append(')');
}

/** convert string to Position3D */
bool SvXMLUnitConverter::convertPosition3D( drawing::Position3D& rPosition,
    const OUString& rValue )
{
    OUString aContentX,aContentY,aContentZ;
    if ( !lcl_getPositions(rValue,aContentX,aContentY,aContentZ) )
        return false;

    if ( !convertDouble( rPosition.PositionX, aContentX ) )
        return false;
    if ( !convertDouble( rPosition.PositionY, aContentY ) )
        return false;
    return convertDouble( rPosition.PositionZ, aContentZ );
}

/** convert Position3D to string */
void SvXMLUnitConverter::convertPosition3D( OUStringBuffer &rBuffer,
                                           const drawing::Position3D& rPosition )
{
    rBuffer.append( '(' );
    convertDouble( rBuffer, rPosition.PositionX );
    rBuffer.append( ' ' );
    convertDouble( rBuffer, rPosition.PositionY );
    rBuffer.append( ' ' );
    convertDouble( rBuffer, rPosition.PositionZ );
    rBuffer.append( ')' );
}

bool SvXMLUnitConverter::convertNumFormat(
        sal_Int16& rType,
        const OUString& rNumFmt,
        const OUString& rNumLetterSync,
        bool bNumberNone ) const
{
    bool bRet = true;
    bool bExt = false;

    sal_Int32 nLen = rNumFmt.getLength();
    if( 0 == nLen )
    {
        if( bNumberNone )
            rType = NumberingType::NUMBER_NONE;
        else
            bRet = false;
    }
    else if( 1 == nLen )
    {
        switch( rNumFmt[0] )
        {
        case '1':  rType = NumberingType::ARABIC;          break;
        case 'a':  rType = NumberingType::CHARS_LOWER_LETTER;  break;
        case 'A':  rType = NumberingType::CHARS_UPPER_LETTER;  break;
        case 'i':  rType = NumberingType::ROMAN_LOWER; break;
        case 'I':  rType = NumberingType::ROMAN_UPPER; break;
        default:                bExt = true; break;
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
        bExt = true;
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
        SAL_WARN_IF( eFormat == XML_TOKEN_INVALID, "xmloff", "invalid number format" );
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
                               sal_Int16 nType )
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
        SAL_WARN_IF( eSync == XML_TOKEN_INVALID, "xmloff", "invalid number format" );
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

void SvXMLUnitConverter::convertPropertySet(uno::Reference<beans::XPropertySet> const & rProperties,
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
        bool *pEncoded ) const
{
    if( pEncoded )
        *pEncoded = false;

    sal_Int32 nLen = rName.getLength();
    OUStringBuffer aBuffer( nLen );

    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        sal_Unicode c = rName[i];
        bool bValidChar = false;
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
                bValidChar = false;
            }
            else if( (c >= 0x02bbU && c <= 0x02c1U) || c == 0x0559 ||
                     c == 0x06e5 || c == 0x06e6 )
            {
                bValidChar = true;
            }
            else if( c == 0x0387 )
            {
                bValidChar = i > 0;
            }
            else
            {
                if (!m_pImpl->m_xCharClass.is())
                {
                    m_pImpl->m_xCharClass = CharacterClassification::create( m_pImpl->m_xContext );
                }
                sal_Int16 nType = m_pImpl->m_xCharClass->getType(rName, i);

                switch( nType )
                {
                case UnicodeType::UPPERCASE_LETTER:     // Lu
                case UnicodeType::LOWERCASE_LETTER:     // Ll
                case UnicodeType::TITLECASE_LETTER:     // Lt
                case UnicodeType::OTHER_LETTER:         // Lo
                case UnicodeType::LETTER_NUMBER:        // Nl
                    bValidChar = true;
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
            aBuffer.append( '_' );
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
            aBuffer.append( '_' );
            if( pEncoded )
                *pEncoded = true;
        }
    }

    // check for length
    if( aBuffer.getLength() > ((1<<15)-1) )
    {
        aBuffer = rName;
        if( pEncoded )
            *pEncoded = false;
    }


    return aBuffer.makeStringAndClear();
}

/** convert string (hex) to number (sal_uInt32) */
bool SvXMLUnitConverter::convertHex( sal_uInt32& nVal,
                                       const OUString& rValue )
{
    if( rValue.getLength() != 8 )
        return false;

    nVal = 0;
    for ( int i = 0; i < 8; i++ )
    {
        nVal = ( nVal << 4 )
            | sal::static_int_cast< sal_uInt32 >( lcl_gethex( rValue[i] ) );
    }

    return true;
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
