/*************************************************************************
 *
 *  $RCSfile: propertyimport.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-26 10:28:04 $
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

#ifndef _XMLOFF_FORMS_PROPERTYIMPORT_HXX_
#include "propertyimport.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
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
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml;

    // NO using namespace ...util !!!
    // need a tools Date/Time/DateTime below, which would conflict with the uno types then

#define TYPE_DATE       1
#define TYPE_TIME       2
#define TYPE_DATETIME   3

    //=====================================================================
    //= OPropertyImport
    //=====================================================================
    //---------------------------------------------------------------------
    OPropertyImport::OPropertyImport(IFormsImportContext& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName)
        :SvXMLImportContext(_rImport.getGlobalContext(), _nPrefix, _rName)
        ,m_rContext(_rImport)
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
                +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=  ::rtl::OString(")!"));
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
        }

        // TODO: create PropertyValues for all the attributes which were not present, because they were implied
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
            aNewValue.Value = convertString(m_rContext.getGlobalContext(), pProperty->aPropertyType, _rValue, pProperty->pEnumMap);
            m_aValues.push_back(aNewValue);
        }
        else
#if SUPD<622
        if (0 != _rLocalName.compareToAscii("style-name"))
#endif
        {
            OSL_ENSURE(sal_False, "OPropertyImport::handleAttribute: can't handle attributes which do not describe properties or the style!");
        }
    }

    //---------------------------------------------------------------------
    Any OPropertyImport::convertString(SvXMLImport& _rImporter, const ::com::sun::star::uno::Type& _rExpectedType, const ::rtl::OUString& _rReadCharacters, const SvXMLEnumMapEntry* _pEnumMap)
    {
        Any aReturn;
        sal_Bool bEnumAsInt = sal_False;
        switch (_rExpectedType.getTypeClass())
        {
            case TypeClass_BOOLEAN:     // sal_Bool
            {
                sal_Bool bValue;
            #ifdef _DEBUG
                sal_Bool bSuccess =
            #endif
                _rImporter.GetMM100UnitConverter().convertBool(bValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                    +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("\" into a boolean!"));
                aReturn = ::cppu::bool2any(bValue);
            }
            break;
            case TypeClass_SHORT:       // sal_Int16
            case TypeClass_LONG:        // sal_Int32
                if (!_pEnumMap)
                {   // it's a real int32/16 property
                    sal_Int32 nValue(0);
            #ifdef _DEBUG
                    sal_Bool bSuccess =
            #endif
                    _rImporter.GetMM100UnitConverter().convertNumber(nValue, _rReadCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                        +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=  ::rtl::OString("\" into an integer!"));
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
            #ifdef _DEBUG
                sal_Bool bSuccess =
            #endif
                _rImporter.GetMM100UnitConverter().convertEnum(nEnumValue, _rReadCharacters, _pEnumMap);
                OSL_ENSURE(bSuccess, "OPropertyImport::convertString: could not convert to an enum value!");
                if (bEnumAsInt)
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
            #ifdef _DEBUG
                sal_Bool bSuccess =
            #endif
                _rImporter.GetMM100UnitConverter().convertDouble(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                    +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("\" into a double!"));
                aReturn <<= (double)nValue;
            }
            break;
            case TypeClass_STRING:
                aReturn <<= _rReadCharacters;
                break;
            case TypeClass_STRUCT:
            {
                // recognized structs:
                static ::com::sun::star::uno::Type s_aDateType      = ::getCppuType(static_cast< ::com::sun::star::util::Date* >(NULL));
                static ::com::sun::star::uno::Type s_aTimeType      = ::getCppuType(static_cast< ::com::sun::star::util::Time* >(NULL));
                static ::com::sun::star::uno::Type s_aDateTimeType  = ::getCppuType(static_cast< ::com::sun::star::util::DateTime* >(NULL));
                sal_Int32 nType = 0;
                if  (   (_rExpectedType.equals(s_aDateType) && (nType = TYPE_DATE))
                    ||  (_rExpectedType.equals(s_aTimeType) && (nType = TYPE_TIME))
                    ||  (_rExpectedType.equals(s_aDateTimeType) && (nType = TYPE_DATETIME))
                    )
                {
                    // first extract the double
                    double nValue = 0;
                #ifdef _DEBUG
                    sal_Bool bSuccess =
                #endif
                    _rImporter.GetMM100UnitConverter().convertDouble(nValue, _rReadCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                        +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=  ::rtl::OString("\" into a double!"));

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
        sal_Int32 nIntValue = _nValue * 8640000.0;
        nIntValue *= 8640000.0;
        aTime.HundredthSeconds = nIntValue % 100;
        nIntValue /= 100;
        aTime.Seconds = nIntValue % 60;
        nIntValue /= 60;
        aTime.Minutes = nIntValue % 60;
        nIntValue /= 60;
        OSL_ENSURE(nIntValue < 24, "OPropertyImport::implGetTime: more than a day?");
        aTime.Hours = nIntValue;

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
                +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=  ::rtl::OString("\")!"));
            return new SvXMLImportContext(GetImport(), _nPrefix, _rLocalName);
        }
    }

#ifdef _DEBUG
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
                +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                +=  ::rtl::OString("\")!"));
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
            s_aTypeNameMap[::rtl::OUString::createFromAscii("boolean")] = ::getBooleanCppuType();
            s_aTypeNameMap[::rtl::OUString::createFromAscii("short")]   = ::getCppuType( static_cast< sal_Int16* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("int")]     = ::getCppuType( static_cast< sal_Int32* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("long")]    = ::getCppuType( static_cast< sal_Int64* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("double")]  = ::getCppuType( static_cast< double* >(NULL) );
            s_aTypeNameMap[::rtl::OUString::createFromAscii("string")]  = ::getCppuType( static_cast< ::rtl::OUString* >(NULL) );
        }

        const ConstMapString2TypeIterator aTypePos = s_aTypeNameMap.find(sType);
        OSL_ENSURE(s_aTypeNameMap.end() != aTypePos, "OSinglePropertyContext::StartElement: invalid property name!");
        if (s_aTypeNameMap.end() != aTypePos)
            m_aPropType = aTypePos->second;
    }

#ifdef _DEBUG
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
            ::rtl::OUString sCharacters = m_xValueReader->getCharacters();
            // convert these characters into the property value target type
            m_aPropValue.Value = OPropertyImport::convertString(GetImport(), m_aPropType, sCharacters);
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
    {
    }

    //---------------------------------------------------------------------
    void OAccumulateCharacters::Characters(const ::rtl::OUString& _rChars)
    {
        m_sCharacters += _rChars;
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2001/02/13 09:10:03  fs
 *  prevent an assertion because of an outdated file format ('til SUPD>=622)
 *
 *  Revision 1.5  2001/02/01 09:46:47  fs
 *  no own style handling anymore - the shape exporter is responsible for our styles now
 *
 *  Revision 1.4  2000/12/18 15:14:35  fs
 *  some changes ... now exporting/importing styles
 *
 *  Revision 1.3  2000/12/13 10:40:15  fs
 *  new import related implementations - at this version, we should be able to import everything we export (which is all except events and styles)
 *
 *  Revision 1.2  2000/12/12 12:01:05  fs
 *  new implementations for the import - still under construction
 *
 *  Revision 1.1  2000/12/06 17:31:52  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 04.12.00 15:36:58  fs
 ************************************************************************/

