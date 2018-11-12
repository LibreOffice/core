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

#include <sal/config.h>

#include <cmath>
#include <map>

#include "xmlControlProperty.hxx"

#include <o3tl/temporary.hxx>
#include <rtl/strbuf.hxx>
#include <sax/tools/converter.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <rtl/math.hxx>

#define TYPE_DATE       1
#define TYPE_TIME       2
#define TYPE_DATETIME   3

namespace rptxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;

OXMLControlProperty::OXMLControlProperty( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XPropertySet >& _xControl
                ,OXMLControlProperty* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xControl(_xControl)
    ,m_pContainer(_pContainer)
    ,m_bIsList(false)
{
    m_aPropType = cppu::UnoType<void>::get();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    OSL_ENSURE(m_xControl.is(),"Control is NULL!");

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetControlPropertyElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        const OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_LIST_PROPERTY:
                m_bIsList = sValue == "true";
                break;
            case XML_TOK_VALUE_TYPE:
                {
                    // needs to be translated into a css::uno::Type
                    static std::map< OUString, css::uno::Type > const s_aTypeNameMap
                    {
                        { GetXMLToken( XML_BOOLEAN)   , cppu::UnoType<bool>::get() },
                        // Not a copy paste error, see comment xmloff/source/forms/propertyimport.cxx lines 244-248
                        { GetXMLToken( XML_FLOAT)     , cppu::UnoType<double>::get() },
                        { GetXMLToken( XML_DOUBLE)    , cppu::UnoType<double>::get() },
                        { GetXMLToken( XML_STRING)    , cppu::UnoType<OUString>::get() },
                        { GetXMLToken( XML_INT)       , cppu::UnoType<sal_Int32>::get() },
                        { GetXMLToken( XML_SHORT)     , cppu::UnoType<sal_Int16>::get() },
                        { GetXMLToken( XML_DATE)      , cppu::UnoType<css::util::Date>::get() },
                        { GetXMLToken( XML_TIME)      , cppu::UnoType<css::util::Time>::get() },
                        { GetXMLToken( XML_VOID)      , cppu::UnoType<void>::get() },
                    };

                    const std::map< OUString, css::uno::Type >::const_iterator aTypePos = s_aTypeNameMap.find(sValue);
                    OSL_ENSURE(s_aTypeNameMap.end() != aTypePos, "OXMLControlProperty::OXMLControlProperty: invalid type!");
                    if (s_aTypeNameMap.end() != aTypePos)
                        m_aPropType = aTypePos->second;
                }
                break;
            case XML_TOK_PROPERTY_NAME:
                m_aSetting.Name = sValue;
                break;
            default:
                break;
        }
    }

}


OXMLControlProperty::~OXMLControlProperty()
{
}

SvXMLImportContextRef OXMLControlProperty::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetControlPropertyElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_LIST_PROPERTY:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLControlProperty( rImport, nPrefix, rLocalName,xAttrList,m_xControl);
            break;
        case XML_TOK_VALUE:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLControlProperty( rImport, nPrefix, rLocalName,xAttrList,m_xControl,this );
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void OXMLControlProperty::EndElement()
{
    if ( !m_aSetting.Name.isEmpty() && m_xControl.is() )
    {
        if ( m_bIsList && !m_aSequence.getLength() )
            m_aSetting.Value <<= m_aSequence;
        try
        {
            m_xControl->setPropertyValue(m_aSetting.Name,m_aSetting.Value);
        }
        catch(const Exception&)
        {
            OSL_FAIL("Unknown property found!");
        }
    }
}

void OXMLControlProperty::Characters( const OUString& rChars )
{
    if ( m_pContainer )
        m_pContainer->addValue(rChars);
}

void OXMLControlProperty::addValue(const OUString& _sValue)
{
    Any aValue;
    if( TypeClass_VOID != m_aPropType.getTypeClass() )
        aValue = convertString(m_aPropType, _sValue);

    if ( !m_bIsList )
        m_aSetting.Value = aValue;
    else
    {
        sal_Int32 nPos = m_aSequence.getLength();
        m_aSequence.realloc(nPos+1);
        m_aSequence[nPos] = aValue;
    }
}

ORptFilter& OXMLControlProperty::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}

Any OXMLControlProperty::convertString(const css::uno::Type& _rExpectedType, const OUString& _rReadCharacters)
{
    Any aReturn;
    switch (_rExpectedType.getTypeClass())
    {
        case TypeClass_BOOLEAN:     // sal_Bool
        {
            bool bValue(false);
            bool bSuccess =
                ::sax::Converter::convertBool(bValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    OStringBuffer("OXMLControlProperty::convertString: could not convert \"").
                append(OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                append("\" into a boolean!").getStr());
            aReturn <<= bValue;
        }
        break;
        case TypeClass_SHORT:       // sal_Int16
        case TypeClass_LONG:        // sal_Int32
            {   // it's a real int32/16 property
                sal_Int32 nValue(0);
                bool bSuccess =
                    ::sax::Converter::convertNumber(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        OStringBuffer("OXMLControlProperty::convertString: could not convert \"").
                    append(OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                    append("\" into an integer!").getStr());
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= static_cast<sal_Int16>(nValue);
                else
                    aReturn <<= nValue;
                break;
            }
        case TypeClass_HYPER:
        {
            OSL_FAIL("OXMLControlProperty::convertString: 64-bit integers not implemented yet!");
        }
        break;
        case TypeClass_DOUBLE:
        {
            double nValue = 0.0;
            bool bSuccess =
                ::sax::Converter::convertDouble(nValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    OStringBuffer("OXMLControlProperty::convertString: could not convert \"").
                append(OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                append("\" into a double!").getStr());
            aReturn <<= nValue;
        }
        break;
        case TypeClass_STRING:
            aReturn <<= _rReadCharacters;
            break;
        case TypeClass_STRUCT:
            {
                // recognized structs:
                static css::uno::Type s_aDateType      = ::cppu::UnoType<css::util::Date>::get();
                static css::uno::Type s_aTimeType      = ::cppu::UnoType<css::util::Time>::get();
                static css::uno::Type s_aDateTimeType  = ::cppu::UnoType<css::util::DateTime>::get();
                sal_Int32 nType = 0;
                if  ( _rExpectedType.equals(s_aDateType) )
                    nType = TYPE_DATE;
                else if ( _rExpectedType.equals(s_aTimeType) )
                    nType = TYPE_TIME;
                else if ( _rExpectedType.equals(s_aDateTimeType) )
                    nType = TYPE_DATETIME;
                if ( nType )
                {
                    // first extract the double
                    double nValue = 0;
                    bool bSuccess =
                        ::sax::Converter::convertDouble(nValue, _rReadCharacters);
                    OSL_ENSURE(bSuccess,
                            OStringBuffer("OPropertyImport::convertString: could not convert \"").
                        append(OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                        append("\" into a double!").getStr());

                    // then convert it into the target type
                    switch (nType)
                    {
                        case TYPE_DATE:
                        {
                            OSL_ENSURE(std::modf(nValue, &o3tl::temporary(double())) == 0,
                                "OPropertyImport::convertString: a Date value with a fractional part?");
                            aReturn <<= implGetDate(nValue);
                        }
                        break;
                        case TYPE_TIME:
                        {
                            OSL_ENSURE((static_cast<sal_uInt32>(nValue)) == 0,
                                "OPropertyImport::convertString: a tools::Time value with more than a fractional part?");
                            aReturn <<= implGetTime(nValue);
                        }
                        break;
                        case TYPE_DATETIME:
                        {
                            css::util::Time aTime = implGetTime(nValue);
                            css::util::Date aDate = implGetDate(nValue);

                            css::util::DateTime aDateTime;
                            aDateTime.NanoSeconds = aTime.NanoSeconds;
                            aDateTime.Seconds = aTime.Seconds;
                            aDateTime.Minutes = aTime.Minutes;
                            aDateTime.Hours = aTime.Hours;
                            aDateTime.Day = aDate.Day;
                            aDateTime.Month = aDate.Month;
                            aDateTime.Year = aDate.Year;
                            aReturn <<= aDateTime;
                        }
                        break;
                        default:
                            break;
                    }
                }
                else
                    OSL_FAIL("OPropertyImport::convertString: unsupported property type!");
            }
            break;
        default:
            OSL_FAIL("OXMLControlProperty::convertString: invalid type class!");
    }

    return aReturn;
}

css::util::Time OXMLControlProperty::implGetTime(double _nValue)
{
    css::util::Time aTime;
    sal_uInt64 nIntValue = ::rtl::math::round(_nValue * 86400000000000.0);
    aTime.NanoSeconds = static_cast<sal_uInt16>( nIntValue % 1000000000 );
    nIntValue /= 1000000000;
    aTime.Seconds = static_cast<sal_uInt16>( nIntValue % 60 );
    nIntValue /= 60;
    aTime.Minutes = static_cast<sal_uInt16>( nIntValue % 60 );
    nIntValue /= 60;
    OSL_ENSURE(nIntValue < 24, "OPropertyImport::implGetTime: more than a day?");
    aTime.Hours = static_cast< sal_uInt16 >( nIntValue );

    return aTime;
}


css::util::Date OXMLControlProperty::implGetDate(double _nValue)
{
    Date aToolsDate(static_cast<sal_uInt32>(_nValue));
    css::util::Date aDate;
    ::utl::typeConvert(aToolsDate, aDate);
    return aDate;
}

} // namespace rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
