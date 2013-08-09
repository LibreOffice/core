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

#include <stdio.h>
#include "propertyexport.hxx"
#include <xmloff/xmlexp.hxx>
#include "strings.hxx"
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#include <sax/tools/converter.hxx>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include "callbacks.hxx"
#include <unotools/datetime.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    // NO using namespace ...util !!!
    // need a tools Date/Time/DateTime below, which would conflict with the uno types then

    using namespace ::comphelper;

    //= OPropertyExport
    OPropertyExport::OPropertyExport(IFormsExportContext& _rContext, const Reference< XPropertySet >& _rxProps)
        :m_rContext(_rContext)
        ,m_xProps(_rxProps)
        ,m_xPropertyInfo( m_xProps->getPropertySetInfo() )
        ,m_xPropertyState( _rxProps, UNO_QUERY )
    {
        // caching
        OUStringBuffer aBuffer;
        ::sax::Converter::convertBool(aBuffer, true);
        m_sValueTrue = aBuffer.makeStringAndClear();
        ::sax::Converter::convertBool(aBuffer, false);
        m_sValueFalse = aBuffer.makeStringAndClear();

        OSL_ENSURE(m_xPropertyInfo.is(), "OPropertyExport::OPropertyExport: need an XPropertySetInfo!");

        // collect the properties which need to be exported
        examinePersistence();
    }

    bool OPropertyExport::shouldExportProperty( const OUString& i_propertyName ) const
    {
        // if the property state is DEFAULT, it does not need to be written - at least
        // if it's a built-in property, and not a dynamically-added one.
        bool bIsDefaultValue =    m_xPropertyState.is()
                            &&  ( PropertyState_DEFAULT_VALUE == m_xPropertyState->getPropertyState( i_propertyName ) );
        bool bIsDynamicProperty =  m_xPropertyInfo.is()
                                && ( ( m_xPropertyInfo->getPropertyByName( i_propertyName ).Attributes & PropertyAttribute::REMOVABLE ) != 0 );
        return ( !bIsDefaultValue || bIsDynamicProperty );
    }

    template< typename T > void
    OPropertyExport::exportRemainingPropertiesSequence(
        Any const & value, token::XMLTokenEnum eValueAttName)
    {
        OSequenceIterator< T > i(value);
        while (i.hasMoreElements())
        {
            OUString sValue(implConvertAny(i.nextElement()));
            AddAttribute(XML_NAMESPACE_OFFICE, eValueAttName, sValue );
            SvXMLElementExport aValueTag(
                m_rContext.getGlobalContext(), XML_NAMESPACE_FORM,
                token::XML_LIST_VALUE, sal_True, sal_False);
        }
    }

    void OPropertyExport::exportRemainingProperties()
    {
        // the properties tag (will be created if we have at least one no-default property)
        SvXMLElementExport* pPropertiesTag = NULL;

        try
        {
            Any aValue;
            OUString sValue;

            // loop through all the properties which are yet to be exported
            for (   ConstStringSetIterator  aProperty = m_aRemainingProps.begin();
                    aProperty != m_aRemainingProps.end();
                    ++aProperty
                )
            {
                DBG_CHECK_PROPERTY_NO_TYPE(*aProperty);

    #if OSL_DEBUG_LEVEL > 0
                const OUString sPropertyName = *aProperty; (void)sPropertyName;
    #endif
                if ( !shouldExportProperty( *aProperty ) )
                    continue;

                // now that we have the first sub-tag we need the form:properties element
                if (!pPropertiesTag)
                    pPropertiesTag = new SvXMLElementExport(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, token::XML_PROPERTIES, sal_True, sal_True);

                // add the name attribute
                AddAttribute(XML_NAMESPACE_FORM, token::XML_PROPERTY_NAME, *aProperty);

                // get the value
                aValue = m_xProps->getPropertyValue(*aProperty);

                // the type to export
                Type aExportType;

                // is it a sequence
                sal_Bool bIsSequence = TypeClass_SEQUENCE == aValue.getValueTypeClass();
                // the type of the property, maybe reduced to the element type of a sequence
                if (bIsSequence)
                    aExportType = getSequenceElementType( aValue.getValueType() );
                else
                    aExportType = aValue.getValueType();

                // the type attribute

                bool bIsEmptyValue = TypeClass_VOID == aValue.getValueType().getTypeClass();
                if ( bIsEmptyValue )
                {
                    com::sun::star::beans::Property aPropDesc;
                    aPropDesc = m_xPropertyInfo->getPropertyByName( *aProperty );
                    aExportType = aPropDesc.Type;
                }
                token::XMLTokenEnum eValueType = implGetPropertyXMLType( aExportType );

                if ( bIsEmptyValue )
                    AddAttribute( XML_NAMESPACE_OFFICE, token::XML_VALUE_TYPE, token::XML_VOID );
                else
                    AddAttribute( XML_NAMESPACE_OFFICE, token::XML_VALUE_TYPE, eValueType );

                token::XMLTokenEnum eValueAttName( token::XML_VALUE );
                switch ( eValueType )
                {
                case token::XML_BOOLEAN:    eValueAttName = token::XML_BOOLEAN_VALUE; break;
                case token::XML_STRING:     eValueAttName = token::XML_STRING_VALUE;  break;
                default:    break;
                }

                if( !bIsSequence && !bIsEmptyValue )
                {   // the simple case

                    sValue = implConvertAny(aValue);
                    AddAttribute(XML_NAMESPACE_OFFICE, eValueAttName, sValue );
                }

                // start the property tag
                SvXMLElementExport aValueTag1(m_rContext.getGlobalContext(),
                        XML_NAMESPACE_FORM,
                        bIsSequence ? token::XML_LIST_PROPERTY
                                    : token::XML_PROPERTY, sal_True, sal_True);

                if (!bIsSequence)
                    continue;

                // the not-that-simple case, we need to iterate through the sequence elements
                switch ( aExportType.getTypeClass() )
                {
                    case TypeClass_STRING:
                        exportRemainingPropertiesSequence< OUString >(
                            aValue, eValueAttName);
                        break;
                    case TypeClass_DOUBLE:
                        exportRemainingPropertiesSequence< double >(
                            aValue, eValueAttName);
                        break;
                    case TypeClass_BOOLEAN:
                        exportRemainingPropertiesSequence< sal_Bool >(
                            aValue, eValueAttName);
                        break;
                    case TypeClass_BYTE:
                        exportRemainingPropertiesSequence< sal_Int8 >(
                            aValue, eValueAttName);
                        break;
                    case TypeClass_SHORT:
                        exportRemainingPropertiesSequence< sal_Int16 >(
                            aValue, eValueAttName);
                        break;
                    case TypeClass_LONG:
                        exportRemainingPropertiesSequence< sal_Int32 >(
                            aValue, eValueAttName);
                        break;
                    case TypeClass_HYPER:
                        exportRemainingPropertiesSequence< sal_Int64 >(
                            aValue, eValueAttName);
                        break;
                    default:
                        OSL_FAIL("OPropertyExport::exportRemainingProperties: unsupported sequence tyoe !");
                        break;
                }
            }
        }
        catch(...)
        {
            delete pPropertiesTag;
            throw;
        }
        delete pPropertiesTag;
    }

    void OPropertyExport::examinePersistence()
    {
        m_aRemainingProps.clear();
        Sequence< Property > aProperties = m_xPropertyInfo->getProperties();
        const Property* pProperties = aProperties.getConstArray();
        for (sal_Int32 i=0; i<aProperties.getLength(); ++i, ++pProperties)
        {
            // no transient props
            if ( pProperties->Attributes & PropertyAttribute::TRANSIENT )
                continue;
            // no read-only props
            if ( ( pProperties->Attributes & PropertyAttribute::READONLY ) != 0 )
                // except they're dynamically added
                if ( ( pProperties->Attributes & PropertyAttribute::REMOVABLE ) == 0 )
                    continue;
            m_aRemainingProps.insert(pProperties->Name);
        }
    }

    void OPropertyExport::exportStringPropertyAttribute( const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const OUString& _rPropertyName )
    {
        DBG_CHECK_PROPERTY( _rPropertyName, OUString );

        // no try-catch here, this would be to expensive. The outer scope has to handle exceptions (which should not
        // happen if we're used correctly :)

        // this is way simple, as we don't need to convert anything (the property already is a string)

        // get the string
        OUString sPropValue;
        m_xProps->getPropertyValue( _rPropertyName ) >>= sPropValue;

        // add the attribute
        if ( !sPropValue.isEmpty() )
            AddAttribute( _nNamespaceKey, _pAttributeName, sPropValue );

        // the property does not need to be handled anymore
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportBooleanPropertyAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const OUString& _rPropertyName, const sal_Int8 _nBooleanAttributeFlags)
    {
        DBG_CHECK_PROPERTY_NO_TYPE( _rPropertyName );
        // no check of the property value type: this method is allowed to be called with any interger properties
        // (e.g. sal_Int32, sal_uInt16 etc)

        sal_Bool bDefault = (BOOLATTR_DEFAULT_TRUE == (BOOLATTR_DEFAULT_MASK & _nBooleanAttributeFlags));
        sal_Bool bDefaultVoid = (BOOLATTR_DEFAULT_VOID == (BOOLATTR_DEFAULT_MASK & _nBooleanAttributeFlags));

        // get the value
        sal_Bool bCurrentValue = bDefault;
        Any aCurrentValue = m_xProps->getPropertyValue( _rPropertyName );
        if (aCurrentValue.hasValue())
        {
            bCurrentValue = ::cppu::any2bool(aCurrentValue);
            // this will extract a boolean value even if the Any contains a int or short or something like that ...

            if (_nBooleanAttributeFlags & BOOLATTR_INVERSE_SEMANTICS)
                bCurrentValue = !bCurrentValue;

            // we have a non-void current value
            if (bDefaultVoid || (bDefault != bCurrentValue))
                // and (the default is void, or the non-void default does not equal the current value)
                // -> write the attribute
                AddAttribute(_nNamespaceKey, _pAttributeName, bCurrentValue ? m_sValueTrue : m_sValueFalse);
        }
        else
            // we have a void current value
            if (!bDefaultVoid)
                // and we have a non-void default
                // -> write the attribute
                AddAttribute(_nNamespaceKey, _pAttributeName, bCurrentValue ? m_sValueTrue : m_sValueFalse);

        // the property does not need to be handled anymore
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportInt16PropertyAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
        const OUString& _rPropertyName, const sal_Int16 _nDefault, bool force)
    {
        DBG_CHECK_PROPERTY( _rPropertyName, sal_Int16 );

        // get the value
        sal_Int16 nCurrentValue(_nDefault);
        m_xProps->getPropertyValue( _rPropertyName ) >>= nCurrentValue;

        // add the attribute
        if (force || _nDefault != nCurrentValue)
        {
            // let the formatter of the export context build a string
            OUStringBuffer sBuffer;
            ::sax::Converter::convertNumber(sBuffer, (sal_Int32)nCurrentValue);

            AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
        }

        // the property does not need to be handled anymore
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportInt32PropertyAttribute( const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
        const OUString& _rPropertyName, const sal_Int32 _nDefault )
    {
        DBG_CHECK_PROPERTY( _rPropertyName, sal_Int32 );

        // get the value
        sal_Int32 nCurrentValue( _nDefault );
        m_xProps->getPropertyValue( _rPropertyName ) >>= nCurrentValue;

        // add the attribute
        if ( _nDefault != nCurrentValue )
        {
            // let the formatter of the export context build a string
            OUStringBuffer sBuffer;
            ::sax::Converter::convertNumber( sBuffer, nCurrentValue );

            AddAttribute( _nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear() );
        }

        // the property does not need to be handled anymore
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportEnumPropertyAttribute(
            const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const OUString &rPropertyName, const SvXMLEnumMapEntry* _pValueMap,
            const sal_Int32 _nDefault, const sal_Bool _bVoidDefault)
    {
        // get the value
        sal_Int32 nCurrentValue(_nDefault);
        Any aValue = m_xProps->getPropertyValue(rPropertyName);

        if (aValue.hasValue())
        {   // we have a non-void current value
            ::cppu::enum2int(nCurrentValue, aValue);

            // add the attribute
            if ((_nDefault != nCurrentValue) || _bVoidDefault)
            {   // the default does not equal the value, or the default is void and the value isn't

                // let the formatter of the export context build a string
                OUStringBuffer sBuffer;
                m_rContext.getGlobalContext().GetMM100UnitConverter().convertEnum(sBuffer, (sal_uInt16)nCurrentValue, _pValueMap);

                AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
            }
        }
        else
        {
            if (!_bVoidDefault)
                AddAttributeASCII(_nNamespaceKey, _pAttributeName, "");
        }

        // the property does not need to be handled anymore
        exportedProperty(rPropertyName);
    }

    void OPropertyExport::exportTargetFrameAttribute()
    {
        DBG_CHECK_PROPERTY( PROPERTY_TARGETFRAME, OUString );

        OUString sTargetFrame = comphelper::getString(m_xProps->getPropertyValue(PROPERTY_TARGETFRAME));
        if (0 != sTargetFrame.compareToAscii("_blank"))
        {   // an empty string and "_blank" have the same meaning and don't have to be written
            AddAttribute(OAttributeMetaData::getCommonControlAttributeNamespace(CCA_TARGET_FRAME)
                        ,OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_FRAME)
                        ,sTargetFrame);
        }

        exportedProperty(PROPERTY_TARGETFRAME);
    }

    void OPropertyExport::exportRelativeTargetLocation(const OUString& _sPropertyName,sal_Int32 _nProperty,bool _bAddType)
    {
        DBG_CHECK_PROPERTY( _sPropertyName, OUString );

        OUString sTargetLocation = comphelper::getString(m_xProps->getPropertyValue(_sPropertyName));
        if ( !sTargetLocation.isEmpty() )
                    // If this isn't a GraphicObject then GetRelativeReference
                    // will be called anyway ( in AddEmbeddedGraphic )
            sTargetLocation = m_rContext.getGlobalContext().AddEmbeddedGraphicObject(sTargetLocation);
        AddAttribute(OAttributeMetaData::getCommonControlAttributeNamespace(_nProperty)
                    ,OAttributeMetaData::getCommonControlAttributeName(_nProperty)
                    , sTargetLocation);

        // #i110911# add xlink:type="simple" if required
        if (_bAddType)
            AddAttribute(XML_NAMESPACE_XLINK, token::XML_TYPE, token::XML_SIMPLE);

        exportedProperty(_sPropertyName);
    }
    void OPropertyExport::flagStyleProperties()
    {
        // flag all the properties which are part of the style as "handled"
        UniReference< XMLPropertySetMapper > xStylePropertiesSupplier = m_rContext.getStylePropertyMapper()->getPropertySetMapper();
        for (sal_Int32 i=0; i<xStylePropertiesSupplier->GetEntryCount(); ++i)
            exportedProperty(xStylePropertiesSupplier->GetEntryAPIName(i));

        // the font properties are exported as single properties, but there is a FontDescriptor property which
        // collects them all-in-one, this has been exported implicitly
        exportedProperty(PROPERTY_FONT);

        // for the DateFormat and TimeFormat, there exist wrapper properties which has been exported as
        // style, too
        exportedProperty(PROPERTY_DATEFORMAT);
        exportedProperty(PROPERTY_TIMEFORMAT);

        // the following properties should have been exported at the shape already:
        exportedProperty( "VerticalAlign" );
        exportedProperty( "WritingMode" );
        exportedProperty( "ScaleMode" );
        // ditto the TextWritingMode
        exportedProperty( "WritingMode" );
    }

    void OPropertyExport::exportGenericPropertyAttribute(
            const sal_uInt16 _nAttributeNamespaceKey, const sal_Char* _pAttributeName, const sal_Char* _pPropertyName)
    {
        DBG_CHECK_PROPERTY_ASCII_NO_TYPE( _pPropertyName );

        OUString sPropertyName = OUString::createFromAscii(_pPropertyName);
        exportedProperty(sPropertyName);

        Any aCurrentValue = m_xProps->getPropertyValue(sPropertyName);
        if (!aCurrentValue.hasValue())
            // nothing to do without a concrete value
            return;

        OUString sValue = implConvertAny(aCurrentValue);
        if (sValue.isEmpty() && (TypeClass_STRING == aCurrentValue.getValueTypeClass()))
        {
            // check whether or not the property is allowed to be VOID
            Property aProperty = m_xPropertyInfo->getPropertyByName(sPropertyName);
            if ((aProperty.Attributes & PropertyAttribute::MAYBEVOID) == 0)
                // the string is empty, and the property is not allowed to be void
                // -> don't need to write the attibute, 'cause missing it is unambiguous
                return;
        }

        // finally add the attribuite to the context
        AddAttribute(_nAttributeNamespaceKey, _pAttributeName, sValue);
    }

    void OPropertyExport::exportStringSequenceAttribute(const sal_uInt16 _nAttributeNamespaceKey, const sal_Char* _pAttributeName,
        const OUString& _rPropertyName,
        const sal_Unicode _aQuoteCharacter, const sal_Unicode _aListSeparator)
    {
        DBG_CHECK_PROPERTY( _rPropertyName, Sequence< OUString > );
        OSL_ENSURE(_aListSeparator != 0, "OPropertyExport::exportStringSequenceAttribute: invalid separator character!");

        Sequence< OUString > aItems;
        m_xProps->getPropertyValue( _rPropertyName ) >>= aItems;

        OUString sFinalList;

        // unfortunately the OUString can't append single sal_Unicode characters ...
        const OUString sQuote(&_aQuoteCharacter, 1);
        const OUString sSeparator(&_aListSeparator, 1);
        const sal_Bool bQuote = !sQuote.isEmpty();

        // concatenate the string items
        const OUString* pItems = aItems.getConstArray();
        const OUString* pEnd = pItems + aItems.getLength();
        const OUString* pLastElement = pEnd - 1;
        for (   ;
                pItems != pEnd;
                ++pItems
            )
        {
            OSL_ENSURE(!_aQuoteCharacter || (-1 == pItems->indexOf(_aQuoteCharacter)),
                "OPropertyExport::exportStringSequenceAttribute: there is an item which contains the quote character!");
            OSL_ENSURE(_aQuoteCharacter || (-1 == pItems->indexOf(_aListSeparator)),
                "OPropertyExport::exportStringSequenceAttribute: no quote character, but there is an item containing the separator character!");

            if (bQuote)
                sFinalList += sQuote;
            sFinalList += *pItems;
            if (bQuote)
                sFinalList += sQuote;

            if (pItems != pLastElement)
                sFinalList += sSeparator;
        }

        if (!sFinalList.isEmpty())
            AddAttribute(_nAttributeNamespaceKey, _pAttributeName, sFinalList);

        exportedProperty( _rPropertyName );
    }

    OUString OPropertyExport::implConvertAny(const Any& _rValue)
    {
        OUStringBuffer aBuffer;
        switch (_rValue.getValueTypeClass())
        {
            case TypeClass_STRING:
            {   // extract the string
                OUString sCurrentValue;
                _rValue >>= sCurrentValue;
                aBuffer.append(sCurrentValue);
            }
            break;
            case TypeClass_DOUBLE:
                // let the unit converter format is as string
                ::sax::Converter::convertDouble(aBuffer, getDouble(_rValue));
                break;
            case TypeClass_BOOLEAN:
                aBuffer = getBOOL(_rValue) ? m_sValueTrue : m_sValueFalse;
                break;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
            case TypeClass_LONG:
                // let the unit converter format is as string
                ::sax::Converter::convertNumber(aBuffer, getINT32(_rValue));
                break;
            case TypeClass_HYPER:
                // TODO
                OSL_FAIL("OPropertyExport::implConvertAny: missing implementation for sal_Int64!");
                break;
            case TypeClass_ENUM:
            {
                // convert it into an int32
                sal_Int32 nValue = 0;
                ::cppu::enum2int(nValue, _rValue);
                ::sax::Converter::convertNumber(aBuffer, nValue);
            }
            break;
            default:
            {   // hmmm .... what else do we know?
                double fValue = 0;
                ::com::sun::star::util::Date aDate;
                ::com::sun::star::util::Time aTime;
                ::com::sun::star::util::DateTime aDateTime;
                if (_rValue >>= aDate)
                {
                    Date aToolsDate( Date::EMPTY );
                    ::utl::typeConvert(aDate, aToolsDate);
                    fValue = aToolsDate.GetDate();
                }
                else if (_rValue >>= aTime)
                {
                    fValue = aTime.Hours       / static_cast<double>(::Time::hourPerDay) +
                             aTime.Minutes     / static_cast<double>(::Time::minutePerDay) +
                             aTime.Seconds     / static_cast<double>(::Time::secondPerDay) +
                             aTime.NanoSeconds / static_cast<double>(::Time::nanoSecPerDay);
                }
                else if (_rValue >>= aDateTime)
                {
                    DateTime aToolsDateTime( DateTime::EMPTY );
                    ::utl::typeConvert(aDateTime, aToolsDateTime);
                    // the time part (the digits behind the comma)
                    fValue = aTime.Hours       / static_cast<double>(::Time::hourPerDay) +
                             aTime.Minutes     / static_cast<double>(::Time::minutePerDay) +
                             aTime.Seconds     / static_cast<double>(::Time::secondPerDay) +
                             aTime.NanoSeconds / static_cast<double>(::Time::nanoSecPerDay);
                    // plus the data part (the digits in front of the comma)
                    fValue += aToolsDateTime.GetDate();
                }
                else
                {
                    // if any other types are added here, please remember to adjust implGetPropertyXMLType accordingly

                    // no more options ...
                    OSL_FAIL("OPropertyExport::implConvertAny: unsupported value type!");
                    break;
                }
                // let the unit converter format is as string
                ::sax::Converter::convertDouble(aBuffer, fValue);
            }
            break;
        }

        return aBuffer.makeStringAndClear();
    }

    token::XMLTokenEnum OPropertyExport::implGetPropertyXMLType(const ::com::sun::star::uno::Type& _rType)
    {
        // handle the type description
        switch (_rType.getTypeClass())
        {
            case TypeClass_STRING:
                return token::XML_STRING;
            case TypeClass_DOUBLE:
            case TypeClass_BYTE:
            case TypeClass_SHORT:
            case TypeClass_LONG:
            case TypeClass_HYPER:
            case TypeClass_ENUM:
                return token::XML_FLOAT;
            case TypeClass_BOOLEAN:
                return token::XML_BOOLEAN;

            default:
                return token::XML_FLOAT;
        }
    }

#ifdef DBG_UTIL
    void OPropertyExport::AddAttribute(sal_uInt16 _nPrefix, const sal_Char* _pName, const OUString& _rValue)
    {
        OSL_ENSURE(m_rContext.getGlobalContext().GetXAttrList()->getValueByName(OUString::createFromAscii(_pName)).isEmpty(),
            "OPropertyExport::AddAttribute: already have such an attribute");

        m_rContext.getGlobalContext().AddAttribute(_nPrefix, _pName, _rValue);
    }

    void OPropertyExport::AddAttribute( sal_uInt16 _nPrefix, const OUString& _rName, const OUString& _rValue )
    {
        OSL_ENSURE(m_rContext.getGlobalContext().GetXAttrList()->getValueByName( _rName ).isEmpty(),
            "OPropertyExport::AddAttribute: already have such an attribute");

        m_rContext.getGlobalContext().AddAttribute( _nPrefix, _rName, _rValue );
    }

    void OPropertyExport::AddAttributeASCII(sal_uInt16 _nPrefix, const sal_Char* _pName, const sal_Char *pValue)
    {
        OSL_ENSURE(m_rContext.getGlobalContext().GetXAttrList()->getValueByName(OUString::createFromAscii(_pName)).isEmpty(),
            "OPropertyExport::AddAttributeASCII: already have such an attribute");

        m_rContext.getGlobalContext().AddAttributeASCII(_nPrefix, _pName, pValue);
    }

    void OPropertyExport::AddAttribute(sal_uInt16 _nPrefix, ::xmloff::token::XMLTokenEnum _eName, const OUString& _rValue)
    {
        OSL_ENSURE(m_rContext.getGlobalContext().GetXAttrList()->getValueByName(::xmloff::token::GetXMLToken(_eName)).isEmpty(),
            "OPropertyExport::AddAttribute: already have such an attribute");

        m_rContext.getGlobalContext().AddAttribute(_nPrefix, _eName, _rValue);
    }

    void OPropertyExport::AddAttribute(sal_uInt16 _nPrefix, ::xmloff::token::XMLTokenEnum _eName, ::xmloff::token::XMLTokenEnum _eValue )
    {
        OSL_ENSURE(m_rContext.getGlobalContext().GetXAttrList()->getValueByName(::xmloff::token::GetXMLToken(_eName)).isEmpty(),
            "OPropertyExport::AddAttribute: already have such an attribute");

        m_rContext.getGlobalContext().AddAttribute(_nPrefix, _eName, _eValue);
    }

    void OPropertyExport::dbg_implCheckProperty(const OUString& _rPropertyName, const Type* _pType)
    {
        try
        {
            // the property must exist
            if (!m_xPropertyInfo->hasPropertyByName(_rPropertyName))
            {
                OStringBuffer aBuf("OPropertyExport::dbg_implCheckProperty: no property with the name ");
                aBuf.append(OUStringToOString(_rPropertyName, RTL_TEXTENCODING_ASCII_US)).append('!');
                OSL_FAIL(aBuf.getStr());
                return;
            }

            if (_pType)
            {
                // and it must have the correct type
                Property aPropertyDescription = m_xPropertyInfo->getPropertyByName(_rPropertyName);
                OSL_ENSURE(aPropertyDescription.Type.equals(*_pType), "OPropertyExport::dbg_implCheckProperty: invalid property type!");
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("OPropertyExport::dbg_implCheckProperty: caught an exception, could not check the property!");
        }
    }
#endif // DBG_UTIL - dbg_implCheckProperty

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
