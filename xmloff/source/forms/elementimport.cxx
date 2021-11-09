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

#include "elementimport.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include "strings.hxx"
#include "callbacks.hxx"
#include <xmloff/xmlnamespace.hxx>
#include "eventimport.hxx"
#include <xmloff/txtstyli.hxx>
#include "formenums.hxx"
#include <xmloff/xmltoken.hxx>
#include "gridcolumnproptranslator.hxx"
#include "property_description.hxx"
#include "property_meta_data.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
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
#include <sal/log.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <comphelper/sequence.hxx>

#include <algorithm>

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
    using namespace ::com::sun::star::xml::sax;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::text;
    using namespace ::comphelper;
    using ::com::sun::star::xml::sax::XAttributeList;

#define PROPID_VALUE            1
#define PROPID_CURRENT_VALUE    2
#define PROPID_MIN_VALUE        3
#define PROPID_MAX_VALUE        4

    namespace {

    struct PropertyValueLess
    {
        bool operator()(const PropertyValue& _rLeft, const PropertyValue& _rRight)
        {
            return _rLeft.Name < _rRight.Name;
        }
    };

    }

    //= OElementNameMap
    std::map<sal_Int32, OControlElement::ElementType>  OElementNameMap::s_sElementTranslations2;

    const OControlElement::ElementType& operator ++(OControlElement::ElementType& _e)
    {
        OControlElement::ElementType e = _e;
        sal_Int32 nAsInt = static_cast<sal_Int32>(e);
        _e = static_cast<OControlElement::ElementType>( ++nAsInt );
        return _e;
    }

    OControlElement::ElementType OElementNameMap::getElementType(sal_Int32 nElement)
    {
        if ( s_sElementTranslations2.empty() )
        {   // initialize
            for (ElementType eType=ElementType(0); eType<UNKNOWN; ++eType)
                s_sElementTranslations2[getElementToken(eType)] = eType;
        }
        auto aPos = s_sElementTranslations2.find(nElement & TOKEN_MASK);
        if (s_sElementTranslations2.end() != aPos)
            return aPos->second;

        return UNKNOWN;
    }

    //= OElementImport
    OElementImport::OElementImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer)
        :OPropertyImport(_rImport)
        ,m_rFormImport(_rImport)
        ,m_rEventManager(_rEventManager)
        ,m_pStyleElement( nullptr )
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

    void OElementImport::startFastElement(sal_Int32 nElement, const Reference< css::xml::sax::XFastAttributeList >& _rxAttrList)
    {
        ENTER_LOG_CONTEXT( "xmloff::OElementImport - importing one element" );

        const OUString sControlImplementation = _rxAttrList->getOptionalValue( XML_ELEMENT(FORM, XML_CONTROL_IMPLEMENTATION) );

        // retrieve the service name
        if ( !sControlImplementation.isEmpty() )
        {
            OUString sOOoImplementationName;
            const sal_uInt16 nImplPrefix = GetImport().GetNamespaceMap().GetKeyByAttrValueQName( sControlImplementation, &sOOoImplementationName );
            m_sServiceName = ( nImplPrefix == XML_NAMESPACE_OOO ) ? sOOoImplementationName : sControlImplementation;
        }

        if ( m_sServiceName.isEmpty() )
            m_sServiceName = determineDefaultServiceName();

        // create the object *now*. This allows setting properties in the various handleAttribute methods.
        // (Though currently not all code is migrated to this pattern, most attributes are still handled
        // by remembering the value (via implPushBackPropertyValue), and setting the correct property value
        // later (in OControlImport::StartElement).)
        m_xElement = createElement();
        if ( m_xElement.is() )
            m_xInfo = m_xElement->getPropertySetInfo();

        // call the base class
        OPropertyImport::startFastElement( nElement, _rxAttrList );
    }

    css::uno::Reference< css::xml::sax::XFastContextHandler > OElementImport::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& _rxAttrList )
    {
        if( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
            return new OFormEventsImportContext(m_rFormImport.getGlobalContext(), *this);

        return OPropertyImport::createFastChildContext(nElement, _rxAttrList);
    }

    void OElementImport::endFastElement(sal_Int32 )
    {
        OSL_ENSURE(m_xElement.is(), "OElementImport::EndElement: invalid element created!");
        if (!m_xElement.is())
            return;

        // apply the non-generic properties
        implApplySpecificProperties();

        // set the generic properties
        implApplyGenericProperties();

        // set the style properties
        if ( m_pStyleElement && m_xElement.is() )
        {
            Reference< XPropertySet > xPropTranslation =
                new OGridColumnPropertyTranslator( Reference< XMultiPropertySet >( m_xElement, UNO_QUERY ) );
            const_cast< XMLTextStyleContext* >( m_pStyleElement )->FillPropertySet( xPropTranslation );

            const OUString sNumberStyleName = m_pStyleElement->GetDataStyleName( );
            if ( !sNumberStyleName.isEmpty() )
                // the style also has a number (sub) style
                m_rContext.applyControlNumberStyle( m_xElement, sNumberStyleName );
        }

        // insert the element into the parent container
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

        // set all the properties we collected
#if OSL_DEBUG_LEVEL > 0
        // check if the object has all the properties
        // (We do this in the non-pro version only. Doing it all the time would be too much expensive)
        if ( m_xInfo.is() )
        {
            for ( const auto& rCheck : m_aValues )
            {
                OSL_ENSURE(m_xInfo->hasPropertyByName(rCheck.Name),
                        OStringBuffer("OElementImport::implApplySpecificProperties: read a property (" +
                            OUStringToOString(rCheck.Name, RTL_TEXTENCODING_ASCII_US) +
                            ") which does not exist on the element!").getStr());
            }
        }
#endif

        // set the properties
        const Reference< XMultiPropertySet > xMultiProps(m_xElement, UNO_QUERY);
        bool bSuccess = false;
        if (xMultiProps.is())
        {
            // translate our properties so that the XMultiPropertySet can handle them

            // sort our property value array so that we can use it in a setPropertyValues
            ::std::sort( m_aValues.begin(), m_aValues.end(), PropertyValueLess());

            // the names
            Sequence< OUString > aNames(m_aValues.size());
            OUString* pNames = aNames.getArray();
            // the values
            Sequence< Any > aValues(m_aValues.size());
            Any* pValues = aValues.getArray();
            // copy

            for ( const auto& rPropValues : m_aValues )
            {
                *pNames = rPropValues.Name;
                *pValues = rPropValues.Value;
                ++pNames;
                ++pValues;
            }

            try
            {
                xMultiProps->setPropertyValues(aNames, aValues);
                bSuccess = true;
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("xmloff.forms");
                OSL_FAIL("OElementImport::implApplySpecificProperties: could not set the properties (using the XMultiPropertySet)!");
            }
        }

        if (bSuccess)
            return;

        // no XMultiPropertySet or setting all properties at once failed
        for ( const auto& rPropValues : m_aValues )
        {
            // this try/catch here is expensive, but because this is just a fallback which should normally not be
            // used it's acceptable this way ...
            try
            {
                m_xElement->setPropertyValue(rPropValues.Name, rPropValues.Value);
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("xmloff.forms");
                OSL_FAIL(OStringBuffer("OElementImport::implApplySpecificProperties: could not set the property \"" +
                        OUStringToOString(rPropValues.Name, RTL_TEXTENCODING_ASCII_US) +
                        "\"!").getStr());
            }
        }
    }

    void OElementImport::implApplyGenericProperties()
    {
        if ( m_aGenericValues.empty() )
            return;

        Reference< XPropertyContainer > xDynamicProperties( m_xElement, UNO_QUERY );

        // PropertyValueArray::iterator aEnd = m_aGenericValues.end();
        for ( auto& rPropValues : m_aGenericValues )
        {
            // check property type for numeric types before setting
            // the property
            try
            {
                // if such a property does not yet exist at the element, create it if necessary
                const bool bExistentProperty = m_xInfo->hasPropertyByName( rPropValues.Name );
                if ( !bExistentProperty )
                {
                    if ( !xDynamicProperties.is() )
                    {
                        SAL_WARN( "xmloff", "OElementImport::implApplyGenericProperties: encountered an unknown property ("
                                    << rPropValues.Name << "), but component is no PropertyBag!");
                        continue;
                    }

                    xDynamicProperties->addProperty(
                        rPropValues.Name,
                        PropertyAttribute::BOUND | PropertyAttribute::REMOVABLE,
                        rPropValues.Value
                    );

                    // re-fetch the PropertySetInfo
                    m_xInfo = m_xElement->getPropertySetInfo();
                }

                // determine the type of the value (source for the following conversion)
                TypeClass eValueTypeClass = rPropValues.Value.getValueTypeClass();
                const bool bValueIsSequence = TypeClass_SEQUENCE == eValueTypeClass;
                if ( bValueIsSequence )
                {
                    uno::Type aSimpleType( getSequenceElementType( rPropValues.Value.getValueType() ) );
                    eValueTypeClass = aSimpleType.getTypeClass();
                }

                // determine the type of the property (target for the following conversion)
                const Property aProperty( m_xInfo->getPropertyByName( rPropValues.Name ) );
                TypeClass ePropTypeClass = aProperty.Type.getTypeClass();
                const bool bPropIsSequence = TypeClass_SEQUENCE == ePropTypeClass;
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
                    Sequence< Any > aXMLValueList;
                    rPropValues.Value >>= aXMLValueList;
                    // just skip this part if empty sequence
                    if (!aXMLValueList.getLength())
                        continue;

                    Sequence< sal_Int16 > aPropertyValueList( aXMLValueList.getLength() );

                    SAL_WARN_IF( eValueTypeClass != TypeClass_ANY, "xmloff",
                        "OElementImport::implApplyGenericProperties: only ANYs should have been imported as generic list property!" );
                        // (OPropertyImport should produce only Sequencer< Any >, since it cannot know the real type

                    SAL_WARN_IF( ePropTypeClass != TypeClass_SHORT, "xmloff",
                        "OElementImport::implApplyGenericProperties: conversion to sequences other than 'sequence< short >' not implemented, yet!" );


                    std::transform(std::cbegin(aXMLValueList), std::cend(aXMLValueList), aPropertyValueList.getArray(),
                        [](const Any& rXMLValue) -> sal_Int16 {
                            // only value sequences of numeric types implemented so far.
                            double nVal( 0 );
                            OSL_VERIFY( rXMLValue >>= nVal );
                            return static_cast< sal_Int16 >( nVal );
                        });

                    rPropValues.Value <<= aPropertyValueList;
                }
                else if ( ePropTypeClass != eValueTypeClass )
                {
                    switch ( eValueTypeClass )
                    {
                    case TypeClass_DOUBLE:
                    {
                        double nVal = 0;
                        rPropValues.Value >>= nVal;
                        switch( ePropTypeClass )
                        {
                        case TypeClass_BYTE:
                            rPropValues.Value <<= static_cast< sal_Int8 >( nVal );
                            break;
                        case TypeClass_SHORT:
                            rPropValues.Value <<= static_cast< sal_Int16 >( nVal );
                            break;
                        case TypeClass_UNSIGNED_SHORT:
                            rPropValues.Value <<= static_cast< sal_uInt16 >( nVal );
                            break;
                        case TypeClass_LONG:
                        case TypeClass_ENUM:
                            rPropValues.Value <<= static_cast< sal_Int32 >( nVal );
                            break;
                        case TypeClass_UNSIGNED_LONG:
                            rPropValues.Value <<= static_cast< sal_uInt32 >( nVal );
                            break;
                        case TypeClass_UNSIGNED_HYPER:
                            rPropValues.Value <<= static_cast< sal_uInt64 >( nVal );
                            break;
                        case TypeClass_HYPER:
                            rPropValues.Value <<= static_cast< sal_Int64 >( nVal );
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

                m_xElement->setPropertyValue( rPropValues.Name, rPropValues.Value );
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("xmloff.forms");
                OSL_FAIL(OStringBuffer("OElementImport::EndElement: could not set the property \"" +
                            OUStringToOString(rPropValues.Name, RTL_TEXTENCODING_ASCII_US) +
                            "\"!").getStr());
            }
        }
    }

    OUString OElementImport::implGetDefaultName() const
    {
        // no optimization here. If this method gets called, the XML stream did not contain a name for the
        // element, which is a heavy error. So in this case we don't care for performance
        static constexpr OUStringLiteral sUnnamedName = u"unnamed";
        OSL_ENSURE(m_xParentContainer.is(), "OElementImport::implGetDefaultName: no parent container!");
        if (!m_xParentContainer.is())
            return sUnnamedName;
        Sequence< OUString > aNames = m_xParentContainer->getElementNames();

        for (sal_Int32 i=0; i<32768; ++i)   // the limit is nearly arbitrary...
        {
            // assemble the new name (suggestion)
            OUString sReturn = sUnnamedName + OUString::number(i);
            // check the existence (this is the bad performance part...)
            if (comphelper::findValue(aNames, sReturn) == -1)
                // not found the name
                return sReturn;
        }
        OSL_FAIL("OElementImport::implGetDefaultName: did not find a free name!");
        return sUnnamedName;
    }

    PropertyGroups::const_iterator OElementImport::impl_matchPropertyGroup( const PropertyGroups& i_propertyGroups ) const
    {
        ENSURE_OR_RETURN( m_xInfo.is(), "OElementImport::impl_matchPropertyGroup: no property set info!", i_propertyGroups.end() );

        return std::find_if(i_propertyGroups.cbegin(), i_propertyGroups.cend(), [&](const PropertyDescriptionList& rGroup) {
            return std::all_of(rGroup.cbegin(), rGroup.cend(), [&](const PropertyDescription* prop) {
                return m_xInfo->hasPropertyByName( prop->propertyName );
            });
        });
    }

    bool OElementImport::tryGenericAttribute( sal_Int32 nElement, const OUString& _rValue )
    {
        // the generic approach (which I hope all props will be migrated to, on the medium term): property handlers
        const AttributeDescription attribute( metadata::getAttributeDescription( nElement ) );
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
                if ( !handler )
                {
                    SAL_WARN( "xmloff.forms", "OElementImport::handleAttribute: invalid property handler!" );
                    break;
                }

                PropertyValues aValues;
                for ( const auto& propDesc : rProperties )
                {
                    aValues[ propDesc->propertyId ] = Any();
                }
                if ( handler->getPropertyValues( _rValue, aValues ) )
                {
                    for ( const auto& propDesc : rProperties )
                    {
                        implPushBackPropertyValue( propDesc->propertyName, aValues[ propDesc->propertyId ] );
                    }
                }
            }
            while ( false );

            // handled
            return true;
        }
        return false;
    }

    bool OElementImport::handleAttribute(sal_Int32 nElement, const OUString& _rValue)
    {
        auto nLocal = nElement & TOKEN_MASK;
        if ( nLocal == XML_CONTROL_IMPLEMENTATION )
            // ignore this, it has already been handled in OElementImport::StartElement
            return true;

        if ( nLocal == XML_NAME )
        {
            if ( m_sName.isEmpty() )
                // remember the name for later use in EndElement
                m_sName = _rValue;
            return true;
        }

        // maybe it's the style attribute?
        if ( nLocal == XML_TEXT_STYLE_NAME )
        {
            const SvXMLStyleContext* pStyleContext = m_rContext.getStyleElement( _rValue );
            OSL_ENSURE( pStyleContext, "OElementImport::handleAttribute: do not know the style!" );
            // remember the element for later usage.
            m_pStyleElement = dynamic_cast<const XMLTextStyleContext*>( pStyleContext  );
            return true;
        }

        if ( m_bImplicitGenericAttributeHandling )
            if ( tryGenericAttribute( nElement, _rValue ) )
                return true;

        // let the base class handle it
        return OPropertyImport::handleAttribute( nElement, _rValue);
    }

    Reference< XPropertySet > OElementImport::createElement()
    {
        Reference< XPropertySet > xReturn;
        if (!m_sServiceName.isEmpty())
        {
            Reference< XComponentContext > xContext = m_rFormImport.getGlobalContext().GetComponentContext();
            Reference< XInterface > xPure = xContext->getServiceManager()->createInstanceWithContext(m_sServiceName, xContext);
            OSL_ENSURE(xPure.is(),
                        OStringBuffer("OElementImport::createElement: service factory gave me no object (service name: " +
                            OUStringToOString(m_sServiceName, RTL_TEXTENCODING_ASCII_US) +
                            ")!").getStr());
            xReturn.set(xPure, UNO_QUERY);
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

    void OElementImport::simulateDefaultedAttribute(sal_Int32 nElement, const OUString& _rPropertyName, const char* _pAttributeDefault)
    {
        OSL_ENSURE( m_xInfo.is(), "OPropertyImport::simulateDefaultedAttribute: the component should be more gossipy about it's properties!" );

        if ( !m_xInfo.is() || m_xInfo->hasPropertyByName( _rPropertyName ) )
        {
            if ( !encounteredAttribute( nElement ) )
                OSL_VERIFY( handleAttribute( XML_ELEMENT(FORM, (nElement & TOKEN_MASK)), OUString::createFromAscii( _pAttributeDefault ) ) );
        }
    }

    //= OControlImport
    OControlImport::OControlImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer)
        :OElementImport(_rImport, _rEventManager, _rxParentContainer)
        ,m_eElementType(OControlElement::UNKNOWN)
    {
        disableImplicitGenericAttributeHandling();
    }

    OControlImport::OControlImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OElementImport(_rImport, _rEventManager, _rxParentContainer)
        ,m_eElementType(_eType)
    {
        disableImplicitGenericAttributeHandling();
    }

    OUString OControlImport::determineDefaultServiceName() const
    {
        const char* pServiceName = nullptr;
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
        case OControlElement::VALUERANGE:        pServiceName = "com.sun.star.form.component.ScrollBar"; break;
        case OControlElement::TIME:              pServiceName = "com.sun.star.form.component.TimeField"; break;
        case OControlElement::DATE:              pServiceName = "com.sun.star.form.component.DateField"; break;
        default:                                 break;
        }
        if ( pServiceName != nullptr )
            return OUString::createFromAscii( pServiceName );
        return OUString();
    }

    void OControlImport::addOuterAttributes(const Reference< XFastAttributeList >& _rxOuterAttribs)
    {
        OSL_ENSURE(!m_xOuterAttributes.is(), "OControlImport::addOuterAttributes: already have these attributes!");
        m_xOuterAttributes = _rxOuterAttribs;
    }

    bool OControlImport::handleAttribute(sal_Int32 nElement, const OUString& _rValue)
    {
        static sal_Int32 nLinkedCellAttributeName = OAttributeMetaData::getBindingAttributeToken(BAFlags::LinkedCell);

        if ((nElement & TOKEN_MASK) == XML_ID)
        {   // it's the control id
            if (IsTokenInNamespace(nElement, XML_NAMESPACE_XML))
            {
                m_sControlId = _rValue;
            }
            else if (IsTokenInNamespace(nElement, XML_NAMESPACE_FORM))
            {
                if (m_sControlId.isEmpty())
                {
                    m_sControlId = _rValue;
                }
            }
            return true;
        }

        if ( (nElement & TOKEN_MASK) == nLinkedCellAttributeName )
        {   // it's the address of a spreadsheet cell
            m_sBoundCellAddress = _rValue;
            return true;
        }

        if ( nElement == XML_ELEMENT(XFORMS, XML_BIND ) )
        {
            m_sBindingID = _rValue;
            return true;
        }

        if ( nElement == XML_ELEMENT(FORM, XML_XFORMS_LIST_SOURCE)  )
        {
            m_sListBindingID = _rValue;
            return true;
        }

        if  (   nElement == XML_ELEMENT(FORM, XML_XFORMS_SUBMISSION)
            ||  nElement == XML_ELEMENT(XFORMS, XML_SUBMISSION) )
        {
            m_sSubmissionID = _rValue;
            return true;
        }

        if ( OElementImport::tryGenericAttribute( nElement, _rValue ) )
            return true;

        static const sal_Int32 nValueAttributeName = OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::Value);
        static const sal_Int32 nCurrentValueAttributeName = OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::CurrentValue);
        static const sal_Int32 nMinValueAttributeName = OAttributeMetaData::getSpecialAttributeToken(SCAFlags::MinValue);
        static const sal_Int32 nMaxValueAttributeName = OAttributeMetaData::getSpecialAttributeToken(SCAFlags::MaxValue);
        static const sal_Int32 nRepeatDelayAttributeName = OAttributeMetaData::getSpecialAttributeToken( SCAFlags::RepeatDelay );

        sal_Int32 nHandle = -1;
        if ( (nElement & TOKEN_MASK) == nValueAttributeName )
            nHandle = PROPID_VALUE;
        else if ( (nElement & TOKEN_MASK) == nCurrentValueAttributeName )
            nHandle = PROPID_CURRENT_VALUE;
        else if ( (nElement & TOKEN_MASK) == nMinValueAttributeName )
            nHandle = PROPID_MIN_VALUE;
        else if ( (nElement & TOKEN_MASK) == nMaxValueAttributeName )
            nHandle = PROPID_MAX_VALUE;
        if ( nHandle != -1 )
        {
            // for the moment, simply remember the name and the value
            PropertyValue aProp;
            aProp.Name = SvXMLImport::getNameFromToken(nElement);
            aProp.Handle = nHandle;
            aProp.Value <<= _rValue;
            m_aValueProperties.push_back(aProp);
            return true;
        }

        if ( (nElement & TOKEN_MASK) == nRepeatDelayAttributeName )
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

        return OElementImport::handleAttribute( nElement, _rValue );
    }

    void OControlImport::startFastElement(sal_Int32 nElement, const Reference< css::xml::sax::XFastAttributeList >& _rxAttrList)
    {
        css::uno::Reference< css::xml::sax::XFastAttributeList > xMergedAttributes;
        if( m_xOuterAttributes.is() )
        {
            // merge the attribute lists, our own one
            rtl::Reference<sax_fastparser::FastAttributeList> xMerger(new sax_fastparser::FastAttributeList(_rxAttrList));
            // and the ones of our enclosing element
            xMerger->add(m_xOuterAttributes);
            xMergedAttributes = xMerger.get();
        }
        else
        {
            xMergedAttributes = _rxAttrList;
        }

        // let the base class handle all the attributes
        OElementImport::startFastElement(nElement, xMergedAttributes);

        if ( m_aValueProperties.empty() || !m_xElement.is())
            return;

        // get the property set info
        if (!m_xInfo.is())
        {
            OSL_FAIL("OControlImport::StartElement: no PropertySetInfo!");
            return;
        }

        std::optional<OUString> pValueProperty;
        std::optional<OUString> pCurrentValueProperty;
        std::optional<OUString> pMinValueProperty;
        std::optional<OUString> pMaxValueProperty;

        bool bRetrievedValues = false;
        bool bRetrievedValueLimits = false;

        // get the class id of our element
        sal_Int16 nClassId = FormComponentType::CONTROL;
        m_xElement->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;

        // translate the value properties we collected in handleAttributes
        for ( auto& rValueProps : m_aValueProperties )
        {
            bool bSuccess = false;
            switch (rValueProps.Handle)
            {
                case PROPID_VALUE:
                case PROPID_CURRENT_VALUE:
                {
                    // get the property names
                    if (!bRetrievedValues)
                    {
                        getValuePropertyNames(m_eElementType, nClassId, pCurrentValueProperty, pValueProperty);
                        if ( !pCurrentValueProperty && !pValueProperty )
                        {
                            SAL_WARN( "xmloff.forms", "OControlImport::StartElement: illegal value property names!" );
                            break;
                        }

                        bRetrievedValues = true;
                    }
                    if ( PROPID_VALUE == rValueProps.Handle && !pValueProperty )
                    {
                        SAL_WARN( "xmloff.forms", "OControlImport::StartElement: the control does not have a value property!");
                        break;
                    }

                    if ( PROPID_CURRENT_VALUE == rValueProps.Handle && !pCurrentValueProperty )
                    {
                        SAL_WARN( "xmloff.forms", "OControlImport::StartElement: the control does not have a current-value property!");
                        break;
                    }

                    // transfer the name
                    if (PROPID_VALUE == rValueProps.Handle)
                        rValueProps.Name = *pValueProperty;
                    else
                        rValueProps.Name = *pCurrentValueProperty;
                    bSuccess = true;
                }
                break;
                case PROPID_MIN_VALUE:
                case PROPID_MAX_VALUE:
                {
                    // get the property names
                    if (!bRetrievedValueLimits)
                    {
                        getValueLimitPropertyNames(nClassId, pMinValueProperty, pMaxValueProperty);
                        if ( !pMinValueProperty || !pMaxValueProperty )
                        {
                            SAL_WARN( "xmloff.forms", "OControlImport::StartElement: illegal value limit property names!" );
                            break;
                        }

                        bRetrievedValueLimits = true;
                    }
                    OSL_ENSURE((PROPID_MIN_VALUE != rValueProps.Handle) || pMinValueProperty,
                        "OControlImport::StartElement: the control does not have a value property!");
                    OSL_ENSURE((PROPID_MAX_VALUE != rValueProps.Handle) || pMaxValueProperty,
                        "OControlImport::StartElement: the control does not have a current-value property!");

                    // transfer the name
                    if (PROPID_MIN_VALUE == rValueProps.Handle)
                        rValueProps.Name = *pMinValueProperty;
                    else
                        rValueProps.Name = *pMaxValueProperty;
                    bSuccess = true;
                }
                break;
            }

            if ( !bSuccess )
                continue;

            // translate the value
            implTranslateValueProperty(m_xInfo, rValueProps);
            // add the property to the base class' array
            implPushBackPropertyValue(rValueProps);
        }

    }

    void OControlImport::implTranslateValueProperty(const Reference< XPropertySetInfo >& _rxPropInfo,
        PropertyValue& _rPropValue)
    {
        OSL_ENSURE(_rxPropInfo->hasPropertyByName(_rPropValue.Name),
            "OControlImport::implTranslateValueProperty: invalid property name!");

        // retrieve the type of the property
        Property aProp = _rxPropInfo->getPropertyByName(_rPropValue.Name);
        // the untranslated string value as read in handleAttribute
        OUString sValue;
        bool bSuccess = _rPropValue.Value >>= sValue;
        OSL_ENSURE(bSuccess, "OControlImport::implTranslateValueProperty: supposed to be called with non-translated string values!");

        if (TypeClass_ANY == aProp.Type.getTypeClass())
        {
            // we have exactly 2 properties where this type class is allowed:
            SAL_WARN_IF(
                    _rPropValue.Name != PROPERTY_EFFECTIVE_VALUE
                &&  _rPropValue.Name != PROPERTY_EFFECTIVE_DEFAULT, "xmloff",
                "OControlImport::implTranslateValueProperty: invalid property type/name combination, Any and " << _rPropValue.Name);

            // Both properties are allowed to have a double or a string value,
            // so first try to convert the string into a number
            double nValue;
            if (::sax::Converter::convertDouble(nValue, sValue))
                _rPropValue.Value <<= nValue;
            else
                _rPropValue.Value <<= sValue;
        }
        else
            _rPropValue.Value = PropertyConversion::convertString(aProp.Type, sValue);
    }

    void OControlImport::endFastElement(sal_Int32 nElement)
    {
        OSL_ENSURE(m_xElement.is(), "OControlImport::EndElement: invalid control!");
        if ( !m_xElement.is() )
            return;

        // register our control with its id
        if (!m_sControlId.isEmpty())
            m_rFormImport.registerControlId(m_xElement, m_sControlId);
        // it's allowed to have no control id. In this case we're importing a column

        // one more pre-work to do:
        // when we set default values, then by definition the respective value is set
        // to this default value, too. This means if the sequence contains for example
        // a DefaultText value, then the Text will be affected by this, too.
        // In case the Text is not part of the property sequence (or occurs _before_
        // the DefaultText, which can happen for other value/default-value property names),
        // this means that the Text (the value property) is incorrectly imported.

        bool bRestoreValuePropertyValue = false;
        Any aValuePropertyValue;

        sal_Int16 nClassId = FormComponentType::CONTROL;
        try
        {
            // get the class id of our element
            m_xElement->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION("xmloff.forms",
                                 "caught an exception while retrieving the class id!");
        }

        std::optional<OUString> pValueProperty;
        std::optional<OUString> pDefaultValueProperty;
        getRuntimeValuePropertyNames(m_eElementType, nClassId, pValueProperty, pDefaultValueProperty);
        if ( pDefaultValueProperty && pValueProperty )
        {
            bool bNonDefaultValuePropertyValue = false;
                // is the "value property" part of the sequence?

            // look up this property in our sequence
            for ( const auto& rCheck : m_aValues )
            {
                if ( rCheck.Name == *pDefaultValueProperty )
                    bRestoreValuePropertyValue = true;
                else if ( rCheck.Name == *pValueProperty )
                {
                    bNonDefaultValuePropertyValue = true;
                    // we need to restore the value property we found here, nothing else
                    aValuePropertyValue = rCheck.Value;
                }
            }

            if ( bRestoreValuePropertyValue && !bNonDefaultValuePropertyValue )
            {
                // found it -> need to remember (and restore) the "value property value", which is not set explicitly
                try
                {
                    aValuePropertyValue = m_xElement->getPropertyValue( *pValueProperty );
                }
                catch( const Exception& )
                {
                    TOOLS_WARN_EXCEPTION(
                        "xmloff.forms",
                        "caught an exception while retrieving the current value property!");
                }
            }
        }

        // let the base class set all the values
        OElementImport::endFastElement(nElement);

        // restore the "value property value", if necessary
        if ( bRestoreValuePropertyValue && pValueProperty )
        {
            try
            {
                m_xElement->setPropertyValue( *pValueProperty, aValuePropertyValue );
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION("xmloff.forms",
                                     "caught an exception while restoring the value property!");
            }
        }

        // the external cell binding, if applicable
        if ( m_xElement.is() && !m_sBoundCellAddress.isEmpty() )
            doRegisterCellValueBinding( m_sBoundCellAddress );

        // XForms binding, if applicable
        if ( m_xElement.is() && !m_sBindingID.isEmpty() )
            doRegisterXFormsValueBinding( m_sBindingID );

        // XForms list binding, if applicable
        if ( m_xElement.is() && !m_sListBindingID.isEmpty() )
            doRegisterXFormsListBinding( m_sListBindingID );

        // XForms submission, if applicable
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

    //= OImagePositionImport
    OImagePositionImport::OImagePositionImport( OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
        const Reference< XNameContainer >& _rxParentContainer,
        OControlElement::ElementType _eType )
        :OControlImport( _rImport, _rEventManager, _rxParentContainer, _eType )
        ,m_nImagePosition( -1 )
        ,m_nImageAlign( 0 )
        ,m_bHaveImagePosition( false )
    {
    }

    bool OImagePositionImport::handleAttribute( sal_Int32 nElement,
        const OUString& _rValue )
    {
        static const sal_Int32 s_nImageDataAttributeName = OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::ImageData);

        if ( (nElement & TOKEN_MASK) == s_nImageDataAttributeName)
        {
            m_xGraphic = m_rContext.getGlobalContext().loadGraphicByURL(_rValue);
            return true;
        }
        else if ( (nElement & TOKEN_MASK) == XML_IMAGE_POSITION )
        {
            OSL_VERIFY( PropertyConversion::convertString(
                cppu::UnoType<decltype(m_nImagePosition)>::get(),
                _rValue, aImagePositionMap
            ) >>= m_nImagePosition );
            m_bHaveImagePosition = true;
            return true;
        }
        else if ( (nElement & TOKEN_MASK) == XML_IMAGE_ALIGN )
        {
            OSL_VERIFY( PropertyConversion::convertString(
                cppu::UnoType<decltype(m_nImageAlign)>::get(),
                _rValue, aImageAlignMap
            ) >>= m_nImageAlign );
            return true;
        }

        return OControlImport::handleAttribute( nElement, _rValue );
    }

    void OImagePositionImport::startFastElement(sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList)
    {
        OControlImport::startFastElement( nElement, _rxAttrList );

        if (m_xGraphic.is())
        {
            PropertyValue aGraphicProperty;
            aGraphicProperty.Name = PROPERTY_GRAPHIC;
            aGraphicProperty.Value <<= m_xGraphic;
            implPushBackPropertyValue(aGraphicProperty);
        }
        if ( !m_bHaveImagePosition )
            return;

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

    //= OReferredControlImport
    OReferredControlImport::OReferredControlImport(
            OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer )
        :OControlImport(_rImport, _rEventManager, _rxParentContainer)
    {
    }

    void OReferredControlImport::startFastElement(sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList)
    {
        OControlImport::startFastElement(nElement, _rxAttrList);

        // the base class should have created the control, so we can register it
        if ( !m_sReferringControls.isEmpty() )
            m_rFormImport.registerControlReferences(m_xElement, m_sReferringControls);
    }

    bool OReferredControlImport::handleAttribute(sal_Int32 nElement,
        const OUString& _rValue)
    {
        static const sal_Int32 s_nReferenceAttributeName = OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::For);
        if ((nElement & TOKEN_MASK) == s_nReferenceAttributeName)
        {
            m_sReferringControls = _rValue;
            return true;
        }
        return OControlImport::handleAttribute(nElement, _rValue);
    }

    //= OPasswordImport
    OPasswordImport::OPasswordImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _rxParentContainer, _eType)
    {
    }

    bool OPasswordImport::handleAttribute(sal_Int32 nElement, const OUString& _rValue)
    {
        static const sal_Int32 s_nEchoCharAttributeName = OAttributeMetaData::getSpecialAttributeToken(SCAFlags::EchoChar);
        if ((nElement & TOKEN_MASK) == s_nEchoCharAttributeName)
        {
            // need a special handling for the EchoChar property
            PropertyValue aEchoChar;
            aEchoChar.Name = PROPERTY_ECHOCHAR;
            OSL_ENSURE(_rValue.getLength() == 1, "OPasswordImport::handleAttribute: invalid echo char attribute!");
                // we ourself should not have written values other than of length 1
            if (_rValue.getLength() >= 1)
                aEchoChar.Value <<= static_cast<sal_Int16>(_rValue[0]);
            else
                aEchoChar.Value <<= sal_Int16(0);
            implPushBackPropertyValue(aEchoChar);
            return true;
        }
        return OControlImport::handleAttribute(nElement, _rValue);
    }

    //= ORadioImport
    ORadioImport::ORadioImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OImagePositionImport( _rImport, _rEventManager, _rxParentContainer, _eType )
    {
    }

    bool ORadioImport::handleAttribute(sal_Int32 nElement, const OUString& _rValue)
    {
        // need special handling for the State & CurrentState properties:
        // they're stored as booleans, but expected to be int16 properties
        static const sal_Int32 nCurrentSelectedAttributeName = OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::CurrentSelected);
        static const sal_Int32 nSelectedAttributeName = OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::Selected);
        if  (  (nElement & TOKEN_MASK) == nCurrentSelectedAttributeName
            || (nElement & TOKEN_MASK) == nSelectedAttributeName
            )
        {
            const OAttribute2Property::AttributeAssignment* pProperty = m_rContext.getAttributeMap().getAttributeTranslation(nElement & TOKEN_MASK);
            assert(pProperty && "ORadioImport::handleAttribute: invalid property map!");
            if (pProperty)
            {
                const Any aBooleanValue( PropertyConversion::convertString(pProperty->aPropertyType, _rValue, pProperty->pEnumMap) );

                // create and store a new PropertyValue
                PropertyValue aNewValue;
                aNewValue.Name = pProperty->sPropertyName;
                aNewValue.Value <<= static_cast<sal_Int16>(::cppu::any2bool(aBooleanValue));

                implPushBackPropertyValue(aNewValue);
            }
            return true;
        }
        return OImagePositionImport::handleAttribute( nElement, _rValue );
    }

    //= OURLReferenceImport
    OURLReferenceImport::OURLReferenceImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OImagePositionImport(_rImport, _rEventManager, _rxParentContainer, _eType)
    {
    }

    bool OURLReferenceImport::handleAttribute(sal_Int32 nElement, const OUString& _rValue)
    {
        static const sal_Int32 s_nTargetLocationAttributeName   = OAttributeMetaData::getCommonControlAttributeToken( CCAFlags::TargetLocation );
        static const sal_Int32 s_nImageDataAttributeName        = OAttributeMetaData::getCommonControlAttributeToken( CCAFlags::ImageData );

        // need to make the URL absolute if
        // * it's the image-data attribute
        // * it's the target-location attribute, and we're dealing with an object which has the respective property
        bool bMakeAbsolute =
                (nElement & TOKEN_MASK) == s_nImageDataAttributeName
            ||  (   (nElement & TOKEN_MASK) == s_nTargetLocationAttributeName
                &&  (   ( OControlElement::BUTTON == m_eElementType )
                    ||  ( OControlElement::IMAGE == m_eElementType )
                    )
                );

        if (bMakeAbsolute && !_rValue.isEmpty())
        {
            OUString sAdjustedValue = _rValue;
            if ((nElement & TOKEN_MASK) != s_nImageDataAttributeName)
                sAdjustedValue = m_rContext.getGlobalContext().GetAbsoluteReference( _rValue );
            return OImagePositionImport::handleAttribute( nElement, sAdjustedValue );
        }

        return OImagePositionImport::handleAttribute( nElement, _rValue );
    }

    //= OButtonImport
    OButtonImport::OButtonImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OURLReferenceImport(_rImport, _rEventManager, _rxParentContainer, _eType)
    {
        enableTrackAttributes();
    }

    void OButtonImport::startFastElement(sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList)
    {
        OURLReferenceImport::startFastElement(nElement, _rxAttrList);

        // handle the target-frame attribute
        simulateDefaultedAttribute(OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::TargetFrame), PROPERTY_TARGETFRAME, "_blank");
    }

    //= OValueRangeImport
    OValueRangeImport::OValueRangeImport( OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType )
        :OControlImport( _rImport, _rEventManager, _rxParentContainer, _eType )
        ,m_nStepSizeValue( 1 )
    {

    }

    bool OValueRangeImport::handleAttribute( sal_Int32 nElement, const OUString& _rValue )
    {
        if ( (nElement & TOKEN_MASK) == OAttributeMetaData::getSpecialAttributeToken( SCAFlags::StepSize ) )
        {
            ::sax::Converter::convertNumber( m_nStepSizeValue, _rValue );
            return true;
        }
        return OControlImport::handleAttribute( nElement, _rValue );
    }

    void OValueRangeImport::startFastElement( sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList )
    {
        OControlImport::startFastElement( nElement, _rxAttrList );

        if ( m_xInfo.is() )
        {
            if ( m_xInfo->hasPropertyByName( PROPERTY_SPIN_INCREMENT ) )
                m_xElement->setPropertyValue( PROPERTY_SPIN_INCREMENT, makeAny( m_nStepSizeValue ) );
            else if ( m_xInfo->hasPropertyByName( PROPERTY_LINE_INCREMENT ) )
                m_xElement->setPropertyValue( PROPERTY_LINE_INCREMENT, makeAny( m_nStepSizeValue ) );
        }
    }

    //= OTextLikeImport
    OTextLikeImport::OTextLikeImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _rxParentContainer, _eType)
        ,m_bEncounteredTextPara( false )
    {
        enableTrackAttributes();
    }

    css::uno::Reference< css::xml::sax::XFastContextHandler > OTextLikeImport::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
    {
        if ( nElement == XML_ELEMENT(TEXT, XML_P) )
        {
            OSL_ENSURE( m_eElementType == OControlElement::TEXT_AREA,
                "OTextLikeImport::CreateChildContext: text paragraphs in a non-text-area?" );

            if ( m_eElementType == OControlElement::TEXT_AREA )
            {
                Reference< XText > xTextElement( m_xElement, UNO_QUERY );
                if ( xTextElement.is() )
                {
                    rtl::Reference < XMLTextImportHelper > xTextImportHelper( m_rContext.getGlobalContext().GetTextImport() );

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
                        return xTextImportHelper->CreateTextChildContext( m_rContext.getGlobalContext(), nElement, xAttrList );
                    }
                }
                else
                {
                    // in theory, we could accumulate all the text portions (without formatting),
                    // and set it as Text property at the model ...
                }
            }
        }

        return OControlImport::createFastChildContext( nElement, xAttrList );
    }

    void OTextLikeImport::startFastElement(sal_Int32 nElement, const Reference< css::xml::sax::XFastAttributeList >& _rxAttrList)
    {
        OControlImport::startFastElement(nElement, _rxAttrList);

        // handle the convert-empty-to-null attribute, whose default is different from the property default
        // unfortunately, different classes are imported by this class ('cause they're represented by the
        // same XML element), though not all of them know this property.
        // So we have to do a check ...
        if (m_xElement.is() && m_xInfo.is() && m_xInfo->hasPropertyByName(PROPERTY_EMPTY_IS_NULL) )
            simulateDefaultedAttribute(OAttributeMetaData::getDatabaseAttributeToken(DAFlags::ConvertEmpty), PROPERTY_EMPTY_IS_NULL, "false");
    }

    namespace {

    struct EqualHandle
    {
        const sal_Int32 m_nHandle;
        explicit EqualHandle( sal_Int32 _nHandle ) : m_nHandle( _nHandle ) { }

        bool operator()( const PropertyValue& _rProp )
        {
            return _rProp.Handle == m_nHandle;
        }
    };

    }

    void OTextLikeImport::removeRedundantCurrentValue()
    {
        if ( !m_bEncounteredTextPara )
            return;

        // In case the text is written in the text:p elements, we need to ignore what we read as
        // current-value attribute, since it's redundant.
        // fortunately, OElementImport tagged the value property with the PROPID_CURRENT_VALUE
        // handle, so we do not need to determine the name of our value property here
        // (normally, it should be "Text", since no other controls than the edit field should
        // have the text:p elements)
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
                m_aValues.erase(aValuePropertyPos);
            }
        }

        // additionally, we need to set the "RichText" property of our element to sal_True
        // (the presence of the text:p is used as indicator for the value of the RichText property)
        bool bHasRichTextProperty = false;
        if ( m_xInfo.is() )
            bHasRichTextProperty = m_xInfo->hasPropertyByName( PROPERTY_RICH_TEXT );
        OSL_ENSURE( bHasRichTextProperty, "OTextLikeImport::EndElement: text:p, but no rich text control?" );
        if ( bHasRichTextProperty )
            m_xElement->setPropertyValue( PROPERTY_RICH_TEXT, makeAny( true ) );
        // Note that we do *not* set the RichText property (in case our element has one) to sal_False here
        // since this is the default of this property, anyway.
    }

    namespace {

    struct EqualName
    {
        const OUString & m_sName;
        explicit EqualName( const OUString& _rName ) : m_sName( _rName ) { }

        bool operator()( const PropertyValue& _rProp )
        {
            return _rProp.Name == m_sName;
        }
    };

    }

    void OTextLikeImport::adjustDefaultControlProperty()
    {
        // In OpenOffice.org 2.0, we changed the implementation of the css.form.component.TextField (the model of a text field control),
        // so that it now uses another default control. So if we encounter a text field where the *old* default
        // control property is writing, we are not allowed to use it
        PropertyValueArray::iterator aDefaultControlPropertyPos = ::std::find_if(
            m_aValues.begin(),
            m_aValues.end(),
            EqualName( "DefaultControl" )
        );
        if ( aDefaultControlPropertyPos != m_aValues.end() )
        {
            OUString sDefaultControl;
            OSL_VERIFY( aDefaultControlPropertyPos->Value >>= sDefaultControl );
            if ( sDefaultControl == "stardiv.one.form.control.Edit" )
            {
                // complete remove this property value from the array. Today's "default value" of the "DefaultControl"
                // property is sufficient
                m_aValues.erase(aDefaultControlPropertyPos);
            }
        }
    }

    void OTextLikeImport::endFastElement(sal_Int32 nElement)
    {
        removeRedundantCurrentValue();
        adjustDefaultControlProperty();

        // let the base class do the stuff
        OControlImport::endFastElement(nElement);

        // some cleanups
        rtl::Reference < XMLTextImportHelper > xTextImportHelper( m_rContext.getGlobalContext().GetTextImport() );
        if ( m_xCursor.is() )
        {
            // delete the newline which has been imported erroneously
            // TODO (fs): stole this code somewhere - why don't we fix the text import??
            m_xCursor->gotoEnd( false );
            m_xCursor->goLeft( 1, true );
            m_xCursor->setString( OUString() );

            // reset cursor
            xTextImportHelper->ResetCursor();
        }

        if ( m_xOldCursor.is() )
            xTextImportHelper->SetCursor( m_xOldCursor );

    }

    //= OListAndComboImport
    OListAndComboImport::OListAndComboImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _rxParentContainer, _eType)
        ,m_nEmptyListItems( 0 )
        ,m_nEmptyValueItems( 0 )
        ,m_bEncounteredLSAttrib( false )
        ,m_bLinkWithIndexes( false )
    {
        if (OControlElement::COMBOBOX == m_eElementType)
            enableTrackAttributes();
    }

    css::uno::Reference< css::xml::sax::XFastContextHandler > OListAndComboImport::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& _rxAttrList )
    {
        // is it the "option" sub tag of a listbox ?
        if ((nElement & TOKEN_MASK) == XML_OPTION)
            return new OListOptionImport(GetImport(), this);

        // is it the "item" sub tag of a combobox ?
        if ((nElement & TOKEN_MASK) == XML_ITEM)
            return new OComboItemImport(GetImport(), this);

        // everything else
        return OControlImport::createFastChildContext(nElement, _rxAttrList);
    }

    void OListAndComboImport::startFastElement(sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList)
    {
        m_bLinkWithIndexes = false;

        OControlImport::startFastElement(nElement, _rxAttrList);

        if (OControlElement::COMBOBOX == m_eElementType)
        {
            // for the auto-completion
            // the attribute default does not equal the property default, so in case we did not read this attribute,
            // we have to simulate it
            simulateDefaultedAttribute( OAttributeMetaData::getSpecialAttributeToken( SCAFlags::AutoCompletion ), PROPERTY_AUTOCOMPLETE, "false");

            // same for the convert-empty-to-null attribute, which's default is different from the property default
            simulateDefaultedAttribute( OAttributeMetaData::getDatabaseAttributeToken( DAFlags::ConvertEmpty ), PROPERTY_EMPTY_IS_NULL, "false");
        }
    }

    void OListAndComboImport::endFastElement(sal_Int32 nElement)
    {
        // append the list source property the properties sequence of our importer
        // the string item list
        PropertyValue aItemList;
        aItemList.Name = PROPERTY_STRING_ITEM_LIST;
        aItemList.Value <<= comphelper::containerToSequence(m_aListSource);
        implPushBackPropertyValue(aItemList);

        if (OControlElement::LISTBOX == m_eElementType)
        {
            OSL_ENSURE((m_aListSource.size() + m_nEmptyListItems) == (m_aValueList.size() + m_nEmptyValueItems),
                "OListAndComboImport::EndElement: inconsistence between labels and values!");

            if ( !m_bEncounteredLSAttrib )
            {
                // the value sequence
                PropertyValue aValueList;
                aValueList.Name = PROPERTY_LISTSOURCE;
                aValueList.Value <<= comphelper::containerToSequence(m_aValueList);
                implPushBackPropertyValue(aValueList);
            }

            // the select sequence
            PropertyValue aSelected;
            aSelected.Name = PROPERTY_SELECT_SEQ;
            aSelected.Value <<= comphelper::containerToSequence(m_aSelectedSeq);
            implPushBackPropertyValue(aSelected);

            // the default select sequence
            PropertyValue aDefaultSelected;
            aDefaultSelected.Name = PROPERTY_DEFAULT_SELECT_SEQ;
            aDefaultSelected.Value <<= comphelper::containerToSequence(m_aDefaultSelectedSeq);
            implPushBackPropertyValue(aDefaultSelected);
        }

        OControlImport::endFastElement(nElement);

        // the external list source, if applicable
        if ( m_xElement.is() && !m_sCellListSource.isEmpty() )
            m_rContext.registerCellRangeListSource( m_xElement, m_sCellListSource );
    }

    void OListAndComboImport::doRegisterCellValueBinding( const OUString& _rBoundCellAddress )
    {
        OUString sBoundCellAddress( _rBoundCellAddress );
        if ( m_bLinkWithIndexes )
        {
            // This is a HACK. We register a string which is no valid address, but allows
            // (somewhere else) to determine that a non-standard binding should be created.
            // This hack is acceptable for OOo 1.1.1, since the file format for value
            // bindings of form controls is to be changed afterwards, anyway.
            sBoundCellAddress += ":index";
        }

        OControlImport::doRegisterCellValueBinding( sBoundCellAddress );
    }

    bool OListAndComboImport::handleAttribute(sal_Int32 nElement, const OUString& _rValue)
    {
        static const sal_Int32 nListSourceAttributeName = OAttributeMetaData::getDatabaseAttributeToken(DAFlags::ListSource);
        if ( (nElement & TOKEN_MASK) == nListSourceAttributeName )
        {
            PropertyValue aListSource;
            aListSource.Name = PROPERTY_LISTSOURCE;

            // it's the ListSource attribute
            m_bEncounteredLSAttrib = true;
            if ( OControlElement::COMBOBOX == m_eElementType )
            {
                aListSource.Value <<= _rValue;
            }
            else
            {
                // a listbox which has a list-source attribute must have a list-source-type of something
                // not equal to ValueList.
                // In this case, the list-source value is simply the one and only element of the ListSource property.
                Sequence<OUString> aListSourcePropValue { _rValue };
                aListSource.Value <<= aListSourcePropValue;
            }

            implPushBackPropertyValue( aListSource );
            return true;
        }

        if ( (nElement & TOKEN_MASK) == OAttributeMetaData::getBindingAttributeToken( BAFlags::ListCellRange ) )
        {
            m_sCellListSource = _rValue;
            return true;
        }

        if ( (nElement & TOKEN_MASK) == OAttributeMetaData::getBindingAttributeToken( BAFlags::ListLinkingType ) )
        {
            sal_Int16 nLinkageType = 0;
            PropertyConversion::convertString(
                ::cppu::UnoType<sal_Int16>::get(),
                _rValue,
                aListLinkageMap
            ) >>= nLinkageType;

            m_bLinkWithIndexes = ( nLinkageType != 0 );
            return true;
        }

        return OControlImport::handleAttribute(nElement, _rValue);
    }

    void OListAndComboImport::implPushBackLabel(const OUString& _rLabel)
    {
        OSL_ENSURE(!m_nEmptyListItems, "OListAndComboImport::implPushBackValue: label list is already done!");
        if (!m_nEmptyListItems)
            m_aListSource.push_back(_rLabel);
    }

    void OListAndComboImport::implPushBackValue(const OUString& _rValue)
    {
        OSL_ENSURE(!m_nEmptyValueItems, "OListAndComboImport::implPushBackValue: value list is already done!");
        if (!m_nEmptyValueItems)
        {
            OSL_ENSURE( !m_bEncounteredLSAttrib, "OListAndComboImport::implPushBackValue: invalid structure! Did you save this document with a version prior SRC641 m?" );
                // We already had the list-source attribute, which means that the ListSourceType is
                // not ValueList, which means that the ListSource should contain only one string in
                // the first element of the sequence
                // All other values in the file are invalid

            m_aValueList.push_back( _rValue );
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
        OSL_ENSURE((m_aListSource.size() + m_nEmptyListItems) == (m_aValueList.size() + m_nEmptyValueItems),
            "OListAndComboImport::implSelectCurrentItem: inconsistence between labels and values!");

        sal_Int16 nItemNumber = static_cast<sal_Int16>(m_aListSource.size() - 1 + m_nEmptyListItems);
        m_aSelectedSeq.push_back(nItemNumber);
    }

    void OListAndComboImport::implDefaultSelectCurrentItem()
    {
        OSL_ENSURE((m_aListSource.size() + m_nEmptyListItems) == (m_aValueList.size() + m_nEmptyValueItems),
            "OListAndComboImport::implDefaultSelectCurrentItem: inconsistence between labels and values!");

        sal_Int16 nItemNumber = static_cast<sal_Int16>(m_aListSource.size() - 1 + m_nEmptyListItems);
        m_aDefaultSelectedSeq.push_back(nItemNumber);
    }

    //= OListOptionImport
    OListOptionImport::OListOptionImport(SvXMLImport& _rImport,
            const OListAndComboImportRef& _rListBox)
        :SvXMLImportContext(_rImport)
        ,m_xListBoxImport(_rListBox)
    {
    }

    void OListOptionImport::startFastElement(sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList)
    {
        // the label and the value
        const sal_Int32 nLabelAttribute = (nElement & ~TOKEN_MASK) | XML_LABEL;
        const sal_Int32 nValueAttribute = (nElement & ~TOKEN_MASK) | XML_VALUE;

        // the label attribute
        OUString sValue = _rxAttrList->getOptionalValue(nLabelAttribute);
        bool bNonexistentAttribute = !_rxAttrList->hasAttribute(nLabelAttribute);

        if (bNonexistentAttribute)
            m_xListBoxImport->implEmptyLabelFound();
        else
            m_xListBoxImport->implPushBackLabel( sValue );

        // the value attribute
        sValue = _rxAttrList->getOptionalValue(nValueAttribute);
        bNonexistentAttribute = !_rxAttrList->hasAttribute(nValueAttribute);

        if (bNonexistentAttribute)
            m_xListBoxImport->implEmptyValueFound();
        else
            m_xListBoxImport->implPushBackValue( sValue );

        // the current-selected and selected
        const sal_Int32 nSelectedAttribute = (nElement & ~TOKEN_MASK) | OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::CurrentSelected);
        const sal_Int32 nDefaultSelectedAttribute = (nElement & ~TOKEN_MASK) | OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::Selected);

        // propagate the selected flag
        bool bSelected(false);
        (void)::sax::Converter::convertBool(bSelected,
            _rxAttrList->getOptionalValue(nSelectedAttribute));
        if (bSelected)
            m_xListBoxImport->implSelectCurrentItem();

        // same for the default selected
        bool bDefaultSelected(false);
        (void)::sax::Converter::convertBool(bDefaultSelected,
            _rxAttrList->getOptionalValue(nDefaultSelectedAttribute));
        if (bDefaultSelected)
            m_xListBoxImport->implDefaultSelectCurrentItem();
    }

    //= OComboItemImport
    OComboItemImport::OComboItemImport(SvXMLImport& _rImport,
            const OListAndComboImportRef& _rListBox)
        :SvXMLImportContext(_rImport)
        ,m_xListBoxImport(_rListBox)
    {
    }

    void OComboItemImport::startFastElement(sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList)
    {
        const sal_Int32 nLabelAttributeName = (nElement & ~TOKEN_MASK) |
            OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::Label);
        m_xListBoxImport->implPushBackLabel(_rxAttrList->getOptionalValue(nLabelAttributeName));
    }

    //= OColumnWrapperImport
    OColumnWrapperImport::OColumnWrapperImport(OFormLayerXMLImport_Impl& _rImport,
            IEventAttacherManager& _rEventManager, sal_Int32 /*nElement*/,
            const Reference< XNameContainer >& _rxParentContainer)
        :SvXMLImportContext(_rImport.getGlobalContext())
        ,m_xParentContainer(_rxParentContainer)
        ,m_rFormImport(_rImport)
        ,m_rEventManager(_rEventManager)
    {
    }
    css::uno::Reference< css::xml::sax::XFastContextHandler > OColumnWrapperImport::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
    {
        OControlImport* pReturn = implCreateChildContext(nElement, OElementNameMap::getElementType(nElement & TOKEN_MASK));
        if (pReturn)
        {
            OSL_ENSURE(m_xOwnAttributes.is(), "OColumnWrapperImport::CreateChildContext: had no form:column element!");
            pReturn->addOuterAttributes(m_xOwnAttributes);
        }
        return pReturn;
    }
    void OColumnWrapperImport::startFastElement(sal_Int32 /*nElement*/, const Reference< XFastAttributeList >& _rxAttrList)
    {
        OSL_ENSURE(!m_xOwnAttributes.is(), "OColumnWrapperImport::StartElement: already have the cloned list!");

        // clone the attributes
        Reference< XCloneable > xCloneList(_rxAttrList, UNO_QUERY_THROW);
        m_xOwnAttributes.set(xCloneList->createClone(), UNO_QUERY_THROW);
    }

    OControlImport* OColumnWrapperImport::implCreateChildContext(
            sal_Int32 /*nElement*/,
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
                return new OColumnImport<OListAndComboImport>(m_rFormImport, m_rEventManager, m_xParentContainer, _eType );

            case OControlElement::PASSWORD:
                return new OColumnImport<OPasswordImport>(m_rFormImport, m_rEventManager, m_xParentContainer, _eType );

            case OControlElement::TEXT:
            case OControlElement::TEXT_AREA:
            case OControlElement::FORMATTED_TEXT:
                return new OColumnImport< OTextLikeImport >( m_rFormImport, m_rEventManager, m_xParentContainer, _eType );

            default:
                return new OColumnImport<OControlImport>(m_rFormImport, m_rEventManager, m_xParentContainer, _eType );
        }
    }

    //= OGridImport
    OGridImport::OGridImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _rxParentContainer)
    {
        setElementType(_eType);
    }

    css::uno::Reference< css::xml::sax::XFastContextHandler > OGridImport::createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
    {
        // maybe it's a sub control
        if ((nElement & TOKEN_MASK) == XML_COLUMN)
        {
            if (m_xMeAsContainer.is())
                return new OColumnWrapperImport(m_rFormImport, *this, nElement, m_xMeAsContainer);
            else
            {
                OSL_FAIL("OGridImport::CreateChildContext: don't have an element!");
                return nullptr;
            }
        }

        return OControlImport::createFastChildContext(nElement, xAttrList);
    }

    void OGridImport::endFastElement(sal_Int32 nElement)
    {
        OControlImport::endFastElement(nElement);

        // now that we have all children, attach the events
        css::uno::Reference< css::container::XIndexAccess > xIndexContainer(m_xMeAsContainer, css::uno::UNO_QUERY);
        if (xIndexContainer.is())
            ODefaultEventAttacherManager::setEvents(xIndexContainer);
    }

    css::uno::Reference< css::beans::XPropertySet > OGridImport::createElement()
    {
        // let the base class create the object
        css::uno::Reference< css::beans::XPropertySet > xReturn = OControlImport::createElement();
        if (!xReturn.is())
            return xReturn;

        // ensure that the object is a XNameContainer (we strongly need this for inserting child elements)
        m_xMeAsContainer.set(xReturn, css::uno::UNO_QUERY);
        if (!m_xMeAsContainer.is())
        {
            OSL_FAIL("OContainerImport::createElement: invalid element (no XNameContainer) created!");
            xReturn.clear();
        }

        return xReturn;
    }

    //= OFormImport
    OFormImport::OFormImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager,
            const Reference< XNameContainer >& _rxParentContainer)
        :OElementImport(_rImport, _rEventManager, _rxParentContainer)
    {
        enableTrackAttributes();
    }

    css::uno::Reference< css::xml::sax::XFastContextHandler > OFormImport::createFastChildContext(
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList>& _rxAttrList )
    {
        auto nToken = (nElement & TOKEN_MASK);
        if( nToken == XML_FORM )
            return new OFormImport( m_rFormImport, *this, m_xMeAsContainer);
        else if ( nToken == XML_CONNECTION_RESOURCE )
            return new OXMLDataSourceImport(GetImport(), _rxAttrList, m_xElement);
        else if( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) ||
                 nToken == XML_PROPERTIES )
            return OElementImport::createFastChildContext( nElement, _rxAttrList );
        else
        {
            OControlElement::ElementType eType = OElementNameMap::getElementType(nToken);
            switch (eType)
            {
                case OControlElement::TEXT:
                case OControlElement::TEXT_AREA:
                case OControlElement::FORMATTED_TEXT:
                    return new OTextLikeImport(m_rFormImport, *this, m_xMeAsContainer, eType);
                case OControlElement::GRID:
                    return new OGridImport(m_rFormImport, *this, m_xMeAsContainer, eType);
                case OControlElement::COMBOBOX:
                case OControlElement::LISTBOX:
                    return new OListAndComboImport(m_rFormImport, *this, m_xMeAsContainer, eType);
                case OControlElement::PASSWORD:
                    return new OPasswordImport(m_rFormImport, *this, m_xMeAsContainer, eType);
                case OControlElement::BUTTON:
                case OControlElement::IMAGE:
                case OControlElement::IMAGE_FRAME:
                    return new OButtonImport( m_rFormImport, *this, m_xMeAsContainer, eType );
                case OControlElement::RADIO:
                    return new ORadioImport(m_rFormImport, *this, m_xMeAsContainer, eType);
                case OControlElement::CHECKBOX:
                    return new OImagePositionImport(m_rFormImport, *this, m_xMeAsContainer, eType);
                case OControlElement::FRAME:
                case OControlElement::FIXED_TEXT:
                    return new OReferredControlImport(m_rFormImport, *this, m_xMeAsContainer);
                case OControlElement::VALUERANGE:
                    return new OValueRangeImport( m_rFormImport, *this, m_xMeAsContainer, eType );
                default:
                    return new OControlImport(m_rFormImport, *this, m_xMeAsContainer, eType);
            }
        }
    }

    void OFormImport::startFastElement(sal_Int32 nElement, const Reference< XFastAttributeList >& _rxAttrList)
    {
        m_rFormImport.enterEventContext();
        OElementImport::startFastElement(nElement, _rxAttrList);

        // handle the target-frame attribute
        simulateDefaultedAttribute(OAttributeMetaData::getCommonControlAttributeToken(CCAFlags::TargetFrame), PROPERTY_TARGETFRAME, "_blank");
    }

    void OFormImport::endFastElement(sal_Int32 nElement)
    {
        OElementImport::endFastElement(nElement);

        // now that we have all children, attach the events
        css::uno::Reference< css::container::XIndexAccess > xIndexContainer(m_xMeAsContainer, css::uno::UNO_QUERY);
        if (xIndexContainer.is())
            ODefaultEventAttacherManager::setEvents(xIndexContainer);

        m_rFormImport.leaveEventContext();
    }

    css::uno::Reference< css::beans::XPropertySet > OFormImport::createElement()
    {
        // let the base class create the object
        css::uno::Reference< css::beans::XPropertySet > xReturn = OElementImport::createElement();
        if (!xReturn.is())
            return xReturn;

        // ensure that the object is a XNameContainer (we strongly need this for inserting child elements)
        m_xMeAsContainer.set(xReturn, css::uno::UNO_QUERY);
        if (!m_xMeAsContainer.is())
        {
            OSL_FAIL("OContainerImport::createElement: invalid element (no XNameContainer) created!");
            xReturn.clear();
        }

        return xReturn;
    }

    bool OFormImport::handleAttribute(sal_Int32 nElement, const OUString& _rValue)
    {
        // handle the master/details field attributes (they're way too special to let the OPropertyImport handle them)
        static const sal_Int32 s_nMasterFieldsAttributeName = OAttributeMetaData::getFormAttributeToken(faMasterFields);
        static const sal_Int32 s_nDetailFieldsAttributeName = OAttributeMetaData::getFormAttributeToken(faDetailFields);

        if ( (nElement & TOKEN_MASK) == s_nMasterFieldsAttributeName)
        {
            implTranslateStringListProperty(PROPERTY_MASTERFIELDS, _rValue);
            return true;
        }

        if ( (nElement & TOKEN_MASK) == s_nDetailFieldsAttributeName)
        {
            implTranslateStringListProperty(PROPERTY_DETAILFIELDS, _rValue);
            return true;
        }

        return OElementImport::handleAttribute(nElement, _rValue);
    }

    void OFormImport::implTranslateStringListProperty(const OUString& _rPropertyName, const OUString& _rValue)
    {
        PropertyValue aProp;
        aProp.Name = _rPropertyName;

        Sequence< OUString > aList;

        // split up the value string
        if (!_rValue.isEmpty())
        {
            // For the moment, we build a vector instead of a Sequence. It's easier to handle because of its
            // push_back method
            ::std::vector< OUString > aElements;
            // estimate the number of tokens
            sal_Int32 nEstimate = 0, nLength = _rValue.getLength();
            const sal_Unicode* pChars = _rValue.getStr();
            for (sal_Int32 i=0; i<nLength; ++i, ++pChars)
                if (*pChars == ',')
                    ++nEstimate;
            aElements.reserve(nEstimate + 1);
                // that's the worst case. If the string contains the separator character _quoted_, we reserved too much...

            sal_Int32 nElementStart = 0;
            sal_Int32 nNextSep = 0;
            sal_Int32 nElementLength;
            OUString sElement;
            do
            {
                // extract the current element
                nNextSep = ::sax::Converter::indexOfComma(
                    _rValue, nElementStart);
                if (-1 == nNextSep)
                    nNextSep = nLength;
                sElement = _rValue.copy(nElementStart, nNextSep - nElementStart);

                nElementLength = sElement.getLength();
                // when writing the sequence, we quoted the single elements with " characters
                OSL_ENSURE( sElement.startsWith("\"") && sElement.endsWith("\""),
                        "OFormImport::implTranslateStringListProperty: invalid quoted element name.");
                sElement = sElement.copy(1, nElementLength - 2);

                aElements.push_back(sElement);

                // switch to the next element
                nElementStart = 1 + nNextSep;
            }
            while (nElementStart < nLength);

            aList = Sequence< OUString >(aElements.data(), aElements.size());
        }
        else
        {
            OSL_FAIL("OFormImport::implTranslateStringListProperty: invalid value (empty)!");
        }

        aProp.Value <<= aList;

        // add the property to the base class' array
        implPushBackPropertyValue(aProp);
    }
    //= OXMLDataSourceImport
    OXMLDataSourceImport::OXMLDataSourceImport(
                    SvXMLImport& _rImport
                    ,const Reference< css::xml::sax::XFastAttributeList > & _xAttrList
                    ,const css::uno::Reference< css::beans::XPropertySet >& _xElement) :
        SvXMLImportContext( _rImport)
    {
        for( auto& aIter : sax_fastparser::castToFastAttributeList(_xAttrList) )
        {
            if  (  aIter.getToken() ==
                    XML_ELEMENT(XLINK, OAttributeMetaData::getCommonControlAttributeToken( CCAFlags::TargetLocation ) ) )
            {
                OUString sValue = aIter.toString();
                sValue = _rImport.GetAbsoluteReference(sValue);
                INetURLObject aURL(sValue);
                if ( aURL.GetProtocol() == INetProtocol::File )
                    _xElement->setPropertyValue(PROPERTY_DATASOURCENAME,makeAny(sValue));
                else
                    _xElement->setPropertyValue(PROPERTY_URL,makeAny(sValue)); // the url is the "sdbc:" string
                break;
            }
            else
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aIter.toString());
        }
    }

    OUString OFormImport::determineDefaultServiceName() const
    {
        return "com.sun.star.form.component.Form";
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
