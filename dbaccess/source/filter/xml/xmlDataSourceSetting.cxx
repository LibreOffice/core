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

#include "sal/config.h"

#include <map>

#include "xmlDataSourceSetting.hxx"
#include "xmlDataSource.hxx"
#include <sax/tools/converter.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <rtl/strbuf.hxx>
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLDataSourceSetting)

OXMLDataSourceSetting::OXMLDataSourceSetting( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,OXMLDataSourceSetting* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_pContainer(_pContainer)
    ,m_bIsList(sal_False)
{
    DBG_CTOR(OXMLDataSourceSetting,NULL);

    m_aPropType = ::getVoidCppuType();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceInfoElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_DATA_SOURCE_SETTING_IS_LIST:
                m_bIsList = sValue == "true";
                break;
            case XML_TOK_DATA_SOURCE_SETTING_TYPE:
                {
                    // needs to be translated into a ::com::sun::star::uno::Type
                    static std::map< OUString, css::uno::Type > s_aTypeNameMap;
                    if (s_aTypeNameMap.empty())
                    {
                        s_aTypeNameMap[GetXMLToken( XML_BOOLEAN)]   = ::getBooleanCppuType();
                        s_aTypeNameMap[GetXMLToken( XML_FLOAT)]     = ::getCppuType( static_cast< double* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_DOUBLE)]    = ::getCppuType( static_cast< double* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_STRING)]    = ::getCppuType( static_cast< OUString* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_INT)]       = ::getCppuType( static_cast< sal_Int32* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_SHORT)]     = ::getCppuType( static_cast< sal_Int16* >(NULL) );
                        s_aTypeNameMap[GetXMLToken( XML_VOID)]      = ::getVoidCppuType();
                    }

                    const std::map< OUString, css::uno::Type >::const_iterator aTypePos = s_aTypeNameMap.find(sValue);
                    OSL_ENSURE(s_aTypeNameMap.end() != aTypePos, "OXMLDataSourceSetting::OXMLDataSourceSetting: invalid type!");
                    if (s_aTypeNameMap.end() != aTypePos)
                        m_aPropType = aTypePos->second;
                }
                break;
            case XML_TOK_DATA_SOURCE_SETTING_NAME:
                m_aSetting.Name = sValue;
                break;
        }
    }

}

OXMLDataSourceSetting::~OXMLDataSourceSetting()
{
    DBG_DTOR(OXMLDataSourceSetting,NULL);
}

SvXMLImportContext* OXMLDataSourceSetting::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDataSourceInfoElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_DATA_SOURCE_SETTING:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSetting( GetOwnImport(), nPrefix, rLocalName,xAttrList);
            break;
        case XML_TOK_DATA_SOURCE_SETTING_VALUE:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSetting( GetOwnImport(), nPrefix, rLocalName,xAttrList,this );
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void OXMLDataSourceSetting::EndElement()
{
    if ( !m_aSetting.Name.isEmpty() )
    {
        if ( m_bIsList && m_aInfoSequence.getLength() )
            m_aSetting.Value <<= m_aInfoSequence;

        // if our property is of type string, but was empty, ensure that
        // we don't add a VOID value
        if ( !m_bIsList && ( m_aPropType.getTypeClass() == TypeClass_STRING ) && !m_aSetting.Value.hasValue() )
            m_aSetting.Value <<= OUString();

        GetOwnImport().addInfo(m_aSetting);
    }
}

void OXMLDataSourceSetting::Characters( const OUString& rChars )
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

Any OXMLDataSourceSetting::convertString(const ::com::sun::star::uno::Type& _rExpectedType, const OUString& _rReadCharacters)
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
                    aReturn <<= (sal_Int16)nValue;
                else
                    aReturn <<= (sal_Int32)nValue;
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
            aReturn <<= (double)nValue;
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
