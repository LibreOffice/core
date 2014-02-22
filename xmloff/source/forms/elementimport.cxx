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

#include "elementimport.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "strings.hxx"
#include "callbacks.hxx"
#include "attriblistmerge.hxx"
#include <xmloff/xmlnmspe.hxx>
#include "eventimport.hxx"
#include <xmloff/txtstyli.hxx>
#include "formenums.hxx"
#include <xmloff/xmltoken.hxx>
#include "gridcolumnproptranslator.hxx"
#include "property_description.hxx"
#include "property_meta_data.hxx"

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <sax/tools/converter.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>

#include <algorithm>
#include <functional>

namespace xmloff
{

    using namespace ::xmloff::token;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::xml;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::text;
    using namespace ::comphelper;
    using ::com::sun::star::xml::sax::XAttributeList;

#define PROPID_VALUE            1
#define PROPID_CURRENT_VALUE    2
#define PROPID_MIN_VALUE        3
#define PROPID_MAX_VALUE        4

    struct PropertyValueLess
    {
        sal_Bool operator()(const PropertyValue& _rLeft, const PropertyValue& _rRight)
        {
            return _rLeft.Name < _rRight.Name;
        }
    };

    struct PropertyValueCompare : public ::std::binary_function< PropertyValue, OUString, bool>
    {
        bool operator() (const PropertyValue& lhs, const OUString& rhs) const
        {
            return lhs.Name == rhs;
        }
        bool operator() (const OUString& lhs, const PropertyValue& rhs) const
        {
            return lhs == rhs.Name;
        }
    };

    template <class ELEMENT>
    void pushBackSequenceElement(Sequence< ELEMENT >& _rContainer, const ELEMENT& _rElement)
    {
        sal_Int32 nLen = _rContainer.getLength();
        _rContainer.realloc(nLen + 1);
        _rContainer[nLen] = _rElement;
    }

    
    OElementNameMap::MapString2Element  OElementNameMap::s_sElementTranslations;

    const OControlElement::ElementType& operator ++(OControlElement::ElementType& _e)
    {
        OControlElement::ElementType e = _e;
        sal_Int32 nAsInt = static_cast<sal_Int32>(e);
        _e = static_cast<OControlElement::ElementType>( ++nAsInt );
        return _e;
    }

    OControlElement::ElementType OElementNameMap::getElementType(const OUString& _rName)
    {
        if ( s_sElementTranslations.empty() )
        {   
            for (ElementType eType=(ElementType)0; eType<UNKNOWN; ++eType)
                s_sElementTranslations[OUString::createFromAscii(getElementName(eType))] = eType;
        }
        MapString2Element::const_iterator aPos = s_sElementTranslations.find(_rName);
        if (s_sElementTranslations.end() != aPos)
            return aPos->second;

        return UNKNOWN;
    }

    
    OElementImport::OElementImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OPropertyImport(_rImport, _nPrefix, _rName)
        ,m_rFormImport(_rImport)
        ,m_rEventManager(_rEventManager)
        ,m_pStyleElement( NULL )
        ,m_xParentContainer(_rxParentContainer)
        ,m_bImplicitGenericAttributeHandling( true )
    {
        OSL_ENSURE(m_xParentContainer.is(), "OElementImport::OElementImport: invalid parent container!");
    }

    OElementImport::~OElementImport()
    {
    }

    OUString OElementImport::determineDefaultServiceName() const
    {
        return OUString();
    }

    void OElementImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        ENTER_LOG_CONTEXT( "xmloff::OElementImport - importing one element" );

        const SvXMLNamespaceMap& rMap = m_rContext.getGlobalContext().GetNamespaceMap();
        const OUString sImplNameAttribute = rMap.GetQNameByKey( XML_NAMESPACE_FORM, GetXMLToken( XML_CONTROL_IMPLEMENTATION ) );
        const OUString sControlImplementation = _rxAttrList->getValueByName( sImplNameAttribute );

        
        if ( !sControlImplementation.isEmpty() )
        {
            OUString sOOoImplementationName;
            const sal_uInt16 nImplPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sControlImplementation, &sOOoImplementationName );
            m_sServiceName = ( nImplPrefix == XML_NAMESPACE_OOO ) ? sOOoImplementationName : sControlImplementation;
        }

        if ( m_sServiceName.isEmpty() )
            m_sServiceName = determineDefaultServiceName();

        
        
        
        
        m_xElement = createElement();
        if ( m_xElement.is() )
            m_xInfo = m_xElement->getPropertySetInfo();

        
        OPropertyImport::StartElement( _rxAttrList );
    }

    SvXMLImportContext* OElementImport::CreateChildContext(sal_uInt16 _nPrefix, const OUString& _rLocalName,
        const Reference< XAttributeList >& _rxAttrList)
    {
        if( token::IsXMLToken(_rLocalName, token::XML_EVENT_LISTENERS) && (XML_NAMESPACE_OFFICE == _nPrefix))
            return new OFormEventsImportContext(m_rFormImport.getGlobalContext(), _nPrefix, _rLocalName, *this);

        return OPropertyImport::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
    }

    void OElementImport::EndElement()
    {
        OSL_ENSURE(m_xElement.is(), "OElementImport::EndElement: invalid element created!");
        if (!m_xElement.is())
            return;

        
        implApplySpecificProperties();

        
        implApplyGenericProperties();

        
        if ( m_pStyleElement && m_xElement.is() )
        {
            Reference< XPropertySet > xPropTranslation =
                new OGridColumnPropertyTranslator( Reference< XMultiPropertySet >( m_xElement, UNO_QUERY ) );
            const_cast< XMLTextStyleContext* >( m_pStyleElement )->FillPropertySet( xPropTranslation );

            const OUString sNumberStyleName = const_cast< XMLTextStyleContext* >( m_pStyleElement )->GetDataStyleName( );
            if ( !sNumberStyleName.isEmpty() )
                
                m_rContext.applyControlNumberStyle( m_xElement, sNumberStyleName );
        }

        
        if (m_sName.isEmpty())
        {
            OSL_FAIL("OElementImport::EndElement: did not find a name attribute!");
            m_sName = implGetDefaultName();
        }

        if (m_xParentContainer.is())
            m_xParentContainer->insertByName(m_sName, makeAny(m_xElement));

        LEAVE_LOG_CONTEXT( );
    }

    void OElementImport::implApplySpecificProperties()
    {
        if ( m_aValues.empty() )
            return;

        
#if OSL_DEBUG_LEVEL > 0
        
        
        if ( m_xInfo.is() )
        {
            PropertyValueArray::const_iterator aEnd = m_aValues.end();
            for (   PropertyValueArray::iterator aCheck = m_aValues.begin();
                    aCheck != aEnd;
                    ++aCheck
                )
            {
                OSL_ENSURE(m_xInfo->hasPropertyByName(aCheck->Name),
                        OStringBuffer("OElementImport::implApplySpecificProperties: read a property (").
                    append(OUStringToOString(aCheck->Name, RTL_TEXTENCODING_ASCII_US)).
                    append(") which does not exist on the element!").getStr());
            }
        }
#endif

        
        const Reference< XMultiPropertySet > xMultiProps(m_xElement, UNO_QUERY);
        sal_Bool bSuccess = sal_False;
        if (xMultiProps.is())
        {
            

            
            ::std::sort( m_aValues.begin(), m_aValues.end(), PropertyValueLess());

            
            Sequence< OUString > aNames(m_aValues.size());
            OUString* pNames = aNames.getArray();
            
            Sequence< Any > aValues(m_aValues.size());
            Any* pValues = aValues.getArray();
            

            PropertyValueArray::iterator aEnd = m_aValues.end();
            for (   PropertyValueArray::iterator aPropValues = m_aValues.begin();
                    aPropValues != aEnd;
                    ++aPropValues, ++pNames, ++pValues
                )
            {
                *pNames = aPropValues->Name;
                *pValues = aPropValues->Value;
            }

            try
            {
                xMultiProps->setPropertyValues(aNames, aValues);
                bSuccess = sal_True;
            }
            catch(const Exception&)
            {
                OSL_FAIL("OElementImport::implApplySpecificProperties: could not set the properties (using the XMultiPropertySet)!");
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        if (!bSuccess)
        {   
            PropertyValueArray::iterator aEnd = m_aValues.end();
            for (   PropertyValueArray::iterator aPropValues = m_aValues.begin();
                    aPropValues != aEnd;
                    ++aPropValues
                )
            {
                
                
                try
                {
                    m_xElement->setPropertyValue(aPropValues->Name, aPropValues->Value);
                }
                catch(const Exception&)
                {
                    OSL_FAIL(OStringBuffer("OElementImport::implApplySpecificProperties: could not set the property \"").
                        append(OUStringToOString(aPropValues->Name, RTL_TEXTENCODING_ASCII_US)).
                        append("\"!").getStr());
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
    }

    void OElementImport::implApplyGenericProperties()
    {
        if ( m_aGenericValues.empty() )
            return;

        Reference< XPropertyContainer > xDynamicProperties( m_xElement, UNO_QUERY );

        PropertyValueArray::iterator aEnd = m_aGenericValues.end();
        for (   PropertyValueArray::iterator aPropValues =
                m_aGenericValues.begin();
                aPropValues != aEnd;
                ++aPropValues
            )
        {
            
            
            try
            {
                
                const bool bExistentProperty = m_xInfo->hasPropertyByName( aPropValues->Name );
                if ( !bExistentProperty )
                {
                    if ( !xDynamicProperties.is() )
                    {
                    #if OSL_DEBUG_LEVEL > 0
                        OString aMessage( "OElementImport::implApplyGenericProperties: encountered an unknown property (" );
                        aMessage += OUStringToOString( aPropValues->Name, RTL_TEXTENCODING_ASCII_US );
                        aMessage += "), but component is no PropertyBag!";
                        OSL_FAIL( aMessage.getStr() );
                    #endif
                        continue;
                    }

                    xDynamicProperties->addProperty(
                        aPropValues->Name,
                        PropertyAttribute::BOUND | PropertyAttribute::REMOVABLE,
                        aPropValues->Value
                    );

                    
                    m_xInfo = m_xElement->getPropertySetInfo();
                }

                
                TypeClass eValueTypeClass = aPropValues->Value.getValueTypeClass();
                const sal_Bool bValueIsSequence = TypeClass_SEQUENCE == eValueTypeClass;
                if ( bValueIsSequence )
                {
                    uno::Type aSimpleType( getSequenceElementType( aPropValues->Value.getValueType() ) );
                    eValueTypeClass = aSimpleType.getTypeClass();
                }

                
                const Property aProperty( m_xInfo->getPropertyByName( aPropValues->Name ) );
                TypeClass ePropTypeClass = aProperty.Type.getTypeClass();
                const sal_Bool bPropIsSequence = TypeClass_SEQUENCE == ePropTypeClass;
                if( bPropIsSequence )
                {
                    uno::Type aSimpleType( ::comphelper::getSequenceElementType( aProperty.Type ) );
                    ePropTypeClass = aSimpleType.getTypeClass();
                }

                if ( bPropIsSequence != bValueIsSequence )
                {
                    OSL_FAIL( "OElementImport::implImportGenericProperties: either both value and property should be a sequence, or none of them!" );
                    continue;
                }

                if ( bValueIsSequence )
                {
                    OSL_ENSURE( eValueTypeClass == TypeClass_ANY,
                        "OElementImport::implApplyGenericProperties: only ANYs should have been imported as generic list property!" );
                        

                    OSL_ENSURE( ePropTypeClass == TypeClass_SHORT,
                        "OElementImport::implApplyGenericProperties: conversion to sequences other than 'sequence< short >' not implemented, yet!" );

                    Sequence< Any > aXMLValueList;
                    aPropValues->Value >>= aXMLValueList;
                    Sequence< sal_Int16 > aPropertyValueList( aXMLValueList.getLength() );

                    const Any*       pXMLValue = aXMLValueList.getConstArray();
                          sal_Int16* pPropValue = aPropertyValueList.getArray();

                    for ( sal_Int32 i=0; i<aXMLValueList.getLength(); ++i, ++pXMLValue, ++pPropValue )
                    {
                        
                        double nVal( 0 );
                        OSL_VERIFY( *pXMLValue >>= nVal );
                        *pPropValue = static_cast< sal_Int16 >( nVal );
                    }

                    aPropValues->Value <<= aPropertyValueList;
                }
                else if ( ePropTypeClass != eValueTypeClass )
                {
                    switch ( eValueTypeClass )
                    {
                    case TypeClass_DOUBLE:
                    {
                        double nVal = 0;
                        aPropValues->Value >>= nVal;
                        switch( ePropTypeClass )
                        {
                        case TypeClass_BYTE:
                            aPropValues->Value <<= static_cast< sal_Int8 >( nVal );
                            break;
                        case TypeClass_SHORT:
                            aPropValues->Value <<= static_cast< sal_Int16 >( nVal );
                            break;
                        case TypeClass_LONG:
                        case TypeClass_ENUM:
                            aPropValues->Value <<= static_cast< sal_Int32 >( nVal );
                            break;
                        case TypeClass_HYPER:
                            aPropValues->Value <<= static_cast< sal_Int64 >( nVal );
                            break;
                        default:
                            OSL_FAIL( "OElementImport::implImportGenericProperties: unsupported value type!" );
                            break;
                        }
                    }
                    break;
                    default:
                        OSL_FAIL( "OElementImport::implImportGenericProperties: non-double values not supported!" );
                        break;
                    }
                }

                m_xElement->setPropertyValue( aPropValues->Name, aPropValues->Value );
            }
            catch(const Exception&)
            {
                OSL_FAIL(OStringBuffer("OElementImport::EndElement: could not set the property \"").
                    append(OUStringToOString(aPropValues->Name, RTL_TEXTENCODING_ASCII_US)).
                    append("\"!").getStr());
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    OUString OElementImport::implGetDefaultName() const
    {
        
        
        static const OUString sUnnamedName("unnamed");
        OSL_ENSURE(m_xParentContainer.is(), "OElementImport::implGetDefaultName: no parent container!");
        if (!m_xParentContainer.is())
            return sUnnamedName;
        Sequence< OUString > aNames = m_xParentContainer->getElementNames();

        OUString sReturn;
        const OUString* pNames = NULL;
        const OUString* pNamesEnd = aNames.getConstArray() + aNames.getLength();
        for (sal_Int32 i=0; i<32768; ++i)   
        {
            
            sReturn = sUnnamedName;
            sReturn += OUString::number(i);
            
            for (pNames = aNames.getConstArray(); pNames<pNamesEnd; ++pNames)
            {
                if (*pNames == sReturn)
                {
                    break;
                }
            }
            if (pNames<pNamesEnd)
                
                continue;
            return sReturn;
        }
        OSL_FAIL("OElementImport::implGetDefaultName: did not find a free name!");
        return sUnnamedName;
    }

    PropertyGroups::const_iterator OElementImport::impl_matchPropertyGroup( const PropertyGroups& i_propertyGroups ) const
    {
        ENSURE_OR_RETURN( m_xInfo.is(), "OElementImport::impl_matchPropertyGroup: no property set info!", i_propertyGroups.end() );

        for (   PropertyGroups::const_iterator group = i_propertyGroups.begin();
                group != i_propertyGroups.end();
                ++group
            )
        {
            bool missingProp = false;
            for (   PropertyDescriptionList::const_iterator prop = group->begin();
                    prop != group->end();
                    ++prop
                )
            {
                if ( !m_xInfo->hasPropertyByName( (*prop)->propertyName ) )
                {
                    missingProp = true;
                    break;
                }
            }

            if ( missingProp )
                
                continue;

            return group;
        }

        return i_propertyGroups.end();
    }

    bool OElementImport::tryGenericAttribute( sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue )
    {
        
        const AttributeDescription attribute( metadata::getAttributeDescription( _nNamespaceKey, _rLocalName ) );
        if ( attribute.attributeToken != XML_TOKEN_INVALID )
        {
            PropertyGroups propertyGroups;
            metadata::getPropertyGroupList( attribute, propertyGroups );
            const PropertyGroups::const_iterator pos = impl_matchPropertyGroup( propertyGroups );
            if ( pos == propertyGroups.end() )
                return false;

            do
            {
                const PropertyDescriptionList& rProperties( *pos );
                const PropertyDescription* first = *rProperties.begin();
                if ( !first )
                {
                    SAL_WARN( "xmloff.forms", "OElementImport::handleAttribute: invalid property description!" );
                    break;
                }

                const PPropertyHandler handler = (*first->factory)( first->propertyId );
                if ( !handler.get() )
                {
                    SAL_WARN( "xmloff.forms", "OElementImport::handleAttribute: invalid property handler!" );
                    break;
                }

                PropertyValues aValues;
                for (   PropertyDescriptionList::const_iterator propDesc = rProperties.begin();
                        propDesc != rProperties.end();
                        ++propDesc
                    )
                {
                    aValues[ (*propDesc)->propertyId ] = Any();
                }
                if ( handler->getPropertyValues( _rValue, aValues ) )
                {
                    for (   PropertyDescriptionList::const_iterator propDesc = rProperties.begin();
                            propDesc != rProperties.end();
                            ++propDesc
                        )
                    {
                        implPushBackPropertyValue( (*propDesc)->propertyName, aValues[ (*propDesc)->propertyId ] );
                    }
                }
            }
            while ( false );

            
            return true;
        }
        return false;
    }

    bool OElementImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue)
    {
        if ( token::IsXMLToken( _rLocalName, token::XML_CONTROL_IMPLEMENTATION ) )
            
            return true;

        if ( token::IsXMLToken( _rLocalName, token::XML_NAME ) )
        {
            if ( m_sName.isEmpty() )
                
                m_sName = _rValue;
            return true;
        }

        
        if ( token::IsXMLToken( _rLocalName, token::XML_TEXT_STYLE_NAME ) )
        {
            const SvXMLStyleContext* pStyleContext = m_rContext.getStyleElement( _rValue );
            OSL_ENSURE( pStyleContext, "OElementImport::handleAttribute: do not know the style!" );
            
            m_pStyleElement = PTR_CAST( XMLTextStyleContext, pStyleContext );
            return true;
        }

        if ( m_bImplicitGenericAttributeHandling )
            if ( tryGenericAttribute( _nNamespaceKey, _rLocalName, _rValue ) )
                return true;

        
        return OPropertyImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    Reference< XPropertySet > OElementImport::createElement()
    {
        Reference< XPropertySet > xReturn;
        if (!m_sServiceName.isEmpty())
        {
            Reference< XComponentContext > xContext = m_rFormImport.getGlobalContext().GetComponentContext();
            Reference< XInterface > xPure = xContext->getServiceManager()->createInstanceWithContext(m_sServiceName, xContext);
            OSL_ENSURE(xPure.is(),
                        OStringBuffer("OElementImport::createElement: service factory gave me no object (service name: ").append(OUStringToOString(m_sServiceName, RTL_TEXTENCODING_ASCII_US)).append(")!").getStr());
            xReturn = Reference< XPropertySet >(xPure, UNO_QUERY);
        }
        else
            OSL_FAIL("OElementImport::createElement: no service name to create an element!");

        return xReturn;
    }

    void OElementImport::registerEvents(const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        OSL_ENSURE(m_xElement.is(), "OElementImport::registerEvents: no element to register events for!");
        m_rEventManager.registerEvents(m_xElement, _rEvents);
    }

    void OElementImport::simulateDefaultedAttribute(const sal_Char* _pAttributeName, const OUString& _rPropertyName, const sal_Char* _pAttributeDefault)
    {
        OSL_ENSURE( m_xInfo.is(), "OPropertyImport::simulateDefaultedAttribute: the component should be more gossipy about it's properties!" );

        if ( !m_xInfo.is() || m_xInfo->hasPropertyByName( _rPropertyName ) )
        {
            OUString sLocalAttrName = OUString::createFromAscii(_pAttributeName);
            if ( !encounteredAttribute( sLocalAttrName ) )
                OSL_VERIFY( handleAttribute( XML_NAMESPACE_FORM, sLocalAttrName, OUString::createFromAscii( _pAttributeDefault ) ) );
        }
    }

    
    OControlImport::OControlImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OElementImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
        ,m_eElementType(OControlElement::UNKNOWN)
    {
        disableImplicitGenericAttributeHandling();
    }

    OControlImport::OControlImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OElementImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
        ,m_eElementType(_eType)
    {
        disableImplicitGenericAttributeHandling();
    }

    OUString OControlImport::determineDefaultServiceName() const
    {
        const sal_Char* pServiceName = NULL;
        switch ( m_eElementType )
        {
        case OControlElement::TEXT:
        case OControlElement::TEXT_AREA:
        case OControlElement::PASSWORD:          pServiceName = "com.sun.star.form.component.TextField"; break;
        case OControlElement::FILE:              pServiceName = "com.sun.star.form.component.FileControl"; break;
        case OControlElement::FORMATTED_TEXT:    pServiceName = "com.sun.star.form.component.FormattedField"; break;
        case OControlElement::FIXED_TEXT:        pServiceName = "com.sun.star.form.component.FixedText"; break;
        case OControlElement::COMBOBOX:          pServiceName = "com.sun.star.form.component.ComboBox"; break;
        case OControlElement::LISTBOX:           pServiceName = "com.sun.star.form.component.ListBox"; break;
        case OControlElement::BUTTON:            pServiceName = "com.sun.star.form.component.CommandButton"; break;
        case OControlElement::IMAGE:             pServiceName = "com.sun.star.form.component.ImageButton"; break;
        case OControlElement::CHECKBOX:          pServiceName = "com.sun.star.form.component.CheckBox"; break;
        case OControlElement::RADIO:             pServiceName = "com.sun.star.form.component.RadioButton"; break;
        case OControlElement::FRAME:             pServiceName = "com.sun.star.form.component.GroupBox"; break;
        case OControlElement::IMAGE_FRAME:       pServiceName = "com.sun.star.form.component.DatabaseImageControl"; break;
        case OControlElement::HIDDEN:            pServiceName = "com.sun.star.form.component.HiddenControl"; break;
        case OControlElement::GRID:              pServiceName = "com.sun.star.form.component.GridControl"; break;
        case OControlElement::TIME:              pServiceName = "com.sun.star.form.component.TimeField"; break;
        case OControlElement::DATE:              pServiceName = "com.sun.star.form.component.DateField"; break;
        default:                                 break;
        }
        if ( pServiceName != NULL )
            return OUString::createFromAscii( pServiceName );
        return OUString();
    }

    void OControlImport::addOuterAttributes(const Reference< XAttributeList >& _rxOuterAttribs)
    {
        OSL_ENSURE(!m_xOuterAttributes.is(), "OControlImport::addOuterAttributes: already have these attributes!");
        m_xOuterAttributes = _rxOuterAttribs;
    }

    bool OControlImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue)
    {
        static const sal_Char* pLinkedCellAttributeName = OAttributeMetaData::getBindingAttributeName(BA_LINKED_CELL);

        if (IsXMLToken(_rLocalName, XML_ID))
        {   
            if (XML_NAMESPACE_XML == _nNamespaceKey)
            {
                m_sControlId = _rValue;
            }
            else if (XML_NAMESPACE_FORM == _nNamespaceKey)
            {
                if (m_sControlId.isEmpty())
                {
                    m_sControlId = _rValue;
                }
            }
            return true;
        }

        if ( _rLocalName.equalsAscii( pLinkedCellAttributeName ) )
        {   
            m_sBoundCellAddress = _rValue;
            return true;
        }

        if ( _nNamespaceKey == XML_NAMESPACE_XFORMS && IsXMLToken( _rLocalName, XML_BIND ) )
        {
            m_sBindingID = _rValue;
            return true;
        }

        if ( _nNamespaceKey == XML_NAMESPACE_FORM && IsXMLToken( _rLocalName, XML_XFORMS_LIST_SOURCE )  )
        {
            m_sListBindingID = _rValue;
            return true;
        }

        if  (   (   ( _nNamespaceKey == XML_NAMESPACE_FORM )
                &&  IsXMLToken( _rLocalName, XML_XFORMS_SUBMISSION )
                )
            ||  (   ( _nNamespaceKey == XML_NAMESPACE_XFORMS )
                &&  IsXMLToken( _rLocalName, XML_SUBMISSION )
                )
            )
        {
            m_sSubmissionID = _rValue;
            return true;
        }

        if ( OElementImport::tryGenericAttribute( _nNamespaceKey, _rLocalName, _rValue ) )
            return true;

        static const sal_Char* pValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_VALUE);
        static const sal_Char* pCurrentValueAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_VALUE);
        static const sal_Char* pMinValueAttributeName = OAttributeMetaData::getSpecialAttributeName(SCA_MIN_VALUE);
        static const sal_Char* pMaxValueAttributeName = OAttributeMetaData::getSpecialAttributeName(SCA_MAX_VALUE);
        static const sal_Char* pRepeatDelayAttributeName = OAttributeMetaData::getSpecialAttributeName( SCA_REPEAT_DELAY );

        sal_Int32 nHandle = -1;
        if ( _rLocalName.equalsAscii( pValueAttributeName ) )
            nHandle = PROPID_VALUE;
        else if ( _rLocalName.equalsAscii( pCurrentValueAttributeName ) )
            nHandle = PROPID_CURRENT_VALUE;
        else if ( _rLocalName.equalsAscii( pMinValueAttributeName ) )
            nHandle = PROPID_MIN_VALUE;
        else if ( _rLocalName.equalsAscii( pMaxValueAttributeName ) )
            nHandle = PROPID_MAX_VALUE;
        if ( nHandle != -1 )
        {
            
            PropertyValue aProp;
            aProp.Name = _rLocalName;
            aProp.Handle = nHandle;
            aProp.Value <<= _rValue;
            m_aValueProperties.push_back(aProp);
            return true;
        }

        if ( _rLocalName.equalsAscii( pRepeatDelayAttributeName ) )
        {
            util::Duration aDuration;
            if (::sax::Converter::convertDuration(aDuration, _rValue))
            {
                PropertyValue aProp;
                aProp.Name = PROPERTY_REPEAT_DELAY;
                sal_Int32 const nMS =
                    ((aDuration.Hours * 60 + aDuration.Minutes) * 60
                     + aDuration.Seconds) * 1000 + aDuration.NanoSeconds/1000000;
                aProp.Value <<= nMS;

                implPushBackPropertyValue(aProp);
            }
            return true;
        }

        return OElementImport::handleAttribute( _nNamespaceKey, _rLocalName, _rValue );
    }

    void OControlImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttributes;
        if( m_xOuterAttributes.is() )
        {
            
            OAttribListMerger* pMerger = new OAttribListMerger;
            
            pMerger->addList(_rxAttrList);
            
            pMerger->addList(m_xOuterAttributes);
            xAttributes = pMerger;
        }
        else
        {
            xAttributes = _rxAttrList;
        }

        
        OElementImport::StartElement(xAttributes);

        if ( !m_aValueProperties.empty() && m_xElement.is())
        {
            
            if (!m_xInfo.is())
            {
                OSL_FAIL("OControlImport::StartElement: no PropertySetInfo!");
                return;
            }

            const sal_Char* pValueProperty = NULL;
            const sal_Char* pCurrentValueProperty = NULL;
            const sal_Char* pMinValueProperty = NULL;
            const sal_Char* pMaxValueProperty = NULL;

            sal_Bool bRetrievedValues = sal_False;
            sal_Bool bRetrievedValueLimits = sal_False;

            
            sal_Int16 nClassId = FormComponentType::CONTROL;
            m_xElement->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;

            
            PropertyValueArray::iterator aEnd = m_aValueProperties.end();
            for (   PropertyValueArray::iterator aValueProps = m_aValueProperties.begin();
                    aValueProps != aEnd;
                    ++aValueProps
                )
            {
                bool bSuccess = false;
                switch (aValueProps->Handle)
                {
                    case PROPID_VALUE:
                    case PROPID_CURRENT_VALUE:
                    {
                        
                        if (!bRetrievedValues)
                        {
                            getValuePropertyNames(m_eElementType, nClassId, pCurrentValueProperty, pValueProperty);
                            if ( !pCurrentValueProperty && !pValueProperty )
                            {
                                SAL_WARN( "xmloff.forms", "OControlImport::StartElement: illegal value property names!" );
                                break;
                            }

                            bRetrievedValues = sal_True;
                        }
                        if ( PROPID_VALUE == aValueProps->Handle && !pValueProperty )
                        {
                            SAL_WARN( "xmloff.forms", "OControlImport::StartElement: the control does not have a value property!");
                            break;
                        }

                        if ( PROPID_CURRENT_VALUE == aValueProps->Handle && !pCurrentValueProperty )
                        {
                            SAL_WARN( "xmloff.forms", "OControlImport::StartElement: the control does not have a current-value property!");
                            break;
                        }

                        
                        if (PROPID_VALUE == aValueProps->Handle)
                            aValueProps->Name = OUString::createFromAscii(pValueProperty);
                        else
                            aValueProps->Name = OUString::createFromAscii(pCurrentValueProperty);
                        bSuccess = true;
                    }
                    break;
                    case PROPID_MIN_VALUE:
                    case PROPID_MAX_VALUE:
                    {
                        
                        if (!bRetrievedValueLimits)
                        {
                            getValueLimitPropertyNames(nClassId, pMinValueProperty, pMaxValueProperty);
                            if ( !pMinValueProperty || !pMaxValueProperty )
                            {
                                SAL_WARN( "xmloff.forms", "OControlImport::StartElement: illegal value limit property names!" );
                                break;
                            }

                            bRetrievedValueLimits = sal_True;
                        }
                        OSL_ENSURE((PROPID_MIN_VALUE != aValueProps->Handle) || pMinValueProperty,
                            "OControlImport::StartElement: the control does not have a value property!");
                        OSL_ENSURE((PROPID_MAX_VALUE != aValueProps->Handle) || pMaxValueProperty,
                            "OControlImport::StartElement: the control does not have a current-value property!");

                        
                        if (PROPID_MIN_VALUE == aValueProps->Handle)
                            aValueProps->Name = OUString::createFromAscii(pMinValueProperty);
                        else
                            aValueProps->Name = OUString::createFromAscii(pMaxValueProperty);
                        bSuccess = true;
                    }
                    break;
                }

                if ( !bSuccess )
                    continue;

                
                implTranslateValueProperty(m_xInfo, *aValueProps);
                
                implPushBackPropertyValue(*aValueProps);
            }
        }
    }

    void OControlImport::implTranslateValueProperty(const Reference< XPropertySetInfo >& _rxPropInfo,
        PropertyValue& _rPropValue)
    {
        OSL_ENSURE(_rxPropInfo->hasPropertyByName(_rPropValue.Name),
            "OControlImport::implTranslateValueProperty: invalid property name!");

        
        Property aProp = _rxPropInfo->getPropertyByName(_rPropValue.Name);
        
        OUString sValue;
    #if OSL_DEBUG_LEVEL > 0
        sal_Bool bSuccess =
    #endif
        _rPropValue.Value >>= sValue;
        OSL_ENSURE(bSuccess, "OControlImport::implTranslateValueProperty: supposed to be called with non-translated string values!");

        if (TypeClass_ANY == aProp.Type.getTypeClass())
        {
            
            OSL_ENSURE(
                    !_rPropValue.Name.equals(PROPERTY_EFFECTIVE_VALUE)
                ||  !_rPropValue.Name.equals(PROPERTY_EFFECTIVE_DEFAULT),
                "OControlImport::implTranslateValueProperty: invalid property type/name combination!");

            
            
            double nValue;
            if (::sax::Converter::convertDouble(nValue, sValue))
                _rPropValue.Value <<= nValue;
            else
                _rPropValue.Value <<= sValue;
        }
        else
            _rPropValue.Value = PropertyConversion::convertString(GetImport(), aProp.Type, sValue);
    }

    void OControlImport::EndElement()
    {
        OSL_ENSURE(m_xElement.is(), "OControlImport::EndElement: invalid control!");
        if ( !m_xElement.is() )
            return;

        
        if (!m_sControlId.isEmpty())
            m_rFormImport.registerControlId(m_xElement, m_sControlId);
        

        
        
        
        
        
        
        

        sal_Bool bRestoreValuePropertyValue = sal_False;
        Any aValuePropertyValue;

        sal_Int16 nClassId = FormComponentType::CONTROL;
        try
        {
            
            m_xElement->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OControlImport::EndElement: caught an exception while retrieving the class id!" );
            DBG_UNHANDLED_EXCEPTION();
        }

        const sal_Char* pValueProperty = NULL;
        const sal_Char* pDefaultValueProperty = NULL;
        getRuntimeValuePropertyNames(m_eElementType, nClassId, pValueProperty, pDefaultValueProperty);
        if ( pDefaultValueProperty && pValueProperty )
        {
            sal_Bool bNonDefaultValuePropertyValue = sal_False;
                

            
            PropertyValueArray::iterator aEnd = m_aValues.end();
            for (   PropertyValueArray::iterator aCheck = m_aValues.begin();
                    ( aCheck != aEnd );
                    ++aCheck
                )
            {
                if ( aCheck->Name.equalsAscii( pDefaultValueProperty ) )
                    bRestoreValuePropertyValue = sal_True;
                else if ( aCheck->Name.equalsAscii( pValueProperty ) )
                {
                    bNonDefaultValuePropertyValue = sal_True;
                    
                    aValuePropertyValue = aCheck->Value;
                }
            }

            if ( bRestoreValuePropertyValue && !bNonDefaultValuePropertyValue )
            {
                
                try
                {
                    aValuePropertyValue = m_xElement->getPropertyValue( OUString::createFromAscii( pValueProperty ) );
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "OControlImport::EndElement: caught an exception while retrieving the current value property!" );
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }

        
        OElementImport::EndElement();

        
        if ( bRestoreValuePropertyValue && pValueProperty )
        {
            try
            {
                m_xElement->setPropertyValue( OUString::createFromAscii( pValueProperty ), aValuePropertyValue );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "OControlImport::EndElement: caught an exception while restoring the value property!" );
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        
        if ( m_xElement.is() && !m_sBoundCellAddress.isEmpty() )
            doRegisterCellValueBinding( m_sBoundCellAddress );

        
        if ( m_xElement.is() && !m_sBindingID.isEmpty() )
            doRegisterXFormsValueBinding( m_sBindingID );

        
        if ( m_xElement.is() && !m_sListBindingID.isEmpty() )
            doRegisterXFormsListBinding( m_sListBindingID );

        
        if ( m_xElement.is() && !m_sSubmissionID.isEmpty() )
            doRegisterXFormsSubmission( m_sSubmissionID );
    }

    void OControlImport::doRegisterCellValueBinding( const OUString& _rBoundCellAddress )
    {
        OSL_PRECOND( m_xElement.is(), "OControlImport::doRegisterCellValueBinding: invalid element!" );
        OSL_PRECOND( !_rBoundCellAddress.isEmpty(),
            "OControlImport::doRegisterCellValueBinding: invalid address!" );

        m_rContext.registerCellValueBinding( m_xElement, _rBoundCellAddress );
    }

    void OControlImport::doRegisterXFormsValueBinding( const OUString& _rBindingID )
    {
        OSL_PRECOND( m_xElement.is(), "need element" );
        OSL_PRECOND( !_rBindingID.isEmpty(), "binding ID is not valid" );

        m_rContext.registerXFormsValueBinding( m_xElement, _rBindingID );
    }

    void OControlImport::doRegisterXFormsListBinding( const OUString& _rBindingID )
    {
        OSL_PRECOND( m_xElement.is(), "need element" );
        OSL_PRECOND( !_rBindingID.isEmpty(), "binding ID is not valid" );

        m_rContext.registerXFormsListBinding( m_xElement, _rBindingID );
    }

    void OControlImport::doRegisterXFormsSubmission( const OUString& _rSubmissionID )
    {
        OSL_PRECOND( m_xElement.is(), "need element" );
        OSL_PRECOND( !_rSubmissionID.isEmpty(), "binding ID is not valid" );

        m_rContext.registerXFormsSubmission( m_xElement, _rSubmissionID );
    }

    Reference< XPropertySet > OControlImport::createElement()
    {
        const Reference<XPropertySet> xPropSet = OElementImport::createElement();
        if ( xPropSet.is() )
        {
            m_xInfo = xPropSet->getPropertySetInfo();
            if ( m_xInfo.is() && m_xInfo->hasPropertyByName(PROPERTY_ALIGN) )
            {
                Any aValue;
                xPropSet->setPropertyValue(PROPERTY_ALIGN,aValue);
            }
        }
        return xPropSet;
    }

    
    OImagePositionImport::OImagePositionImport( OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
        sal_uInt16 _nPrefix, const OUString& _rName, const Reference< XNameContainer >& _rxParentContainer,
        OControlElement::ElementType _eType )
        :OControlImport( _rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType )
        ,m_nImagePosition( -1 )
        ,m_nImageAlign( 0 )
        ,m_bHaveImagePosition( sal_False )
    {
    }

    bool OImagePositionImport::handleAttribute( sal_uInt16 _nNamespaceKey, const OUString& _rLocalName,
        const OUString& _rValue )
    {
        if ( _rLocalName == GetXMLToken( XML_IMAGE_POSITION ) )
        {
            OSL_VERIFY( PropertyConversion::convertString(
                m_rContext.getGlobalContext(), ::getCppuType( &m_nImagePosition ),
                _rValue, OEnumMapper::getEnumMap( OEnumMapper::epImagePosition )
            ) >>= m_nImagePosition );
            m_bHaveImagePosition = sal_True;
            return true;
        }

        if ( _rLocalName == GetXMLToken( XML_IMAGE_ALIGN ) )
        {
            OSL_VERIFY( PropertyConversion::convertString(
                m_rContext.getGlobalContext(), ::getCppuType( &m_nImageAlign ),
                _rValue, OEnumMapper::getEnumMap( OEnumMapper::epImageAlign )
            ) >>= m_nImageAlign );
            return true;
        }

        return OControlImport::handleAttribute( _nNamespaceKey, _rLocalName, _rValue );
    }

    void OImagePositionImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        OControlImport::StartElement( _rxAttrList );

        if ( m_bHaveImagePosition )
        {
            sal_Int16 nUnoImagePosition = ImagePosition::Centered;
            if ( m_nImagePosition >= 0 )
            {
                OSL_ENSURE( ( m_nImagePosition <= 3 ) && ( m_nImageAlign >= 0 ) && ( m_nImageAlign < 3 ),
                    "OImagePositionImport::StartElement: unknown image align and/or position!" );
                nUnoImagePosition = m_nImagePosition * 3 + m_nImageAlign;
            }

            PropertyValue aImagePosition;
            aImagePosition.Name = PROPERTY_IMAGE_POSITION;
            aImagePosition.Value <<= nUnoImagePosition;
            implPushBackPropertyValue( aImagePosition );
        }
    }

    
    OReferredControlImport::OReferredControlImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType )
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
    {
    }

    void OReferredControlImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        OControlImport::StartElement(_rxAttrList);

        
        if ( !m_sReferringControls.isEmpty() )
            m_rFormImport.registerControlReferences(m_xElement, m_sReferringControls);
    }

    bool OReferredControlImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName,
        const OUString& _rValue)
    {
        static const OUString s_sReferenceAttributeName = OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_FOR));
        if (_rLocalName == s_sReferenceAttributeName)
        {
            m_sReferringControls = _rValue;
            return true;
        }
        return OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    
    OPasswordImport::OPasswordImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
    }

    bool OPasswordImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue)
    {
        static const OUString s_sEchoCharAttributeName = OUString::createFromAscii(OAttributeMetaData::getSpecialAttributeName(SCA_ECHO_CHAR));
        if (_rLocalName == s_sEchoCharAttributeName)
        {
            
            PropertyValue aEchoChar;
            aEchoChar.Name = PROPERTY_ECHOCHAR;
            OSL_ENSURE(_rValue.getLength() == 1, "OPasswordImport::handleAttribute: invalid echo char attribute!");
                
            if (_rValue.getLength() >= 1)
                aEchoChar.Value <<= (sal_Int16)_rValue[0];
            else
                aEchoChar.Value <<= (sal_Int16)0;
            implPushBackPropertyValue(aEchoChar);
            return true;
        }
        return OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    
    ORadioImport::ORadioImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OImagePositionImport( _rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType )
    {
    }

    bool ORadioImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue)
    {
        
        
        static const sal_Char* pCurrentSelectedAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED);
        static const sal_Char* pSelectedAttributeName = OAttributeMetaData::getCommonControlAttributeName(CCA_SELECTED);
        if  (  _rLocalName.equalsAscii( pCurrentSelectedAttributeName )
            || _rLocalName.equalsAscii( pSelectedAttributeName )
            )
        {
            const OAttribute2Property::AttributeAssignment* pProperty = m_rContext.getAttributeMap().getAttributeTranslation(_rLocalName);
            OSL_ENSURE(pProperty, "ORadioImport::handleAttribute: invalid property map!");
            if (pProperty)
            {
                const Any aBooleanValue( PropertyConversion::convertString(m_rContext.getGlobalContext(), pProperty->aPropertyType, _rValue, pProperty->pEnumMap) );

                
                PropertyValue aNewValue;
                aNewValue.Name = pProperty->sPropertyName;
                aNewValue.Value <<= (sal_Int16)::cppu::any2bool(aBooleanValue);

                implPushBackPropertyValue(aNewValue);
            }
            return true;
        }
        return OImagePositionImport::handleAttribute( _nNamespaceKey, _rLocalName, _rValue );
    }

    
    OURLReferenceImport::OURLReferenceImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OImagePositionImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
    }

    bool OURLReferenceImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue)
    {
        static const sal_Char* s_pTargetLocationAttributeName   = OAttributeMetaData::getCommonControlAttributeName( CCA_TARGET_LOCATION );
        static const sal_Char* s_pImageDataAttributeName        = OAttributeMetaData::getCommonControlAttributeName( CCA_IMAGE_DATA );

        
        
        
        sal_Bool bMakeAbsolute =
                _rLocalName.equalsAscii( s_pImageDataAttributeName )
            ||  (   _rLocalName.equalsAscii( s_pTargetLocationAttributeName )
                &&  (   ( OControlElement::BUTTON == m_eElementType )
                    ||  ( OControlElement::IMAGE == m_eElementType )
                    )
                );

        if ( bMakeAbsolute && !_rValue.isEmpty() )
        {
            
            OUString sAdjustedValue;
            
            
            
            if ( _rLocalName.equalsAscii( s_pImageDataAttributeName ) )
                sAdjustedValue = m_rContext.getGlobalContext().ResolveGraphicObjectURL( _rValue, sal_False );
            else
                sAdjustedValue = m_rContext.getGlobalContext().GetAbsoluteReference( _rValue );
            return OImagePositionImport::handleAttribute( _nNamespaceKey, _rLocalName, sAdjustedValue );
        }

        return OImagePositionImport::handleAttribute( _nNamespaceKey, _rLocalName, _rValue );
    }

    
    OButtonImport::OButtonImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OURLReferenceImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
        enableTrackAttributes();
    }

    void OButtonImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        OURLReferenceImport::StartElement(_rxAttrList);

        
        simulateDefaultedAttribute(OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_FRAME), PROPERTY_TARGETFRAME, "_blank");
    }

    
    OValueRangeImport::OValueRangeImport( OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType )
        :OControlImport( _rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType )
        ,m_nStepSizeValue( 1 )
    {

    }

    bool OValueRangeImport::handleAttribute( sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue )
    {
        if ( _rLocalName.equalsAscii( OAttributeMetaData::getSpecialAttributeName( SCA_STEP_SIZE ) ) )
        {
            ::sax::Converter::convertNumber( m_nStepSizeValue, _rValue );
            return true;
        }
        return OControlImport::handleAttribute( _nNamespaceKey, _rLocalName, _rValue );
    }

    void OValueRangeImport::StartElement( const Reference< XAttributeList >& _rxAttrList )
    {
        OControlImport::StartElement( _rxAttrList );

        if ( m_xInfo.is() )
        {
            if ( m_xInfo->hasPropertyByName( PROPERTY_SPIN_INCREMENT ) )
                m_xElement->setPropertyValue( PROPERTY_SPIN_INCREMENT, makeAny( m_nStepSizeValue ) );
            else if ( m_xInfo->hasPropertyByName( PROPERTY_LINE_INCREMENT ) )
                m_xElement->setPropertyValue( PROPERTY_LINE_INCREMENT, makeAny( m_nStepSizeValue ) );
        }
    }

    
    OTextLikeImport::OTextLikeImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
        ,m_bEncounteredTextPara( false )
    {
        enableTrackAttributes();
    }

    SvXMLImportContext* OTextLikeImport::CreateChildContext( sal_uInt16 _nPrefix, const OUString& _rLocalName,
        const Reference< XAttributeList >& _rxAttrList )
    {
        if ( ( XML_NAMESPACE_TEXT == _nPrefix ) && _rLocalName.equalsIgnoreAsciiCase("p") )
        {
            OSL_ENSURE( m_eElementType == OControlElement::TEXT_AREA,
                "OTextLikeImport::CreateChildContext: text paragraphs in a non-text-area?" );

            if ( m_eElementType == OControlElement::TEXT_AREA )
            {
                Reference< XText > xTextElement( m_xElement, UNO_QUERY );
                if ( xTextElement.is() )
                {
                    UniReference < XMLTextImportHelper > xTextImportHelper( m_rContext.getGlobalContext().GetTextImport() );

                    if ( !m_xCursor.is() )
                    {
                        m_xOldCursor = xTextImportHelper->GetCursor();
                        m_xCursor = xTextElement->createTextCursor();

                        if ( m_xCursor.is() )
                            xTextImportHelper->SetCursor( m_xCursor );
                    }
                    if ( m_xCursor.is() )
                    {
                        m_bEncounteredTextPara = true;
                        return xTextImportHelper->CreateTextChildContext( m_rContext.getGlobalContext(), _nPrefix, _rLocalName, _rxAttrList );
                    }
                }
                else
                {
                    
                    
                }
            }
        }

        return OControlImport::CreateChildContext( _nPrefix, _rLocalName, _rxAttrList );
    }

    void OTextLikeImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        OControlImport::StartElement(_rxAttrList);

        
        
        
        
        if (m_xElement.is() && m_xInfo.is() && m_xInfo->hasPropertyByName(PROPERTY_EMPTY_IS_NULL) )
            simulateDefaultedAttribute(OAttributeMetaData::getDatabaseAttributeName(DA_CONVERT_EMPTY), PROPERTY_EMPTY_IS_NULL, "false");
    }

    struct EqualHandle : public ::std::unary_function< PropertyValue, bool >
    {
        const sal_Int32 m_nHandle;
        EqualHandle( sal_Int32 _nHandle ) : m_nHandle( _nHandle ) { }

        inline bool operator()( const PropertyValue& _rProp )
        {
            return _rProp.Handle == m_nHandle;
        }
    };

    void OTextLikeImport::removeRedundantCurrentValue()
    {
        if ( m_bEncounteredTextPara )
        {
            
            
            
            
            
            
            PropertyValueArray::iterator aValuePropertyPos = ::std::find_if(
                m_aValues.begin(),
                m_aValues.end(),
                EqualHandle( PROPID_CURRENT_VALUE )
            );
            if ( aValuePropertyPos != m_aValues.end() )
            {
                OSL_ENSURE( aValuePropertyPos->Name == PROPERTY_TEXT, "OTextLikeImport::EndElement: text:p was present, but our value property is *not* 'Text'!" );
                if ( aValuePropertyPos->Name == PROPERTY_TEXT )
                {
                    ::std::copy(
                        aValuePropertyPos + 1,
                        m_aValues.end(),
                        aValuePropertyPos
                    );
                    m_aValues.resize( m_aValues.size() - 1 );
                }
            }

            
            
            sal_Bool bHasRichTextProperty = sal_False;
            if ( m_xInfo.is() )
                bHasRichTextProperty = m_xInfo->hasPropertyByName( PROPERTY_RICH_TEXT );
            OSL_ENSURE( bHasRichTextProperty, "OTextLikeImport::EndElement: text:p, but no rich text control?" );
            if ( bHasRichTextProperty )
                m_xElement->setPropertyValue( PROPERTY_RICH_TEXT, makeAny( (sal_Bool)sal_True ) );
        }
        
        
    }

    struct EqualName : public ::std::unary_function< PropertyValue, bool >
    {
        const OUString m_sName;
        EqualName( const OUString& _rName ) : m_sName( _rName ) { }

        inline bool operator()( const PropertyValue& _rProp )
        {
            return _rProp.Name == m_sName;
        }
    };

    void OTextLikeImport::adjustDefaultControlProperty()
    {
        
        
        
        PropertyValueArray::iterator aDefaultControlPropertyPos = ::std::find_if(
            m_aValues.begin(),
            m_aValues.end(),
            EqualName( OUString( "DefaultControl"  ) )
        );
        if ( aDefaultControlPropertyPos != m_aValues.end() )
        {
            OUString sDefaultControl;
            OSL_VERIFY( aDefaultControlPropertyPos->Value >>= sDefaultControl );
            if ( sDefaultControl == "stardiv.one.form.control.Edit" )
            {
                
                
                ::std::copy(
                    aDefaultControlPropertyPos + 1,
                    m_aValues.end(),
                    aDefaultControlPropertyPos
                );
                m_aValues.resize( m_aValues.size() - 1 );
            }
        }
    }

    void OTextLikeImport::EndElement()
    {
        removeRedundantCurrentValue();
        adjustDefaultControlProperty();

        
        OControlImport::EndElement();

        
        UniReference < XMLTextImportHelper > xTextImportHelper( m_rContext.getGlobalContext().GetTextImport() );
        if ( m_xCursor.is() )
        {
            
            
            m_xCursor->gotoEnd( sal_False );
            m_xCursor->goLeft( 1, sal_True );
            m_xCursor->setString( OUString() );

            
            xTextImportHelper->ResetCursor();
        }

        if ( m_xOldCursor.is() )
            xTextImportHelper->SetCursor( m_xOldCursor );

    }

    
    OListAndComboImport::OListAndComboImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
        ,m_nEmptyListItems( 0 )
        ,m_nEmptyValueItems( 0 )
        ,m_bEncounteredLSAttrib( sal_False )
        ,m_bLinkWithIndexes( sal_False )
    {
        if (OControlElement::COMBOBOX == m_eElementType)
            enableTrackAttributes();
    }

    SvXMLImportContext* OListAndComboImport::CreateChildContext(sal_uInt16 _nPrefix, const OUString& _rLocalName,
            const Reference< XAttributeList >& _rxAttrList)
    {
        
        static const OUString s_sOptionElementName("option");
        if (s_sOptionElementName == _rLocalName)
            return new OListOptionImport(GetImport(), _nPrefix, _rLocalName, this);

        
        static const OUString s_sItemElementName("item");
        if (s_sItemElementName == _rLocalName)
            return new OComboItemImport(GetImport(), _nPrefix, _rLocalName, this);

        
        return OControlImport::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
    }

    void OListAndComboImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        m_bLinkWithIndexes = sal_False;

        OControlImport::StartElement(_rxAttrList);

        if (OControlElement::COMBOBOX == m_eElementType)
        {
            
            
            
            simulateDefaultedAttribute( OAttributeMetaData::getSpecialAttributeName( SCA_AUTOMATIC_COMPLETION ), PROPERTY_AUTOCOMPLETE, "false");

            
            simulateDefaultedAttribute( OAttributeMetaData::getDatabaseAttributeName( DA_CONVERT_EMPTY ), PROPERTY_EMPTY_IS_NULL, "false");
        }
    }

    void OListAndComboImport::EndElement()
    {
        
        
        PropertyValue aItemList;
        aItemList.Name = PROPERTY_STRING_ITEM_LIST;
        aItemList.Value <<= m_aListSource;
        implPushBackPropertyValue(aItemList);

        if (OControlElement::LISTBOX == m_eElementType)
        {
            OSL_ENSURE((m_aListSource.getLength() + m_nEmptyListItems) == (m_aValueList.getLength() + m_nEmptyValueItems),
                "OListAndComboImport::EndElement: inconsistence between labels and values!");

            if ( !m_bEncounteredLSAttrib )
            {
                
                PropertyValue aValueList;
                aValueList.Name = PROPERTY_LISTSOURCE;
                aValueList.Value <<= m_aValueList;
                implPushBackPropertyValue(aValueList);
            }

            
            PropertyValue aSelected;
            aSelected.Name = PROPERTY_SELECT_SEQ;
            aSelected.Value <<= m_aSelectedSeq;
            implPushBackPropertyValue(aSelected);

            
            PropertyValue aDefaultSelected;
            aDefaultSelected.Name = PROPERTY_DEFAULT_SELECT_SEQ;
            aDefaultSelected.Value <<= m_aDefaultSelectedSeq;
            implPushBackPropertyValue(aDefaultSelected);
        }

        OControlImport::EndElement();

        
        if ( m_xElement.is() && !m_sCellListSource.isEmpty() )
            m_rContext.registerCellRangeListSource( m_xElement, m_sCellListSource );
    }

    void OListAndComboImport::doRegisterCellValueBinding( const OUString& _rBoundCellAddress )
    {
        OUString sBoundCellAddress( _rBoundCellAddress );
        if ( m_bLinkWithIndexes )
        {
            
            
            
            
            sBoundCellAddress += OUString( ":index"  );
        }

        OControlImport::doRegisterCellValueBinding( sBoundCellAddress );
    }

    bool OListAndComboImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue)
    {
        static const sal_Char* pListSourceAttributeName = OAttributeMetaData::getDatabaseAttributeName(DA_LIST_SOURCE);
        if ( _rLocalName.equalsAscii(pListSourceAttributeName) )
        {
            PropertyValue aListSource;
            aListSource.Name = PROPERTY_LISTSOURCE;

            
            m_bEncounteredLSAttrib = sal_True;
            if ( OControlElement::COMBOBOX == m_eElementType )
            {
                aListSource.Value <<= _rValue;
            }
            else
            {
                
                
                
                Sequence< OUString > aListSourcePropValue( 1 );
                aListSourcePropValue[0] = _rValue;
                aListSource.Value <<= aListSourcePropValue;
            }

            implPushBackPropertyValue( aListSource );
            return true;
        }

        if ( _rLocalName.equalsAscii( OAttributeMetaData::getBindingAttributeName( BA_LIST_CELL_RANGE ) ) )
        {
            m_sCellListSource = _rValue;
            return true;
        }

        if ( _rLocalName.equalsAscii( OAttributeMetaData::getBindingAttributeName( BA_LIST_LINKING_TYPE ) ) )
        {
            sal_Int16 nLinkageType = 0;
            PropertyConversion::convertString(
                m_rContext.getGlobalContext(),
                ::getCppuType( static_cast< sal_Int16* >( NULL ) ),
                _rValue,
                OEnumMapper::getEnumMap( OEnumMapper::epListLinkageType )
            ) >>= nLinkageType;

            m_bLinkWithIndexes = ( nLinkageType != 0 );
            return true;
        }

        return OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    void OListAndComboImport::implPushBackLabel(const OUString& _rLabel)
    {
        OSL_ENSURE(!m_nEmptyListItems, "OListAndComboImport::implPushBackValue: label list is already done!");
        if (!m_nEmptyListItems)
            pushBackSequenceElement(m_aListSource, _rLabel);
    }

    void OListAndComboImport::implPushBackValue(const OUString& _rValue)
    {
        OSL_ENSURE(!m_nEmptyValueItems, "OListAndComboImport::implPushBackValue: value list is already done!");
        if (!m_nEmptyValueItems)
        {
            OSL_ENSURE( !m_bEncounteredLSAttrib, "OListAndComboImport::implPushBackValue: invalid structure! Did you save this document with a version prior SRC641 m?" );
                
                
                
                

            pushBackSequenceElement( m_aValueList, _rValue );
        }
    }

    void OListAndComboImport::implEmptyLabelFound()
    {
        ++m_nEmptyListItems;
    }

    void OListAndComboImport::implEmptyValueFound()
    {
        ++m_nEmptyValueItems;
    }

    void OListAndComboImport::implSelectCurrentItem()
    {
        OSL_ENSURE((m_aListSource.getLength() + m_nEmptyListItems) == (m_aValueList.getLength() + m_nEmptyValueItems),
            "OListAndComboImport::implSelectCurrentItem: inconsistence between labels and values!");

        sal_Int16 nItemNumber = (sal_Int16)(m_aListSource.getLength() - 1 + m_nEmptyListItems);
        pushBackSequenceElement(m_aSelectedSeq, nItemNumber);
    }

    void OListAndComboImport::implDefaultSelectCurrentItem()
    {
        OSL_ENSURE((m_aListSource.getLength() + m_nEmptyListItems) == (m_aValueList.getLength() + m_nEmptyValueItems),
            "OListAndComboImport::implDefaultSelectCurrentItem: inconsistence between labels and values!");

        sal_Int16 nItemNumber = (sal_Int16)(m_aListSource.getLength() - 1 + m_nEmptyListItems);
        pushBackSequenceElement(m_aDefaultSelectedSeq, nItemNumber);
    }

    
    OListOptionImport::OListOptionImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const OUString& _rName,
            const OListAndComboImportRef& _rListBox)
        :SvXMLImportContext(_rImport, _nPrefix, _rName)
        ,m_xListBoxImport(_rListBox)
    {
    }

    void OListOptionImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        
        const SvXMLNamespaceMap& rMap = GetImport().GetNamespaceMap();
        const OUString sLabelAttribute = rMap.GetQNameByKey(
            GetPrefix(), OUString("label"));
        const OUString sValueAttribute = rMap.GetQNameByKey(
            GetPrefix(), OUString("value"));

        
        OUString sValue = _rxAttrList->getValueByName(sLabelAttribute);
        sal_Bool bNonexistentAttribute = sal_False;
        if (sValue.isEmpty())
            if (_rxAttrList->getTypeByName(sLabelAttribute).isEmpty())
                
                bNonexistentAttribute = sal_True;

        if (bNonexistentAttribute)
            m_xListBoxImport->implEmptyLabelFound();
        else
            m_xListBoxImport->implPushBackLabel( sValue );

        
        sValue = _rxAttrList->getValueByName(sValueAttribute);
        bNonexistentAttribute = sal_False;
        if (sValue.isEmpty())
            if (_rxAttrList->getTypeByName(sValueAttribute).isEmpty())
                
                bNonexistentAttribute = sal_True;

        if (bNonexistentAttribute)
            m_xListBoxImport->implEmptyValueFound();
        else
            m_xListBoxImport->implPushBackValue( sValue );

        
        const OUString sSelectedAttribute = rMap.GetQNameByKey(
            GetPrefix(), OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_CURRENT_SELECTED)));
        const OUString sDefaultSelectedAttribute = rMap.GetQNameByKey(
            GetPrefix(), OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_SELECTED)));

        
        bool bSelected(false);
        ::sax::Converter::convertBool(bSelected,
            _rxAttrList->getValueByName(sSelectedAttribute));
        if (bSelected)
            m_xListBoxImport->implSelectCurrentItem();

        
        bool bDefaultSelected(false);
        ::sax::Converter::convertBool(bDefaultSelected,
            _rxAttrList->getValueByName(sDefaultSelectedAttribute));
        if (bDefaultSelected)
            m_xListBoxImport->implDefaultSelectCurrentItem();

        SvXMLImportContext::StartElement(_rxAttrList);
    }

    
    OComboItemImport::OComboItemImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const OUString& _rName,
            const OListAndComboImportRef& _rListBox)
        :SvXMLImportContext(_rImport, _nPrefix, _rName)
        ,m_xListBoxImport(_rListBox)
    {
    }

    void OComboItemImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        const OUString sLabelAttributeName = GetImport().GetNamespaceMap().GetQNameByKey(
            GetPrefix(), OUString::createFromAscii(OAttributeMetaData::getCommonControlAttributeName(CCA_LABEL)));
        m_xListBoxImport->implPushBackLabel(_rxAttrList->getValueByName(sLabelAttributeName));

        SvXMLImportContext::StartElement(_rxAttrList);
    }

    
    OColumnWrapperImport::OColumnWrapperImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :SvXMLImportContext(_rImport.getGlobalContext(), _nPrefix, _rName)
        ,m_xParentContainer(_rxParentContainer)
        ,m_rFormImport(_rImport)
        ,m_rEventManager(_rEventManager)
    {
    }
    SvXMLImportContext* OColumnWrapperImport::CreateChildContext(sal_uInt16 _nPrefix, const OUString& _rLocalName,
        const Reference< XAttributeList >&)
    {
        OControlImport* pReturn = implCreateChildContext(_nPrefix, _rLocalName, OElementNameMap::getElementType(_rLocalName));
        if (pReturn)
        {
            OSL_ENSURE(m_xOwnAttributes.is(), "OColumnWrapperImport::CreateChildContext: had no form:column element!");
            pReturn->addOuterAttributes(m_xOwnAttributes);
        }
        return pReturn;
    }
    void OColumnWrapperImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        OSL_ENSURE(!m_xOwnAttributes.is(), "OColumnWrapperImport::StartElement: aready have the cloned list!");

        
        Reference< XCloneable > xCloneList(_rxAttrList, UNO_QUERY);
        OSL_ENSURE(xCloneList.is(), "OColumnWrapperImport::StartElement: AttributeList not cloneable!");
        if ( xCloneList.is() )
            m_xOwnAttributes = Reference< XAttributeList >(xCloneList->createClone(), UNO_QUERY);
        OSL_ENSURE(m_xOwnAttributes.is(), "OColumnWrapperImport::StartElement: no cloned list!");
    }

    OControlImport* OColumnWrapperImport::implCreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            OControlElement::ElementType _eType)
    {
        OSL_ENSURE( (OControlElement::TEXT == _eType)
                ||  (OControlElement::TEXT_AREA == _eType)
                ||  (OControlElement::FORMATTED_TEXT == _eType)
                ||  (OControlElement::CHECKBOX == _eType)
                ||  (OControlElement::LISTBOX == _eType)
                ||  (OControlElement::COMBOBOX == _eType)
                ||  (OControlElement::TIME == _eType)
                ||  (OControlElement::DATE == _eType),
                "OColumnWrapperImport::implCreateChildContext: invalid or unrecognized sub element!");

        switch (_eType)
        {
            case OControlElement::COMBOBOX:
            case OControlElement::LISTBOX:
                return new OColumnImport<OListAndComboImport>(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType );

            case OControlElement::PASSWORD:
                return new OColumnImport<OPasswordImport>(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType );

            case OControlElement::TEXT:
            case OControlElement::TEXT_AREA:
            case OControlElement::FORMATTED_TEXT:
                return new OColumnImport< OTextLikeImport >( m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType );

            default:
                return new OColumnImport<OControlImport>(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType );
        }
    }

    
    OGridImport::OGridImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OGridImport_Base(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, "column")
    {
        setElementType(_eType);
    }

    SvXMLImportContext* OGridImport::implCreateControlWrapper(sal_uInt16 _nPrefix, const OUString& _rLocalName)
    {
        return new OColumnWrapperImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer);
    }

    
    OFormImport::OFormImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OFormImport_Base(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, "control")
    {
        enableTrackAttributes();
    }

    SvXMLImportContext* OFormImport::CreateChildContext(sal_uInt16 _nPrefix, const OUString& _rLocalName,
        const Reference< XAttributeList >& _rxAttrList)
    {
        if( token::IsXMLToken(_rLocalName, token::XML_FORM) )
            return new OFormImport( m_rFormImport, *this, _nPrefix, _rLocalName,
                                    m_xMeAsContainer);
        else if ( token::IsXMLToken(_rLocalName, token::XML_CONNECTION_RESOURCE) )
            return new OXMLDataSourceImport(GetImport(), _nPrefix, _rLocalName, _rxAttrList,m_xElement);
        else if( (token::IsXMLToken(_rLocalName, token::XML_EVENT_LISTENERS) &&
                 (XML_NAMESPACE_OFFICE == _nPrefix)) ||
                 token::IsXMLToken( _rLocalName, token::XML_PROPERTIES) )
            return OElementImport::CreateChildContext( _nPrefix, _rLocalName,
                                                       _rxAttrList );
        else
            return implCreateChildContext( _nPrefix, _rLocalName,
                        OElementNameMap::getElementType(_rLocalName) );
    }

    void OFormImport::StartElement(const Reference< XAttributeList >& _rxAttrList)
    {
        m_rFormImport.enterEventContext();
        OFormImport_Base::StartElement(_rxAttrList);

        
        simulateDefaultedAttribute(OAttributeMetaData::getCommonControlAttributeName(CCA_TARGET_FRAME), PROPERTY_TARGETFRAME, "_blank");
    }

    void OFormImport::EndElement()
    {
        OFormImport_Base::EndElement();
        m_rFormImport.leaveEventContext();
    }

    SvXMLImportContext* OFormImport::implCreateControlWrapper(sal_uInt16 _nPrefix, const OUString& _rLocalName)
    {
        OSL_ENSURE( !this, "illegal call to OFormImport::implCreateControlWrapper" );
        return new SvXMLImportContext(GetImport(), _nPrefix, _rLocalName );
    }

    bool OFormImport::handleAttribute(sal_uInt16 _nNamespaceKey, const OUString& _rLocalName, const OUString& _rValue)
    {
        
        static const OUString s_sMasterFieldsAttributeName = OUString::createFromAscii(OAttributeMetaData::getFormAttributeName(faMasterFields));
        static const OUString s_sDetailFieldsAttributeName = OUString::createFromAscii(OAttributeMetaData::getFormAttributeName(faDetailFiels));

        if ( s_sMasterFieldsAttributeName == _rLocalName )
        {
            implTranslateStringListProperty(PROPERTY_MASTERFIELDS, _rValue);
            return true;
        }

        if ( s_sDetailFieldsAttributeName == _rLocalName )
        {
            implTranslateStringListProperty(PROPERTY_DETAILFIELDS, _rValue);
            return true;
        }

        return OFormImport_Base::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    void OFormImport::implTranslateStringListProperty(const OUString& _rPropertyName, const OUString& _rValue)
    {
        PropertyValue aProp;
        aProp.Name = _rPropertyName;

        Sequence< OUString > aList;

        
        if (!_rValue.isEmpty())
        {
            
            
            ::std::vector< OUString > aElements;
            
            sal_Int32 nEstimate = 0, nLength = _rValue.getLength();
            const sal_Unicode* pChars = _rValue.getStr();
            for (sal_Int32 i=0; i<nLength; ++i, ++pChars)
                if (*pChars == ',')
                    ++nEstimate;
            aElements.reserve(nEstimate + 1);
                

            sal_Int32 nElementStart = 0;
            sal_Int32 nNextSep = 0;
            sal_Int32 nElementLength;
            OUString sElement;
            do
            {
                
                nNextSep = ::sax::Converter::indexOfComma(
                    _rValue, nElementStart);
                if (-1 == nNextSep)
                    nNextSep = nLength;
                sElement = _rValue.copy(nElementStart, nNextSep - nElementStart);

                nElementLength = sElement.getLength();
                
                OSL_ENSURE( sElement.startsWith("\"") && sElement.endsWith("\""),
                        "OFormImport::implTranslateStringListProperty: invalid quoted element name.");
                sElement = sElement.copy(1, nElementLength - 2);

                aElements.push_back(sElement);

                
                nElementStart = 1 + nNextSep;
            }
            while (nElementStart < nLength);

            OUString *pElements = aElements.empty() ? 0 : &aElements[0];
            aList = Sequence< OUString >(pElements, aElements.size());
        }
        else
        {
            OSL_FAIL("OFormImport::implTranslateStringListProperty: invalid value (empty)!");
        }

        aProp.Value <<= aList;

        
        implPushBackPropertyValue(aProp);
    }
    
    OXMLDataSourceImport::OXMLDataSourceImport(
                    SvXMLImport& _rImport
                    ,sal_uInt16 nPrfx
                    , const OUString& _sLocalName
                    ,const Reference< ::com::sun::star::xml::sax::XAttributeList > & _xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xElement) :
        SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    {
        OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
        const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();

        sal_Int16 nLength = (_xElement.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            OUString sLocalName;
            OUString sAttrName = _xAttrList->getNameByIndex( i );
            sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName, &sLocalName );

            if  (   ( nPrefix == OAttributeMetaData::getCommonControlAttributeNamespace( CCA_TARGET_LOCATION ) )
                &&  ( sLocalName.equalsAscii( OAttributeMetaData::getCommonControlAttributeName( CCA_TARGET_LOCATION ) ) )
                )
            {
                OUString sValue = _xAttrList->getValueByIndex( i );

                INetURLObject aURL(sValue);
                if ( aURL.GetProtocol() == INET_PROT_FILE )
                    _xElement->setPropertyValue(PROPERTY_DATASOURCENAME,makeAny(sValue));
                else
                    _xElement->setPropertyValue(PROPERTY_URL,makeAny(sValue)); 
                break;
            }
        }
    }
    OControlImport* OFormImport::implCreateChildContext(
            sal_uInt16 _nPrefix, const OUString& _rLocalName,
            OControlElement::ElementType _eType )
    {
        switch (_eType)
        {
            case OControlElement::TEXT:
            case OControlElement::TEXT_AREA:
            case OControlElement::FORMATTED_TEXT:
                return new OTextLikeImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::BUTTON:
            case OControlElement::IMAGE:
            case OControlElement::IMAGE_FRAME:
                return new OButtonImport( m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType );

            case OControlElement::COMBOBOX:
            case OControlElement::LISTBOX:
                return new OListAndComboImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::RADIO:
                return new ORadioImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::CHECKBOX:
                return new OImagePositionImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::PASSWORD:
                return new OPasswordImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::FRAME:
            case OControlElement::FIXED_TEXT:
                return new OReferredControlImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::GRID:
                return new OGridImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::VALUERANGE:
                return new OValueRangeImport( m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType );

            default:
                return new OControlImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);
        }
    }

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
