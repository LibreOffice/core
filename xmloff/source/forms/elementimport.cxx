/*************************************************************************
 *
 *  $RCSfile: elementimport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-12-06 17:31:03 $
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

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::xml;

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
    const OControlElement::ElementType& operator ++(OControlElement::ElementType& _e)
    {
        sal_Int32 nAsInt = static_cast<sal_Int32>(_e);
        _e = static_cast<OControlElement::ElementType>( ++nAsInt );
        return _e;
    }

    /** helper class which allows fast translation of xml tag names into element types.
    */
    class OElementNameMap : public OControlElement
    {
    protected:
        DECLARE_STL_USTRINGACCESS_MAP( ElementType, MapString2Element );
        static MapString2Element    s_sElementTranslations;

    protected:
        OElementNameMap() { }

    public:
        static ElementType getElementType(const ::rtl::OUString& _rName);
    };

    //---------------------------------------------------------------------
    OElementNameMap::MapString2Element  OElementNameMap::s_sElementTranslations;

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
    OElementImport::OElementImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            const Reference< XNameContainer >& _rxParentContainer,
            const Reference< XMultiServiceFactory >& _rxORB)
        :OPropertyImport(_rImport, _nPrefix, _rName, _rAttributeMap)
        ,m_xParentContainer(_rxParentContainer)
        ,m_xORB(_rxORB)
    {
        OSL_ENSURE(m_xParentContainer.is(), "OElementImport::OElementImport: invalid parent container!");
        OSL_ENSURE(m_xORB.is(), "OElementImport::OElementImport: invalid service factory!");
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
            Reference< XInterface > xPure = m_xORB->createInstance(m_sServiceName);
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
    OControlImport::OControlImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            const Reference< XNameContainer >& _rxParentContainer, const Reference< XMultiServiceFactory >& _rxORB)
        :OElementImport(_rImport, _nPrefix, _rName, _rAttributeMap, _rxParentContainer, _rxORB)
    {
    }

    //---------------------------------------------------------------------
    void OControlImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        static const ::rtl::OUString s_sControlIdAttribute = ::rtl::OUString::createFromAscii(getCommonControlAttributeName(CCA_CONTROL_ID));
        if (!m_sControlId.getLength() && (_rLocalName == s_sControlIdAttribute))
        {   // it's the control id
            m_sControlId = _rValue;
        }
        else
            OElementImport::handleAttribute(_nNamespaceKey, _rLocalName, _rValue);
    }

    //=====================================================================
    //= OListAndComboImport
    //=====================================================================
    //---------------------------------------------------------------------
    OListAndComboImport::OListAndComboImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            OControlElement::ElementType _eType,
            const Reference< XNameContainer >& _rxParentContainer,
            const Reference< XMultiServiceFactory >& _rxORB)
        :OControlImport(_rImport, _nPrefix, _rName, _rAttributeMap, _rxParentContainer, _rxORB)
        ,m_eElementType(_eType)
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
            GetPrefix(), ::rtl::OUString::createFromAscii("current-selected"));
        const ::rtl::OUString sDefaultSelectedAttribute = GetImport().GetNamespaceMap().GetQNameByIndex(
            GetPrefix(), ::rtl::OUString::createFromAscii("selected"));

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
    //= OFormImport
    //=====================================================================
    //---------------------------------------------------------------------
    OFormImport::OFormImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            const ::vos::ORef< OAttribute2Property >& _rAttributeMap,
            const Reference< XNameContainer >& _rxParentContainer, const Reference< XMultiServiceFactory >& _rxORB)
        :OElementImport(_rImport, _nPrefix, _rName, _rAttributeMap, _rxParentContainer, _rxORB)
    {
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OFormImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        // maybe it's a sub control
        OControlElement::ElementType eType = OElementNameMap::getElementType(_rLocalName);
        if (OControlElement::UNKNOWN != eType)
        {
            if (m_xElement.is())
            {
                switch (eType)
                {
                    case OControlElement::COMBOBOX:
                    case OControlElement::LISTBOX:
                        return new OListAndComboImport(GetImport(), _nPrefix, _rLocalName,
                            m_xAttributeMap, eType, Reference< XNameContainer >(m_xElement, UNO_QUERY), m_xORB);
                    default:
                        return new OControlImport(GetImport(), _nPrefix, _rLocalName,
                            m_xAttributeMap, Reference< XNameContainer >(m_xElement, UNO_QUERY), m_xORB);
                        // we already checked (in createElement) that the element is an XNameContainer
                }
            }
            else
                OSL_ENSURE(sal_False, "OFormImport::CreateChildContext: don't have an element which is a XNameContainer!");
        }

        return OElementImport::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
    }

    //---------------------------------------------------------------------
    Reference< XPropertySet > OFormImport::createElement()
    {
        // let the base class create the object
        Reference< XPropertySet > xReturn = OElementImport::createElement();
        if (!xReturn.is())
            return xReturn;

        // ensure that the object is a XNameContainer (we strongly need this for inserting child elements)
        if (!Reference< XNameContainer >(xReturn, UNO_QUERY).is())
            xReturn.clear();

        return xReturn;
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 05.12.00 11:09:36  fs
 ************************************************************************/

