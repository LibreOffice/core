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

#include <cmath>

#include "propertyimport.hxx"

#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <o3tl/temporary.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <comphelper/extract.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <unotools/datetime.hxx>
#include <rtl/strbuf.hxx>

using namespace ::xmloff::token;

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml;
    using ::com::sun::star::xml::sax::XAttributeList;
    using ::com::sun::star::xml::sax::XFastAttributeList;

    // NO using namespace ...util !!!
    // need a tools Date/Time/DateTime below, which would conflict with the uno types then

#define TYPE_DATE       1
#define TYPE_TIME       2
#define TYPE_DATETIME   3

//= PropertyConversion
namespace
{
    css::util::Time lcl_getTime(double _nValue)
    {
        css::util::Time aTime;
        sal_uInt64 nIntValue = static_cast<sal_uInt64>(_nValue * ::tools::Time::nanoSecPerDay);
        aTime.NanoSeconds = nIntValue % ::tools::Time::nanoSecPerSec;
        nIntValue /= ::tools::Time::nanoSecPerSec;
        aTime.Seconds = nIntValue % ::tools::Time::secondPerMinute;
        nIntValue /= ::tools::Time::secondPerMinute;
        aTime.Minutes = nIntValue % ::tools::Time::minutePerHour;
        nIntValue /= ::tools::Time::minutePerHour;
        OSL_ENSURE(nIntValue < 24, "lcl_getTime: more than a day?");
        aTime.Hours = nIntValue;

        return aTime;
    }

    css::util::Date lcl_getDate( double _nValue )
    {
        Date aToolsDate(static_cast<sal_uInt32>(_nValue));
        css::util::Date aDate;
        ::utl::typeConvert(aToolsDate, aDate);
        return aDate;
    }
}

Any PropertyConversion::convertString( const css::uno::Type& _rExpectedType,
    const OUString& _rReadCharacters, const SvXMLEnumMapEntry<sal_uInt16>* _pEnumMap, const bool _bInvertBoolean )
{
    Any aReturn;
    bool bEnumAsInt = false;
    switch (_rExpectedType.getTypeClass())
    {
        case TypeClass_BOOLEAN:     // sal_Bool
        {
            bool bValue;
            bool bSuccess =
                ::sax::Converter::convertBool(bValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    OStringBuffer("PropertyConversion::convertString: could not convert \"" +
                        OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US) +
                        "\" into a boolean!").getStr());
            aReturn <<= (_bInvertBoolean ? !bValue : bValue);
        }
        break;
        case TypeClass_SHORT:       // sal_Int16
        case TypeClass_LONG:        // sal_Int32
            if (!_pEnumMap)
            {   // it's a real int32/16 property
                sal_Int32 nValue(0);
                bool bSuccess =
                    ::sax::Converter::convertNumber(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        OStringBuffer("PropertyConversion::convertString: could not convert \"" +
                            OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US) +
                            "\" into an integer!").getStr());
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= static_cast<sal_Int16>(nValue);
                else
                    aReturn <<= nValue;
                break;
            }
            bEnumAsInt = true;
            [[fallthrough]];
        case TypeClass_ENUM:
        {
            sal_uInt16 nEnumValue(0);
            bool bSuccess = SvXMLUnitConverter::convertEnum(nEnumValue, _rReadCharacters, _pEnumMap);
            OSL_ENSURE(bSuccess, "PropertyConversion::convertString: could not convert to an enum value!");

            if (bEnumAsInt)
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= static_cast<sal_Int16>(nEnumValue);
                else
                    aReturn <<= static_cast<sal_Int32>(nEnumValue);
            else
                aReturn = ::cppu::int2enum(static_cast<sal_Int32>(nEnumValue), _rExpectedType);
        }
        break;
        case TypeClass_HYPER:
        {
            OSL_FAIL("PropertyConversion::convertString: 64-bit integers not implemented yet!");
        }
        break;
        case TypeClass_DOUBLE:
        {
            double nValue;
            bool bSuccess =
                ::sax::Converter::convertDouble(nValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    OStringBuffer(OString::Concat("PropertyConversion::convertString: could not convert \"") +
                        OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US) +
                        "\" into a double!").getStr());
            aReturn <<= nValue;
        }
        break;
        case TypeClass_STRING:
            aReturn <<= _rReadCharacters;
            break;
        case TypeClass_STRUCT:
        {
            sal_Int32 nType = 0;
            if ( _rExpectedType.equals( ::cppu::UnoType< css::util::Date >::get() ) )
                nType = TYPE_DATE;
            else if ( _rExpectedType.equals( ::cppu::UnoType< css::util::Time >::get() ) )
                nType = TYPE_TIME;
            else  if ( _rExpectedType.equals( ::cppu::UnoType< css::util::DateTime >::get() ) )
                nType = TYPE_DATETIME;

            if ( nType )
            {
                // first extract the double
                double nValue = 0;
                bool bSuccess =
                    ::sax::Converter::convertDouble(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        OStringBuffer("PropertyConversion::convertString: could not convert \"" +
                            OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US) +
                            "\" into a double!").getStr());

                // then convert it into the target type
                switch (nType)
                {
                    case TYPE_DATE:
                    {
                        OSL_ENSURE(std::modf(nValue, &o3tl::temporary(double())) == 0,
                            "PropertyConversion::convertString: a Date value with a fractional part?");
                        aReturn <<= lcl_getDate(nValue);
                    }
                    break;
                    case TYPE_TIME:
                    {
                        OSL_ENSURE((static_cast<sal_uInt32>(nValue)) == 0,
                            "PropertyConversion::convertString: a tools::Time value with more than a fractional part?");
                        aReturn <<= lcl_getTime(nValue);
                    }
                    break;
                    case TYPE_DATETIME:
                    {
                        css::util::Time aTime = lcl_getTime(nValue);
                        css::util::Date aDate = lcl_getDate(nValue);

                        css::util::DateTime aDateTime;
                        aDateTime.NanoSeconds = aTime.NanoSeconds;
                        aDateTime.Seconds = aTime.Seconds;
                        aDateTime.Minutes = aTime.Minutes;
                        aDateTime.Hours = aTime.Hours;
                        aDateTime.Day = aDate.Day;
                        aDateTime.Month = aDate.Month;
                        aDateTime.Year = aDate.Year;
                        aReturn <<= aDateTime;
                    }
                    break;
                }
            }
            else
                OSL_FAIL("PropertyConversion::convertString: unsupported property type!");
        }
        break;
        default:
            OSL_FAIL("PropertyConversion::convertString: invalid type class!");
    }

    return aReturn;
}

Type PropertyConversion::xmlTypeToUnoType( const OUString& _rType )
{
    Type aUnoType( cppu::UnoType<void>::get() );

    static std::map< OUString, css::uno::Type > s_aTypeNameMap
    {
        { token::GetXMLToken( token::XML_BOOLEAN ) , cppu::UnoType<bool>::get()},
        // Not a copy paste error, quotation from:
        // http://nabble.documentfoundation.org/Question-unoType-for-getXmlToken-dbaccess-reportdesign-module-tp4109071p4109116.html
        // all numeric types (including the UNO double)
        // consistently map to XML_FLOAT, so taking the extra precision from the
        // C++ type "float" to "double" makes absolute sense
        { token::GetXMLToken( token::XML_FLOAT )   , ::cppu::UnoType<double>::get()},
        { token::GetXMLToken( token::XML_STRING )  , ::cppu::UnoType<OUString>::get()},
        { token::GetXMLToken( token::XML_VOID )    , cppu::UnoType<void>::get() },
    };

    const std::map< OUString, css::uno::Type >::iterator aTypePos = s_aTypeNameMap.find( _rType );
    OSL_ENSURE( s_aTypeNameMap.end() != aTypePos, "PropertyConversion::xmlTypeToUnoType: invalid property name!" );
    if ( s_aTypeNameMap.end() != aTypePos )
        aUnoType = aTypePos->second;

    return aUnoType;
}

//= OPropertyImport
OPropertyImport::OPropertyImport(OFormLayerXMLImport_Impl& _rImport)
    :SvXMLImportContext(_rImport.getGlobalContext())
    ,m_rContext(_rImport)
    ,m_bTrackAttributes(false)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OPropertyImport::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if( (nElement & TOKEN_MASK) == token::XML_PROPERTIES )
    {
        return new OPropertyElementsContext( m_rContext.getGlobalContext(), this);
    }
    else
        SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    return nullptr;
}

void OPropertyImport::startFastElement(sal_Int32 /*nElement*/, const Reference< XFastAttributeList >& xAttrList)
{

    // assume the 'worst' case: all attributes describe properties. This should save our property array
    // some reallocs
    m_aValues.reserve(sax_fastparser::castToFastAttributeList(xAttrList).size());

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        handleAttribute(aIter.getToken(), aIter.toString());

        if (m_bTrackAttributes)
            m_aEncounteredAttributes.insert(aIter.getToken() & TOKEN_MASK);
    }

    // TODO: create PropertyValues for all the attributes which were not present, because they were implied
    // this is necessary as soon as we have properties where the XML default is different from the property
    // default
}

bool OPropertyImport::encounteredAttribute(sal_Int32 nAttributeToken) const
{
    OSL_ENSURE(m_bTrackAttributes, "OPropertyImport::encounteredAttribute: attribute tracking not enabled!");
    return m_aEncounteredAttributes.end() != m_aEncounteredAttributes.find(nAttributeToken & TOKEN_MASK);
}

void OPropertyImport::characters(const OUString& _rChars )
{
    // ignore them (should be whitespace only)
    OSL_ENSURE(_rChars.trim().isEmpty(), "OPropertyImport::Characters: non-whitespace characters!");
}

bool OPropertyImport::handleAttribute(sal_Int32 nAttributeToken, const OUString& _rValue)
{
    const OAttribute2Property::AttributeAssignment* pProperty = m_rContext.getAttributeMap().getAttributeTranslation(nAttributeToken & TOKEN_MASK);
    if (pProperty)
    {
        // create and store a new PropertyValue
        PropertyValue aNewValue;
        aNewValue.Name = pProperty->sPropertyName;

        // convert the value string into the target type
        if ((nAttributeToken & TOKEN_MASK) == token::XML_HREF)
        {
            aNewValue.Value <<= m_rContext.getGlobalContext().GetAbsoluteReference(_rValue);
        }
        else
        {
            aNewValue.Value = PropertyConversion::convertString(
                pProperty->aPropertyType, _rValue, pProperty->pEnumMap,
                pProperty->bInverseSemantics);
        }
        implPushBackPropertyValue( aNewValue );
        return true;
    }
    if ((nAttributeToken & TOKEN_MASK) != token::XML_TYPE)  // xlink:type is valid but ignored for <form:form>
    {
        SAL_WARN( "xmloff", "OPropertyImport::handleAttribute: Can't handle "
                    << SvXMLImport::getPrefixAndNameFromToken(nAttributeToken) << "=" << _rValue );
        return false;
    }
    return true;
}

//= OPropertyElementsContext
OPropertyElementsContext::OPropertyElementsContext(SvXMLImport& _rImport,
        const OPropertyImportRef& _rPropertyImporter)
    :SvXMLImportContext(_rImport)
    ,m_xPropertyImporter(_rPropertyImporter)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OPropertyElementsContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if( (nElement & TOKEN_MASK) == XML_PROPERTY )
    {
        return new OSinglePropertyContext(GetImport(), m_xPropertyImporter);
    }
    else if( (nElement & TOKEN_MASK) == XML_LIST_PROPERTY )
    {
        return new OListPropertyContext( GetImport(), m_xPropertyImporter );
    }
    return nullptr;
}

#if OSL_DEBUG_LEVEL > 0
    void OPropertyElementsContext::startFastElement(
        sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
    {
        OSL_ENSURE(0 == xAttrList->getFastAttributes().getLength(), "OPropertyElementsContext::StartElement: the form:properties element should not have attributes!");
    }

    void OPropertyElementsContext::characters(const OUString& _rChars)
    {
        OSL_ENSURE(_rChars.trim().isEmpty(), "OPropertyElementsContext::Characters: non-whitespace characters detected!");
    }
#endif

//= OSinglePropertyContext
OSinglePropertyContext::OSinglePropertyContext(SvXMLImport& _rImport,
        const OPropertyImportRef& _rPropertyImporter)
    :SvXMLImportContext(_rImport)
    ,m_xPropertyImporter(_rPropertyImporter)
{
}

void OSinglePropertyContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    css::beans::PropertyValue aPropValue;      // the property the instance imports currently
    css::uno::Type aPropType;          // the type of the property the instance imports currently

    OUString sType, sValue;
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(FORM, XML_PROPERTY_NAME):
                aPropValue.Name = aIter.toString();
                break;
            case XML_ELEMENT(OFFICE, XML_VALUE_TYPE):
                sType = aIter.toString();
                break;
            case XML_ELEMENT(OFFICE, XML_VALUE):
            case XML_ELEMENT(OFFICE, XML_BOOLEAN_VALUE):
            case XML_ELEMENT(OFFICE, XML_STRING_VALUE):
                sValue = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    // the name of the property
    OSL_ENSURE(!aPropValue.Name.isEmpty(), "OSinglePropertyContext::StartElement: invalid property name!");

    // needs to be translated into a css::uno::Type
    aPropType = PropertyConversion::xmlTypeToUnoType( sType );
    if( TypeClass_VOID == aPropType.getTypeClass() )
    {
        aPropValue.Value = Any();
    }
    else
    {
        aPropValue.Value =
            PropertyConversion::convertString(aPropType,
                                           sValue);
    }

    // now that we finally have our property value, add it to our parent object
    if( !aPropValue.Name.isEmpty() )
        m_xPropertyImporter->implPushBackGenericPropertyValue(aPropValue);
}

//= OListPropertyContext
OListPropertyContext::OListPropertyContext( SvXMLImport& _rImport,
    const OPropertyImportRef& _rPropertyImporter )
    :SvXMLImportContext( _rImport )
    ,m_xPropertyImporter( _rPropertyImporter )
{
}

void OListPropertyContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(FORM, XML_PROPERTY_NAME):
                m_sPropertyName = aIter.toString();
                break;
            case XML_ELEMENT(OFFICE, XML_VALUE_TYPE):
                m_sPropertyType = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

void OListPropertyContext::endFastElement(sal_Int32 )
{
    OSL_ENSURE( !m_sPropertyName.isEmpty() && !m_sPropertyType.isEmpty(),
        "OListPropertyContext::EndElement: no property name or type!" );

    if ( m_sPropertyName.isEmpty() || m_sPropertyType.isEmpty() )
        return;

    Sequence< Any > aListElements( m_aListValues.size() );
    Any* pListElement = aListElements.getArray();
    css::uno::Type aType = PropertyConversion::xmlTypeToUnoType( m_sPropertyType );
    for ( const auto& rListValue : m_aListValues )
    {
        *pListElement = PropertyConversion::convertString( aType, rListValue );
        ++pListElement;
    }

    PropertyValue aSequenceValue;
    aSequenceValue.Name = m_sPropertyName;
    aSequenceValue.Value <<= aListElements;

    m_xPropertyImporter->implPushBackGenericPropertyValue( aSequenceValue );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OListPropertyContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if ( (nElement & TOKEN_MASK) == XML_LIST_VALUE )
    {
        m_aListValues.emplace_back();
        return new OListValueContext( GetImport(), *m_aListValues.rbegin() );
    }
    return nullptr;
}

//= OListValueContext
OListValueContext::OListValueContext( SvXMLImport& _rImport, OUString& _rListValueHolder )
    :SvXMLImportContext( _rImport )
    ,m_rListValueHolder( _rListValueHolder )
{
}

void OListValueContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(OFFICE, XML_VALUE):
            case XML_ELEMENT(OFFICE, XML_STRING_VALUE):
            case XML_ELEMENT(OFFICE, XML_BOOLEAN_VALUE):
                m_rListValueHolder = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
