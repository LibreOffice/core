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

#include "xmlQuery.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "xmlEnums.hxx"
#include <strings.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;


OXMLQuery::OXMLQuery( ODBFilter& rImport
                ,const Reference< XFastAttributeList > & _xAttrList
                ,const css::uno::Reference< css::container::XNameAccess >& _xParentContainer
                ) :
    OXMLTable( rImport, _xAttrList,_xParentContainer, u"com.sun.star.sdb.CommandDefinition"_ustr )
        ,m_bEscapeProcessing(true)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( _xAttrList ))
    {
        switch( aIter.getToken() & TOKEN_MASK )
        {
            case XML_COMMAND:
                m_sCommand = aIter.toString();
                break;
            case XML_ESCAPE_PROCESSING:
                m_bEscapeProcessing = aIter.toView() == "true";
                break;
            default:
                XMLOFF_WARN_UNKNOWN("dbaccess", aIter);
        }
    }
}

OXMLQuery::~OXMLQuery()
{

}

css::uno::Reference< css::xml::sax::XFastContextHandler > OXMLQuery::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext = OXMLTable::createFastChildContext(nElement,xAttrList );
    if (!xContext)
    {
        switch( nElement & TOKEN_MASK )
        {
            case XML_UPDATE_TABLE:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                OUString s1;
                fillAttributes(xAttrList,s1,m_sTable,m_sSchema,m_sCatalog);
                break;
            }
        }
    }

    return xContext;
}

void OXMLQuery::setProperties(Reference< XPropertySet > & _xProp )
{
    try
    {
        if ( _xProp.is() )
        {
            OXMLTable::setProperties(_xProp);

            _xProp->setPropertyValue(PROPERTY_COMMAND,Any(m_sCommand));
            _xProp->setPropertyValue(PROPERTY_ESCAPE_PROCESSING,Any(m_bEscapeProcessing));

            if ( !m_sTable.isEmpty() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_TABLENAME,Any(m_sTable));
            if ( !m_sCatalog.isEmpty() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_CATALOGNAME,Any(m_sCatalog));
            if ( !m_sSchema.isEmpty() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_SCHEMANAME,Any(m_sSchema));

            const ODBFilter::TPropertyNameMap& rSettings = GetOwnImport().getQuerySettings();
            ODBFilter::TPropertyNameMap::const_iterator aFind = rSettings.find(m_sName);
            if ( aFind != rSettings.end() )
                _xProp->setPropertyValue(PROPERTY_LAYOUTINFORMATION,Any(aFind->second));
        }
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION( "dbaccess", "OXMLTable::EndElement");
    }
}

} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
