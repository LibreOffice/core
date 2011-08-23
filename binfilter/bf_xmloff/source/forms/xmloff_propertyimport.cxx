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

#include "propertyimport.hxx"
#include "xmlimp.hxx"
#include "xmluconv.hxx"
#include "nmspmap.hxx"
#include <osl/diagnose.h>
#include <comphelper/extract.hxx>
#include "callbacks.hxx"
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml;

    // NO using namespace ...util !!!
    // need a tools Date/Time/DateTime below, which would conflict with the uno types then

#define TYPE_DATE		1
#define TYPE_TIME		2
#define TYPE_DATETIME	3

    //=====================================================================
    //= OPropertyImport
    //=====================================================================
    //---------------------------------------------------------------------
    OPropertyImport::OPropertyImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName)
        :SvXMLImportContext(_rImport.getGlobalContext(), _nPrefix, _rName)
        ,m_rContext(_rImport)
        ,m_bTrackAttributes(sal_False)
    {
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OPropertyImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        if (0 == _rLocalName.compareToAscii("properties"))
        {
            return new OPropertyElementsContext(m_rContext.getGlobalContext(), _nPrefix, _rLocalName, this);
        }
        else
        {
            OSL_ENSURE(sal_False,
                    ::rtl::OString("OPropertyImport::CreateChildContext: unknown sub element (only \"properties\" is recognized, but it is ")
                +=	::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=	::rtl::OString(")!"));
            return SvXMLImportContext::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
        }
    }

    //---------------------------------------------------------------------
    void OPropertyImport::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        OSL_ENSURE(_rxAttrList.is(), "OPropertyImport::StartElement: invalid attribute list!");
        sal_Int32 nAttributeCount = _rxAttrList->getLength();

        // assume the 'worst' case: all attributes describe properties. This should save our property array
        // some reallocs
        m_aValues.reserve(nAttributeCount);

        sal_uInt16 nNamespace;
        ::rtl::OUString sLocalName;
        for (sal_Int16 i=0; i<nAttributeCount; ++i)
        {
            nNamespace = m_rContext.getGlobalContext().GetNamespaceMap().GetKeyByAttrName(_rxAttrList->getNameByIndex(i), &sLocalName);
            handleAttribute(nNamespace, sLocalName, _rxAttrList->getValueByIndex(i));

            if (m_bTrackAttributes)
                m_aEncounteredAttributes.insert(sLocalName);
        }

        // TODO: create PropertyValues for all the attributes which were not present, because they were implied
    }

    //---------------------------------------------------------------------
    sal_Bool OPropertyImport::encounteredAttribute(const ::rtl::OUString& _rAttributeName) const
    {
        OSL_ENSURE(m_bTrackAttributes, "OPropertyImport::encounteredAttribute: attribute tracking not enabled!");
        return m_aEncounteredAttributes.end() != m_aEncounteredAttributes.find(_rAttributeName);
    }

    //---------------------------------------------------------------------
    void OPropertyImport::Characters(const ::rtl::OUString& _rChars)
    {
        // ignore them (should be whitespaces only)
        OSL_ENSURE(0 == _rChars.trim().getLength(), "OPropertyImport::Characters: non-whitespace characters!");
    }

    //---------------------------------------------------------------------
    void OPropertyImport::handleAttribute(sal_uInt16 _nNamespaceKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue)
    {
        const OAttribute2Property::AttributeAssignment* pProperty = m_rContext.getAttributeMap().getAttributeTranslation(_rLocalName);
        if (pProperty)
        {
            // create and store a new PropertyValue
            PropertyValue aNewValue;
            aNewValue.Name = pProperty->sPropertyName;

            // convert the value string into the target type
            aNewValue.Value = convertString(m_rContext.getGlobalContext(), pProperty->aPropertyType, _rValue, pProperty->pEnumMap, pProperty->bInverseSemantics);
            implPushBackPropertyValue( aNewValue );
        }
        else
            OSL_ENSURE( sal_False, "OPropertyImport::handleAttribute: can't handle attributes which do not describe properties!" );
    }

    //---------------------------------------------------------------------
    Any OPropertyImport::convertString(SvXMLImport& _rImporter, const ::com::sun::star::uno::Type& _rExpectedType, const ::rtl::OUString& _rReadCharacters, const SvXMLEnumMapEntry* _pEnumMap, const sal_Bool _bInvertBoolean)
    {
        Any aReturn;
        sal_Bool bEnumAsInt = sal_False;
        switch (_rExpectedType.getTypeClass())
        {
            case TypeClass_BOOLEAN:		// sal_Bool
            {
                sal_Bool bValue;
            #if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
            #endif
                _rImporter.GetMM100UnitConverter().convertBool(bValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                    +=	::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=	::rtl::OString("\" into a boolean!"));
                aReturn = ::cppu::bool2any(_bInvertBoolean ? !bValue : bValue);
            }
            break;
            case TypeClass_SHORT:		// sal_Int16
            case TypeClass_LONG:		// sal_Int32
                if (!_pEnumMap)
                {	// it's a real int32/16 property
                    sal_Int32 nValue(0);
            #if OSL_DEBUG_LEVEL > 0
                    sal_Bool bSuccess =
            #endif
                    _rImporter.GetMM100UnitConverter().convertNumber(nValue, _rReadCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                        +=	::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=	::rtl::OString("\" into an integer!"));
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
                sal_uInt16 nEnumValue;
            #if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
            #endif
                _rImporter.GetMM100UnitConverter().convertEnum(nEnumValue, _rReadCharacters, _pEnumMap);
                OSL_ENSURE(bSuccess, "OPropertyImport::convertString: could not convert to an enum value!");
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
                OSL_ENSURE(sal_False, "OPropertyImport::convertString: 64-bit integers not implemented yet!");
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
                        ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                    +=	::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=	::rtl::OString("\" into a double!"));
                aReturn <<= (double)nValue;
            }
            break;
            case TypeClass_STRING:
                aReturn <<= _rReadCharacters;
                break;
            case TypeClass_STRUCT:
            {
                // recognized structs:
                static ::com::sun::star::uno::Type s_aDateType		= ::getCppuType(static_cast< ::com::sun::star::util::Date* >(NULL));
                static ::com::sun::star::uno::Type s_aTimeType		= ::getCppuType(static_cast< ::com::sun::star::util::Time* >(NULL));
                static ::com::sun::star::uno::Type s_aDateTimeType	= ::getCppuType(static_cast< ::com::sun::star::util::DateTime* >(NULL));
                sal_Int32 nType = 0;
                if	(	(_rExpectedType.equals(s_aDateType) && (nType = TYPE_DATE))
                    ||	(_rExpectedType.equals(s_aTimeType) && (nType = TYPE_TIME))
                    ||	(_rExpectedType.equals(s_aDateTimeType) && (nType = TYPE_DATETIME))
                    )
                {
                    // first extract the double
                    double nValue = 0;
                #if OSL_DEBUG_LEVEL > 0
                    sal_Bool bSuccess =
                #endif
                    _rImporter.GetMM100UnitConverter().convertDouble(nValue, _rReadCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                        +=	::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=	::rtl::OString("\" into a double!"));

                    // then convert it into the target type
                    switch (nType)
                    {
                        case TYPE_DATE:
                        {
                            OSL_ENSURE(((sal_uInt32)nValue) - nValue == 0,
                                "OPropertyImport::convertString: a Date value with a fractional part?");
                            aReturn <<= implGetDate(nValue);
                        }
                        break;
                        case TYPE_TIME:
                        {
                            OSL_ENSURE(((sal_uInt32)nValue) == 0,
                                "OPropertyImport::convertString: a Time value with more than a fractional part?");
                            aReturn <<= implGetTime(nValue);
                        }
                        break;
                        case TYPE_DATETIME:
                        {
                            ::com::sun::star::util::Time aTime = implGetTime(nValue);
                            ::com::sun::star::util::Date aDate = implGetDate(nValue);

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
                    OSL_ENSURE(sal_False, "OPropertyImport::convertString: unsupported property type!");
            }
            break;
            default:
                OSL_ENSURE(sal_False, "OPropertyImport::convertString: invalid type class!");
        }

        return aReturn;
    }

    //---------------------------------------------------------------------
    ::com::sun::star::util::Time OPropertyImport::implGetTime(double _nValue)
    {
        ::com::sun::star::util::Time aTime;
        sal_Int32 nIntValue = sal_Int32(_nValue * 8640000);
        nIntValue *= 8640000;
        aTime.HundredthSeconds = nIntValue % 100;
        nIntValue /= 100;
        aTime.Seconds = nIntValue % 60;
        nIntValue /= 60;
        aTime.Minutes = nIntValue % 60;
        nIntValue /= 60;
        OSL_ENSURE(nIntValue < 24, "OPropertyImport::implGetTime: more than a day?");
        aTime.Hours = (sal_uInt16)nIntValue;

        return aTime;
    }

    //---------------------------------------------------------------------
    ::com::sun::star::util::Date OPropertyImport::implGetDate(double _nValue)
    {
        Date aToolsDate((sal_uInt32)_nValue);
        ::com::sun::star::util::Date aDate;
        ::utl::typeConvert(aToolsDate, aDate);
        return aDate;
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
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        static const ::rtl::OUString s_sSinglePropertyElementName = ::rtl::OUString::createFromAscii("property");
        if (_rLocalName == s_sSinglePropertyElementName)
        {
            return new OSinglePropertyContext(GetImport(), _nPrefix, _rLocalName, m_xPropertyImporter);
        }
        else
        {
            OSL_ENSURE(sal_False,
                    ::rtl::OString("OPropertyElementsContext::CreateChildContext: unknown child element (\"")
                +=	::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=	::rtl::OString("\")!"));
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
            const Reference< sax::XAttributeList >& _rxAttrList)
    {
        static const ::rtl::OUString s_sPropertyValueElementName = ::rtl::OUString::createFromAscii("property-value");
        if (_rLocalName == s_sPropertyValueElementName)
        {
            OSL_ENSURE(!m_xValueReader.Is(), "OSinglePropertyContext::CreateChildContext: already had a value element!");
            m_xValueReader = new OAccumulateCharacters(GetImport(), _nPrefix, _rLocalName);
            return &m_xValueReader;
        }
        else
        {
            OSL_ENSURE(sal_False,
                    ::rtl::OString("OSinglePropertyContext::CreateChildContext: unknown child element (\"")
                +=	::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=	::rtl::OString("\")!"));
            return new SvXMLImportContext(GetImport(), _nPrefix, _rLocalName);
        }
    }

    //---------------------------------------------------------------------
    void OSinglePropertyContext::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        ::rtl::OUString sNameAttribute = GetImport().GetNamespaceMap().GetQNameByIndex(
            GetPrefix(), ::rtl::OUString::createFromAscii("property-name"));
        ::rtl::OUString sTypeAttribute = GetImport().GetNamespaceMap().GetQNameByIndex(
            GetPrefix(), ::rtl::OUString::createFromAscii("property-type"));

        // the name of the property
        m_aPropValue.Name = _rxAttrList->getValueByName(sNameAttribute);
        OSL_ENSURE(m_aPropValue.Name.getLength(), "OSinglePropertyContext::StartElement: invalid property name!");

        // the type of the property
        ::rtl::OUString sType = _rxAttrList->getValueByName(sTypeAttribute);

        // needs to be translated into a ::com::sun::star::uno::Type
        DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Type, MapString2Type );
        static MapString2Type s_aTypeNameMap;
        if (!s_aTypeNameMap.size())
        {
            s_aTypeNameMap[::rtl::OUString::createFromAscii("boolean")]	= ::getBooleanCppuType();
            s_aTypeNameMap[::rtl::OUString::createFromAscii("short")]	= ::getCppuType( static_cast< sal_Int16* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("int")]		= ::getCppuType( static_cast< sal_Int32* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("long")]	= ::getCppuType( static_cast< sal_Int64* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("double")]	= ::getCppuType( static_cast< double* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("string")]	= ::getCppuType( static_cast< ::rtl::OUString* >(NULL) );
        }

        const ConstMapString2TypeIterator aTypePos = s_aTypeNameMap.find(sType);
        OSL_ENSURE(s_aTypeNameMap.end() != aTypePos, "OSinglePropertyContext::StartElement: invalid property name!");
        if (s_aTypeNameMap.end() != aTypePos)
            m_aPropType = aTypePos->second;
    }

#if OSL_DEBUG_LEVEL > 0
    //---------------------------------------------------------------------
    void OSinglePropertyContext::Characters(const ::rtl::OUString& _rChars)
    {
        OSL_ENSURE(0 == _rChars.trim(), "OSinglePropertyContext::Characters: non-whitespace characters detected!");
        SvXMLImportContext::Characters(_rChars);
    }
#endif

    //---------------------------------------------------------------------
    void OSinglePropertyContext::EndElement()
    {
        OSL_ENSURE(m_xValueReader.Is(), "OSinglePropertyContext::EndElement: did not encounter a value tag!");
        if (m_xValueReader.Is())
        {
            //modified by BerryJia for Bug102407
            if (m_xValueReader->isVoid())
                m_aPropValue.Value = Any();
            else
            {
                ::rtl::OUString sCharacters = m_xValueReader->getCharacters();
                // convert these characters into the property value target type
                m_aPropValue.Value = OPropertyImport::convertString(GetImport(), m_aPropType, sCharacters);
            }
        }

        // now that we finally have our property value, add it to our parent object
        m_xPropertyImporter->implPushBackPropertyValue(m_aPropValue);
    }

    //=====================================================================
    //= OAccumulateCharacters
    //=====================================================================
    //---------------------------------------------------------------------
    OAccumulateCharacters::OAccumulateCharacters(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName)
        :SvXMLImportContext(_rImport, _nPrefix, _rName)
        //added by BerryJia for Bug102407
        ,m_bPropertyIsVoid(sal_False)
    {
    }

    //---------------------------------------------------------------------
    void OAccumulateCharacters::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        ::rtl::OUString sIsVoidAttributeName = GetImport().GetNamespaceMap().GetQNameByIndex(
            GetPrefix(), ::rtl::OUString::createFromAscii("property-is-void"));
        ::rtl::OUString sIsVoidAttributeValue = _rxAttrList->getValueByName(sIsVoidAttributeName);

        if (sIsVoidAttributeValue.getLength())
        {
            //modified by BerryJia for Bug102407
            m_bPropertyIsVoid = sal_False;
            GetImport().GetMM100UnitConverter().convertBool(m_bPropertyIsVoid, sIsVoidAttributeValue);
        }
    }
    //---------------------------------------------------------------------
    void OAccumulateCharacters::Characters(const ::rtl::OUString& _rChars)
    {
        m_sCharacters += _rChars;
    }
    //---------------------------------------------------------------------
    //added by BerryJia for Bug102407
    sal_Bool OAccumulateCharacters::isVoid()
    {
        return m_bPropertyIsVoid;
    }
//.........................................................................
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
