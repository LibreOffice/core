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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "propertyimport.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <osl/diagnose.h>
#include <comphelper/extract.hxx>
#include "callbacks.hxx"
#include "xmlnmspe.hxx"
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <unotools/datetime.hxx>
#include <rtl/logfile.hxx>

#if OSL_DEBUG_LEVEL > 0
    #ifndef _OSL_THREAD_H_
    #include <osl/thread.h>
    #endif
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml;

    // NO using namespace ...util !!!
    // need a tools Date/Time/DateTime below, which would conflict with the uno types then

#define TYPE_DATE       1
#define TYPE_TIME       2
#define TYPE_DATETIME   3

//=====================================================================
//= PropertyConversion
//=====================================================================
namespace
{
    //---------------------------------------------------------------------
    ::com::sun::star::util::Time lcl_getTime(double _nValue)
    {
        ::com::sun::star::util::Time aTime;
        sal_uInt32 nIntValue = sal_Int32(_nValue * 8640000);
        nIntValue *= 8640000;
        aTime.HundredthSeconds = (sal_uInt16)( nIntValue % 100 );
        nIntValue /= 100;
        aTime.Seconds = (sal_uInt16)( nIntValue % 60 );
        nIntValue /= 60;
        aTime.Minutes = (sal_uInt16)( nIntValue % 60 );
        nIntValue /= 60;
        OSL_ENSURE(nIntValue < 24, "lcl_getTime: more than a day?");
        aTime.Hours = static_cast< sal_uInt16 >( nIntValue );

        return aTime;
    }

    //---------------------------------------------------------------------
    static ::com::sun::star::util::Date lcl_getDate( double _nValue )
    {
        Date aToolsDate((sal_uInt32)_nValue);
        ::com::sun::star::util::Date aDate;
        ::utl::typeConvert(aToolsDate, aDate);
        return aDate;
    }
}

//---------------------------------------------------------------------
Any PropertyConversion::convertString( SvXMLImport& _rImporter, const ::com::sun::star::uno::Type& _rExpectedType,
    const ::rtl::OUString& _rReadCharacters, const SvXMLEnumMapEntry* _pEnumMap, const sal_Bool _bInvertBoolean )
{
    Any aReturn;
    sal_Bool bEnumAsInt = sal_False;
    switch (_rExpectedType.getTypeClass())
    {
        case TypeClass_BOOLEAN:     // sal_Bool
        {
            bool bValue;
        #if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
        #endif
            _rImporter.GetMM100UnitConverter().convertBool(bValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    ::rtl::OString("PropertyConversion::convertString: could not convert \"")
                +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=  ::rtl::OString("\" into a boolean!"));
            aReturn = ::cppu::bool2any(_bInvertBoolean ? !bValue : bValue);
        }
        break;
        case TypeClass_SHORT:       // sal_Int16
        case TypeClass_LONG:        // sal_Int32
            if (!_pEnumMap)
            {   // it's a real int32/16 property
                sal_Int32 nValue(0);
        #if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
        #endif
                _rImporter.GetMM100UnitConverter().convertNumber(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OString("PropertyConversion::convertString: could not convert \"")
                    +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("\" into an integer!"));
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= (sal_Int16)nValue;
                else
                    aReturn <<= (sal_Int32)nValue;
                break;
            }
            bEnumAsInt = sal_True;
            // NO BREAK! handle it as enum
        case TypeClass_ENUM:
        {
            sal_uInt16 nEnumValue(0);
        #if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
        #endif
            _rImporter.GetMM100UnitConverter().convertEnum(nEnumValue, _rReadCharacters, _pEnumMap);
            OSL_ENSURE(bSuccess, "PropertyConversion::convertString: could not convert to an enum value!");
            if (bEnumAsInt)
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= (sal_Int16)nEnumValue;
                else
                    aReturn <<= (sal_Int32)nEnumValue;
            else
                aReturn = ::cppu::int2enum((sal_Int32)nEnumValue, _rExpectedType);
        }
        break;
        case TypeClass_HYPER:
        {
            OSL_ENSURE(sal_False, "PropertyConversion::convertString: 64-bit integers not implemented yet!");
        }
        break;
        case TypeClass_DOUBLE:
        {
            double nValue;
        #if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
        #endif
            _rImporter.GetMM100UnitConverter().convertDouble(nValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    ::rtl::OString("PropertyConversion::convertString: could not convert \"")
                +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=  ::rtl::OString("\" into a double!"));
            aReturn <<= (double)nValue;
        }
        break;
        case TypeClass_STRING:
            aReturn <<= _rReadCharacters;
            break;
        case TypeClass_STRUCT:
        {
            sal_Int32 nType = 0;
            if ( _rExpectedType.equals( ::cppu::UnoType< ::com::sun::star::util::Date >::get() ) )
                nType = TYPE_DATE;
            else if ( _rExpectedType.equals( ::cppu::UnoType< ::com::sun::star::util::Time >::get() ) )
                nType = TYPE_TIME;
            else  if ( _rExpectedType.equals( ::cppu::UnoType< ::com::sun::star::util::DateTime >::get() ) )
                nType = TYPE_DATETIME;

            if ( nType )
            {
                // first extract the double
                double nValue = 0;
            #if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
            #endif
                _rImporter.GetMM100UnitConverter().convertDouble(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OString("PropertyConversion::convertString: could not convert \"")
                    +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("\" into a double!"));

                // then convert it into the target type
                switch (nType)
                {
                    case TYPE_DATE:
                    {
                        OSL_ENSURE(((sal_uInt32)nValue) - nValue == 0,
                            "PropertyConversion::convertString: a Date value with a fractional part?");
                        aReturn <<= lcl_getDate(nValue);
                    }
                    break;
                    case TYPE_TIME:
                    {
                        OSL_ENSURE(((sal_uInt32)nValue) == 0,
                            "PropertyConversion::convertString: a Time value with more than a fractional part?");
                        aReturn <<= lcl_getTime(nValue);
                    }
                    break;
                    case TYPE_DATETIME:
                    {
                        ::com::sun::star::util::Time aTime = lcl_getTime(nValue);
                        ::com::sun::star::util::Date aDate = lcl_getDate(nValue);

                        ::com::sun::star::util::DateTime aDateTime;
                        aDateTime.HundredthSeconds = aTime.HundredthSeconds;
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
                OSL_ENSURE(sal_False, "PropertyConversion::convertString: unsupported property type!");
        }
        break;
        default:
            OSL_ENSURE(sal_False, "PropertyConversion::convertString: invalid type class!");
    }

    return aReturn;
}

//---------------------------------------------------------------------
Type PropertyConversion::xmlTypeToUnoType( const ::rtl::OUString& _rType )
{
    Type aUnoType( ::getVoidCppuType() );

    DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Type, MapString2Type );
    static MapString2Type s_aTypeNameMap;
    if ( s_aTypeNameMap.empty() )
    {
        s_aTypeNameMap[ token::GetXMLToken( token::XML_BOOLEAN ) ] = ::getBooleanCppuType();
        s_aTypeNameMap[ token::GetXMLToken( token::XML_FLOAT )   ] = ::getCppuType( static_cast< double* >(NULL) );
        s_aTypeNameMap[ token::GetXMLToken( token::XML_STRING )  ] = ::getCppuType( static_cast< ::rtl::OUString* >(NULL) );
        s_aTypeNameMap[ token::GetXMLToken( token::XML_VOID )    ] = ::getVoidCppuType();
    }

    const ConstMapString2TypeIterator aTypePos = s_aTypeNameMap.find( _rType );
    OSL_ENSURE( s_aTypeNameMap.end() != aTypePos, "PropertyConversion::xmlTypeToUnoType: invalid property name!" );
    if ( s_aTypeNameMap.end() != aTypePos )
        aUnoType = aTypePos->second;

    return aUnoType;
}

//=====================================================================
//= OPropertyImport
//=====================================================================
//---------------------------------------------------------------------
OPropertyImport::OPropertyImport(OFormLayerXMLImport_Impl& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName)
    :SvXMLImportContext(_rImport.getGlobalContext(), _nPrefix, _rName)
    ,m_rContext(_rImport)
    ,m_bTrackAttributes(sal_False)
{
}

//---------------------------------------------------------------------
SvXMLImportContext* OPropertyImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
    const Reference< sax::XAttributeList >& _rxAttrList)
{
    if( token::IsXMLToken( _rLocalName, token::XML_PROPERTIES) )
    {
        return new OPropertyElementsContext( m_rContext.getGlobalContext(),
                                             _nPrefix, _rLocalName, this);
    }
    else
    {
        OSL_ENSURE(sal_False,
                ::rtl::OString("OPropertyImport::CreateChildContext: unknown sub element (only \"properties\" is recognized, but it is ")
            +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
            +=  ::rtl::OString(")!"));
        return SvXMLImportContext::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
    }
}

//---------------------------------------------------------------------
void OPropertyImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
{
    OSL_ENSURE(_rxAttrList.is(), "OPropertyImport::StartElement: invalid attribute list!");
    const sal_Int32 nAttributeCount = _rxAttrList->getLength();

    // assume the 'worst' case: all attributes describe properties. This should save our property array
    // some reallocs
    m_aValues.reserve(nAttributeCount);

    const SvXMLNamespaceMap& rMap = m_rContext.getGlobalContext().GetNamespaceMap();
    sal_uInt16 nNamespace;
    ::rtl::OUString sLocalName;
    for (sal_Int16 i=0; i<nAttributeCount; ++i)
    {
        nNamespace = rMap.GetKeyByAttrName(_rxAttrList->getNameByIndex(i), &sLocalName);
        handleAttribute(nNamespace, sLocalName, _rxAttrList->getValueByIndex(i));

        if (m_bTrackAttributes)
            m_aEncounteredAttributes.insert(sLocalName);
    }

    // TODO: create PropertyValues for all the attributes which were not present, because they were implied
    // this is necessary as soon as we have properties where the XML default is different from the property
    // default
}

//---------------------------------------------------------------------
sal_Bool OPropertyImport::encounteredAttribute(const ::rtl::OUString& _rAttributeName) const
{
    OSL_ENSURE(m_bTrackAttributes, "OPropertyImport::encounteredAttribute: attribute tracking not enabled!");
    return m_aEncounteredAttributes.end() != m_aEncounteredAttributes.find(_rAttributeName);
}

//---------------------------------------------------------------------
void OPropertyImport::Characters(const ::rtl::OUString&
#if OSL_DEBUG_LEVEL > 0
_rChars
#endif
)
{
    // ignore them (should be whitespaces only)
    OSL_ENSURE(0 == _rChars.trim().getLength(), "OPropertyImport::Characters: non-whitespace characters!");
}

//---------------------------------------------------------------------
void OPropertyImport::handleAttribute(sal_uInt16 /*_nNamespaceKey*/, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
{
    const OAttribute2Property::AttributeAssignment* pProperty = m_rContext.getAttributeMap().getAttributeTranslation(_rLocalName);
    if (pProperty)
    {
        // create and store a new PropertyValue
        PropertyValue aNewValue;
        aNewValue.Name = pProperty->sPropertyName;

        // convert the value string into the target type
        aNewValue.Value = PropertyConversion::convertString(m_rContext.getGlobalContext(), pProperty->aPropertyType, _rValue, pProperty->pEnumMap, pProperty->bInverseSemantics);
        implPushBackPropertyValue( aNewValue );
    }
#if OSL_DEBUG_LEVEL > 0
    else if (!token::IsXMLToken(_rLocalName, token::XML_TYPE))  // xlink:type is valid but ignored for <form:form>
    {
        ::rtl::OString sMessage( "OPropertyImport::handleAttribute: Can't handle the following:\n" );
        sMessage += ::rtl::OString( "  Attribute name: " );
        sMessage += ::rtl::OString( _rLocalName.getStr(), _rLocalName.getLength(), osl_getThreadTextEncoding() );
        sMessage += ::rtl::OString( "\n  value: " );
        sMessage += ::rtl::OString( _rValue.getStr(), _rValue.getLength(), osl_getThreadTextEncoding() );
        OSL_ENSURE( sal_False, sMessage.getStr() );
    }
#endif
}

//=====================================================================
//= OPropertyElementsContext
//=====================================================================
//---------------------------------------------------------------------
OPropertyElementsContext::OPropertyElementsContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
        const OPropertyImportRef& _rPropertyImporter)
    :SvXMLImportContext(_rImport, _nPrefix, _rName)
    ,m_xPropertyImporter(_rPropertyImporter)
{
}

//---------------------------------------------------------------------
SvXMLImportContext* OPropertyElementsContext::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
    const Reference< sax::XAttributeList >&)
{
    if( token::IsXMLToken( _rLocalName, token::XML_PROPERTY ) )
    {
        return new OSinglePropertyContext(GetImport(), _nPrefix, _rLocalName, m_xPropertyImporter);
    }
    else if( token::IsXMLToken( _rLocalName, token::XML_LIST_PROPERTY ) )
    {
        return new OListPropertyContext( GetImport(), _nPrefix, _rLocalName, m_xPropertyImporter );
    }
    else
    {
        OSL_ENSURE(sal_False,
                ::rtl::OString("OPropertyElementsContext::CreateChildContext: unknown child element (\"")
            +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
            +=  ::rtl::OString("\")!"));
        return new SvXMLImportContext(GetImport(), _nPrefix, _rLocalName);
    }
}

#if OSL_DEBUG_LEVEL > 0
    //---------------------------------------------------------------------
    void OPropertyElementsContext::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OSL_ENSURE(0 == _rxAttrList->getLength(), "OPropertyElementsContext::StartElement: the form:properties element should not have attributes!");
        SvXMLImportContext::StartElement(_rxAttrList);
    }

    //---------------------------------------------------------------------
    void OPropertyElementsContext::Characters(const ::rtl::OUString& _rChars)
    {
        OSL_ENSURE(0 == _rChars.trim(), "OPropertyElementsContext::Characters: non-whitespace characters detected!");
        SvXMLImportContext::Characters(_rChars);
    }

#endif

//=====================================================================
//= OSinglePropertyContext
//=====================================================================
//---------------------------------------------------------------------
OSinglePropertyContext::OSinglePropertyContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
        const OPropertyImportRef& _rPropertyImporter)
    :SvXMLImportContext(_rImport, _nPrefix, _rName)
    ,m_xPropertyImporter(_rPropertyImporter)
{
}

//---------------------------------------------------------------------
SvXMLImportContext* OSinglePropertyContext::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >&)
{
    OSL_ENSURE(sal_False,
            ::rtl::OString("OSinglePropertyContext::CreateChildContext: unknown child element (\"")
        +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
        +=  ::rtl::OString("\")!"));
    return new SvXMLImportContext(GetImport(), _nPrefix, _rLocalName);
}

//---------------------------------------------------------------------
void OSinglePropertyContext::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
{
    ::com::sun::star::beans::PropertyValue aPropValue;      // the property the instance imports currently
    ::com::sun::star::uno::Type aPropType;          // the type of the property the instance imports currently

    ::rtl::OUString sType, sValue;
    const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
    const sal_Int16 nAttrCount = _rxAttrList.is() ? _rxAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const ::rtl::OUString& rAttrName = _rxAttrList->getNameByIndex( i );

        ::rtl::OUString aLocalName;
        sal_uInt16 nPrefix =
            rMap.GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        if( XML_NAMESPACE_FORM == nPrefix )
        {
            if( token::IsXMLToken( aLocalName, token::XML_PROPERTY_NAME ) )
                aPropValue.Name = _rxAttrList->getValueByIndex( i );

        }
        else if( XML_NAMESPACE_OFFICE == nPrefix )
        {
            if( token::IsXMLToken( aLocalName, token::XML_VALUE_TYPE ) )
                sType = _rxAttrList->getValueByIndex( i );
            else if( token::IsXMLToken( aLocalName,
                                        token::XML_VALUE ) ||
                        token::IsXMLToken( aLocalName,
                                        token::XML_BOOLEAN_VALUE ) ||
                     token::IsXMLToken( aLocalName,
                                        token::XML_STRING_VALUE ) )
                sValue = _rxAttrList->getValueByIndex( i );
        }
    }

    // the name of the property
    OSL_ENSURE(aPropValue.Name.getLength(), "OSinglePropertyContext::StartElement: invalid property name!");

    // needs to be translated into a ::com::sun::star::uno::Type
    aPropType = PropertyConversion::xmlTypeToUnoType( sType );
    if( TypeClass_VOID == aPropType.getTypeClass() )
    {
        aPropValue.Value = Any();
    }
    else
    {
        aPropValue.Value =
            PropertyConversion::convertString(GetImport(), aPropType,
                                           sValue);
    }

    // now that we finally have our property value, add it to our parent object
    if( aPropValue.Name.getLength() )
        m_xPropertyImporter->implPushBackGenericPropertyValue(aPropValue);
}

//=====================================================================
//= OListPropertyContext
//=====================================================================
//---------------------------------------------------------------------
OListPropertyContext::OListPropertyContext( SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
    const OPropertyImportRef& _rPropertyImporter )
    :SvXMLImportContext( _rImport, _nPrefix, _rName )
    ,m_xPropertyImporter( _rPropertyImporter )
{
}

//---------------------------------------------------------------------
void OListPropertyContext::StartElement( const Reference< sax::XAttributeList >& _rxAttrList )
{
    sal_Int32 nAttributeCount = _rxAttrList->getLength();

    sal_uInt16 nNamespace;
    ::rtl::OUString sAttributeName;
    const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
    for ( sal_Int16 i = 0; i < nAttributeCount; ++i )
    {
        nNamespace = rMap.GetKeyByAttrName( _rxAttrList->getNameByIndex( i ), &sAttributeName );
        if  (   ( XML_NAMESPACE_FORM == nNamespace )
            &&  ( token::IsXMLToken( sAttributeName, token::XML_PROPERTY_NAME ) )
            )
        {
            m_sPropertyName = _rxAttrList->getValueByIndex( i );
        }
        else if (   ( XML_NAMESPACE_OFFICE == nNamespace )
                &&  ( token::IsXMLToken( sAttributeName, token::XML_VALUE_TYPE ) )
                )
        {
            m_sPropertyType = _rxAttrList->getValueByIndex( i );
        }
        else
        {
            OSL_ENSURE( false,
                    ::rtl::OString( "OListPropertyContext::StartElement: unknown child element (\"")
                +=  ::rtl::OString( sAttributeName.getStr(), sAttributeName.getLength(), RTL_TEXTENCODING_ASCII_US )
                +=  ::rtl::OString( "\")!" ) );
        }
    }
}

//---------------------------------------------------------------------
void OListPropertyContext::EndElement()
{
    OSL_ENSURE( m_sPropertyName.getLength() && m_sPropertyType.getLength(),
        "OListPropertyContext::EndElement: no property name or type!" );

    if ( !m_sPropertyName.getLength() || !m_sPropertyType.getLength() )
        return;

    Sequence< Any > aListElements( m_aListValues.size() );
    Any* pListElement = aListElements.getArray();
    com::sun::star::uno::Type aType = PropertyConversion::xmlTypeToUnoType( m_sPropertyType );
    for (   ::std::vector< ::rtl::OUString >::const_iterator values = m_aListValues.begin();
            values != m_aListValues.end();
            ++values, ++pListElement
        )
    {
        *pListElement = PropertyConversion::convertString( GetImport(), aType, *values );
    }

    PropertyValue aSequenceValue;
    aSequenceValue.Name = m_sPropertyName;
    aSequenceValue.Value <<= aListElements;

    m_xPropertyImporter->implPushBackGenericPropertyValue( aSequenceValue );
}

//---------------------------------------------------------------------
SvXMLImportContext* OListPropertyContext::CreateChildContext( sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName, const Reference< sax::XAttributeList >& /*_rxAttrList*/ )
{
    if ( token::IsXMLToken( _rLocalName, token::XML_LIST_VALUE ) )
    {
        m_aListValues.resize( m_aListValues.size() + 1 );
        return new OListValueContext( GetImport(), _nPrefix, _rLocalName, *m_aListValues.rbegin() );
    }
    else
    {
        OSL_ENSURE( sal_False,
                ::rtl::OString("OListPropertyContext::CreateChildContext: unknown child element (\"")
            +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
            +=  ::rtl::OString("\")!"));
        return new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );
    }
}

//=====================================================================
//= OListValueContext
//=====================================================================
//---------------------------------------------------------------------
OListValueContext::OListValueContext( SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName, ::rtl::OUString& _rListValueHolder )
    :SvXMLImportContext( _rImport, _nPrefix, _rName )
    ,m_rListValueHolder( _rListValueHolder )
{
}

//---------------------------------------------------------------------
void OListValueContext::StartElement( const Reference< sax::XAttributeList >& _rxAttrList )
{
    const sal_Int32 nAttributeCount = _rxAttrList->getLength();

    sal_uInt16 nNamespace;
    ::rtl::OUString sAttributeName;
    const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
    for ( sal_Int16 i = 0; i < nAttributeCount; ++i )
    {
        nNamespace = rMap.GetKeyByAttrName( _rxAttrList->getNameByIndex( i ), &sAttributeName );
        if ( XML_NAMESPACE_OFFICE == nNamespace )
        {
            if  (   token::IsXMLToken( sAttributeName, token::XML_VALUE )
               ||   token::IsXMLToken( sAttributeName, token::XML_STRING_VALUE )
               ||   token::IsXMLToken( sAttributeName, token::XML_BOOLEAN_VALUE )
                )
            {
                m_rListValueHolder = _rxAttrList->getValueByIndex( i );
                continue;
            }
        }

        OSL_ENSURE( false,
                ::rtl::OString( "OListValueContext::StartElement: unknown child element (\"")
            +=  ::rtl::OString( sAttributeName.getStr(), sAttributeName.getLength(), RTL_TEXTENCODING_ASCII_US )
            +=  ::rtl::OString( "\")!" ) );
    }
}

//.........................................................................
}   // namespace xmloff
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
