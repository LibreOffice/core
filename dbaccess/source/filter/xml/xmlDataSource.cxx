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

#include "xmlDataSource.hxx"
#include "xmlLogin.hxx"
#include "xmlTableFilterList.hxx"
#include "xmlDataSourceInfo.hxx"
#include "xmlDataSourceSettings.hxx"
#include "xmlDataSourceSetting.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <stringconstants.hxx>
#include <strings.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include "xmlConnectionData.hxx"

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLDataSource::OXMLDataSource( ODBFilter& rImport,
                sal_uInt16 nPrfx, const OUString& _sLocalName,
                const Reference< XAttributeList > & _xAttrList, const UsedFor _eUsedFor ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceElemTokenMap();

    Reference<XPropertySet> xDataSource = rImport.getDataSource();

    PropertyValue aProperty;
    bool bFoundParamNameSubstitution = false;
    bool bFoundTableNameLengthLimited = false;
    bool bFoundAppendTableAliasName = false;
    bool bFoundSuppressVersionColumns = false;

    const sal_Int16 nLength = (xDataSource.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        OUString sValue = _xAttrList->getValueByIndex( i );

        aProperty.Name.clear();
        aProperty.Value = Any();

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_CONNECTION_RESOURCE:
                try
                {
                    xDataSource->setPropertyValue(PROPERTY_URL,makeAny(sValue));
                }
                catch(const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
                }
                break;
            case XML_TOK_SUPPRESS_VERSION_COLUMNS:
                try
                {
                    xDataSource->setPropertyValue(PROPERTY_SUPPRESSVERSIONCL,makeAny(sValue == s_sTRUE));
                    bFoundSuppressVersionColumns = true;
                }
                catch(const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
                }
                break;
            case XML_TOK_JAVA_DRIVER_CLASS:
                aProperty.Name = INFO_JDBCDRIVERCLASS;
                break;
            case XML_TOK_EXTENSION:
                aProperty.Name = INFO_TEXTFILEEXTENSION;
                break;
            case XML_TOK_IS_FIRST_ROW_HEADER_LINE:
                aProperty.Name = INFO_TEXTFILEHEADER;
                aProperty.Value <<= (sValue == s_sTRUE);
                break;
            case XML_TOK_SHOW_DELETED:
                aProperty.Name = INFO_SHOWDELETEDROWS;
                aProperty.Value <<= (sValue == s_sTRUE);
                break;
            case XML_TOK_IS_TABLE_NAME_LENGTH_LIMITED:
                aProperty.Name = INFO_ALLOWLONGTABLENAMES;
                aProperty.Value <<= (sValue == s_sTRUE);
                bFoundTableNameLengthLimited = true;
                break;
            case XML_TOK_SYSTEM_DRIVER_SETTINGS:
                aProperty.Name = INFO_ADDITIONALOPTIONS;
                break;
            case XML_TOK_ENABLE_SQL92_CHECK:
                aProperty.Name = PROPERTY_ENABLESQL92CHECK;
                aProperty.Value <<= (sValue == s_sTRUE);
                break;
            case XML_TOK_APPEND_TABLE_ALIAS_NAME:
                aProperty.Name = INFO_APPEND_TABLE_ALIAS;
                aProperty.Value <<= (sValue == s_sTRUE);
                bFoundAppendTableAliasName = true;
                break;
            case XML_TOK_PARAMETER_NAME_SUBSTITUTION:
                aProperty.Name = INFO_PARAMETERNAMESUBST;
                aProperty.Value <<= (sValue == s_sTRUE);
                bFoundParamNameSubstitution = true;
                break;
            case XML_TOK_IGNORE_DRIVER_PRIVILEGES:
                aProperty.Name = INFO_IGNOREDRIVER_PRIV;
                aProperty.Value <<= (sValue == s_sTRUE);
                break;
            case XML_TOK_BOOLEAN_COMPARISON_MODE:
                aProperty.Name = PROPERTY_BOOLEANCOMPARISONMODE;
                if ( sValue == "equal-integer" )
                    aProperty.Value <<= sal_Int32(0);
                else if ( sValue == "is-boolean" )
                    aProperty.Value <<= sal_Int32(1);
                else if ( sValue == "equal-boolean" )
                    aProperty.Value <<= sal_Int32(2);
                else if ( sValue == "equal-use-only-zero" )
                    aProperty.Value <<= sal_Int32(3);
                break;
            case XML_TOK_USE_CATALOG:
                aProperty.Name = INFO_USECATALOG;
                aProperty.Value <<= (sValue == s_sTRUE);
                break;
            case XML_TOK_BASE_DN:
                aProperty.Name = INFO_CONN_LDAP_BASEDN;
                break;
            case XML_TOK_MAX_ROW_COUNT:
                aProperty.Name = INFO_CONN_LDAP_ROWCOUNT;
                aProperty.Value <<= sValue.toInt32();
                break;
            case XML_TOK_JAVA_CLASSPATH:
                aProperty.Name = "JavaDriverClassPath";
                break;
        }
        if ( !aProperty.Name.isEmpty() )
        {
            if ( !aProperty.Value.hasValue() )
                aProperty.Value <<= sValue;
            rImport.addInfo(aProperty);
        }
    }
    if ( rImport.isNewFormat() )
    {
        if ( !bFoundTableNameLengthLimited && ( _eUsedFor == eAppSettings ) )
        {
            aProperty.Name = INFO_ALLOWLONGTABLENAMES;
            aProperty.Value <<= true;
            rImport.addInfo(aProperty);
        }
        if ( !bFoundParamNameSubstitution && ( _eUsedFor == eDriverSettings ) )
        {
            aProperty.Name = INFO_PARAMETERNAMESUBST;
            aProperty.Value <<= true;
            rImport.addInfo(aProperty);
        }
        if ( !bFoundAppendTableAliasName && ( _eUsedFor == eAppSettings ) )
        {
            aProperty.Name = INFO_APPEND_TABLE_ALIAS;
            aProperty.Value <<= true;
            rImport.addInfo(aProperty);
        }
        if ( !bFoundSuppressVersionColumns && ( _eUsedFor == eAppSettings ) )
        {
            try
            {
                xDataSource->setPropertyValue(PROPERTY_SUPPRESSVERSIONCL,makeAny(true));
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
    }
}

OXMLDataSource::~OXMLDataSource()
{

}

SvXMLImportContextRef OXMLDataSource::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDataSourceElemTokenMap();
    const sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );

    switch( nToken )
    {
        case XML_TOK_LOGIN:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLLogin( GetOwnImport(), nPrefix, rLocalName,xAttrList );
            break;

        case XML_TOK_TABLE_FILTER:
        case XML_TOK_TABLE_TYPE_FILTER:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLTableFilterList( GetImport(), nPrefix, rLocalName );
            break;
        case XML_TOK_AUTO_INCREMENT:
        case XML_TOK_DELIMITER:
        case XML_TOK_FONT_CHARSET:
        case XML_TOK_CHARACTER_SET:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceInfo( GetOwnImport(), nPrefix, rLocalName,xAttrList,nToken);
            break;
        case XML_TOK_DATA_SOURCE_SETTINGS:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSettings( GetOwnImport(), nPrefix, rLocalName);
            break;
        case XML_TOK_CONNECTION_DATA:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLConnectionData( GetOwnImport(), nPrefix, rLocalName);
            break;
        case XML_TOK_DRIVER_SETTINGS:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSource( GetOwnImport(), nPrefix, rLocalName, xAttrList, OXMLDataSource::eDriverSettings );
            break;
        case XML_TOK_APPLICATION_CONNECTION_SETTINGS:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSource( GetOwnImport(), nPrefix, rLocalName, xAttrList, OXMLDataSource::eAppSettings );
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

ODBFilter& OXMLDataSource::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
