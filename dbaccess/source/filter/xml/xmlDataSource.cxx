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
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <strings.hxx>
#include <tools/diagnose_ex.h>
#include "xmlConnectionData.hxx"

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLDataSource::OXMLDataSource( ODBFilter& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& _xAttrList,
        const UsedFor _eUsedFor ) :
    SvXMLImportContext( rImport )
{

    Reference<XPropertySet> xDataSource = rImport.getDataSource();

    PropertyValue aProperty;
    bool bFoundParamNameSubstitution = false;
    bool bFoundTableNameLengthLimited = false;
    bool bFoundAppendTableAliasName = false;
    bool bFoundSuppressVersionColumns = false;

    if (xDataSource.is())
    {
        for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
        {
            aProperty.Name.clear();
            aProperty.Value = Any();

            switch( aIter.getToken() & TOKEN_MASK )
            {
                case XML_CONNECTION_RESOURCE:
                    try
                    {
                        xDataSource->setPropertyValue(PROPERTY_URL,makeAny(aIter.toString()));
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                    break;
                case XML_SUPPRESS_VERSION_COLUMNS:
                    try
                    {
                        xDataSource->setPropertyValue(PROPERTY_SUPPRESSVERSIONCL,makeAny(IsXMLToken(aIter, XML_TRUE)));
                        bFoundSuppressVersionColumns = true;
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                    break;
                case XML_JAVA_DRIVER_CLASS:
                    aProperty.Name = INFO_JDBCDRIVERCLASS;
                    break;
                case XML_EXTENSION:
                    aProperty.Name = INFO_TEXTFILEEXTENSION;
                    break;
                case XML_IS_FIRST_ROW_HEADER_LINE:
                    aProperty.Name = INFO_TEXTFILEHEADER;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    break;
                case XML_SHOW_DELETED:
                    aProperty.Name = INFO_SHOWDELETEDROWS;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    break;
                case XML_IS_TABLE_NAME_LENGTH_LIMITED:
                    aProperty.Name = INFO_ALLOWLONGTABLENAMES;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    bFoundTableNameLengthLimited = true;
                    break;
                case XML_SYSTEM_DRIVER_SETTINGS:
                    aProperty.Name = INFO_ADDITIONALOPTIONS;
                    break;
                case XML_ENABLE_SQL92_CHECK:
                    aProperty.Name = PROPERTY_ENABLESQL92CHECK;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    break;
                case XML_APPEND_TABLE_ALIAS_NAME:
                    aProperty.Name = INFO_APPEND_TABLE_ALIAS;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    bFoundAppendTableAliasName = true;
                    break;
                case XML_PARAMETER_NAME_SUBSTITUTION:
                    aProperty.Name = INFO_PARAMETERNAMESUBST;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    bFoundParamNameSubstitution = true;
                    break;
                case XML_IGNORE_DRIVER_PRIVILEGES:
                    aProperty.Name = INFO_IGNOREDRIVER_PRIV;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    break;
                case XML_BOOLEAN_COMPARISON_MODE:
                    aProperty.Name = PROPERTY_BOOLEANCOMPARISONMODE;
                    if ( aIter.toView() == "equal-integer" )
                        aProperty.Value <<= sal_Int32(0);
                    else if ( aIter.toView() == "is-boolean" )
                        aProperty.Value <<= sal_Int32(1);
                    else if ( aIter.toView() == "equal-boolean" )
                        aProperty.Value <<= sal_Int32(2);
                    else if ( aIter.toView() == "equal-use-only-zero" )
                        aProperty.Value <<= sal_Int32(3);
                    break;
                case XML_USE_CATALOG:
                    aProperty.Name = INFO_USECATALOG;
                    aProperty.Value <<= IsXMLToken(aIter, XML_TRUE);
                    break;
                case XML_BASE_DN:
                    aProperty.Name = INFO_CONN_LDAP_BASEDN;
                    break;
                case XML_MAX_ROW_COUNT:
                    aProperty.Name = INFO_CONN_LDAP_ROWCOUNT;
                    aProperty.Value <<= aIter.toInt32();
                    break;
                case XML_JAVA_CLASSPATH:
                    aProperty.Name = "JavaDriverClassPath";
                    break;
                default:
                    XMLOFF_WARN_UNKNOWN("dbaccess", aIter);
            }
            if ( !aProperty.Name.isEmpty() )
            {
                if ( !aProperty.Value.hasValue() )
                    aProperty.Value <<= aIter.toString();
                rImport.addInfo(aProperty);
            }
        }
    }
    if ( !rImport.isNewFormat() )
        return;

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

OXMLDataSource::~OXMLDataSource()
{

}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLDataSource::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement & TOKEN_MASK )
    {
        case XML_LOGIN:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLLogin( GetOwnImport(), xAttrList );
            break;

        case XML_TABLE_FILTER:
        case XML_TABLE_TYPE_FILTER:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLTableFilterList( GetImport() );
            break;
        case XML_AUTO_INCREMENT:
        case XML_DELIMITER:
        case XML_FONT_CHARSET:
        case XML_CHARACTER_SET:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceInfo( GetOwnImport(), nElement, xAttrList );
            break;
        case XML_DATA_SOURCE_SETTINGS:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSettings( GetOwnImport() );
            break;
        case XML_CONNECTION_DATA:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLConnectionData( GetOwnImport() );
            break;
        case XML_DRIVER_SETTINGS:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSource( GetOwnImport(), xAttrList, OXMLDataSource::eDriverSettings );
            break;
        case XML_APPLICATION_CONNECTION_SETTINGS:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSource( GetOwnImport(), xAttrList, OXMLDataSource::eAppSettings );
            break;
        default:
            SAL_WARN("dbaccess", "unknown element " << nElement);
    }

    return pContext;
}

ODBFilter& OXMLDataSource::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
