/*************************************************************************
 *
 *  $RCSfile: propertyexport.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-29 12:18:58 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>

#ifndef _XMLOFF_FORMS_PROPERTYEXPORT_HXX_
#include "propertyexport.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif

#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    // NO using namespace ...util !!!
    // need a tools Date/Time/DateTime below, which would conflict with the uno types then

    using namespace ::comphelper;

    //=====================================================================
    //= iterating through sequences
    //=====================================================================
    class IIterator
    {
    public:
        virtual sal_Bool hasMoreElements() const = 0;
        virtual Any nextElement() = 0;

        virtual ~IIterator() { }
    };

    /** a helper class for iterating through a sequence
    */
    template <class TYPE>
    class OSequenceIterator : public IIterator
    {
        const TYPE* m_pElements;
        sal_Int32   m_nLen;
        const TYPE* m_pCurrent;

    public:
        /** contrcuct a sequence iterator from a sequence
        */
        OSequenceIterator(const Sequence< TYPE >& _rSeq);
        /** contrcuct a sequence iterator from a Any containing a sequence
        */
        OSequenceIterator(const Any& _rSequenceAny);

        virtual sal_Bool hasMoreElements() const;
        virtual Any nextElement();

    protected:
        void construct(const Sequence< TYPE >& _rSeq);
    };

    //---------------------------------------------------------------------
    template <class TYPE>
    OSequenceIterator<TYPE>::OSequenceIterator(const Sequence< TYPE >& _rSeq)
        :m_pElements(NULL)
        ,m_nLen(0)
        ,m_pCurrent(NULL)
    {
        construct(_rSeq);
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    OSequenceIterator<TYPE>::OSequenceIterator(const ::com::sun::star::uno::Any& _rSequenceAny)
        :m_pElements(NULL)
        ,m_nLen(0)
        ,m_pCurrent(NULL)
    {
        ::com::sun::star::uno::Sequence< TYPE > aContainer;
    #ifdef DBG_UTIL
        sal_Bool bSuccess =
    #endif
        _rSequenceAny >>= aContainer;
    #ifdef DBG_UTIL
        OSL_ENSURE(bSuccess, "OSequenceIterator::OSequenceIterator: invalid Any!");
    #endif
        construct(aContainer);
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    void OSequenceIterator<TYPE>::construct(const ::com::sun::star::uno::Sequence< TYPE >& _rSeq)
    {
        m_pElements = _rSeq.getConstArray();
        m_nLen = _rSeq.getLength();
        m_pCurrent = m_pElements;
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    sal_Bool OSequenceIterator<TYPE>::hasMoreElements() const
    {
        return m_pCurrent - m_pElements < m_nLen;
    }

    //---------------------------------------------------------------------
    template <class TYPE>
    ::com::sun::star::uno::Any OSequenceIterator<TYPE>::nextElement()
    {
        return ::com::sun::star::uno::makeAny(*m_pCurrent++);
    }

    //=====================================================================
    //= OPropertyExport
    //=====================================================================
    //---------------------------------------------------------------------
    OPropertyExport::OPropertyExport(IFormsExportContext& _rContext, const Reference< XPropertySet >& _rxProps)
        :m_rContext(_rContext)
        ,m_xProps(_rxProps)
    {
        // caching
        ::rtl::OUStringBuffer aBuffer;
        m_rContext.getGlobalContext().GetMM100UnitConverter().convertBool(aBuffer, sal_True);
        m_sValueTrue = aBuffer.makeStringAndClear();
        m_rContext.getGlobalContext().GetMM100UnitConverter().convertBool(aBuffer, sal_False);
        m_sValueFalse = aBuffer.makeStringAndClear();

        m_xPropertyInfo = m_xProps->getPropertySetInfo();
        OSL_ENSURE(m_xPropertyInfo.is(), "OPropertyExport::OPropertyExport: need an XPropertySetInfo!");

        // collect the properties which need to be exported
        examinePersistence();
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportRemainingProperties()
    {
        // the properties tag
        SvXMLElementExport aPropertiesTag(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "properties", sal_True, sal_True);

        Reference< XPropertyState > xPropertyState(m_xProps, UNO_QUERY);

        Any aValue;
        ::rtl::OUString sValue;
        ::rtl::OUString sTypeDescription;

        // loop through all the properties which are yet to be exported
        for (   ConstStringSetIterator  aProperty = m_aRemainingProps.begin();
                aProperty != m_aRemainingProps.end();
                ++aProperty
            )
        {
            DBG_CHECK_PROPERTY_NO_TYPE(*aProperty);

#ifdef DBG_UTIL
            const ::rtl::OUString sPropertyName = *aProperty;
#endif
            // if the property state is DEFAULT, it does not need to be written
            if (xPropertyState.is() && (PropertyState_DEFAULT_VALUE == xPropertyState->getPropertyState(*aProperty)))
                continue;

            // add the name attribute
            AddAttribute(XML_NAMESPACE_FORM, "property-name", *aProperty);

            // get the value
            aValue = m_xProps->getPropertyValue(*aProperty);

            if (sal_False)
            {
                ::rtl::OUString sTemp;
                aValue >>= sTemp;
                aValue <<= makeAny(Sequence< ::rtl::OUString >(&sTemp, 1));
            }

            // is it a sequence
            sal_Bool bIsSequence = TypeClass_SEQUENCE == aValue.getValueTypeClass();
            // the type of the property, maybe reduced to the element type of a sequence
            Type aSimpleType;
            if (bIsSequence)
                aSimpleType = getSequenceElementType(aValue.getValueType());
            else
                aSimpleType = aValue.getValueType();

            // the type attribute
            AddAttribute(XML_NAMESPACE_FORM, "property-type", implGetPropertyXMLType(aValue.getValueType()));

            if (bIsSequence)
                // we have a special attribute indicating that the property is a list
                AddAttribute(XML_NAMESPACE_FORM, "property-is-list", m_sValueTrue);

            // start the property tag
            SvXMLElementExport aValueTag(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "property", sal_True, sal_True);

            if (!bIsSequence)
            {   // the simple case
                sValue = implConvertAny(aValue);

                SvXMLElementExport aValueTag(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "property-value", sal_True, sal_False);
                    // (no whitespace inside the tag)
                m_rContext.getGlobalContext().GetDocHandler()->characters(sValue);
                // done with this property
                continue;
            }

            // the not-that-simple case, we need to iterate through the sequence elements
            IIterator* pSequenceIterator = NULL;
            switch (aSimpleType.getTypeClass())
            {
                case TypeClass_STRING:
                    pSequenceIterator = new OSequenceIterator< ::rtl::OUString >(aValue);
                    break;
                case TypeClass_DOUBLE:
                    pSequenceIterator = new OSequenceIterator< double >(aValue);
                    break;
                case TypeClass_BOOLEAN:
                    pSequenceIterator = new OSequenceIterator< sal_Bool >(aValue);
                    break;
                case TypeClass_BYTE:
                    pSequenceIterator = new OSequenceIterator< sal_Int8 >(aValue);
                    break;
                case TypeClass_SHORT:
                    pSequenceIterator = new OSequenceIterator< sal_Int16 >(aValue);
                    break;
                case TypeClass_LONG:
                    pSequenceIterator = new OSequenceIterator< sal_Int32 >(aValue);
                    break;
                case TypeClass_HYPER:
                    pSequenceIterator = new OSequenceIterator< sal_Int64 >(aValue);
                    break;
                default:
                    OSL_ENSURE(sal_False, "OPropertyExport::exportRemainingProperties: unsupported sequence tyoe !");
                    break;
            }
            if (pSequenceIterator)
            {
                ::rtl::OUString sCurrent;
                while (pSequenceIterator->hasMoreElements())
                {
                    SvXMLElementExport aValueTag(m_rContext.getGlobalContext(), XML_NAMESPACE_FORM, "property-value", sal_True, sal_False);
                        // (no whitespace inside the tag)
                    m_rContext.getGlobalContext().GetDocHandler()->characters(implConvertAny(pSequenceIterator->nextElement()));
                }
            }
            delete pSequenceIterator;

        }
    }

    //---------------------------------------------------------------------
    void OPropertyExport::examinePersistence()
    {
        m_aRemainingProps.clear();
        Sequence< Property > aProperties = m_xPropertyInfo->getProperties();
        const Property* pProperties = aProperties.getConstArray();
        for (sal_Int32 i=0; i<aProperties.getLength(); ++i, ++pProperties)
        {
            static sal_Int32 nExcludeIndicator = PropertyAttribute::READONLY | PropertyAttribute::TRANSIENT;
            if (pProperties->Attributes & nExcludeIndicator)
                // no readonly props, no transient props
                continue;
            m_aRemainingProps.insert(pProperties->Name);
        }
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportStringPropertyAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName)
    {
        DBG_CHECK_PROPERTY(_pPropertyName, ::rtl::OUString);

        // no try-catch here, this would be to expensive. The outer scope has to handle exceptions (which should not
        // happen if we're used correctly :)

        // this is way simple, as we don't need to convert anything (the property already is a string)

        ::rtl::OUString sPropertyName(::rtl::OUString::createFromAscii(_pPropertyName));
        // get the string
        ::rtl::OUString sPropValue;
        m_xProps->getPropertyValue(sPropertyName) >>= sPropValue;

        // add the attribute
        if (sPropValue.getLength())
            AddAttribute(_nNamespaceKey, _pAttributeName, sPropValue);

        // the property does not need to be handled anymore
        exportedProperty(sPropertyName);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportBooleanPropertyAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName, const sal_Int8 _nBooleanAttributeFlags)
    {
        DBG_CHECK_PROPERTY_NO_TYPE(_pPropertyName);
        // no check of the property value type: this method is allowed to be called with any interger properties
        // (e.g. sal_Int32, sal_uInt16 etc)

        sal_Bool bDefault = (BOOLATTR_DEFAULT_TRUE == (BOOLATTR_DEFAULT_MASK & _nBooleanAttributeFlags));
        sal_Bool bDefaultVoid = (BOOLATTR_DEFAULT_VOID == (BOOLATTR_DEFAULT_MASK & _nBooleanAttributeFlags));

        // get the value
        sal_Bool bCurrentValue = bDefault;
        ::rtl::OUString sPropertyName(::rtl::OUString::createFromAscii(_pPropertyName));
        Any aCurrentValue = m_xProps->getPropertyValue(sPropertyName);
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
        exportedProperty(sPropertyName);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportInt16PropertyAttribute(const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName, const sal_Int16 _nDefault)
    {
        DBG_CHECK_PROPERTY(_pPropertyName, sal_Int16);

        // get the value
        sal_Int16 nCurrentValue(_nDefault);
        ::rtl::OUString sPropertyName(::rtl::OUString::createFromAscii(_pPropertyName));
        m_xProps->getPropertyValue(sPropertyName) >>= nCurrentValue;

        // add the attribute
        if (_nDefault != nCurrentValue)
        {
            // let the formatter of the export context build a string
            ::rtl::OUStringBuffer sBuffer;
            m_rContext.getGlobalContext().GetMM100UnitConverter().convertNumber(sBuffer, (sal_Int32)nCurrentValue);

            AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
        }

        // the property does not need to be handled anymore
        exportedProperty(sPropertyName);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportEnumPropertyAttribute(
            const sal_uInt16 _nNamespaceKey, const sal_Char* _pAttributeName,
            const sal_Char* _pPropertyName, const SvXMLEnumMapEntry* _pValueMap, const sal_Int32 _nDefault)
    {
        // get the value
        sal_Int32 nCurrentValue(_nDefault);
        ::rtl::OUString sPropertyName(::rtl::OUString::createFromAscii(_pPropertyName));
        ::cppu::enum2int(nCurrentValue, m_xProps->getPropertyValue(sPropertyName));

        // add the attribute
        if (_nDefault != nCurrentValue)
        {
            // let the formatter of the export context build a string
            ::rtl::OUStringBuffer sBuffer;
            m_rContext.getGlobalContext().GetMM100UnitConverter().convertEnum(sBuffer, (sal_uInt16)nCurrentValue, _pValueMap);

            AddAttribute(_nNamespaceKey, _pAttributeName, sBuffer.makeStringAndClear());
        }

        // the property does not need to be handled anymore
        exportedProperty(sPropertyName);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportTargetFrameAttribute()
    {
        DBG_CHECK_PROPERTY((const sal_Char*)PROPERTY_TARGETFRAME, ::rtl::OUString);

        ::rtl::OUString sTargetFrame = comphelper::getString(m_xProps->getPropertyValue(PROPERTY_TARGETFRAME));
        if (0 != sTargetFrame.compareToAscii("_blank"))
        {   // an empty string and "_blank" have the same meaning and don't have to be written
            AddAttribute(getCommonControlAttributeNamespace(CCA_TARGET_FRAME), getCommonControlAttributeName(CCA_TARGET_FRAME), sTargetFrame);
        }

        exportedProperty(PROPERTY_TARGETFRAME);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::flagStyleProperties()
    {
        // flag all the properties which are part of the style as "handled"
        UniReference< XMLPropertySetMapper > xStylePropertiesSupplier = m_rContext.getStylePropertyMapper()->getPropertySetMapper();
        for (sal_Int32 i=0; i<xStylePropertiesSupplier->GetEntryCount(); ++i)
            exportedProperty(xStylePropertiesSupplier->GetEntryAPIName(i));

        // the font properties are exported as single properties, but there is a FontDescriptor property which
        // collects them all-in-one, this has been exported implicitly
        exportedProperty(PROPERTY_FONT);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::exportGenericPropertyAttribute(
            const sal_uInt16 _nAttributeNamespaceKey, const sal_Char* _pAttributeName, const sal_Char* _pPropertyName)
    {
        DBG_CHECK_PROPERTY_NO_TYPE(_pPropertyName);

        ::rtl::OUString sPropertyName = ::rtl::OUString::createFromAscii(_pPropertyName);
        exportedProperty(sPropertyName);

        Any aCurrentValue = m_xProps->getPropertyValue(sPropertyName);
        if (!aCurrentValue.hasValue())
            // nothing to do without a concrete value
            return;

        ::rtl::OUString sValue = implConvertAny(aCurrentValue);
        if (!sValue.getLength() && (TypeClass_STRING == aCurrentValue.getValueTypeClass()))
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

    //---------------------------------------------------------------------
    void OPropertyExport::exportStringSequenceAttribute(const sal_uInt16 _nAttributeNamespaceKey, const sal_Char* _pAttributeName,
        const sal_Char* _pPropertyName,
        const sal_Unicode _aQuoteCharacter, const sal_Unicode _aListSeparator)
    {
        DBG_CHECK_PROPERTY(_pPropertyName, Sequence< ::rtl::OUString >);
        OSL_ENSURE(_aListSeparator != 0, "OPropertyExport::exportStringSequenceAttribute: invalid separator character!");

        ::rtl::OUString sPropertyName(::rtl::OUString::createFromAscii(_pPropertyName));
        Sequence< ::rtl::OUString > aItems;
        m_xProps->getPropertyValue(sPropertyName) >>= aItems;

        ::rtl::OUString sFinalList;

        // unfortunately the OUString can't append single sal_Unicode characters ...
        const ::rtl::OUString sQuote(&_aQuoteCharacter, 1);
        const ::rtl::OUString sSeparator(&_aListSeparator, 1);
        const sal_Bool bQuote = 0 != sQuote.getLength();

        // concatenate the string items
        const ::rtl::OUString* pItems = aItems.getConstArray();
        const ::rtl::OUString* pEnd = pItems + aItems.getLength();
        const ::rtl::OUString* pLastElement = pEnd - 1;
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

        if (sFinalList.getLength())
            AddAttribute(_nAttributeNamespaceKey, _pAttributeName, sFinalList);

        exportedProperty(sPropertyName);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OPropertyExport::implConvertAny(const Any& _rValue)
    {
        ::rtl::OUStringBuffer aBuffer;
        switch (_rValue.getValueTypeClass())
        {
            case TypeClass_STRING:
            {   // extract the string
                ::rtl::OUString sCurrentValue;
                _rValue >>= sCurrentValue;
                aBuffer.append(sCurrentValue);
            }
            break;
            case TypeClass_DOUBLE:
                // let the unit converter format is as string
                m_rContext.getGlobalContext().GetMM100UnitConverter().convertDouble(aBuffer, getDouble(_rValue));
                break;
            case TypeClass_BOOLEAN:
                aBuffer = getBOOL(_rValue) ? m_sValueTrue : m_sValueFalse;
                break;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
            case TypeClass_LONG:
                // let the unit converter format is as string
                m_rContext.getGlobalContext().GetMM100UnitConverter().convertNumber(aBuffer, getINT32(_rValue));
                break;
            case TypeClass_HYPER:
                // TODO
                OSL_ENSURE(sal_False, "OPropertyExport::implConvertAny: missing implementation for sal_Int64!");
                break;
            case TypeClass_ENUM:
            {
                // convert it into an int32
                sal_Int32 nValue;
                ::cppu::enum2int(nValue, _rValue);
                m_rContext.getGlobalContext().GetMM100UnitConverter().convertNumber(aBuffer, getINT32(_rValue));
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
                    Date aToolsDate;
                    ::utl::typeConvert(aDate, aToolsDate);
                    fValue = aToolsDate.GetDate();
                }
                else if (_rValue >>= aTime)
                {
                    fValue = ((aTime.Hours * 60 + aTime.Minutes) * 60 + aTime.Seconds) * 100 + aTime.HundredthSeconds;
                    fValue = fValue / 8640000.0;
                }
                else if (_rValue >>= aDateTime)
                {
                    DateTime aToolsDateTime;
                    ::utl::typeConvert(aDateTime, aToolsDateTime);
                    // the time part (the digits behind the comma)
                    fValue = ((aDateTime.Hours * 60 + aDateTime.Minutes) * 60 + aDateTime.Seconds) * 100 + aDateTime.HundredthSeconds;
                    fValue = fValue / 8640000.0;
                    // plus the data part (the digits in front of the comma)
                    fValue += aToolsDateTime.GetDate();
                }
                else
                {
                    // if any other types are added here, please remember to adjust implGetPropertyXMLType accordingly

                    // no more options ...
                    OSL_ENSURE(sal_False, "OPropertyExport::implConvertAny: unsupported value type!");
                    break;
                }
                // let the unit converter format is as string
                m_rContext.getGlobalContext().GetMM100UnitConverter().convertDouble(aBuffer, fValue);
            }
            break;
        }

        return aBuffer.makeStringAndClear();
    }


    //---------------------------------------------------------------------
    ::rtl::OUString OPropertyExport::implGetPropertyXMLType(const ::com::sun::star::uno::Type& _rType)
    {
        // possible types we can write (either because we recognize them directly or because we convert _rValue
        // into one of these types)
        static const ::rtl::OUString s_sTypeBoolean (RTL_CONSTASCII_USTRINGPARAM("boolean"));
        static const ::rtl::OUString s_sTypeShort   (RTL_CONSTASCII_USTRINGPARAM("short"));
        static const ::rtl::OUString s_sTypeInteger (RTL_CONSTASCII_USTRINGPARAM("int"));
        static const ::rtl::OUString s_sTypeLong    (RTL_CONSTASCII_USTRINGPARAM("long"));
        static const ::rtl::OUString s_sTypeDouble  (RTL_CONSTASCII_USTRINGPARAM("double"));
        static const ::rtl::OUString s_sTypeString  (RTL_CONSTASCII_USTRINGPARAM("string"));

        // handle the type description
        switch (_rType.getTypeClass())
        {
            case TypeClass_STRING:
                return s_sTypeString;
            case TypeClass_DOUBLE:
                return s_sTypeDouble;
            case TypeClass_BOOLEAN:
                return s_sTypeBoolean;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
                return s_sTypeShort;
            case TypeClass_LONG:
                return s_sTypeInteger;
            case TypeClass_HYPER:
                return s_sTypeLong;
            case TypeClass_ENUM:
                return s_sTypeInteger;
            default:
                return s_sTypeDouble;
        }
    }

#ifdef DBG_UTIL
    //---------------------------------------------------------------------
    void OPropertyExport::AddAttribute(sal_uInt16 _nPrefix, const sal_Char* _pName, const ::rtl::OUString& _rValue)
    {
        OSL_ENSURE(0 == m_rContext.getGlobalContext().GetXAttrList()->getValueByName(::rtl::OUString::createFromAscii(_pName)).getLength(),
            "OPropertyExport::AddAttribute: already have such an attribute");

        m_rContext.getGlobalContext().AddAttribute(_nPrefix, _pName, _rValue);
    }

    //---------------------------------------------------------------------
    void OPropertyExport::dbg_implCheckProperty(const ::rtl::OUString& _rPropertyName, const Type* _pType)
    {
        try
        {
            // the property must exist
            if (!m_xPropertyInfo->hasPropertyByName(_rPropertyName))
            {
                OSL_ENSURE(sal_False,
                    ::rtl::OString("OPropertyExport::dbg_implCheckProperty: no property with the name ") +=
                    ::rtl::OString(_rPropertyName.getStr(), _rPropertyName.getLength(), RTL_TEXTENCODING_ASCII_US) +=
                    ::rtl::OString("!"));
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
            OSL_ENSURE(sal_False, "OPropertyExport::dbg_implCheckProperty: caught an exception, could not check the property!");
        }
    }
#endif // DBG_UTIL - dbg_implCheckProperty

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.13  2001/03/28 13:58:52  fs
 *  #85371# write target frame attribute even if the prop value is an empty string
 *
 *  Revision 1.12  2001/03/16 14:36:39  sab
 *  did the required change (move of extract.hxx form cppuhelper to comphelper)
 *
 *  Revision 1.11  2001/02/26 10:28:04  aw
 *  Changed double import/export to use it's own conversion routines
 *  so iots more clear what type is used
 *
 *  Revision 1.10  2001/02/01 09:46:47  fs
 *  no own style handling anymore - the shape exporter is responsible for our styles now
 *
 *  Revision 1.9  2001/01/03 16:25:34  fs
 *  file format change (extra wrapper element for controls, similar to columns)
 *
 *  Revision 1.8  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.7  2000/12/18 13:25:01  mib
 *  #82036#: new graphic properties
 *
 *  Revision 1.6  2000/12/13 10:38:49  fs
 *  slightly modified some documentations
 *
 *  Revision 1.5  2000/12/12 11:58:32  fs
 *  generic properies: writer shorts really as shorts
 *
 *  Revision 1.4  2000/12/06 17:28:05  fs
 *  changes for the formlayer import - still under construction
 *
 *  Revision 1.3  2000/11/29 10:36:05  mh
 *  add: stdio.h for Solaris8
 *
 *  Revision 1.2  2000/11/19 15:41:32  fs
 *  extended the export capabilities - generic controls / grid columns / generic properties / some missing form properties
 *
 *  Revision 1.1  2000/11/17 19:03:06  fs
 *  initial checkin - export and/or import the applications form layer
 *
 *
 *  Revision 1.0 15.11.00 17:50:20  fs
 ************************************************************************/

