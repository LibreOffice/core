/*************************************************************************
 *
 *  $RCSfile: elementimport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-12-13 10:40:15 $
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

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
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
        sal_Int32 nAsInt = static_cast<sal_Int32>(_e);
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

        return UNKNOWN;
    }

    //=====================================================================
    //= OElementImport
    //=====================================================================
    //---------------------------------------------------------------------
    OElementImport::OElementImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OPropertyImport(_rImport.getGlobalContext(), _nPrefix, _rName, _rImport.getAttributeMap())
        ,m_xParentContainer(_rxParentContainer)
        ,m_rFormImport(_rImport)
    {
        OSL_ENSURE(m_xParentContainer.is(), "OElementImport::OElementImport: invalid parent container!");
    }

    //---------------------------------------------------------------------
    void OElementImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        // call the base class. This should give us enough information (especially the service name)
        // to create our UNO element
        OPropertyImport::StartElement(_rxAttrList);

        // create the element
        m_xElement = createElement();
    }

    //---------------------------------------------------------------------
    void OElementImport::EndElement()
    {
        OSL_ENSURE(m_xElement.is(), "OElementImport::EndElement: invalid element created!");
        if (!m_xElement.is())
            return;

        // set all the properties we collected
#ifdef _DEBUG
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
                OSL_ENSURE(xPropInfo->hasPropertyByName(aCheck->Name), "OElementImport::EndElement: read a property which does not exist on the element!");
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

        // insert the element into the parent container
        if (!m_sName.getLength())
        {
            OSL_ENSURE(sal_False, "OElementImport::EndElement: did not find a name attribute!");
            m_sName = implGetDefaultName();
        }

        m_xParentContainer->insertByName(m_sName, makeAny(m_xElement));
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

    //=====================================================================
    //= OControlImport
    //=====================================================================
    //---------------------------------------------------------------------
    OControlImport::OControlImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OElementImport(_rImport, _nPrefix, _rName, _rxParentContainer)
        ,m_eElementType(OControlElement::UNKNOWN)
    {
    }

    //---------------------------------------------------------------------
    OControlImport::OControlImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer, OControlElement::ElementType _eType)
        :OElementImport(_rImport, _nPrefix, _rName, _rxParentContainer)
        ,m_eElementType(_eType)
    {
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
        // let the base class handle all the attributes
        OElementImport::StartElement(_rxAttrList);

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
    #ifdef DEBUG
        sal_Bool bSuccess =
    #endif
        _rPropValue.Value >>= sValue;
        OSL_ENSURE(bSuccess, "OControlImport::implTranslateValueProperty: supposed to be called with non-translated string values!");

        _rPropValue.Value = convertString(GetImport(), aProp.Type, sValue);
    }

    //---------------------------------------------------------------------
    void OControlImport::EndElement()
    {
        OSL_ENSURE(m_xElement.is(), "OControlImport::EndElement: invalid control!");
        // register our control with it's id
        if (m_xElement.is() && m_sControlId.getLength())
            m_rFormImport.getControlIdMap().registerControlId(m_xElement, m_sControlId);
        // it's allowed to have no control id. In this case we're importing a column

        OElementImport::EndElement();
    }

    //=====================================================================
    //= OReferredControlImport
    //=====================================================================
    //---------------------------------------------------------------------
    OReferredControlImport::OReferredControlImport(
            IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _nPrefix, _rName, _rxParentContainer)
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
    //= OListAndComboImport
    //=====================================================================
    //---------------------------------------------------------------------
    OListAndComboImport::OListAndComboImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OControlImport(_rImport, _nPrefix, _rName, _rxParentContainer, _eType)
    {
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
    void OListAndComboImport::EndElement()
    {
        // append the list source property the the properties sequence of our importer
        // the string item list
        PropertyValue aItemList;
        aItemList.Name = PROPERTY_STRING_ITEM_LIST;
        if (OControlElement::LISTBOX == m_eElementType)
            aItemList.Value <<= m_aListSource;
        else
            aItemList.Value <<= m_aListSource.getLength() ? m_aListSource[0] : ::rtl::OUString();
        implPushBackPropertyValue(aItemList);

        if (OControlElement::LISTBOX == m_eElementType)
        {
            // the value sequence
            PropertyValue aValueList;
            aValueList.Name = PROPERTY_LISTSOURCE;
            aValueList.Value <<= m_aValueList;
            implPushBackPropertyValue(aValueList);

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
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        static const ::rtl::OUString s_sListSourceAttributeName = ::rtl::OUString::createFromAscii(getDatabaseAttributeName(DA_LIST_SOURCE));
        if (s_sListSourceAttributeName == _rLocalName)
        {
            // it's the ListSource attribute
            OSL_ENSURE(0 == m_aListSource.getLength(), "OListAndComboImport::handleAttribute: already have a value for the ListSource property!");
            m_aListSource.realloc(1);
            m_aListSource[0] = _rValue;
        }
        else
            OControlImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implPushBackLabel(const ::rtl::OUString& _rLabel)
    {
        pushBackSequenceElement(m_aListSource, _rLabel);
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implPushBackValue(const ::rtl::OUString& _rValue)
    {
        pushBackSequenceElement(m_aValueList, _rValue);
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implSelectCurrentItem()
    {
        OSL_ENSURE(m_aListSource.getLength() == m_aValueList.getLength(),
            "OListAndComboImport::implSelectCurrentItem: ambiguous index!");

        pushBackSequenceElement(m_aSelectedSeq, (sal_Int16)(m_aListSource.getLength() - 1));
    }

    //---------------------------------------------------------------------
    void OListAndComboImport::implDefaultSelectCurrentItem()
    {
        OSL_ENSURE(m_aListSource.getLength() == m_aValueList.getLength(),
            "OListAndComboImport::implSelectCurrentItem: ambiguous index!");

        pushBackSequenceElement(m_aDefaultSelectedSeq, (sal_Int16)(m_aListSource.getLength() - 1));
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
        const ::rtl::OUString sLabelAttribute = GetImport().GetNamespaceMap().GetQNameByIndex(
            GetPrefix(), ::rtl::OUString::createFromAscii("label"));
        const ::rtl::OUString sValueAttribute = GetImport().GetNamespaceMap().GetQNameByIndex(
            GetPrefix(), ::rtl::OUString::createFromAscii("value"));

        m_xListBoxImport->implPushBackLabel( _rxAttrList->getValueByName(sLabelAttribute) );
        m_xListBoxImport->implPushBackValue( _rxAttrList->getValueByName(sValueAttribute) );

        // the current-selected and selected
        const ::rtl::OUString sSelectedAttribute = GetImport().GetNamespaceMap().GetQNameByIndex(
            GetPrefix(), ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_CURRENT_SELECTED)));
        const ::rtl::OUString sDefaultSelectedAttribute = GetImport().GetNamespaceMap().GetQNameByIndex(
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
        :OAccumulateCharacters(_rImport, _nPrefix, _rName)
        ,m_xListBoxImport(_rListBox)
    {
    }

    //---------------------------------------------------------------------
    void OComboItemImport::EndElement()
    {
        m_xListBoxImport->implPushBackLabel(getCharacters());
    }

    //=====================================================================
    //= OColumnWrapperImport
    //=====================================================================
    //---------------------------------------------------------------------
    OColumnWrapperImport::OColumnWrapperImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :SvXMLImportContext(_rImport.getGlobalContext(), _nPrefix, _rName)
        ,m_rFormImport(_rImport)
        ,m_xParentContainer(_rxParentContainer)
    {
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OColumnWrapperImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OControlElement::ElementType eType = OElementNameMap::getElementType(_rLocalName);
        OSL_ENSURE( (OControlElement::UNKNOWN != eType)
                &&  (OControlElement::COLUMN != eType)
                &&  (OControlElement::GRID != eType)
                &&  (OControlElement::FRAME != eType)
                &&  (OControlElement::FIXED_TEXT != eType),

                "OColumnWrapperImport::CreateChildContext: invalid or unrecognized sub element!");

        switch (eType)
        {
            case OControlElement::COMBOBOX:
            case OControlElement::LISTBOX:
                OSL_ENSURE(m_xOwnAttributes.is(), "OColumnWrapperImport::CreateChildContext: had no form:column element!");
                return new OColumnImport<OListAndComboImport>(m_rFormImport, _nPrefix, _rLocalName, m_xParentContainer, eType, m_xOwnAttributes);

            default:
                OSL_ENSURE(m_xOwnAttributes.is(), "OColumnWrapperImport::CreateChildContext: had no form:column element!");
                return new OColumnImport<OControlImport>(m_rFormImport, _nPrefix, _rLocalName, m_xParentContainer, eType, m_xOwnAttributes);
        }
    }

    //---------------------------------------------------------------------
    void OColumnWrapperImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OSL_ENSURE(!m_xOwnAttributes.is(), "OColumnWrapperImport::StartElement: aready have the cloned list!");

        // clone the attributes
        Reference< XCloneable > xCloneList(_rxAttrList, UNO_QUERY);
        OSL_ENSURE(xCloneList.is(), "OColumnWrapperImport::StartElement: AttributeList not cloneable!");
        m_xOwnAttributes = Reference< sax::XAttributeList >(xCloneList->createClone(), UNO_QUERY);
        OSL_ENSURE(m_xOwnAttributes.is(), "OColumnWrapperImport::StartElement: no cloned list!");

        // forward an empty attribute list to the base class
        // (the attributes are merged into the ones of the upcoming xml element which really describes the column)
        SvXMLImportContext::StartElement(new OAttribListMerger);
    }

    //=====================================================================
    //= OGridImport
    //=====================================================================
    //---------------------------------------------------------------------
    OGridImport::OGridImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer,
            OControlElement::ElementType _eType)
        :OGridImport_Base(_rImport, _nPrefix, _rName, _rxParentContainer)
    {
        setElementType(_eType);
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OGridImport::implCreateControlChild(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        OControlElement::ElementType _eType)
    {
        switch (_eType)
        {
            case OControlElement::COLUMN:
                // this is the wrapper element.
                return new OColumnWrapperImport(m_rFormImport, _nPrefix, _rLocalName, m_xMeAsContainer);
                break;
            default:
                OSL_ENSURE(sal_False, "OGridImport::implCreateControlChild: invalid sub element!");
                // below a form:grid element, the only allowed "control type" is COLUMN
                return NULL;
        }
        OSL_ENSURE(sal_False, "OGridImport::implCreateControlChild: reached the unreacheable!");
        return NULL;
    }

    //=====================================================================
    //= OFormImport
    //=====================================================================
    //---------------------------------------------------------------------
    OFormImport::OFormImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const Reference< XNameContainer >& _rxParentContainer)
        :OFormImport_Base(_rImport, _nPrefix, _rName, _rxParentContainer)
    {
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormImport::implCreateControlChild(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        OControlElement::ElementType _eType)
    {
        switch (_eType)
        {
            case OControlElement::COMBOBOX:
            case OControlElement::LISTBOX:
                return new OListAndComboImport(m_rFormImport, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::FRAME:
            case OControlElement::FIXED_TEXT:
                return new OReferredControlImport(m_rFormImport, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            case OControlElement::GRID:
                return new OGridImport(m_rFormImport, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);

            default:
                return new OControlImport(m_rFormImport, _nPrefix, _rLocalName, m_xMeAsContainer, _eType);
        }
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/12/12 12:01:05  fs
 *  new implementations for the import - still under construction
 *
 *  Revision 1.1  2000/12/06 17:31:03  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 05.12.00 11:09:36  fs
 ************************************************************************/

