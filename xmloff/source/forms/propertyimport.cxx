/*************************************************************************
 *
 *  $RCSfile: propertyimport.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:15:07 $
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
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
        ,m_bTrackAttributes(sal_False)
    {
    }

    //---------------------------------------------------------------------
    SvXMLImportContext* OPropertyImport::CreateChildContext(sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList)
    {
        if( token::IsXMLToken( _rLocalName, token::XML_PROPERTIES) )
        {
            return new OPropertyElementsContext( m_rContext.getGlobalContext(),
                                                 _nPrefix, _rLocalName, this);
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

            if (m_bTrackAttributes)
                m_aEncounteredAttributes.insert(sLocalName);
        }

        // TODO: create PropertyValues for all the attributes which were not present, because they were implied
        // this is necessary as soon as we have properties where the XML default is different from the property
        // default
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
            case TypeClass_BOOLEAN:     // sal_Bool
            {
                sal_Bool bValue;
            #if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
            #endif
                _rImporter.GetMM100UnitConverter().convertBool(bValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OString("OPropertyImport::convertString: could not convert \"")
                    +=  ::rtl::OString(_rReadCharacters.getStr(), _rReadCharacters.getLength(), RTL_TEXTENCODING_ASCII_US)
                    +=  ::rtl::OString("\" into a boolean!"));
                aReturn = ::cppu::bool2any(_bInvertBoolean ? !bValue : bValue);
            }
            break;
            case TypeClass_SHORT:       // sal_Int16
            case TypeClass_LONG:        // sal_Int32
                if (!_pEnumMap)
                {   // it's a real int32/16 property
                    sal_Int32 nValue(0);
            #if OSL_DEBUG_LEVEL > 0
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
                #if OSL_DEBUG_LEVEL > 0
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
        sal_uInt32 nIntValue = sal_Int32(_nValue * 8640000);
        nIntValue *= 8640000;
        aTime.HundredthSeconds = (sal_uInt16)( nIntValue % 100 );
        nIntValue /= 100;
        aTime.Seconds = (sal_uInt16)( nIntValue % 60 );
        nIntValue /= 60;
        aTime.Minutes = (sal_uInt16)( nIntValue % 60 );
        nIntValue /= 60;
        OSL_ENSURE(nIntValue < 24, "OPropertyImport::implGetTime: more than a day?");
        aTime.Hours = static_cast< sal_uInt16 >( nIntValue );

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
        if( token::IsXMLToken( _rLocalName, token::XML_PROPERTY ) )
        {
            return new OSinglePropertyContext(GetImport(), _nPrefix, _rLocalName, m_xPropertyImporter);
        }
        else if( token::IsXMLToken( _rLocalName, token::XML_LIST_PROPERTY ) )
        {
            OSL_ENSURE(sal_False, "list properties aren't supported" );
            return new SvXMLImportContext(GetImport(), _nPrefix, _rLocalName);
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
        OSL_ENSURE(sal_False,
                ::rtl::OString("OSinglePropertyContext::CreateChildContext: unknown child element (\"")
            +=  ::rtl::OString(_rLocalName.getStr(), _rLocalName.getLength(), RTL_TEXTENCODING_ASCII_US)
            +=  ::rtl::OString("\")!"));
        return new SvXMLImportContext(GetImport(), _nPrefix, _rLocalName);
    }

    //---------------------------------------------------------------------
    void OSinglePropertyContext::StartElement(const Reference< sax::XAttributeList >& _rxAttrList)
    {
        ::com::sun::star::beans::PropertyValue aPropValue;      // the property the instance imports currently
        ::com::sun::star::uno::Type aPropType;          // the type of the property the instance imports currently

        ::rtl::OUString sType, sValue;
        sal_Int16 nAttrCount = _rxAttrList.is() ? _rxAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            const ::rtl::OUString& rAttrName = _rxAttrList->getNameByIndex( i );
            const ::rtl::OUString& rValue = _rxAttrList->getValueByIndex( i );

            ::rtl::OUString aLocalName;
            sal_uInt16 nPrefix =
                GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
            if( XML_NAMESPACE_FORM == nPrefix )
            {
                if( token::IsXMLToken( aLocalName, token::XML_PROPERTY_NAME ) )
                    aPropValue.Name = rValue;

            }
            else if( XML_NAMESPACE_OFFICE == nPrefix )
            {
                if( token::IsXMLToken( aLocalName, token::XML_VALUE_TYPE ) )
                    sType = rValue;
                else if( token::IsXMLToken( aLocalName,
                                            token::XML_VALUE ) ||
                            token::IsXMLToken( aLocalName,
                                            token::XML_BOOLEAN_VALUE ) ||
                         token::IsXMLToken( aLocalName,
                                            token::XML_STRING_VALUE ) )
                    sValue = rValue;
            }
        }

        // the name of the property
        OSL_ENSURE(aPropValue.Name.getLength(), "OSinglePropertyContext::StartElement: invalid property name!");

        // needs to be translated into a ::com::sun::star::uno::Type
        DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Type, MapString2Type );
        static MapString2Type s_aTypeNameMap;
        if (!s_aTypeNameMap.size())
        {
            s_aTypeNameMap[token::GetXMLToken( token::XML_BOOLEAN)] = ::getBooleanCppuType();
            s_aTypeNameMap[token::GetXMLToken( token::XML_FLOAT)]   = ::getCppuType( static_cast< double* >(NULL) );
            s_aTypeNameMap[token::GetXMLToken( token::XML_STRING)]  = ::getCppuType( static_cast< ::rtl::OUString* >(NULL) );
            s_aTypeNameMap[token::GetXMLToken( token::XML_VOID)]    = ::getVoidCppuType();
        }

        const ConstMapString2TypeIterator aTypePos = s_aTypeNameMap.find(sType);
        OSL_ENSURE(s_aTypeNameMap.end() != aTypePos, "OSinglePropertyContext::StartElement: invalid property name!");
        if (s_aTypeNameMap.end() != aTypePos)
            aPropType = aTypePos->second;
        if( TypeClass_VOID == aPropType.getTypeClass() )
        {
            aPropValue.Value = Any();
        }
        else
        {
            aPropValue.Value =
                OPropertyImport::convertString(GetImport(), aPropType,
                                               sValue);
        }

        // now that we finally have our property value, add it to our parent object
        if( aPropValue.Name.getLength() )
            m_xPropertyImporter->implPushBackGenericPropertyValue(aPropValue);
    }

#if OSL_DEBUG_LEVEL > 0
    //---------------------------------------------------------------------
    void OSinglePropertyContext::Characters(const ::rtl::OUString& _rChars)
    {
        OSL_ENSURE(0 == _rChars.trim(), "OSinglePropertyContext::Characters: non-whitespace characters detected!");
        SvXMLImportContext::Characters(_rChars);
    }
#endif
//.........................................................................
}   // namespace xmloff
//.........................................................................

