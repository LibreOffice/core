/*************************************************************************
 *
 *  $RCSfile: elementimport.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:38:38 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <algorithm>

#ifndef _XMLOFF_FORMS_ELEMENTIMPORT_HXX_
#include "elementimport.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _XMLOFF_FORMS_ATTRIBLISTMERGE_HXX_
#include "attriblistmerge.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#include "eventimport.hxx"
#endif
#ifndef XMLOFF_FORMSTYLES_HXX
#include "formstyles.hxx"
#endif
#ifndef _XMLOFF_FORMENUMS_HXX_
#include "formenums.hxx"
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

#include <algorithm>

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::xml;
    using namespace ::com::sun::star::util;

#define PROPID_VALUE            1
#define PROPID_CURRENT_VALUE    2
#define PROPID_MIN_VALUE        3
#define PROPID_MAX_VALUE        4

    //=====================================================================
    struct PropertyValueLess
    {
        sal_Bool operator()(const PropertyValue& _rLeft, const PropertyValue& _rRight)
        {
            return _rLeft.Name < _rRight.Name;
        }
    };

    //=====================================================================
    template <class ELEMENT>
    void pushBackSequenceElement(Sequence< ELEMENT >& _rContainer, const ELEMENT& _rElement)
    {
        sal_Int32 nLen = _rContainer.getLength();
        _rContainer.realloc(nLen + 1);
        _rContainer[nLen] = _rElement;
    }

    //=====================================================================
    //= OElementNameMap
    //=====================================================================
    //---------------------------------------------------------------------
    OElementNameMap::MapString2Element  OElementNameMap::s_sElementTranslations;

    //---------------------------------------------------------------------
    const OControlElement::ElementType& operator ++(OControlElement::ElementType& _e)
    {
        OControlElement::ElementType e = _e;
        sal_Int32 nAsInt = static_cast<sal_Int32>(e);
        _e = static_cast<OControlElement::ElementType>( ++nAsInt );
        return _e;
    }

    //---------------------------------------------------------------------
    OControlElement::ElementType OElementNameMap::getElementType(const ::rtl::OUString& _rName)
    {
        if (0 == s_sElementTranslations.size())
        {   // initialize
            for (ElementType eType=(ElementType)0; eType<UNKNOWN; ++eType)
                s_sElementTranslations[::rtl::OUString::createFromAscii(getElementName(eType))] = eType;
        }
        ConstMapString2ElementIterator aPos = s_sElementTranslations.find(_rName);
        if (s_sElementTranslations.end() != aPos)
            return aPos->second;

#if SUPD<624
        // compatibility
        if (_rName.compareToAscii("text-area"))
            return TEXT_AREA;
#endif
        return UNKNOWN;
    }

    //=====================================================================
    //= OElementImport
    //=====================================================================
    //---------------------------------------------------------------------
    OElementImport::OElementImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OPropertyImport(_rImport, _nPrefix, _rName)
        ,m_xParentContainer(_rxParentContainer)
        ,m_rFormImport(_rImport)
        ,m_rEventManager(_rEventManager)
        ,m_pStyleElement( NULL )
    {
        OSL_ENSURE(m_xParentContainer.is(), "OElementImport::OElementImport: invalid parent container!");
    }

    //---------------------------------------------------------------------
    void OElementImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        ENTER_LOG_CONTEXT( "xmloff::OElementImport - importing one element" );
        // call the base class. This should give us enough information (especially the service name)
        // to create our UNO element
        OPropertyImport::StartElement(_rxAttrList);

        // create the element
        m_xElement = createElement();
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OElementImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        static const ::rtl::OUString s_sEventTagName = ::rtl::OUString::createFromAscii("events");
        if ((s_sEventTagName == _rLocalName) && (XML_NAMESPACE_OFFICE == _nPrefix))
            return new OFormEventsImportContext(m_rFormImport.getGlobalContext(), _nPrefix, _rLocalName, *this);

        return OPropertyImport::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
    }

    //---------------------------------------------------------------------
    void OElementImport::EndElement()
    {
        OSL_ENSURE(m_xElement.is(), "OElementImport::EndElement: invalid element created!");
        if (!m_xElement.is())
            return;

        // set all the properties we collected
#if OSL_DEBUG_LEVEL > 0
        // check if the object has all the properties
        // (We do this in the non-pro version only. Doing it all the time would be much to expensive)
        Reference< XPropertySetInfo > xPropInfo = m_xElement->getPropertySetInfo();
        if (xPropInfo.is())
        {
            for (   ConstPropertyValueArrayIterator aCheck = m_aValues.begin();
                    aCheck != m_aValues.end();
                    ++aCheck
                )
            {
                OSL_ENSURE(xPropInfo->hasPropertyByName(aCheck->Name),
                        ::rtl::OString("OElementImport::EndElement: read a property (")
                    +=  ::rtl::OString(aCheck->Name.getStr(), aCheck->Name.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString(") which does not exist on the element!"));
            }
        }
#endif
        OSL_ENSURE(m_aValues.size(), "OElementImport::EndElement: no properties read!");

        // set the properties
        Reference< XMultiPropertySet > xMultiProps(m_xElement, UNO_QUERY);
        sal_Bool bSuccess = sal_False;
        if (xMultiProps.is())
        {
            // translate our properties so that the XMultiPropertySet can handle them

            // sort our property value array so that we can use it in a setPropertyValues
            ::std::sort( m_aValues.begin(), m_aValues.end(), PropertyValueLess());

            // the names
            Sequence< ::rtl::OUString > aNames(m_aValues.size());
            ::rtl::OUString* pNames = aNames.getArray();
            // the values
            Sequence< Any > aValues(m_aValues.size());
            Any* pValues = aValues.getArray();
            // copy

            for (   ConstPropertyValueArrayIterator aPropValues = m_aValues.begin();
                    aPropValues != m_aValues.end();
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
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OElementImport::EndElement: could not set the properties (using the XMultiPropertySet)!");
            }
        }

        if (!bSuccess)
        {   // no XMultiPropertySet or setting all properties at once failed
            for (   ConstPropertyValueArrayIterator aPropValues = m_aValues.begin();
                    aPropValues != m_aValues.end();
                    ++aPropValues
                )
            {
                // this try/catch here is expensive, but because this is just a fallback which should normally not be
                // used it's acceptable this way ...
                try
                {
                    m_xElement->setPropertyValue(aPropValues->Name, aPropValues->Value);
                }
                catch(Exception&)
                {
                    OSL_ENSURE(sal_False,
                            ::rtl::OString("OElementImport::EndElement: could not set the property \"")
                        +=  ::rtl::OString(aPropValues->Name.getStr(), aPropValues->Name.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=  ::rtl::OString("\"!"));
                }
            }
        }

        // set the style properties
        if ( m_pStyleElement && m_xElement.is() )
        {
            const_cast< OControlStyleContext* >( m_pStyleElement )->FillPropertySet( m_xElement );

            ::rtl::OUString sNumberStyleName = const_cast< OControlStyleContext* >( m_pStyleElement )->getNumberStyleName( );
            if ( sNumberStyleName.getLength() )
                // the style also has a number (sub) style
                m_rContext.applyControlNumberStyle( m_xElement, sNumberStyleName );
        }

        // insert the element into the parent container
        if (!m_sName.getLength())
        {
            OSL_ENSURE(sal_False, "OElementImport::EndElement: did not find a name attribute!");
            m_sName = implGetDefaultName();
        }

        m_xParentContainer->insertByName(m_sName, makeAny(m_xElement));
        LEAVE_LOG_CONTEXT( );
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OElementImport::implGetDefaultName() const
    {
        // no optimization here. If this method gets called, the XML stream did not contain a name for the
        // element, which is a heavy error. So in this case we don't care for performance
        Sequence< ::rtl::OUString > aNames = m_xParentContainer->getElementNames();
        static const ::rtl::OUString sUnnamedName = ::rtl::OUString::createFromAscii("unnamed");

        ::rtl::OUString sReturn;
        const ::rtl::OUString* pNames = NULL;
        const ::rtl::OUString* pNamesEnd = aNames.getConstArray() + aNames.getLength();
        for (sal_Int32 i=0; i<32768; ++i)   // the limit is nearly arbitrary ...
        {
            // assemble the new name (suggestion)
            sReturn = sUnnamedName;
            sReturn += ::rtl::OUString::valueOf(i);
            // check the existence (this is the bad performance part ....)
            for (pNames = aNames.getConstArray(); pNames<pNamesEnd; ++pNames)
            {
                if (*pNames == sReturn)
                {
                    break;
                }
            }
            if (pNames<pNamesEnd)
                // found the name
                continue;
            return sReturn;
        }
        OSL_ENSURE(sal_False, "OElementImport::implGetDefaultName: did not find a free name!");
        return sUnnamedName;
    }

    //---------------------------------------------------------------------
    void OElementImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        static const ::rtl::OUString s_sServiceNameAttribute = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_SERVICE_NAME));
        static const ::rtl::OUString s_sNameAttribute = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_NAME));

        if (!m_sServiceName.getLength() && (_rLocalName == s_sServiceNameAttribute))
        {   // it's the service name
            m_sServiceName = _rValue;
        }
        else
        {
            if (!m_sName.getLength() && (_rLocalName == s_sNameAttribute))
                // remember the name for later use in EndElement
                m_sName = _rValue;

            // maybe it's the style attribute?
            if ( 0 == _rLocalName.compareToAscii( getSpecialAttributeName( SCA_COLUMN_STYLE_NAME ) ) )
            {
                const SvXMLStyleContext* pStyleContext = m_rContext.getStyleElement( _rValue );
                OSL_ENSURE( pStyleContext, "OPropertyImport::handleAttribute: do not know the style!" );
                // remember the element for later usage.
                m_pStyleElement = static_cast< const OControlStyleContext* >( pStyleContext );
            }
            else
                // let the base class handle it
                OPropertyImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
        }
    }

    //---------------------------------------------------------------------
    Reference< XPropertySet > OElementImport::createElement()
    {
        Reference< XPropertySet > xReturn;
        if (m_sServiceName.getLength())
        {
            Reference< XInterface > xPure = m_rFormImport.getServiceFactory()->createInstance(m_sServiceName);
            OSL_ENSURE(xPure.is(),
                        ::rtl::OString("OElementImport::createElement: service factory gave me no object (service name: ")
                    +=  ::rtl::OString(m_sServiceName.getStr(), m_sServiceName.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString(")!"));
            xReturn = Reference< XPropertySet >(xPure, UNO_QUERY);
        }
        else
            OSL_ENSURE(sal_False, "OElementImport::createElement: no service name to create an element!");

        return xReturn;
    }

    //---------------------------------------------------------------------
    void OElementImport::registerEvents(const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        OSL_ENSURE(m_xElement.is(), "OElementImport::registerEvents: no element to register events for!");
        m_rEventManager.registerEvents(m_xElement, _rEvents);
    }

    //---------------------------------------------------------------------
    void OElementImport::simulateDefaultedAttribute(const sal_Char* _pAttributeName, const ::rtl::OUString& _rPropertyName, const sal_Char* _pAttributeDefault)
    {
        Reference< XPropertySetInfo > xPropsInfo = m_xElement->getPropertySetInfo();
        OSL_ENSURE( xPropsInfo.is(), "OPropertyImport::simulateDefaultedAttribute: the component should be more gossipy about it's properties!" );

        if ( !xPropsInfo.is() || xPropsInfo->hasPropertyByName( _rPropertyName ) )
        {
            ::rtl::OUString sLocalAttrName = ::rtl::OUString::createFromAscii(_pAttributeName);
            if ( !encounteredAttribute( sLocalAttrName ) )
                handleAttribute( XML_NAMESPACE_FORM, sLocalAttrName, ::rtl::OUString::createFromAscii( _pAttributeDefault ) );
        }
    }

    //=====================================================================
    //= OControlImport
    //=====================================================================
    //---------------------------------------------------------------------
    OControlImport::OControlImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OElementImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
        ,m_eElementType(OControlElement::UNKNOWN)
    {
    }

    //---------------------------------------------------------------------
    OControlImport::OControlImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OElementImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
        ,m_eElementType(_eType)
    {
    }

    //---------------------------------------------------------------------
    void OControlImport::addOuterAttributes(const Reference< sax::XAttributeList >& _rxOuterAttribs)
    {
        OSL_ENSURE(!m_xOuterAttributes.is(), "OControlImport::addOuterAttributes: already have these attributes!");
        m_xOuterAttributes = _rxOuterAttribs;
    }

    //---------------------------------------------------------------------
    void OControlImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        // the control id
        static const ::rtl::OUString s_sControlIdAttribute = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_CONTROL_ID));

        // the value attributes
        static const ::rtl::OUString s_sValueAttribute          = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_VALUE));
        static const ::rtl::OUString s_sCurrentValueAttribute   = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_CURRENT_VALUE));
        static const ::rtl::OUString s_sMinValueAttribute       = ::rtl::OUString::createFromAscii(getSpecialAttributeName(SCA_MIN_VALUE));
        static const ::rtl::OUString s_sMaxValueAttribute       = ::rtl::OUString::createFromAscii(getSpecialAttributeName(SCA_MAX_VALUE));

        if (!m_sControlId.getLength() && (_rLocalName == s_sControlIdAttribute))
        {   // it's the control id
            m_sControlId = _rValue;
        }
        else if ( _rLocalName.equalsAscii( getBindingAttributeName( BA_LINKED_CELL ) ) )
        {
            m_sBoundCellAddress = _rValue;
        }
        else
        {
            sal_Int32 nHandle;
            if  (   ((_rLocalName == s_sValueAttribute) && (nHandle = PROPID_VALUE))
                ||  ((_rLocalName == s_sCurrentValueAttribute) && (nHandle = PROPID_CURRENT_VALUE))
                ||  ((_rLocalName == s_sMinValueAttribute) && (nHandle = PROPID_MIN_VALUE))
                ||  ((_rLocalName == s_sMaxValueAttribute) && (nHandle = PROPID_MAX_VALUE))
                // it's no == in the second part, it's an assignment!!!!!
                )
            {
                // for the moment, simply remember the name and the value
                PropertyValue aProp;
                aProp.Name = _rLocalName;
                aProp.Handle = nHandle;
                aProp.Value <<= _rValue;
                m_aValueProperties.push_back(aProp);
            }
            else
                OElementImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
        }
    }

    //---------------------------------------------------------------------
    void OControlImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        // merge the attribute lists
        OAttribListMerger* pMerger = new OAttribListMerger;
        // our own one
        pMerger->addList(_rxAttrList);
        // and the ones of our enclosing element
        pMerger->addList(m_xOuterAttributes);
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttributes = pMerger;

        // let the base class handle all the attributes
        OElementImport::StartElement(xAttributes);

        if (m_aValueProperties.size() && m_xElement.is())
        {
            // get the property set info
            Reference< XPropertySetInfo > xPropsInfo = m_xElement->getPropertySetInfo();
            if (!xPropsInfo.is())
            {
                OSL_ENSURE(sal_False, "OControlImport::StartElement: no PropertySetInfo!");
                return;
            }

            const sal_Char* pValueProperty = NULL;
            const sal_Char* pCurrentValueProperty = NULL;
            const sal_Char* pMinValueProperty = NULL;
            const sal_Char* pMaxValueProperty = NULL;

            sal_Bool bRetrievedValues = sal_False;
            sal_Bool bRetrievedValueLimits = sal_False;

            // get the class id of our element
            sal_Int16 nClassId = FormComponentType::CONTROL;
            m_xElement->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;

            // translate the value properties we collected in handleAttributes
            for (   PropertyValueArrayIterator aValueProps = m_aValueProperties.begin();
                    aValueProps != m_aValueProperties.end();
                    ++aValueProps
                )
            {
                switch (aValueProps->Handle)
                {
                    case PROPID_VALUE:
                    case PROPID_CURRENT_VALUE:
                    {
                        // get the property names
                        if (!bRetrievedValues)
                        {
                            getValuePropertyNames(m_eElementType, nClassId, pCurrentValueProperty, pValueProperty);
                            bRetrievedValues = sal_True;
                        }
                        OSL_ENSURE((PROPID_VALUE != aValueProps->Handle) || pValueProperty,
                            "OControlImport::StartElement: the control does not have a value property!");
                        OSL_ENSURE((PROPID_CURRENT_VALUE != aValueProps->Handle) || pCurrentValueProperty,
                            "OControlImport::StartElement: the control does not have a current-value property!");

                        // transfer the name
                        if (PROPID_VALUE == aValueProps->Handle)
                            aValueProps->Name = ::rtl::OUString::createFromAscii(pValueProperty);
                        else
                            aValueProps->Name = ::rtl::OUString::createFromAscii(pCurrentValueProperty);
                    }
                    break;
                    case PROPID_MIN_VALUE:
                    case PROPID_MAX_VALUE:
                    {
                        // get the property names
                        if (!bRetrievedValueLimits)
                        {
                            getValueLimitPropertyNames(nClassId, pMinValueProperty, pMaxValueProperty);
                            bRetrievedValueLimits = sal_True;
                        }
                        OSL_ENSURE((PROPID_MIN_VALUE != aValueProps->Handle) || pMinValueProperty,
                            "OControlImport::StartElement: the control does not have a value property!");
                        OSL_ENSURE((PROPID_MAX_VALUE != aValueProps->Handle) || pMaxValueProperty,
                            "OControlImport::StartElement: the control does not have a current-value property!");

                        // transfer the name
                        if (PROPID_MIN_VALUE == aValueProps->Handle)
                            aValueProps->Name = ::rtl::OUString::createFromAscii(pMinValueProperty);
                        else
                            aValueProps->Name = ::rtl::OUString::createFromAscii(pMaxValueProperty);
                    }
                    break;
                }

                // translate the value
                implTranslateValueProperty(xPropsInfo, *aValueProps);
                // add the property to the base class' array
                implPushBackPropertyValue(*aValueProps);
            }
        }
    }

    //---------------------------------------------------------------------
    void OControlImport::implTranslateValueProperty(const Reference< XPropertySetInfo >& _rxPropInfo,
        PropertyValue& _rPropValue)
    {
        OSL_ENSURE(_rxPropInfo->hasPropertyByName(_rPropValue.Name),
            "OControlImport::implTranslateValueProperty: invalid property name!");

        // retrieve the type of the property
        Property aProp = _rxPropInfo->getPropertyByName(_rPropValue.Name);
        // the untranslated string value as read in handleAttribute
        ::rtl::OUString sValue;
    #if OSL_DEBUG_LEVEL > 0
        sal_Bool bSuccess =
    #endif
        _rPropValue.Value >>= sValue;
        OSL_ENSURE(bSuccess, "OControlImport::implTranslateValueProperty: supposed to be called with non-translated string values!");

        if (TypeClass_ANY == aProp.Type.getTypeClass())
        {
            // we have exactly 2 properties where this type class is allowed:
            OSL_ENSURE(
                    (0 == _rPropValue.Name.compareToAscii(PROPERTY_EFFECTIVE_VALUE))
                ||  (0 == _rPropValue.Name.compareToAscii(PROPERTY_EFFECTIVE_DEFAULT)),
                "OControlImport::implTranslateValueProperty: invalid property type/name combination!");

            // Both properties are allowed to have a double or a string value,
            // so first try to convert the string into a number
            double nValue;
            if (GetImport().GetMM100UnitConverter().convertDouble(nValue, sValue))
                _rPropValue.Value <<= nValue;
            else
                _rPropValue.Value <<= sValue;
        }
        else
            _rPropValue.Value = convertString(GetImport(), aProp.Type, sValue);
    }

    //---------------------------------------------------------------------
    void OControlImport::EndElement()
    {
        OSL_ENSURE(m_xElement.is(), "OControlImport::EndElement: invalid control!");
        if ( !m_xElement.is() )
            return;

        // register our control with it's id
        if (m_sControlId.getLength())
            m_rFormImport.getControlIdMap().registerControlId(m_xElement, m_sControlId);
        // it's allowed to have no control id. In this case we're importing a column

        // one more pre-work to do:
        // when we set default values, then by definition the respective value is set
        // to this default value, too. This means if the sequence contains for example
        // a DefaultText value, then the Text will be affected by this, too.
        // In case the Text is not part of the property sequence (or occurs _before_
        // the DefaultText, which can happen for other value/default-value property names),
        // this means that the Text (the value property) is incorrectly imported.
        // #102475# - 04.09.2002 - fs@openoffice.org

        sal_Bool bRestoreValuePropertyValue = sal_False;
        Any aValuePropertyValue;

        sal_Int16 nClassId = FormComponentType::CONTROL;
        try
        {
            // get the class id of our element
            m_xElement->getPropertyValue(PROPERTY_CLASSID) >>= nClassId;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OControlImport::EndElement: caught an exception while retrieving the class id!" );
        }

        const sal_Char* pValueProperty = NULL;
        const sal_Char* pDefaultValueProperty = NULL;
        getRuntimeValuePropertyNames(m_eElementType, nClassId, pValueProperty, pDefaultValueProperty);
        if ( pDefaultValueProperty && pValueProperty )
        {
            sal_Bool bNonDefaultValuePropertyValue = sal_False;
                // is the "value property" part of the sequence?

            // look up this property in our sequence
            for (   ConstPropertyValueArrayIterator aCheck = m_aValues.begin();
                    ( aCheck != m_aValues.end() );
                    ++aCheck
                )
            {
                if ( aCheck->Name.equalsAscii( pDefaultValueProperty ) )
                    bRestoreValuePropertyValue = sal_True;
                else if ( aCheck->Name.equalsAscii( pValueProperty ) )
                {
                    bNonDefaultValuePropertyValue = sal_True;
                    // we need to restore the value property we found here, nothing else
                    aValuePropertyValue = aCheck->Value;
                }
            }

            if ( bRestoreValuePropertyValue && !bNonDefaultValuePropertyValue )
            {
                // found it -> need to remember (and restore) the "value property value", which is not set explicitly
                try
                {
                    aValuePropertyValue = m_xElement->getPropertyValue( ::rtl::OUString::createFromAscii( pValueProperty ) );
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "OControlImport::EndElement: caught an exception while retrieving the current value property!" );
                }
            }
        }

        // let the base class set all the values
        OElementImport::EndElement();

        // restore the "value property value", if necessary
        if ( bRestoreValuePropertyValue && pValueProperty )
        {
            try
            {
                m_xElement->setPropertyValue( ::rtl::OUString::createFromAscii( pValueProperty ), aValuePropertyValue );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OControlImport::EndElement: caught an exception while restoring the value property!" );
            }
        }

        // the external cell binding, if applicable
        if ( m_xElement.is() && m_sBoundCellAddress.getLength() )
            doRegisterCellValueBinding( m_sBoundCellAddress );
    }

    //---------------------------------------------------------------------
    void OControlImport::doRegisterCellValueBinding( const ::rtl::OUString& _rBoundCellAddress )
    {
        OSL_PRECOND( m_xElement.is(), "OControlImport::doRegisterCellValueBinding: invalid element!" );
        OSL_PRECOND( _rBoundCellAddress.getLength(),
            "OControlImport::doRegisterCellValueBinding: invalid address!" );

        m_rContext.registerCellValueBinding( m_xElement, _rBoundCellAddress );
    }

    //---------------------------------------------------------------------
    //added by BerryJia for fixing bug102407 2002-11-5
    Reference< XPropertySet > OControlImport::createElement()
    {
        Reference<XPropertySet> xPropSet = OElementImport::createElement();
        if(xPropSet.is())
        {
            Reference<XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
            if(xInfo.is() && xInfo->hasPropertyByName(PROPERTY_ALIGN))
            {
                Any aValue;
                xPropSet->setPropertyValue(PROPERTY_ALIGN,aValue);
            }
        }
        return xPropSet;
    }
    //=====================================================================
    //= OReferredControlImport
    //=====================================================================
    //---------------------------------------------------------------------
    OReferredControlImport::OReferredControlImport(
            IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
    {
    }

    //---------------------------------------------------------------------
    void OReferredControlImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OControlImport::StartElement(_rxAttrList);

        // the base class should have created the control, so we can register it
        if (m_sReferringControls.getLength())
            m_rFormImport.getControlIdMap().registerControlReferences(m_xElement, m_sReferringControls);
    }

    //---------------------------------------------------------------------
    void OReferredControlImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName,
        const ::rtl::OUString& _rValue)
    {
        static const ::rtl::OUString s_sReferenceAttributeName = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_FOR));
        if (_rLocalName == s_sReferenceAttributeName)
            m_sReferringControls = _rValue;
        else
            OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    //=====================================================================
    //= OPasswordImport
    //=====================================================================
    //---------------------------------------------------------------------
    OPasswordImport::OPasswordImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
    }

    //---------------------------------------------------------------------
    void OPasswordImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        static const ::rtl::OUString s_sEchoCharAttributeName = ::rtl::OUString::createFromAscii(getSpecialAttributeName(SCA_ECHO_CHAR));
        if (_rLocalName == s_sEchoCharAttributeName)
        {
            // need a special handling for the EchoChar property
            PropertyValue aEchoChar;
            aEchoChar.Name = PROPERTY_ECHOCHAR;
            OSL_ENSURE(_rValue.getLength() == 1, "OPasswordImport::handleAttribute: invalid echo char attribute!");
                // we ourself should not have written values other than of length 1
            if (_rValue.getLength() >= 1)
                aEchoChar.Value <<= (sal_Int16)_rValue.getStr()[0];
            else
                aEchoChar.Value <<= (sal_Int16)0;
            implPushBackPropertyValue(aEchoChar);
        }
        else
            OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    //=====================================================================
    //= ORadioImport
    //=====================================================================
    //---------------------------------------------------------------------
    ORadioImport::ORadioImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
    }

    //---------------------------------------------------------------------
    void ORadioImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        // need special handling for the State & CurrentState properties:
        // they're stored as booleans, but expected to be int16 properties
        static const ::rtl::OUString s_sCurrentSelected = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_CURRENT_SELECTED));
        static const ::rtl::OUString s_sSelected = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_SELECTED));
        if ((_rLocalName == s_sCurrentSelected) || (_rLocalName == s_sSelected))
        {
            const OAttribute2Property::AttributeAssignment* pProperty = m_rContext.getAttributeMap().getAttributeTranslation(_rLocalName);
            OSL_ENSURE(pProperty, "ORadioImport::handleAttribute: invalid property map!");
            if (pProperty)
            {
                Any aBooleanValue = convertString(m_rContext.getGlobalContext(), pProperty->aPropertyType, _rValue, pProperty->pEnumMap);

                // create and store a new PropertyValue
                PropertyValue aNewValue;
                aNewValue.Name = pProperty->sPropertyName;
                aNewValue.Value <<= (sal_Int16)::cppu::any2bool(aBooleanValue);

                implPushBackPropertyValue(aNewValue);
            }
        }
        else
            OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    //=====================================================================
    //= OURLReferenceImport
    //=====================================================================
    OURLReferenceImport::OURLReferenceImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
    }

    //---------------------------------------------------------------------
    void OURLReferenceImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        static const sal_Char* s_pTargetLocationAttributeName   = getCommonControlAttributeName( CCA_TARGET_LOCATION );
        static const sal_Char* s_pImageDataAttributeName        = getCommonControlAttributeName( CCA_IMAGE_DATA );

        // need to make the URL absolute if
        // * it's the image-data attribute
        // * it's the target-location attribute, and we're dealign with an object which has the respective property
        sal_Bool bMakeAbsolute =
                ( 0 == _rLocalName.compareToAscii( s_pImageDataAttributeName ) )
            ||  (   ( 0 == _rLocalName.compareToAscii( s_pTargetLocationAttributeName ) )
                &&  (   ( OControlElement::BUTTON == m_eElementType )
                    ||  ( OControlElement::IMAGE == m_eElementType )
                    )
                );

        if ( bMakeAbsolute )
        {
            // make a global URL out of the local one
            ::rtl::OUString sAdjustedValue = m_rContext.getGlobalContext().GetAbsoluteReference( _rValue );
            OControlImport::handleAttribute( _nNamespaceKey, _rLocalName, sAdjustedValue );
        }
        else
            OControlImport::handleAttribute( _nNamespaceKey, _rLocalName, _rValue );
    }

    //=====================================================================
    //= OButtonImport
    //=====================================================================
    //---------------------------------------------------------------------
    OButtonImport::OButtonImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OURLReferenceImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
        enableTrackAttributes();
    }

    //---------------------------------------------------------------------
    void OButtonImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OControlImport::StartElement(_rxAttrList);

        // handle the target-frame attribute
        simulateDefaultedAttribute(getCommonControlAttributeName(CCA_TARGET_FRAME), PROPERTY_TARGETFRAME, "_blank");
    }

    //=====================================================================
    //= OTextLikeImport
    //=====================================================================
    //---------------------------------------------------------------------
    OTextLikeImport::OTextLikeImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    {
        enableTrackAttributes();
    }

    //---------------------------------------------------------------------
    void OTextLikeImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OControlImport::StartElement(_rxAttrList);

        // handle the convert-empty-to-null attribute, which's default is different from the property default
        sal_Bool bHaveEmptyIsNull = sal_False;
        // unfortunately, different classes are imported by this class ('cause they're represented by the
        // same XML element), though not all of them know this property.
        // So we have to do a check ...
        if (m_xElement.is())
        {
            Reference< XPropertySetInfo > xProps = m_xElement->getPropertySetInfo();
            if (xProps.is())
                bHaveEmptyIsNull = xProps->hasPropertyByName(PROPERTY_EMPTY_IS_NULL);
        }

        if (bHaveEmptyIsNull)
            simulateDefaultedAttribute(getDatabaseAttributeName(DA_CONVERT_EMPTY), PROPERTY_EMPTY_IS_NULL, "false");
    }

    //=====================================================================
    //= OListAndComboImport
    //=====================================================================
    //---------------------------------------------------------------------
    OListAndComboImport::OListAndComboImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
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

    //---------------------------------------------------------------------
    SvXMLImportContext* OListAndComboImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            const Reference< sax::XAttributeList >& _rxAttrList)
    {
        // is it the "option" sub tag of a listbox ?
        static const ::rtl::OUString s_sOptionElementName = ::rtl::OUString::createFromAscii("option");
        if (s_sOptionElementName == _rLocalName)
            return new OListOptionImport(GetImport(), _nPrefix, _rLocalName, this);

        // is it the "item" sub tag of a combobox ?
        static const ::rtl::OUString s_sItemElementName = ::rtl::OUString::createFromAscii("item");
        if (s_sItemElementName == _rLocalName)
            return new OComboItemImport(GetImport(), _nPrefix, _rLocalName, this);

        // everything else
        return OControlImport::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        m_bLinkWithIndexes = sal_False;

        OControlImport::StartElement(_rxAttrList);

        if (OControlElement::COMBOBOX == m_eElementType)
        {
            // for the auto-completion
            // the attribute default does not equal the property default, so in case we did not read this attribute,
            // we have to simulate it
            simulateDefaultedAttribute( getSpecialAttributeName( SCA_AUTOMATIC_COMPLETION ), PROPERTY_AUTOCOMPLETE, "false");

            // same for the convert-empty-to-null attribute, which's default is different from the property default
            simulateDefaultedAttribute( getDatabaseAttributeName( DA_CONVERT_EMPTY ), PROPERTY_EMPTY_IS_NULL, "false");
        }
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::EndElement()
    {
        // append the list source property the the properties sequence of our importer
        // the string item list
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
                // the value sequence
                PropertyValue aValueList;
                aValueList.Name = PROPERTY_LISTSOURCE;
                aValueList.Value <<= m_aValueList;
                implPushBackPropertyValue(aValueList);
            }

            // the select sequence
            PropertyValue aSelected;
            aSelected.Name = PROPERTY_SELECT_SEQ;
            aSelected.Value <<= m_aSelectedSeq;
            implPushBackPropertyValue(aSelected);

            // the default select sequence
            PropertyValue aDefaultSelected;
            aDefaultSelected.Name = PROPERTY_DEFAULT_SELECT_SEQ;
            aDefaultSelected.Value <<= m_aDefaultSelectedSeq;
            implPushBackPropertyValue(aDefaultSelected);
        }

        OControlImport::EndElement();

        // the external list source, if applicable
        if ( m_xElement.is() && m_sCellListSource.getLength() )
            m_rContext.registerCellRangeListSource( m_xElement, m_sCellListSource );
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::doRegisterCellValueBinding( const ::rtl::OUString& _rBoundCellAddress )
    {
        ::rtl::OUString sBoundCellAddress( _rBoundCellAddress );
        if ( m_bLinkWithIndexes )
        {
            // This is a HACK. We register a string which is no valid address, but allows
            // (somewhere else) to determine that a non-standard binding should be created.
            // This hack is acceptable for OOo 1.1.1, since the file format for value
            // bindings of form controls is to be changed afterwards, anyway.
            sBoundCellAddress += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ":index" ) );
        }

        OControlImport::doRegisterCellValueBinding( sBoundCellAddress );
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        static const ::rtl::OUString s_sListSourceAttributeName = ::rtl::OUString::createFromAscii(getDatabaseAttributeName(DA_LIST_SOURCE));
        if (s_sListSourceAttributeName == _rLocalName)
        {
            PropertyValue aListSource;
            aListSource.Name = PROPERTY_LISTSOURCE;

            // it's the ListSource attribute
            m_bEncounteredLSAttrib = sal_True;
            if ( OControlElement::COMBOBOX == m_eElementType )
            {
                aListSource.Value <<= _rValue;
            }
            else
            {
                // a listbox which has a list-source attribute must have a list-source-type of something
                // not equal to ValueList.
                // In this case, the list-source value is simply the one and only element of the ListSource property.
                Sequence< ::rtl::OUString > aListSourcePropValue( 1 );
                aListSourcePropValue[0] = _rValue;
                aListSource.Value <<= aListSourcePropValue;
            }

            implPushBackPropertyValue( aListSource );
        }
        else if ( _rLocalName.equalsAscii( getBindingAttributeName( BA_LIST_CELL_RANGE ) ) )
        {
            m_sCellListSource = _rValue;
        }
        else if ( _rLocalName.equalsAscii( getBindingAttributeName( BA_LIST_LINKING_TYPE ) ) )
        {
            sal_Int16 nLinkageType = 0;
            convertString(
                m_rContext.getGlobalContext(),
                ::getCppuType( static_cast< sal_Int16* >( NULL ) ),
                _rValue,
                OEnumMapper::getEnumMap( OEnumMapper::epListLinkageType )
            ) >>= nLinkageType;

            m_bLinkWithIndexes = ( nLinkageType != 0 );
        }
        else
            OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implPushBackLabel(const ::rtl::OUString& _rLabel)
    {
        OSL_ENSURE(!m_nEmptyListItems, "OListAndComboImport::implPushBackValue: label list is already done!");
        if (!m_nEmptyListItems)
            pushBackSequenceElement(m_aListSource, _rLabel);
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implPushBackValue(const ::rtl::OUString& _rValue)
    {
        OSL_ENSURE(!m_nEmptyValueItems, "OListAndComboImport::implPushBackValue: value list is already done!");
        if (!m_nEmptyValueItems)
        {
            OSL_ENSURE( !m_bEncounteredLSAttrib, "OListAndComboImport::implPushBackValue: invalid structure! Did you save this document with a version prior SRC641 m?" );
                // We already had the list-source attribute, which means that the ListSourceType is
                // not ValueList, which means that the ListSource should contain only one string in
                // the first element of the sequence
                // All other values in the file are invalid

            pushBackSequenceElement( m_aValueList, _rValue );
        }
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implEmptyLabelFound()
    {
        ++m_nEmptyListItems;
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implEmptyValueFound()
    {
        ++m_nEmptyValueItems;
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implSelectCurrentItem()
    {
        OSL_ENSURE((m_aListSource.getLength() + m_nEmptyListItems) == (m_aValueList.getLength() + m_nEmptyValueItems),
            "OListAndComboImport::implSelectCurrentItem: inconsistence between labels and values!");

        sal_Int16 nItemNumber = (sal_Int16)(m_aListSource.getLength() - 1 + m_nEmptyListItems);
        pushBackSequenceElement(m_aSelectedSeq, nItemNumber);
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implDefaultSelectCurrentItem()
    {
        OSL_ENSURE((m_aListSource.getLength() + m_nEmptyListItems) == (m_aValueList.getLength() + m_nEmptyValueItems),
            "OListAndComboImport::implDefaultSelectCurrentItem: inconsistence between labels and values!");

        sal_Int16 nItemNumber = (sal_Int16)(m_aListSource.getLength() - 1 + m_nEmptyListItems);
        pushBackSequenceElement(m_aDefaultSelectedSeq, nItemNumber);
    }

    //=====================================================================
    //= OListOptionImport
    //=====================================================================
    //---------------------------------------------------------------------
    OListOptionImport::OListOptionImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const OListAndComboImportRef& _rListBox)
        :SvXMLImportContext(_rImport, _nPrefix, _rName)
        ,m_xListBoxImport(_rListBox)
    {
    }

    //---------------------------------------------------------------------
    void OListOptionImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        // the label and the value
        const ::rtl::OUString sLabelAttribute = GetImport().GetNamespaceMap().GetQNameByKey(
            GetPrefix(), ::rtl::OUString::createFromAscii("label"));
        const ::rtl::OUString sValueAttribute = GetImport().GetNamespaceMap().GetQNameByKey(
            GetPrefix(), ::rtl::OUString::createFromAscii("value"));

        // -------------------
        // the label attribute
        ::rtl::OUString sValue = _rxAttrList->getValueByName(sLabelAttribute);
        sal_Bool bNonexistentAttribute = sal_False;
        if (!sValue.getLength())
            if (0 == _rxAttrList->getTypeByName(sLabelAttribute).getLength())
                // this attribute does not really exist
                bNonexistentAttribute = sal_True;

        if (bNonexistentAttribute)
            m_xListBoxImport->implEmptyLabelFound();
        else
            m_xListBoxImport->implPushBackLabel( sValue );

        // -------------------
        // the value attribute
        sValue = _rxAttrList->getValueByName(sValueAttribute);
        bNonexistentAttribute = sal_False;
        if (!sValue.getLength())
            if (0 == _rxAttrList->getTypeByName(sValueAttribute).getLength())
                // this attribute does not really exist
                bNonexistentAttribute = sal_True;

        if (bNonexistentAttribute)
            m_xListBoxImport->implEmptyValueFound();
        else
            m_xListBoxImport->implPushBackValue( sValue );

        // the current-selected and selected
        const ::rtl::OUString sSelectedAttribute = GetImport().GetNamespaceMap().GetQNameByKey(
            GetPrefix(), ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_CURRENT_SELECTED)));
        const ::rtl::OUString sDefaultSelectedAttribute = GetImport().GetNamespaceMap().GetQNameByKey(
            GetPrefix(), ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_SELECTED)));

        // propagate the selected flag
        sal_Bool bSelected;
        GetImport().GetMM100UnitConverter().convertBool(bSelected, _rxAttrList->getValueByName(sSelectedAttribute));
        if (bSelected)
            m_xListBoxImport->implSelectCurrentItem();

        // same for the default selected
        sal_Bool bDefaultSelected;
        GetImport().GetMM100UnitConverter().convertBool(bDefaultSelected, _rxAttrList->getValueByName(sDefaultSelectedAttribute));
        if (bDefaultSelected)
            m_xListBoxImport->implDefaultSelectCurrentItem();

        SvXMLImportContext::StartElement(_rxAttrList);
    }

    //=====================================================================
    //= OComboItemImport
    //=====================================================================
    //---------------------------------------------------------------------
    OComboItemImport::OComboItemImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const OListAndComboImportRef& _rListBox)
        :SvXMLImportContext(_rImport, _nPrefix, _rName)
        ,m_xListBoxImport(_rListBox)
    {
    }

    //---------------------------------------------------------------------
    void OComboItemImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        const ::rtl::OUString sLabelAttributeName = GetImport().GetNamespaceMap().GetQNameByKey(
            GetPrefix(), ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_LABEL)));
        m_xListBoxImport->implPushBackLabel(_rxAttrList->getValueByName(sLabelAttributeName));

        SvXMLImportContext::StartElement(_rxAttrList);
    }

    //=====================================================================
    //= OControlWrapperImport
    //=====================================================================
    //---------------------------------------------------------------------
    OControlWrapperImport::OControlWrapperImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :SvXMLImportContext(_rImport.getGlobalContext(), _nPrefix, _rName)
        ,m_rFormImport(_rImport)
        ,m_xParentContainer(_rxParentContainer)
        ,m_rEventManager(_rEventManager)
    {
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OControlWrapperImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OControlImport* pReturn = implCreateChildContext(_nPrefix, _rLocalName, OElementNameMap::getElementType(_rLocalName));
        if (pReturn)
        {
            OSL_ENSURE(m_xOwnAttributes.is(), "OControlWrapperImport::CreateChildContext: had no form:column element!");
            pReturn->addOuterAttributes(m_xOwnAttributes);
        }
        return pReturn;
    }

    //---------------------------------------------------------------------
    void OControlWrapperImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OSL_ENSURE(!m_xOwnAttributes.is(), "OControlWrapperImport::StartElement: aready have the cloned list!");

        // clone the attributes
        Reference< XCloneable > xCloneList(_rxAttrList, UNO_QUERY);
        OSL_ENSURE(xCloneList.is(), "OControlWrapperImport::StartElement: AttributeList not cloneable!");
        m_xOwnAttributes = Reference< sax::XAttributeList >(xCloneList->createClone(), UNO_QUERY);
        OSL_ENSURE(m_xOwnAttributes.is(), "OControlWrapperImport::StartElement: no cloned list!");

        // forward an empty attribute list to the base class
        // (the attributes are merged into the ones of the upcoming xml element which really describes the control)
        SvXMLImportContext::StartElement(new OAttribListMerger);
    }

    //---------------------------------------------------------------------
    OControlImport* OControlWrapperImport::implCreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            OControlElement::ElementType _eType)
    {
        switch (_eType)
        {
            case OControlElement::TEXT:
            case OControlElement::TEXT_AREA:
            case OControlElement::FORMATTED_TEXT:
                return new OTextLikeImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);

            case OControlElement::BUTTON:
            case OControlElement::IMAGE:
                return new OButtonImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);

            case OControlElement::IMAGE_FRAME:
                return new OURLReferenceImport( m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType );

            case OControlElement::COMBOBOX:
            case OControlElement::LISTBOX:
                return new OListAndComboImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);

            case OControlElement::RADIO:
                return new ORadioImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);

            case OControlElement::PASSWORD:
                return new OPasswordImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);

            case OControlElement::FRAME:
            case OControlElement::FIXED_TEXT:
                return new OReferredControlImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);

            case OControlElement::GRID:
                return new OGridImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);

            default:
                return new OControlImport(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType);
        }
    }

    //=====================================================================
    //= OColumnWrapperImport
    //=====================================================================
    //---------------------------------------------------------------------
    OColumnWrapperImport::OColumnWrapperImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OControlWrapperImport(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer)
    {
    }

    //---------------------------------------------------------------------
    OControlImport* OColumnWrapperImport::implCreateChildContext(
            sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
            OControlElement::ElementType _eType)
    {
        OSL_ENSURE( (OControlElement::TEXT == _eType)
                ||  (OControlElement::TEXT_AREA == _eType)
                ||  (OControlElement::FORMATTED_TEXT == _eType)
                ||  (OControlElement::CHECKBOX == _eType)
                ||  (OControlElement::LISTBOX == _eType)
                ||  (OControlElement::COMBOBOX == _eType),
                "OColumnWrapperImport::implCreateChildContext: invalid or unrecognized sub element!");

        switch (_eType)
        {
            case OControlElement::COMBOBOX:
            case OControlElement::LISTBOX:
                return new OColumnImport<OListAndComboImport>(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType, m_xOwnAttributes);

            case OControlElement::PASSWORD:
                return new OColumnImport<OPasswordImport>(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType, m_xOwnAttributes);

            default:
                return new OColumnImport<OControlImport>(m_rFormImport, m_rEventManager, _nPrefix, _rLocalName, m_xParentContainer, _eType, m_xOwnAttributes);
        }
    }

    //=====================================================================
    //= OGridImport
    //=====================================================================
    //---------------------------------------------------------------------
    OGridImport::OGridImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OGridImport_Base(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, "column")
    {
        setElementType(_eType);
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OGridImport::implCreateControlWrapper(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName)
    {
        return new OColumnWrapperImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer);
    }

    //=====================================================================
    //= OFormImport
    //=====================================================================
    //---------------------------------------------------------------------
    OFormImport::OFormImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OFormImport_Base(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, "control")
    {
        enableTrackAttributes();
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        static const ::rtl::OUString s_sFormElementName = ::rtl::OUString::createFromAscii("form");
        if (s_sFormElementName.equals(_rLocalName))
            return new OFormImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer);

        return OFormImport_Base::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
    }

    //---------------------------------------------------------------------
    void OFormImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        m_rFormImport.enterEventContext();
        OFormImport_Base::StartElement(_rxAttrList);

        // handle the target-frame attribute
        simulateDefaultedAttribute(getCommonControlAttributeName(CCA_TARGET_FRAME), PROPERTY_TARGETFRAME, "_blank");
    }

    //---------------------------------------------------------------------
    void OFormImport::EndElement()
    {
        OFormImport_Base::EndElement();
        m_rFormImport.leaveEventContext();
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormImport::implCreateControlWrapper(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName)
    {
        return new OControlWrapperImport(m_rFormImport, *this, _nPrefix, _rLocalName, m_xMeAsContainer);
    }

    //---------------------------------------------------------------------
    void OFormImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        // handle the master/details field attributes (they're way too special to let the OPropertyImport handle them)
        static const ::rtl::OUString s_sMasterFieldsAttributeName = ::rtl::OUString::createFromAscii(getFormAttributeName(faMasterFields));
        static const ::rtl::OUString s_sDetailFieldsAttributeName = ::rtl::OUString::createFromAscii(getFormAttributeName(faDetailFiels));
        if (s_sMasterFieldsAttributeName == _rLocalName)
            implTranslateStringListProperty(PROPERTY_MASTERFIELDS, _rValue);
        else if (s_sDetailFieldsAttributeName == _rLocalName)
            implTranslateStringListProperty(PROPERTY_DETAILFIELDS, _rValue);
#if SUPD<632
        // for compatibility (had a typo in the attribute name)
        else if (0 == _rLocalName.compareToAscii("tabbing-cycle"))
            OFormImport_Base::handleAttribute(_nNamespaceKey, ::rtl::OUString::createFromAscii("tab-cycle"), _rValue);
#endif
        else
            OFormImport_Base::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    //---------------------------------------------------------------------
    void OFormImport::implTranslateStringListProperty(const ::rtl::OUString& _rPropertyName, const ::rtl::OUString& _rValue)
    {
        PropertyValue aProp;
        aProp.Name = _rPropertyName;

        Sequence< ::rtl::OUString > aList;

        // split up the value string
        if (_rValue.getLength())
        {
            // For the moment, we build a vector instead of a Sequence. It's easier to handle because of it's
            // push_back method
            ::std::vector< ::rtl::OUString > aElements;
            // estimate the number of tokens
            sal_Int32 nEstimate = 0, nLength = _rValue.getLength();
            const sal_Unicode* pChars = _rValue.getStr();
            for (sal_Int32 i=0; i<nLength; ++i, ++pChars)
                if (*pChars == ',')
                    ++nEstimate;
            aElements.reserve(nEstimate + 1);
                // that's the worst case. If the string contains the separator character _quoted_, we reserved to much ...


            SvXMLUnitConverter& rConverter = GetImport().GetMM100UnitConverter();
            sal_Int32 nElementStart = 0;
            sal_Int32 nNextSep = 0;
            sal_Int32 nElementLength;
            ::rtl::OUString sElement;
            do
            {
                // extract the current element
                nNextSep = rConverter.indexOfComma(_rValue, nElementStart);
                if (-1 == nNextSep)
                    nNextSep = nLength;
                sElement = _rValue.copy(nElementStart, nNextSep - nElementStart);

                nElementLength = sElement.getLength();
                // when writing the sequence, we quoted the single elements with " characters
                OSL_ENSURE( (nElementLength >= 2)
                        &&  (sElement.getStr()[0] == '"')
                        &&  (sElement.getStr()[nElementLength - 1] == '"'),
                        "OFormImport::implTranslateStringListProperty: invalid quoted element name.");
                sElement = sElement.copy(1, nElementLength - 2);

                aElements.push_back(sElement);

                // swith to the next element
                nElementStart = 1 + nNextSep;
            }
            while (nElementStart < nLength);

            aList = Sequence< ::rtl::OUString >(aElements.begin(), aElements.size());
        }
        else
        {
            OSL_ENSURE(sal_False, "OFormImport::implTranslateStringListProperty: invalid value (empty)!");
        }

        aProp.Value <<= aList;

        // add the property to the base class' array
        implPushBackPropertyValue(aProp);
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

