/*************************************************************************
 *
 *  $RCSfile: propertyimport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-12-12 12:01:05 $
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

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml;

    //=====================================================================
    //= OPropertyImport
    //=====================================================================
    //---------------------------------------------------------------------
    OPropertyImport::OPropertyImport(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
            OAttribute2Property& _rAttributeMap)
        :SvXMLImportContext(_rImport, _nPrefix, _rName)
        ,m_rAttributeMap(_rAttributeMap)
    {
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OPropertyImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        if (0 == _rLocalName.compareToAscii("properties"))
        {
            return new OPropertyElementsContext(GetImport(), _nPrefix, _rLocalName, this);
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
            nNamespace = GetImport().GetNamespaceMap().GetKeyByAttrName(_rxAttrList->getNameByIndex(i), &sLocalName);
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
        const OAttribute2Property::AttributeAssignment* pProperty = m_rAttributeMap.getAttributeTranslation(_rLocalName);
        if (pProperty)
        {
            // create and store a new PropertyValue
            PropertyValue aNewValue;
            aNewValue.Name = pProperty->sPropertyName;

            // convert the value string into the target type
            sal_Bool bEnumAsInt = sal_False;
            switch (pProperty->aPropertyType.getTypeClass())
            {
                case TypeClass_BOOLEAN:
                {
                    sal_Bool bValue(sal_False);
#ifdef _DEBUG
                    sal_Bool bSuccess =
#endif
                    GetImport().GetMM100UnitConverter().convertBool(bValue, _rValue);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OPropertyImport::handleAttribute: could not convert the value of the \"")
                        +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=  ::rtl::OString("\" attribute (\"")
                        +=  ::rtl::OString(_rValue.getStr(), _rValue.getLength(), RTL_TEXTENCODING_ASCII_US));
                        aNewValue.Value = ::cppu::bool2any(bValue);
                }
                break;
                case TypeClass_STRING:
                    aNewValue.Value <<= _rValue;
                    break;
                case TypeClass_SHORT:
                case TypeClass_LONG:
                    if (!pProperty->pEnumMap)
                    {   // it's a real int32/16 property
                        sal_Int32 nValue(0);
#ifdef _DEBUG
                        sal_Bool bSuccess =
#endif
                        GetImport().GetMM100UnitConverter().convertNumber(nValue, _rValue);
                        OSL_ENSURE(bSuccess,
                                ::rtl::OString("OPropertyImport::handleAttribute: could not convert the value of the \"")
                            +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
                            +=  ::rtl::OString("\" attribute (\"")
                            +=  ::rtl::OString(_rValue.getStr(), _rValue.getLength(), RTL_TEXTENCODING_ASCII_US));
                        if (TypeClass_SHORT == pProperty->aPropertyType.getTypeClass())
                            aNewValue.Value <<= (sal_Int16)nValue;
                        else
                            aNewValue.Value <<= (sal_Int32)nValue;
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
                    GetImport().GetMM100UnitConverter().convertEnum(nEnumValue, _rValue, pProperty->pEnumMap);
                    OSL_ENSURE(bSuccess, "OPropertyImport::handleAttribute: could not convert to an enum value!");
                    if (bEnumAsInt)
                        aNewValue.Value <<= (sal_Int32)nEnumValue;
                    else
                        aNewValue.Value = ::cppu::int2enum((sal_Int32)nEnumValue, pProperty->aPropertyType);
                }
                break;
                default:
                    OSL_ENSURE(sal_False, "OPropertyImport::handleAttribute: unknown property value type!");
            }
            m_aValues.push_back(aNewValue);
        }
        else
            OSL_ENSURE(sal_False, "OPropertyImport::handleAttribute: can't handle attributes which do not describe properties!");
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
            switch (m_aPropType.getTypeClass())
            {
                case TypeClass_BOOLEAN:     // sal_Bool
                {
                    sal_Bool bValue;
                #ifdef _DEBUG
                    sal_Bool bSuccess =
                #endif
                    GetImport().GetMM100UnitConverter().convertBool(bValue, sCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OSinglePropertyContext::EndElement: could not convert \"")
                        +=  ::rtl::OString(sCharacters.getStr(), sCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=  ::rtl::OString("\" into a boolean!"));
                    m_aPropValue.Value = ::cppu::bool2any(bValue);
                }
                break;
                case TypeClass_SHORT:       // sal_Int16
                case TypeClass_LONG:        // sal_Int32
                {
                    sal_Int32 nValue;
                #ifdef _DEBUG
                    sal_Bool bSuccess =
                #endif
                    GetImport().GetMM100UnitConverter().convertNumber(nValue, sCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OSinglePropertyContext::EndElement: could not convert \"")
                        +=  ::rtl::OString(sCharacters.getStr(), sCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=  ::rtl::OString("\" into a ninteger!"));
                    if (TypeClass_SHORT == m_aPropType.getTypeClass())
                        m_aPropValue.Value <<= (sal_Int16)nValue;
                    else
                        m_aPropValue.Value <<= (sal_Int32)nValue;
                }
                break;
                case TypeClass_HYPER:
                {
                    OSL_ENSURE(sal_False, "OSinglePropertyContext::EndElement: 64-bit integers not implemented yet!");
                }
                break;
                case TypeClass_DOUBLE:
                {
                    double nValue;
                #ifdef _DEBUG
                    sal_Bool bSuccess =
                #endif
                    GetImport().GetMM100UnitConverter().convertNumber(nValue, sCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OString("OSinglePropertyContext::EndElement: could not convert \"")
                        +=  ::rtl::OString(sCharacters.getStr(), sCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                        +=  ::rtl::OString("\" into a double!"));
                    m_aPropValue.Value <<= (double)nValue;
                }
                break;
                case TypeClass_STRING:
                    m_aPropValue.Value <<= sCharacters;
                    break;
                default:
                    OSL_ENSURE(sal_False, "OSinglePropertyContext::EndElement: invalid type class!");
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
 *  Revision 1.1  2000/12/06 17:31:52  fs
 *  initial checkin - implementations for formlayer import/export - still under construction
 *
 *
 *  Revision 1.0 04.12.00 15:36:58  fs
 ************************************************************************/

