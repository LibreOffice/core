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

#include "xmlControlProperty.hxx"

#include <rtl/strbuf.hxx>
#include <sax/tools/converter.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <com/sun/star/util/DateTime.hpp>

#define TYPE_DATE       1
#define TYPE_TIME       2
#define TYPE_DATETIME   3

namespace rptxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;

DBG_NAME( rpt_OXMLControlProperty )

OXMLControlProperty::OXMLControlProperty( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XPropertySet >& _xControl
                ,OXMLControlProperty* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xControl(_xControl)
    ,m_pContainer(_pContainer)
    ,m_bIsList(sal_False)
{
    DBG_CTOR( rpt_OXMLControlProperty,NULL);

    m_aPropType = ::getVoidCppuType();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    OSL_ENSURE(m_xControl.is(),"Control is NULL!");

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetControlPropertyElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
     ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_LIST_PROPERTY:
                m_bIsList = sValue == "true";
                break;
            case XML_TOK_VALUE_TYPE:
                {
                    // needs to be translated into a ::com::sun::star::uno::Type
                    DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Type, MapString2Type );
                    static MapString2Type s_aTypeNameMap;
                    if (!s_aTypeNameMap.size())
                    {
                        s_aTypeNameMap[GetXMLToken( XML_BOOLEAN)]   = ::getBooleanCppuType();
                        s_aTypeNameMap[GetXMLToken( XML_FLOAT)]     = ::getCppuType( static_cast< double* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_DOUBLE)]    = ::getCppuType( static_cast< double* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_STRING)]    = ::getCppuType( static_cast< ::rtl::OUString* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_INT)]       = ::getCppuType( static_cast< sal_Int32* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_SHORT)]     = ::getCppuType( static_cast< sal_Int16* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_DATE)]      = ::getCppuType( static_cast< com::sun::star::util::Date* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_TIME)]      = ::getCppuType( static_cast< com::sun::star::util::Time* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_VOID)]      = ::getVoidCppuType();
                    }

                    const ConstMapString2TypeIterator aTypePos = s_aTypeNameMap.find(sValue);
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
// -----------------------------------------------------------------------------

OXMLControlProperty::~OXMLControlProperty()
{
    DBG_DTOR( rpt_OXMLControlProperty,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLControlProperty::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
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
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void OXMLControlProperty::Characters( const ::rtl::OUString& rChars )
{
    if ( m_pContainer )
        m_pContainer->addValue(rChars);
}
// -----------------------------------------------------------------------------
void OXMLControlProperty::addValue(const ::rtl::OUString& _sValue)
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
// -----------------------------------------------------------------------------
ORptFilter& OXMLControlProperty::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
Any OXMLControlProperty::convertString(const ::com::sun::star::uno::Type& _rExpectedType, const ::rtl::OUString& _rReadCharacters)
{
    Any aReturn;
    switch (_rExpectedType.getTypeClass())
    {
        case TypeClass_BOOLEAN:     // sal_Bool
        {
            bool bValue(false);
        #if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
        #endif
                ::sax::Converter::convertBool(bValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    ::rtl::OStringBuffer("OXMLControlProperty::convertString: could not convert \"").
                append(::rtl::OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                append("\" into a boolean!").getStr());
            aReturn <<= bValue;
        }
        break;
        case TypeClass_SHORT:       // sal_Int16
        case TypeClass_LONG:        // sal_Int32
            {   // it's a real int32/16 property
                sal_Int32 nValue(0);
        #if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
        #endif
                    ::sax::Converter::convertNumber(nValue, _rReadCharacters);
                OSL_ENSURE(bSuccess,
                        ::rtl::OStringBuffer("OXMLControlProperty::convertString: could not convert \"").
                    append(rtl::OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                    append("\" into an integer!").getStr());
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= (sal_Int16)nValue;
                else
                    aReturn <<= (sal_Int32)nValue;
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
        #if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
        #endif
                ::sax::Converter::convertDouble(nValue, _rReadCharacters);
            OSL_ENSURE(bSuccess,
                    ::rtl::OStringBuffer("OXMLControlProperty::convertString: could not convert \"").
                append(::rtl::OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                append("\" into a double!").getStr());
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
                if  ( _rExpectedType.equals(s_aDateType) )
                    nType = TYPE_DATE;
                else if ( _rExpectedType.equals(s_aTimeType) )
                    nType = TYPE_TIME;
                else if ( _rExpectedType.equals(s_aDateTimeType) )
                    nType = TYPE_DATETIME;
                if ( !nType )
                {
                    // first extract the double
                    double nValue = 0;
                #if OSL_DEBUG_LEVEL > 0
                    sal_Bool bSuccess =
                #endif
                    ::sax::Converter::convertDouble(nValue, _rReadCharacters);
                    OSL_ENSURE(bSuccess,
                            ::rtl::OStringBuffer("OPropertyImport::convertString: could not convert \"").
                        append(rtl::OUStringToOString(_rReadCharacters, RTL_TEXTENCODING_ASCII_US)).
                        append("\" into a double!").getStr());

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
//---------------------------------------------------------------------
::com::sun::star::util::Time OXMLControlProperty::implGetTime(double _nValue)
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
::com::sun::star::util::Date OXMLControlProperty::implGetDate(double _nValue)
{
    Date aToolsDate((sal_uInt32)_nValue);
    ::com::sun::star::util::Date aDate;
    ::utl::typeConvert(aToolsDate, aDate);
    return aDate;
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
