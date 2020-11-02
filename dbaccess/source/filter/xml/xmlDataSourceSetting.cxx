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
#include <sal/log.hxx>

#include <map>

#include "xmlDataSourceSetting.hxx"
#include <sax/tools/converter.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <osl/diagnose.h>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLDataSourceSetting::OXMLDataSourceSetting( ODBFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,OXMLDataSourceSetting* _pContainer) :
    SvXMLImportContext( rImport )
    ,m_pContainer(_pContainer)
    ,m_bIsList(false)
{

    m_aPropType = cppu::UnoType<void>::get();

    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        OUString sValue = aIter.toString();

        switch( aIter.getToken() & TOKEN_MASK )
        {
            case XML_DATA_SOURCE_SETTING_IS_LIST:
                m_bIsList = sValue == "true";
                break;
            case XML_DATA_SOURCE_SETTING_TYPE:
                {
                    // needs to be translated into a css::uno::Type
                    static std::map< OUString, css::uno::Type > s_aTypeNameMap = [&]()
                    {
                        std::map< OUString, css::uno::Type > tmp;
                        tmp[GetXMLToken( XML_BOOLEAN)]   = cppu::UnoType<bool>::get();
                        // Not a copy paste error, see comment xmloff/source/forms/propertyimport.cxx lines 244-248
                        tmp[GetXMLToken( XML_FLOAT)]     = ::cppu::UnoType<double>::get();
                        tmp[GetXMLToken( XML_DOUBLE)]    = ::cppu::UnoType<double>::get();
                        tmp[GetXMLToken( XML_STRING)]    = ::cppu::UnoType<OUString>::get();
                        tmp[GetXMLToken( XML_INT)]       = ::cppu::UnoType<sal_Int32>::get();
                        tmp[GetXMLToken( XML_SHORT)]     = ::cppu::UnoType<sal_Int16>::get();
                        tmp[GetXMLToken( XML_VOID)]      = cppu::UnoType<void>::get();
                        return tmp;
                    }();

                    const std::map< OUString, css::uno::Type >::const_iterator aTypePos = s_aTypeNameMap.find(sValue);
                    OSL_ENSURE(s_aTypeNameMap.end() != aTypePos, "OXMLDataSourceSetting::OXMLDataSourceSetting: invalid type!");
                    if (s_aTypeNameMap.end() != aTypePos)
                        m_aPropType = aTypePos->second;
                }
                break;
            case XML_DATA_SOURCE_SETTING_NAME:
                m_aSetting.Name = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN_ATTR("dbaccess", aIter.getToken(), aIter.toString());
        }
    }

}

OXMLDataSourceSetting::~OXMLDataSourceSetting()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLDataSourceSetting::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement & TOKEN_MASK )
    {
        case XML_DATA_SOURCE_SETTING:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSetting( GetOwnImport(), xAttrList);
            break;
        case XML_DATA_SOURCE_SETTING_VALUE:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSetting( GetOwnImport(), xAttrList,this );
            break;
    }

    return pContext;
}

void OXMLDataSourceSetting::endFastElement(sal_Int32 )
{
    if ( !m_aSetting.Name.isEmpty() )
    {
        if ( m_bIsList && m_aInfoSequence.hasElements() )
            m_aSetting.Value <<= m_aInfoSequence;

        // if our property is of type string, but was empty, ensure that
        // we don't add a VOID value
        if ( !m_bIsList && ( m_aPropType.getTypeClass() == TypeClass_STRING ) && !m_aSetting.Value.hasValue() )
            m_aSetting.Value <<= OUString();

        GetOwnImport().addInfo(m_aSetting);
    }
}

void OXMLDataSourceSetting::characters( const OUString& rChars )
{
    if ( m_pContainer )
        m_pContainer->addValue(rChars);
}

void OXMLDataSourceSetting::addValue(const OUString& _sValue)
{
    Any aValue;
    if( TypeClass_VOID != m_aPropType.getTypeClass() )
        aValue = convertString(m_aPropType, _sValue);

    if ( !m_bIsList )
        m_aSetting.Value = aValue;
    else
    {
        sal_Int32 nPos = m_aInfoSequence.getLength();
        m_aInfoSequence.realloc(nPos+1);
        m_aInfoSequence[nPos] = aValue;
    }
}

ODBFilter& OXMLDataSourceSetting::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

Any OXMLDataSourceSetting::convertString(const css::uno::Type& _rExpectedType, const OUString& _rReadCharacters)
{
    Any aReturn;
    switch (_rExpectedType.getTypeClass())
    {
        case TypeClass_BOOLEAN:     // sal_Bool
        {
            bool bValue(false);
            bool const bSuccess =
                ::sax::Converter::convertBool(bValue, _rReadCharacters);
            SAL_WARN_IF(!bSuccess, "dbaccess",
                "OXMLDataSourceSetting::convertString: could not convert \""
                << _rReadCharacters << "\" into a boolean!");
            aReturn <<= bValue;
        }
        break;
        case TypeClass_SHORT:       // sal_Int16
        case TypeClass_LONG:        // sal_Int32
            {   // it's a real int32/16 property
                sal_Int32 nValue(0);
                bool const bSuccess =
                    ::sax::Converter::convertNumber(nValue, _rReadCharacters);
                SAL_WARN_IF(!bSuccess, "dbaccess",
                    "OXMLDataSourceSetting::convertString: could not convert \""
                    << _rReadCharacters << "\" into an integer!");
                if (TypeClass_SHORT == _rExpectedType.getTypeClass())
                    aReturn <<= static_cast<sal_Int16>(nValue);
                else
                    aReturn <<= nValue;
                break;
            }
        case TypeClass_HYPER:
        {
            OSL_FAIL("OXMLDataSourceSetting::convertString: 64-bit integers not implemented yet!");
        }
        break;
        case TypeClass_DOUBLE:
        {
            double nValue = 0.0;
            bool const bSuccess =
                ::sax::Converter::convertDouble(nValue, _rReadCharacters);
            SAL_WARN_IF(!bSuccess, "dbaccess",
                "OXMLDataSourceSetting::convertString: could not convert \""
                << _rReadCharacters << "\" into a double!");
            aReturn <<= nValue;
        }
        break;
        case TypeClass_STRING:
            aReturn <<= _rReadCharacters;
            break;
        default:
            SAL_WARN("dbaccess",
                "OXMLDataSourceSetting::convertString: invalid type class!");
    }

    return aReturn;
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
