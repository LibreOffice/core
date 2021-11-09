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

#include "propertyexport.hxx"

#include <memory>

#include <xmloff/xmlexp.hxx>
#include "strings.hxx"
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlprmap.hxx>
#include <sax/tools/converter.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include "callbacks.hxx"
#include <unotools/datetime.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>

namespace xmloff
{

    using namespace css;
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
        css::uno::Sequence<T> anySeq;
        bool bSuccess = value >>= anySeq;
        assert(bSuccess); (void)bSuccess;
        for (T const & i : std::as_const(anySeq))
        {
            OUString sValue(implConvertAny(makeAny(i)));
            AddAttribute(XML_NAMESPACE_OFFICE, eValueAttName, sValue );
            SvXMLElementExport aValueTag(
                m_rContext.getGlobalContext(), XML_NAMESPACE_FORM,
                token::XML_LIST_VALUE, true, false);
        }
    }

    void OPropertyExport::exportRemainingProperties()
    {
        // the properties tag (will be created if we have at least one no-default property)
        std::unique_ptr<SvXMLElementExport> pPropertiesTag;

        Any aValue;
        OUString sValue;

        // loop through all the properties which are yet to be exported
        for ( const auto& rProperty : m_aRemainingProps )
        {
            DBG_CHECK_PROPERTY_NO_TYPE(rProperty);

            if ( !shouldExportProperty( rProperty ) )
                continue;

            // now that we have the first sub-tag we need the form:properties element
            if (!pPropertiesTag)
                pPropertiesTag = std::make_unique<SvXMLElementExport>(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, token::XML_PROPERTIES, true, true);

            // add the name attribute
            AddAttribute(XML_NAMESPACE_FORM, token::XML_PROPERTY_NAME, rProperty);

            // get the value
            aValue = m_xProps->getPropertyValue(rProperty);

            // the type to export
            Type aExportType;

            // is it a sequence
            bool bIsSequence = TypeClass_SEQUENCE == aValue.getValueTypeClass();
            // the type of the property, maybe reduced to the element type of a sequence
            if (bIsSequence)
                aExportType = getSequenceElementType( aValue.getValueType() );
            else
                aExportType = aValue.getValueType();

            // the type attribute

            bool bIsEmptyValue = TypeClass_VOID == aValue.getValueType().getTypeClass();
            if ( bIsEmptyValue )
            {
                css::beans::Property aPropDesc = m_xPropertyInfo->getPropertyByName( rProperty );
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
                                : token::XML_PROPERTY, true, true);

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
                    OSL_FAIL("OPropertyExport::exportRemainingProperties: unsupported sequence type !");
                    break;
            }
        }
    }

    void OPropertyExport::examinePersistence()
    {
        m_aRemainingProps.clear();
        const Sequence< Property > aProperties = m_xPropertyInfo->getProperties();
        for (const auto& rProp : aProperties)
        {
            // no transient props
            if ( rProp.Attributes & PropertyAttribute::TRANSIENT )
                continue;
            // no read-only props
            if ( ( rProp.Attributes & PropertyAttribute::READONLY ) != 0 )
                // except they're dynamically added
                if ( ( rProp.Attributes & PropertyAttribute::REMOVABLE ) == 0 )
                    continue;
            m_aRemainingProps.insert(rProp.Name);
        }
    }

    void OPropertyExport::exportStringPropertyAttribute( const sal_uInt16 _nNamespaceKey, const OUString& _pAttributeName,
            const OUString& _rPropertyName )
    {
        DBG_CHECK_PROPERTY( _rPropertyName, OUString );

        // no try-catch here, this would be too expensive. The outer scope has to handle exceptions (which should not
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

    void OPropertyExport::exportBooleanPropertyAttribute(const sal_uInt16 _nNamespaceKey, const OUString& _pAttributeName,
            const OUString& _rPropertyName, const BoolAttrFlags _nBooleanAttributeFlags)
    {
        DBG_CHECK_PROPERTY_NO_TYPE( _rPropertyName );
        // no check of the property value type: this method is allowed to be called with any integer properties
        // (e.g. sal_Int32, sal_uInt16 etc)

        bool bDefault(BoolAttrFlags::DefaultTrue & _nBooleanAttributeFlags);
        bool bDefaultVoid(BoolAttrFlags::DefaultVoid & _nBooleanAttributeFlags);

        // get the value
        bool bCurrentValue = bDefault;
        Any aCurrentValue = m_xProps->getPropertyValue( _rPropertyName );
        if (aCurrentValue.hasValue())
        {
            bCurrentValue = ::cppu::any2bool(aCurrentValue);
            // this will extract a boolean value even if the Any contains a int or short or something like that ...

            if (_nBooleanAttributeFlags & BoolAttrFlags::InverseSemantics)
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

    void OPropertyExport::exportInt16PropertyAttribute(const sal_uInt16 _nNamespaceKey, const OUString& _pAttributeName,
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
            AddAttribute(_nNamespaceKey, _pAttributeName, OUString::number(nCurrentValue));
        }

        // the property does not need to be handled anymore
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportInt32PropertyAttribute( const sal_uInt16 _nNamespaceKey, const OUString& _pAttributeName,
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
            AddAttribute( _nNamespaceKey, _pAttributeName, OUString::number(nCurrentValue) );
        }

        // the property does not need to be handled anymore
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportEnumPropertyAttributeImpl(
            const sal_uInt16 _nNamespaceKey, const OUString& _pAttributeName,
            const OUString &rPropertyName, const SvXMLEnumMapEntry<sal_uInt16>* _pValueMap,
            const sal_uInt16 _nDefault, const bool _bVoidDefault)
    {
        // get the value
        Any aValue = m_xProps->getPropertyValue(rPropertyName);

        if (aValue.hasValue())
        {   // we have a non-void current value
            sal_Int32 nCurrentValue(_nDefault);
            ::cppu::enum2int(nCurrentValue, aValue);

            // add the attribute
            if ((_nDefault != nCurrentValue) || _bVoidDefault)
            {   // the default does not equal the value, or the default is void and the value isn't

                // let the formatter of the export context build a string
                OUStringBuffer sBuffer;
                SvXMLUnitConverter::convertEnum(sBuffer, static_cast<sal_uInt16>(nCurrentValue), _pValueMap);

                AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
            }
        }
        else
        {
            if (!_bVoidDefault)
                AddAttribute(_nNamespaceKey, _pAttributeName, OUString());
        }

        // the property does not need to be handled anymore
        exportedProperty(rPropertyName);
    }

    void OPropertyExport::exportTargetFrameAttribute()
    {
        DBG_CHECK_PROPERTY( PROPERTY_TARGETFRAME, OUString );

        OUString sTargetFrame = comphelper::getString(m_xProps->getPropertyValue(PROPERTY_TARGETFRAME));
        if( sTargetFrame != "_blank" )
        {   // an empty string and "_blank" have the same meaning and don't have to be written
            AddAttribute(OAttributeMetaData::getCommonControlAttributeNamespace(CCAFlags::TargetFrame)
                        ,OAttributeMetaData::getCommonControlAttributeName(CCAFlags::TargetFrame)
                        ,sTargetFrame);
        }

        exportedProperty(PROPERTY_TARGETFRAME);
    }

    void OPropertyExport::exportRelativeTargetLocation(const OUString& _sPropertyName,CCAFlags _nProperty,bool _bAddType)
    {
        Any aAny = m_xProps->getPropertyValue(_sPropertyName);

        OUString sTargetLocation;
        if (aAny.has<uno::Reference<graphic::XGraphic>>())
        {
            auto xGraphic = aAny.get<uno::Reference<graphic::XGraphic>>();
            OUString sOutMimeType;
            sTargetLocation = m_rContext.getGlobalContext().AddEmbeddedXGraphic(xGraphic, sOutMimeType);
        }
        else if (aAny.has<OUString>())
        {
            auto sURL = aAny.get<OUString>();
            sTargetLocation = m_rContext.getGlobalContext().AddEmbeddedObject(sURL);
        }
        else
        {
            SAL_WARN("xmloff.forms", "OPropertyExport::exportRelativeTargetLocation: "
                "Value of " << _sPropertyName << " not found!");
        }

        if (!sTargetLocation.isEmpty())
        {
            AddAttribute(OAttributeMetaData::getCommonControlAttributeNamespace(_nProperty)
                        ,OAttributeMetaData::getCommonControlAttributeName(_nProperty)
                        , sTargetLocation);

            // #i110911# add xlink:type="simple" if required
            if (_bAddType)
                AddAttribute(XML_NAMESPACE_XLINK, token::XML_TYPE, token::XML_SIMPLE);

            exportedProperty(_sPropertyName);
        }
    }
    void OPropertyExport::flagStyleProperties()
    {
        // flag all the properties which are part of the style as "handled"
        rtl::Reference< XMLPropertySetMapper > xStylePropertiesSupplier = m_rContext.getStylePropertyMapper()->getPropertySetMapper();
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
            const sal_uInt16 _nAttributeNamespaceKey, const OUString& _pAttributeName, const OUString& sPropertyName)
    {
        DBG_CHECK_PROPERTY_NO_TYPE( sPropertyName );

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
                // -> don't need to write the attribute, 'cause missing it is unambiguous
                return;
        }

        // finally add the attribute to the context
        AddAttribute(_nAttributeNamespaceKey, _pAttributeName, sValue);
    }

    void OPropertyExport::exportStringSequenceAttribute(const sal_uInt16 _nAttributeNamespaceKey, const OUString& _pAttributeName,
        const OUString& _rPropertyName)
    {
        const sal_Unicode _aListSeparator = ',';
        const sal_Unicode _aQuoteCharacter = '"';
        DBG_CHECK_PROPERTY( _rPropertyName, Sequence< OUString > );

        Sequence< OUString > aItems;
        m_xProps->getPropertyValue( _rPropertyName ) >>= aItems;

        OUStringBuffer sFinalList;

        // unfortunately the OUString can't append single sal_Unicode characters ...
        const OUString sQuote(&_aQuoteCharacter, 1);
        const OUString sSeparator(&_aListSeparator, 1);
        const bool bQuote = !sQuote.isEmpty();

        // concatenate the string items
        const OUString* pItems = aItems.getConstArray();
        const OUString* pEnd = pItems + aItems.getLength();
        const OUString* pLastElement = pEnd - 1;
        for (   ;
                pItems != pEnd;
                ++pItems
            )
        {
            OSL_ENSURE(-1 == pItems->indexOf(_aQuoteCharacter),
                "OPropertyExport::exportStringSequenceAttribute: there is an item which contains the quote character!");

            if (bQuote)
                sFinalList.append(sQuote);
            sFinalList.append(*pItems);
            if (bQuote)
                sFinalList.append(sQuote);

            if (pItems != pLastElement)
                sFinalList.append(sSeparator);
        }

        if (!sFinalList.isEmpty())
            AddAttribute(_nAttributeNamespaceKey, _pAttributeName, sFinalList.makeStringAndClear());

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
            case TypeClass_UNSIGNED_SHORT:
            case TypeClass_SHORT:
            case TypeClass_LONG:
                // let the unit converter format is as string
                aBuffer.append(getINT32(_rValue));
                break;
            case TypeClass_UNSIGNED_LONG:
            case TypeClass_HYPER:
                aBuffer.append(getINT64(_rValue));
                break;
            case TypeClass_UNSIGNED_HYPER:
                aBuffer.append(static_cast<sal_Int64>(_rValue.get<sal_uInt64>()));
                break;
            case TypeClass_ENUM:
            {
                // convert it into an int32
                sal_Int32 nValue = 0;
                ::cppu::enum2int(nValue, _rValue);
                aBuffer.append(nValue);
            }
            break;
            default:
            {   // hmmm... what else do we know?
                double fValue = 0;
                css::util::Date aDate;
                css::util::Time aTime;
                css::util::DateTime aDateTime;
                if (_rValue >>= aDate)
                {
                    Date aToolsDate( Date::EMPTY );
                    ::utl::typeConvert(aDate, aToolsDate);
                    fValue = aToolsDate.GetDate();
                }
                else if (_rValue >>= aTime)
                {
                    fValue = aTime.Hours       / static_cast<double>(::tools::Time::hourPerDay) +
                             aTime.Minutes     / static_cast<double>(::tools::Time::minutePerDay) +
                             aTime.Seconds     / static_cast<double>(::tools::Time::secondPerDay) +
                             aTime.NanoSeconds / static_cast<double>(::tools::Time::nanoSecPerDay);
                }
                else if (_rValue >>= aDateTime)
                {
                    DateTime aToolsDateTime( DateTime::EMPTY );
                    ::utl::typeConvert(aDateTime, aToolsDateTime);
                    // the time part (the digits behind the comma)
                    fValue = aTime.Hours       / static_cast<double>(::tools::Time::hourPerDay) +
                             aTime.Minutes     / static_cast<double>(::tools::Time::minutePerDay) +
                             aTime.Seconds     / static_cast<double>(::tools::Time::secondPerDay) +
                             aTime.NanoSeconds / static_cast<double>(::tools::Time::nanoSecPerDay);
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

    token::XMLTokenEnum OPropertyExport::implGetPropertyXMLType(const css::uno::Type& _rType)
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
    void OPropertyExport::AddAttribute( sal_uInt16 _nPrefix, const OUString& _rName, const OUString& _rValue )
    {
        OSL_ENSURE(m_rContext.getGlobalContext().GetXAttrList()->getValueByName( _rName ).isEmpty(),
            "OPropertyExport::AddAttribute: already have such an attribute");

        m_rContext.getGlobalContext().AddAttribute( _nPrefix, _rName, _rValue );
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
                SAL_WARN("xmloff.forms", "OPropertyExport: "
                        "no property with the name " + _rPropertyName + "!");
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
            TOOLS_WARN_EXCEPTION("xmloff.forms", "could not check the property!");
        }
    }
#endif // DBG_UTIL - dbg_implCheckProperty

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
