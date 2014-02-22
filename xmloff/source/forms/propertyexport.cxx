/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <stdio.h>
#include "propertyexport.hxx"
#include <xmloff/xmlexp.hxx>
#include "strings.hxx"
#include <xmloff/xmlnmspe.hxx>
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

    
    

    using namespace ::comphelper;

    
    OPropertyExport::OPropertyExport(IFormsExportContext& _rContext, const Reference< XPropertySet >& _rxProps)
        :m_rContext(_rContext)
        ,m_xProps(_rxProps)
        ,m_xPropertyInfo( m_xProps->getPropertySetInfo() )
        ,m_xPropertyState( _rxProps, UNO_QUERY )
    {
        
        OUStringBuffer aBuffer;
        ::sax::Converter::convertBool(aBuffer, true);
        m_sValueTrue = aBuffer.makeStringAndClear();
        ::sax::Converter::convertBool(aBuffer, false);
        m_sValueFalse = aBuffer.makeStringAndClear();

        OSL_ENSURE(m_xPropertyInfo.is(), "OPropertyExport::OPropertyExport: need an XPropertySetInfo!");

        
        examinePersistence();
    }

    bool OPropertyExport::shouldExportProperty( const OUString& i_propertyName ) const
    {
        
        
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
        
        SvXMLElementExport* pPropertiesTag = NULL;

        try
        {
            Any aValue;
            OUString sValue;

            
            for (   StringSet::const_iterator aProperty = m_aRemainingProps.begin();
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

                
                if (!pPropertiesTag)
                    pPropertiesTag = new SvXMLElementExport(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, token::XML_PROPERTIES, sal_True, sal_True);

                
                AddAttribute(XML_NAMESPACE_FORM, token::XML_PROPERTY_NAME, *aProperty);

                
                aValue = m_xProps->getPropertyValue(*aProperty);

                
                Type aExportType;

                
                sal_Bool bIsSequence = TypeClass_SEQUENCE == aValue.getValueTypeClass();
                
                if (bIsSequence)
                    aExportType = getSequenceElementType( aValue.getValueType() );
                else
                    aExportType = aValue.getValueType();

                

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
                {   

                    sValue = implConvertAny(aValue);
                    AddAttribute(XML_NAMESPACE_OFFICE, eValueAttName, sValue );
                }

                
                SvXMLElementExport aValueTag1(m_rContext.getGlobalContext(),
                        XML_NAMESPACE_FORM,
                        bIsSequence ? token::XML_LIST_PROPERTY
                                    : token::XML_PROPERTY, sal_True, sal_True);

                if (!bIsSequence)
                    continue;

                
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
            
            if ( pProperties->Attributes & PropertyAttribute::TRANSIENT )
                continue;
            
            if ( ( pProperties->Attributes & PropertyAttribute::READONLY ) != 0 )
                
                if ( ( pProperties->Attributes & PropertyAttribute::REMOVABLE ) == 0 )
                    continue;
            m_aRemainingProps.insert(pProperties->Name);
        }
    }

    void OPropertyExport::exportStringPropertyAttribute( const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const OUString& _rPropertyName )
    {
        DBG_CHECK_PROPERTY( _rPropertyName, OUString );

        
        

        

        
        OUString sPropValue;
        m_xProps->getPropertyValue( _rPropertyName ) >>= sPropValue;

        
        if ( !sPropValue.isEmpty() )
            AddAttribute( _nNamespaceKey, _pAttributeName, sPropValue );

        
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportBooleanPropertyAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const OUString& _rPropertyName, const sal_Int8 _nBooleanAttributeFlags)
    {
        DBG_CHECK_PROPERTY_NO_TYPE( _rPropertyName );
        
        

        sal_Bool bDefault = (BOOLATTR_DEFAULT_TRUE == (BOOLATTR_DEFAULT_MASK & _nBooleanAttributeFlags));
        sal_Bool bDefaultVoid = (BOOLATTR_DEFAULT_VOID == (BOOLATTR_DEFAULT_MASK & _nBooleanAttributeFlags));

        
        sal_Bool bCurrentValue = bDefault;
        Any aCurrentValue = m_xProps->getPropertyValue( _rPropertyName );
        if (aCurrentValue.hasValue())
        {
            bCurrentValue = ::cppu::any2bool(aCurrentValue);
            

            if (_nBooleanAttributeFlags & BOOLATTR_INVERSE_SEMANTICS)
                bCurrentValue = !bCurrentValue;

            
            if (bDefaultVoid || (bDefault != bCurrentValue))
                
                
                AddAttribute(_nNamespaceKey, _pAttributeName, bCurrentValue ? m_sValueTrue : m_sValueFalse);
        }
        else
            
            if (!bDefaultVoid)
                
                
                AddAttribute(_nNamespaceKey, _pAttributeName, bCurrentValue ? m_sValueTrue : m_sValueFalse);

        
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportInt16PropertyAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
        const OUString& _rPropertyName, const sal_Int16 _nDefault, bool force)
    {
        DBG_CHECK_PROPERTY( _rPropertyName, sal_Int16 );

        
        sal_Int16 nCurrentValue(_nDefault);
        m_xProps->getPropertyValue( _rPropertyName ) >>= nCurrentValue;

        
        if (force || _nDefault != nCurrentValue)
        {
            
            OUStringBuffer sBuffer;
            ::sax::Converter::convertNumber(sBuffer, (sal_Int32)nCurrentValue);

            AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
        }

        
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportInt32PropertyAttribute( const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
        const OUString& _rPropertyName, const sal_Int32 _nDefault )
    {
        DBG_CHECK_PROPERTY( _rPropertyName, sal_Int32 );

        
        sal_Int32 nCurrentValue( _nDefault );
        m_xProps->getPropertyValue( _rPropertyName ) >>= nCurrentValue;

        
        if ( _nDefault != nCurrentValue )
        {
            
            OUStringBuffer sBuffer;
            ::sax::Converter::convertNumber( sBuffer, nCurrentValue );

            AddAttribute( _nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear() );
        }

        
        exportedProperty( _rPropertyName );
    }

    void OPropertyExport::exportEnumPropertyAttribute(
            const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const OUString &rPropertyName, const SvXMLEnumMapEntry* _pValueMap,
            const sal_Int32 _nDefault, const sal_Bool _bVoidDefault)
    {
        
        sal_Int32 nCurrentValue(_nDefault);
        Any aValue = m_xProps->getPropertyValue(rPropertyName);

        if (aValue.hasValue())
        {   
            ::cppu::enum2int(nCurrentValue, aValue);

            
            if ((_nDefault != nCurrentValue) || _bVoidDefault)
            {   

                
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

        
        exportedProperty(rPropertyName);
    }

    void OPropertyExport::exportTargetFrameAttribute()
    {
        DBG_CHECK_PROPERTY( PROPERTY_TARGETFRAME, OUString );

        OUString sTargetFrame = comphelper::getString(m_xProps->getPropertyValue(PROPERTY_TARGETFRAME));
        if( ! sTargetFrame.equalsAscii("_blank") )
        {   
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
                    
                    
            sTargetLocation = m_rContext.getGlobalContext().AddEmbeddedGraphicObject(sTargetLocation);
        AddAttribute(OAttributeMetaData::getCommonControlAttributeNamespace(_nProperty)
                    ,OAttributeMetaData::getCommonControlAttributeName(_nProperty)
                    , sTargetLocation);

        
        if (_bAddType)
            AddAttribute(XML_NAMESPACE_XLINK, token::XML_TYPE, token::XML_SIMPLE);

        exportedProperty(_sPropertyName);
    }
    void OPropertyExport::flagStyleProperties()
    {
        
        UniReference< XMLPropertySetMapper > xStylePropertiesSupplier = m_rContext.getStylePropertyMapper()->getPropertySetMapper();
        for (sal_Int32 i=0; i<xStylePropertiesSupplier->GetEntryCount(); ++i)
            exportedProperty(xStylePropertiesSupplier->GetEntryAPIName(i));

        
        
        exportedProperty(PROPERTY_FONT);

        
        
        exportedProperty(PROPERTY_DATEFORMAT);
        exportedProperty(PROPERTY_TIMEFORMAT);

        
        exportedProperty( "VerticalAlign" );
        exportedProperty( "WritingMode" );
        exportedProperty( "ScaleMode" );
        
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
            
            return;

        OUString sValue = implConvertAny(aCurrentValue);
        if (sValue.isEmpty() && (TypeClass_STRING == aCurrentValue.getValueTypeClass()))
        {
            
            Property aProperty = m_xPropertyInfo->getPropertyByName(sPropertyName);
            if ((aProperty.Attributes & PropertyAttribute::MAYBEVOID) == 0)
                
                
                return;
        }

        
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

        
        const OUString sQuote(&_aQuoteCharacter, 1);
        const OUString sSeparator(&_aListSeparator, 1);
        const sal_Bool bQuote = !sQuote.isEmpty();

        
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
            {   
                OUString sCurrentValue;
                _rValue >>= sCurrentValue;
                aBuffer.append(sCurrentValue);
            }
            break;
            case TypeClass_DOUBLE:
                
                ::sax::Converter::convertDouble(aBuffer, getDouble(_rValue));
                break;
            case TypeClass_BOOLEAN:
                aBuffer = getBOOL(_rValue) ? m_sValueTrue : m_sValueFalse;
                break;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
            case TypeClass_LONG:
                
                ::sax::Converter::convertNumber(aBuffer, getINT32(_rValue));
                break;
            case TypeClass_HYPER:
                
                OSL_FAIL("OPropertyExport::implConvertAny: missing implementation for sal_Int64!");
                break;
            case TypeClass_ENUM:
            {
                
                sal_Int32 nValue = 0;
                ::cppu::enum2int(nValue, _rValue);
                ::sax::Converter::convertNumber(aBuffer, nValue);
            }
            break;
            default:
            {   
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
                    
                    fValue = aTime.Hours       / static_cast<double>(::Time::hourPerDay) +
                             aTime.Minutes     / static_cast<double>(::Time::minutePerDay) +
                             aTime.Seconds     / static_cast<double>(::Time::secondPerDay) +
                             aTime.NanoSeconds / static_cast<double>(::Time::nanoSecPerDay);
                    
                    fValue += aToolsDateTime.GetDate();
                }
                else
                {
                    

                    
                    OSL_FAIL("OPropertyExport::implConvertAny: unsupported value type!");
                    break;
                }
                
                ::sax::Converter::convertDouble(aBuffer, fValue);
            }
            break;
        }

        return aBuffer.makeStringAndClear();
    }

    token::XMLTokenEnum OPropertyExport::implGetPropertyXMLType(const ::com::sun::star::uno::Type& _rType)
    {
        
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
            
            if (!m_xPropertyInfo->hasPropertyByName(_rPropertyName))
            {
                SAL_WARN("xmloff.forms", "OPropertyExport: "
                        "no property with the name " + _rPropertyName + "!");
                return;
            }

            if (_pType)
            {
                
                Property aPropertyDescription = m_xPropertyInfo->getPropertyByName(_rPropertyName);
                OSL_ENSURE(aPropertyDescription.Type.equals(*_pType), "OPropertyExport::dbg_implCheckProperty: invalid property type!");
            }
        }
        catch(Exception&)
        {
            OSL_FAIL("OPropertyExport::dbg_implCheckProperty: caught an exception, could not check the property!");
        }
    }
#endif 

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
